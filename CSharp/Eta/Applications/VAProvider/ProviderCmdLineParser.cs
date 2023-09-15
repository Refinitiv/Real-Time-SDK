/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.ValueAdd.Provider
{
    internal class ProviderCmdLineParser : ICommandLineParser
    {
		public string? PortNo { get; set; }
		public string? InterfaceName { get; set; }
		public string? ServiceName { get; set; }
		public int ServiceId { get; set; }
		public int Runtime { get; set; } = 1200; // default runtime is 1200 seconds
		public bool EnableXmlTracing { get; set; }
		public bool EnableRtt { get; set; }
		public string? KeyCertificate { get; set; } = null;
		public string? PrivateKey { get; set; } = null;
		public ConnectionType ConnectionType { get; set; } = ConnectionType.SOCKET;

		public bool ParseArgs(string[] args)
		{
			try
			{
				int argsCount = 0;

				while (argsCount < args.Length)
				{
					if ("-p".Equals(args[argsCount]))
					{
						PortNo = args[++argsCount];
						++argsCount;
					}
					else if ("-i".Equals(args[argsCount]))
					{
						InterfaceName = args[++argsCount];
						++argsCount;
					}
					else if ("-s".Equals(args[argsCount]))
					{
						ServiceName = args[++argsCount];
						++argsCount;
					}
					else if ("-id".Equals(args[argsCount]))
					{
						ServiceId = int.Parse(args[++argsCount]);
						++argsCount;
					}
					else if ("-runtime".Equals(args[argsCount]))
					{
						Runtime = int.Parse(args[++argsCount]);
						++argsCount;
					}
					else if ("-x".Equals(args[argsCount]))
					{
						EnableXmlTracing = true;
						++argsCount;
					}
					else if ("-rtt".Equals(args[argsCount]))
					{
						EnableRtt = true;
						++argsCount;
					}
					else if ("-c".Equals(args[argsCount]))
					{
						++argsCount;
						if ("socket".Equals(args[argsCount]))
						{
							ConnectionType = ConnectionType.SOCKET;
						}
						else if ("encrypted".Equals(args[argsCount]))
						{
							ConnectionType = ConnectionType.ENCRYPTED;
						}
						else
						{
							Console.WriteLine("\nUnrecognized connection type...\n");
							return false;
						}
						++argsCount;
					}
					else if ("-cert".Equals(args[argsCount]))
					{
						KeyCertificate = args[++argsCount];
						++argsCount;
					}
					else if ("-keyfile".Equals(args[argsCount]))
					{
						PrivateKey = args[++argsCount];
						++argsCount;
					}
					else // unrecognized command line argument
					{
						Console.WriteLine("\nUnrecognized command line argument...\n");
						return false;
					}
				}
			}
			catch (Exception e)
			{
				Console.WriteLine($"\nInvalid command line arguments: {e.Message}");
				return false;
			}

			return true;
		}

		public void PrintUsage()
		{
			Console.WriteLine("Usage: Provider or\nProvider [-p <port number>] [-i <interface name>] [-s <service name>] [-id <service ID>] [-runtime <seconds>]" +
							   "\n -p server port number (defaults to 14002)\n" +
							   "\n -i interface name (defaults to null)\n" +
							   "\n -s service name (defaults to DIRECT_FEED)\n" +
							   "\n -id service id (defaults to 1)\n" +
							   "\n -x provides an XML trace of messages\n" +
							   "\n -runtime application runtime in seconds (default is 1200)" +
							   "\n -rtt application (provider) supports calculation of Round Trip Latency" +
							   "\n -c Provider connection type.  Either \"socket\" or \"encrypted\"" +
							   "\n -key the server private key file" +
							   "\n -cert the server certificate file");
		}


	}
}
