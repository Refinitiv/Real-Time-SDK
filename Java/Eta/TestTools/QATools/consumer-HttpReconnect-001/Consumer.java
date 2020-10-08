package com.refinitiv.eta.examples.consumer;

import java.net.InetAddress;
import java.net.UnknownHostException;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.common.ResponseCallback;
import com.refinitiv.eta.examples.common.DirectoryHandler;
import com.refinitiv.eta.examples.common.DictionaryHandler;
import com.refinitiv.eta.examples.common.LoginHandler;
import com.refinitiv.eta.examples.common.StreamIdWatchList;
import com.refinitiv.eta.examples.common.SymbolListHandler;
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
 * <p>
 * This is a main class to run ETA Consumer application. The purpose of this
 * application is to consume or post content between an OMM consumer and OMM
 * provider. It is a single-threaded client application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * This class is responsible for the following:
 * <ul>
 * <li>Initialize and set command line options.
 * <li>Create a {@link ChannelSession ETA Channel Session}.
 * <li>Create Handler instances to handle Login, Directory, Dictionary,
 * MarketPrice, MarketByOrder, MarketByPrice, and YieldCurve requests and responses.
 * <li>Create handlers for posting and symbol list request, responses.
 * <li>If the dictionary is found in the directory of execution, then it is
 * loaded directly from the file. However, the default configuration for this
 * application is to request the dictionary from the provider.
 * <li>Connect to the provider, login, request source directory, request
 * dictionary if not loaded from file, send item requests for specified domain
 * types, and process responses (refresh, status, update, close).
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
 * <H2>Setup Environment</H2>
 * <p>
 * The RDMFieldDictionary and enumtype.def files could be located in the
 * directory of execution or this application will request dictionary from
 * provider.
 * <p>
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runConsumer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runConsumer -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-h Server host name. Default is <i>localhost</i>.
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-uname Login user name. Default is system user name.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-mp Market Price domain item name. Default is <i>TRI.N</i>. The user can
 * specify multiple -mp instances, where each occurrence is associated with a
 * single item. For example, specifying -mp TRI -mp GOOG will provide content
 * for two MarketPrice items.
 * <li>-mpps Market Price domain item name on private stream. No default. The
 * user can specify multiple -mpps instances, where each occurrence is
 * associated with a single item.
 * <li>-mbo Market By Order domain item name. No default. The user can specify
 * multiple -mbo instances, where each occurrence is associated with a single
 * item.
 * <li>-mbops Market By Order domain item name on private stream. No default.
 * The user can specify multiple -mbops instances, where each occurrence is
 * associated with a single item.
 * <li>-mbp market By Price domain item name. No default. The user can specify
 * multiple -mbp instances, where each occurrence is associated with a single
 * item.
 * <li>-mbpps market By Price domain item name on private stream. No default.
 * The user can specify multiple -mbpps instances, where each occurrence is
 * associated with a single item.
 * <li>-yc Yield Curve domain item name. No default. The user can specify
 * multiple -yc instances, where each occurrence is associated with a
 * single item.
 * <li>-ycps Yield Curve domain item name on private stream. No default. The
 * user can specify multiple -ycps instances, where each occurrence is
 * associated with a single item.
 * <li>-view viewFlag. Default is false. If true, each request will use a basic
 * dynamic view.
 * <li>-post postFlag. Default is false. If true, the application will attempt
 * to send post messages on the first requested Market Price item (i.e.,
 * on-stream).
 * <li>-offpost offPostFlag. Default is false. If true, the application will
 * attempt to send post messages on the login stream (i.e., off-stream).
 * <li>-publisherInfo publisherInfoFlag. Default is false. If true, the application will
 * attempt to send post messages with publisher ID and publisher address.
 * <li>-snapshot snapShotFlag. If true, each request will be non-streaming (i.e.
 * snapshot). Default is false (i.e. streaming requests).
 * <li>-sl symbolListName. symbolListName is optional without a default. If -sl
 * is specified without a symbolListName the itemList from the source directory
 * response will be used.
 * <li>-x Provides XML tracing of messages.
 * <li>-connectionType. Connection Type used (Socket, http, or encrypted).
 * Default is <i>Socket</i>.
 * <li>-runtime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * <li>-proxy proxyFlag. if provided, the application will attempt
 * to make an http or encrypted connection through a proxy server (if
 * connectionType is set to http or encrypted).
 * <li>-ph Proxy host name.
 * <li>-pp Proxy port number.
 * <li>-plogin User name on proxy server.
 * <li>-ppasswd Password on proxy server. 
 * <li>-pdomain Proxy Domain.
 * <li>-krbfile Proxy KRB file. 
 * <li>-keyfile keystore file for encryption.
 * <li>-keypasswd keystore password for encryption.
 * <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
 * <li>-ax Specifies the Authentication Extended information.
 * <li>-aid Specifies the Application ID.
 * 
 * </ul>
 * 
 * @see DictionaryHandler
 * @see DirectoryHandler
 * @see LoginHandler
 * @see PostHandler
 * @see MarketPriceHandler
 * @see MarketByOrderHandler
 * @see MarketByPriceHandler
 * @see SymbolListHandler
 * @see YieldCurveHandler
 */
