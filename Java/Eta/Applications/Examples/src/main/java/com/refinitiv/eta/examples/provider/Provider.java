/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.provider;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.shared.*;
import com.refinitiv.eta.examples.common.ProviderDictionaryHandler;
import com.refinitiv.eta.examples.common.ProviderLoginHandler;
import com.refinitiv.eta.examples.common.UnSupportedMsgHandler;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.*;
import com.refinitiv.eta.transport.Error;

/**
 * This is the main class for the ETA Java Provider application. It is a
 * single-threaded server application. The application uses either the operating
 * parameters entered by the user or a default set of parameters.
 * <p>
 * The purpose of this application is to interactively provide Level I Market Price, 
 * Level II Market By Order, Level II Market By Price, and Symbol List data to 
 * one or more consumers. It allowed, it requests dictionary from the adh.
 * <p>
 * It is a single-threaded server application. First the application initializes 
 * the ETA transport and binds the server. If the dictionary files are in the path
 * it loads dictionary information from the RDMFieldDictionary and enumtype.def files. 
 * Finally, it processes login, source directory, dictionary, market price, 
 * market by order, market by price, and symbol list 
 * requests from consumers and sends the appropriate responses.
 * <p>
 * Level II Market By Price refresh messages are sent as multi-part messages. An 
 * update message is sent between each part of the multi-part refresh message.
 * <p>
 * Dictionary requests are supported by this application. If the Login Request
 * indicates the presence of the dictionary request feature and the dictionaries are
 * not setup on startup, this application sends dictionary requests for both
 * field dictionary and enumtype dictionary. The responses to the dictionary requests
 * are processed as well.
 * <p>
 * Batch requests are supported by this application. The login response message 
 * indicates that batch support is present. Batch requests are accepted and a stream 
 * is opened for each item in the batch request.
 * <p>
 * Posting requests are supported by this application for items that have already 
 * been opened by a consumer. On-stream and off-stream posts are accepted and sent 
 * out to any consumer that has the item open. Off-stream posts for items that 
 * have not already been opened by a consumer are rejected (in this example). 
 * <p>
 * Private stream requests are also supported by this application. All items requested 
 * with the private stream flag set in the request message result in the private 
 * stream flag set in the applicable response messages. If a request is received 
 * without the private stream flag set for the item name of "RES-DS", this application 
 * redirects the consumer to open the "RES-DS" item on a private stream instead 
 * of a normal stream.
 * <p>
 * Symbol List requests are expected to use a symbol list name of "_ETA_ITEM_LIST". 
 * The symbol list name is provided in the source directory response for the consumer 
 * to use.
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
 * The RDMFieldDictionary and enumtype.def files must be located in the
 * directory of execution. If not available and adh supports dictionary requests,
 * the dictionary is down loaded from adh.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runProvider -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runProvider -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-id Service id. Default is <i>1</i>.
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-x Provides XML tracing of messages.
 * <li>-runtime run time. Default is 1200 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * <li>-rtt application (provider) supports calculation of Round Trip Latency
 * <li>-pl commas (',') delineated list of supported sub-protocols for asserting WebSocket connection
 * <li>-httpHdr expand all enumerated values with a JSON protocol
 * <li>-jsonEnumExpand http header will be accessible on the provider side through callback function
 * </ul>
 *
 * @see ProviderSession
 * @see ProviderDictionaryHandler
 * @see ProviderDirectoryHandler
 * @see ProviderLoginHandler
 * @see ItemHandler
 */
public class Provider implements ReceivedMsgCallback, HttpCallback
{
    private ProviderSession _providerSession;
    private DecodeIterator _dIter = CodecFactory.createDecodeIterator();
    private Msg _receivedMsg = CodecFactory.createMsg();
    private Error _error = TransportFactory.createError();
    private UnSupportedMsgHandler _unSupportedMsgHandler;
    private ProviderDictionaryHandler _dictionaryHandler;
    private ProviderDirectoryHandler _directoryHandler;
    private ProviderLoginHandler _loginHandler;
    private ItemHandler _itemHandler;

    private long _runtime;

    private static final int UPDATE_INTERVAL = 1;
    private long publishTime = 0;

    private static final String DEFAULT_WS_PROTOCOL = "rssl.rwf, rssl.json.v2";

