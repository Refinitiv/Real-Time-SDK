/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2024-2025 LSEG. All rights reserved.    --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.consumer;

import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.examples.common.CommandLineParser;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArg;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArgsParser;

import java.util.List;

/**
 * Command line parser for the Value Add consumer application.
 */
class ConsumerCmdLineParser implements CommandLineParser
{
	private final ConnectionArgsParser connectionArgsParser = new ConnectionArgsParser();
	private String userName;
	private String passwd;
	//API QA
	private String clientId1;
	private String clientSecret;
	private String clientId2;
	//END API QA
	private String jwkFile = "";
	private String tokenScope = "";
	private String audience = "";
	private boolean enableView;
	private boolean enablePost;
	private boolean enableOffpost;
	private boolean enableSnapshot;
	private String publisherId;
	private String publisherAddress;
	private int runtime = 600; // default runtime is 600 seconds
	private boolean enableXmlTracing;
	private boolean enableEncrypted;
	private boolean enableHttp;
	private boolean enableWebsocket;
	private boolean enableProxy;
	private boolean enableSessionMgnt;
	private int encryptedConnectionType;
	private String proxyHostname;
	private String proxyPort;
	private String proxyUsername = "";
	private String proxyPasswd = "";
	private String proxyDomain = "";
	private String krbFile = "";
	private String tokenURL_V1 = "";
	private String tokenURL_V2 = "";
	private String serviceDiscoveryURL  = "";
	private String serviceDiscoveryLocation = "";
	private String keystoreFile;
	private String keystorePasswd;
	private String securityProvider;
	private boolean cacheOption;
	private int cacheInterval;
	private int statisticInterval;
	private String authenticationToken;
	private String authenticationExtended;
	private String applicationId;
	private boolean enableRtt;
	private boolean takeExclusiveSignOnControl = true;
	private String protocolList = "tr_json2";
	private boolean sendJsonConvError;
	private String securityProtocol;
	private String securityProtocolVersions;
	private boolean spTLSv12enable = false;
	private boolean spTLSv13enable = false;
	private String restProxyHostName;
	private String restProxyPort;
	private String restProxyUserName;
	private String restProxyPasswd;
	private String restProxyDomain;
	private String restProxyKrb5ConfigFile;

	// Preferred host options
	private boolean enablePH = false;
	private int preferredHostIndex;
	private int detectionTimeInterval;
	private String detectionTimeSchedule = "";

	// IOCtl options
	private int fallBackInterval;
	private int ioctlInterval;
	private boolean ioctlEnablePH = false;
	private int ioctlConnectListIndex;
	private int ioctlDetectionTimeInterval;
	private String ioctlDetectionTimeSchedule = "";

