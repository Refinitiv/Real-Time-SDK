package com.refinitiv.eta.examples.genericcons;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.concurrent.Executors;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.common.GenericResponseStatusFlags;
import com.refinitiv.eta.examples.common.ResponseCallback;
import com.refinitiv.eta.examples.common.DirectoryHandler;
import com.refinitiv.eta.examples.common.LoginHandler;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.shared.ConsumerLoginState;
import com.refinitiv.eta.shared.PingHandler;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

/**
 * <p>
 * This is the main class for the ETA Java Generic Consumer application. The purpose
 * of this application is to send and receive generic messages with opaque data between
 * a generic consumer and generic provider. It request files from a generic provider
 * and stores the file responses to disk.
 * </p>
 * <em>Summary</em>
 * <p>
 * This class is responsible for the following:
 * <ul>
 * <li>Initialize and set command line options.
 * <li>Create a {@link ChannelSession ETA Channel Session}.
 * <li>Create Handler instances to handle Login and Directory requests and responses.
 * <li>Connect to the provider, login, request source directory, send a request for
 * the generic domain to open a steam for generic messages.
 * <li>Get user input and process responses.
 * <li>Cleanup.
 * </ul>
 * <p>
 * This class is also a call back for all events from provider. It dispatches
 * events to domain specific handlers.
 * <p>
 * This application is intended as a basic usage example. Some of the design
 * choices were made to favor simplicity and readability over performance. It is
 * not intended to be used for measuring performance.
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * No special setup is required.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runGenericCons -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runGenericCons -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-h Server host name. Default is <i>localhost</i>.
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-i Interface name. Default is <i>null</i>.
 * <li>-uname Login user name. Default is system user name.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-x Provides XML tracing of messages.
 * <li>-runtime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * </ul>
 * <p>
 * After login, requesting source directory, and sending a request for
 * the generic domain to open a steam for generic messages, the application
 * prompts the user for the file name(s) to retrieve from the generic provider.
 */
public class GenericCons implements ResponseCallback
{
    private final int CONNECTION_RETRY_TIME = 15; // seconds
    private final int MAX_FILE_NAME_LEN = 256;
    private final int MAX_MSG_OVERHEAD = 256;
    private final int GENERIC_DOMAIN = 200; // used by generic provider
    private final int GENERIC_STREAM_ID = 200; // used by generic provider
    private final String GENERIC_ITEM = "GENERIC"; // used by generic provider
    
    private ChannelSession channelSession;
    private ChannelInfo channelInfo;
    private LoginHandler loginHandler;
    private DirectoryHandler srcDirHandler;
    
    private long runtime;
    
    // default server host name
    private static final String defaultSrvrHostname = "localhost";
    
    // default server port number
    private static final String defaultSrvrPortNo = "14002";

    // default service name
    private static final String defaultServiceName = "DIRECT_FEED";
    
    // consumer run-time in seconds
    private static final int defaultRuntime = 600;

    private Error error;    // error information
    private DecodeIterator dIter;
    private Msg responseMsg;
    private EncodeIterator encIter;
    private RequestMsg requestMsg; // used to open stream so we can send/receive generic messages
    private GenericMsg fileRequestMsg;

    private UserInputThread userInputThread;
    
    private int requestCount;

    public GenericCons()
    {
        channelSession = new ChannelSession();
        channelInfo = TransportFactory.createChannelInfo();
        loginHandler = new LoginHandler();
        srcDirHandler = new DirectoryHandler();
        error = TransportFactory.createError();
        dIter = CodecFactory.createDecodeIterator();
        responseMsg = CodecFactory.createMsg();
        encIter = CodecFactory.createEncodeIterator();
        fileRequestMsg = (GenericMsg)CodecFactory.createMsg();
        requestMsg = (RequestMsg)CodecFactory.createMsg();
        userInputThread = new UserInputThread();
        Executors.newSingleThreadExecutor().execute(userInputThread);
    }

    /* Configures command line arguments for the generic consumer application. */
    private static void addCommandLineArgs()
    {
        CommandLine.programName("GenericCons");
        CommandLine.addOption("h", defaultSrvrHostname, "Server host name");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number");
        CommandLine.addOption("i", (String)null, "Interface name");
        CommandLine.addOption("uname", "Login user name. Default is system user name.");
        CommandLine.addOption("s", defaultServiceName, "Service name");
        
        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("x", "Provides XML tracing of messages.");
    }
    
    /* Initializes the generic consumer application. */
    void init(String[] args)
    {
        // process command line args
        addCommandLineArgs();
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }
        
        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("Consumer initializing...");

        runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;

        loginHandler.applicationName("Generic Consumer");
        loginHandler.role(Login.RoleTypes.CONS);
        loginHandler.userName(CommandLine.value("uname"));
        srcDirHandler.serviceName(CommandLine.value("s"));

