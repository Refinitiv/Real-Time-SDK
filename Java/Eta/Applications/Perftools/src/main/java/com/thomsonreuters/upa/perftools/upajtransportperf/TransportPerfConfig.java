package com.thomsonreuters.upa.perftools.upajtransportperf;

import java.io.File;
import java.io.IOException;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.shared.CommandLine;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.WriteFlags;

/** Configures the upajTransportPerf application. */
public class TransportPerfConfig
{
    public static int SERVER = 1;
    public static int CLIENT = 2;

    private static String       _configString;

    private static int          _runTime;                   // Time application runs before exiting
    private static int          _threadCount;               // Number of threads that handle connections
    private static int          _appType;                   // Type of application(server, client)
    private static String       _appTypeString;
    private static boolean      _reflectMsgs;               // Reflect received messages instead of generating them
    
    private static boolean      _busyRead;                  // If set, the application will continually read
                                                            // rather than using notification.
                                                            // Messages cannot be sent in this mode
    
    private static int          _connectionType;            // Type of connection
    private static String       _connectionTypeString;      
    private static String       _portNo;                    // Port number
    private static String       _interfaceName;             // Network interface to bind to
    private static boolean      _tcpNoDelay;                // TCP_NODELAY option for socket
    private static int          _guaranteedOutputBuffers;   // Guaranteed Output Buffers
    private static int          _maxFragmentSize;           // Max fragment size
    private static int          _highWaterMark;             // High water mark
    private static int          _sendBufSize;               // System send buffer size
    private static int          _recvBufSize;               // System receive buffer size
    
    private static String       _summaryFilename;           // Summary file
    private static int          _writeStatsInterval;        // Controls how often statistics are written 
    private static boolean      _displayStats;              // Controls whether stats appear on the screen

    private static int          _compressionType;           // Type of compression to use, if any
    private static String       _compressionTypeString;
    private static int          _compressionLevel;          // Compression level, optional depending on compression algorithm used
    private static String       _hostName;                  // hostName, if using rsslConnect()
    private static String       _sendAddr;                  // Outbound address, if using a multicast connection
    private static String       _recvAddr;                  // Inbound address, if using a multicast connection
    private static String       _sendPort;                  // Outbound port, if using a multicast connection
    private static String       _recvPort;                  // Inbound port, if using a multicast connection
    private static String       _unicastPort;               // Unicast port, if using a mulicast connection
    private static boolean      _sAddr;                     // Whether an outbound address was specified
    private static boolean      _rAddr;                     // Whether an inbound address was specified
    private static boolean      _takeMCastStats;            // Running a multicast connection and we want stats

    private static String 		_tcpControlPort;
    private static int			_portRoamRange;
    
    private static boolean      _proxy;                     // Specifies that the application will make a tunneling connection (http or encrypted)
                                                            // through a proxy server   
    private static String       _proxyHost;                 // proxy host address
    private static int          _proxyPort;                 // proxy port number
    private static String       _proxyUserName;             // proxy login id
    private static String       _proxyPasswd;               // proxy login password
    private static String       _proxyDomain;               // proxy domain
    private static String       _proxyKRBConfigFile;        // proxy config file
    private static String       _keystoreFile;              // security keystore file
    private static String       _keystorePasswd;            // security keystore password
    
