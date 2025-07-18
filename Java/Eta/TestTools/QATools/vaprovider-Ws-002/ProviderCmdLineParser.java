/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;

import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.examples.common.CommandLineParser;
import com.refinitiv.eta.transport.CompressionTypes;

class ProviderCmdLineParser implements CommandLineParser
{
	private String portNo;
	private String interfaceName;
	private String serviceName;
	private int serviceId;
	private int runtime = 1200; // default runtime is 1200 seconds
	private boolean enableXmlTracing;
	private boolean cacheOption;
	private boolean enableRtt;
	private String keyfile = null;
	private String keypasswd = null;
	private int connType = ConnectionTypes.SOCKET;
	private String protocolList = "rssl.rwf, tr_json2, rssl.json.v2";
	private int compressType = CompressionTypes.NONE;
	private int compressLevel = 0;
	private int maxFragmentSize = 6144;

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
				} else if ("-rtt".equals(args[argsCount])) {
					enableRtt = true;
					++argsCount;
				} else if ("-c".equals(args[argsCount])) {
					++argsCount;
					if("socket".equals(args[argsCount]))
					{
						connType = ConnectionTypes.SOCKET;
					}
					else if("encrypted".equals(args[argsCount]))
					{
						connType = ConnectionTypes.ENCRYPTED;
					}
					else
					{
						System.out.println("\nUnrecognized connection type...\n");
						return false;
					}
					++argsCount;
				} else if ("-keyfile".equals(args[argsCount])) {
					keyfile = args[++argsCount];
					++argsCount;
				} else if ("-keypasswd".equals(args[argsCount])) {
					keypasswd = args[++argsCount];
					++argsCount;
				} else if ("-pl".equals(args[argsCount])) {
					protocolList = args[++argsCount];
					++argsCount;
				}
				//API QA
				else if ("-testCompressionZlib".equals(args[argsCount]))
				{
					compressType = CompressionTypes.ZLIB;
					++argsCount;
				}
				else if ("-compressionLevel".equals(args[argsCount]))
				{
					compressLevel = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				else if ("-maxFragmentSize".equals(args[argsCount]))
				{
					maxFragmentSize = Integer.parseInt(args[++argsCount]);
					++argsCount;
				}
				//END API QA
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
						   "\n -pl protocol list (defaults to rssl.rwf, tr_json2, rssl.json.v2)\n" +
						   "\n -runtime application runtime in seconds (default is 1200)" +
						   "\n -rtt application (provider) supports calculation of Round Trip Latency" +
						   "\n -c Provider connection type.  Either \"socket\" or \"encrypted\"" +
						   "\n -keyfile jks encoded keyfile for Encrypted connections" +
						   "\n -keypasswd password for keyfile" +
						   "\n -testCompressionZlib for testing compressionType=Zlib" +
						   "\n -compressionLevel for identifying compression level" +
						   "\n -maxFragmentSize for identifying maxFragmentSize");
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

	boolean enableRtt()
	{
		return enableRtt;
	}

	int connectionType()
	{
		return connType;
	}

	String keyfile()
	{
		return keyfile;
	}

	String keypasswd()
	{
		return keypasswd;
	}

	String protocolList()
	{
		return protocolList;
	}
	//API QA
	int compressionType()
	{
		return compressType;
	}
	int compressionLevel()
	{
		return compressLevel;
	}
	int maxFragmentSize()
	{
		return maxFragmentSize;
	}
    //End API QA
	
}