public class Consumer implements ResponseCallback
{
    private static final int CONSUMER_CONNECTION_RETRY_TIME = 15; // seconds
    
    private ChannelSession channelSession;
    private ChannelInfo channelInfo;
    private boolean postInit = false;
    private LoginHandler loginHandler;
    private DirectoryHandler srcDirHandler;
    private DictionaryHandler dictionaryHandler;
    private MarketPriceHandler marketPriceHandler;
    private MarketByOrderHandler marketByOrderHandler;
    private MarketByPriceHandler marketByPriceHandler;
    private PostHandler postHandler;
    private SymbolListHandler symbolListHandler;
    private YieldCurveHandler yieldCurveHandler;
 
    private boolean shouldOffStreamPost = false;
    private boolean shouldOnStreamPost = false;   
    private Buffer postItemName;
    private StreamIdWatchList itemWatchList;

    // indicates if requested service is up
    private boolean requestsSent;
    
    private long runtime;
    
    // default server host name
    private static final String defaultSrvrHostname = "localhost";
    
    // default server port number
    private static final String defaultSrvrPortNo = "14002";
    
    // default service name
    private static final String defaultServiceName = "DIRECT_FEED";
    
    // default item name
    private static final String defaultItemName = "TRI.N";
    
    // consumer run-time in seconds
    private static final int defaultRuntime = 600;

    private Error error;    // error information

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();
    
    // private streams items are non-recoverable, it is not sent again after recovery
    private boolean mppsRequestSent = false;
    private boolean mbopsRequestSent = false;
    private boolean mbppsRequestSent = false;
    private boolean ycpsRequestSent = false;

    private ConnectOptions _tunnelingConnectOpts;
    private String _localHostName = null;
    private String _localIPaddress = null;

    public Consumer()
    {
        channelInfo = TransportFactory.createChannelInfo();
        itemWatchList = new StreamIdWatchList();
        loginHandler = new LoginHandler();
        srcDirHandler = new DirectoryHandler();
        dictionaryHandler = new DictionaryHandler();
        marketPriceHandler = new MarketPriceHandler(itemWatchList);
        marketByOrderHandler = new MarketByOrderHandler(itemWatchList);
        marketByPriceHandler = new MarketByPriceHandler(itemWatchList);
        postHandler = new PostHandler();
        symbolListHandler = new SymbolListHandler();
        yieldCurveHandler = new YieldCurveHandler(itemWatchList);
        channelSession = new ChannelSession();
        postItemName = CodecFactory.createBuffer();
        error = TransportFactory.createError();

        _tunnelingConnectOpts = TransportFactory.createConnectOptions();
    }

