/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
	{
		Console.WriteLine(refreshMsg);
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine(updateMsg);
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine(statusMsg);
	}

	public void OnError(string errorText, ServiceEndpointDiscoveryEvent _)
	{
		Console.WriteLine("Failed to query RDP service discovery. Error text: " + errorText);
	}
}

    /// <summary>
    /// This is for example purposes, For best security, please use a proper credential store.
    /// </summary>
    class CredentialStore
{
	public string? ClientSecret { get; set; }
	public string? ClientId { get; set; }
    public OmmConsumer? Consumer { get; set; }
}

/* Implementation of OmmOAuth2ConsumerClient.  This is a very basic callback that uses the closure to obtain the OmmConsumer and call submitOAuthCredentialRenewal.
 * This is intended to show functionality, so this example does not implement or use secure credential storage.
 */
class OAuthCallback : IOmmOAuth2ConsumerClient
{
    public void OnOAuth2CredentialRenewal(IOmmConsumerEvent evt, OAuth2CredentialRenewal creds)
    {
        CredentialStore? credentials = (CredentialStore?)evt.Closure;
		creds.Clear();
        creds.ClientId(credentials?.ClientId!);
        creds.ClientSecret(credentials?.ClientSecret!);
        Console.WriteLine("Submitting credentials due to token renewal");
        evt.Consumer.RenewOAuthCredentials(creds);
    }
}

public class Consumer
{
	static readonly OAuthCallback oAuthCallback = new OAuthCallback();
	static readonly CredentialStore credentials = new CredentialStore();
	static string? proxyHostName;
	static string proxyPort = "";
	static string? proxyUserName;
	static string? proxyPassword;
	public static string? Host { get; set; } = null;
	public static string? Port { get; set; } = null;

	public static string ItemName { get; set; } = "IBM.N";

	public static string tokenUrlV2 = "https://api.refinitiv.com/auth/oauth2/v2/token";

	static void printHelp()
	{
	    Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -h hostname to connect to (mandatory)\r\n" 
	    		+ "  -p port to connect to (mandatory)\r\n" 
	    		+ "  -clientId machine account to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory).\n"
	    		+ "  -clientSecret associated client secret with the machine id \r\n"
	    		+ "\tservice (mandatory).\n"
				+ "  -tokenURLV2 URL to perform authentication to get access and refresh tokens (optional).\n"
	    		+ "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -itemName Request item name (optional).\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n"
	    		+ "\n");
	}
	
	static bool readCommandlineArgs(string[] args, OmmConsumerConfig config)
	{
	    try
	    {
	        int argsCount = 0;

	        while (argsCount < args.Length)
	        {
	            if (0 == args[argsCount].CompareTo("-?"))
	            {
	                printHelp();
	                return false;
	            }
	            else if ("-clientId".Equals(args[argsCount]))
    			{
	            	credentials.ClientId = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-clientSecret".Equals(args[argsCount]))
    			{
	            	credentials.ClientSecret = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-itemName".Equals(args[argsCount]))
    			{
    				if(argsCount < (args.Length-1))	ItemName = args[++argsCount];
    				++argsCount;
    			}
    			else if ("-h".Equals(args[argsCount]))
    			{
    				Host = argsCount < (args.Length-1) ? args[++argsCount] : Host;
    				++argsCount;				
    			}
    			else if ("-p".Equals(args[argsCount]))
    			{
    				Port = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-ph".Equals(args[argsCount]))
    			{
    				proxyHostName = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-pp".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
					{
						proxyPort = args[++argsCount];
                    }
    				++argsCount;				
    			}
    			else if ("-plogin".Equals(args[argsCount]))
    			{
    				proxyUserName = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-ppasswd".Equals(args[argsCount]))
    			{
    				proxyPassword = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
				else if ("-tokenURLV2".Equals(args[argsCount]))
				{
					if ( argsCount < (args.Length-1) ) {
						tokenUrlV2 = args[++argsCount];
						config.TokenUrlV2( tokenUrlV2 );
					}
					++argsCount;
				}
    			else // unrecognized command line argument
    			{
    				printHelp();
    				return false;
    			}			
    		}
	        
	        if (Host == null || Port == null || credentials.ClientSecret == null || credentials.ClientId == null)
			{
				Console.WriteLine("host, port, ClientId and client Secret must be specified on the command line. Exiting...");
				printHelp();
				return false;
			}
        }
        catch
        {
        	printHelp();
            return false;
        }
		
		return true;
	}
	
	static void CreateProgramaticConfig(Map configDb)
	{
		Map elementMap = new();
		ElementList elementList = new();
		ElementList innerElementList = new();
		
		innerElementList.AddAscii("Channel", "Channel_1");
		
		elementMap.AddKeyAscii("Consumer_1", MapAction.ADD, innerElementList.Complete());
		innerElementList.Clear();
		
		elementList.AddMap("ConsumerList", elementMap.Complete());
		elementMap.Clear();
		
		configDb.AddKeyAscii("ConsumerGroup", MapAction.ADD, elementList.Complete());
		elementList.Clear();
		
		innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.ENCRYPTED);
		
		innerElementList.AddAscii("Host", Host!);
		innerElementList.AddAscii("Port", Port!);
		innerElementList.AddUInt("EnableSessionManagement", 1);
		
		elementMap.AddKeyAscii("Channel_1", MapAction.ADD, innerElementList.Complete());
		innerElementList.Clear();
		
		elementList.AddMap("ChannelList", elementMap.Complete());
		elementMap.Clear();
		
		configDb.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList.Complete());
		elementList.Clear();

		configDb.Complete();
	}
	
	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
		ServiceEndpointDiscovery? serviceDiscovery = null;
		try
		{
			AppClient appClient = new();
			OmmConsumerConfig config = new();
			Map configDb = new();
			
			if (!readCommandlineArgs(args, config)) return;

			CreateProgramaticConfig(configDb);
			
			if ( (proxyHostName == null) && (proxyPort == "") )
			{
				consumer  = new(config.ConsumerName("Consumer_1").ClientSecret(credentials.ClientSecret!)
					.ClientId(credentials.ClientId!).TokenUrlV2(tokenUrlV2).Config(configDb), oAuthCallback, credentials);
			}
			else
			{
				consumer  = new(config.ConsumerName("Consumer_1").ClientSecret(credentials.ClientSecret!)
					.ClientId(credentials.ClientId!).Config(configDb).TokenUrlV2(tokenUrlV2)
					.ProxyHost(proxyHostName!).ProxyPort(proxyPort)
					.ProxyUserName(proxyUserName!).ProxyPassword(proxyPassword!), oAuthCallback, credentials);
			}
			
			/* Set the consumer on the credential structure for the callback */
			credentials.Consumer = consumer;
			
			consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Name(ItemName), appClient);
			
			Thread.Sleep(900000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
		} 
		catch (OmmException excp)
		{
			Console.WriteLine(excp.Message);
		}
		finally 
		{
			consumer?.Uninitialize();
			serviceDiscovery?.Uninitialize();
		}
	}
}
