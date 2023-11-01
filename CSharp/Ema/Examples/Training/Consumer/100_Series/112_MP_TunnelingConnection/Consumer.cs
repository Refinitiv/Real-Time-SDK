/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _) =>
        Console.WriteLine(refreshMsg);

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) =>
        Console.WriteLine(updateMsg);

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) =>
        Console.WriteLine(statusMsg);
}

public class Consumer 
{
	static void PrintHelp()
	{

        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n" 
	    		+ "  if the application will attempt to make an http or encrypted \n "
	    		+ "           connection, ChannelType must need to be set to \n"
	    		+ "            ChannelType::RSSL_ENCRYPTED in EMA configuration file.\n"
	    		+ "  -ph Proxy host name.\n"
	    		+ "  -pp Proxy port number.\n"
	    		+ "  -plogin User name on proxy server.\n"
	    		+ "  -ppasswd Password on proxy server.\n"
	    		+ "\n");
	}

	static bool ReadCommandlineArgs(String[] args, OmmConsumerConfig config)
	{
		    try
		    {
		        int argsCount = 0;

		        while (argsCount < args.Length)
		        {
		            if (args[argsCount] == "-?")
		            {
		                PrintHelp();
		                return false;
		            }
	    			else if (args[argsCount] == "-ph")
	    			{
	    				config.ProxyHost(args[++argsCount]);
	    				++argsCount;				
	    			}	
	    			else if (args[argsCount]== "-pp")
	    			{
						if(argsCount < (args.Length - 1))
	    					config.ProxyPort(args[++argsCount]);
	    				++argsCount;				
	    			}			
	    			else if (args[argsCount] == "-plogin")
	    			{
						if(argsCount < (args.Length - 1))
	    					config.ProxyUserName(args[++argsCount]);
	    				++argsCount;				
	    			}	
	    			else if (args[argsCount] == "-ppasswd")
	    			{
						if (argsCount < (args.Length - 1))
							config.ProxyPassword(args[++argsCount]);
	    				++argsCount;				
	    			}	
	    			else // unrecognized command line argument
	    			{
	    				PrintHelp();
	    				return false;
	    			}			
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
			
			consumer = new OmmConsumer(config.ConsumerName("Consumer_3"));
			
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 0);
			
			Thread.Sleep(60000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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