    /* default server port number */
    private static final String defaultSrvrPortNo = "14002";

    /* default service name */
    private static final String defaultServiceName = "DIRECT_FEED";

    /* default run time in seconds */
    private static final String defaultRuntime = "1200"; // seconds

    public static final int CLIENT_SESSION_INIT_TIMEOUT = 30; // seconds

    public Provider()
    {
        _providerSession = new ProviderSession();
        _unSupportedMsgHandler = new UnSupportedMsgHandler(_providerSession);
        _dictionaryHandler = new ProviderDictionaryHandler(_providerSession);
        _directoryHandler = new ProviderDirectoryHandler(_providerSession);
        _loginHandler = new ProviderLoginHandler(_providerSession);
        _itemHandler = new ItemHandler(_providerSession, _dictionaryHandler, _loginHandler);
    }

    public static void main(String[] args)
    {
        Provider provider = new Provider();
        provider.init(args);
        provider.run();
        provider.uninit();
    }

    /**
     * Parses command line arguments, initializes provider session which creates
     * listening socket. It also initializes Login, Directory, Dictionary and
     * Item Handlers.
     *
     * @param args - command line arguments
     */
    public void init(String[] args)
    {
        /* process command line args */
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
        System.out.println("connectionType: " + CommandLine.value("c"));
        System.out.println("portNo: " + CommandLine.value("p"));
        System.out.println("interfaceName: " + CommandLine.value("i"));
        System.out.println("serviceName: " + CommandLine.value("s"));
        System.out.println("serviceId: " + CommandLine.value("id"));
        System.out.println("enableRTT: " + CommandLine.value("rtt"));
        String connectionType = CommandLine.value("c");
        if(connectionType != null && connectionType.equals("encrypted"))
        {
            System.out.println("keyfile: " + CommandLine.value("keyfile"));
            System.out.println("keypasswd: " + CommandLine.value("keypasswd"));
        }
        System.out.println("protocolList: " + CommandLine.value("pl"));
        System.out.println("httpHdrEnabled: " + CommandLine.value("httpHdr"));
        System.out.println("jsonEnumExpand: " + CommandLine.value("jsonEnumExpand"));
        System.out.println("serverSharedSocket: " + CommandLine.value("serverSharedSocket"));

        if ( ! _dictionaryHandler.loadDictionary(_error) )
        {
            /* if no local dictionary found maybe we can request it from ADH */
            System.out.println("Local dictionary not available, will try to request it from ADH if it supports the Provider Dictionary Download\n");
        }

        // get bind options from the provider session
        BindOptions bindOptions = _providerSession.getBindOptions();

        // set the connection parameters on the bind options
        bindOptions.serviceName(CommandLine.value("p"));
        bindOptions.interfaceName(CommandLine.value("i"));
        if(connectionType != null && connectionType.equals("encrypted"))
        {
            bindOptions.connectionType(ConnectionTypes.ENCRYPTED);
            bindOptions.encryptionOptions().keystoreFile(CommandLine.value("keyfile"));
            bindOptions.encryptionOptions().keystorePasswd(CommandLine.value("keypasswd"));
            String securityProvider = CommandLine.value("securityProvider");
            if(securityProvider != null && !securityProvider.isEmpty())
                bindOptions.encryptionOptions().securityProvider(securityProvider);
            if (CommandLine.hasArg("spTLSv1.2") && CommandLine.hasArg("spTLSv1.3"))
            {
            	bindOptions.encryptionOptions().securityProtocol("TLS");
            	bindOptions.encryptionOptions().securityProtocolVersions(new String[] {"1.2","1.3"});
            }
            else if (CommandLine.hasArg("spTLSv1.2"))
            {
            	bindOptions.encryptionOptions().securityProtocol("TLS");
            	bindOptions.encryptionOptions().securityProtocolVersions(new String[] {"1.2"});
            }
            else if (CommandLine.hasArg("spTLSv1.3"))
            {
            	bindOptions.encryptionOptions().securityProtocol("TLS");
            	bindOptions.encryptionOptions().securityProtocolVersions(new String[] {"1.3"});
            }
            else
            {
            	bindOptions.encryptionOptions().securityProtocol("TLS");
            	bindOptions.encryptionOptions().securityProtocolVersions(new String[] {"1.2","1.3"});
            }
        }
        bindOptions.wSocketOpts().protocols(CommandLine.value("pl"));

        if (CommandLine.booleanValue("httpHdr")) {
            bindOptions.wSocketOpts().httpCallback(this);
        }

        if (CommandLine.booleanValue("serverSharedSocket")) {
            bindOptions.serverSharedSocket(true);
            if (System.getProperty("os.name").toLowerCase().contains("windows")) {
                if (Boolean.parseBoolean(System.getProperty("sun.net.useExclusiveBind", "true"))) {
                    System.setProperty("sun.net.useExclusiveBind", "false");
                }
                System.out.println("sun.net.useExclusiveBind: " + System.getProperty("sun.net.useExclusiveBind"));
            }
        }

        final JsonConverterInitOptions converterInitOptions = new JsonConverterInitOptions(
                _directoryHandler,
                _dictionaryHandler.dictionary(),
                CommandLine.booleanValue("jsonEnumExpand"),
                CommandLine.booleanValue("x")
        );

        int defaultServiceId = CommandLine.intValue("id");
        if (defaultServiceId != 0) {
            converterInitOptions.setDefaultServiceId(defaultServiceId);
        }

        int ret = _providerSession.init(converterInitOptions, false, _error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            System.out.println("Error initializing server: " + _error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }


        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            _providerSession.enableXmlTrace(_dictionaryHandler.dictionary());
        }

        _loginHandler.init();
        _directoryHandler.init();
        _directoryHandler.serviceName(CommandLine.value("s"));
        _itemHandler.init();
        try
        {
            _loginHandler.enableRtt(CommandLine.booleanValue("rtt"));
            _directoryHandler.serviceId(CommandLine.intValue("id"));
            _itemHandler.serviceId(CommandLine.intValue("id"));
            _runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;
        }
        catch (NumberFormatException ile)
        {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.exit(-1);
        }
    }

