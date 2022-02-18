///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license                             --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.                         --
// *|                See the project's LICENSE.md for details.                                         --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.                                   --
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex113_MP_SessionMgmt;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println(refreshMsg);
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
	{
		System.out.println(updateMsg);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
	{
		System.out.println(statusMsg);
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer {

	static String userName;
	static String password;
	static String clientId;
	static String clientSecret;
	static boolean connectWebSocket = false;

	static String itemName = "IBM.N";

	static void printHelp()
	{
		System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
						   + "  -username machine ID to perform authorization with the\r\n"
						   + "\ttoken service (mandatory for V1 password credentials).\n"
						   + "  -password password to perform authorization with the token \r\n"
						   + "\tservice (mandatory for V1 password credentials).\n"
						   + "  -clientId client ID for application making the request to (mandatory) \r\n"
						   + "  -clientSecret client secret for application making the request to (mandatory for V2 oAuth client credentials)\r\n"
						   + "  -websocket Use the WebSocket transport protocol (optional) \r\n"
						   + "\tRDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
						   + "  -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials(optional, only for V1 oAuth password credentials).\r\n"
						   + "  -keyfile keystore file for encryption (mandatory).\n"
						   + "  -keypasswd keystore password for encryption (mandatory).\n"
						   + "  -tokenURL URL to perform authentication to get access and refresh tokens for V1 oAuth password credentials (optional).\n"
						   + "  -tokenURLV1 URL to perform authentication to get access and refresh tokens for V1 oAuth password credentials (optional).\n"
						   + "  -tokenURLV2 URL to perform authentication to get access and refresh tokens for V2 oAuth password credentials (optional).\n"
						   + "  -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).\n"
						   + "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
						   + "  -itemName Request item name (optional).\n"
						   + "  -ph Proxy host name (optional).\n"
						   + "  -pp Proxy port number (optional).\n"
						   + "  -plogin User name on proxy server (optional).\n"
						   + "  -ppasswd Password on proxy server (optional).\n"
						   + "  -pdomain Proxy Domain (optional).\n"
						   + "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos \r\n"
						   + "\tand Kerberos authentications (optional).\n"
						   + "\n");
	}

	static boolean readCommandlineArgs(String[] args, OmmConsumerConfig config)
	{
		try
		{
			int argsCount = 0;

			while (argsCount < args.length)
			{
				if (0 == args[argsCount].compareTo("-?"))
				{
					printHelp();
					return false;
				}
				else if ("-username".equals(args[argsCount]))
				{
					userName = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
					config.username(userName);
				}
				else if ("-password".equals(args[argsCount]))
				{
					password = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
					config.password(password);
				}
				else if ("-clientId".equals(args[argsCount]))
				{
					clientId = argsCount < (args.length-1) ? args[++argsCount] : null;
					config.clientId(clientId);
					++argsCount;
				}
				else if ("-clientSecret".equals(args[argsCount]))
				{
					clientSecret = argsCount < (args.length-1) ? args[++argsCount] : null;
					config.clientSecret(clientSecret);
					++argsCount;
				}
				else if ("-keyfile".equals(args[argsCount]))
				{
					config.tunnelingKeyStoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
					config.tunnelingSecurityProtocol("TLS");
					++argsCount;
				}
				else if ("-keypasswd".equals(args[argsCount]))
				{
					config.tunnelingKeyStorePasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-itemName".equals(args[argsCount]))
				{
					itemName = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-ph".equals(args[argsCount]))
				{
					config.tunnelingProxyHostName(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-pp".equals(args[argsCount]))
				{
					config.tunnelingProxyPort(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-plogin".equals(args[argsCount]))
				{
					config.tunnelingCredentialUserName(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-ppasswd".equals(args[argsCount]))
				{
					config.tunnelingCredentialPasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-pdomain".equals(args[argsCount]))
				{
					config.tunnelingCredentialDomain(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-krbfile".equals(args[argsCount]))
				{
					config.tunnelingCredentialKRB5ConfigFile(argsCount < (args.length-1) ? args[++argsCount] : null);
					++argsCount;
				}
				else if ("-takeExclusiveSignOnControl".equals(args[argsCount]))
				{
					String takeExclusiveSignOnControl = argsCount < (args.length-1) ? args[++argsCount] : null;

					if(takeExclusiveSignOnControl != null)
					{
						if(takeExclusiveSignOnControl.equalsIgnoreCase("true"))
							config.takeExclusiveSignOnControl(true);
						else if (takeExclusiveSignOnControl.equalsIgnoreCase("false"))
							config.takeExclusiveSignOnControl(false);
					}

					++argsCount;
				}
				else if ("-websocket".equals(args[argsCount]))
				{
					connectWebSocket = true;
					++argsCount;
				}
				else if ("-tokenURL".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) )
					{
						config.tokenServiceUrl( args[++argsCount] );
					}
					++argsCount;
				}
				else if ("-tokenURLV1".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) )
					{
						config.tokenServiceUrlV1( args[++argsCount] );
					}
					++argsCount;
				}
				else if ("-tokenURLV2".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) )
					{
						config.tokenServiceUrlV2( args[++argsCount] );
					}
					++argsCount;
				}
				else if ("-serviceDiscoveryURL".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) )
					{
						config.serviceDiscoveryUrl( args[++argsCount] );
					}
					++argsCount;
				}
				else // unrecognized command line argument
				{
					printHelp();
					return false;
				}
			}

			if ( (userName == null || password == null || clientId == null) && (clientId == null || clientSecret == null))
			{
				System.out.println("Username, password, and clientId or clientId and clientSecret must be specified on the command line. Exiting...");
				printHelp();
				return false;
			}
		}
		catch (Exception e)
		{
			printHelp();
			return false;
		}

		return true;
	}

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();

			if (!readCommandlineArgs(args, config))
				return;

			AppClient appClient = new AppClient();

			// The "Consumer_4" uses EncryptedProtocolType::RSSL_SOCKET while the "Consumer_5" uses EncryptedProtocolType::RSSL_WEBSOCKET predefined in EmaConfig.xml
			consumer  = EmaFactory.createOmmConsumer(config.consumerName(connectWebSocket ? "Consumer_5" : "Consumer_4"));

			consumer.registerClient( EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name(itemName), appClient);

			Thread.sleep(900000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