    /**
     * Main loop that handles channel connection, login requests, reading and
     * processing responses from channel.
     */
    public void run()
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
                    "  Ping Timeout: %d\n",
                              channelInfo.maxFragmentSize(),
                              channelInfo.maxOutputBuffers(), channelInfo.guaranteedOutputBuffers(),
                              channelInfo.numInputBuffers(),
                              channelInfo.sysSendBufSize(), channelInfo.sysRecvBufSize(),
                              channelInfo.pingTimeout()
                    );
            System.out.println( "  Client To Server Pings: " + channelInfo.clientToServerPings() +
					"\n  Server To Client Pings: " + channelInfo.serverToClientPings() +
					"\n");
            System.out.printf("  Connected component version: ");
            
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
            channelSession.isLoginReissue = false;
            if (loginHandler.sendRequest(channelSession, error) != CodecReturnCodes.SUCCESS)
            {
            	System.err.println("Error sending Login request, exit.");
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
        int ret = 0;
        while (System.currentTimeMillis() < runtime)
        {
            // read until no more to read
            ret = channelSession.read(pingHandler, this, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Read failure, " + error.text());
                System.out.println("Consumer exits...");
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
                System.out.println("Consumer exits...");
                System.exit(TransportReturnCodes.FAILURE);
            }

            handlePosting();

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
        }
    }

    // on and off stream posting if enabled
    private void handlePosting()
    {
        if (postHandler.enableOnstreamPost())
        {
            postItemName.clear();
            int postStreamId = marketPriceHandler.getFirstItem(postItemName);
            if (postStreamId == 0 || postItemName.length() == 0)
            {
                return;
            }
            postHandler.streamId(postStreamId);
            postHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
            postHandler.dictionary(dictionaryHandler.dictionary());
            postHandler.postItemName().data(postItemName.data(), postItemName.position(), postItemName.length());

            int ret = postHandler.handlePosts(channelSession, error);
            if (ret < CodecReturnCodes.SUCCESS)
                System.out.println("Error posting onstream: " + error.text());
        }
        if (postHandler.enableOffstreamPost())
        {
            postHandler.streamId(loginHandler.refreshInfo().streamId());
            postHandler.postItemName().data("OFFPOST");
            postHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
            postHandler.dictionary(dictionaryHandler.dictionary());
            int ret = postHandler.handlePosts(channelSession, error);
            if (ret < CodecReturnCodes.SUCCESS)
                System.out.println("Error posting offstream: " + error.text());
        }
    }

    /**
     * Initializes consumer application.
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments. Initializing all domain
     * handlers. Loading dictionaries from file.
     * 
     * @param args
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
            System.out.println("Consumer exits...");
            System.exit(CodecReturnCodes.FAILURE);
        }
        if (args.length == 0 || (!isMarketPriceArgSpecified() && !isYieldCurveArgSpecified()))
        {
            CommandLine.parseArgs(new String[] { "-mp", defaultItemName });
        }

        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("Consumer initializing...");

        try
        {
            runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;
        }
        catch (NumberFormatException ile)
        {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.out.println("Consumer exits...");
            System.exit(-1);
        }
        shouldOffStreamPost = CommandLine.booleanValue("offpost");
        // this application requires at least one market price item to be
        // requested for on-stream posting to be performed
        shouldOnStreamPost = CommandLine.booleanValue("post");
        if (shouldOnStreamPost)
        {
            if (!CommandLine.hasArg("mp"))
            {
                System.out.println("\nPosting will not be performed as no Market Price items were requested");
                shouldOnStreamPost = false;
            }
        }
        postHandler.enableOnstreamPost(shouldOnStreamPost);

        String value = CommandLine.value("publisherInfo");
        if (value!= null) 
        {
        	String [] pieces = value.split(",");
        		
        	if( pieces.length > 1 )
            {
        		String publisherId = pieces[0];
            		
        		String publisherAddress = pieces[1];
            	
            	postHandler.setPublisherInfo(publisherId, publisherAddress);            		
            }  
        	else
        	{
                System.err.println("Error loading command line arguments for publisherInfo [id, address]:\t");
                System.out.println("Consumer exits...");
                System.exit(CodecReturnCodes.FAILURE);        		        		        		
        	}
        }
                       
        loginHandler.userName(CommandLine.value("uname"));
        loginHandler.authenticationToken(CommandLine.value("at"));
        loginHandler.authenticationExtended(CommandLine.value("ax"));
        loginHandler.applicationId(CommandLine.value("aid"));

        // set service name in directory handler
        srcDirHandler.serviceName(CommandLine.value("s"));
        marketPriceHandler.snapshotRequest(CommandLine.booleanValue("snapshot"));
        marketByOrderHandler.snapshotRequest(CommandLine.booleanValue("snapshot"));
        marketByPriceHandler.snapshotRequest(CommandLine.booleanValue("snapshot"));
        yieldCurveHandler.snapshotRequest(CommandLine.booleanValue("snapshot"));
        symbolListHandler.snapshotRequest(CommandLine.booleanValue("snapshot"));

        marketPriceHandler.viewRequest(CommandLine.booleanValue("view"));

        if (channelSession.initTransport(false, error) < CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error initialize transport.");
            System.out.println("Consumer exits...");
            System.exit(error.errorId());
        }
        
        String connectionType = CommandLine.value("connectionType");
        if (connectionType.equals("encrypted"))
        {
            channelSession.setConnectionType(ConnectionTypes.ENCRYPTED);
            _tunnelingConnectOpts.tunnelingInfo().tunnelingType("encrypted");
            // build tunneling and credentials config and pass to channelSession
            setEncryptedConfiguration(_tunnelingConnectOpts);
        }
        else if (connectionType.equals("http"))
        {
            channelSession.setConnectionType(ConnectionTypes.HTTP);
            _tunnelingConnectOpts.tunnelingInfo().tunnelingType("http");
            // build http and credentials config and pass to channelSession
            setHTTPconfiguration(_tunnelingConnectOpts);
        }

        // load dictionary
        dictionaryHandler.loadDictionary();

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(dictionaryHandler.dictionary());
        }
    }

    private void setEncryptedConfiguration(ConnectOptions options)
    {
        if (CommandLine.booleanValue("proxy"))
        {
            options.tunnelingInfo().HTTPproxy(true);
            
            String proxyHost = CommandLine.value("ph");
            if ( proxyHost == null)
            {
            	System.err.println("Error: Proxy hostname not provided.");  
            	System.out.println("Consumer exits...");
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }           
            String proxyPort = CommandLine.value("pp");
            if ( proxyPort == null)
            {
            	System.err.println("Error: Proxy port number not provided.");
            	 System.out.println("Consumer exits...");
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }                                   
                        
            options.tunnelingInfo().HTTPproxyHostName(proxyHost);  
            try
            {
            	options.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort));                   	
            }
            catch(Exception e)
            {
               	System.err.println("Error: Proxy port number not provided."); 
                System.out.println("Consumer exits...");
            	System.exit(CodecReturnCodes.FAILURE);    
            }
        }
        
        String keyFile = CommandLine.value("keyfile");
        if (keyFile == null)
        {
        	System.err.println("Error: Keystore file is missing for connectionType of encryption.");
        	System.out.println("Consumer exits...");
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }
        
        String keyPasswd = CommandLine.value("keypasswd");
        if (keyPasswd == null)
        {
        	System.err.println("Error: Keystore Password is missing for connectionType of encryption.");
        	System.out.println("Consumer exits...");
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }
                
        options.tunnelingInfo().KeystoreFile(keyFile);
        options.tunnelingInfo().KeystorePasswd(keyPasswd);        
        channelSession.tunnelingConnectOptions(_tunnelingConnectOpts);

        // credentials
        if (options.tunnelingInfo().HTTPproxy())
        {
            setCredentials(options);
            channelSession.credentialsConnectOptions(options);
        }
    }

    private void setHTTPconfiguration(ConnectOptions options)
    {
        if (CommandLine.booleanValue("proxy"))
        {
            options.tunnelingInfo().HTTPproxy(true);

            String proxyHost = CommandLine.value("ph");
            if ( proxyHost == null)
            {
            	System.err.println("Error: Proxy hostname not provided."); 
            	System.out.println("Consumer exits...");
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }           
            String proxyPort = CommandLine.value("pp");
            if ( proxyPort == null)
            {
            	System.err.println("Error: Proxy port number not provided."); 
            	System.out.println("Consumer exits...");
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }                       
                        
            options.tunnelingInfo().HTTPproxyHostName(proxyHost);
            try
            {
            	options.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort));            
            }
            catch(Exception e)
            {
            	System.err.println("Error: Proxy port number not provided."); 
            	System.out.println("Consumer exits...");
            	System.exit(CodecReturnCodes.FAILURE);       
            }
        }
        channelSession.tunnelingConnectOptions(_tunnelingConnectOpts);

        // credentials
        if (options.tunnelingInfo().HTTPproxy())
        {
            setCredentials(options);
            channelSession.credentialsConnectOptions(options);
        }
    }

    /*
     * For BASIC authentication we need: HTTPproxyUsername, HTTPproxyPasswd For
     * NTLM authentication we need: HTTPproxyUsername, HTTPproxyPasswd,
     * HTTPproxyDomain, HTTPproxyLocalHostname For Negotiate/Kerberos we need:
     * HTTPproxyUsername, HTTPproxyPasswd, HTTPproxyDomain,
     * HTTPproxyKRB5configFile
     */
    private void setCredentials(ConnectOptions options)
    {
    	String proxyUsername = CommandLine.value("plogin");
        if ( proxyUsername == null)
        {
        	System.err.println("Error: Proxy username not provided.");  
        	System.out.println("Consumer exits...");
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }         	
        String proxyPasswd = CommandLine.value("ppasswd");
        if ( proxyPasswd == null)
        {
        	System.err.println("Error: Proxy password not provided.");  
        	System.out.println("Consumer exits...");
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }     
        String proxyDomain = CommandLine.value("pdomain");
        if ( proxyDomain == null)
        {
        	System.err.println("Error: Proxy domain not provided.");  
        	System.out.println("Consumer exits...");
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }             
    	    	
        options.credentialsInfo().HTTPproxyUsername(proxyUsername);
        options.credentialsInfo().HTTPproxyPasswd(proxyPasswd);
        options.credentialsInfo().HTTPproxyDomain(proxyDomain);
        
        try
        {
            _localIPaddress = InetAddress.getLocalHost().getHostAddress();
            _localHostName = InetAddress.getLocalHost().getHostName();
        }
        catch (UnknownHostException e)
        {
            _localHostName = _localIPaddress;
        }
        options.credentialsInfo().HTTPproxyLocalHostname(_localHostName);
        
        String proxyKrbfile = CommandLine.value("krbfile");
        if (proxyKrbfile == null)
        {
        	System.err.println("Error: Proxy krbfile not provided.");  
        	System.out.println("Consumer exits...");
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }                              
        options.credentialsInfo().HTTPproxyKRB5configFile(proxyKrbfile);        
    }

    private boolean isMarketPriceArgSpecified()
    {
        return (CommandLine.hasArg("mp") ||
                CommandLine.hasArg("mpps") ||
                CommandLine.hasArg("mbp") ||
                CommandLine.hasArg("mbpps") ||
                CommandLine.hasArg("mbo") ||
                CommandLine.hasArg("mbops") || CommandLine.hasArg("sl"));
    }

    private boolean isYieldCurveArgSpecified()
    {
        return (CommandLine.hasArg("yc") ||
                CommandLine.hasArg("ycps"));
    }    
    
    /**
     * Call back method to process responses from channel. Processing responses
     * consists of performing a high level decode of the message and then
     * calling the applicable specific method for further processing.
     * chnl - The channel of the response
     * buffer - The message buffer containing the response.
     */
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
        	System.out.println("Consumer exits...");
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
                checkAndInitPostingSupport();
                break;
            case DomainTypes.MARKET_PRICE:
                processMarketPriceResp(responseMsg, dIter);

                break;
            case DomainTypes.MARKET_BY_ORDER:
                processMarketByOrderResp(responseMsg, dIter);

                break;
            case DomainTypes.MARKET_BY_PRICE:
                processMarketByPriceResp(responseMsg, dIter);
                break;
            case DomainTypes.SYMBOL_LIST:
                processSymbolListResp(responseMsg, dIter);
                break;
            case DomainTypes.YIELD_CURVE:
            	processYieldCurveResp(responseMsg, dIter);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }

    private void processSymbolListResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (symbolListHandler.processResponse(responseMsg, dIter, dictionaryHandler.dictionary()) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.err.println("Error processing response, exit.");
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processMarketByPriceResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (marketByPriceHandler.processResponse(responseMsg, dIter, dictionaryHandler
                .dictionary(), error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.err.println("Error processing marketByPrice response, exit.");
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processMarketByOrderResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (marketByOrderHandler.processResponse(responseMsg, dIter, dictionaryHandler
                .dictionary(), error) != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error processing marketByOrder response, exit.");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processMarketPriceResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (marketPriceHandler.processResponse(responseMsg, dIter, dictionaryHandler
                .dictionary(), error) != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error processing marketPrice response, exit.");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processDictionaryResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        if (dictionaryHandler.processResponse(chnl.channel(), responseMsg, dIter, error) != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error processing dictionary response, exit.");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (dictionaryHandler.isFieldDictionaryLoaded() &&
                dictionaryHandler.isEnumTypeDictionaryLoaded() &&
                responseMsg.msgClass() == MsgClasses.REFRESH)
        {
            System.out.println("Dictionary ready, requesting item(s)...");

            itemWatchList.clear();
            sendMPRequests(chnl);
            sendMBORequests(chnl);
            sendMBPRequests(chnl);
            sendSymbolListRequests(chnl);
            sendYieldCurveRequests(chnl);
            postHandler.enableOffstreamPost(shouldOffStreamPost);
            postHandler.enableOnstreamPost(shouldOnStreamPost);
        }
    }

    private void processSourceDirectoryResp(ChannelSession chnl, Msg responseMsg)
    {
        int ret = srcDirHandler.processResponse(chnl.channel(), responseMsg, dIter, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error processing source directory response, exit.");
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
            // service not up or
            // previously up service went down
            requestsSent = false;

            System.out.println("Requested service '" + CommandLine.value("s") + "' not up. Waiting for service to be up...");
        }
    }

    private void processYieldCurveResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (yieldCurveHandler.processResponse(responseMsg, dIter, dictionaryHandler
                .dictionary(), error) != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error processing yieldCurve response, exit.");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
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
            if (srcDirHandler.serviceInfo().info().dictionariesProvidedList().size() > 0)
            {
                sendDictionaryRequests(chnl);
            }
            else
            {
                System.out.println("\nDictionary download not supported by the indicated provider");
            }
            return;
        }

        itemWatchList.clear();
        sendMPRequests(chnl);
        sendMBORequests(chnl);
        sendMBPRequests(chnl);
        sendSymbolListRequests(chnl);
        sendYieldCurveRequests(chnl);

        checkAndInitPostingSupport();

        postHandler.enableOffstreamPost(shouldOffStreamPost);
        postHandler.enableOnstreamPost(shouldOnStreamPost);

        requestsSent = true;
    }

    private void sendDictionaryRequests(ChannelSession chnl)
    {
        dictionaryHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
        if (dictionaryHandler.sendRequests(chnl, error) != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error sending dictionary request, exit.");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void sendSymbolListRequests(ChannelSession chnl)
    {
        if (!CommandLine.hasArg("sl"))
            return;

        if (!srcDirHandler.serviceInfo().checkHasInfo())
        {
            System.err.println("Error sending symbolList request, exit.");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        Service.ServiceInfo info = srcDirHandler.serviceInfo().info();

        if (info.qosList().size() > 0)
        {
            Qos qos = info.qosList().get(0);
            symbolListHandler.qos().dynamic(qos.isDynamic());
            symbolListHandler.qos().rate(qos.rate());
            symbolListHandler.qos().timeliness(qos.timeliness());
        }
        else
        {
            symbolListHandler.qos().dynamic(false);
            symbolListHandler.qos().rate(QosRates.TICK_BY_TICK);
            symbolListHandler.qos().timeliness(QosTimeliness.REALTIME);
        }
        symbolListHandler.capabilities().addAll(info.capabilitiesList());
        symbolListHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
        String cmdSLName = CommandLine.value("sl");
        if (cmdSLName == null)
        {
            symbolListHandler.symbolListName().data(info.itemList().data(), info.itemList().position(), info.itemList().length());
        }
        else
        {
            symbolListHandler.symbolListName().data(cmdSLName);
        }
        if (symbolListHandler.sendRequest(chnl, error) != CodecReturnCodes.SUCCESS)
        {
        	System.out.println(error.text());
        	System.err.println("Error sending symbolList request, exit...");
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void sendMBPRequests(ChannelSession chnl)
    {
        if (marketByPriceHandler.sendItemRequests(chnl, CommandLine.values("mbp"), false, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
        {
        	System.err.println("Error sending MBP request, exit.");
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (CommandLine.hasArg("mbpps") && !mbppsRequestSent)
        {
            if (marketByPriceHandler.sendItemRequests(chnl, CommandLine.values("mbpps"), true, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
            {
                System.out.println(error.text());
                closeChannel();
            	System.out.println("Consumer exits...");
                System.exit(TransportReturnCodes.FAILURE);
            }
            mbppsRequestSent = true;
        }
    }

    private void sendMBORequests(ChannelSession chnl)
    {
        if (marketByOrderHandler.sendItemRequests(chnl, CommandLine.values("mbo"), false, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
        {
        	System.err.println("Error sending MBO request, exit.");
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (CommandLine.hasArg("mbops") && !mbopsRequestSent)
        {
            if (marketByOrderHandler.sendItemRequests(chnl, CommandLine.values("mbops"), true, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
            {
            	System.err.println("Error sending MBO request, exit.");
                System.out.println(error.text());
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
            mbopsRequestSent = true;
        }
    }

    private void sendMPRequests(ChannelSession chnl)
    {
        if (marketPriceHandler.sendItemRequests(chnl, CommandLine.values("mp"), false, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
        {
        	System.err.println("Error sending MP request, exit.");
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (CommandLine.hasArg("mpps") && !mppsRequestSent)
        {
            if (marketPriceHandler.sendItemRequests(chnl, CommandLine.values("mpps"), true, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
            {
            	System.err.println("Error sending MP request, exit...");
                System.out.println(error.text());
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
            mppsRequestSent = true;
        }
    }

    private void sendYieldCurveRequests(ChannelSession chnl)
    {
        if (yieldCurveHandler.sendItemRequests(chnl, CommandLine.values("yc"), false, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
        {
        	System.err.println("Error sending YC request, exit.");
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (CommandLine.hasArg("ycps") && !ycpsRequestSent)
        {
            if (yieldCurveHandler.sendItemRequests(chnl, CommandLine.values("ycps"), true, loginHandler.refreshInfo(), srcDirHandler.serviceInfo(), error) != CodecReturnCodes.SUCCESS)
            {
            	System.err.println("Error sending YC request, exit...");
                System.out.println(error.text());
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
            ycpsRequestSent = true;
        }
    }    
    
    private boolean isDictionariesLoaded()
    {
        return dictionaryHandler.isFieldDictionaryLoaded()
                && dictionaryHandler.isEnumTypeDictionaryLoaded();
    }

    private void checkAndInitPostingSupport()
    {
        if (!(shouldOnStreamPost || shouldOffStreamPost) || postInit)
            return;

        // set up posting if its enabled 
        
        // ensure that provider supports posting - if not, disable posting
        if (!loginHandler.refreshInfo().checkHasFeatures() ||
                !loginHandler.refreshInfo().features().checkHasSupportPost() ||
                loginHandler.refreshInfo().features().supportOMMPost() == 0)
        {
            // provider does not support posting, disable it
            shouldOffStreamPost = false;
            shouldOnStreamPost = false;
            postHandler.enableOnstreamPost(false);
            postHandler.enableOffstreamPost(false);
            System.out.println("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
            return;
        }

        // This sets up our basic timing so post messages will be sent periodically
        postHandler.initPostHandler();

        // posting has been initialized
        postInit = true;
    }

    private void processLoginResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        int ret = loginHandler.processResponse(responseMsg, dIter, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
        	System.err.println("Error processing Login response, exit.");
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        // Handle login states
        ConsumerLoginState loginState = loginHandler.loginState();
        if (loginState == ConsumerLoginState.OK_SOLICITED)
        {
        	if (!chnl.isLoginReissue)
        	{
                //APIQA
                System.err.println("....ReconnectClient....");
                chnl.channel().reconnectClient(error);
               //END APIQA
            ret = srcDirHandler.sendRequest(chnl, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error sending directory request: " + error.text());
                closeChannel();
            	System.out.println("Consumer exits...");
                System.exit(TransportReturnCodes.FAILURE);
            }
        }
        }
        else if (loginState == ConsumerLoginState.CLOSED)
        {
            System.out.println(error.text());
            closeChannel();
        	System.out.println("Consumer exits...");
            System.exit(TransportReturnCodes.FAILURE);
        }
        else if (loginState == ConsumerLoginState.CLOSED_RECOVERABLE)
        {
            ret = channelSession.recoverConnection(error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error recovering connection: " + error.text());
            	System.out.println("Consumer exits...");
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
                	System.out.println("Consumer exits...");
                    System.exit(TransportReturnCodes.FAILURE);
                }

            	if (!chnl.isLoginReissue)
            	{
                ret = srcDirHandler.sendRequest(chnl, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error sending directory request: " + error.text());
                    closeChannel();
                	System.out.println("Consumer exits...");
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
            }
            // login suspect from single-open provider: provider handles
            // recovery. consumer does nothing in this case.
        }
        
		// get login reissue time from authenticationTTReissue
        if (responseMsg.msgClass() == MsgClasses.REFRESH &&
        	loginHandler.refreshInfo().checkHasAuthenticationTTReissue())
        {
			chnl.loginReissueTime = loginHandler.refreshInfo().authenticationTTReissue() * 1000;
			chnl.canSendLoginReissue = true;
        }
    }

    private void closeDictAndItemStreams()
    {
        // have offstream posting post close status
        postHandler.streamId(loginHandler.refreshInfo().streamId());
        postHandler.postItemName().data("OFFPOST");
        postHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
        postHandler.dictionary(dictionaryHandler.dictionary());
        postHandler.closeOffStreamPost(channelSession, error);

        // close item streams if opened
        marketPriceHandler.closeStreams(channelSession, error);
        marketByOrderHandler.closeStreams(channelSession, error);
        marketByPriceHandler.closeStreams(channelSession, error);
        symbolListHandler.closeStream(channelSession, error);
        yieldCurveHandler.closeStreams(channelSession, error);

        // close dictionary streams if opened
        dictionaryHandler.closeStreams(channelSession, error);
    }

    /**
     * Closes all streams for the consumer.
     */
    public void uninitialize()
    {
        System.out.println("Consumer unitializing and exiting...");
        if (channelSession.channel() == null)
        {
            channelSession.uninit(error);
        	System.out.println("Consumer exits...");
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
            System.out.println("Flush() failed with return code " + retval + "- <" + error
                    .text() + ">");
        }
    }

    private void closeChannel()
    {
        channelSession.uninit(error);
    }

    private static void addCommandLineArgs()
    {
        CommandLine.programName("Consumer");
        CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain.");
        CommandLine.addOption("mpps", "For each occurrence, requests item using Market Price domain on private stream. Default is no market price private stream requests.");
        CommandLine.addOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
        CommandLine.addOption("mbops", "For each occurrence, requests item using Market By Order domain on private stream. Default is no market by order private stream requests.");
        CommandLine.addOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");
        CommandLine.addOption("mbpps", "For each occurrence, requests item using Market By Price domain on private stream. Default is no market by price private stream requests.");
        CommandLine.addOption("yc", "For each occurrence, requests item using Yield Curve domain. Default is no yield curve requests.");
        CommandLine.addOption("ycps", "For each occurrence, requests item using Yield Curve domain on private stream. Default is no yield curve private stream requests.");
        CommandLine.addOption("view", "Specifies each request using a basic dynamic view. Default is false.");
        CommandLine.addOption("post", "Specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream). Default is false.");
        CommandLine.addOption("offpost", "Specifies that the application should attempt to send post messages on the login stream (i.e., off-stream).");
        CommandLine.addOption("publisherInfo", "Specifies that the application should add user provided publisher Id and publisher ipaddress when posting");       
        CommandLine.addOption("snapshot", "Specifies each request using non-streaming. Default is false(i.e. streaming requests.)");
        CommandLine.addOption("sl", "Requests symbol list using Symbol List domain. (symbol list name optional). Default is no symbol list requests.");
        CommandLine.addOption("h", defaultSrvrHostname, "Server host name");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number");
        CommandLine.addOption("i", (String)null, "Interface name");
        CommandLine.addOption("s", defaultServiceName, "Service name");
        CommandLine.addOption("uname", "Login user name. Default is system user name.");
        CommandLine.addOption("connectionType", "Socket", "Specifies the connection type that the connection should use. Possible values are: 'Socket', 'http', 'encrypted'");

        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("x", "Provides XML tracing of messages.");
        
        CommandLine.addOption("proxy", "Specifies that the application will make a tunneling connection (http or encrypted) through a proxy server, default is false");         
        CommandLine.addOption("ph", "", "Proxy server host name");
        CommandLine.addOption("pp", "", "Proxy port number");         
        CommandLine.addOption("plogin", "", "User Name on proxy server");
        CommandLine.addOption("ppasswd", "", "Password on proxy server");
        CommandLine.addOption("pdomain", "", "Proxy server domain");
        CommandLine.addOption("krbfile", "", "KRB File location and name");
        CommandLine.addOption("keyfile", "", "Keystore file location and name");
        CommandLine.addOption("keypasswd", "", "Keystore password");        
        CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
        CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
        CommandLine.addOption("aid", "", "Specifies the Application ID.");
    }

    public static void main(String[] args) throws Exception
    {
        Consumer consumer = new Consumer();
        consumer.init(args);
        consumer.run();
        consumer.uninitialize();
        System.out.println("Consumer done");
    }
}
