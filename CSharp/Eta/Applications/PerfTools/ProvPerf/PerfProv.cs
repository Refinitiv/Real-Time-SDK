/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;

using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// The ProvPerf application. Implements an interactive provider,
    /// which allows the requesting of items, and responds to them with images and bursts of updates.
    /// </summary>
    public class ProvPerf
    {
        private Provider m_Provider;
        private XmlMsgData m_XmlMsgData;                // message data information from XML file
        private Error m_Error;                          // error information
        private BindOptions m_BindOptions;              // server bind options
        private AcceptOptions m_AcceptOptions;          // server accept options
        private IServer? m_Server;                       // eta server
        private InitArgs m_InitArgs;

        private volatile bool m_ShutdownApp;            // Indicates whether or not application should be shutdown

        private CountdownEvent m_RunLoopExited = new CountdownEvent(1);

        public ProvPerf()
        {
            m_Provider = new Provider();
            m_Error = new Error();
            m_BindOptions = new BindOptions();
            m_InitArgs = new InitArgs();
            m_AcceptOptions = new AcceptOptions();
            m_XmlMsgData = new XmlMsgData();
        }

        /// <summary>
        /// Main loop for provider perf application
        /// </summary>
        private void Run()
        {
            // This loop will block Socket.Select for up to 1 second and wait for accept
            // If any channel is trying to connect during this time it will be accepted
            // the time tracking parameters and counters are updated at the second interval
            // at the configured time intervals the stats will be printed
            // the loop exits when time reaches the configured end time.

            long nextTime = (long)GetTime.GetMilliseconds() + 1000L;
            int intervalSeconds = 0;
            int currentRuntimeSec = 0;
            int writeStatsInterval = ProviderPerfConfig.WriteStatsInterval;
            int runTime = ProviderPerfConfig.RunTime;
            bool displayStats = ProviderPerfConfig.DisplayStats;
            List<Socket> socketList = new List<Socket>();

            // this is the main loop
            while (!m_ShutdownApp)
            {
                int selectTime = (int)(nextTime - GetTime.GetMilliseconds());
                try
                {
                    socketList.Clear();
                    if (m_Server!.Socket != null && m_Server.Socket.IsBound)
                    {
                        socketList.Add(m_Server.Socket!);
                        Socket.Select(socketList, null, null, selectTime > 0 ? selectTime * 1000 : 0);
                    }

                    if (socketList.Count > 0)
                    {
                        if (!ProviderPerfConfig.UseReactor) // use ETA Channel
                        {
                            IChannel clientChannel = m_Server.Accept(m_AcceptOptions, out m_Error);
                            if (clientChannel == null)
                            {
                                Console.Error.WriteLine($"ETA Server Accept failed, error: {m_Error?.Text}\n");
                            }
                            else
                            {
                                Console.WriteLine($"Server accepting new channel '{clientChannel.Socket.Handle}'.\n\n");
                                SendToLeastLoadedThread(clientChannel);
                            }
                        }
        				else // use ETA VA Reactor
        				{
                            if (AcceptReactorConnection(m_Server, out var errorInfo) != ReactorReturnCode.SUCCESS)
                            {
                                Console.Error.WriteLine("AcceptReactorConnection: failed <{0}>", errorInfo?.Error.Text);
                            }
        				}
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Socket.Select failed: {e.Message}");
                    Environment.Exit(-1);
                }

                if (GetTime.GetMilliseconds() >= nextTime)
                {
                    nextTime += 1000;
                    ++intervalSeconds;
                    ++currentRuntimeSec;

                    // Check if it's time to print stats
                    if (intervalSeconds >= writeStatsInterval)
                    {
                        m_Provider.CollectStats(true, displayStats, currentRuntimeSec, writeStatsInterval);
                        intervalSeconds = 0;
                    }
                }

                // Handle run-time
                if (currentRuntimeSec >= runTime)
                {
                    Console.WriteLine($"\nRun time of {runTime} seconds has expired.\n\n");
                    break;
                }
            }

            Stop();
            m_RunLoopExited.Signal(1);
        }

        private void Init(string[] args)
        {
            ProviderPerfConfig.Init(args);

            //init binding options first to be able print effective values
            InitBindOptions();
            m_AcceptOptions.SysSendBufSize = ProviderPerfConfig.SendBufSize;
            m_AcceptOptions.SysRecvBufSize = ProviderPerfConfig.RecvBufSize;
            if (ProviderPerfConfig.SendTimeout > 0)
            {
                m_AcceptOptions.SendTimeout = ProviderPerfConfig.SendTimeout;
            }
            if (ProviderPerfConfig.RecvTimeout > 0)
            {
                m_AcceptOptions.ReceiveTimeout = ProviderPerfConfig.RecvTimeout;
            }
            Console.WriteLine(ProviderPerfConfig.ConvertToString(m_BindOptions));

            // parse message data XML file
            if (m_XmlMsgData.ParseFile(ProviderPerfConfig.MsgFilename!) == PerfToolsReturnCode.FAILURE)
            {
                Console.WriteLine($"Failed to load message data from file {ProviderPerfConfig.MsgFilename}.\n");
                Environment.Exit(-1);
            }

            m_InitArgs.Clear();

            if (!ProviderPerfConfig.UseReactor) // use ETA Channel for sending and receiving
            {
                m_InitArgs.GlobalLocking = ProviderPerfConfig.ThreadCount > 1 ? true : false;
            }
            else
            {
                m_InitArgs.GlobalLocking = true;
            }

            if (Transport.Initialize(m_InitArgs, out m_Error) != TransportReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"Error: Transport failed to initialize: {m_Error.Text}");
                Environment.Exit(-1);
            }

            m_Server = Transport.Bind(m_BindOptions, out m_Error);
            if (m_Server == null)
            {
                Console.Error.WriteLine($"Error: Transport bind failure, error: {m_Error?.Text}");
                Environment.Exit(-1);
            }
            Console.WriteLine($"\nServer bound on port {m_Server.PortNumber}");

            // The application will exit if error happens during initialization
            m_Provider.Init(xmlMsgData => new IProviderThread(xmlMsgData), ProviderType.PROVIDER_INTERACTIVE, m_XmlMsgData, ProviderPerfConfig.SummaryFilename!);
            m_Provider.StartThreads();
        }

        private void InitBindOptions()
        {
            m_BindOptions.GuaranteedOutputBuffers = ProviderPerfConfig.GuaranteedOutputBuffers;
            if (ProviderPerfConfig.MaxOutputBuffers > 0)
                m_BindOptions.MaxOutputBuffers = ProviderPerfConfig.MaxOutputBuffers;
            m_BindOptions.ServiceName = ProviderPerfConfig.PortNo;
            if (ProviderPerfConfig.InterfaceName != null)
                m_BindOptions.InterfaceName = ProviderPerfConfig.InterfaceName;

            m_BindOptions.ConnectionType = ProviderPerfConfig.ConnectionType;
            if (m_BindOptions.ConnectionType == ConnectionType.ENCRYPTED)
            {
                m_BindOptions.BindEncryptionOpts.ServerCertificate = ProviderPerfConfig.Cert;
                m_BindOptions.BindEncryptionOpts.ServerPrivateKey = ProviderPerfConfig.KeyFile;

                m_BindOptions.BindEncryptionOpts.EncryptionProtocolFlags = ProviderPerfConfig.EncryptionProtocol;
            }
            m_BindOptions.MajorVersion = Codec.Codec.MajorVersion();
            m_BindOptions.MinorVersion = Codec.Codec.MajorVersion();
            m_BindOptions.ProtocolType = Codec.Codec.RWF_PROTOCOL_TYPE;
            m_BindOptions.SysRecvBufSize = ProviderPerfConfig.RecvBufSize;
            m_BindOptions.MaxFragmentSize = ProviderPerfConfig.MaxFragmentSize;
            m_BindOptions.TcpOpts.TcpNoDelay = ProviderPerfConfig.TcpNoDelay;
        }

        private void StopProviderThreads()
        {
            for (int i = 0; i < ProviderPerfConfig.ThreadCount; i++)
            {
                m_Provider.ProviderThreadList![i].Shutdown = true;
            }

            for (int i = 0; i < ProviderPerfConfig.ThreadCount; i++)
            {
                int shutdownCount = 0;
                // wait for provider thread cleanup or timeout
                while (!m_Provider.ProviderThreadList![i].ShutdownAck && shutdownCount < 3)
                {
                    Thread.Sleep(1000);
                    shutdownCount++;
                }
            }

            Console.WriteLine("Shutting down.");
        }

        private void SendToLeastLoadedThread(IChannel channel)
        {
            IProviderThread? provThread = null;

            int minProvConnCount = 0x7fffffff;

            for (int i = 0; i < ProviderPerfConfig.ThreadCount; ++i)
            {
                IProviderThread tmpProvThread = (IProviderThread)m_Provider.ProviderThreadList![i];
                tmpProvThread.HandlerLock().EnterWriteLock();
                int connCount = tmpProvThread.ConnectionCount;
                if (connCount < minProvConnCount)
                {
                    minProvConnCount = connCount;
                    provThread = tmpProvThread;
                }
                tmpProvThread.HandlerLock().ExitWriteLock();
            }

            provThread!.HandlerLock().EnterWriteLock();
            provThread.AcceptNewChannel(channel);
            provThread.HandlerLock().ExitWriteLock();
        }

        private ReactorReturnCode AcceptReactorConnection(IServer server, out ReactorErrorInfo? errorInfo)
        {
            IProviderThread provThread = (IProviderThread)m_Provider.ProviderThreadList![0];
            int minProvConnCount = provThread.ConnectionCount;

            // find least loaded thread
            for (int i = 1; i < ProviderPerfConfig.ThreadCount; ++i)
            {
                IProviderThread tmpProvThread = (IProviderThread) m_Provider.ProviderThreadList[i];
                int connCount = tmpProvThread.ConnectionCount;
                if (connCount < minProvConnCount)
                {
                    minProvConnCount = connCount;
                    provThread = tmpProvThread;
                }
            }

            // accept new reactor channel
            return provThread.AcceptNewReactorChannel(server, out errorInfo);
        }

        private void Cleanup()
        {
            m_Provider.Cleanup();
            m_Server!.Close(out m_Error);

            Transport.Uninitialize();
        }

        private void Stop()
        {
            StopProviderThreads();
            m_Provider.PrintFinalStats();
            Cleanup();
        }

        private void RegisterShutdownDelegate()
        {
            Console.CancelKeyPress += delegate {
                try
                {
                    m_ShutdownApp = true;
                    m_RunLoopExited.Wait();
                    Environment.Exit(0);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.StackTrace);
                }
            };
        }

        public static void Main(string[] args)
        {
            ProvPerf provperf = new ProvPerf();
            provperf.RegisterShutdownDelegate();
            provperf.Init(args);
            provperf.Run();
            Environment.Exit(0);
        }
    }
}
