package com.thomsonreuters.upa.valueadd.examples.provider;

import com.thomsonreuters.upa.valueadd.examples.common.CommandLineParser;

class ProviderCmdLineParser implements CommandLineParser
{
	private String portNo;
	private String interfaceName;
	private String serviceName;
	private int serviceId;
	private int runtime = 1200; // default runtime is 1200 seconds
	private boolean enableXmlTracing;
	private boolean cacheOption;

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
		System.out.println("Usage: Provider or\nProvider [-p <port number>] [-i <interface name>] [-s <service name>] [-id <service ID>] [-runtime <seconds>]" +
				"\n -p server port number (defaults to 14002)\n" +
				"\n -i interface name (defaults to null)\n" +
				"\n -s service name (defaults to DIRECT_FEED)\n" +
				"\n -id service id (defaults to 1)\n" +
				"\n -x provides an XML trace of messages\n" +
				"\n -cache turn on the cache feature of the application\n" +
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
}