	@Override
	public boolean parseArgs(String[] args)
	{
		try
		{
			int argsCount = 0;
			boolean ioctlSet = false;

			while (argsCount < args.length)
			{
				if (connectionArgsParser.isStart(args, argsCount))
				{
					if ("-segmentedMulticast".equals(args[argsCount]))
					{
						// consumer does not currently handle multicast
						return false;
					}

					if ((argsCount = connectionArgsParser.parse(args, argsCount)) < 0)
					{
						// error
						System.out.println("\nError parsing connection arguments...\n");
						return false;
					}
				}
				else if ("-uname".equals(args[argsCount]))
				{
					userName = args[++argsCount];
					++argsCount;
				}
				else if ("-passwd".equals(args[argsCount]))
				{
					passwd = args[++argsCount];
					++argsCount;
				}
				else if ("-sessionMgnt".equals(args[argsCount]))
				{
					enableSessionMgnt = true;
					++argsCount;
				}
				//API QA
				else if ("-clientId1".equals(args[argsCount]))
				{
					clientId1 = args[++argsCount];
					++argsCount;
				}
				else if ("-clientSecret".equals(args[argsCount]))
				{
					clientSecret = args[++argsCount];
					++argsCount;
				}
				else if ("-clientId2".equals(args[argsCount]))
				{
					clientId2 = args[++argsCount];
					++argsCount;
				}
				// END API QA
				else if ("-jwkFile".equals(args[argsCount]))
				{
					jwkFile = args[++argsCount];
					++argsCount;
				}
				else if ("-tokenScope".equals(args[argsCount]))
				{
					tokenScope = args[++argsCount];
					++argsCount;
				}
				else if ("-audience".equals(args[argsCount]))
				{
					audience = args[++argsCount];
					++argsCount;
				}
				else if ("-view".equals(args[argsCount]))
				{
					enableView =  true;
					++argsCount;
				}
				else if ("-post".equals(args[argsCount]))
				{
					enablePost =  true;
					++argsCount;
				}
				else if ("-offpost".equals(args[argsCount]))
				{
					enableOffpost =  true;
					++argsCount;
				}
				else if ("-publisherInfo".equals(args[argsCount]))
				{
					String value = args[++argsCount];
					if (value!= null)
					{
						String [] pieces = value.split(",");

						if( pieces.length > 1 )
						{
							publisherId = pieces[0];

							publisherAddress = pieces[1];
						}
						else
						{
							System.out.println("Error loading command line arguments for publisherInfo [id, address]:\t");
							return false;
						}
					}
					++argsCount;
				}
				else if ("-snapshot".equals(args[argsCount]))
				{
					enableSnapshot =  true;
					++argsCount;
				}
				else if ("-runtime".equals(args[argsCount]))
				{
					runtime = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				else if ("-x".equals(args[argsCount]))
				{
					enableXmlTracing =  true;
					++argsCount;
				}
				else if ("-connectionType".equals(args[argsCount]))
				{
					// will overwrite connectionArgsParser's connectionList's connectionType based on the flag
					String connectionType = args[++argsCount];
					++argsCount;
					if (connectionType.equals("encrypted"))
					{
						enableEncrypted = true;
					}
					else if (connectionType.equals("http"))
					{
						enableHttp = true;
					}
					else if(connectionType.equals("websocket"))
					{
						enableWebsocket = true;
					}
				}
				else if ("-encryptedConnectionType".equals(args[argsCount]))
				{
					// will overwrite connectionArgsParser's connectionList's connectionType based on the flag
					String connectionType = args[++argsCount];
					++argsCount;
					if (connectionType.equals("socket"))
					{
						encryptedConnectionType = ConnectionTypes.SOCKET;
					}
					else if (connectionType.equals("http"))
					{
						encryptedConnectionType = ConnectionTypes.HTTP;
					}
					else if (connectionType.equals("websocket"))
					{
						encryptedConnectionType = ConnectionTypes.WEBSOCKET;
					}
				}
				else if ("-keyfile".equals(args[argsCount]))
				{
					keystoreFile = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-keypasswd".equals(args[argsCount]))
				{
					keystorePasswd = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-securityProvider".equals(args[argsCount]))
				{
					securityProvider = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-proxy".equals(args[argsCount]))
				{
					enableProxy =  true;
					++argsCount;
				}
				else if ("-restProxyHost".equals(args[argsCount]))
				{
					String restProxyHostName = args[++argsCount];
					++argsCount;
					this.restProxyHostName = restProxyHostName;
				}
				else if ("-restProxyPort".equals(args[argsCount]))
				{
					String restProxyPort = args[++argsCount];
					++argsCount;
					this.restProxyPort = restProxyPort;
				}
				else if ("-restProxyUserName".equals(args[argsCount]))
				{
					String restProxyUserName = args[++argsCount];
					++argsCount;
					this.restProxyUserName = restProxyUserName;
				}
				else if ("-restProxyPasswd".equals(args[argsCount]))
				{
					String restProxyPasswd = args[++argsCount];
					++argsCount;
					this.restProxyPasswd = restProxyPasswd;
				}
				else if ("-restProxyDomain".equals(args[argsCount]))
				{
					String restProxyDomain = args[++argsCount];
					++argsCount;
					this.restProxyDomain = restProxyDomain;
				}
				else if ("-restProxyKrb5ConfigFile".equals(args[argsCount]))
				{
					String restProxyKrb5ConfigFile = args[++argsCount];
					++argsCount;
					this.restProxyKrb5ConfigFile = restProxyKrb5ConfigFile;
				}
				else if ("-ph".equals(args[argsCount]))
				{
					proxyHostname = args[++argsCount];
					++argsCount;
				}
				else if ("-pp".equals(args[argsCount]))
				{
					proxyPort = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-plogin".equals(args[argsCount]))
				{
					proxyUsername = args[++argsCount];
					++argsCount;
				}
				else if ("-ppasswd".equals(args[argsCount]))
				{
					proxyPasswd = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-pdomain".equals(args[argsCount]))
				{
					proxyDomain = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-krbfile".equals(args[argsCount]))
				{
					krbFile = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-cache".equals(args[argsCount]))
				{
					cacheOption =  true;
					++argsCount;
				}
				else if ("-cacheInterval".equals(args[argsCount]))
				{
					cacheInterval = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				else if ("-statisticInterval".equals(args[argsCount]))
				{
					statisticInterval = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				else if ("-at".equals(args[argsCount]))
				{
					authenticationToken = args[++argsCount];
					++argsCount;
				}
				else if ("-ax".equals(args[argsCount]))
				{
					authenticationExtended = args[++argsCount];
					++argsCount;
				}
				else if ("-aid".equals(args[argsCount]))
				{
					applicationId = args[++argsCount];
					++argsCount;
				} else if ("-rtt".equals(args[argsCount])) {
					enableRtt = true;
					++argsCount;
				}
				else if ("-takeExclusiveSignOnControl".equals(args[argsCount]))
				{
					String takeExclusiveSignOnControlStr = args[++argsCount];

					if(takeExclusiveSignOnControlStr.equalsIgnoreCase("true"))
						takeExclusiveSignOnControl = true;
					else if (takeExclusiveSignOnControlStr.equalsIgnoreCase("false"))
						takeExclusiveSignOnControl = false;

					++argsCount;
				}
				else if ("-tokenURLV1".equals(args[argsCount]))
				{
					tokenURL_V1 = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-tokenURLV2".equals(args[argsCount]))
				{
					tokenURL_V2 = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-serviceDiscoveryURL".equals(args[argsCount]))
				{
					serviceDiscoveryURL = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-location".equals(args[argsCount]))
				{
					serviceDiscoveryLocation = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-pl".equals(args[argsCount])) {
					protocolList = args[++argsCount];
					++argsCount;
				}
				else if ("-sendJsonConvError".equals(args[argsCount]))
				{
					sendJsonConvError = true;
					++argsCount;
				}
				else if ("-spTLSv1.2".equals(args[argsCount]))
				{
					spTLSv12enable = true;
					++argsCount;
				}
				else if ("-spTLSv1.3".equals(args[argsCount]))
				{
					spTLSv13enable = true;
					++argsCount;
				}
				// Preferred host options
				else if ("-enablePH".equals(args[argsCount]))
				{
					enablePH = true;
					ioctlEnablePH = true;
					++argsCount;
				}
				else if ("-preferredHostIndex".equals(args[argsCount]))
				{
					preferredHostIndex = Integer.parseInt(args[++argsCount]);
					ioctlConnectListIndex = preferredHostIndex;
					++argsCount;
				}
				else if ("-detectionTimeInterval".equals(args[argsCount]))
				{
					detectionTimeInterval = Integer.parseInt(args[++argsCount]);
					ioctlDetectionTimeInterval = detectionTimeInterval;
					++argsCount;
				}
				else if ("-detectionTimeSchedule".equals(args[argsCount]))
				{
					detectionTimeSchedule = args[++argsCount];
					ioctlDetectionTimeSchedule = detectionTimeSchedule;
					++argsCount;
				}
				// IOCtl options
				else if ("-fallBackInterval".equals(args[argsCount]))
				{
					fallBackInterval = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				else if ("-ioctlInterval".equals(args[argsCount]))
				{
					ioctlInterval = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				else if ("-ioctlEnablePH".equals(args[argsCount]))
				{
					String ioctlEnablePHStr = args[++argsCount];

					if(ioctlEnablePHStr.equalsIgnoreCase("true"))
						ioctlEnablePH = true;
					else if (ioctlEnablePHStr.equalsIgnoreCase("false"))
						ioctlEnablePH = false;

					ioctlSet = true;
					++argsCount;
				}
				else if ("-ioctlConnectListIndex".equals(args[argsCount]))
				{
					ioctlConnectListIndex = Integer.parseInt(args[++argsCount]);
					ioctlSet = true;
					++argsCount;
				}
				else if ("-ioctlDetectionTimeInterval".equals(args[argsCount]))
				{
					ioctlDetectionTimeInterval = Integer.parseInt(args[++argsCount]);
					ioctlSet = true;
					++argsCount;
				}
				else if ("-ioctlDetectionTimeSchedule".equals(args[argsCount]))
				{
					ioctlDetectionTimeSchedule = args[++argsCount];
					ioctlSet = true;
					++argsCount;
				}
				else // unrecognized command line argument
				{
					System.out.println("\nUnrecognized command line argument: " + args[argsCount]);
					return false;
				}
			}

			if(ioctlSet && ioctlInterval <= 0) {
				System.out.println("\nioctlInterval should have a positive value if any ioctl parameters are specified");
				return false;
			}

			// Set TLS options (default sets both 1.2 and 1.3)
			if ((spTLSv12enable && spTLSv13enable) || (!spTLSv12enable && !spTLSv13enable))
			{
				securityProtocol = "TLS";
				securityProtocolVersions = "1.2,1.3";
			}
			else if (spTLSv12enable)
			{
				securityProtocol = "TLS";
				securityProtocolVersions = "1.2";
			}
			else if (spTLSv13enable)
			{
				securityProtocol = "TLS";
				securityProtocolVersions = "1.3";
			}
		}
		catch (Exception e)
		{
			System.out.println("\nInvalid command line arguments...");
			return false;
		}

		return true;
	}
	
	List<ConnectionArg> connectionList()
	{
		return connectionArgsParser.connectionList();
	}

	String userName()
	{
		return userName;
	}

	boolean enableView()
	{
		return enableView;
	}

	String passwd()
	{
		return passwd;
	}

	//API QA
	String clientId1()
	{
		return clientId1;
	}

	String clientSecret()
	{
		return clientSecret;
	}

	String clientId2()
	{
		return clientId2;
	}
	//END API QA
	
	String jwkFile()
	{
		return jwkFile;
	}
	
	String tokenURLV1()
	{
		return tokenURL_V1;
	}
	
	String tokenURLV2()
	{
		return tokenURL_V2;
	}
	String serviceDiscoveryURL()
	{
		return serviceDiscoveryURL;
	}
	String serviceDiscoveryLocation()
	{
		return serviceDiscoveryLocation;
	}
	String tokenScope()
	{
		return tokenScope;
	}
	
	String audience()
	{
		return audience;
	}
	
	boolean enableSessionMgnt()
	{
		return enableSessionMgnt;
	}

	boolean enablePost()
	{
		return enablePost;
	}

	boolean enableOffpost()
	{
		return enableOffpost;
	}

	String publisherId()
	{
		return publisherId;
	}

	String publisherAddress()
	{
		return publisherAddress;
	}

	boolean enableSnapshot()
	{
		return enableSnapshot;
	}

	int runtime()
	{
		return runtime;
	}

	boolean enableXmlTracing()
	{
		return enableXmlTracing;
	}

	boolean enableEncrypted()
	{
		return enableEncrypted;
	}

	boolean enableHttp()
	{
		return enableHttp;
	}

	boolean enableWebsocket()
	{
		return enableWebsocket;
	}

	boolean enableProxy()
	{
		return enableProxy;
	}

	String proxyHostname()
	{
		return proxyHostname;
	}

	String proxyPort()
	{
		return proxyPort;
	}

	String proxyUsername()
	{
		return proxyUsername;
	}

	String proxyPassword()
	{
		return proxyPasswd;
	}

	String proxyDomain()
	{
		return proxyDomain;
	}

	String krbFile()
	{
		return krbFile;
	}

	String keyStoreFile()
	{
		return keystoreFile;
	}

	String keystorePassword()
	{
		return keystorePasswd;
	}

	String securityProvider()
	{
		return securityProvider;
	}

	boolean cacheOption()
	{
		return cacheOption;
	}

	int cacheInterval()
	{
		return cacheInterval;
	}

	int statisticInterval()
	{
		return statisticInterval;
	}

	String authenticationToken()
	{
		return authenticationToken;
	}

	String authenticationExtended()
	{
		return authenticationExtended;
	}

	String applicationId()
	{
		return applicationId;
	}

	boolean enableRtt() {
		return enableRtt;
	}

	boolean takeExclusiveSignOnControl()
	{
		return takeExclusiveSignOnControl;
	}

	int encryptedConnectionType()
	{
		return encryptedConnectionType;
	}

	String protocolList()
	{
		return protocolList;
	}

	boolean sendJsonConvError()
	{ 
		return sendJsonConvError;
	}
	
	String securityProtocol()
	{
		return securityProtocol;
	}
	
	String securityProtocolVersions()
	{
		return securityProtocolVersions;
	}
	
	String restProxyHostName()
	{
		return restProxyHostName;
	}
	
	String restProxyPort()
	{
		return restProxyPort;
	}
	
	String restProxyUserName()
	{
		return restProxyUserName;
	}
	
	String restProxyPasswd()
	{
		return restProxyPasswd;
	}
	
	String restProxyDomain()
	{
		return restProxyDomain;
	}
	
	String restProxyKrb5ConfigFile()
	{
		return restProxyKrb5ConfigFile;
	}

	boolean enablePH()
	{
		return enablePH;
	}

	int preferredHostIndex()
	{
		return preferredHostIndex;
	}

	int detectionTimeInterval()
	{
		return detectionTimeInterval;
	}

	String detectionTimeSchedule()
	{
		return detectionTimeSchedule;
	}

	int fallBackInterval()
	{
		return fallBackInterval;
	}

	int ioctlInterval()
	{
		return ioctlInterval;
	}

	boolean ioctlEnablePH()
	{
		return ioctlEnablePH;
	}

	int ioctlConnectListIndex()
	{
		return ioctlConnectListIndex;
	}

	int ioctlDetectionTimeInterval()
	{
		return ioctlDetectionTimeInterval;
	}

	String ioctlDetectionTimeSchedule()
	{
		return ioctlDetectionTimeSchedule;
	}

	@Override
	public void printUsage()
	{
		System.out.println("Usage: Consumer or\nConsumer [-c <hostname>:<port>[,<hostname>:<port>,...] <service name> <domain>:<item name>,...] [-bc <hostname>:<port>] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>]" +
						   "\n -c specifies a connection to open and a list of items to request or use for queue messaging:\n" +
						   "\n     hostname:        Hostname of provider to connect to" +
						   "\n     port:            Port of provider to connect to" +
						   "\n     service:         Name of service to request items from on this connection" +
						   "\n     domain:itemName: Domain and name of an item to request" +
						   "\n         A comma-separated list of these may be specified." +
						   "\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)" +
						   "\n         The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)" +
						   "\n         Example Usage: -c localhost:14002,localhost:14003 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl" +
						   "\n           (for SymbolList requests, a name can be optionally specified)\n" +
						   "\n     -qSourceName (optional) specifies the source name for queue messages (if specified, configures consumer to receive queue messages)\n" +
						   "\n     -qDestName (optional) specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified)\n" +
						   "\n     -tunnel (optional) enables consumer to open tunnel stream and send basic text messages" +
						   "\n     -tsServiceName (optional) specifies the service name for queue messages (if not specified, the service name specified in -c/-tcp is used)\n" +
						   "\n     -tsAuth (optional) causes consumer to request authentication when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging.\n" +
						   "\n     -tsDomain (optional) specifes the domain a consumer uses when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging.\n" +
						   "\n -pl protocol list (defaults to rssl.rwf, tr_json2, rssl.json.v2)\n" +
							//API QA
							"\n -uname changes the username used when logging into the provider\n" +
							"\n -passwd changes the password used when logging into the provider\n" +
							"\n -clientId1 specifies clientID for a V1\n" +
							"\n -clientId2 specifies clientID for a V2\n" +
							"\n -clientSecret associated client secret for the client ID with the V2 login." +
							//END API QA
							"\n -sessionMgnt enables the session management in the Reactor\n" +
						   "\n -jwk Specifies the file containing the JWK encoded private key for V2 JWT logins.\n" +
						   "\n -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials. This is only used with V1 password credential logins(optional for V1 password credential logins).\n" +
						   "\n -tokenURLV1 specifies the URL for the V1 token generator(optional)." +
						   "\n -tokenURLV2 specifies the URL for the V2 token generator(optional)." +
						   "\n -serviceDiscoveryURL specifies the RDP Service Discovery URL to override the default value.\n" +
						   "\n -location specifies location/region when dogin service discovery.\n" +
						   "\n -view specifies each request using a basic dynamic view\n" +
						   "\n -restProxyHost specifies the REST proxy host name. Used for REST requests only for service discovery and authentication.\n" +
						   "\n -restProxyPort specifies the REST proxy port. Used for REST requests only for service discovery and authentication.\n" +
						   "\n -restProxyUserName specifies the REST proxy user name. Used for REST requests only for service discovery and authentication.\n" +
						   "\n -restProxyPasswd specifies the REST proxy password. Used for REST requests only for service discovery and authentication.\n" +
						   "\n -restProxyDomain specifies the REST proxy domain. Used for REST requests only for service discovery and authentication.\n" +
						   "\n -restProxyKrb5ConfigFile specifies the REST proxy kerberos5 config file. Used for REST requests only for service discovery and authentication.\n" +
						   "\n -post specifies that the application should attempt to send post messages on the first requested Market Price item\n" +
						   "\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n" +
						   "\n -publisherInfo specifies that the application should add user provided publisher Id and publisher ipaddress when posting\n" +
						   "\n -snapshot specifies each request using non-streaming\n"  +
						   "\n -connectionType specifies the connection type that the connection should use (possible values are: 'socket', 'http', 'websocket', 'encrypted')\n" +
						   "\n -encryptedConnectionType specifies the encrypted connection type that the connection should use (possible values are: 'socket', 'http', 'websocket')\n" +
						   "\n -proxy specifies that proxy is used for connectionType of http or encrypted\n" +
						   "\n -ph specifies proxy server host name\n" +
						   "\n -pp specifies proxy port number\n" +
						   "\n -plogin specifies user name on proxy server\n" +
						   "\n -ppasswd specifies password on proxy server\n" +
						   "\n -pdomain specifies proxy server domain\n" +
						   "\n -krbfile specifies KRB File location and name\n" +
						   "\n -keyfile specifies keystore file location and name\n" +
						   "\n -keypasswd specifies keystore password\n" +
				           "\n -securityProvider Specifies security provider, default is SunJSSE, also supports Conscrypt\n" +
						   "\n       Example Usage for proxy with http/encryption:  -proxy -ph hostname1.com -pp 8080 -plogin David.Smith -ppasswd hello1 -pdomain workplace.com\n" +
						   "\n                                                     -krbfile C:\\Kerberos\\krb5.conf -keyfile C:\\Certificates\\cert1.jks -keypasswd keypass1 \n" +
						   "\n -x provides an XML trace of messages\n" +
						   "\n -cache will store all open items in cache and periodically dump contents\n" +
						   "\n -cacheInterval number of seconds between displaying cache contents, must greater than 0\n" +
						   "\n -statisticInterval number of seconds between displaying reactor channel statistics, must greater than 0\n" +
						   "\n -runtime adjusts the running time of the application" +
						   "\n -at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN" +
						   "\n -ax Specifies the Authentication Extended information" +
						   "\n -aid Specifies the Application ID" +
						   "\n -rtt Enables rtt support by a consumer. If provider makes distribution of RTT messages, consumer will return back them. In another case, consumer will ignore them." +
						   "\n -sendJsonConvError enable send json conversion error to provider " +
						   "\n -spTLSv1.2 specifies for an encrypted connection to be able to use TLS 1.2, default is 1.2 and 1.3 enabled" + 
						   "\n -spTLSv1.3 specifies for an encrypted connection to be able to use TLS 1.3, default is 1.2 and 1.3 enabled\n" +
							"\n Options for Preferred host (optional):" +
							"\n -enablePH enables Preferred host feature. By default, all the connections will set as a connection list in ReactorConnectOptions" +
							"\n -preferredHostIndex <index> specifies the preferred host as the index in the connection list. Default is 0" +
							"\n -detectionTimeInterval <time interval> specifies time interval (in seconds) to switch over to a preferred host. 0 indicates that the detection time interval is disabled. Default is 0" +
							"\n -detectionTimeSchedule <Cron time> specifies Cron time format to switch over to a preferred host. detectionTimeInterval is used instead if this member is set to empty. Default is empty\n" +
							"\n Options for IOCtl and Fallback calls (optional):" +
							"\n -fallBackInterval <time interval> specifies time interval (in seconds) in application before Ad Hoc Fallback function is invoked. O indicates that function won't be invoked. Default is 0" +
							"\n -ioctlInterval <time interval> specifies time interval (in seconds) before IOCtl function is invoked. O indicates that function won't be invoked. Default is 0" +
							"\n -ioctlEnablePH <true/false> enables Preferred host feature. Default is a value of enablePH" +
							"\n -ioctlConnectListIndex <index> specifies the preferred host as the index in the connection list. Default is a value of preferredHostIndex" +
							"\n -ioctlDetectionTimeInterval <time interval> specifies time interval (in seconds) to switch over to a preferred host. 0 indicates that the detection time interval is disabled. Default is a value of detectionTimeInterval" +
							"\n -ioctlDetectionTimeSchedule <Cron time> specifies Cron time format to switch over to a preferred host. Default is a value of detectionTimeSchedule\n");
	}
}

