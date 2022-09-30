/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Example.VACommon;
using Refinitiv.Eta.Transports;

namespace Refinitiv.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// Command line parser for the ValueAdd NIProvider application.
    /// </summary>
    public class NIProviderCmdLineParser : ICommandLineParser
    {
		private ConnectionArgsParser m_ConnectionArgsParser = new ConnectionArgsParser();
		
        public bool HasBackupConfig { get; set; } = false;
        public ConnectionType BackupConnectionType { get; set; }
        public string? BackupHostname { get; set; }
        public string? BackupPort { get; set; }
        public string? UserName { get; set; }
        public int Runtime { get; set; } = 600; // default runtime is 600 seconds
        public bool EnableXmlTracing { get; set; }
        public string? AuthenticationToken { get; set; }
        public string? AuthenticationExtended { get; set; }
        public string? ApplicationId { get; set; }

        /// <summary>
        /// The list of connection endpoints
        /// </summary>
        public List<ConnectionArg> ConnectionList { get => m_ConnectionArgsParser.ConnectionList; }

        /// <summary>
        /// Parses command line arguments
        /// </summary>
        /// <param name="args">input array of command line arguments</param>
        /// <returns><see cref="true"/> in case the operation was successful, <see cref="false"/> otherwise</returns>
        public bool ParseArgs(string[] args)
        {
            try
            {
                int argsCount = 0;

                while (argsCount < args.Length)
                {
                    if (m_ConnectionArgsParser.IsStart(args, argsCount))
                    {
                        if ((argsCount = m_ConnectionArgsParser.Parse(args, argsCount)) < 0)
                        {
                            // error
                            Console.WriteLine("\nError parsing connection arguments...\n");
                            return false;
                        }
                    }
                    else if ("-bc".Equals(args[argsCount]))
                    {
                        HasBackupConfig = true;
                        BackupConnectionType = ConnectionType.SOCKET;
                        if (args[argsCount + 1].Contains(":"))
                        {
                            string[] tokens = args[++argsCount].Split(":");
                            if (tokens.Length == 2)
                            {
                                BackupHostname = tokens[0];
                                BackupPort = tokens[1];
                                ++argsCount;
                            }
                            else
                            {
                                // error
                                Console.WriteLine("\nError parsing backup connection arguments...\n");
                                return false;
                            }
                        }
                        else
                        {
                            // error
                            Console.WriteLine("\nError parsing backup connection arguments...\n");
                            return false;
                        }
                    }
                    else if ("-uname".Equals(args[argsCount]))
                    {
                        UserName = args[++argsCount];
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
                    else if ("-at".Equals(args[argsCount]))
                    {
                        AuthenticationToken = args[++argsCount];
                        ++argsCount;
                    }
                    else if ("-ax".Equals(args[argsCount]))
                    {
                        AuthenticationExtended = args[++argsCount];
                        ++argsCount;
                    }
                    else if ("-aid".Equals(args[argsCount]))
                    {
                        ApplicationId = args[++argsCount];
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

        /// <summary>
        /// Prints out usage summary
        /// </summary>
        public void PrintUsage()
        {
            Console.WriteLine("Usage: NIProvider or\nNIProvider [-c <hostname>:<port> <service name> <domain>:<item name>,...] " +
                    "[-bc <hostname>:<port>] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>]" +
                    "\n -c specifies a connection to open and a list of items to provide:\n" +
                    "\n     hostname:        Hostname of ADH to connect to" +
                    "\n     port:            Port of ADH to connect to" +
                    "\n     service:         Name of service to provide items on this connection" +
                    "\n     domain:itemName: Domain and name of an item to provide" +
                    "\n         A comma-separated list of these may be specified." +
                    "\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder)" +
                    "\n         Example Usage: -c localhost:14002 NI_PUB mp:TRI,mp:GOOG,mbo:MSFT\n" +
                    "\n -bc specifies a backup connection that is attempted if the primary connection fails (addr:port)\n" +
                    "\n -uname changes the username used when logging into the provider\n" +
                    "\n -x provides an XML trace of messages\n" +
                    "\n -runtime adjusts the running time of the application" +
                    "\n -at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN" +
                    "\n -ax Specifies the Authentication Extended information" +
                    "\n -aid Specifies the Application ID");
        }
    }
}
