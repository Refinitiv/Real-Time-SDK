/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.IO;
using System.Threading;
using ElementList = LSEG.Ema.Access.ElementList;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient, IServiceEndpointDiscoveryClient
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

	public void OnSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent @event)
	{
		Console.WriteLine(serviceEndpointResp); // dump service discovery endpoints
		string? endPoint = null;
		string? port = null;
		foreach(ServiceEndpointDiscoveryInfo info in serviceEndpointResp.ServiceEndpointInfoList)
		{
			if(info.LocationList.Count >= 2 && info.LocationList[0].StartsWith(Consumer.Location)) // Get an endpoint that provides auto failover for the specified location.
			{
				endPoint = info.Endpoint;
				port = info.Port;
				break;
			}
			// Try to get backups and keep looking for main case. Keep only the first item met.
			else if(info.LocationList.Count > 0 && info.LocationList[0].StartsWith(Consumer.Location) &&
					endPoint == null && port == null)
			{
				endPoint = info.Endpoint;
				port = info.Port;
			}
		}

		if(endPoint is not null && port is not null) {
			Consumer.Host = endPoint;
			Consumer.Port = port;
		}
	}

	public void OnError(String errorText, ServiceEndpointDiscoveryEvent @event)
	{
		Console.WriteLine("Failed to query RDP service discovery. Error text: " + errorText);
	}
}

public class Consumer
{
	private static string? clientId;
    private static string? clientSecret;
	private static string? clientJwk;
	private static string? audience;
    private static string? proxyHostName;
    private static string? proxyPort = "";
    private static string? proxyUserName;
    private static string? proxyPassword;
    private static string itemName = "IBM.N";
    private static string? tokenUrlV2;
    private static string? serviceDiscoveryUrl;

    public static string? Host { get; set; }
    public static string? Port { get; set; }
    public static string Location { get; set; } = "us-east-1";

    static void PrintHelp()
	{
	    Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -location location to get an endpoint from RDP service \r\n"
	    		+ "\tdiscovery. Defaults to \"us-east-1\" (optional).\n"
	    		+ "  -clientId client ID for application making the request to(mandatory for V2 client credentials grant) \r\n"
	    		+ "  -clientSecret service account secret (mandatory for V2 client credentials grant).\n"
				+ "  -jwkFile file containing the private JWK encoded in JSON format.\n"
				+ "  -audience audience location for the JWK (optional). \n"
				+ "  -tokenURLV2 V2 URL to perform authentication to get access and refresh tokens (optional).\n"
				+ "  -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).\n"
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
	            if (args[argsCount].Equals("-?"))
	            {
	                PrintHelp();
	                return false;
	            }
	            else if ("-clientId".Equals(args[argsCount]))
    			{
	            	clientId = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-clientSecret".Equals(args[argsCount]))
    			{
	            	clientSecret = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
				else if ("-jwkFile".Equals(args[argsCount]))
				{
					string? jwkFile = argsCount < (args.Length - 1) ? args[++argsCount] : null;
					if (jwkFile != null)
					{
						try
						{
							// Get the full contents of the JWK file.
							string jwkText = File.ReadAllText(jwkFile);

							clientJwk = jwkText;
						}
						catch (Exception e)
						{
							Console.Error.WriteLine("Error loading JWK file: " + e.Message);
							Console.Error.WriteLine();
							Console.WriteLine("Consumer exits...");
							Environment.Exit((int)Eta.Codec.CodecReturnCode.FAILURE);
						}
					}
					++argsCount;
				}
				else if ("-audience".Equals(args[argsCount]))
				{
					audience = argsCount < (args.Length - 1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-location".Equals(args[argsCount]))
    			{
	            	Location = argsCount < (args.Length-1) ? args[++argsCount] : Location;
    				++argsCount;				
    			}
    			else if ("-itemName".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
                    {
                        itemName = args[++argsCount];
                    }
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
						tokenUrlV2 = args[++argsCount];
						config.TokenUrlV2( tokenUrlV2 );
					}
					++argsCount;
				}
				else if ("-serviceDiscoveryURL".Equals(args[argsCount]))
				{
					if ( argsCount < (args.Length-1) ) {
						serviceDiscoveryUrl = args[++argsCount];
						config.ServiceDiscoveryUrl( serviceDiscoveryUrl );
					}
					++argsCount;
				}
    			else // unrecognized command line argument
    			{
    				PrintHelp();
    				return false;
    			}			
    		}

            if (clientId == null || (clientSecret == null && clientJwk == null))
            {
                Console.WriteLine("clientId/clientSecret or jwkFile must be specified on the command line. Exiting...");
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

			serviceDiscovery = new ServiceEndpointDiscovery(tokenUrlV2!, serviceDiscoveryUrl!);

            ServiceEndpointDiscoveryOption options = new()
            {
                ClientId = clientId,
                Transport = TransportProtocol.TCP,
                ProxyHostName = proxyHostName,
				ProxyPassword = proxyPassword,
                ProxyPort = proxyPort
            };

			if (clientJwk is null)
			{
				options.ClientSecret = clientSecret;
			}
			else
            {
				options.ClientJWK = clientJwk;
				
				if(audience != null)
                {
					options.Audience = audience;
				}
			}

			serviceDiscovery.RegisterClient(options, appClient);

			if (Host == null || Port == null)
			{
				Console.WriteLine("Both hostname and port are not avaiable for establishing a connection with Real-Time - Optimized. Exiting...");
				return;
			}

			CreateProgramaticConfig(configDb);
			config.ConsumerName("Consumer_1");
			config.ClientId(clientId!);

            if (clientSecret is not null)
            {
                config.ClientSecret(clientSecret);
            }
			if(clientJwk is not null)
            {
				config.ClientJwk(clientJwk);
            }
			if(audience is not null)
            {
				config.Audience(audience);
            }
            if (proxyHostName is not null)
            {
                config.ProxyHost(proxyHostName);
            }
            if (proxyPassword is not null)
            {
                config.ProxyPassword(proxyPassword);
            }
            if (proxyPort is not null)
            {
                config.ProxyPort(proxyPort);
            }
            if (proxyUserName is not null)
            {
                config.ProxyUserName(proxyUserName);
            }
            if (tokenUrlV2 is not null)
            {
                config.TokenUrlV2(tokenUrlV2);
            }
            if (serviceDiscoveryUrl is not null)
            {
                config.ServiceDiscoveryUrl(serviceDiscoveryUrl);
            }
            consumer = new(config.Config(configDb));
            consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Name(itemName), appClient);
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