    private static void addCommandLineArgs()
    {
        CommandLine.programName("ETA Provider");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number");
        CommandLine.addOption("s", defaultServiceName, "Service name");
        CommandLine.addOption("i", (String)null, "Interface name");
        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("id", "1", "Service id");
        CommandLine.addOption("x", "Provides XML tracing of messages.");
        CommandLine.addOption("rtt", false, "Provider supports calculation of Round Trip Latency");
        CommandLine.addOption("c", (String)null, "Provider connection type.  Either \"socket\" or \"encrypted\"");
        CommandLine.addOption("keyfile", (String)null, "jks encoded keyfile for Encrypted connections");
        CommandLine.addOption("keypasswd", (String)null, "password for keyfile");
		CommandLine.addOption("securityProvider", "", "Specifies security provider, default is SunJSSE, also supports Conscrypt");
        CommandLine.addOption("pl", DEFAULT_WS_PROTOCOL, "commas (',') delineated list of supported sub-protocols for asserting WebSocket connection");
        CommandLine.addOption("jsonEnumExpand", false, "if specified, expand all enumerated values with a JSON protocol");
        CommandLine.addOption("httpHdr", false, "if specified, http header will be accessible on the provider side through callback function");
        CommandLine.addOption("serverSharedSocket", false, "if specified, turn on server shared socket");
        CommandLine.addOption("spTLSv1.2", "Specifies for the application to be able to use TLS version 1.2. Default enables both TLS versions 1.2 and 1.3.");
        CommandLine.addOption("spTLSv1.3", "Specifies for the application to be able to use TLS version 1.3. Default enables both TLS versions 1.2 and 1.3.");
        }