        if (channelSession.initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(null);
        }
    }

    /*
     * Main loop that handles channel connection, login requests, reading and
     * processing responses from channel.
     */
    void run()
    {
        PingHandler pingHandler = new PingHandler();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        while (true)
        {
            try
            {
                connectRetry(inProg);
            }
            catch (InterruptedException intExp)
            {
                System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:" + intExp.getLocalizedMessage());
                return;
            }

            // Handle run-time
            if (System.currentTimeMillis() >= runtime)
            {
                System.out.println("Consumer run-time expired...");
                break;
            }

            if (channelSession.channel().info(channelInfo, error) != TransportReturnCodes.SUCCESS)
            {
                System.out.println("Channel.info() failed");
                closeChannel();
                return;
            }
            System.out.printf("Channel Info:\n" +
                    "  Max Fragment Size: %d\n" +
                    "  Output Buffers: %d Max, %d Guaranteed\n" +
                    "  Input Buffers: %d\n" +
                    "  Send/Recv Buffer Sizes: %d/%d\n" +
                    "  Ping Timeout: %d\n" +
                    "  Connected component version: ",
                              channelInfo.maxFragmentSize(),
                              channelInfo.maxOutputBuffers(), channelInfo.guaranteedOutputBuffers(),
                              channelInfo.numInputBuffers(),
                              channelInfo.sysSendBufSize(), channelInfo.sysRecvBufSize(),
                              channelInfo.pingTimeout()
                    );
            int count = channelInfo.componentInfo().size();
            if (count == 0)
                System.out.printf("(No component info)");
            else
            {
                for (int i = 0; i < count; ++i)
                {
                    System.out.println(channelInfo.componentInfo().get(i).componentVersion());
                    if (i < count - 1)
                        System.out.printf(", ");
                }
            }

            loginHandler.applicationName("Consumer");
            loginHandler.role(Login.RoleTypes.CONS);

            // Send login request message
            if (loginHandler.sendRequest(channelSession, error) != CodecReturnCodes.SUCCESS)
            {
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }

            // Initialize ping handler
            pingHandler.initPingHandler(channelSession.channel().pingTimeout());

            // this is the message processing loop
            readAndProcessResp(pingHandler);
        }
    }

    // connection recovery loop
    private void connectRetry(InProgInfo inProg) throws InterruptedException
    {
        String hostName = CommandLine.value("h");
        String portNumber = CommandLine.value("p");
        String interfaceName = CommandLine.value("i");

        while (System.currentTimeMillis() < runtime && channelSession.shouldRecoverConnection())
        {
            System.out.println("Starting connection...");

            // get connect options from the channel session
            ConnectOptions copts = channelSession.getConnectOptions();

            // set the connection parameters on the connect options
            copts.unifiedNetworkInfo().address(hostName);
            copts.unifiedNetworkInfo().serviceName(portNumber);
            copts.unifiedNetworkInfo().interfaceName(interfaceName);

            channelSession.connect(inProg, error);

            // connection hand-shake loop
            waitUntilChannelActive(inProg);
            if (channelSession.shouldRecoverConnection())
            {
                /* sleep before trying to recover connection */
                Thread.sleep(CONNECTION_RETRY_TIME * 1000);
                continue;
            }
        }
    }

    /*
     * Wait for channel to become active. This finalizes the three-way handshake.
     */
    private void waitUntilChannelActive(InProgInfo inProg) throws InterruptedException
    {
        while (System.currentTimeMillis() < runtime && channelSession.channelState() != ChannelState.ACTIVE)
        {
            if (channelSession.initChannel(inProg, error) < TransportReturnCodes.SUCCESS)
            {
                System.out.println("Error initializing channel, will retry: " + error.text());
            }
            if (channelSession.channel() == null || channelSession.channelState() == ChannelState.ACTIVE)
                break;

            Thread.sleep(1000);
        }
    }

    /* read processing loop */
    private void readAndProcessResp(PingHandler pingHandler)
    {
        String fileName = null;
        int ret = 0;
        while (System.currentTimeMillis() < runtime)
        {
            // read until no more to read
            ret = channelSession.read(pingHandler, this, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println(error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }

            //  break out of message processing loop if connection should recover
            if (channelSession.shouldRecoverConnection())
                break;

            //Handle pings
            if (pingHandler.handlePings(channelSession.channel(), error) != CodecReturnCodes.SUCCESS)
            {
                closeChannel();
                System.out.println("Error handling pings: " + error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }
            
            // retrieve file(s) from generic provider
            if ((fileName = userInputThread.nextFilename()) != null)
            {
                if (!fileName.contains(",")) // single file
                {
                    requestFileFromProvider(fileName);
                }
                else // comma separated list of files
                {
                    String[] fileNameList = fileName.trim().split(",");
                    for (int i = 0; i < fileNameList.length; i++)
                    {
                        requestFileFromProvider(fileNameList[i].trim());
                    }
                }
            }
        }
    }
    
    /* Request a file from the generic provider. */
    private void requestFileFromProvider(String fileName)
    {
        int ret = 0;
        int fileNameLen = fileName.length();
        
        if (fileNameLen < MAX_FILE_NAME_LEN)
        {
            // get buffer for request
            TransportBuffer buffer = channelSession.channel().getBuffer(MAX_FILE_NAME_LEN + MAX_MSG_OVERHEAD, false, error);
            
            encIter.clear();
            encIter.setBufferAndRWFVersion(buffer, channelSession.channel().majorVersion(), channelSession.channel().minorVersion());
            fileRequestMsg.clear();
            fileRequestMsg.msgClass(MsgClasses.GENERIC);
            fileRequestMsg.domainType(GENERIC_DOMAIN);
            fileRequestMsg.streamId(GENERIC_STREAM_ID);
            fileRequestMsg.applyMessageComplete();
            // use MsgKey to store file name to request
            fileRequestMsg.applyHasMsgKey();
            fileRequestMsg.msgKey().applyHasName();
            fileRequestMsg.msgKey().name().data(fileName);
            fileRequestMsg.msgKey().applyHasServiceId();
            fileRequestMsg.msgKey().serviceId(srcDirHandler.serviceInfo().serviceId());
            
            // encode generic message
            if ((ret = fileRequestMsg.encode(encIter)) != CodecReturnCodes.SUCCESS)
            {
                System.out.println("GenericMsg encoding failure: " + ret);
                System.exit(-1);
            }
            
            // send message
            if (( ret = channelSession.write(buffer, error)) != TransportReturnCodes.SUCCESS)
            {
                System.out.println("GenericMsg write failure: " + ret);
                System.exit(-1);
            }
            
            requestCount++;
        }
        else
        {
            System.out.println("Invalid filename: must be less than or equal to 255 characters");
        }
    }

    /**
     * Call back method to process responses from channel. Processing responses
     * consists of performing a high level decode of the message and then
     * calling the applicable specific method for further processing.
     */
    @Override
    public void processResponse(ChannelSession chnl, TransportBuffer buffer)
    {
        // clear decode iterator
        dIter.clear();

        // set buffer and version info
        dIter.setBufferAndRWFVersion(buffer, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = responseMsg.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                    + chnl.channel().selectableChannel() + "  Size " + (buffer.data().limit() - buffer.data().position()));
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        processResponse(chnl, responseMsg, dIter);
    }

    /* Processes the response after top-level decoding. */
    private void processResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        switch (responseMsg.domainType())
        {
            case DomainTypes.LOGIN:
                processLoginResp(chnl, responseMsg, dIter);
                break;
            case DomainTypes.SOURCE:
                processSourceDirectoryResp(chnl, responseMsg);
                break;
            case GENERIC_DOMAIN:
                if (responseMsg.msgClass() == MsgClasses.GENERIC) 
                {
                    // process generic provider response
                    processGenericProviderResponse(responseMsg);
                    
                    // tell user thread we're ready for more input
                    if (--requestCount == 0)
                    {
                        userInputThread.readyForInput();
                    }
                }
                else if (responseMsg.msgClass() == MsgClasses.REFRESH)
                {
                    userInputThread.readyForInput();
                }
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }
    
    /* Processes a login response. */
    private void processLoginResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        // handle standard login responses
        int ret = loginHandler.processResponse(responseMsg, dIter, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        // Handle login states
        ConsumerLoginState loginState = loginHandler.loginState();
        if (loginState == ConsumerLoginState.OK_SOLICITED)
        {
            ret = srcDirHandler.sendRequest(chnl, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error sending directory request: " + error.text());
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
        }
        else if (loginState == ConsumerLoginState.CLOSED)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
        else if (loginState == ConsumerLoginState.CLOSED_RECOVERABLE)
        {
            ret = channelSession.recoverConnection(error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error recovering connection: " + error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }
        }
        else if (loginState == ConsumerLoginState.SUSPECT)
        {
            if (!loginHandler.refreshInfo().checkHasAttrib() || // default behavior when singleopen attrib not set
            loginHandler.refreshInfo().attrib().singleOpen() == 0)
            {
                // reopen directory stream, which in turn reopens other streams
                // (dict, item streams)
                ret = srcDirHandler.closeStream(channelSession, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error closing directory stream: " + error.text());
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }

                ret = srcDirHandler.sendRequest(chnl, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error sending directory request: " + error.text());
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
            // login suspect from single-open provider: provider handles
            // recovery. consumer does nothing in this case.
        }
    }
    
    private void processSourceDirectoryResp(ChannelSession chnl, Msg responseMsg)
    {
        int ret = srcDirHandler.processResponse(chnl.channel(), responseMsg, dIter, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (srcDirHandler.isRequestedServiceUp())
        {
            // make request to open stream so we can send/receive generic messages
            
            // get buffer for request
            TransportBuffer buffer = channelSession.channel().getBuffer(MAX_MSG_OVERHEAD, false, error);
            
            encIter.clear();
            encIter.setBufferAndRWFVersion(buffer, channelSession.channel().majorVersion(), channelSession.channel().minorVersion());
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.domainType(GENERIC_DOMAIN);
            requestMsg.streamId(GENERIC_STREAM_ID);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data(GENERIC_ITEM);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(srcDirHandler.serviceInfo().serviceId());
            
            // encode generic message
            if ((ret = requestMsg.encode(encIter)) != CodecReturnCodes.SUCCESS)
            {
                System.out.println("RequestMsg encoding failure: " + ret);
                System.exit(-1);
            }
            
            // send message
            if (( ret = channelSession.write(buffer, error)) != TransportReturnCodes.SUCCESS)
            {
                System.out.println("RequestMsg write failure: " + ret);
                System.exit(-1);
            }
        }
        else
        {
            // service not up or
            // previously up service went down
            System.out.println("Requested service '" + CommandLine.value("s") + "' not up. Waiting for service to be up...");
        }
    }
    
    /* Processes a generic provider response. */
    private void processGenericProviderResponse(Msg responseMsg)
    {
        String fileName = responseMsg.msgKey().name().toString();
        Buffer buffer = responseMsg.encodedDataBody();
        
        if (buffer.length() > 0)
        {
            // get status byte first
            int status = buffer.data().get(buffer.position());
            
            if (status == GenericResponseStatusFlags.SUCCESS)
            {
                // get file from response and store in new file
                try
                {
                    String newFilename = newFilename(fileName);
                    OutputStream newFile = new BufferedOutputStream(new FileOutputStream(newFilename));
                    for (int i = 0; i < buffer.length() - 1; i++)
                    {
                        newFile.write(buffer.data().get(buffer.position() + 1 + i));                
                    }
                    newFile.flush();
                    newFile.close();
                    System.out.println("\nReceived \"" + fileName + "\" from provider and saved as \"" + newFilename + "\"");
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }
            }
            else // error response
            {
                // get error text from response
                byte[] errorTextArray  = new byte[buffer.length() - 1];
                for (int i = 0; i < buffer.length() - 1; i++)
                {
                    errorTextArray[i] = buffer.data().get(buffer.position() + 1 + i);                
                }
                String errorText = new String(errorTextArray);
                System.out.println("\nError response for \"" + fileName + "\" request: " + errorText);
            }
        }
        else
        {
            System.out.println("\nInvalid response from generic provider");
        }        
    }
    
    /* Creates a new file name from the requested file name. */
    private String newFilename(String requestedFilename)
    {
        String newFilename = null;
        
        if (!requestedFilename.contains("."))
        {
            newFilename = requestedFilename + "_NEW";
        }
        else
        {
            int dotIndex = requestedFilename.indexOf('.');
            String beforeDot = requestedFilename.substring(0, dotIndex);
            String afterDot = requestedFilename.substring(dotIndex);
            newFilename = beforeDot + "_NEW" + afterDot;
        }
        
        return newFilename;
    }

    /* Uninitializes the generic consumer application. */
    void uninitialize()
    {
        System.out.println(" Generic Consumer unitializing and exiting...");
        if (channelSession.channel() == null)
        {
            channelSession.uninit(error);
            System.exit(TransportReturnCodes.SUCCESS);
        }

        // close directory stream
        srcDirHandler.closeStream(channelSession, error);

        // close login stream
        loginHandler.closeStream(channelSession, error);

        // flush before exiting
        flushChannel();

        closeChannel();
    }

    /* Flushes the channel. */
    private void flushChannel()
    {
        int retval = 1;

        while (retval > TransportReturnCodes.SUCCESS)
        {
            retval = channelSession.flush(error);
        }

        if (retval < TransportReturnCodes.SUCCESS)
        {
            System.out.println("Flush() failed with return code " + retval + "- <" + error
                    .text() + ">");
        }
    }

    /* Closes the channel. */
    private void closeChannel()
    {
        channelSession.uninit(error);
    }

    public static void main(String[] args) throws Exception
    {
        GenericCons genericcons = new GenericCons();
        genericcons.init(args);
        genericcons.run();
        genericcons.uninitialize();
    }
}
