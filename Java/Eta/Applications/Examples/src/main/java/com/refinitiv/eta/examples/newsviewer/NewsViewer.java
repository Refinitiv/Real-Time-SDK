/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.newsviewer;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.common.DirectoryHandler;
import com.refinitiv.eta.examples.common.DictionaryHandler;
import com.refinitiv.eta.examples.common.LoginHandler;
import com.refinitiv.eta.examples.common.ResponseCallback;
import com.refinitiv.eta.examples.common.StreamIdWatchList;
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
 * This is a main class to run ETA NewsViewer application. The purpose of this
 * application is to consume content between an OMM news consumer and OMM news
 * provider. It is a single-threaded client application.
 * </p>
 * <em>Summary</em>
 * <p>
 * This class is responsible for the following:
 * <ul>
 * <li>Initialize and set command line options.
 * <li>Create a {@link ChannelSession ETA Channel Session}.
 * <li>Create Handler instances to handle Login, Dictionary, and news items.
 * <li>If the dictionary is found in the directory of execution, then it is
 * loaded directly from the file. However, the default configuration for this
 * application is to request the dictionary from the provider.
 * <li>Connect to the provider, login, request source directory, request
 * dictionary if not loaded from file, send item requests for news items , and
 * process responses (refresh, status, update, close).
 * <li>Cleanup.
 * </ul>
 * <p>
 * This class is also a call back for all events from provider. It dispatches
 * events to domain specific handlers.
 * <p>
 * This application is intended as a basic usage example. Some of the design
 * choices were made to favor simplicity and readability over performance. It is
 * not intended to be used for measuring performance. This application uses
 * Value Add and shows how using Value Add simplifies the writing of ETA
 * applications. Because Value Add is a layer on top of ETA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the ETA interfaces.
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * The RDMFieldDictionary and enumtype.def files could be located in the
 * directory of execution or this application will request dictionary from
 * provider.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runNewsViewer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runNewsViewer -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-h Server host name. Default is <i>localhost</i>.
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-uname Login user name. Default is system user name.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-item News item name. Default is <i>N2_UBMS</i>. The user can specify
 * multiple -item instances, where each occurrence is associated with a single
 * item. For example, specifying -item A -item B will provide content for two
 * news items
 * <li>-runtime run time. Default is 999999 seconds to keep it running non-stop.
 * Controls the time the application will run before exiting, in seconds.
 * <li>-font font used. Default is Ariel Unicode MS. Decides the font used for
 * the application's output.
 * <li>-fontSize font size used. Default is 12. Decides the font size used for
 * the application's output;
 * 
 * </ul>
 * 
 * @see DictionaryHandler
 * @see Headline
 * @see NewsHandler
 * @see NewsHeadlineListener
 * @see NewsHeadlineViewer
 * @see NewsStoryViewer
 * @see NewsViewerFrame
 * @see StreamIdWatchList
 */
public class NewsViewer implements ResponseCallback
{
    private static final int CONSUMER_CONNECTION_RETRY_TIME = 15; // seconds

    private ChannelSession channelSession;
    private ChannelInfo channelInfo;
    private LoginHandler loginHandler;
    private DirectoryHandler srcDirHandler;
    private DictionaryHandler dictionaryHandler;
    private NewsHandler marketPriceHandler;

    private StreamIdWatchList itemWatchList;

    // indicates if requested service is up
    private boolean requestsSent;

    private long runtime;

    private String font;

    private int fontSize;

    // default server host name
    private static final String defaultSrvrHostname = "localhost";

    // default server port number
    private static final String defaultSrvrPortNo = "14002";

    // default service name
    private static final String defaultServiceName = "DIRECT_FEED";

    // default item name
    private static final String defaultItemName = "N2_UBMS";

    // newsviewer run-time in seconds
    private static final int defaultRuntime = 999999;

    // default font
    private static final String defaultFont = "Arial Unicode MS";

    // default font size
    private static final int defaultFontSize = 12;

    private Error error; // error information

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();

    /**
     * Instantiates a new news viewer.
     */
    public NewsViewer()
    {
        channelInfo = TransportFactory.createChannelInfo();
        itemWatchList = new StreamIdWatchList();
        loginHandler = new LoginHandler();
        srcDirHandler = new DirectoryHandler();
        dictionaryHandler = new DictionaryHandler();
        channelSession = new ChannelSession();
        error = TransportFactory.createError();
    }

