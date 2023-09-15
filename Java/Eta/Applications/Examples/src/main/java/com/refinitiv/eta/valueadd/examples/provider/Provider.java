/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.shared.DirectoryRejectReason;
import com.refinitiv.eta.shared.LoginRejectReason;
import com.refinitiv.eta.shared.DictionaryRejectReason;
import com.refinitiv.eta.shared.network.ChannelHelper;
import com.refinitiv.eta.shared.provider.ItemRejectReason;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.*;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.valueadd.cache.CacheFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;
import com.refinitiv.eta.valueadd.examples.common.CacheInfo;
import com.refinitiv.eta.valueadd.reactor.*;

/**
 * <p>
 * This is a main class to run the ETA Value Add Provider application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * The purpose of this application is to demonstrate interactively providing
 * data to OMM Consumer applications using ValueAdd components. It is a 
 * single-threaded server application.
 * </p>
 * <p>
 * The provider application implements callbacks that process requests
 * received by the consumer. It creates a Server and a Reactor and
 * waits for connections. It accepts these connections into the Reactor
 * and responds to messages through its Login, Directory, Dictionary, and 
 * default message callbacks.
 * </p>
 * <p>
 * This provider supports providing Level I Market Price, Level II Market By
 * Order, Level II Market By Price, and Symbol List data. Level II Market By
 * Price refresh messages are sent as multi-part messages. An update
 * message is sent between each part of the multi-part refresh message.
 * </p>
 * <p>
 * Batch requests are supported by this application. The login response message indicates
 * that batch support is present. Batch requests are accepted and a stream is opened
 * for each item in the batch request.
 * </p>
 * <p>
 * Posting requests are supported by this application for items that have already been
 * opened by a consumer. On-stream and off-stream posts are accepted and sent out to any
 * consumer that has the item open. Off-stream posts for items that have not already
 * been opened by a consumer are rejected (in this example).
 * </p>
 * <p>
 * Symbol List requests are expected to use a symbol list name of "_ETA_ITEM_LIST". The
 * symbol list name is provided in the source directory response for the consumer to use.
 * </p>
 * <p>
 * This application is intended as a basic usage example. Some of the design choices
 * were made to favor simplicity and readability over performance. This application 
 * is not intended to be used for measuring performance. This application uses
 * Value Add and shows how using Value Add simplifies the writing of ETA
 * applications. Because Value Add is a layer on top of ETA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the ETA interfaces.
 * </p>
 * <H2>Setup Environment</H2>
 * <p>
 * The RDMFieldDictionary and enumtype.def files must be located in the
 * directory of execution.
 * </p>
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runVAProvider -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runVAProvider -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-p server port number (defaults to 14002)
 * <li>-i interface name (defaults to null)
 * <li>-s service name (defaults to DIRECT_FEED)
 * <li>-id service id (defaults to 1)
 * <li>-x provides an XML trace of messages
 * <li>-cache application supports apply/retrieve data to/from cache
 * <li>-runtime application runtime in seconds (default is 1200)
 * <li>-rtt application (provider) supports calculation of Round Trip Latency
 * <li>-c Provider connection type.  Either "socket" or "encrypted"
 * <li>-keyfile jks encoded keyfile for Encrypted connections
 * <li>-keypasswd password for keyfile
 * </ul>
 */
