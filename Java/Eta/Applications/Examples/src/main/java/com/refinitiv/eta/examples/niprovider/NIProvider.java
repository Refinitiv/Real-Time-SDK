/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.niprovider;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.common.ResponseCallback;
import com.refinitiv.eta.examples.common.LoginHandler;
import com.refinitiv.eta.examples.niprovider.DirectoryHandler;
import com.refinitiv.eta.examples.common.NIProviderDictionaryHandler;
import com.refinitiv.eta.examples.niprovider.MarketByOrderHandler;
import com.refinitiv.eta.examples.niprovider.MarketPriceHandler;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.shared.ConsumerLoginState;
import com.refinitiv.eta.shared.PingHandler;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;

/**
 * This is a main class to run ETA NIProvider application. The purpose of this
 * application is to non-interactively provide Level I Market Price and Level 2
 * Market By Order data to an Advanced Data Hub (ADH). If supported it requests
 * dictionary from an adh.
 * It is a single-threaded client application.
 * <p>
 * <em>Summary</em>
 * <p>
 * This class is responsible for the following:
 * <ul>
 * <li>Initialize and set command line options.
 * <li>Create a {@link ChannelSession ETA Channel Session}.
 * <li>Create Handler instances to handle Login, Directory, MarketPrice and
 * MarketByOrder responses.
 * <li>Load Dictionary from file, if file available in the path.
 * <li>Connect to the ADH provider, send login request, send source directory
 * refresh, optionally send dictionary request, handle dictionary refreshes,
 * send item refreshes, then send item updates.
 * <li>Cleanup.
 * </ul>
 * <p>
 * This class is also a call back for all events from Consumer/ADH. It
 * dispatches events to domain specific handlers.
 * <p>
 * Reliable multicast can be used to communicate between this application and any
 * ADH on the network. The non-interactive provider can then send one message to
 * all ADH's on the network instead of having to fan-out messages to each ADH
 * TCP/IP connection.
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
 * The RDMFieldDictionary and enumtype.def files must be located in the
 * directory of execution. If not available and adh supports dictionary requests,
 * the dictionary is down loaded from adh.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runNIProvider -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runNIProvider -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-h Infrastructure host name for TCP socket. Default is <i>localhost</i>.
 * <li>-p Infrastructure port number for TCP socket. Default is <i>14003</i>.
 * <li>-sa Send address for reliable multicast. Default is.
 * <li>-sp Send port for reliable multicast. Default is.
 * <li>-ra Receive address for reliable multicast. Default is.
 * <li>-rp Receive port for reliable multicast. Default is.
 * <li>-u Unicast port for reliable multicast. Default is.
 * <li>-i Interface name for reliable multicast. Default is.
 * <li>-uname Login user name. Default is system user name.
 * <li>-s Service name. Default is <i>NI_PUB</i>.
 * <li>-id Service id. Default is <i>1</i>.
 * <li>-mp Market Price domain item name. Default is <i>TRI</i>. The user can
 * specify multiple -mp instances, where each occurrence is associated with a
 * single item. For example, specifying -mp TRI -mp GOOG will provide content
 * for two MarketPrice items.
 * <li>-mbo Market By Order domain item name. No default. The user can specify
 * multiple -mbo instances, where each occurrence is associated with a single
 * item.
 * <li>-x. Provides XML tracing of messages.
 * <li>-runtime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
 * <li>-ax Specifies the Authentication Extended information.
 * <li>-aid Specifies the Application ID.
 * </ul>
 * 
 * @see LoginHandler
 * @see DirectoryHandler
 * @see NIProviderDictionaryHandler
 * @see MarketPriceHandler
 * @see MarketByOrderHandler
 * @see StreamIdWatchList
 */
