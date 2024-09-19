/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.niprovider;

import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.examples.common.CommandLineParser;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArg;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArgsParser;

import java.util.List;

/*
 * Command line parser for the Value Add NIProvider application.
 */
class NIProviderCmdLineParser implements CommandLineParser
{
	private ConnectionArgsParser connectionArgsParser = new ConnectionArgsParser();
	private boolean hasBackupConfig = false;
	private int backupConnectionType;
	private String backupHostname;
	private String backupPort;
	private String userName;
	private String backupMcastSendAddress;
	private String backupMcastSendPort;
	private String backupMcastRecvAddress;
	private String backupMcastRecvPort;
	private String backupMcastInterfaceName;
	private String backupUnicastPort;
	private int runtime = 600; // default runtime is 600 seconds
	private boolean enableXmlTracing;
	private boolean cacheOption;
	private String authenticationToken;
	private String authenticationExtended;
	private String applicationId;
	private boolean enableSessionManagement = false;
	
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
    				if ((argsCount = connectionArgsParser.parse(args, argsCount)) < 0)
    				{
    					// error
    					System.out.println("\nError parsing connection arguments...\n");
    					return false;
    				}
    			}
    			else if ("-bc".equals(args[argsCount]))
    			{
    				hasBackupConfig = true;
    				backupConnectionType = ConnectionTypes.SOCKET; 
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
    			else if ("-mbc".equals(args[argsCount]))
    			{
    				hasBackupConfig = true;
    				backupConnectionType = ConnectionTypes.RELIABLE_MCAST; 
    				
    				if (args[argsCount+1].contains(":") && args[argsCount+2].contains(":") &&
    					!args[argsCount+3].contains(":"))					
    				{
    					String[] sendAddrTokens = args[argsCount+1].split(":");
    					if (sendAddrTokens.length == 2 || sendAddrTokens.length == 3)
    					{
    						backupMcastSendAddress = sendAddrTokens[0];
    						backupMcastSendPort = sendAddrTokens[1];
    						if (sendAddrTokens.length == 3)
    						{
    							backupMcastInterfaceName = sendAddrTokens[2];
    						}
    					}
    					else
    					{
    						System.out.println("\nError parsing backup multicast connection arguments...\n");
    						return false;
    					}
    					String[] recvAddrTokens = args[argsCount+2].split(":");
    					if (recvAddrTokens.length == 2)
    					{
    						backupMcastRecvAddress = recvAddrTokens[0];
    						backupMcastRecvPort = recvAddrTokens[1];
    						backupUnicastPort = args[argsCount+3];								                                                
    						argsCount +=4;
    					}
    					else
    					{
    						System.out.println("\nError parsing backup multicast arguments...\n");
    						return false;
    
    					}
    				}
    			}				
    			else if ("-uname".equals(args[argsCount]))
    			{
    				userName = args[++argsCount];
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
    			else if ("-cache".equals(args[argsCount]))
    			{
    				cacheOption =  true;
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
    			}
    			else if ("-sm".equals(args[argsCount]))
    			{
    				enableSessionManagement = true;
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
	
	boolean hasBackupConfig()
	{
		return hasBackupConfig;
	}
	
	int backupConnectionType()
	{
		return backupConnectionType;
	}

	String backupHostname()
	{
		return backupHostname;
	}

	String backupPort()
	{
		return backupPort;
	}
	
	String backupSendAddress()
	{
		return backupMcastSendAddress;
	}
	
	String backupSendPort()
	{
		return backupMcastSendPort;
	}
	
	String backupRecvAddress()
	{
		return backupMcastRecvAddress;
	}
	
	String backupRecvPort()
	{
		return backupMcastRecvPort;
	}
	
	String backupInterfaceName()
	{
		return backupMcastInterfaceName;
	}
	
	String backupUnicastPort()
	{
		return backupUnicastPort;	
	}

	List<ConnectionArg> connectionList()
	{
		return connectionArgsParser.connectionList();
	}

	String userName()
	{
		return userName;
	}

	int runtime()
	{
		return runtime;
	}
	
	boolean enableXmlTracing()
	{
		return enableXmlTracing;
	}

	boolean cacheOption()
	{
		return cacheOption;
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
	
	boolean enableSessionManagement()
	{
		return enableSessionManagement;
	}	

	@Override
	public void printUsage()
	{
		System.out.println("Usage: NIProvider or\nNIProvider [-c <hostname>:<port> <service name> <domain>:<item name>,... OR -segmentedMulticast <sa>:<sp>:<if> <ra>:<rp> <up> <service > <domain>:<item name>,...] " +
	            "[-bc <hostname>:<port> OR -mbc <bsa>:<bsp>:<bif> <bra>:<brp> <bup>] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>]" +
				"\n -c specifies a connection to open and a list of items to provide:\n" +
				"\n     hostname:        Hostname of ADH to connect to" +
				"\n     port:            Port of ADH to connect to" +
				"\n     service:         Name of service to provide items on this connection" +
				"\n     domain:itemName: Domain and name of an item to provide" +
				"\n         A comma-separated list of these may be specified." +
				"\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder)" +
				"\n         Example Usage: -c localhost:14002 NI_PUB mp:TRI,mp:GOOG,mbo:MSFT\n" +
				"\n -segmentedMulticast specifies a reliable multicast connection to open and a list of items to provide:\n" +
				"\n     sa: Sender address of ADH to connect to" +
				"\n     sp: Sender port of ADH to connect to" +
				"\n     if: Interface of ADH to connect to" +
				"\n     ra: Receiver address of ADH to connect to" + 
				"\n     rp: Receiver port of ADH to connect to" +
				"\n     up: Unicast port number" +
				"\n     service: Name of service to provide items on this connection" + 
				"\n     domain:itemName: Domain and name of an item to provide" +
				"\n         A comma-separated list of these may be specified." +
				"\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder)" +
				"\n         Example Usage: -segmentedMulticast 235.5.5.5:14002:localhost 235.5.5.4:14003 14007 NI_PUB mp:TRI.N,mp:IBM.N -mbc 235.5.5.6:14004:localhost 235.5.5.7:14005 14008\n" +
				"\n -bc specifies a backup connection that is attempted if the primary connection fails (addr:port)\n" +
				"\n -mbc specifies a backup multicast connection that is attempted if the primary connection fails (sa:sp:if ra:rp up)\n" +
				"\n -uname changes the username used when logging into the provider\n" +
				"\n -x provides an XML trace of messages\n" +
				"\n -cache turn on the cache feature of the application\n" +
				"\n -runtime adjusts the running time of the application" +
				"\n -at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN" +
				"\n -ax Specifies the Authentication Extended information" +
				"\n -aid Specifies the Application ID" +
				"\n -sm (optional) Enable Session Management");
	}
}

