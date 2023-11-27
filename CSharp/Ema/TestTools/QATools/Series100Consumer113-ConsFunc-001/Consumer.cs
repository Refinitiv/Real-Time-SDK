/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Threading;
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
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
}

public class Consumer {
    static string? clientId = null;
    static string? clientSecret = null;

    static void PrintHelp()
	{
	    Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -clientId client ID for application making the request to \r\n" 
	    		+ "\tRDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
                + "  -clientSecret client secret for application making the request to \r\n"
                + "\tRDP token service (mandatory).\n"
                + "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n"
	    		+ "  -tokenServiceUrl \r\n" 
	    		+ "  -serviceDiscoveryUrl \r\n" 
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
	            	clientId = argsCount < (args.Length-1) ? args[++argsCount] : clientId;
					if(clientId is not null)
					{
                        config.ClientId(clientId);
                    }
    				++argsCount;				
    			}
                else if ("-clientSecret".Equals(args[argsCount]))
                {
                    clientSecret = argsCount < (args.Length - 1) ? args[++argsCount] : clientSecret;
                    if (clientSecret is not null)
                    {
                        config.ClientSecret(clientSecret);
                    }
                    ++argsCount;
                }
                else if ("-ph".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.ProxyHost(args[++argsCount]);
    				++argsCount;				
    			}
    			else if ("-pp".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.ProxyPort(args[++argsCount]);
    				++argsCount;				
    			}
    			else if ("-plogin".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length-1))
    				config.ProxyUserName(args[++argsCount]);
    				++argsCount;				
    			}
    			else if ("-ppasswd".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.ProxyPassword(args[++argsCount]);
    				++argsCount;				
    			}
    			else if ("-tokenServiceUrl".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.TokenUrlV2(args[++argsCount]);
    				++argsCount;				
    			}
    			else if ("-serviceDiscoveryUrl".Equals(args[argsCount]))
    			{
					if(argsCount < (args.Length - 1))
    					config.ServiceDiscoveryUrl(args[++argsCount]);
    				++argsCount;				
    			}
    			else // unrecognized command line argument
    			{
    				PrintHelp();
    				return false;
    			}			
    		}
	        
	        if (clientSecret == null || clientId == null)
			{
				Console.WriteLine("clientId and clientSecret all must be specified on the command line. Exiting...");
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

	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
		try
		{
			OmmConsumerConfig config = new();
			
			if (!ReadCommandlineArgs(args, config))
                return;
			
			AppClient appClient = new();
			
			consumer  = new OmmConsumer(config.ConsumerName("Consumer_4"));
			
			consumer.RegisterClient( new RequestMsg().ServiceName("ELEKTRON_DD").Name("BBL.BK"), appClient);
			
			Thread.Sleep(900000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
		}
        catch (ThreadInterruptedException threadInterruptedException)
        {
            Console.WriteLine(threadInterruptedException.Message);
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
        finally 
		{
			consumer?.Uninitialize();
		}
	}
}
