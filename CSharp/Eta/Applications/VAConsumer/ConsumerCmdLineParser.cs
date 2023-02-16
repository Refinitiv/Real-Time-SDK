/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Consumer
{
    /// <summary>Command line parser for the Value Add consumer application.</summary>
    internal class ConsumerCmdLineParser : ICommandLineParser
    {
        #region Configuration Properties
        internal string? BackupHostname { get; private set; }

        internal string? BackupPort { get; private set; }

        internal List<ConnectionArg> ConnectionList
        {
            get => m_ConnectionArgsParser.ConnectionList;
        }

        internal string? UserName { get; private set; }

        internal string? Passwd { get; private set; }

        internal string? ClientId { get; private set; }

        internal string? ClientSecret { get; private set; }

        internal string? JwkFile { get; private set; }

        internal string? Audience { get; private set; }

        internal string? TokenURL { get; private set; }

        internal string? serviceDiscoveryURL { get; private set; }

        internal string? TokenScope { get; private set; }

        internal bool EnableView { get; private set; }

        internal bool EnablePost { get; private set; }

        internal bool EnableOffpost { get; private set; }

        internal bool EnableSnapshot { get; private set; }

        internal string? PublisherId { get; private set; }

        internal string? PublisherAddress { get; private set; }

        internal TimeSpan Runtime { get; private set; } = TimeSpan.FromSeconds(600); // default runtime is 600 seconds

        internal bool EnableXmlTracing { get; private set; }

        internal bool EnableEncrypted { get; private set; }

        internal bool EnableProxy { get; private set; }

        internal bool EnableSessionMgnt { get; private set; }

        internal ConnectionType EncryptedProtocolType { get; private set; }

        internal string? ProxyHostname { get; private set; }

        internal string? ProxyPort { get; private set; }

        internal string? ProxyUsername { get; private set; } = "";

        internal string? ProxyPasswd { get; private set; } = "";

        internal string? AuthenticationToken { get; private set; }

        internal string? AuthenticationExtended { get; private set; }

        internal string? ApplicationId { get; private set; }

        internal bool EnableRtt { get; private set; }

        internal bool EnableRestLogging { get; private set; }

        internal string? RestLoggingFileName { get; private set; }

        #endregion

        private ConnectionArgsParser m_ConnectionArgsParser = new ConnectionArgsParser();


        public bool ParseArgs(string[] args)
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
                    if (args[argsCount + 1].Contains(':'))
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
                else if ("-passwd".Equals(args[argsCount]))
                {
                    Passwd = args[++argsCount];
                    ++argsCount;
                }
                else if ("-sessionMgnt".Equals(args[argsCount]))
                {
                    EnableSessionMgnt = true;
                    ++argsCount;
                }
                else if ("-clientId".Equals(args[argsCount]))
                {
                    ClientId = args[++argsCount];
                    ++argsCount;
                }
                else if ("-clientSecret".Equals(args[argsCount]))
                {
                    ClientSecret = args[++argsCount];
                    ++argsCount;
                }
                else if ("-jwkFile".Equals(args[argsCount]))
                {
                    JwkFile = args[++argsCount];
                    ++argsCount;
                }
                else if ("-audience".Equals(args[argsCount]))
                {
                    Audience = args[++argsCount];
                    ++argsCount;
                }
                else if ("-tokenURL".Equals(args[argsCount]))
                {
                    TokenURL = args[++argsCount];
                    ++argsCount;
                }
                else if ("-serviceDiscoveryURL".Equals(args[argsCount]))
                {
                    serviceDiscoveryURL = args[++argsCount];
                    ++argsCount;
                }
                else if ("-tokenScope".Equals(args[argsCount]))
                {
                    TokenScope = args[++argsCount];
                    ++argsCount;
                }
                else if ("-view".Equals(args[argsCount]))
                {
                    EnableView = true;
                    ++argsCount;
                }
                else if ("-post".Equals(args[argsCount]))
                {
                    EnablePost = true;
                    ++argsCount;
                }
                else if ("-offpost".Equals(args[argsCount]))
                {
                    EnableOffpost = true;
                    ++argsCount;
                }
                else if ("-publisherInfo".Equals(args[argsCount]))
                {
                    String value = args[++argsCount];
                    if (value != null)
                    {
                        String[] pieces = value.Split(",");

                        if (pieces.Length > 1)
                        {
                            PublisherId = pieces[0];

                            PublisherAddress = pieces[1];
                        }
                        else
                        {
                            Console.WriteLine("Error loading command line arguments for publisherInfo [id, address]:\t");
                            return false;
                        }
                    }
                    ++argsCount;
                }
                else if ("-snapshot".Equals(args[argsCount]))
                {
                    EnableSnapshot = true;
                    ++argsCount;
                }
                else if ("-runtime".Equals(args[argsCount]))
                {
                    if (Int32.TryParse(args[++argsCount], out var runTimeSeconds))
                    {
                        Runtime = TimeSpan.FromSeconds(runTimeSeconds);
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse runtime '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-x".Equals(args[argsCount]))
                {
                    EnableXmlTracing = true;
                    ++argsCount;
                }
                else if ("-connectionType".Equals(args[argsCount]))
                {
                    // will overwrite connectionArgsParser's connectionList's connectionType based on the flag
                    string connectionType = args[++argsCount];
                    ++argsCount;
                    if (connectionType.Equals("encrypted"))
                    {
                        EnableEncrypted = true;
                    }
                }
                else if ("-encryptedProtocolType".Equals(args[argsCount]))
                {
                    // will overwrite connectionArgsParser's connectionList's connectionType based on the flag
                    String connectionType = args[++argsCount];
                    ++argsCount;
                    if (connectionType.Equals("socket"))
                    {
                        EncryptedProtocolType = ConnectionType.SOCKET;
                    }
                }
                else if ("-proxy".Equals(args[argsCount]))
                {
                    EnableProxy = true;
                    ++argsCount;
                }
                else if ("-ph".Equals(args[argsCount]))
                {
                    ProxyHostname = args[++argsCount];
                    ++argsCount;
                }
                else if ("-pp".Equals(args[argsCount]))
                {
                    ProxyPort = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else if ("-plogin".Equals(args[argsCount]))
                {
                    ProxyUsername = args[++argsCount];
                    ++argsCount;
                }
                else if ("-ppasswd".Equals(args[argsCount]))
                {
                    ProxyPasswd = argsCount < (args.Length - 1) ? args[++argsCount] : null;
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
                else if ("-rtt".Equals(args[argsCount]))
                {
                    EnableRtt = true;
                    ++argsCount;
                }
                else if ("-restEnableLog".Equals(args[argsCount]))
                {
                    EnableRestLogging = true;
                    ++argsCount;
                }
                else if ("-restLogFileName".Equals(args[argsCount]))
                {
                    RestLoggingFileName = args[++argsCount];
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    Console.WriteLine($"\nUnrecognized command line argument '{args[argsCount]}'\n");
                    return false;
                }
            }

            return true;
        }


        public void PrintUsage()
        {
            Console.WriteLine("Usage: Consumer or\nConsumer [-c <hostname>:<port> <service name> <domain>:<item name>,...] [-bc <hostname>:<port>] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>]" +
                               "\n -c specifies a connection to open and a list of items to request:\n" +
                               "\n     hostname:        Hostname of provider to connect to" +
                               "\n     port:            Port of provider to connect to" +
                               "\n     service:         Name of service to request items from on this connection" +
                               "\n     domain:itemName: Domain and name of an item to request" +
                               "\n         A comma-separated list of these may be specified." +
                               "\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)" +
                               "\n         The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)" +
                               "\n         Example Usage: -c localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl" +
                               "\n           (for SymbolList requests, a name can be optionally specified)\n" +
                               "\n -bc specifies a backup connection that is attempted if the primary connection fails\n" +
                               "\n -uname changes the username used when logging into the provider\n" +
                               "\n -passwd changes the password used when logging into the provider\n" +
                               "\n -clientId specifies a unique ID for application making the request to RDP token service\n" +
                               "\n -clientSecret specifies the associated secret with the client ID\n" +
                               "\n -jwkFile specifies a file containing the JWK encoded private key for V2 JWT logins.\n" +
                               "\n -audience audience claim for v2 JWT logins.\n" +
                               "\n -sessionMgnt enables the session management in the Reactor\n" +
                               "\n -tokenURL specifies the URL for the token service to override the default value.\n" +
                               "\n -serviceDiscoveryURL specifies the RDP Service Discovery URL to override the default value.\n" +
                               "\n -tokenScope specifies a scope for the token service.\n" +
                               "\n -view specifies each request using a basic dynamic view\n" +
                               "\n -post specifies that the application should attempt to send post messages on the first requested Market Price item\n" +
                               "\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n" +
                               "\n -publisherInfo specifies that the application should add user provided publisher Id and publisher ipaddress when posting\n" +
                               "\n -snapshot specifies each request using non-streaming\n" +
                               "\n -connectionType specifies the connection type that the connection should use (possible values are: 'socket', 'encrypted')\n" +
                               "\n -encryptedProtocolType specifies the encrypted protocol type that the connection should use (possible values are: 'socket')\n" +
                               "\n -proxy specifies that proxy is used for connectionType\n" +
                               "\n -ph specifies proxy server host name\n" +
                               "\n -pp specifies proxy port number\n" +
                               "\n -plogin specifies user name on proxy server\n" +
                               "\n -ppasswd specifies password on proxy server\n" +
                               "\n -x provides an XML trace of messages\n" +
                               "\n -runtime adjusts the running time of the application" +
                               "\n -aid Specifies the Application ID" +
                               "\n -restEnableLog enable REST logging message" +
                               "\n -restLogFileName set REST logging output stream" +
                               "\n -rtt Enables rtt support by a consumer. If provider makes distribution of RTT messages, consumer will return back them. In another case, consumer will ignore them.");
        }
    }
}
