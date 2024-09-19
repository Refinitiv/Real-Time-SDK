/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.NiProvider
{
    /// <summary>
    /// This is a main class to run ETA NIProvider application. 
    /// The purpose of this application is to non-interactively provide Level I Market Price and 
    /// Level 2 Market By Order data to an Advanced Data Hub (ADH). If supported it requests dictionary from an adh. 
    /// </summary>
    public class NIProvider : ResponseCallback
    {
        private const int NIPROVIDER_CONNECTION_RETRY_TIME = 15; // seconds
        private const int SEND_INTERVAL = 1000; // send content every 1000 milliseconds
        private Error? error = null;
        private ChannelSession channelSession;
        private bool isUnifiedNetworkConnection = true;
        private ChannelInfo channelInfo;
        private LoginHandler loginHandler;
        private NiProviderDirectoryHandler srcDirHandler;
        private NiProviderDictionaryHandler dictionaryHandler;
        private NiProviderMarketPriceHandler marketPriceHandler;
        private NiProviderMarketByOrderHandler marketByOrderHandler;
        private long runtime;
        private NiStreamIdWatchList itemWatchList;
        private DecodeIterator dIter = new DecodeIterator();
        private EncodeIterator encIter = new EncodeIterator();
        private bool refreshesSent;

        private Msg incomingMsg = new Msg();
        private Msg outgoingMsg = new Msg();

        //default server host
        private const string defaultSrvrHostname = "localhost";
        //default server port number
        private const string defaultSrvrPortNo = "14003";
        //default service name
        private const string defaultServiceName = "NI_PUB";
        //default serviceId 
        private const int defaultServiceId = 1;
        //default item name
        private const string defaultItemName = "TRI.N,IBM.N";

        //NIProvider run-time in seconds
        private const int defaultRuntime = 600;

        public static int TRANSPORT_BUFFER_SIZE_STATUS_MSG = ChannelSession.MAX_MSG_SIZE;

        /// <summary>
        /// Instantiates a new NI provider.
        /// </summary>
        public NIProvider()
        {
            channelInfo = new ChannelInfo();
            itemWatchList = new NiStreamIdWatchList();
            loginHandler = new LoginHandler();
            srcDirHandler = new NiProviderDirectoryHandler();
            dictionaryHandler = new NiProviderDictionaryHandler();
            marketPriceHandler = new NiProviderMarketPriceHandler(itemWatchList, dictionaryHandler.DataDictionary);
            marketByOrderHandler = new NiProviderMarketByOrderHandler(itemWatchList, dictionaryHandler.DataDictionary);
            channelSession = new ChannelSession();
            channelSession.SelectTime(1000); // 1000ms
            isUnifiedNetworkConnection = true;
        }

        /// <summary>
        /// Main loop that handles channel connection, login requests, 
        /// reading and processing responses from channel, and providing content.
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
                    Console.WriteLine("Exception while reconnecting: " + e.Message);
                    return;
                }

                //Handle run-time
                if (DateTimeOffset.Now.ToUnixTimeMilliseconds() >= runtime)
                {
                    Console.WriteLine("NIProvider run-time expired...");
                    break;
                }

                if (channelSession.Channel!.Info(channelInfo, out error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Channel.Info() failed");
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
                Console.WriteLine($"Channel Info: {channelInfo.ToString()}\n");
                Console.WriteLine();
                int count = channelInfo.ComponentInfoList.Count;
                if (count == 0)
                    Console.WriteLine("(No component info)");
                else
                {
                    for (int i = 0; i < count; ++i)
                    {
                        Console.WriteLine(channelInfo.ComponentInfoList[i].ComponentVersion);
                        if (i < count - 1)
                        {
                            Console.WriteLine(", ");
                        }  
                    }
                }

                loginHandler.ApplicationName = "NIProvider";
                loginHandler.Role = Login.RoleTypes.PROV;

                // Send login request message
                channelSession.IsLoginReissue = false;
                if (loginHandler.SendRequest(channelSession, out error) != TransportReturnCode.SUCCESS)
                {
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                // Initialize ping handler
                pingHandler.InitPingHandler(channelSession.Channel.PingTimeOut);

                // this is the content handler processing loop
                ContentHandler(pingHandler);
            }
        }

        private void ConnectRetry(InProgInfo inProg)
        {
            while (DateTimeOffset.Now.ToUnixTimeMilliseconds() < runtime && channelSession.ShouldRecoverConnection)
            {
                Console.WriteLine("Starting connection...");

                // get connect options from the channel
                ConnectOptions copts = channelSession.ConnectOptions;

                // set the connection parameters on the connect options 
                if (isUnifiedNetworkConnection) // unified connection is used for TCP socket
                {
                    copts.UnifiedNetworkInfo.Address = CommandLine.Value("h");
                    copts.UnifiedNetworkInfo.ServiceName = CommandLine.Value("p");            	
                }

                channelSession.Connect(out error!);

                // connection hand-shake loop
                WaitUntilChannelActive(inProg);
                if (channelSession.ShouldRecoverConnection)
                {
                    // sleep before trying to recover connection */
                    Thread.Sleep(NIPROVIDER_CONNECTION_RETRY_TIME * 1000);
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
                    Console.WriteLine($"Failed initializing channel, error: {error?.Text}");
                }

                if (channelSession.Channel == null || channelSession.GetChannelState() == ChannelState.ACTIVE)
                break;

                Thread.Sleep(1000);
            }
        }

        private void ContentHandler(PingHandler pingHandler)
        {
            TransportReturnCode ret;
            long currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            long nextSendTime = currentTime + SEND_INTERVAL;
            while (currentTime < runtime)
            {
                //read
                ret = channelSession.Select(pingHandler, this, out error);
                if (ret != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine(error?.Text);
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                // break out of message processing loop if connection should recover
                if (channelSession.ShouldRecoverConnection)
                {
                    refreshesSent = false;
                    break;
                }

                //provide content (item updates)
                if (srcDirHandler.IsServiceUp())
                {
                    if (currentTime >= nextSendTime)
                    {
                        if (refreshesSent) // send updates since refreshes already sent
                        {
                            if (ProvideContent(pingHandler, out error) != TransportReturnCode.SUCCESS)
                            {
                                CloseChannel();
                                Console.WriteLine($"Failed sending updates, error: {error?.Text}");
                                Environment.Exit((int)TransportReturnCode.FAILURE);
                            }
                        }
                        else // send refreshes first

                        {
                            SendItemRefreshes(channelSession, out error);
                            refreshesSent = true;
                        }
                        nextSendTime += SEND_INTERVAL;
                    }
                }

                //handle pings
                if (pingHandler.HandlePings(channelSession.Channel!, out error) != TransportReturnCode.SUCCESS)
                {
                    CloseChannel();
                    Console.WriteLine($"Failed handling pings, error: {error?.Text}");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }

                // send login reissue if login reissue time has passed
                if (channelSession.CanSendLoginReissue && DateTimeOffset.Now.ToUnixTimeMilliseconds() >= channelSession.LoginReissueTime)
                {
                    channelSession.IsLoginReissue = true;
                    if (loginHandler.SendRequest(channelSession, out error) != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Login reissue failed. Error: {error?.Text}");
                    }
                    else
                    {
                        Console.WriteLine("Login reissue sent");
                    }
                    channelSession.CanSendLoginReissue = false;
                }

                currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            }
        }

        private TransportReturnCode ProvideContent(PingHandler pingHandler, out Error? error)
        {
            TransportReturnCode ret;
            ret = marketPriceHandler.SendItemUpdates(channelSession, out error);
            if (ret != TransportReturnCode.SUCCESS)
                return ret;

            ret = marketByOrderHandler.SendItemUpdates(channelSession, out error);
            if (ret != TransportReturnCode.SUCCESS)
                return ret;

            pingHandler.SentLocalMsg = true;
            return ret;
        }

        /// <summary>
        /// Initializes NIProvider application. 
        /// It is responsible for: 1) Initializing command line options used by the application. 
        /// 2) Parsing command line arguments. 3) Initializing all domain handlers. 4) Loading dictionaries 
        /// from file. 5) Enabling XML tracing, if specified.
        /// </summary>
        /// <param name="args">Application arguments.</param>
        public void Init(string[] args)
        {
            AddCommandLineArgs();
            try
            {
                CommandLine.ParseArgs(args);
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.Error.WriteLine(e.Message);
                Console.Error.WriteLine();
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            if (args.Length == 0 || !IsMarketPriceArgSpecified())
            {
                CommandLine.ParseArgs(new string[] { "-mp", defaultItemName });
            }

            //display command line information
            if (isUnifiedNetworkConnection) // unified connection
            {
                Console.WriteLine("NIProvider initializing using TCP Socket connection type...\ninfraHostname=" + CommandLine.Value("h")
                                   + " infraPortNo=" + CommandLine.Value("p") + " serviceName="
                                   + CommandLine.Value("s") + " serviceId=" + CommandLine.Value("id"));
            }

            try
            {
                runtime = DateTimeOffset.Now.ToUnixTimeMilliseconds() + CommandLine.IntValue("runtime") * 1000;
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Invalid argument, number expected.\t");
                Console.Error.WriteLine(e.Message);
                Environment.Exit(-1);
            }
            loginHandler.UserName = CommandLine.Value("uname");
            loginHandler.AuthenticationToken = CommandLine.Value("at");
            loginHandler.AuthenticationExtended = CommandLine.Value("ax");
            loginHandler.ApplicationId = CommandLine.Value("aid");

            /* set service name in directory handler */
            srcDirHandler.ServiceName.Data(CommandLine.Value("s"));

            /* set service Id in directory handler */
            if (CommandLine.HasArg("id"))
            {
                try
                {
                    srcDirHandler.ServiceId = CommandLine.IntValue("id");
                }
                catch (FormatException e)
                {
                    Console.Error.WriteLine("Invalid argument, number expected.\t");
                    Console.Error.WriteLine(e.Message);
                    Environment.Exit(-1);
                }
            }
            else
            {
                srcDirHandler.ServiceId = defaultServiceId;
            }

            if (channelSession.InitTransport(false, out error) < TransportReturnCode.SUCCESS)
            {
                Environment.Exit((int)error.ErrorId);
            }
                
            if (CommandLine.BoolValue("x"))
            {
                channelSession.EnableXmlTrace(dictionaryHandler.DataDictionary);
            }
        }

        private bool IsMarketPriceArgSpecified()
        {
            return CommandLine.HasArg("mp") || CommandLine.HasArg("mbo");
        }

        /// <summary>
        /// Processes a response from a channel. 
        /// This consists of performing a high level decoding of the message 
        /// and then calling the applicable specific method for further processing.
        /// </summary>
        /// <param name="chnl">The channel of the response.</param>
        /// <param name="buffer">The message buffer containing the response.</param>
        public void ProcessResponse(ChannelSession chnl, ITransportBuffer buffer)
        {
            dIter.Clear();
            dIter.SetBufferAndRWFVersion(buffer, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = incomingMsg.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"\nDecodeMsg(): Error {ret.GetAsString()} on SessionData Channel={chnl.Channel}, Size {(buffer.Data.Limit - buffer.Data.Position)}");
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            ProcessReceivedMessage(chnl, incomingMsg, dIter);
        }

        private TransportReturnCode SendNotSupportedStatus(ChannelSession chnl, Msg receivedMsg)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_STATUS_MSG, false, out error);
            if (msgBuf == null)
            {
                Console.WriteLine($"Channel.GetBuffer() failed: {error?.Text}");
                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode ret = EncodeNotSupportedStatus(chnl, receivedMsg, msgBuf);
            if (ret != TransportReturnCode.SUCCESS)
            {
                chnl.Channel!.ReleaseBuffer(msgBuf, out error);
                return ret;
            }

            Console.WriteLine($"\nRejecting Item Request with streamId={receivedMsg.StreamId}. Reason: Domain {DomainTypes.ToString(receivedMsg.DomainType)} Not Supported");

            //send not supported status
            ret = chnl.Write(msgBuf, out error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine($"Channel.Write() failed: {error?.Text}");
                return TransportReturnCode.FAILURE;
            }

            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode EncodeNotSupportedStatus(ChannelSession chnl, Msg receivedMsg, ITransportBuffer msgBuf)
        {
            encIter.Clear();
            outgoingMsg.Clear();
            outgoingMsg.MsgClass = MsgClasses.STATUS;
            outgoingMsg.StreamId = receivedMsg.StreamId;
            outgoingMsg.DomainType = receivedMsg.DomainType;
            outgoingMsg.ContainerType = DataTypes.NO_DATA;
            outgoingMsg.Flags = StatusMsgFlags.HAS_STATE;

            IStatusMsg statusMsg = (IStatusMsg)outgoingMsg;
            statusMsg.State.StreamState(StreamStates.CLOSED);
            statusMsg.State.DataState(DataStates.SUSPECT);
            statusMsg.State.Code(StateCodes.USAGE_ERROR);
            statusMsg.State.Text().Data("Request rejected for stream id " 
                + receivedMsg.StreamId + " - domain type '" 
                + DomainTypes.ToString(receivedMsg.DomainType) + "' is not supported");

            encIter.Clear();
            CodecReturnCode ret = encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIterator.SetBufferAndRWFVersion() failed: <{ret.GetAsString()}>");
            }

            ret = statusMsg.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"StatusMsg.Encode() failed: <{ret.GetAsString()}>");
            }

            return TransportReturnCode.SUCCESS;
        }

        private void ProcessReceivedMessage(ChannelSession channelSession, Msg receivedMsg, DecodeIterator dIter)
        {
            switch (receivedMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    {
                        ProcessLoginResponse(channelSession, receivedMsg, dIter);
                        break;
                    }
                case (int)DomainType.DICTIONARY:
                    {
                        dictionaryHandler.ProcessResponse(receivedMsg, dIter, out error);
                        break;
                    }
                default:
                    {
                        if (SendNotSupportedStatus(channelSession, receivedMsg) != TransportReturnCode.SUCCESS)
                        {
                            CloseChannel();
                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                        break;
                    }
            }
        }

        private void ProcessLoginResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
        {
            TransportReturnCode ret = loginHandler.ProcessResponse(responseMsg, dIter, out error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine(error?.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            //Handle login states
            ConsumerLoginState loginState = loginHandler.LoginState;
            if (loginState == ConsumerLoginState.OK_SOLICITED)
            {
                if (!chnl.IsLoginReissue)
                {
                    ret = srcDirHandler.SendRefresh(chnl, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Error sending directory request: {error?.Text}");
                        CloseChannel();
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }

                    SetupDictionary(chnl);
                }
            }
            else if (loginState == ConsumerLoginState.CLOSED)
            {
                Console.WriteLine(error?.Text);
                CloseChannel();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            else if (loginState == ConsumerLoginState.CLOSED_RECOVERABLE)
            {
                ret = channelSession.RecoverConnection(out error);
                if (ret != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Error recovering connection: {error?.Text}");
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }
            else if (loginState == ConsumerLoginState.SUSPECT)
            {
                if (!loginHandler.LoginRefresh.HasAttrib || loginHandler.LoginRefresh.LoginAttrib.SingleOpen == 0)
                {
                     // login suspect from no single-open provider: 1) close source
                     // directory stream and item streams. 2) reopen streams
                    CloseItemStreams();

                    // reopen directory stream, which in turn reopens other streams
                    // (item streams)
                    ret = srcDirHandler.CloseStream(channelSession, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Error closing directory stream: {error?.Text}");
                        CloseChannel();
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }

                    if (!chnl.IsLoginReissue)
                    {
                        ret = srcDirHandler.SendRefresh(chnl, out error);
                        if (ret != TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"Error sending directory request: {error?.Text}");
                            CloseChannel();
                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }

                        SendItemRefreshes(chnl, out error);
                    }
                }
            }

            // get login reissue time from authenticationTTReissue
            if (responseMsg.MsgClass == MsgClasses.REFRESH && loginHandler.LoginRefresh.HasAuthenicationTTReissue)
            {
                chnl.LoginReissueTime = loginHandler.LoginRefresh.AuthenticationTTReissue * 1000;
                chnl.CanSendLoginReissue = true;
            }
        }

        private void SetupDictionary(ChannelSession chnl)
        {
            CodecError codecError;
            if (!dictionaryHandler.LoadDictionary(out codecError))
            {
                // if no local dictionary found maybe we can request it from ADH 
                Console.WriteLine("Local dictionary not available, will try to request it from ADH if it supports the Provider Dictionary Download\n");

                if (loginHandler.LoginRefresh.HasFeatures && loginHandler.LoginRefresh.SupportedFeatures.HasSupportProviderDictionaryDownload
                        && loginHandler.LoginRefresh.SupportedFeatures.SupportProviderDictionaryDownload == 1)
                {
                    TransportReturnCode sendStatus = dictionaryHandler.SendDictionaryRequests(chnl, out error, defaultServiceId);

                    if (sendStatus == TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Sent Dictionary Request\n");
                    }
                    else
                    {
                        Console.WriteLine($"Dictionary could not be downloaded, unable to send request to the connection: {error?.Text}");
                        CloseChannel();
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
                else
                {
                    Console.WriteLine("ADH does not support the Provider Dictionary Download\n");
                    CloseChannel();
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }
        }

        private TransportReturnCode SendItemRefreshes(ChannelSession chnl, out Error? error)
        {
            TransportReturnCode ret;
            ret = marketPriceHandler.SendItemRefreshes(chnl, CommandLine.Values("mp")!, srcDirHandler.ServiceInfo, out error);
            if (ret != TransportReturnCode.SUCCESS)
                return ret;

            ret = marketByOrderHandler.SendItemRefreshes(chnl, CommandLine.Values("mbo")!, srcDirHandler.ServiceInfo, out error);
            return ret;
        }

        private void CloseItemStreams()
        {
            marketPriceHandler.CloseStreams(channelSession, out error);
            marketByOrderHandler.CloseStreams(channelSession, out error);
        }

        /// <summary>
        /// Closes all streams for the NIProvider.
        /// </summary>
        public void Uninitialize()
        {
            Console.WriteLine("NIProvider unitializing and exiting...");
            if (channelSession.Channel == null)
            {
                channelSession.UnInit(out error);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            //close all streams
            CloseItemStreams();

            dictionaryHandler.CloseStream(channelSession, out error);
            srcDirHandler.CloseStream(channelSession, out error);

            //close login stream
            loginHandler.CloseStream(channelSession, out error);

            //flush before exiting
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
                Console.WriteLine($"Flush() failed with return code {retval}, error: {error?.Text}");
            }
        }

        private void CloseChannel()
        {
            channelSession.UnInit(out error);
        }

        private static void AddCommandLineArgs()
        {
            CommandLine.ProgName("NIProvider");
            CommandLine.AddOption("mp", "For each occurrence, requests item using Market Price domain.");
            CommandLine.AddOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");

            CommandLine.AddOption("h", defaultSrvrHostname, "Server host name for TCP socket");
            CommandLine.AddOption("p", defaultSrvrPortNo, "Server port number for TCP socket");

            CommandLine.AddOption("s", defaultServiceName, "Service name");
            CommandLine.AddOption("id", defaultServiceId, "ServiceId");
            CommandLine.AddOption("uname", "Login user name. Default is system user name.");
            CommandLine.AddOption("runtime", defaultRuntime, "Program runtime in seconds");
            CommandLine.AddOption("x", "Provides XML tracing of messages.");

            CommandLine.AddOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
            CommandLine.AddOption("ax", "", "Specifies the Authentication Extended information.");
            CommandLine.AddOption("aid", "", "Specifies the Application ID.");
        }

        /// <summary>
        /// The main method.
        /// </summary>
        /// <param name="args">The application arguments.</param>
        public static void Main(string[] args)
        {
            NIProvider niprovider = new NIProvider();
            niprovider.Init(args);
            niprovider.Run();
            niprovider.Uninitialize();
        }
    }
}