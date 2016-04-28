package com.thomsonreuters.upa.valueadd.examples.queueconsumer;

import com.thomsonreuters.upa.transport.ConnectionTypes;

import java.util.ArrayList;
import java.util.List;

/*
 * Command line parser for the Value Add queue consumer application.
 */
class ConsumerCmdLineParser 
{
	private String userName;

	private int runtime = 600; // default runtime is 600 seconds
	private boolean enableXmlTracing;

	private ConnectArg connectArg;
	private String qSourceName;
    private List<String> qDestNameList = new ArrayList<String>();	

    boolean parseArgs(String[] args)
	{
	    try
	    {
    		int argsCount = 0;
    		
    		while (argsCount < args.length)
    		{
    			if ("-c".equals(args[argsCount]))
    			{
    				argsCount = parseSocketConnectionArgs(args, argsCount);
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
                else if ("-qServiceName".equals(args[argsCount]))
                {
                	if ( connectArg == null)
                		connectArg = new ConnectArg();
                	connectArg.service = args[++argsCount];
                    ++argsCount;
                }    			
                else if ("-qSourceName".equals(args[argsCount]))
                {
                    qSourceName = args[++argsCount];
                    ++argsCount;
                }
                else if ("-qDestName".equals(args[argsCount]))
                {
                    qDestNameList.add(args[++argsCount]);
                    ++argsCount;
                }
				else if ("-tsAuth".equals(args[argsCount]))
				{
					connectArg.tunnelAuth = true;
                    ++argsCount;
				}
				else if ("-tsDomain".equals(args[argsCount]))
				{
					connectArg.tunnelDomain = Integer.valueOf(args[++argsCount]);
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
	
	private int parseSocketConnectionArgs(String[] args, int argOffset)
	{
		int retCode = -1;
		
		if ((args.length-1) >= argOffset+3)
		
			if (args[argOffset+1].contains(":") && !args[argOffset+2].contains(":"))
			{
				String[] tokens = args[argOffset+1].split(":");
				if (tokens.length == 2)
				{
					if ( connectArg == null ) connectArg = new ConnectArg();
					connectArg.connectionType = ConnectionTypes.SOCKET;

					connectArg.hostname = tokens[0];
					connectArg.port = tokens[1];
					retCode = argOffset + 2;
				}
				else
				{
					return retCode;
				}
			}
			else
			{
				return retCode;
			}
		else
			return retCode;
					
		return retCode;
	}
	

	ConnectArg connection()
	{
		return connectArg;
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


    String qSourceName()
    {
        return qSourceName;
    }
	
    List<String> qDestNames()
    {
        return qDestNameList;
    }
    
	void printUsage()
	{
		System.out.println("Usage: QueueConsumer -c <hostname>:<port> -qServiceName <queue service name> -qSourceName <queue source name> -qDestName <queue destination name>" +
				"\n -c specifies a connection to open and a list of items to request:\n" +
				"\n     hostname:        Hostname of provider to connect to" +
				"\n     port:            Port of provider to connect to" +
				"\n -uname changes the username used when logging into the provider\n" +
				"\n -qServiceName specifies the service name for queue messages\n" +
		        "\n -qSourceName specifies the source name for queue messages (if specified, configures consumer to receive queue messages)\n" +
		        "\n -qDestName specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified)\n" +
				"\n -tsAuth (optional) causes consumer to request authentication when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging.\n" +
				"\n -tsDomain (optional) specifes the domain a consumer uses when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging.\n" +
  				"\n -x provides an XML trace of messages\n" +
				"\n -runtime adjusts the running time of the application");
	}
}