public class NIProvider implements ResponseCallback
{
    private static final int NIPROVIDER_CONNECTION_RETRY_TIME = 15; // seconds
    private static final int SEND_INTERVAL = 1000; // send content every 1000 milliseconds
    private Error error = TransportFactory.createError();
    private ChannelSession channelSession;
    private boolean isUnifiedNetworkConnection;
    private ChannelInfo channelInfo;
    private LoginHandler loginHandler;
    private DirectoryHandler srcDirHandler;
    private NIProviderDictionaryHandler dictionaryHandler;
    private MarketPriceHandler marketPriceHandler;
    private MarketByOrderHandler marketByOrderHandler;
    private long runtime;
    private StreamIdWatchList itemWatchList;
    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private boolean refreshesSent;
    
    private Msg incomingMsg = CodecFactory.createMsg();
    private Msg outgoingMsg = CodecFactory.createMsg();

    //default server host
    private static final String defaultSrvrHostname = "localhost";
    //default server port number
    private static final String defaultSrvrPortNo = "14003";
    //default service name
    private static final String defaultServiceName = "NI_PUB";
    //default serviceId 
    private static final int defaultServiceId = 1;
    //default item name
    private static final String defaultItemName = "TRI";

    //NIProvider run-time in seconds
    private static final int defaultRuntime = 600;
    
   public static int TRANSPORT_BUFFER_SIZE_STATUS_MSG = ChannelSession.MAX_MSG_SIZE;

    /**
     * Instantiates a new NI provider.
     */
    public NIProvider()
    {
    	channelInfo = TransportFactory.createChannelInfo();
        itemWatchList = new StreamIdWatchList();
        loginHandler = new LoginHandler();
        srcDirHandler = new DirectoryHandler();
        dictionaryHandler = new NIProviderDictionaryHandler();
        marketPriceHandler = new MarketPriceHandler(itemWatchList, dictionaryHandler.dictionary());
        marketByOrderHandler = new MarketByOrderHandler(itemWatchList,
                dictionaryHandler.dictionary());
        channelSession = new ChannelSession();
        channelSession.selectTime(1000); // 1000ms
        // unified network connection is default
        isUnifiedNetworkConnection = true;
    }