public class Provider implements ProviderCallback, TunnelStreamListenerCallback, ReactorJsonConversionEventCallback,
        ReactorServiceNameToIdCallback
{
    // client sessions over this limit gets rejected with NAK mount
    static final int NUM_CLIENT_SESSIONS = 5;

    private static final int UPDATE_INTERVAL = 1;

    private BindOptions bindOptions = TransportFactory.createBindOptions();
    private Reactor reactor;
    private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
    private ReactorAcceptOptions reactorAcceptOptions = ReactorFactory.createReactorAcceptOptions();
    private ProviderRole providerRole = ReactorFactory.createProviderRole();
    private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
    private ReactorJsonConverterOptions jsonConverterOptions = ReactorFactory.createReactorJsonConverterOptions();
    private ProviderCmdLineParser providerCmdLineParser = new ProviderCmdLineParser();
    private Selector selector;
    private Error error = TransportFactory.createError();
    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private DictionaryHandler dictionaryHandler;
    private DirectoryHandler directoryHandler;
    private LoginHandler loginHandler;
    private ItemHandler itemHandler;
    private HashMap<ReactorChannel, TunnelStreamHandler> _tunnelStreamHandlerHashMap = new HashMap<ReactorChannel, TunnelStreamHandler>();
    private final HashMap<ReactorChannel, Integer> socketFdValueMap = new HashMap<>();

    private int clientSessionCount = 0;
    ArrayList<ReactorChannel> reactorChannelList = new ArrayList<ReactorChannel>();

    private String portNo;
    private String serviceName;
    private int serviceId;
    private long runtime;
    private CacheInfo cacheInfo = new CacheInfo();

    private long closetime;
    private long closeRunTime;
    boolean closeHandled;

    private long debugInfoInterval;
    private long nextDebugTime;

    /* default server port number */
    private static final String defaultSrvrPortNo = "14002";

    /* default service name */
    private static final String defaultServiceName = "DIRECT_FEED";

    /* default service id */
    private static final int defaultServiceId = 1;

    boolean _finalStatusEvent;

    public Provider()
    {
        dictionaryHandler = new DictionaryHandler();
        directoryHandler = new DirectoryHandler();
        loginHandler = new LoginHandler();
        itemHandler = new ItemHandler(dictionaryHandler, loginHandler);
        providerRole.channelEventCallback(this);
        providerRole.defaultMsgCallback(this);
        providerRole.dictionaryMsgCallback(this);
        providerRole.directoryMsgCallback(this);
        providerRole.loginMsgCallback(this);
        providerRole.tunnelStreamListenerCallback(this);
        _finalStatusEvent = true;
        closetime = 10;
    }

    /*
     * Initializes the Value Add consumer application. Parses command line arguments,
     * loads the dictionaries, creates a reactor and a binds a server for listening.
     * It also initializes Login, Directory, Dictionary and Item Handlers.
     */
    private void init(String[] args)
    {
        // parse command line
        if (!providerCmdLineParser.parseArgs(args))
        {
            System.err.println("\nError loading command line arguments:\n");
            providerCmdLineParser.printUsage();
            System.exit(ReactorReturnCodes.FAILURE);
        }

        if (providerCmdLineParser.portNo() != null)
        {
            portNo = providerCmdLineParser.portNo();
        }
        else
        {
            portNo = defaultSrvrPortNo;
        }
        if (providerCmdLineParser.serviceName() != null)
        {
            serviceName = providerCmdLineParser.serviceName();
        }
        else
        {
            serviceName = defaultServiceName;
        }
        if (providerCmdLineParser.serviceId() != 0)
        {
            serviceId = providerCmdLineParser.serviceId();
        }
        else
        {
            serviceId = defaultServiceId;
        }

        runtime = System.currentTimeMillis() + (providerCmdLineParser.runtime() * 1000);
        closeRunTime = System.currentTimeMillis() + (providerCmdLineParser.runtime() + closetime) * 1000;

        System.out.println("ConnectionType:" + (providerCmdLineParser.connectionType() == ConnectionTypes.SOCKET ? "socket" : "encrypted"));
        System.out.println("portNo: " + portNo);
        System.out.println("interfaceName: " + providerCmdLineParser.interfaceName());
        System.out.println("serviceName: " + serviceName);
        System.out.println("serviceId: " + serviceId);
        System.out.println("enableRTT: " + providerCmdLineParser.enableRtt());
        System.out.println("protocolList: " + providerCmdLineParser.protocolList());

        if(providerCmdLineParser.connectionType() == ConnectionTypes.ENCRYPTED)
        {
            System.out.println("keyfile: " + providerCmdLineParser.keyfile());
            System.out.println("keypasswd: " + providerCmdLineParser.keypasswd());
        }

        // load dictionary
        if (!dictionaryHandler.loadDictionary(error))
        {
            System.out.println("Error loading dictionary: " + error.text());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        if (providerCmdLineParser.cacheOption())
        {
            initializeCache();
            initializeCacheDictionary();
        }

        // enable Reactor XML tracing if specified
        if (providerCmdLineParser.enableXmlTracing())
        {
            reactorOptions.enableXmlTracing();
        }

        reactorOptions.debuggerOptions().setDebuggingLevels(providerCmdLineParser.getDebuggingLevels());
        debugInfoInterval = providerCmdLineParser.getDebugInfoInterval();

        // open selector
        try
        {
            selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.out.println("Unable to open selector: " + exception.getMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        // create reactor
        reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
        if (errorInfo.code() != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("createReactor() failed: " + errorInfo.toString());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        jsonConverterOptions.dataDictionary(dictionaryHandler.dictionary());
        jsonConverterOptions.serviceNameToIdCallback(this);
        jsonConverterOptions.jsonConversionEventCallback(this);
        jsonConverterOptions.defaultServiceId(serviceId);
        jsonConverterOptions.sendJsonConvError(providerCmdLineParser.sendJsonConvError());

        // Initialize the JSON converter
        if ( reactor.initJsonConverter(jsonConverterOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("Reactor.initJsonConverter() failed: " + errorInfo.toString());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        // bind server
        if(providerCmdLineParser.connectionType() == ConnectionTypes.ENCRYPTED)
        {
            bindOptions.connectionType(providerCmdLineParser.connectionType());
            bindOptions.encryptionOptions().keystoreFile(providerCmdLineParser.keyfile());
            bindOptions.encryptionOptions().keystorePasswd(providerCmdLineParser.keypasswd());
        }
        bindOptions.guaranteedOutputBuffers(1500);
        bindOptions.majorVersion(Codec.majorVersion());
        bindOptions.minorVersion(Codec.minorVersion());
        bindOptions.protocolType(Codec.protocolType());
        bindOptions.serviceName(portNo);
        bindOptions.interfaceName(providerCmdLineParser.interfaceName());
        bindOptions.wSocketOpts().protocols(providerCmdLineParser.protocolList());

        Server server = Transport.bind(bindOptions, error);
        if (server == null)
        {
            System.out.println("Error initializing server: " + error.text());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        System.out.println("\nServer bound on port " + server.portNumber());

        // register server for ACCEPT
        try
        {
            server.selectableChannel().register(selector, SelectionKey.OP_ACCEPT, server);
        }
        catch (ClosedChannelException e)
        {
            System.out.println("selector register failed: " + e.getLocalizedMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        // register selector with reactor's reactorChannel
        try
        {
            reactor.reactorChannel().selectableChannel().register(selector,
                    SelectionKey.OP_READ,
                    reactor.reactorChannel());
        }
        catch (ClosedChannelException e)
        {
            System.out.println("selector register failed: " + e.getLocalizedMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        // initialize handlers
        loginHandler.init();
        loginHandler.enableRtt(providerCmdLineParser.enableRtt());
        directoryHandler.init();
        directoryHandler.serviceName(serviceName);
        itemHandler.init(cacheInfo);
        directoryHandler.serviceId(serviceId);
        itemHandler.serviceId(serviceId);

        if (providerCmdLineParser.getDebuggingLevels() != ReactorDebuggerLevels.LEVEL_NONE) {
            nextDebugTime = System.currentTimeMillis() + debugInfoInterval;
        }
    }

    /*
     * Runs the Value Add consumer application. Polls socket events from server socket.
     * Accepts new client connections and reads requests from already established client
     * connection. Checks for runtime expiration. If there is no activity on the socket,
     * periodically sends item updates to connected client sessions that has requested
     * market price items.
     */
    private void run()
    {
        int ret = 0;
        // main loop
        while (true)
        {
            Set<SelectionKey> keySet = null;
            try
            {
                if (selector.select(UPDATE_INTERVAL * 1000) > 0)
                {
                    keySet = selector.selectedKeys();
                }
            }
            catch (IOException e1)
            {
                System.out.println(e1.getMessage());
                cleanupAndExit();
            }

            // nothing to read
            if (keySet == null)
            {
                /* Send market price updates for each connected channel */
                itemHandler.updateItemInfo();
                for (ReactorChannel reactorChnl : reactorChannelList)
                {
                    if (reactorChnl != null && reactorChnl.state() == ReactorChannel.State.READY)
                    {
                        /*try to send rtt message*/
                        loginHandler.sendRTT(reactorChnl, errorInfo);

                        /*process market price updates*/
                        ret = itemHandler.sendItemUpdates(reactorChnl, errorInfo);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                                reactorChnl.state() != ReactorChannel.State.DOWN)
                            {
                                System.out.println(errorInfo.error().text());
                                cleanupAndExit();
                            }
                        }
                    }
                }
            }
            else // notification occurred
            {
                Iterator<SelectionKey> iter = keySet.iterator();
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    if(!key.isValid())
                        continue;
                    if (key.isAcceptable()) // accept new connection
                    {
                        Server server = (Server)key.attachment();
                        clientSessionCount++;
                        reactorAcceptOptions.clear();
                        reactorAcceptOptions.acceptOptions().userSpecObject(server);
                        if (clientSessionCount <= NUM_CLIENT_SESSIONS)
                        {
                            reactorAcceptOptions.acceptOptions().nakMount(false);
                        }
                        else
                        {
                            reactorAcceptOptions.acceptOptions().nakMount(true);
                        }
                        if (reactor.accept(server, reactorAcceptOptions, providerRole, errorInfo) == ReactorReturnCodes.FAILURE)
                        {
                            System.out.println("Reactor accept error, text: " + errorInfo.error().text());
                            cleanupAndExit();
                        }
                    }
                    else if (key.isReadable()) // read from reactor channel
                    {
                        // retrieve associated reactor channel and dispatch on that channel
                        ReactorChannel reactorChnl = (ReactorChannel)key.attachment();
                        // dispatch until no more messages
                        while ((ret = reactorChnl.dispatch(dispatchOptions, errorInfo)) > 0) {}
                        if (ret == ReactorReturnCodes.FAILURE)
                        {
                            if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                                reactorChnl.state() != ReactorChannel.State.DOWN)
                            {
                                System.out.println("ReactorChannel dispatch failed");
                                cleanupAndExit();
                            }
                        }
                    }
                }
            }

            if (reactorOptions.debuggerOptions().debugEnabled() && System.currentTimeMillis() > nextDebugTime) {
                byte[] info = reactor.getDebuggingInfo();
                for (int i = 0; i < info.length; i++) {
                    System.out.print((char)info[i]);
                }
                nextDebugTime += debugInfoInterval;
            }

            // Handle run-time
            if (System.currentTimeMillis() >= runtime && !closeHandled)
            {
                System.out.println("Provider run-time expired, close now...");
                sendCloseStatusMessages();
                closeHandled = true;
            }
            else if (System.currentTimeMillis() >= closeRunTime )
            {
                System.out.println("Provider run-time expired...");
                break;
            }
            if(closeHandled && allTunnelStreamsClosed())
            {
                break;
            }
        }
    }

    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        ReactorChannel reactorChannel = event.reactorChannel();

        switch(event.eventType())
        {
            case ReactorChannelEventTypes.CHANNEL_UP:
            {
                /* A channel that we have requested via Reactor.accept() has come up.
                 * Register selector so we can be notified to start calling dispatch() for
                 * this channel. */
                System.out.println("\nConnection up!");
                Server server = (Server)reactorChannel.userSpecObj();
                System.out.println("Server " + server.selectableChannel() +
                                   ": New client on Channel " + reactorChannel.channel().selectableChannel());

                // register selector with channel event's reactorChannel
                try
                {
                    reactorChannel.selectableChannel().register(selector,
                            SelectionKey.OP_READ,
                            reactorChannel);
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }

                int rcvBufSize = 65535;
                int sendBufSize = 65535;
                /* Change size of send/receive buffer since it's small by default on some platforms */
                if (reactorChannel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    System.out.println("channel.ioctl() failed: " + error.text());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }

                if (reactorChannel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, rcvBufSize, errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    System.out.println("channel.ioctl() failed: " + error.text());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_READY:
                /* The channel has exchanged the messages necessary to setup the connection
                 * and is now ready for general use. For an RDM Provider, this normally immediately
                 * follows the CHANNEL_UP event. */
                reactorChannelList.add(reactorChannel);

                //define new socket fd value
                int fdSocketId =
                        ChannelHelper.defineFdValueOfSelectableChannel(reactorChannel.channel().selectableChannel());
                socketFdValueMap.put(reactorChannel, fdSocketId);
                break;
            case ReactorChannelEventTypes.FD_CHANGE:
                /* The notifier representing the ReactorChannel has been changed.
                 * Update notifiers. */
                System.out.println("Channel Change - Old Channel: "
                                   + reactorChannel.oldSelectableChannel() + " New Channel: "
                                   + reactorChannel.selectableChannel());

                //define new FDValue
                fdSocketId = ChannelHelper.defineFdValueOfSelectableChannel(reactorChannel.channel().selectableChannel());
                socketFdValueMap.put(reactorChannel, fdSocketId);

                // cancel old reactorChannel select
                SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();

                // register selector with channel event's new reactorChannel
                try
                {
                    reactorChannel.selectableChannel().register(selector,
                            SelectionKey.OP_READ,
                            reactorChannel);
                }
                catch (Exception e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down");

                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("	Error text: " + event.errorInfo().error().text() + "\n");

                // send close status messages to all item streams
                itemHandler.sendCloseStatusMsgs(reactorChannel, errorInfo);

                // send close status message to source directory stream
                directoryHandler.sendCloseStatus(reactorChannel, errorInfo);

                // send close status messages to dictionary streams
                dictionaryHandler.sendCloseStatusMsgs(reactorChannel, errorInfo);

                // close the tunnel stream
                TunnelStreamHandler tunnelStreamHandler = _tunnelStreamHandlerHashMap.get(reactorChannel);
                if (tunnelStreamHandler != null)
                {
                    tunnelStreamHandler.closeStream(_finalStatusEvent, errorInfo);
                }

                /* It is important to make sure that no more interface calls are made using the channel after
                 * calling ReactorChannel.close(). Because this application is single-threaded, it is safe
                 * to call it inside callback functions. */
                removeClientSessionForChannel(reactorChannel, errorInfo);
                break;
            }
            case ReactorChannelEventTypes.WARNING:
                System.out.println("Received ReactorChannel WARNING event\n");
                break;
            default:
                System.out.println("Unknown channel event!\n");
                cleanupAndExit();
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int defaultMsgCallback(ReactorMsgEvent event)
    {
        Msg msg = event.msg();
        ReactorChannel reactorChannel = event.reactorChannel();

        if (msg == null)
        {
            System.out.printf("defaultMsgCallback() received error: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
            removeClientSessionForChannel(reactorChannel, errorInfo);
            return ReactorCallbackReturnCodes.SUCCESS;
        }

        // clear decode iterator
        dIter.clear();
        // set buffer and version info
        if (msg.encodedDataBody().data() != null)
        {
            dIter.setBufferAndRWFVersion(msg.encodedDataBody(), reactorChannel.majorVersion(), reactorChannel.minorVersion());
        }

        switch (msg.domainType())
        {
            case DomainTypes.MARKET_PRICE:
            case DomainTypes.MARKET_BY_ORDER:
            case DomainTypes.MARKET_BY_PRICE:
            case DomainTypes.YIELD_CURVE:
            case DomainTypes.SYMBOL_LIST:
                if (itemHandler.processRequest(reactorChannel, msg, dIter, errorInfo) != CodecReturnCodes.SUCCESS)
                {
                    removeClientSessionForChannel(reactorChannel, errorInfo);
                    break;
                }
                break;
            default:
                switch (msg.msgClass())
                {
                    case MsgClasses.REQUEST:
                        if (itemHandler.sendItemRequestReject(reactorChannel, msg.streamId(), msg.domainType(), ItemRejectReason.DOMAIN_NOT_SUPPORTED, false, errorInfo) != CodecReturnCodes.SUCCESS)
                            removeClientSessionForChannel(reactorChannel, errorInfo);
                        break;
                    case MsgClasses.CLOSE:
                        System.out.println("Received close message with streamId=" + msg.streamId() + " and unsupported Domain '" + msg.domainType() + "'");
                        break;
                    default:
                        System.out.println("Received unhandled Msg Class: " + MsgClasses.toString(msg.msgClass()) + " with streamId=" + msg.streamId() + " and unsupported Domain '" + msg.domainType() + "'");
                        break;
                }
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        LoginMsg loginMsg = event.rdmLoginMsg();
        ReactorChannel reactorChannel = event.reactorChannel();

        if (loginMsg == null)
        {
            System.out.printf("loginMsgCallback() received error: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());

            if (event.msg() != null)
            {
                if (loginHandler.sendRequestReject(reactorChannel, event.msg().streamId(), LoginRejectReason.LOGIN_RDM_DECODER_FAILED, errorInfo) != CodecReturnCodes.SUCCESS)
                    removeClientSessionForChannel(reactorChannel, errorInfo);

                return ReactorCallbackReturnCodes.SUCCESS;
            }
            else
            {
                removeClientSessionForChannel(reactorChannel, errorInfo);
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }

        switch(loginMsg.rdmMsgType())
        {
            case REQUEST:
            {
                LoginRequest loginRequest = (LoginRequest)loginMsg;

                if (loginHandler.getLoginRequestInfo(reactorChannel, loginRequest) == null)
                {
                    removeClientSessionForChannel(reactorChannel, errorInfo);
                    break;
                }

                System.out.println("\nReceived Login Request for Username: " + loginRequest.userName());

                /* send login response */
                if (loginHandler.sendRefresh(reactorChannel, loginRequest, errorInfo) != CodecReturnCodes.SUCCESS)
                    removeClientSessionForChannel(reactorChannel, errorInfo);

                break;
            }
            case CLOSE:
                System.out.println("\nReceived Login Close for StreamId " + loginMsg.streamId());

                /* close login stream */
                loginHandler.closeStream(loginMsg.streamId());
                break;
            case RTT:
                LoginRTT loginRTT = (LoginRTT) loginMsg;
                System.out.printf("Received login RTT message from Consumer %d.\n", socketFdValueMap.get(event.reactorChannel()));
                System.out.printf("\tRTT Tick value is %dus.\n", TimeUnit.NANOSECONDS.toMicros(loginRTT.ticks()));
                if (loginRTT.checkHasTCPRetrans()) {
                    System.out.printf("\tConsumer side TCP retransmissions: %d\n", loginRTT.tcpRetrans());
                }
                long calculatedRtt = loginRTT.calculateRTTLatency(TimeUnit.MICROSECONDS);
                LoginRTT storedLoginRtt = loginHandler.getLoginRtt(reactorChannel).loginRtt();
                loginRTT.copy(storedLoginRtt);
                System.out.printf("\tLast RTT message latency is %dus.\n\n", calculatedRtt);
                break;
            default:
                System.out.println("\nReceived unhandled login msg type: " + loginMsg.rdmMsgType());
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        DirectoryMsg directoryMsg = event.rdmDirectoryMsg();
        ReactorChannel reactorChannel = event.reactorChannel();

        if (directoryMsg == null)
        {
            System.out.printf("directoryMsgCallback() received error: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());

            if (event.msg() != null)
            {
                if (directoryHandler.sendRequestReject(reactorChannel, event.msg().streamId(), DirectoryRejectReason.DIRECTORY_RDM_DECODER_FAILED, errorInfo) != CodecReturnCodes.SUCCESS)
                    removeClientSessionForChannel(reactorChannel, errorInfo);

                return ReactorCallbackReturnCodes.SUCCESS;
            }
            else
            {
                removeClientSessionForChannel(reactorChannel, errorInfo);
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }

        switch(directoryMsg.rdmMsgType())
        {
            case REQUEST:
            {
                DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsg;

                /* Reject any request that does not request at least the Info, State, and Group filters. */
                if (((directoryRequest.filter() & Directory.ServiceFilterFlags.INFO) == 0) ||
                    ((directoryRequest.filter() & Directory.ServiceFilterFlags.STATE) == 0) ||
                    ((directoryRequest.filter() & Directory.ServiceFilterFlags.GROUP) == 0))
                {
                    if (directoryHandler.sendRequestReject(reactorChannel, event.msg().streamId(), DirectoryRejectReason.INCORRECT_FILTER_FLAGS, errorInfo) != CodecReturnCodes.SUCCESS)
                        removeClientSessionForChannel(reactorChannel, errorInfo);

                    break;
                }

                if (directoryHandler.getDirectoryRequest(reactorChannel, directoryRequest) == null)
                {
                    removeClientSessionForChannel(reactorChannel, errorInfo);
                    break;
                }

                System.out.println("\nReceived Source Directory Request");

                /* send source directory response */
                if (directoryHandler.sendRefresh(reactorChannel, directoryRequest, errorInfo) != CodecReturnCodes.SUCCESS)
                    removeClientSessionForChannel(reactorChannel, errorInfo);

                break;
            }
            case CLOSE:
            {
                System.out.println("\nReceived Source Directory Close for StreamId " + directoryMsg.streamId());

                /* close source directory stream */
                directoryHandler.closeStream(directoryMsg.streamId());
                break;
            }
            default:
                System.out.println("\nReceived unhandled Source Directory msg type: " + directoryMsg.rdmMsgType());
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        int ret;
        DictionaryMsg dictionaryMsg = event.rdmDictionaryMsg();
        ReactorChannel reactorChannel = event.reactorChannel();

        if (dictionaryMsg == null)
        {
            System.out.printf("dictionaryMsgCallback() received error: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
            if (event.msg() != null)
            {
                if (dictionaryHandler.sendRequestReject(reactorChannel, event.msg().streamId(), DictionaryRejectReason.DICTIONARY_RDM_DECODER_FAILED, errorInfo) != CodecReturnCodes.SUCCESS)
                    removeClientSessionForChannel(reactorChannel, errorInfo);

                return ReactorCallbackReturnCodes.SUCCESS;
            }
            else
            {
                removeClientSessionForChannel(reactorChannel, errorInfo);
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }

        switch(dictionaryMsg.rdmMsgType())
        {
            case REQUEST:
            {
                DictionaryRequest dictionaryRequest = (DictionaryRequest)dictionaryMsg;

                if (dictionaryHandler.getDictionaryRequestInfo(reactorChannel, dictionaryRequest) == null)
                {
                    removeClientSessionForChannel(reactorChannel, errorInfo);
                    break;
                }

                System.out.println("\nReceived Dictionary Request for DictionaryName: " + dictionaryRequest.dictionaryName());

                if (DictionaryHandler.fieldDictionaryDownloadName.equals(dictionaryRequest.dictionaryName()))
                {
                    /* Name matches field dictionary. Send the field dictionary refresh. */
                    if ((ret = dictionaryHandler.sendFieldDictionaryResponse(reactorChannel, dictionaryRequest, errorInfo)) != CodecReturnCodes.SUCCESS)
                    {
                        System.out.println("sendFieldDictionaryResponse() failed: " + ret);
                        removeClientSessionForChannel(reactorChannel, errorInfo);
                    }
                }
                else if (DictionaryHandler.enumTypeDictionaryDownloadName.equals(dictionaryRequest.dictionaryName()))
                {
                    /* Name matches the enum types dictionary. Send the enum types dictionary refresh. */
                    if ((ret = dictionaryHandler.sendEnumTypeDictionaryResponse(reactorChannel, dictionaryRequest, errorInfo)) != CodecReturnCodes.SUCCESS)
                    {
                        System.out.println("sendEnumTypeDictionaryResponse() failed: " + ret);
                        removeClientSessionForChannel(reactorChannel ,errorInfo);
                    }
                }
                else
                {
                    if (dictionaryHandler.sendRequestReject(reactorChannel, event.msg().streamId(), DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, errorInfo) != CodecReturnCodes.SUCCESS)
                    {
                        removeClientSessionForChannel(reactorChannel, errorInfo);
                    }
                }
                break;
            }
            case CLOSE:
                System.out.println("\nReceived Dictionary Close for StreamId " + dictionaryMsg.streamId());

                /* close dictionary stream */
                dictionaryHandler.closeStream(dictionaryMsg.streamId());
                break;
            default:
                System.out.println("\nReceived Unhandled Dictionary Msg Type: " + dictionaryMsg.rdmMsgType());
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int listenerCallback(TunnelStreamRequestEvent event)
    {
        TunnelStreamHandler tunnelStreamHandler = new TunnelStreamHandler();

        _tunnelStreamHandlerHashMap.put(event.reactorChannel(), tunnelStreamHandler);

        tunnelStreamHandler.processNewStream(event);

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent)
    {
        System.out.println("JSON Conversion error: " + jsonConversionEvent.error().text());

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId,
                                              ReactorServiceNameToIdEvent serviceNameToIdEvent)
    {
    	if(directoryHandler.serviceName().equals(serviceNameToId.serviceName()))
    	{
    		serviceNameToId.serviceId(directoryHandler.serviceId());
    		return ReactorReturnCodes.SUCCESS;
    	}

        return ReactorReturnCodes.FAILURE;
    }

    /*
     * Sends close status messages to all streams on all channels.
     */
    private void sendCloseStatusMessages()
    {
        // send close status messages to all streams on all channels
        for (ReactorChannel reactorChnl : reactorChannelList)
        {
            if ((reactorChnl != null))
            {
                // send close status messages to all item streams 
                if (itemHandler.sendCloseStatusMsgs(reactorChnl, errorInfo) != CodecReturnCodes.SUCCESS)
                    System.out.println("Error sending item close: " + errorInfo.error().text());

                // close tunnel stream
                TunnelStreamHandler tunnelStreamHandler = _tunnelStreamHandlerHashMap.get(reactorChnl);
                if (tunnelStreamHandler != null)
                {
                    if (tunnelStreamHandler.closeStream(_finalStatusEvent, errorInfo) != CodecReturnCodes.SUCCESS)
                        System.out.println("Error closing tunnel stream: " + errorInfo.error().text());
                }

                // send close status message to source directory stream
                if (directoryHandler.sendCloseStatus(reactorChnl, errorInfo) != CodecReturnCodes.SUCCESS)
                    System.out.println("Error sending directory close: " + errorInfo.error().text());

                // send close status messages to dictionary streams
                if (dictionaryHandler.sendCloseStatusMsgs(reactorChnl, errorInfo) != CodecReturnCodes.SUCCESS)
                    System.out.println("Error sending dictionary close: " + errorInfo.error().text());

            }
        }
    }

    /*
     * Removes a client session for a channel.
     */
    private void removeClientSessionForChannel(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        if (reactorChannel.selectableChannel() != null)
        {
            SelectionKey key = reactorChannel.selectableChannel().keyFor(selector);
            if (key != null)
                key.cancel();
        }

        reactorChannelList.remove(reactorChannel);
        dictionaryHandler.closeStream(reactorChannel);
        directoryHandler.closeStream(reactorChannel);
        loginHandler.closeStream(reactorChannel);
        itemHandler.closeStream(reactorChannel, errorInfo);
        reactorChannel.close(errorInfo);
        socketFdValueMap.remove(reactorChannel);
        clientSessionCount--;
    }

    /*
     * initializeCache
     */
    private void initializeCache()
    {
        cacheInfo.useCache = true;
        cacheInfo.cacheOptions.maxItems(10000);
        cacheInfo.cacheDictionaryKey.data("cacheDictionary1");

        cacheInfo.cache = CacheFactory.createPayloadCache(cacheInfo.cacheOptions, cacheInfo.cacheError);
        if (cacheInfo.cache == null)
        {
            System.out.println("Error: Failed to create cache. Error (" + cacheInfo.cacheError.errorId() +
                               ") : " + cacheInfo.cacheError.text());
            cacheInfo.useCache = false;
        }

        cacheInfo.cursor = CacheFactory.createPayloadCursor();
        if (cacheInfo.cursor == null)
        {
            System.out.println("Error: Failed to create cache entry cursor.");
            cacheInfo.useCache = false;
        }
    }

    /*
     * unintializeCache
     */
    private void uninitializeCache()
    {
        if (cacheInfo.cache != null)
            cacheInfo.cache.destroy();
        cacheInfo.cache = null;

        if (cacheInfo.cursor != null)
            cacheInfo.cursor.destroy();
        cacheInfo.cursor = null;
    }

    /*
     * initalizeCacheDictionary
     */
    private void initializeCacheDictionary()
    {
        DataDictionary dictionary = dictionaryHandler.dictionary();
        if (dictionary != null)
        {
            if ( cacheInfo.cache.setDictionary(dictionary,	cacheInfo.cacheDictionaryKey.toString(),
                    cacheInfo.cacheError) != CodecReturnCodes.SUCCESS )
            {
                System.out.println("Error: Failed to bind RDM Field dictionary to cache. Error (" + cacheInfo.cacheError.errorId() +
                                   ") : " + cacheInfo.cacheError.text());
                cacheInfo.useCache = false;
            }
        }
        else
        {
            System.out.println("Error: No RDM Field dictionary for cache.\n");
            cacheInfo.useCache = false;
        }
    }

    private boolean allTunnelStreamsClosed()
    {
        boolean ret = true;

        for (TunnelStreamHandler tunnelStreamHandler : _tunnelStreamHandlerHashMap.values())
        {
            if (tunnelStreamHandler.isStreamClosed() != true)
            {
                ret = false;
                break;
            }
        }

        return ret;
    }

    /*
     * Cleans up and exits.
     */
    private void cleanupAndExit()
    {
        uninitializeCache();
        uninitialize();
        System.exit(ReactorReturnCodes.FAILURE);
    }

    /* Uninitializes the Value Add consumer application. */
    private void uninitialize()
    {
        // shutdown reactor
        reactor.shutdown(errorInfo);
    }

    public static void main(String[] args) throws Exception
    {
        Provider provider = new Provider();
        provider.init(args);
        provider.run();
        provider.uninitialize();
        System.exit(ReactorReturnCodes.SUCCESS);
    }
}