    static
    {
        try
        {
            File summaryFile = File.createTempFile("TransportSummary_", ".out",  new File(System.getProperty("user.dir")));
            _summaryFilename = summaryFile.getName();
            /* Temp summary file is created here to get file name only. Delete this file */ 
            summaryFile.delete();
        }
        catch (IOException ioe)
        {
            System.err.println("error: " + ioe.getMessage());
            _summaryFilename = "TransportSummary.out";
        }
    }
    static
    {
        CommandLine.programName("upajTransportPerf");
        CommandLine.addOption("runTime", 300, "Runtime of the application, in seconds");
        CommandLine.addOption("summaryFile", _summaryFilename, "Name of file for logging summary info");
        CommandLine.addOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
        CommandLine.addOption("noDisplayStats", false, "Stop printout of stats to screen");
        CommandLine.addOption("threads", 1, "Number of transport threads to create");
        CommandLine.addOption("connType", "socket", "Type of connection(\"socket\", \"http (client only)\", \"encrypted (client only)\", \"reliableMCast\", \"shmem\", \" seqMCast\")");
        CommandLine.addOption("reflectMsgs", false, "Reflect received messages back, rather than generating our own");
        CommandLine.addOption("outputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in BindOptions/ConnectOptions)");
        CommandLine.addOption("maxFragmentSize", 6144, " Max size of buffers(configures maxFragmentSize in BindOptions/ConnectOptions)");
        CommandLine.addOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in BindOptions/ConnectOptions)");
        CommandLine.addOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in BindOptions/ConnectOptions)");
        CommandLine.addOption("compressionType", "none", "Type of compression to use(\"none\", \"zlib\", \"lz4\")");
        CommandLine.addOption("compressionLevel", 5, "Level of compression");
        CommandLine.addOption("highWaterMark", 0, "Sets the point that causes UPA to automatically flush");
        CommandLine.addOption("if", "", "Name of network interface to use");
        CommandLine.addOption("h", "localhost", "Name of host for socket-based connections");
        CommandLine.addOption("p", "14002", "Port number for socket-based connections");
        CommandLine.addOption("tcpDelay", false, "Turns off tcp_nodelay in BindOptions, enabling Nagle's");
        CommandLine.addOption("sa", "", "Send address for segmented multicast connections");
        CommandLine.addOption("ra", "", "Receive address for segmented multicast connections");
        CommandLine.addOption("sp", "", "Send port for segmented multicast connections");
        CommandLine.addOption("rp", "", "Receive port for segmented multicast connections");
        CommandLine.addOption("u", "", "Unicast port for segmented multicast connections");
        CommandLine.addOption("tp", "", "TCP Control Port");
        CommandLine.addOption("prr", 0, "Port Roam Range");
        CommandLine.addOption("mcastStats", false, "Take Multicast Statistics(Warning: This enables the per-channel lock)");
        CommandLine.addOption("appType", "server", "Type of application(server, client)");
        CommandLine.addOption("busyRead", false, "Continually read instead of using notification");
        CommandLine.addOption("proxy", false, "Use tunneling connection");
        CommandLine.addOption("ph", "localhost", "Address of proxy host");
        CommandLine.addOption("pp", "14002", "Port number for proxy server");
        CommandLine.addOption("plogin", "John.Doe", "Login id on proxy server");
        CommandLine.addOption("ppasswd", "", "Password on proxy server");
        CommandLine.addOption("pdomain", "", "Proxy domain");
        CommandLine.addOption("krbfile", "C:\\Kerberos\\krb5.conf", "proxyKRBConfigFile");
        CommandLine.addOption("keyfile", "C:\\Certificates\\internet.jks", "keystoreFile");
        CommandLine.addOption("keypasswd", "", "keystore password");
        
        // TransportThreadConfig
        CommandLine.addOption("tickRate", 1000, "Ticks per second");
        CommandLine.addOption("pack", 1, "Number of messages packed in a buffer(when count > 1, Channel.packBuffer() is used)");
        CommandLine.addOption("msgRate", 100000, "Message rate per second");
        CommandLine.addOption("msgSize", 76, "Size of messages to send");
        CommandLine.addOption("latencyMsgRate", 10, "Latency Message rate (can specify \"all\" to send it as every msg)");
        CommandLine.addOption("directWrite", false, "Sets direct socket write flag when using Channel.write()");
        CommandLine.addOption("latencyFile", "", "Base name of file for logging latency");        
        CommandLine.addOption("statsFile", "TransportStats", "Base name of file for logging periodic statistics");
        TransportThreadConfig.checkPings(true);
    }
     
    private TransportPerfConfig()
    {
        
    }

    /**
     * Parses command-line arguments to fill in the application's configuration
     * structures.
     *
     * @param args the args
     */
    public static void init(String[] args)
    {
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }

        try
        {
            _runTime = CommandLine.intValue("runTime");
            _summaryFilename = CommandLine.value("summaryFile");
            TransportThreadConfig.statsFilename(CommandLine.value("statsFile"));
            _writeStatsInterval = CommandLine.intValue("writeStatsInterval");
            _displayStats = !CommandLine.booleanValue("noDisplayStats");
            _threadCount = CommandLine.intValue("threads");
			if (_threadCount < 1)
			{
				_threadCount = -1;
				System.out.println(CommandLine.optionHelpString());
				System.exit(-1);
			}
            _connectionTypeString = CommandLine.value("connType");
            if (_connectionTypeString.equals("socket"))
            {
                _connectionType = ConnectionTypes.SOCKET;
            }
            else if (_connectionTypeString.equals("http")) 
            {
                _connectionType = ConnectionTypes.HTTP;
            }
            else if (_connectionTypeString.equals("encrypted"))
            {
                _connectionType = ConnectionTypes.ENCRYPTED;
            }
            else if (_connectionTypeString.equals("reliableMCast"))
            {
                _connectionType = ConnectionTypes.RELIABLE_MCAST;
            }
            else if (_connectionTypeString.equals("shmem"))
            {
                _connectionType = ConnectionTypes.UNIDIR_SHMEM;
            }
              else if (_connectionTypeString.equals("seqMCast"))
                {
                    _connectionType = ConnectionTypes.SEQUENCED_MCAST;
                }
            else
            {
                _connectionType = -1; // error
                System.out.println(CommandLine.optionHelpString());
                System.exit(-1);
            }
            _appTypeString = CommandLine.value("appType");
            if (_appTypeString.equals("server"))
            {
                _appType = SERVER;
            }
            else if (_appTypeString.equals("client"))
            {
                _appType = CLIENT;
            }
            else
            {
                _appType = -1;
                System.out.println(CommandLine.optionHelpString());
                System.exit(-1);
            }
            _reflectMsgs = CommandLine.booleanValue("reflectMsgs");
            TransportThreadConfig.msgSize(CommandLine.intValue("msgSize"));
            _busyRead = CommandLine.booleanValue("busyRead");
            _guaranteedOutputBuffers = CommandLine.intValue("outputBufs");
            _maxFragmentSize = CommandLine.intValue("maxFragmentSize");
            _sendBufSize = CommandLine.intValue("sendBufSize");
            _recvBufSize = CommandLine.intValue("recvBufSize");         
            _highWaterMark = CommandLine.intValue("highWaterMark");
            TransportThreadConfig.latencyLogFilename(CommandLine.value("latencyFile"));
            if (TransportThreadConfig.latencyLogFilename().length() > 0)
            {
                TransportThreadConfig.logLatencyToFile(true);
            }
            _compressionTypeString = CommandLine.value("compressionType");
            if (_compressionTypeString.equals("none"))
            {
                _compressionType = CompressionTypes.NONE;
            }
            else if (_compressionTypeString.equals("zlib"))
            {
                _compressionType = CompressionTypes.ZLIB;
            }
            else if (_compressionTypeString.equals("lz4"))
            {
                _compressionType = CompressionTypes.LZ4;
            }
            else
            {
                _compressionType = -1;
                System.out.println(CommandLine.optionHelpString());
                System.exit(-1);
            }
            _compressionLevel = CommandLine.intValue("compressionLevel");
            _interfaceName = CommandLine.value("if");
            _portNo = CommandLine.value("p");
            _recvPort = CommandLine.value("rp");
            _sendPort = CommandLine.value("sp");
            _unicastPort = CommandLine.value("u");
            _hostName = CommandLine.value("h");
            _recvAddr = CommandLine.value("ra");
            if (_recvAddr.length() > 0)
            {
                _rAddr = true;
            }
            _sendAddr = CommandLine.value("sa");
            if (_sendAddr.length() > 0)
            {
                _sAddr = true;
            }
            
            _tcpNoDelay = !CommandLine.booleanValue("tcpDelay");
                
            TransportThreadConfig.msgsPerSec(CommandLine.intValue("msgRate"));
            if("all".equals(CommandLine.value("latencyMsgRate")))
                TransportThreadConfig.latencyMsgsPerSec(TransportThreadConfig.ALWAYS_SEND_LATENCY_MSG);
            else
                TransportThreadConfig.latencyMsgsPerSec(CommandLine.intValue("latencyMsgRate"));
            TransportThreadConfig.ticksPerSec(CommandLine.intValue("tickRate"));
            TransportThreadConfig.totalBuffersPerPack(CommandLine.intValue("pack"));
            if (CommandLine.booleanValue("directWrite"))
            {
                TransportThreadConfig.writeFlags(TransportThreadConfig.writeFlags() | WriteFlags.DIRECT_SOCKET_WRITE);
            }
            _takeMCastStats = CommandLine.booleanValue("mcastStats");
            
            _tcpControlPort = CommandLine.value("tp");
            _portRoamRange = CommandLine.intValue("prr");
            
            _proxy = CommandLine.booleanValue("proxy");
            if (_connectionType == ConnectionTypes.ENCRYPTED && !_proxy)
            {
                System.err.println("Config error: Requires Proxy connection when Encrypted or HTTP Connection.");
                System.out.println(CommandLine.optionHelpString());
                System.exit(-1);
            }
            _proxyHost = CommandLine.value("ph");
            _proxyPort = CommandLine.intValue("pp");
            _proxyUserName = CommandLine.value("plogin");
            _proxyPasswd = CommandLine.value("ppasswd");
            _proxyDomain = CommandLine.value("pdomain");
            _proxyKRBConfigFile = CommandLine.value("krbfile");
            _keystoreFile = CommandLine.value("keyfile");
            _keystorePasswd = CommandLine.value("keypasswd");              
        }
        catch (NumberFormatException ile)
        {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (_threadCount > 8)
        {
            System.err.println("Config error: Thread count cannot be greater than 8.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
        if (_threadCount < 0)
        {
            System.err.println("Config error: Thread count cannot be less than 0.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (TransportThreadConfig.ticksPerSec() < 1)
        {
            System.err.println("Config Error: Tick rate cannot be less than 1.\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        } 
        
        if (_sendBufSize < 0)
        {
            System.err.println("Config error: Send Buffer Size cannot be less than 0.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
        if (_recvBufSize < 0)
        {
            System.err.println("Config error: Recieve Buffer Size cannot be less than 0.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        /* Conditions */

        if (_appType == CLIENT && 
            _connectionType == ConnectionTypes.UNIDIR_SHMEM && 
            TransportThreadConfig.msgsPerSec() != 0)
        {
            System.err.println("Config Error: shared memory client can only read from shared memory. -msgRate must be zero for the client\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (_connectionType == ConnectionTypes.UNIDIR_SHMEM &&
            _reflectMsgs)
        {
            System.err.println("Config Error: Cannot use reflection with unidirectional shared memory connection.\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        } 
        
        if (_appType == CLIENT && 
                _connectionType == ConnectionTypes.UNIDIR_SHMEM )
        {
            TransportThreadConfig.checkPings(false);
        }

        if (_writeStatsInterval < 1)
        {
            System.err.println("Config error: Write Stats Interval cannot be less than 1.\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
        if (_guaranteedOutputBuffers < 0)
        {
            System.err.println("Config Error: Cannot specify less than 0 guarenteed output buffers.\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (TransportThreadConfig.totalBuffersPerPack() > 1 && _reflectMsgs)
        {
            System.err.println("Config Error: Reflection does not pack messages.\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        } 
            
        if (_appType == SERVER &&
            _connectionType == ConnectionTypes.RELIABLE_MCAST)
        {
            System.err.println("Config Error: appType for Multicast connections must be client\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (_connectionType == -1)
        {
            System.err.println("Config Error: Unknown connectionType. Valid types are \"socket\", \"http\", \"encrypted\", \"reliableMCast\", \"shmem\", \"seqMCast\"  \n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        } 

        if (_appType == SERVER &&
            _connectionType == ConnectionTypes.UNIDIR_SHMEM)
        {           
            /* Shared memory servers cannot receive pings. */
            TransportThreadConfig.checkPings(false);
        }

        if (TransportThreadConfig.latencyMsgsPerSec() > TransportThreadConfig.msgsPerSec())
        {
            System.err.println("Config Error: Latency msg rate cannot be greater than total msg rate. \n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (TransportThreadConfig.latencyMsgsPerSec() > TransportThreadConfig.ticksPerSec())
        {
            System.err.println("Config Error: Latency msg rate cannot be greater than total ticks per second. \n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (TransportThreadConfig.msgsPerSec() != 0 && TransportThreadConfig.msgsPerSec() < TransportThreadConfig.ticksPerSec())
        {
            System.err.println("Config Error: Update rate cannot be less than total ticks per second(unless it is zero).\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        /* Must have room for sending both the timestamp and sequence number. */
        if (TransportThreadConfig.msgSize() < 16)
        {
            System.err.println("Config Error: Message size must be at least 16.\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (TransportThreadConfig.totalBuffersPerPack() < 1)
        {
            System.err.println("Config Error: Cannot specify less than 1 buffer per pack.\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        /* Determine msg rates on per-tick basis */
        TransportThreadConfig.msgsPerTick(TransportThreadConfig.msgsPerSec() / TransportThreadConfig.ticksPerSec());
        TransportThreadConfig.msgsPerTickRemainder(TransportThreadConfig.msgsPerSec() % TransportThreadConfig.ticksPerSec());

        createConfigString();
    }

    // Create config string.
    private static void createConfigString()
    {
        _configString = "--- TEST INPUTS ---\n\n" +
                "                Runtime: " + _runTime + " sec\n" +
                "        Connection Type: " + ConnectionTypes.toString(_connectionType) + "\n" +
                ((_connectionType == ConnectionTypes.RELIABLE_MCAST && _sAddr) ? multicastConfig() : unicastConfig()) + "\n" + 
                                                
                "               App Type: " + (_appType == SERVER ? "server" : "client") + "\n" + 
                "           Thread Count: " + _threadCount + "\n" +
                "              Busy Read: " + (_busyRead ? "Yes" : "No") + "\n" +                
                "               Msg Size: " + TransportThreadConfig.msgSize() + "\n" +
                "               Msg Rate: " + ((_reflectMsgs) ? "0(reflecting)" : TransportThreadConfig.msgsPerSec()) + "\n" +
                "       Latency Msg Rate: " + ((_reflectMsgs) ? "0(reflecting)" : TransportThreadConfig.latencyMsgsPerSec()) + "\n" +
                "         Output Buffers: " + _guaranteedOutputBuffers + "\n" +
                "      Max Fragment Size: " + _maxFragmentSize + "\n" +
                "       Send Buffer Size: " + _sendBufSize + ((_sendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                "       Recv Buffer Size: " + _recvBufSize + ((_recvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                "        High Water Mark: " + _highWaterMark + ((_highWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
                "       Compression Type: " + CompressionTypes.toString(_compressionType) + "(" + _compressionType + ")" + "\n" +
                "      Compression Level: " + _compressionLevel + "\n" +
                "         Interface Name: " + (_interfaceName.length() > 0 ? _interfaceName : "(use default)") + "\n" +
                "         tcpControlPort: " + (_tcpControlPort.length() > 0 ? _tcpControlPort : "(use_default)") + "\n" +
                "          portRoamRange: " + _portRoamRange + "\n" +
                "            Tcp_NoDelay: " + (_tcpNoDelay ? "Yes" : "No") + "\n" +
                "              Tick Rate: " + TransportThreadConfig.ticksPerSec() + "\n" +
                "      Use Direct Writes: " + ((TransportThreadConfig.writeFlags() & WriteFlags.DIRECT_SOCKET_WRITE) > 0  ? "Yes" : "No") + "\n" +
                "       Latency Log File: " + (TransportThreadConfig.latencyLogFilename().length() > 0 ? TransportThreadConfig.latencyLogFilename() : "(none)") + "\n" +
                "           Summary File: " + _summaryFilename + "\n" +
                "             Stats File: " + TransportThreadConfig.statsFilename() + "\n" +
                "   Write Stats Interval: " + _writeStatsInterval + "\n" +
                "          Display Stats: " + (_displayStats ? "Yes" : "No") + "\n" + 
                "                Packing: " + (TransportThreadConfig.totalBuffersPerPack() <= 1 ? "No" :  "Yes(" + TransportThreadConfig.totalBuffersPerPack() + " per pack)") + "\n";

        if ( _connectionType == ConnectionTypes.ENCRYPTED || _connectionType == ConnectionTypes.HTTP)
        {
             _configString  += tunnelingConfig();
        }
    
    
    }

    private static String unicastConfig()
    {
        return "               Hostname: " + _hostName + "\n" +
               "                   Port: " + _portNo;
    }

    private static String multicastConfig()
    {
        return "        Multicast Conn: (send " + _sendAddr + ":" + _sendPort +
                        ", recv " + _recvAddr + ":" + _recvPort + ", unicast" + _unicastPort + ")";
    }
    
    private static String tunnelingConfig()
    {
        String tunneling = "";
        if (_proxy) 
        {           
            tunneling = "         Proxy Hostname: " + _proxyHost + "\n" + 
                        "             Proxy Port: " + _proxyPort + "\n" + 
                        "         Proxy UserName: " + _proxyUserName + "\n" + 
                        "         Proxy Password: " + _proxyPasswd   + "\n" + 
                        "           Proxy Domain: " + _proxyDomain + "\n" + 
                        "     ProxyKRBConfigFile: " + _proxyKRBConfigFile  + "\n";

            if ( _connectionType == ConnectionTypes.ENCRYPTED)
            {
                tunneling +=  "           KeyStoreFile: " + _keystoreFile + "\n" +
                              "       keyStorePassword: " + _keystorePasswd + "\n"; 
            }           
        }
        return tunneling;       
    }

    /**
     *  Time application runs before exiting.
     *
     * @return the int
     */
    public static int runTime()
    {
        return _runTime;
    }

    /**
     *  Number of threads that handle connections.
     *
     * @return the int
     */
    public static int threadCount()
    {
        return _threadCount;
    }
    
    /**
     *  Name of the summary log file.
     *
     * @return the string
     */
    public static String summaryFilename()
    {
        return _summaryFilename;
    }

    /**
     *  Controls how often statistics are written.
     *
     * @return the int
     */
    public static int writeStatsInterval()
    {
        return _writeStatsInterval;
    }

    /**
     *  Controls whether stats appear on the screen.
     *
     * @return true, if successful
     */
    public static boolean displayStats()
    {
        return _displayStats;
    }
    
    /**
     *  Port number.
     *
     * @return the string
     */
    public static String portNo()
    {
        return _portNo;
    }

    /**
     *  Name of interface.
     *
     * @return the string
     */
    public static String interfaceName()
    {
        return _interfaceName;
    }

    /**
     *  Guaranteed Output Buffers.
     *
     * @return the int
     */
    public static int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }
    
    /**
     *  Max Fragment Size.
     *
     * @return the int
     */
    public static int maxFragmentSize()
    {
        return _maxFragmentSize;
    }

    /**
     *  System Send Buffer Size.
     *
     * @return the int
     */
    public static int sendBufSize()
    {
        return _sendBufSize;
    }

    /**
     *  System Receive Buffer Size.
     *
     * @return the int
     */
    public static int recvBufSize()
    {
        return _recvBufSize;
    }

    /**
     *  Sets the point that causes UPA to automatically flush.
     *
     * @return the int
     */
    public static int highWaterMark()
    {
        return _highWaterMark;
    }

    /**
     *  Enable/Disable Nagle's algorithm.
     *
     * @return true, if successful
     */
    public static boolean tcpNoDelay()
    {
        return _tcpNoDelay;
    }
    
    /**
     *  Converts configuration parameters to a string.
     *
     * @return the string
     */
    public static String convertToString()
    {
        return _configString;
    }

    /**
     *  Type of application(server, client).
     *
     * @return the int
     */
    public static int appType()
    {
        return _appType;
    }

    /**
     *  If set, the application will continually read rather than using notification.
     * Messages cannot be sent in this mode.
     *
     * @return true, if successful
     */
    public static boolean busyRead()
    {
        return _busyRead;
    }

    /**
     *  Compression level, optional depending on compression algorithm used.
     *
     * @return the int
     */
    public static int compressionLevel()
    {
        return _compressionLevel;
    }

    /**
     *  Type of compression to use, if any.
     *
     * @return the int
     */
    public static int compressionType()
    {
        return _compressionType;
    }

    /**
     *  Type of connection.
     *
     * @return the int
     */
    public static int connectionType()
    {
        return _connectionType;
    }

    /**
     *  hostName, if using Transport.connect().
     *
     * @return the string
     */
    public static String hostName()
    {
        return _hostName;
    }

    /**
     *  Whether an inbound address was specified.
     *
     * @return true, if successful
     */
    public static boolean rAddr()
    {
        return _rAddr;
    }

    /**
     *  Inbound address, if using a multicast connection.
     *
     * @return the string
     */
    public static String recvAddr()
    {
        return _recvAddr;
    }

    /**
     *  Inbound port, if using a multicast connection.
     *
     * @return the string
     */
    public static String recvPort()
    {
        return _recvPort;
    }

    /**
     *  Reflect received messages instead of generating them.
     *
     * @return true, if successful
     */
    public static boolean reflectMsgs()
    {
        return _reflectMsgs;
    }

    /**
     *  Whether an outbound address was specified.
     *
     * @return true, if successful
     */
    public static boolean sAddr()
    {
        return _sAddr;
    }

    /**
     *  Outbound address, if using a multicast connection.
     *
     * @return the string
     */
    public static String sendAddr()
    {
        return _sendAddr;
    }

    /**
     *  Outbound port, if using a multicast connection.
     *
     * @return the string
     */
    public static String sendPort()
    {
        return _sendPort;
    }

    /**
     *  Running a multicast connection and we want stats.
     *
     * @return true, if successful
     */
    public static boolean takeMCastStats()
    {
        return _takeMCastStats;
    }

    /**
     *  Unicast port, if using a mulicast connection.
     *
     * @return the string
     */
    public static String unicastPort()
    {
        return _unicastPort;
    }
    
    /**
     *  tcpControlPort.
     *
     * @return the string
     */
    public static String tcpControlPort()
    {
        return _tcpControlPort;
    }
    
    /**
     *  unicastPortRange.
     *
     * @return the int
     */
    public static int portRoamRange()
    {
        return _portRoamRange;
    }
    
    /**
     * Checks for proxy.
     *
     * @return true, if successful
     */
    public static boolean hasProxy()
    {
        return _proxy;
    }   
    
    /**
     * Proxy host.
     *
     * @return the string
     */
    public static String proxyHost()
    {
        return _proxyHost;
    }
    
    /**
     * Proxy port.
     *
     * @return the int
     */
    public static int proxyPort()
    {
        return _proxyPort;
    }
        
    /**
     * Proxy user name.
     *
     * @return the string
     */
    public static String proxyUserName()
    {
        return _proxyUserName;
    }
        
    /**
     * Proxy password.
     *
     * @return the string
     */
    public static String proxyPassword()
    {
        return _proxyPasswd;
    }
        
    /**
     * Proxy domain.
     *
     * @return the string
     */
    public static String proxyDomain()
    {
        return _proxyDomain;
    }
        
    /**
     * Proxy KBR config file.
     *
     * @return the string
     */
    public static String proxyKBRConfigFile()
    {
        return _proxyKRBConfigFile;
    }
    
    /**
     * Keystore file.
     *
     * @return the string
     */
    public static String keystoreFile()
    {
        return _keystoreFile;
    }
    
    /**
     * Kestore password.
     *
     * @return the string
     */
    public static String kestorePassword()
    {
        return _keystorePasswd;
    }
        
        
    /**
     *  Converts configuration parameters to a string.
     *
     * @return the string
     */
    public static String configString()
    {
        return _configString;
    }

}
