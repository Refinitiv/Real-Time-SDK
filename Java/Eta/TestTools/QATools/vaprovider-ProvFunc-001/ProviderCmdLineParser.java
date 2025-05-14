/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;

import com.refinitiv.eta.valueadd.examples.common.CommandLineParser;

class ProviderCmdLineParser implements CommandLineParser
{
	private String portNo;
	private String interfaceName;
	private String serviceName;
	private int serviceId;
	private int runtime = 1200; // default runtime is 1200 seconds
	private boolean enableXmlTracing;
	private boolean cacheOption;
	private boolean supportView;
	private int openWindow=0; // 0 will not set

	@Override
	public boolean parseArgs(String[] args)
	{
	    try
	    {
    		int argsCount = 0;
    		
    		while (argsCount < args.length)
    		{
    			if ("-p".equals(args[argsCount]))
    			{
    				portNo = args[++argsCount];
    				++argsCount;
    			}
    			else if ("-i".equals(args[argsCount]))
    			{
    				interfaceName = args[++argsCount];
    				++argsCount;
    			}
    			else if ("-s".equals(args[argsCount]))
    			{
    				serviceName = args[++argsCount];
    				++argsCount;
    			}
    			else if ("-id".equals(args[argsCount]))
    			{
    				serviceId = Integer.parseInt(args[++argsCount]);
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
    			else if ("-supportView".equals(args[argsCount]))
    			{
    				supportView =  true;
    				++argsCount;								
    			}
    			else if ("-openWindow".equals(args[argsCount]))
    			{
    				openWindow = Integer.parseInt(args[++argsCount]);
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

	@Override
	public void printUsage()
	{
		System.out.println("Usage: Provider or\nProvider [-p <port number>] [-i <interface name>] [-s <service name>] [-id <service ID>] [-supportView <false>] [-openWindow <openWindow>] [-runtime <seconds>]" +
				"\n -p server port number (defaults to 14002)\n" +
				"\n -i interface name (defaults to null)\n" +
				"\n -s service name (defaults to DIRECT_FEED)\n" +
				"\n -id service id (defaults to 1)\n" +
				"\n -x provides an XML trace of messages\n" +
				"\n -cache turn on the cache feature of the application\n" +
				"\n -supportView sets supportView on Login Response\n" +
				"\n -openWindow sets value of openWindows on Directory Response\n" +
				"\n -runtime application runtime in seconds (default is 1200)");
	}

	String portNo()
	{
		return portNo;
	}

	String interfaceName()
	{
		return interfaceName;
	}

	String serviceName()
	{
		return serviceName;
	}

	int serviceId()
	{
		return serviceId;
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
	
	boolean supportView()
	{
		return supportView;
	}
	
	int openWindow()
	{
		return openWindow;
	}

}
