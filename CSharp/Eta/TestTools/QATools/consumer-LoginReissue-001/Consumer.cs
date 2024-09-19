/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Threading;
using static LSEG.Eta.Rdm.Login;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Consumer
{
    public class Consumer : ResponseCallback
    {
        private const int CONSUMER_CONNECTION_RETRY_TIME = 15; // seconds

        private ChannelSession channelSession;
        private ChannelInfo channelInfo;
        private bool postInit = false;
        private LoginHandler loginHandler;
        private DirectoryHandler srcDirHandler;
        private DictionaryHandler dictionaryHandler;
        private MarketPriceHandler marketPriceHandler;
        private MarketByOrderHandler marketByOrderHandler;
        private MarketByPriceHandler marketByPriceHandler;
        private PostHandler postHandler;
        private SymbolListHandler symbolListHandler;
        private YieldCurveHandler yieldCurveHandler;

        private bool shouldOffStreamPost = false;
        private bool shouldOnStreamPost = false;
        private Buffer postItemName;
        private StreamIdWatchList itemWatchList;

        // indicates if requested service is up
        private bool requestsSent;

        private long runtime;

        // default server host name
        private const string defaultSrvrHostname = "localhost";

        // default server port number
        private const string defaultSrvrPortNo = "14002";

        // default service name
        private const string defaultServiceName = "DIRECT_FEED";

        // default item name
        private const string defaultItemName = "TRI.N";

        // consumer run-time in seconds
        private const int defaultRuntime = 600;

        private Error error;    // error information

        private DecodeIterator dIter = new DecodeIterator();
        private Msg responseMsg = new Msg();

        // private streams items are non-recoverable, it is not sent again after recovery
        private bool mppsRequestSent = false;
        private bool mbopsRequestSent = false;
        private bool mbppsRequestSent = false;
        private bool ycpsRequestSent = false;

        // APIQA: adding a variable to count updates received
        private long updatesReceived;
        private int updateCount;
        //END APIQA
        public Consumer()
        {
            channelInfo = new ChannelInfo();
            itemWatchList = new StreamIdWatchList();
            loginHandler = new LoginHandler();
            srcDirHandler = new DirectoryHandler();
            dictionaryHandler = new DictionaryHandler();
            marketPriceHandler = new MarketPriceHandler(itemWatchList);
            marketByOrderHandler = new MarketByOrderHandler(itemWatchList);
            marketByPriceHandler = new MarketByPriceHandler(itemWatchList);
            postHandler = new PostHandler();
            symbolListHandler = new SymbolListHandler();
            yieldCurveHandler = new YieldCurveHandler(itemWatchList);
            channelSession = new ChannelSession();
            postItemName = new Buffer();
            error = new Error();
        }

        /// <summary>
        /// Main loop that handles channel connection, login requests, 
        /// reading and processing responses from channel.
        /// </summary>
        public void Run()
        {
            PingHandler pingHandler = new PingHandler();
            InProgInfo inProg = new InProgInfo();

            while (true)
            {
                try
                {
                    ConnectRetry(inProg);
                }
                catch (Exception e)
                {
                    Console.WriteLine("Error during connection process: " + e.Message);
                    return;
                }

                // Handle run-time
                if (DateTimeOffset.Now.ToUnixTimeMilliseconds() >= runtime)
                {
                    Console.WriteLine("Consumer run-time expired...");
                    break;
                }

                if (channelSession.Channel.Info(channelInfo, out error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Channel.Info() failed");
                    CloseChannel();
                    return;
                }
                Console.WriteLine("Channel Info:\n" + channelInfo.ToString() + "\n");
                Console.WriteLine("  Client To Server Pings: " + channelInfo.ClientToServerPings +
                        "\n  Server To Client Pings: " + channelInfo.ServerToClientPings +
                        "\n");

                int count = channelInfo.ComponentInfoList == null ? 0 : channelInfo.ComponentInfoList.Count;
                if (count == 0)
                {
                    Console.WriteLine("No component info...");
                } 
                else
                {
                    Console.Write("Component Version: ");
                    for (int i = 0; i < count; ++i)
                    {
                        Console.Write(channelInfo.ComponentInfoList[i].ComponentVersion);
                        if (i < count - 1)
                            Console.Write(", ");
                    }
                    Console.WriteLine();
                }
                           
                loginHandler.ApplicationName = "Consumer";
                loginHandler.Role = RoleTypes.CONS;

                // Send login request message
                channelSession.IsLoginReissue = false;
                if (loginHandler.SendRequest(channelSession, out error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error sending Login request, exit.");
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                // Initialize ping handler
                pingHandler.InitPingHandler(channelSession.Channel.PingTimeOut);

                // this is the message processing loop
                ReadAndProcessResponse(pingHandler);
            }
        }

        private void ConnectRetry(InProgInfo inProg)
        {
            string hostName = CommandLine.Value("h");
            string portNumber = CommandLine.Value("p");
            string interfaceName = CommandLine.Value("i");

            while (DateTimeOffset.Now.ToUnixTimeMilliseconds() < runtime && channelSession.ShouldRecoverConnection)
            {
                Console.WriteLine("Starting connection...");

                requestsSent = false;

                // get connect options from the channel session
                ConnectOptions copts = channelSession.ConnectOptions;

                // set the connection parameters on the connect options
                copts.UnifiedNetworkInfo.Address = hostName;
                copts.UnifiedNetworkInfo.ServiceName = portNumber;
                copts.UnifiedNetworkInfo.InterfaceName = interfaceName;

                if (channelSession.Connect(out error) == TransportReturnCode.SUCCESS)
                {
                    // connection hand-shake loop
                    WaitUntilChannelActive(inProg);
                }
                
                if (channelSession.ShouldRecoverConnection)
                {
                    // sleep before trying to recover connection
                    Thread.Sleep(CONSUMER_CONNECTION_RETRY_TIME * 1000);
                    continue;
                }
            }
        }

        private void WaitUntilChannelActive(InProgInfo inProg)
        {
            while (DateTimeOffset.Now.ToUnixTimeMilliseconds() < runtime && channelSession.GetChannelState() != ChannelState.ACTIVE)
            {
                if (channelSession.InitChannel(inProg, out error) < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Error initializing channel, will retry. Error: {error?.Text}");
                }
                if (channelSession.Channel == null || channelSession.GetChannelState() == ChannelState.ACTIVE)
                    break;

                Thread.Sleep(1000);
            }
        }

        private void ReadAndProcessResponse(PingHandler pingHandler)
        {
            TransportReturnCode ret;
            while (DateTimeOffset.Now.ToUnixTimeMilliseconds() < runtime)
            {
                // read until no more to read
                ret = channelSession.Select(pingHandler, this, out error);
                if (ret != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Read failure, " + error.Text);
                    Console.WriteLine("Consumer exits...");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                //  break out of message processing loop if connection should recover
                if (channelSession.ShouldRecoverConnection)
                    break;

                //Handle pings
                if (pingHandler.HandlePings(channelSession.Channel, out error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Error handling pings: {error?.Text}");
                    CloseChannel();
                    Console.WriteLine("Consumer exits...");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                HandlePosting();

                // send login reissue if login reissue time has passed
                if (channelSession.CanSendLoginReissue && DateTimeOffset.Now.ToUnixTimeMilliseconds() >= channelSession.LoginReissueTime)
                {
                    channelSession.IsLoginReissue = true;
                    if (loginHandler.SendRequest(channelSession, out error) != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Login reissue failed. Error: " + error.Text);
                    }
                    else
                    {
                        Console.WriteLine("Login reissue sent");
                    }
                    channelSession.CanSendLoginReissue = false;
                }
            }
        }

        /// <summary>
        /// Initializes consumer application. It is responsible for: Initializing command line options used by the application. 
        /// Parsing command line arguments. Initializing all domain handlers. Loading dictionaries from file.
        /// </summary>
        /// <param name="args">application arguments</param>
        public void Init(string[] args)
        {
            AddCommandLineArgs();
            try
            {
                CommandLine.ParseArgs(args);
            }
            catch (Exception ile)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.Error.WriteLine(ile.Message);
                Console.Error.WriteLine();
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Console.WriteLine("Consumer exits...");
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            if (args.Length == 0 || (!IsMarketPriceArgSpecified() && !IsYieldCurveArgSpecified()))
            {
                CommandLine.ParseArgs(new string[] { "-mp", defaultItemName });
            }

            Console.WriteLine("Consumer initializing...");

            try
            {
                runtime = DateTimeOffset.Now.ToUnixTimeMilliseconds() + CommandLine.IntValue("runtime") * 1000;
            }
            catch (FormatException ex)
            {
                Console.WriteLine("Invalid argument, number expected.\t");
                Console.WriteLine(ex.Message);
                Console.WriteLine("Consumer exits...");
                Environment.Exit(-1);
            }
            shouldOffStreamPost = CommandLine.BoolValue("offpost");
            // this application requires at least one market price item to be
            // requested for on-stream posting to be performed
            shouldOnStreamPost = CommandLine.BoolValue("post");
            if (shouldOnStreamPost)
            {
                if (!CommandLine.HasArg("mp"))
                {
                    Console.WriteLine("\nPosting will not be performed as no Market Price items were requested");
                    shouldOnStreamPost = false;
                }
            }

            postHandler.EnableOnstreamPost = shouldOnStreamPost;

            string value = CommandLine.Value("publisherInfo");
            if (value != null)
            {
                string[] pieces = value.Split(",");

                if (pieces.Length > 1)
                {
                    string publisherId = pieces[0];
                    string publisherAddress = pieces[1];

                     postHandler.SetPublisherInfo(publisherId, publisherAddress);
                }
                else
                {
                    Console.Error.WriteLine("Error loading command line arguments for publisherInfo [id, address]:\t");
                    Console.WriteLine("Consumer exits...");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }

            loginHandler.UserName = CommandLine.Value("uname");
            loginHandler.AuthenticationToken = CommandLine.Value("at");
            loginHandler.AuthenticationExtended = CommandLine.Value("ax");
            loginHandler.ApplicationId = CommandLine.Value("aid");
            loginHandler.EnableRtt = CommandLine.BoolValue("rtt");

            // set service name in directory handler
            srcDirHandler.ServiceName.Data(CommandLine.Value("s"));
            marketPriceHandler.SnapshotRequest(CommandLine.BoolValue("snapshot"));
            marketByOrderHandler.SnapshotRequest(CommandLine.BoolValue("snapshot"));
            marketByPriceHandler.SnapshotRequest(CommandLine.BoolValue("snapshot"));
            yieldCurveHandler.SnapshotRequested = CommandLine.BoolValue("snapshot");
            symbolListHandler.SnapshotRequested = CommandLine.BoolValue("snapshot");

            marketPriceHandler.ViewRequest(CommandLine.BoolValue("view"));

            if (channelSession.InitTransport(false, out error) < TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error initialize transport.");
                Console.WriteLine("Consumer exits...");
                Environment.Exit((int)error.ErrorId);
            }

            string connectionType = CommandLine.Value("connectionType");

            if(CommandLine.BoolValue("proxy"))
            {
                string proxyHost = CommandLine.Value("ph");
                if(proxyHost is null)
                {
                    Console.Error.WriteLine("Error: Proxy hostname not provided.");
                    Console.Out.WriteLine("Consumer exits...");
                    System.Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                channelSession.ConnectOptions.ProxyOptions.ProxyHostName = proxyHost;

                string proxyPort = CommandLine.Value("pp");
                if (proxyPort is null)
                {
                    Console.Error.WriteLine("Error: Proxy prot not provided.");
                    Console.Out.WriteLine("Consumer exits...");
                    System.Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                channelSession.ConnectOptions.ProxyOptions.ProxyPort = proxyPort;

                string proxyUsername = CommandLine.Value("plogin");
                channelSession.ConnectOptions.ProxyOptions.ProxyUserName = proxyUsername;

                string proxyPassword = CommandLine.Value("ppasswd");
                channelSession.ConnectOptions.ProxyOptions.ProxyPassword = proxyPassword;
            }

            if (connectionType.Equals("encrypted"))
            {
                channelSession.ConnectOptions.ConnectionType = ConnectionType.ENCRYPTED;
                channelSession.ConnectOptions.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
            }

            CodecError codecError;
            dictionaryHandler.LoadDictionary(out codecError);

            if (CommandLine.BoolValue("x"))
            {
                channelSession.EnableXmlTrace(dictionaryHandler.DataDictionary);
            }
        }

        private bool IsMarketPriceArgSpecified()
        {
            return (CommandLine.HasArg("mp") ||
                    CommandLine.HasArg("mpps") ||
                    CommandLine.HasArg("mbp") ||
                    CommandLine.HasArg("mbpps") ||
                    CommandLine.HasArg("mbo") ||
                    CommandLine.HasArg("mbops") || CommandLine.HasArg("sl"));
        }

        private bool IsYieldCurveArgSpecified()
        {
            return (CommandLine.HasArg("yc") ||
                    CommandLine.HasArg("ycps"));
        }

        /// <summary>
        /// Call back method to process responses from channel. 
        /// Processing responses consists of performing a high level decode of the message and then 
        /// calling the applicable specific method for further processing.
        /// </summary>
        /// <param name="chnl">The channel of the response</param>
        /// <param name="buffer">The message buffer containing the response.</param>
        public void ProcessResponse(ChannelSession chnl, ITransportBuffer buffer)
        {
            CodecReturnCode ret;
            dIter.Clear();
            ret = dIter.SetBufferAndRWFVersion(buffer, chnl.Channel.MajorVersion, chnl.Channel.MinorVersion);

            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("\nDecodeIterator initialization failed with code = ", ret);
                return;
            }

            ret = responseMsg.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                        + chnl.Channel + "  Size " + (buffer.Data.Limit - buffer.Data.Position));
                CloseChannel();
                Console.WriteLine("Consumer exits...");
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            ProcessResponse(chnl, responseMsg, dIter);
        }

        private void ProcessResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
        {
            switch (responseMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    ProcessLoginResponse(chnl, responseMsg, dIter);
                    break;
                case (int)DomainType.SOURCE:
                    ProcessSourceDirectoryResponse(chnl, responseMsg);
                    break;
                case (int)DomainType.DICTIONARY:
                    ProcessDictionaryResponse(chnl, responseMsg, dIter);
                    CheckAndInitPostingSupport();
                    break;
                case (int)DomainType.MARKET_PRICE:
                    ProcessMarketPriceResponse(responseMsg, dIter);
                    break;
                case (int)DomainType.MARKET_BY_ORDER:
                    ProcessMarketByOrderResponse(responseMsg, dIter);
                    break;
                case (int)DomainType.MARKET_BY_PRICE:
                    ProcessMarketByPriceResponse(responseMsg, dIter);
                    break;
                case (int)DomainType.SYMBOL_LIST:
                    ProcessSymbolListResponse(responseMsg, dIter);
                    break;
                case (int)DomainType.YIELD_CURVE:
                    ProcessYieldCurveResponse(responseMsg, dIter);
                    break;
                default:
                    Console.WriteLine("Unhandled Domain Type: " + responseMsg.DomainType);
                    break;
            }

        }

        private void ProcessSymbolListResponse(Msg responseMsg, DecodeIterator dIter)
        {
            if (symbolListHandler.ProcessResponse(responseMsg, dIter, dictionaryHandler.DataDictionary, out error) != TransportReturnCode.SUCCESS)
            {
                CloseChannel();
                Console.Error.WriteLine("Error processing response, exit.");
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
        }

        private void ProcessMarketByPriceResponse(Msg responseMsg, DecodeIterator dIter)
        {
            if (marketByPriceHandler.ProcessResponse(responseMsg, dIter, dictionaryHandler.DataDictionary, out error) != TransportReturnCode.SUCCESS)
            {
                CloseChannel();
                Console.Error.WriteLine("Error processing marketByPrice response, exit.");
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
        }

        private void ProcessMarketByOrderResponse(Msg responseMsg, DecodeIterator dIter)
        {
            if (marketByOrderHandler.ProcessResponse(responseMsg, dIter, dictionaryHandler.DataDictionary, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error processing marketByOrder response, exit.");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
        }

        private void ProcessMarketPriceResponse(Msg responseMsg, DecodeIterator dIter)
        {
            if (marketPriceHandler.ProcessResponse(responseMsg, dIter, dictionaryHandler.DataDictionary, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error processing marketPrice response, exit.");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            // APIQA: Incrementing update counter
            if (responseMsg.MsgClass == MsgClasses.UPDATE)
            {
                updatesReceived++;
                // APIQA: Send a login reissue with PAUSE ALL
                if (updatesReceived == 2)
                {
                    if (loginHandler.SendPauseResumeRequest(channelSession, out error, true, false) != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("------------APIQA: attempted login reissue failed. Error: " + error.Text);
                    }
                    else
                    {
                        Console.WriteLine("------------APIQA: PAUSE-ALL login reissue done.");
                    }
                }
            }
            // END APIQA: Send a login reissue with PAUSE ALL
        }

        private void ProcessDictionaryResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
        {
            if (dictionaryHandler.ProcessResponse(chnl.Channel, responseMsg, dIter, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error processing dictionary response, exit.");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (dictionaryHandler.FieldDictionaryLoaded &&
                    dictionaryHandler.EnumTypeDictionaryLoaded && responseMsg.MsgClass == MsgClasses.REFRESH)
            {
                Console.WriteLine("Dictionary ready, requesting item(s)...");

                itemWatchList.Clear();
                SendMPRequests(chnl);
                SendMBORequests(chnl);
                SendMBPRequests(chnl);
                SendSymbolListRequests(chnl);
                SendYieldCurveRequests(chnl);
                postHandler.EnableOffstreamPost = shouldOffStreamPost;
                postHandler.EnableOnstreamPost = shouldOnStreamPost;
            }
        }

        private void ProcessSourceDirectoryResponse(ChannelSession chnl, Msg responseMsg)
        {
            TransportReturnCode ret = srcDirHandler.ProcessResponse(chnl.Channel, responseMsg, dIter, out error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error processing source directory response, exit.");
                Console.Error.WriteLine(error.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (srcDirHandler.IsRequestedServiceUp())
            {
                SendRequests(chnl);
            }
            else
            {
                // service not up or
                // previously up service went down
                requestsSent = false;

                Console.WriteLine("Requested service '" + CommandLine.Value("s") + "' not up. Waiting for service to be up...");
            }

            // APIQA: Send a login reissue with RESUME ALL
            if (responseMsg.MsgClass == MsgClasses.UPDATE)
            {
                updateCount++;
                if (updateCount == 1)
                {
                    // Do a RESUME
                    if (loginHandler.SendPauseResumeRequest(channelSession, out error, false, true) != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("------------APIQA: attempted login reissue failed. Error: " + error.Text);
                    }
                    else
                    {
                        Console.WriteLine("------------APIQA: RESUME-ALL login reissue done.");
                    }
                }
            }
            // END APIQA
        }

        private void ProcessYieldCurveResponse(Msg responseMsg, DecodeIterator dIter)
        {
            if (yieldCurveHandler.ProcessResponse(responseMsg, dIter, dictionaryHandler.DataDictionary, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error processing yieldCurve response, exit.");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
        }

        private void SendRequests(ChannelSession chnl)
        {
            if (requestsSent)
                return;

            // first load dictionaries. send item requests and post
            // messages only after dictionaries are loaded.
            if (!DictionariesLoaded())
            {
                if (srcDirHandler.ServiceInfo().Info.DictionariesProvidedList.Count > 0)
                {
                    SendDictionaryRequests(chnl);
                }
                else
                {
                    Console.WriteLine("\nDictionary download not supported by the indicated provider");
                }
                return;
            }

            itemWatchList.Clear();
            SendMPRequests(chnl);
            SendMBORequests(chnl);
            SendMBPRequests(chnl);
            SendSymbolListRequests(chnl);
            SendYieldCurveRequests(chnl);

            CheckAndInitPostingSupport();

            postHandler.EnableOffstreamPost = shouldOffStreamPost;
            postHandler.EnableOnstreamPost = shouldOnStreamPost;

            requestsSent = true;
        }

        private void SendDictionaryRequests(ChannelSession chnl)
        {
            dictionaryHandler.ServiceId = srcDirHandler.ServiceInfo().ServiceId;
            
            if (dictionaryHandler.SendRequests(chnl, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error sending dictionary request, exit.");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
        }

        private void SendSymbolListRequests(ChannelSession chnl)
        {
            if (!CommandLine.HasArg("sl"))
                return;

            if (!srcDirHandler.ServiceInfo().HasInfo)
            {
                Console.Error.WriteLine("Error sending symbolList request, exit.");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            ServiceInfo info = srcDirHandler.ServiceInfo().Info;

            if (info.QosList.Count > 0)
            {
                Qos qos = info.QosList[0];
                symbolListHandler.Qos.IsDynamic = qos.IsDynamic;
                symbolListHandler.Qos.Rate(qos.Rate());
                symbolListHandler.Qos.Timeliness(qos.Timeliness());
            }
            else
            {
                symbolListHandler.Qos.IsDynamic = false;
                symbolListHandler.Qos.Rate(QosRates.TICK_BY_TICK);
                symbolListHandler.Qos.Timeliness(QosTimeliness.REALTIME);
            }
            symbolListHandler.Capabilities.AddRange(info.CapabilitiesList);
            symbolListHandler.ServiceId = srcDirHandler.ServiceInfo().ServiceId;
            string cmdSLName = CommandLine.Value("sl");
            if (cmdSLName == null)
            {
                symbolListHandler.SymbolListName.Data(info.ItemList.Data(), info.ItemList.Position, info.ItemList.Length);
            }
            else
            {
                symbolListHandler.SymbolListName.Data(cmdSLName);
            }
            if (symbolListHandler.SendRequest(chnl, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine(error.Text);
                Console.Error.WriteLine("Error sending symbolList request, exit...");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
        }

        private void SendMBPRequests(ChannelSession chnl)
        {
            if (marketByPriceHandler.SendItemRequests(chnl, CommandLine.Values("mbp"), false, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error sending MBP request, exit.");
                Console.WriteLine(error.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (CommandLine.HasArg("mbpps") && !mbppsRequestSent)
            {
                if (marketByPriceHandler.SendItemRequests(chnl, CommandLine.Values("mbpps"), true, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine(error.Text);
                    CloseChannel();
                    Console.WriteLine("Consumer exits...");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                mbppsRequestSent = true;
            }
        }

        private void SendMBORequests(ChannelSession chnl)
        {
            if (marketByOrderHandler.SendItemRequests(chnl, CommandLine.Values("mbo"), false, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error sending MBO request, exit.");
                Console.WriteLine(error.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (CommandLine.HasArg("mbops") && !mbopsRequestSent)
            {
                if (marketByOrderHandler.SendItemRequests(chnl, CommandLine.Values("mbops"), true, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error sending MBO request, exit.");
                    Console.WriteLine(error.Text);
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                mbopsRequestSent = true;
            }
        }

        private void SendMPRequests(ChannelSession chnl)
        {
            if (marketPriceHandler.SendItemRequests(chnl, CommandLine.Values("mp"), false, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error sending MP request, exit.");
                Console.WriteLine(error.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (CommandLine.HasArg("mpps") && !mppsRequestSent)
            {
                if (marketPriceHandler.SendItemRequests(chnl, CommandLine.Values("mpps"), true, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error sending MP request, exit...");
                    Console.WriteLine(error.Text);
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                mppsRequestSent = true;
            }
        }

        private void SendYieldCurveRequests(ChannelSession chnl)
        {
            if (yieldCurveHandler.SendItemRequests(chnl, CommandLine.Values("yc"), false, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error sending YC request, exit.");
                Console.WriteLine(error.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (CommandLine.HasArg("ycps") && !ycpsRequestSent)
            {
                if (yieldCurveHandler.SendItemRequests(chnl, CommandLine.Values("ycps"), true, loginHandler.LoginRefresh, srcDirHandler.ServiceInfo(), out error) != TransportReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error sending YC request, exit...");
                    Console.WriteLine(error.Text);
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                ycpsRequestSent = true;
            }
        }

        private bool DictionariesLoaded()
        {
            return dictionaryHandler.FieldDictionaryLoaded && dictionaryHandler.EnumTypeDictionaryLoaded;
        }

        private void CheckAndInitPostingSupport()
        {
            if (!(shouldOnStreamPost || shouldOffStreamPost) || postInit)
                return;

            // set up posting if its enabled 
            // ensure that provider supports posting - if not, disable posting
            if (!loginHandler.LoginRefresh.HasFeatures || !loginHandler.LoginRefresh.SupportedFeatures.HasSupportPost || loginHandler.LoginRefresh.SupportedFeatures.SupportOMMPost == 0)
            {
                // provider does not support posting, disable it
                shouldOffStreamPost = false;
                shouldOnStreamPost = false;
                postHandler.EnableOnstreamPost = false;
                postHandler.EnableOffstreamPost = false;
                Console.WriteLine("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
                return;
            }

            // This sets up our basic timing so post messages will be sent periodically
            postHandler.InitPostHandler();

            // posting has been initialized
            postInit = true;
        }

        private void ProcessLoginResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
        {
            TransportReturnCode ret = loginHandler.ProcessResponse(responseMsg, dIter, out error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine("Error processing Login response, exit.");
                Console.WriteLine(error.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            if (responseMsg.MsgClass == MsgClasses.GENERIC && responseMsg.ContainerType == DataTypes.ELEMENT_LIST)
            {
                loginHandler.SendRttMessage(chnl, out error);
                return;
            }

            // Handle login states
            ConsumerLoginState loginState = loginHandler.LoginState;
            if (loginState == ConsumerLoginState.OK_SOLICITED)
            {
                if (!chnl.IsLoginReissue)
                {
                    // APIQA: Don't send directory request during PAUSE/RESUME
                    if (updateCount > 0)
                    {
                        return;
                    }
                    // END APIQA
                    ret = srcDirHandler.SendRequest(chnl, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Error sending directory request: " + error.Text);
                        CloseChannel();
                        Console.WriteLine("Consumer exits...");
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }
            else if (loginState == ConsumerLoginState.CLOSED)
            {
                Console.WriteLine(error.Text);
                CloseChannel();
                Console.WriteLine("Consumer exits...");
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            else if (loginState == ConsumerLoginState.CLOSED_RECOVERABLE)
            {
                ret = channelSession.RecoverConnection(out error);
                if (ret != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error recovering connection: " + error.Text);
                    Console.WriteLine("Consumer exits...");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }
            else if (loginState == ConsumerLoginState.SUSPECT)
            {
                if (!loginHandler.LoginRefresh.HasAttrib || // default behavior when singleopen attrib not set
                        loginHandler.LoginRefresh.LoginAttrib.SingleOpen == 0)
                {
                    // login suspect from no single-open provider: 1) close source
                    // directory stream and item streams. 2) reopen streams
                    CloseDictAndItemStreams();

                    // reopen directory stream, which in turn reopens other streams
                    // (dict, item streams)
                    ret = srcDirHandler.CloseStream(channelSession, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Error closing directory stream: " + error.Text);
                        CloseChannel();
                        Console.WriteLine("Consumer exits...");
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }

                    if (!chnl.IsLoginReissue)
                    {
                        ret = srcDirHandler.SendRequest(chnl, out error);
                        if (ret != TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Error sending directory request: " + error.Text);
                            CloseChannel();
                            Console.WriteLine("Consumer exits...");
                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                    }
                }
                // login suspect from single-open provider: provider handles
                // recovery. consumer does nothing in this case.
            }

            // get login reissue time from authenticationTTReissue
            if (responseMsg.MsgClass == MsgClasses.REFRESH && loginHandler.LoginRefresh.HasAuthenicationTTReissue)
            {
                chnl.LoginReissueTime = loginHandler.LoginRefresh.AuthenticationTTReissue * 1000;
                chnl.CanSendLoginReissue = true;
            }
        }

        // on and off stream posting if enabled
        private void HandlePosting()
        {
            if (postHandler.EnableOnstreamPost)
            {
                postItemName.Clear();
                int postStreamId = marketPriceHandler.GetFirstItem(postItemName);
                if (postStreamId == 0 || postItemName.Length == 0)
                {
                    return;
                }
                postHandler.StreamId = postStreamId;
                postHandler.ServiceId = srcDirHandler.ServiceInfo().ServiceId;
                postHandler.Dictionary = dictionaryHandler.DataDictionary;
                postHandler.PostItemName.Data(postItemName.Data(), postItemName.Position, postItemName.Length);

                TransportReturnCode ret = postHandler.HandlePosts(channelSession, out error);
                if (ret < TransportReturnCode.SUCCESS)
                    Console.WriteLine("Error posting onstream: " + error.Text);
            }
            if (postHandler.EnableOffstreamPost)
            {
                postHandler.StreamId = loginHandler.LoginRefresh.StreamId;
                postHandler.PostItemName.Data("OFFPOST");
                postHandler.ServiceId = srcDirHandler.ServiceInfo().ServiceId;
                postHandler.Dictionary = dictionaryHandler.DataDictionary;
                TransportReturnCode ret = postHandler.HandlePosts(channelSession, out error);
                if (ret < TransportReturnCode.SUCCESS)
                    Console.WriteLine("Error posting offstream: " + error.Text);
            }
        }

        private void CloseDictAndItemStreams()
        {
            // have offstream posting post close status
            postHandler.StreamId = loginHandler.LoginRefresh.StreamId;
            postHandler.PostItemName.Data("OFFPOST");
            postHandler.ServiceId = srcDirHandler.ServiceInfo().ServiceId;
            postHandler.Dictionary = dictionaryHandler.DataDictionary;
            postHandler.CloseOffStreamPost(channelSession, error);

            // close item streams if opened
            marketPriceHandler.CloseStreams(channelSession, out error);
            marketByOrderHandler.CloseStreams(channelSession, out error);
            marketByPriceHandler.CloseStreams(channelSession, out error);
            symbolListHandler.CloseStream(channelSession, out error);
            yieldCurveHandler.CloseStreams(channelSession, out error);

            // close dictionary streams if opened
            dictionaryHandler.CloseStreams(channelSession, out error);
        }

        /// <summary>
        /// Closes all streams for the consumer.
        /// </summary>
        public void Uninitialize()
        {
            Console.WriteLine("Consumer unitializing and exiting...");
            if (channelSession.Channel == null)
            {
                channelSession.UnInit(out error);
                Console.WriteLine("Consumer exits...");
                Environment.Exit((int)TransportReturnCode.SUCCESS);
            }

            CloseDictAndItemStreams();
            srcDirHandler.CloseStream(channelSession, out error);
            loginHandler.CloseStream(channelSession, out error);
            FlushChannel();
            CloseChannel();
        }

        private void FlushChannel()
        {
            TransportReturnCode retval;
            do
            {
                retval = channelSession.Flush(out error);
            } while (retval > TransportReturnCode.SUCCESS);

            if (retval < TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Flush() failed with return code " + retval + "- <" + error.Text + ">");
            }
        }

        private void CloseChannel()
        {
            channelSession.UnInit(out error);
        }

        private static void AddCommandLineArgs()
        {
            CommandLine.ProgName("Consumer");
            CommandLine.AddOption("mp", "For each occurrence, requests item using Market Price domain.");
            CommandLine.AddOption("mpps", "For each occurrence, requests item using Market Price domain on private stream. Default is no market price private stream requests.");
            CommandLine.AddOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
            CommandLine.AddOption("mbops", "For each occurrence, requests item using Market By Order domain on private stream. Default is no market by order private stream requests.");
            CommandLine.AddOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");
            CommandLine.AddOption("mbpps", "For each occurrence, requests item using Market By Price domain on private stream. Default is no market by price private stream requests.");
            CommandLine.AddOption("yc", "For each occurrence, requests item using Yield Curve domain. Default is no yield curve requests.");
            CommandLine.AddOption("ycps", "For each occurrence, requests item using Yield Curve domain on private stream. Default is no yield curve private stream requests.");
            CommandLine.AddOption("view", "Specifies each request using a basic dynamic view. Default is false.");
            CommandLine.AddOption("post", "Specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream). Default is false.");
            CommandLine.AddOption("offpost", "Specifies that the application should attempt to send post messages on the login stream (i.e., off-stream).");
            CommandLine.AddOption("publisherInfo", "Specifies that the application should add user provided publisher Id and publisher ipaddress when posting");
            CommandLine.AddOption("snapshot", "Specifies each request using non-streaming. Default is false(i.e. streaming requests.)");
            CommandLine.AddOption("sl", "Requests symbol list using Symbol List domain. (symbol list name optional). Default is no symbol list requests.");
            CommandLine.AddOption("h", defaultSrvrHostname, "Server host name");
            CommandLine.AddOption("p", defaultSrvrPortNo, "Server port number");
            CommandLine.AddOption("i", (string)null, "Interface name");
            CommandLine.AddOption("s", defaultServiceName, "Service name");
            CommandLine.AddOption("uname", "Login user name. Default is system user name.");
            CommandLine.AddOption("connectionType", "Socket", "Specifies the connection type that the connection should use. Possible values are: 'Socket', 'encrypted'");

            CommandLine.AddOption("runtime", defaultRuntime, "Program runtime in seconds");
            CommandLine.AddOption("x", "Provides XML tracing of messages.");

            CommandLine.AddOption("proxy", "Specifies that the application will make a connection through a proxy server, default is false");
            CommandLine.AddOption("ph", "", "Proxy server host name");
            CommandLine.AddOption("pp", "", "Proxy port number");
            CommandLine.AddOption("plogin", "", "User Name on proxy server");
            CommandLine.AddOption("ppasswd", "", "Password on proxy server");
            CommandLine.AddOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
            CommandLine.AddOption("ax", "", "Specifies the Authentication Extended information.");
            CommandLine.AddOption("aid", "", "Specifies the Application ID.");
            CommandLine.AddOption("rtt", false, "Enables RTT feature.");
        }

        /// <summary>
        /// The main method.
        /// </summary>
        /// <param name="args">Application arguments</param>
        public static void Main(string[] args)
        {
            Consumer consumer = new Consumer();
            consumer.Init(args);
            consumer.Run();
            consumer.Uninitialize();
            Console.WriteLine("Consumer done...");
        }
    }
}
