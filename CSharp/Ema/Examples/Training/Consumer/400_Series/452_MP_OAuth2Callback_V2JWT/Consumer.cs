/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Eta.Codec;
using System;
using System.IO;
using System.Threading;
using ElementList = LSEG.Ema.Access.ElementList;
using Map = LSEG.Ema.Access.Map;

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
	public string? ClientJwk { get; set; }
	public string? ClientId { get; set; }
    public OmmConsumer? Consumer { get; set; }
}

/// <summary>
/// Implementation of OmmOAuth2ConsumerClient.  This is a very basic callback that uses the closure to obtain the OmmConsumer and call submitOAuthCredentialRenewal.
/// This is intended to show functionality, so this example does not implement or use secure credential storage.
/// </summary>
class OAuthCallback : IOmmOAuth2ConsumerClient
{
	public void OnOAuth2CredentialRenewal(IOmmConsumerEvent evt, OAuth2CredentialRenewal credentials)
	{
		var cs = (CredentialStore?) evt.Closure;
		credentials.Clear();
        credentials.ClientId(cs?.ClientId!);
        credentials.ClientJWK(cs?.ClientJwk!);
		Console.WriteLine("Submitting credentials due to token renewal");
        evt.Consumer.RenewOAuthCredentials(credentials);
	}
}

public class Consumer
{
    private const string DefaultTokenUrlV2 = "https://api.refinitiv.com/auth/oauth2/v2/token";

    private static readonly OAuthCallback oAuthCallback = new();
    private static readonly CredentialStore credentials = new();
    private static string? proxyHostName;
    private static string? proxyPort = "";
    private static string? proxyUserName;
    private static string? proxyPassword;
    private static string? audience;

    public static string? Host { get; set; }
	public static string? Port { get; set; }
    public static string TokenUrlV2 { get; set; } = DefaultTokenUrlV2;
    public static string ItemName { get; set; } = "IBM.N";

    static void PrintHelp()
	{
	    Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
				+ "  -h hostname to connect to (mandatory)\r\n"
				+ "  -p port to connect to (mandatory)\r\n"
				+ "  -clientId machine account to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory).\n"
	    		+ "  -jwkFile file containing the private JWK encoded in JSON format. (mandatory)\n"
	    		+ "  -audience audience location for the JWK (optional). \r\n"
				+ "  -tokenURLV2 URL to perform authentication to get access and refresh tokens (optional).\n"
	    		+ "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -itemName Request item name (optional).\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n"
	    		+ "\n");
	}
	
	static bool ReadCommandlineArgs(string[] args, OmmConsumerConfig config)
	{
	    try
	    {
	        int argsCount = 0;

	        while (argsCount < args.Length)
	        {
	            if (0 == args[argsCount].CompareTo("-?"))
	            {
	                PrintHelp();
	                return false;
	            }
	            else if ("-clientId".Equals(args[argsCount]))
    			{
	            	credentials.ClientId = argsCount < (args.Length-1) ? args[++argsCount] : null;
	            	config.ClientId(credentials.ClientId!);
    				++argsCount;				
    			}
	            else if ("-jwkFile".Equals(args[argsCount]))
    			{
	            	string? jwkFile = argsCount < (args.Length-1) ? args[++argsCount] : null;
	            	if(jwkFile != null)
	            	{
		            	try
						{
							// Get the full contents of the JWK file.
							string jwkText = File.ReadAllText(jwkFile);
							
							credentials.ClientJwk = jwkText;
			            	config.ClientJwk(credentials.ClientJwk);
						}
						catch(Exception e)
						{
							Console.Error.WriteLine("Error loading JWK file: " + e.Message);
							Console.Error.WriteLine();
							Console.WriteLine("Consumer exits...");
							Environment.Exit((int)CodecReturnCode.FAILURE);
						} 
	            	}
    				++argsCount;				
    			}
    			else if ("-itemName".Equals(args[argsCount]))
    			{
    				if(argsCount < (args.Length-1))	ItemName = args[++argsCount];
    				++argsCount;
    			}
    			else if ("-h".Equals(args[argsCount]))
    			{
    				Host = argsCount < (args.Length-1) ? args[++argsCount] : null;
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
    				proxyPort = argsCount < (args.Length-1) ? args[++argsCount] : null;
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
						TokenUrlV2 = args[++argsCount];
						config.TokenUrlV2(TokenUrlV2);
					}
					++argsCount;
				}
				else if ("-audience".Equals(args[argsCount]))
				{
					audience = argsCount < (args.Length-1) ? args[++argsCount] : null;
					if(audience != null)
						config.Audience(audience);
					
					++argsCount;
				}
    			else // unrecognized command line argument
    			{
    				Console.WriteLine("Unknown argument: " + args[argsCount]);
    				PrintHelp();
    				return false;
    			}			
    		}
	        
	        if (Host == null || Port == null || credentials.ClientJwk == null || credentials.ClientId == null)
			{
				Console.WriteLine("host, port, ClientId and client JWK must be specified on the command line. Exiting...");
				PrintHelp();
				return false;
			}
        }
        catch
        {
        	PrintHelp();
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
			
			if (!ReadCommandlineArgs(args, config)) return;

            CreateProgramaticConfig(configDb);

            if ( (proxyHostName == null) && (proxyPort == "") )
			{
				consumer  = new(config.ConsumerName("Consumer_1")
					.TokenUrlV2(TokenUrlV2).Config(configDb), oAuthCallback, credentials);
			}
			else
			{
				consumer  = new (config.ConsumerName("Consumer_1")
					.TokenUrlV2(TokenUrlV2).Config(configDb)
					.ProxyHost(proxyHostName!)
					.ProxyPort(proxyPort!)
					.ProxyUserName(proxyUserName!)
					.ProxyPassword(proxyPassword!)
					, oAuthCallback, credentials);
			}
			
			/* Set the consumer on the credential structure for the callback */
			credentials.Consumer = consumer;
			
			consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Name(ItemName), appClient);
			
			Thread.Sleep(900000);           // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
            }
            catch (OmmException ommException)
            {
                Console.WriteLine(ommException.Message);
            }
            finally 
		{
			consumer?.Uninitialize();
			if (serviceDiscovery != null) serviceDiscovery.Uninitialize();
		}
	}
}