    /*
     * Handles the run-time for the Provider. Sends close status messages to
     * all streams on all channels after run-time has elapsed.
     */
    private void handleRuntime()
    {
        // get current time
        long currentTime = System.currentTimeMillis();

        if (currentTime >= _runtime)
        {
            // send close status messages to all streams on all channels
            for (ClientSessionInfo clientSessionInfo : _providerSession.clientSessions)
            {
                if ((clientSessionInfo != null) &&
                    (clientSessionInfo.clientChannel() != null &&
                     clientSessionInfo.clientChannel().selectableChannel() != null &&
                     clientSessionInfo.clientChannel().state() != ChannelState.INACTIVE))
                {
                    // send close status messages to all item streams
                    int ret = _itemHandler.sendCloseStatusMsgs(clientSessionInfo.clientChannel(), _error);
                    if (ret != 0)
                        System.out.println("Error sending item close: " + _error.text());

                    // send close status message to source directory stream
                    ret = _directoryHandler.sendCloseStatus(clientSessionInfo.clientChannel(), _error);
                    if (ret != 0)
                        System.out.println("Error sending directory close: " + _error.text());

                    // send close status messages to dictionary streams
                    _dictionaryHandler.sendCloseStatusMsgs(clientSessionInfo.clientChannel(), _error);
                    if (ret != 0)
                        System.out.println("Error sending dictionary close: " + _error.text());

                    // flush before exiting
                    if ( clientSessionInfo.clientChannel() != null && clientSessionInfo.clientChannel().selectableChannel() != null)
                    {
                        SelectionKey key = clientSessionInfo.clientChannel().selectableChannel().keyFor(_providerSession.selector);

                        if (key != null && key.isValid() && key.isWritable())
                        {
                            ret = 1;
                            while (ret > TransportReturnCodes.SUCCESS)
                            {
                                ret = clientSessionInfo.clientChannel().flush(_error);
                            }
                            if (ret < TransportReturnCodes.SUCCESS)
                            {
                                System.out.println("clientChannel.flush() failed with return code " + ret + _error.text());
                            }
                        }
                    }
                }
            }
            System.out.println("provider run-time expired...");
            uninit();
            System.exit(0);
        }

    }

    /**
     * Main loop polls socket events from server socket. Accepts new client
     * connections and reads requests from already established client
     * connection. Checks for runtime expiration. If there is no activity on the
     * socket, periodically sends item updates to connected client sessions that
     * has requested market price items.
     */
    public void run()
    {
        int ret = 0;

        // main loop
        while (true)
        {
            Set<SelectionKey> keySet = null;
            try
            {
                if (_providerSession.selector.select(UPDATE_INTERVAL * 200) > 0)
                {
                    keySet = _providerSession.selector.selectedKeys();
                }
            }
            catch (IOException e1)
            {
                System.out.println(e1.getMessage());
                cleanupAndExit();
            }

            if (publishTime < System.currentTimeMillis())
            {
                /* Send market price updates for each connected channel */
                _itemHandler.updateItemInfo();

                for (ClientSessionInfo clientSessionInfo : _providerSession.clientSessions)
                {
                    if ((clientSessionInfo != null) &&
                        (clientSessionInfo.clientChannel() != null &&
                         clientSessionInfo.clientChannel().selectableChannel() != null &&
                         clientSessionInfo.clientChannel().state() != ChannelState.INACTIVE))
                    {
                        _loginHandler.proceedLoginRttMessage(clientSessionInfo.clientChannel(), _error);
                        ret = _itemHandler.sendItemUpdates(clientSessionInfo.clientChannel(), _error);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            System.out.println(_error.text());
                            processChannelClose(clientSessionInfo.clientChannel());
                            _providerSession.removeClientSessionForChannel(clientSessionInfo.clientChannel(), _error);
                            removeInactiveSessions();
                        }
                    }
                }
                publishTime = System.currentTimeMillis() + 1000;
            }

            if (keySet != null)
            {
                checkTimeout();
                Iterator<SelectionKey> iter = keySet.iterator();

                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    if(!key.isValid())
                    {
                        key.cancel();
                        continue;
                    }
                    if (key.isAcceptable())
                    {
                        ret = _providerSession.handleNewClientSession((Server)key.attachment(), _error);
                        if (ret != TransportReturnCodes.SUCCESS)
                        {
                            System.out.println("accept error, text: " + _error.text());
                            continue;
                        }
                    }
                    else if (key.isReadable())
                    {
                        ret = _providerSession.read((Channel)key.attachment(), _error, this);
                        if (ret != TransportReturnCodes.SUCCESS)
                        {
                            try
                            {
                                key.channel().close();
                            }
                            catch(Exception e)
                            {
                            }
                            System.out.println("read error, text: " + _error.text());
                            continue;
                        }
                    }
                    else if (key.isWritable() && ((Channel)key.attachment()).state() == ChannelState.ACTIVE)
                    {
                        _providerSession.flush(key, _error);
                    }
                }
            }

