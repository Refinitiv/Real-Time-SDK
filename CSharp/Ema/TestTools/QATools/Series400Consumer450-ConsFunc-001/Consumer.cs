/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Threading;
using static LSEG.Ema.Access.EmaConfig;

namespace LSEG.Ema.Example.Traning.Consumer;

class AppClient : IOmmConsumerClient, IServiceEndpointDiscoveryClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine(refreshMsg);
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine(updateMsg);
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine(statusMsg);
	}

	public void OnSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent @event)
	{
		Console.WriteLine(serviceEndpointResp); // dump service discovery endpoints
		
		for(int index = 0; index < serviceEndpointResp.ServiceEndpointInfoList.Count; index++)
		{
			List<string> locationList = serviceEndpointResp.ServiceEndpointInfoList[index].LocationList;
			
			if(locationList.Count == 2) // Get an endpoint that provides auto failover for the specified location.
			{
				if(locationList[0].StartsWith(Consumer.Location))
				{
					Consumer.Host = serviceEndpointResp.ServiceEndpointInfoList[index].Endpoint;
					Consumer.Port = serviceEndpointResp.ServiceEndpointInfoList[index].Port;
					break;
				}
			}
		}
	}

	public void OnError(string errorText, ServiceEndpointDiscoveryEvent @event)
	{
		Console.WriteLine("Failed to query Delivery Platform service discovery. Error text: " + errorText);
	}
}

public class Consumer
{
    private static string? clientId;
    private static string? clientSecret;
    private static string? proxyHostName;
    private static string? proxyPort = "-1";
    private static string? proxyUserName;
    private static string? proxyPassword;
	private static string? serviceDiscoveryUrl;
    private static string? tokenServiceUrl;

    public static string? Host { get; set; }
    public static string? Port { get; set; }
    public static string Location { get; set; } = "us-east-1";
    // API QA
    private static double? tokenReissueRatio;
    private static ulong? restRequestTimeout;
    private static int? reissueTokenAttemptLimit;
    private static int? reissueTokenAttemptInterval;
	// END API QA
	
	static void PrintHelp()
	{
	    Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
            + "  -username machine ID to perform authorization with the\r\n"
                + "\ttoken service (mandatory).\n"
                + "  -password password to perform authorization with the token \r\n"
                + "\tservice (mandatory).\n"
                + "  -location location to get an endpoint from RDP-RT service \r\n"
	    		+ "\tdiscovery. Defaults to \"us-east-1\" (optional).\n"
	    		+ "  -clientId client ID for application making the request to \r\n" 
	    		+ "\tRDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
                + "  -clientSecret client secret for application making the request to \r\n"
                + "\tRDP token service (mandatory).\n"
                + "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n" 
	    		// API QA
	    		+ "  -tokenServiceUrl (for QA testing).\n"
	    		+ "  -serviceDiscoveryUrl (for QA testing).\n"
	    		+ "  -restRequestTimeout (for QA testing).\n"
	    		+ "  -tokenReissueRatio (for QA testing).\n"
	    		+ "  -reissueTokenAttemptLimit (for QA testing).\n"
	    		+ "  -reissueTokenAttemptInterval (for QA testing).\n"
	    		// END API QA
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
	            	clientId = argsCount < (args.Length-1) ? args[++argsCount] : null;
    				++argsCount;
                }
                else if ("-clientSecret".Equals(args[argsCount]))
                {
					if(argsCount < (args.Length - 1))
                        clientSecret = args[++argsCount];
                    ++argsCount;
                }
                else if ("-location".Equals(args[argsCount]))
    			{
	            	Location = argsCount < (args.Length-1) ? args[++argsCount] : Location;
    				++argsCount;				
    			}
	            // API QA
    			else if ("-tokenServiceUrl".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.TokenUrlV2(tokenServiceUrl = args[++argsCount]);
    				++argsCount;				
    			}
    			else if ("-serviceDiscoveryUrl".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.ServiceDiscoveryUrl(serviceDiscoveryUrl = args[++argsCount]);
    				++argsCount;
    			}
    			else if ("-restRequestTimeout".Equals(args[argsCount]))
     			{			
     				restRequestTimeout = argsCount < (args.Length-1) ? ulong.Parse(args[++argsCount]) : default;
    				++argsCount;
     			}
    			else if ("-reissueTokenAttemptInterval".Equals(args[argsCount]))
     			{			
    				reissueTokenAttemptInterval = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : default;
    				++argsCount;	
     			}
    			else if ("-reissueTokenAttemptLimit".Equals(args[argsCount]))
     			{			
     				reissueTokenAttemptLimit = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : default;
    				++argsCount;	
     			}
    			else if ("-tokenReissueRatio".Equals(args[argsCount]))
     			{			
     				tokenReissueRatio = argsCount < (args.Length-1) ? double.Parse(args[++argsCount]) : default;
    				++argsCount;	
     			}
	            