    /**
     * Main loop that handles channel connection, login requests, reading and
     * processing responses from channel, and providing content.
     * 
     * @param args command line arguments
     * @see #addCommandLineArgs()
     */
    public void run(String[] args)
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
                System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:"
                        + intExp.getLocalizedMessage());
                return;
            }

            //Handle run-time
            if (System.currentTimeMillis() >= runtime)
            {
                System.out.println("NIProvider run-time expired...");
                break;
            }
            
        	if (channelSession.channel().info(channelInfo, error) != TransportReturnCodes.SUCCESS)
        	{
    			System.out.println("Channel.info() failed");
            	closeChannel();
            	System.exit(TransportReturnCodes.FAILURE);
        	} 
        	System.out.printf( "Channel Info:\n" +
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
            if(count == 0)
                System.out.printf("(No component info)");
            else
            {
                for(int i = 0; i < count; ++i)
                {
                    System.out.println(channelInfo.componentInfo().get(i).componentVersion());
                   if (i < count - 1)
                       System.out.printf(", ");
                }
            }

            loginHandler.applicationName("NIProvider");
            loginHandler.role(Login.RoleTypes.PROV);

            /* Send login request message */
            channelSession.isLoginReissue = false;
            if (loginHandler.sendRequest(channelSession, error) != CodecReturnCodes.SUCCESS)
            {
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }

            /* Initialize ping handler */
            pingHandler.initPingHandler(channelSession.channel().pingTimeout());

            /* this is the content handler processing loop */
            contentHandler(pingHandler);
        }
    }

    /* connection recovery loop */
    private void connectRetry(InProgInfo inProg) throws InterruptedException
    {
        while (System.currentTimeMillis() < runtime && channelSession.shouldRecoverConnection())
        {
            System.out.println("Starting connection...");
            
            // get connect options from the channel
            ConnectOptions copts = channelSession.getConnectOptions();

            // set the connection parameters on the connect options 
            if (isUnifiedNetworkConnection) // unified connection is used for TCP socket
            {
                copts.unifiedNetworkInfo().address(CommandLine.value("h"));
                copts.unifiedNetworkInfo().serviceName(CommandLine.value("p"));            	
            }
            else // segmented connection is used for reliable multicast
            {
            	channelSession.setConnectionType(ConnectionTypes.RELIABLE_MCAST);
            	copts.segmentedNetworkInfo().sendAddress(CommandLine.value("sa"));
            	copts.segmentedNetworkInfo().sendServiceName(CommandLine.value("sp"));
            	copts.segmentedNetworkInfo().recvAddress(CommandLine.value("ra"));
            	copts.segmentedNetworkInfo().recvServiceName(CommandLine.value("rp"));
            	copts.segmentedNetworkInfo().unicastServiceName(CommandLine.value("u"));
            	copts.segmentedNetworkInfo().interfaceName(CommandLine.value("i"));
            }

            channelSession.connect(inProg, error);
            
            // connection hand-shake loop
            waitUntilChannelActive(inProg);
            if (channelSession.shouldRecoverConnection())
            {
                /* sleep before trying to recover connection */
                Thread.sleep(NIPROVIDER_CONNECTION_RETRY_TIME * 1000);
                continue;
            }
        }
    }

    /*
     * Wait for channel to become active. This finalizes the three-way
     * handshake.
     */
    private void waitUntilChannelActive(InProgInfo inProg) throws InterruptedException
    {
        while (System.currentTimeMillis() < runtime
                && channelSession.channelState() != ChannelState.ACTIVE)
        {

            if (channelSession.initChannel(inProg, error) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error initializing channel: " + error.text());
            }

            if (channelSession.channel() == null
                    || channelSession.channelState() == ChannelState.ACTIVE)
                break;

            Thread.sleep(1000);
        }
    }

    //read processing loop
    private void contentHandler(PingHandler pingHandler)
    {
        int ret = 0;
        long currentTime = System.currentTimeMillis();
        long nextSendTime = currentTime + SEND_INTERVAL;
        while (currentTime < runtime)
        {
            //read until no more to read
            ret = channelSession.read(pingHandler, this, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println(error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }

            /*
             * break out of message processing loop if connection should recover
             */
            if (channelSession.shouldRecoverConnection())
            {
            	refreshesSent = false;
                break;
            }

            //provide content (item updates)
            if (srcDirHandler.isServiceUp())
            {
            	if (currentTime >= nextSendTime)
            	{
            		if (refreshesSent) // send updates since refreshes already sent
            		{
	                if (provideContent(pingHandler) != CodecReturnCodes.SUCCESS)
	                {
	                    closeChannel();
	                    System.out.println("Error sending updates: " + error.text());
	                    System.exit(TransportReturnCodes.FAILURE);
	                }
            		}
            		else // send refreshes first
            		{
                        sendItemRefreshes(channelSession, srcDirHandler.serviceInfo(), error);
                        refreshesSent = true;
            		}
	                nextSendTime += SEND_INTERVAL;
            	}
            }

            //handle pings
            if (pingHandler.handlePings(channelSession.channel(), error) != CodecReturnCodes.SUCCESS)
            {
                closeChannel();
                System.out.println("Error handling pings: " + error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }
            
	        // send login reissue if login reissue time has passed
    		if (channelSession.canSendLoginReissue &&
    			System.currentTimeMillis() >= channelSession.loginReissueTime)
    		{
	        	channelSession.isLoginReissue = true;
				if (loginHandler.sendRequest(channelSession, error) !=  CodecReturnCodes.SUCCESS)
				{
					System.out.println("Login reissue failed. Error: " + error.text());
				}
				else
				{
					System.out.println("Login reissue sent");
				}
				channelSession.canSendLoginReissue = false;
    		}
    		
            currentTime = System.currentTimeMillis();
        }
    }

    /* send item updates for MARKET_PRICE and MARKET_BY_ORDER domains */
    private int provideContent(PingHandler pingHandler)
    {
        int ret;
        ret = marketPriceHandler.sendItemUpdates(channelSession, error);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        ret = marketByOrderHandler.sendItemUpdates(channelSession, error);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        pingHandler.sentMsg();
        return ret;
    }

    /**
     * Initializes NIProvider application. It is responsible for: Initializing
     * command line options used by the application. Parsing command line
     * arguments. Initializing all domain handlers. Loading dictionaries from
     * file. Enabling XML tracing, if specified.
     *
     * @param args the args
     */
    public void init(String[] args)
    {
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
        if (args.length == 0 || !isMarketPriceArgSpecified())
        {
            CommandLine.parseArgs(new String[] { "-mp", defaultItemName });
        }
        
        //display product version information
        System.out.println(Codec.queryVersion().toString());
        
        // determine if command line requests unified or segmented connection
        // segmented connection is used for reliable multicast
        if (CommandLine.value("sa").length() > 0 || CommandLine.value("ra").length() > 0)
        {
        	isUnifiedNetworkConnection = false;
        }

        //display command line information
        if (isUnifiedNetworkConnection) // unified connection
        {
	        System.out.println("NIProvider initializing using TCP Socket connection type...\ninfraHostname=" + CommandLine.value("h")
	                           + " infraPortNo=" + CommandLine.value("p") + " serviceName="
	                           + CommandLine.value("s") + " serviceId=" + CommandLine.value("id"));
        }
        else // segmented connection
        {
	        System.out.println("NIProvider initializing using Reliable Multicast connection type...\nsendAddress=" + CommandLine.value("sa") +
                    " sendPort=" + CommandLine.value("sp") + " recvAddress=" + CommandLine.value("ra") + " recvPort=" + CommandLine.value("rp") +
                    " unicastPort=" + CommandLine.value("u") + " interface=" + CommandLine.value("i") +
                    " serviceName=" + CommandLine.value("s") + " serviceId=" + CommandLine.value("id"));        	
        }
     
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
        loginHandler.authenticationToken(CommandLine.value("at"));
        loginHandler.authenticationExtended(CommandLine.value("ax"));
        loginHandler.applicationId(CommandLine.value("aid"));

        /* set service name in directory handler */
        srcDirHandler.serviceName(CommandLine.value("s"));

        /* set service Id in directory handler */
        if (CommandLine.hasArg("id"))
        {
        	try
        	{
        		srcDirHandler.serviceId(CommandLine.intValue("id"));
        	}
            catch (NumberFormatException ile)
            {
            	System.err.println("Invalid argument, number expected.\t");
            	System.err.println(ile.getMessage());
            	System.exit(-1);
            }
        }
        else
        {
            srcDirHandler.serviceId(defaultServiceId);
        }

        if (channelSession.initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(dictionaryHandler.dictionary());
        }

    }

    private boolean isMarketPriceArgSpecified()
    {
        return (CommandLine.hasArg("mp") || CommandLine.hasArg("mbo"));
    }

    /**
     * Processes a response from a channel. This consists of performing a high
     * level decode of the message and then calling the applicable specific
     * method for further processing.
     * 
     * @param chnl The channel of the response
     * @param buffer The message buffer containing the response
     */
    public void processResponse(ChannelSession chnl, TransportBuffer buffer)
    {
        //clear decode iterator
        dIter.clear();

        //set buffer and version info
        dIter.setBufferAndRWFVersion(buffer, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = incomingMsg.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                        + chnl.channel().selectableChannel() + "  Size "
                        + (buffer.data().limit() - buffer.data().position()));
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        processReceivedMessage(chnl, incomingMsg, dIter);
    }

    private int sendNotSupportedStatus(ChannelSession chnl, Msg receivedMsg)
    {
        //get a buffer for the item close
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_STATUS_MSG, false, error);
        if (msgBuf == null)
        {
            System.out.println("Channel.getBuffer(): Failed " +  error.text());
            return CodecReturnCodes.FAILURE;
        }
        
        //encode not supported status
        int ret = encodeNotSupportedStatus(chnl, receivedMsg, msgBuf);
        if(ret != CodecReturnCodes.SUCCESS)
        {
            chnl.channel().releaseBuffer(msgBuf, error);
            return ret;
        }
        
        System.out.println("\nRejecting Item Request with streamId=" + receivedMsg.streamId() +  " Reason: Domain " + DomainTypes.toString(receivedMsg.domainType()) +  " Not Supported");

        //send not supported status
        ret = chnl.write(msgBuf, error);
        if(ret != TransportReturnCodes.SUCCESS)
        {
            System.out.println("Channel.write(): Failed " +  error.text());
            return CodecReturnCodes.FAILURE;
        }
        
        return CodecReturnCodes.SUCCESS;
    }
    
    private int encodeNotSupportedStatus(ChannelSession chnl, Msg receivedMsg, TransportBuffer msgBuf)
    {
        //clear encode iterator
        encIter.clear();
        
        //set-up message
        outgoingMsg.clear();
        outgoingMsg.msgClass(MsgClasses.STATUS);
        outgoingMsg.streamId(receivedMsg.streamId());
        outgoingMsg.domainType(receivedMsg.domainType());
        outgoingMsg.containerType(DataTypes.NO_DATA);
        outgoingMsg.flags(StatusMsgFlags.HAS_STATE);
        
        StatusMsg statusMsg = (StatusMsg)outgoingMsg;
        statusMsg.state().streamState(StreamStates.CLOSED);
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().code(StateCodes.USAGE_ERROR);
        statusMsg.state().text().data("Request rejected for stream id "  + receivedMsg.streamId() + " - domain type '" +  DomainTypes.toString(receivedMsg.domainType()) + "' is not supported");
        
        //encode message
        encIter.clear();
        int ret = encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeIter.setBufferAndRWFVersion(): Failed <"
                        + CodecReturnCodes.toString(ret) + ">");
        }
        
        ret  = statusMsg.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("StatusMsg.encode(): Failed <"
                        + CodecReturnCodes.toString(ret) + ">");
        }
        
        return ret;
    }
    
    private void processReceivedMessage(ChannelSession channelSession,Msg receivedMsg, DecodeIterator dIter)
    {
        switch (receivedMsg.domainType())
        {
            case DomainTypes.LOGIN:
            {
                processLoginResp(channelSession, receivedMsg, dIter);
                break;
            }   
            
            case DomainTypes.DICTIONARY:
            {
            	dictionaryHandler.processResponse(receivedMsg, dIter, error);
                break;
            }

            default:
            {
                if(sendNotSupportedStatus(channelSession, receivedMsg) != CodecReturnCodes.SUCCESS)
                {
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }
                break;
            }
        }
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

        //Handle login states
        ConsumerLoginState loginState = loginHandler.loginState();
        if (loginState == ConsumerLoginState.OK_SOLICITED)
        {
        	if (!chnl.isLoginReissue)
        	{
            ret = srcDirHandler.sendRefresh(chnl, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error sending directory request: " + error.text());
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
            
            setupDictionary( chnl );
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
            if (!loginHandler.refreshInfo().checkHasAttrib() || //default behavior when singleopen attrib not set
                    loginHandler.refreshInfo().attrib().singleOpen() == 0)
            {
                /*
                 * login suspect from no single-open provider: 1) close source
                 * directory stream and item streams. 2) reopen streams
                 */
                closeItemStreams();

                // reopen directory stream, which in turn reopens other streams
                // (item streams)
                ret = srcDirHandler.closeStream(channelSession, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error closing directory stream: " + error.text());
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }

            	if (!chnl.isLoginReissue)
            	{
                ret = srcDirHandler.sendRefresh(chnl, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error sending directory request: " + error.text());
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }

                sendItemRefreshes(chnl, srcDirHandler.serviceInfo(), error);
            }
            }
            /*
             * login suspect from single-open provider: provider handles
             * recovery. consumer does nothing in this case.
             */
        }
        
		// get login reissue time from authenticationTTReissue
        if (responseMsg.msgClass() == MsgClasses.REFRESH &&
        	loginHandler.refreshInfo().checkHasAuthenticationTTReissue())
        {
			chnl.loginReissueTime = loginHandler.refreshInfo().authenticationTTReissue() * 1000;
			chnl.canSendLoginReissue = true;
        }
    }

    void setupDictionary(ChannelSession chnl)
    {
		if (!dictionaryHandler.loadDictionary(error)) 
		{
			/* if no local dictionary found maybe we can request it from ADH */
			System.out.println("Local dictionary not available, will try to request it from ADH if it supports the Provider Dictionary Download\n");

			if (loginHandler.refreshInfo().checkHasFeatures() 
					&& loginHandler.refreshInfo().features().checkHasSupportProviderDictionaryDownload()
					&& loginHandler.refreshInfo().features().supportProviderDictionaryDownload() == 1) 
    	{
    		int sendStatus = dictionaryHandler.sendDictionaryRequests(chnl,error,defaultServiceId);
    		
    		if( sendStatus == CodecReturnCodes.SUCCESS )
    		{
    			System.out.println("Sent Dictionary Request\n");
    		}
    		else
    		{
					System.out.println("Dictionary could not be downloaded, unable to send request to the connection: " + error.text());
    			closeChannel();
    			System.exit(TransportReturnCodes.FAILURE);
    		}
    	}  
    	else
    	{
				System.out.println("ADH does not support the Provider Dictionary Download\n");
    			closeChannel();
    			System.exit(TransportReturnCodes.FAILURE);
    		}
    	}
    }
    
    /*
     * send item refreshes for MARKET_PRICE and MARKET_BY_ORDER domains.
     */
    private int sendItemRefreshes(ChannelSession chnl, Service serviceInfo, Error error)
    {
        int ret;
        ret = marketPriceHandler.sendItemRefreshes(chnl, CommandLine.values("mp"),
                                                   srcDirHandler.serviceInfo(), error);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        ret = marketByOrderHandler.sendItemRefreshes(chnl, CommandLine.values("mbo"),
                                                     srcDirHandler.serviceInfo(), error);
        return ret;
    }

    private void closeItemStreams()
    {
        /* close item streams */
        marketPriceHandler.closeStreams(channelSession, error);
        marketByOrderHandler.closeStreams(channelSession, error);
    }

    /**
     * Closes all streams for the NIProvider.
     */
    public void uninitialize()
    {
        System.out.println("NIProvider unitializing and exiting...");
        if (channelSession.channel() == null)
        {
            channelSession.uninit(error);
            System.exit(TransportReturnCodes.SUCCESS);
        }

        //close all streams
        closeItemStreams();

        dictionaryHandler.closeStream(channelSession, error);
        
        srcDirHandler.closeStream(channelSession, error);

        //close login stream
        loginHandler.closeStream(channelSession, error);

        //flush before exiting
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
        CommandLine.programName("NIProvider");
        CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain.");
        CommandLine
                .addOption("mbo",
                           "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
        
        CommandLine.addOption("h", defaultSrvrHostname, "Server host name for TCP socket");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number for TCP socket");
        
        CommandLine.addOption("sa", "", "Send address for reliable multicast");
        CommandLine.addOption("sp", "", "Send port for reliable multicast");
        CommandLine.addOption("ra", "", "Receive address for reliable multicast");
        CommandLine.addOption("rp", "", "Receive port for reliable multicast");
        CommandLine.addOption("u", "", "Unicast port for reliable multicast");
        CommandLine.addOption("i", "", "Interface name for reliable multicast");
        
        CommandLine.addOption("s", defaultServiceName, "Service name");
        CommandLine.addOption("id", defaultServiceId, "ServiceId");
        CommandLine.addOption("uname", "Login user name. Default is system user name.");
        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("x", "Provides XML tracing of messages.");

        CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
        CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
        CommandLine.addOption("aid", "", "Specifies the Application ID.");
    }

    /**
     * The main method.
     *
     * @param args the arguments
     * @throws Exception the exception
     */
    public static void main(String[] args) throws Exception
    {
        NIProvider niprovider = new NIProvider();
        niprovider.init(args);
        niprovider.run(args);
        niprovider.uninitialize();
    }
}