            /* Handle pings */
            _providerSession.handlePings();

            /* Handle run-time */
            handleRuntime();
        }
    }

    /**
     * Call back for socket read for client messages.
     */
    public void processReceivedMsg(Channel channel, TransportBuffer msgBuf)
    {
        int ret;

        /* clear decode iterator */
        _dIter.clear();

        if (channel.protocolType() == Codec.JSON_PROTOCOL_TYPE) {
            final ClientSessionInfo clientSessionInfo = _providerSession.getClientSessionForChannel(channel);
            ret = clientSessionInfo.getJsonSession().convertFromJson(msgBuf, _error);

            if (ret == TransportReturnCodes.FAILURE) {
                System.out.printf("\nJson to RWF conversion failed. Additional information: %s\n", _error.text());
                return;
            }

            if (ret == TransportReturnCodes.READ_PING) {
                return;
            }

            ret = _dIter.setBufferAndRWFVersion(
                    clientSessionInfo.getJsonSession().getJsonMsg().rwfMsg().encodedMsgBuffer(),
                    channel.majorVersion(),
                    channel.minorVersion()
            );
        } else {
            /* set buffer and version info */
            ret = _dIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        }

        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            processChannelClose(channel);
            _providerSession.removeClientSessionForChannel(channel, _error);
            removeInactiveSessions();

        }

        ret = _receivedMsg.decode(_dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("RequestMsg.decode() failed with return code: " + CodecReturnCodes.toString(ret) + " on SessionData " + channel.selectableChannel() + "  Size " + (msgBuf.data().limit() - msgBuf.data().position()));
            processChannelClose(channel);
            _providerSession.removeClientSessionForChannel(channel, _error);
            removeInactiveSessions();
        }

        switch (_receivedMsg.domainType())
        {
            case DomainTypes.LOGIN:
            {
                if (_loginHandler.processRequest(channel, _receivedMsg, _dIter, _error) != 0)
                {
                    System.out.println("Error processing login request: " + _error.text());
                    processChannelClose(channel);
                    _providerSession.removeClientSessionForChannel(channel, _error);
                    removeInactiveSessions();
                }

                // request dictionary from ADH if not available locally
                if ( ! _dictionaryHandler.isDictionaryReady() )
                {
                    LoginRequestInfo loginReqInfo = _loginHandler.findLoginRequestInfo(channel);

                    if( loginReqInfo.loginRequest().checkHasAttrib() &&
                        loginReqInfo.loginRequest().attrib().checkHasProviderSupportDictionaryDownload() &&
                        loginReqInfo.loginRequest().attrib().supportProviderDictionaryDownload() ==1 )
                    {
                        int requestStatus = _dictionaryHandler.sendDictionaryRequests(channel,_error,_directoryHandler.serviceId());
                        if( requestStatus == CodecReturnCodes.SUCCESS )
                        {
                            System.out.println("Sent Dictionary Request\n");
                        }
                        else
                        {
                            System.out.println("Dictionary could not be downloaded, unable to send the request to the connection "+_error.text());
                            processChannelClose(channel);
                            _providerSession.removeClientSessionForChannel(channel, _error);
                            removeInactiveSessions();
                        }
                    }
                    else
                    {
                        System.out.println("Dictionary could not be downloaded, the connection does not support Provider Dictionary Download");
                        processChannelClose(channel);
                        _providerSession.removeClientSessionForChannel(channel, _error);
                        removeInactiveSessions();
                    }
                }
                break;
            }

            case DomainTypes.SOURCE:
                if (_directoryHandler.processRequest(channel, _receivedMsg, _dIter, _error) != 0)
                {
                    System.out.println("Error processing directory request: " + _error.text());
                    processChannelClose(channel);
                    _providerSession.removeClientSessionForChannel(channel, _error);
                    removeInactiveSessions();
                }
                break;
            case DomainTypes.DICTIONARY:
                if(_dictionaryHandler.processMessage(channel,_receivedMsg, _dIter, _error) != 0)
                {
                    System.out.println("Error processing dictionary message: " + _error.text());
                    processChannelClose(channel);
                    _providerSession.removeClientSessionForChannel(channel, _error);
                    removeInactiveSessions();
                }
                break;

            case DomainTypes.MARKET_PRICE:
            case DomainTypes.MARKET_BY_ORDER:
            case DomainTypes.MARKET_BY_PRICE:
            case DomainTypes.SYMBOL_LIST:
                if (_itemHandler.processRequest(channel, _receivedMsg, _dIter, _error) != 0)
                {
                    System.out.println("Error processing item request: " + _error.text());
                    processChannelClose(channel);
                    _providerSession.removeClientSessionForChannel(channel, _error);
                    removeInactiveSessions();
                }
                break;
            default:
                if (_unSupportedMsgHandler.processRequest(channel, _receivedMsg, _error) != 0)
                {
                    System.out.println("Error processing unhandled request message: " + _error.text());
                    processChannelClose(channel);
                    _providerSession.removeClientSessionForChannel(channel, _error);
                    removeInactiveSessions();
                }
                break;
        }
    }

    private void checkTimeout()
    {
        for (ClientSessionInfo clientSessionInfo : _providerSession.clientSessions)
        {
            if (clientSessionInfo != null &&
                clientSessionInfo.clientChannel() != null &&
                clientSessionInfo.clientChannel().state() == ChannelState.INITIALIZING)
            {
                if ((System.currentTimeMillis() - clientSessionInfo.startTime()) > CLIENT_SESSION_INIT_TIMEOUT * 1000)
                {
                    System.out.println("Provider close clientSesson due to timeout of initialization " + clientSessionInfo.clientChannel().selectableChannel() +  " curTime = " + System.currentTimeMillis() +  " startTime = " +  clientSessionInfo.startTime());
                    _providerSession.removeClientSessionForChannel(clientSessionInfo.clientChannel(), _error);
                    removeInactiveSessions();
                }
            }
        }
    }

    private void removeInactiveSessions()
    {
        for (ClientSessionInfo clientSessionInfo : _providerSession.clientSessions)
        {
            if (clientSessionInfo != null &&
                clientSessionInfo.clientChannel() != null &&
                clientSessionInfo.clientChannel().state() == ChannelState.INACTIVE && clientSessionInfo.startTime() > 0 )
            {
                System.out.println("Provider close clientSesson due to inactive state ");
                _providerSession.removeInactiveClientSessionForChannel(clientSessionInfo, _error);
            }
        }
    }


    private void uninit()
    {
        _providerSession.uninit();
    }

    private void cleanupAndExit()
    {
        _providerSession.uninit();
        System.exit(TransportReturnCodes.FAILURE);
    }

    @Override
    public void processChannelClose(Channel channel)
    {
        _itemHandler.closeRequests(channel);
        _dictionaryHandler.closeRequests(channel);
        _directoryHandler.closeRequest(channel);
        _loginHandler.closeRequestAndRtt(channel);
    }

    @Override
    public void httpCallback(HttpMessage httpMessage, Error error) {
        if (error.errorId() < TransportReturnCodes.SUCCESS) {
            System.out.printf("Http header error %s \n", error.text());
        }

        if (Objects.nonNull(httpMessage.getHttpRequestConnectionInfo())) {
            if (Objects.nonNull(httpMessage.getHttpRequestConnectionInfo().getConnectionUri())) {
                System.out.println("HTTP URI: " + httpMessage.getHttpRequestConnectionInfo().getConnectionUri());
            }
        }

        final List<HttpHeader> httpHeaders = httpMessage.getHttpHeaders();
        System.out.println("HTTP header data:");
        if (Objects.nonNull(httpHeaders) && !httpHeaders.isEmpty()) {
            httpHeaders.forEach(httpHeader -> System.out.printf("%s: %s\n", httpHeader.getHeaderName(), httpHeader.getSimpleHeaderValue()));
        } else {
            System.out.println("Failed to get HTTP headers.");
        }

        final java.util.Map<String, String> cookies = httpMessage.getCookies();
        System.out.println("HTTP cookies data:");
        if (Objects.nonNull(cookies) && !cookies.isEmpty()) {
            cookies.forEach((key, value) -> System.out.printf("%s: %s\n", key, value));
        } else {
            System.out.println("HTTP cookies is empty.");
        }
    }
}