	            // END API QA
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
    			else // unrecognized command line argument
    			{
    				PrintHelp();
    				return false;
    			}			
    		}
	        
	        if (clientId == null)
			{
				Console.WriteLine("ClientId must be specified on the command line. Exiting...");
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
	
	static Map CreateProgramaticConfig()
	{
		Map configDb = new();
        Map elementMap = new();
		ElementList elementList = new();
		ElementList innerElementList = new();
	
		// API QA	
		//elementList.AddAscii("DefaultConsumer", "Consumer_1" ));
		//innerElementList.AddAscii("Channel", "Channel_1"));
		innerElementList.AddUInt("XmlTraceToStdout", 1);
		if(restRequestTimeout is not null)
			innerElementList.AddUInt("RestRequestTimeOut", restRequestTimeout!.Value);
        if (tokenReissueRatio is not null)
            innerElementList.AddDouble("TokenReissueRatio", tokenReissueRatio!.Value);
        if (reissueTokenAttemptLimit is not null)
            innerElementList.AddInt("ReissueTokenAttemptLimit", reissueTokenAttemptLimit!.Value);
        if (reissueTokenAttemptInterval is not null)
            innerElementList.AddInt("ReissueTokenAttemptInterval",reissueTokenAttemptInterval!.Value);
        innerElementList.AddAscii( "Dictionary", "Dictionary_2");
        innerElementList.AddAscii( "Logger", "Logger_1");
		
		// END API QA
		innerElementList.AddAscii("Channel", "Channel_1");
		
		elementMap.AddKeyAscii("Consumer_1", (int)Eta.Codec.MapEntryActions.ADD, innerElementList.Complete());
		innerElementList.Clear();
		
		elementList.AddMap("ConsumerList", elementMap.Complete());
		elementMap.Clear();
		
		configDb.AddKeyAscii("ConsumerGroup", (int)Eta.Codec.MapEntryActions.ADD, elementList.Complete());
		elementList.Clear();
		
		innerElementList.AddEnum("ChannelType", ConnectionTypeEnum.ENCRYPTED);
		innerElementList.AddAscii("Host", Host!);
		innerElementList.AddAscii("Port", Port!);
		innerElementList.AddUInt("EnableSessionManagement", 1);
		
		elementMap.AddKeyAscii("Channel_1", (int)Eta.Codec.MapEntryActions.ADD, innerElementList.Complete());
		innerElementList.Clear();
		
		elementList.AddMap("ChannelList", elementMap.Complete());
		elementMap.Clear();
		
		configDb.AddKeyAscii("ChannelGroup", (int)Eta.Codec.MapEntryActions.ADD, elementList.Complete());
		return configDb.Complete();
    }
	
	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
		ServiceEndpointDiscovery? serviceDiscovery = null;
		try
		{
			AppClient appClient = new();
			OmmConsumerConfig config = new();
			
			if (!ReadCommandlineArgs(args, config)) return;
			var disOptions = new ServiceEndpointDiscoveryOption()
			{
				ClientId = clientId,
				Transport = TransportProtocol.TCP,
				ProxyHostName = proxyHostName,
				ProxyPort = proxyPort,
				ProxyUserName = proxyUserName,
				ProxyPassword = proxyPassword,
				ClientSecret = clientSecret
            };
            serviceDiscovery = tokenServiceUrl is not null && serviceDiscoveryUrl is not null ? new(tokenServiceUrl, serviceDiscoveryUrl) : new();

            serviceDiscovery.RegisterClient(disOptions, appClient);
			
			if ( Host == null || Port == null )
			{
				Console.WriteLine("Both hostname and port are not avaiable for establishing a connection with Real-Time - Optimized. Exiting...");
				return;
			}

            var configDb = CreateProgramaticConfig();
            config.ConsumerName("Consumer_1").ClientId(clientId!).ClientSecret(clientSecret!).Config(configDb);

            if ((proxyHostName is not null) || (proxyPort != "-1"))
			{
				consumer  = new(config.ProxyHost(proxyHostName!).ProxyPort(proxyPort!)
					.ProxyUserName(proxyUserName!).ProxyPassword(proxyPassword!));
			}
            consumer = new(config);
            consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Name("BBL.BK"), appClient);
			
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
