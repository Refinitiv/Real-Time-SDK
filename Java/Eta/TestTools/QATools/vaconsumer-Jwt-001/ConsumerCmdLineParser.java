/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
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
	private ConnectionArgsParser connectionArgsParser = new ConnectionArgsParser();
	private String backupHostname;
	private String backupPort;
	private String clientSecret1;
	private String clientId1;
	private String jwkFile1 = "";
	private String audience1 = "";
	private String clientSecret2;
	private String clientId2;
	private String jwkFile2 = "";
	private String audience2 = "";
	private String tokenScope = "";		
	//END API QA
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
	private String keystoreFile;
	private String keystorePasswd;
	private boolean cacheOption;
	private int cacheInterval;
	private int statisticInterval;
	private String authenticationToken;
	private String authenticationExtended;
	private String applicationId;
	private boolean enableRtt;
	private boolean takeExclusiveSignOnControl = true;
	private String protocolList = "tr_json2";

	@Override
	public boolean parseArgs(String[] args)
	{
		try
		{
			int argsCount = 0;

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
				else if ("-bc".equals(args[argsCount]))
				{
					if (args[argsCount+1].contains(":"))
					{
						String[] tokens = args[++argsCount].split(":");
						if (tokens.length == 2)
						{
							backupHostname = tokens[0];
							backupPort = tokens[1];
							++argsCount;
						}
						else
						{
							// error
							System.out.println("\nError parsing backup connection arguments...\n");
							return false;
						}
					}
					else
					{
						// error
						System.out.println("\nError parsing backup connection arguments...\n");
						return false;
					}
				}
				//API QA							
				else if ("-clientSecret1".equals(args[argsCount]))
				{
					clientSecret1 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-clientId1".equals(args[argsCount]))
				{
					clientId1 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-jwkFile1".equals(args[argsCount]))
				{
					jwkFile1 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-audience1".equals(args[argsCount]))
				{
					audience1 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-clientSecret2".equals(args[argsCount]))
				{
					clientSecret2 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-clientId2".equals(args[argsCount]))
				{
					clientId2 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-jwkFile2".equals(args[argsCount]))
				{
					jwkFile2 = args[++argsCount];
					++argsCount;
				}
				
				else if ("-audience2".equals(args[argsCount]))
				{
					audience2 = args[++argsCount];
					++argsCount;
				}
				// END API QA
				
				else if ("-tokenScope".equals(args[argsCount]))
				{
					tokenScope = args[++argsCount];
					++argsCount;
				}
				else if ("-sessionMgnt".equals(args[argsCount]))
				{
					enableSessionMgnt = true;
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
				else if ("-proxy".equals(args[argsCount]))
				{
					enableProxy =  true;
					++argsCount;
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
				else if ("-pl".equals(args[argsCount])) {
					protocolList = args[++argsCount];
					++argsCount;
				}
				else // unrecognized command line argument
				{
					System.out.println("\nUnrecognized command line argument...\n");
					return false;
				}
			}
		}
		catch (Exception e)
		{
			System.out.println("\nInvalid command line arguments...");
			return false;
		}

		return true;
	}

	String backupHostname()
	{
		return backupHostname;
	}

	String backupPort()
	{
		return backupPort;
	}

	List<ConnectionArg> connectionList()
	{
		return connectionArgsParser.connectionList();
	}
	
	boolean enableView()
	{
		return enableView;
	}
	//API QA
	
	String clientSecret1()
	{
		return clientSecret1;
	}
	
	String clientId1()
	{
		return clientId1;
	}
	
	String jwkFile1()
	{
		return jwkFile1;
	}
	
	String audience1()
	{
		return audience1;
	}
	
	String clientSecret2()
	{
		return clientSecret2;
	}
	
	String clientId2()
	{
		return clientId2;
	}
		
	String jwkFile2()
	{
		return jwkFile2;
	}
	
	String audience2()
	{
		return audience2;
	}
	
	//END API QA
	
	String tokenScope()
	{
		return tokenScope;
	}
	String tokenURLV1()
	{
		return tokenURL_V1;
	}
	
	String tokenURLV2()
	{
		return tokenURL_V2;
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

	@Override
	public void printUsage()
	{
		System.out.println("Usage: Consumer or\nConsumer [-c <hostname>:<port> <service name> <domain>:<item name>,...] [-bc <hostname>:<port>] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>]" +
						   "\n -c specifies a connection to open and a list of items to request or use for queue messaging:\n" +
						   "\n     hostname:        Hostname of provider to connect to" +
						   "\n     port:            Port of provider to connect to" +
						   "\n     service:         Name of service to request items from on this connection" +
						   "\n     domain:itemName: Domain and name of an item to request" +
						   "\n         A comma-separated list of these may be specified." +
						   "\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)" +
						   "\n         The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)" +
						   "\n         Example Usage: -c localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl" +
						   "\n           (for SymbolList requests, a name can be optionally specified)\n" +
						   "\n     -qSourceName (optional) specifies the source name for queue messages (if specified, configures consumer to receive queue messages)\n" +
						   "\n     -qDestName (optional) specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified)\n" +
						   "\n     -tunnel (optional) enables consumer to open tunnel stream and send basic text messages" +
						   "\n     -tsServiceName (optional) specifies the service name for queue messages (if not specified, the service name specified in -c/-tcp is used)\n" +
						   "\n     -tsAuth (optional) causes consumer to request authentication when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging.\n" +
						   "\n     -tsDomain (optional) specifes the domain a consumer uses when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging.\n" +
						   "\n -bc specifies a backup connection that is attempted if the primary connection fails\n" +
						   "\n -pl protocol list (defaults to rssl.rwf, tr_json2, rssl.json.v2)\n" +						 
						   "\n -sessionMgnt enables the session management in the Reactor\n" +
						   "\n -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials.\n" +
						   "\n -view specifies each request using a basic dynamic view\n" +
						   "\n -post specifies that the application should attempt to send post messages on the first requested Market Price item\n" +
						   "\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n" +
						   "\n -publisherInfo specifies that the application should add user provided publisher Id and publisher ipaddress when posting\n" +
						   "\n -snapshot specifies each request using non-streaming\n"  +
						   "\n -connectionType specifies the connection type that the connection should use (possible values are: 'socket', 'http', 'websocket', 'encrypted')\n" +
						   "\n -encryptedConnectionType specifies the encrypted connection type that the connection should use (possible values are: 'socket', 'http', 'websocket')\n" +
						   "\n -proxy specifies that proxy is used for connectionType of http or encrypted\n" +
						   "\n -ph specifies proxy server host name\n" +
						   "\n -pp specifies roxy port number\n" +
						   "\n -plogin specifies user name on proxy server\n" +
						   "\n -ppasswd specifies password on proxy server\n" +
						   "\n -pdomain specifies proxy server domain\n" +
						   "\n -krbfile specifies KRB File location and name\n" +
						   "\n -keyfile specifies keystore file location and name\n" +
						   "\n -keypasswd specifies keystore password\n" +
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
						   "\n -clientSecret1 associated client secret for the client ID with the v2 login." +
						   "\n -clientId1 specifies the Client ID login v2." +
						   "\n -jwkFile1" +
						   "\n -audience1" +
						   "\n -clientSecret2 associated client secret for the client ID with the v2 login." +
						   "\n -clientId2 specifies the Client ID login v2." +
						   "\n -tokenURLV2 token generator URL V2" +
						   "\n -jwkFile2" +
						   "\n -audience2" );
	}
}