    /**
     * Main loop that handles channel connection, login requests, reading and
     * processing responses from channel.
     */
    public void run()
    {
        PingHandler pingHandler = new PingHandler();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        marketPriceHandler = new NewsHandler(fontSize, font, itemWatchList);

        while (true)
        {
            try
            {
                connectRetry(inProg);
            }
            catch (InterruptedException intExp)
            {
                System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:"
                        + intExp.getLocalizedMessage());
                return;
            }

            // Handle run-time
            if (System.currentTimeMillis() >= runtime)
            {
                System.out.println("NewsViewer run-time expired...");
                break;
            }

            if (channelSession.channel().info(channelInfo, error) != TransportReturnCodes.SUCCESS)
            {
                System.out.println("Channel.info() failed");
                closeChannel();
                return;
            }
            System.out.printf("Channel Info:\n" + "  Max Fragment Size: %d\n"
                                      + "  Output Buffers: %d Max, %d Guaranteed\n"
                                      + "  Input Buffers: %d\n"
                                      + "  Send/Recv Buffer Sizes: %d/%d\n"
                                      + "  Ping Timeout: %d\n" + "  Connected component version: ",
                              channelInfo.maxFragmentSize(),
                              channelInfo.maxOutputBuffers(),
                              channelInfo.guaranteedOutputBuffers(), channelInfo.numInputBuffers(),
                              channelInfo.sysSendBufSize(), channelInfo.sysRecvBufSize(),
                              channelInfo.pingTimeout());
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

            loginHandler.applicationName("NewsViewer");
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

            requestsSent = false;

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
                Thread.sleep(CONSUMER_CONNECTION_RETRY_TIME * 1000);
                continue;
            }
        }
    }

    /*
     * Wait for channel to become active. This finalizes the three-way handshake.
     */
    private void waitUntilChannelActive(InProgInfo inProg) throws InterruptedException
    {
        while (System.currentTimeMillis() < runtime
                && channelSession.channelState() != ChannelState.ACTIVE)
        {
            if (channelSession.initChannel(inProg, error) < TransportReturnCodes.SUCCESS)
            {
                System.out.println("Error initializing channel, will retry: " + error.text());
            }
            if (channelSession.channel() == null
                    || channelSession.channelState() == ChannelState.ACTIVE)
                break;

            Thread.sleep(1000);
        }
    }

    /* read processing loop */
    private void readAndProcessResp(PingHandler pingHandler)
    {
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

            // break out of message processing loop if connection should recover
            if (channelSession.shouldRecoverConnection())
                break;

            // Handle pings
            if (pingHandler.handlePings(channelSession.channel(), error) != CodecReturnCodes.SUCCESS)
            {
                closeChannel();
                System.out.println("Error handling pings: " + error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }

        }
    }

    /**
     * Initializes NewsViewer application.
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments. Initializing all domain
     * handlers. Loading dictionaries from file.
     *
     * @param args the args
     */
    public void init(String[] args)
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
        if (args.length == 0 || (!isMarketPriceArgSpecified()))
        {
            CommandLine.parseArgs(new String[] { "-item", defaultItemName });
        }

        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("NewsViewer initializing...");

        if (CommandLine.hasArg("font"))
            font = CommandLine.value("font");
        else
            font = defaultFont;

        if (CommandLine.hasArg("fontSize"))
            try
            {
                fontSize = CommandLine.intValue("fontSize");
            }
            catch (NumberFormatException ile)
            {
                System.err.println("Invalid argument, number expected.\t");
                System.err.println(ile.getMessage());
                System.exit(-1);
            }
        else
            fontSize = defaultFontSize;

        try
        {
            runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;
        }
        catch (NumberFormatException ile)
        {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.exit(-1);
        }

        loginHandler.userName(CommandLine.value("uname"));

        // set service name in directory handler
        srcDirHandler.serviceName(CommandLine.value("s"));

        if (channelSession.initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // load dictionary
        dictionaryHandler.loadDictionary();

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(dictionaryHandler.dictionary());
        }
    }

    private boolean isMarketPriceArgSpecified()
    {
        return (CommandLine.hasArg("item"));
    }

    /**
     * Call back method to process responses from channel. Processing responses
     * consists of performing a high level decode of the message and then
     * calling the applicable specific method for further processing.
     * chnl - The channel of the response
     * buffer - The message buffer containing the response.
     *
     * @param chnl the chnl
     * @param buffer the buffer
     */
    public void processResponse(ChannelSession chnl, TransportBuffer buffer)
    {
        // clear decode iterator
        dIter.clear();

        // set buffer and version info
        dIter.setBufferAndRWFVersion(buffer, chnl.channel().majorVersion(), chnl.channel()
                .minorVersion());

        int ret = responseMsg.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                    + chnl.channel().selectableChannel() + "  Size "
                    + (buffer.data().limit() - buffer.data().position()));
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        processResponse(chnl, responseMsg, dIter);
    }

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
            case DomainTypes.DICTIONARY:
                processDictionaryResp(chnl, responseMsg, dIter);
                break;
            case DomainTypes.MARKET_PRICE:
                processMarketPriceResp(chnl, responseMsg, dIter);

                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }

    private void processMarketPriceResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        if (marketPriceHandler.processResponse(chnl, responseMsg, dIter,
                                               dictionaryHandler.dictionary(), error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processDictionaryResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        if (dictionaryHandler.processResponse(chnl.channel(), responseMsg, dIter, error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (dictionaryHandler.isFieldDictionaryLoaded()
                && dictionaryHandler.isEnumTypeDictionaryLoaded()
                && responseMsg.msgClass() == MsgClasses.REFRESH)
        {
            System.out.println("Dictionary ready, requesting item(s)...");

            itemWatchList.clear();
            sendItemRequests(chnl);

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
            sendRequests(chnl);
        }
        else
        {
            // service not up or previously up service went down
            requestsSent = false;

            System.out.println("Requested service '" + CommandLine.value("s")
                    + "' not up. Waiting for service to be up...");
        }
    }

    private void sendRequests(ChannelSession chnl)
    {
        if (requestsSent)
            return;

        // first load dictionaries. send item requests and post
        // messages only after dictionaries are loaded.
        if (!isDictionariesLoaded())
        {
            sendDictionaryRequests(chnl);
            return;
        }

        itemWatchList.clear();
        sendItemRequests(chnl);

        requestsSent = true;
    }

    private void sendDictionaryRequests(ChannelSession chnl)
    {
        dictionaryHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
        if (dictionaryHandler.sendRequests(chnl, error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void sendItemRequests(ChannelSession chnl)
    {
        if (marketPriceHandler.sendItemRequests(chnl, CommandLine.values("item"), false,
                                                loginHandler.refreshInfo(),
                                                srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private boolean isDictionariesLoaded()
    {
        return dictionaryHandler.isFieldDictionaryLoaded()
                && dictionaryHandler.isEnumTypeDictionaryLoaded();
    }

    private void processLoginResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
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
                // login suspect from no single-open provider: 1) close source
                // directory stream and item streams. 2) reopen streams
                closeDictAndItemStreams();

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

    private void closeDictAndItemStreams()
    {

        // close item streams if opened
        marketPriceHandler.closeStreams(channelSession, error);

        // close dictionary streams if opened
        dictionaryHandler.closeStreams(channelSession, error);
    }

    /**
     * Closes all streams for the consumer.
     */
    public void uninitialize()
    {
        System.out.println("NewsViewer unitializing and exiting...");
        if (channelSession.channel() == null)
        {
            channelSession.uninit(error);
            System.exit(TransportReturnCodes.SUCCESS);
        }

        // close all streams
        closeDictAndItemStreams();

        srcDirHandler.closeStream(channelSession, error);

        // close login stream
        loginHandler.closeStream(channelSession, error);

        // flush before exiting
        flushChannel();

        closeChannel();
    }

    private void flushChannel()
    {
        int retval = 1;

        while (retval > TransportReturnCodes.SUCCESS)
        {
            retval = channelSession.flush(error);
        }

        if (retval < TransportReturnCodes.SUCCESS)
        {
            System.out.println("Flush() failed with return code " + retval + "- <" + error.text()
                    + ">");
        }
    }

    private void closeChannel()
    {
        channelSession.uninit(error);
    }

    private static void addCommandLineArgs()
    {
        CommandLine.programName("NewsViewer");
        CommandLine.addOption("item", "For each occurrence, requests news item.");
        CommandLine.addOption("h", defaultSrvrHostname, "Server host name");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number");
        CommandLine.addOption("i", (String)null, "Interface name");
        CommandLine.addOption("s", defaultServiceName, "Service name");
        CommandLine.addOption("uname", "Login user name. Default is system user name.");

        CommandLine.addOption("font", defaultFont, "Font used for application.");
        CommandLine.addOption("fontSize", defaultFontSize, "Font size used in the application.");
        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
    }

    /**
     * The main method.
     *
     * @param args the arguments
     * @throws Exception the exception
     */
    public static void main(String[] args) throws Exception
    {
        NewsViewer newsviewer = new NewsViewer();
        newsviewer.init(args);
        newsviewer.run();
        newsviewer.uninitialize();
    }
}
