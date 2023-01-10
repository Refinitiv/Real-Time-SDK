/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using Provider = LSEG.Eta.PerfTools.Common.Provider;

namespace LSEG.Eta.Perftools.NIProvPerf
{
    /// <summary>The main NIProvPerf application. Implements a Non-Interactive Provider.
    /// It connects to an ADH, and provides images and update bursts for a given
    /// number of items.</summary>
    ///
    /// The purpose of this application is to measure performance of the ETA transport,
    /// encoders and decoders, in providing Level I Market Price content to the
    /// Refinitiv Real-Time Distribution System Advanced Data Hub (ADH).
    /// <p>
    /// <em>Summary</em>
    /// <p>
    /// The provider creates two categories of threads:
    /// <ul>
    /// <li>A main thread, which collects and records statistical information.
    /// <li>Multiple (configured number) provider threads, each of which connects to an
    /// ADH and provides market data.
    /// </ul>
    /// <p>
    /// The provider may be configured to provide updates at various rates. To measure
    /// latency, a timestamp is randomly placed in each burst of updates. The consumer
    /// then decodes the timestamp from the update to determine the end-to-end latency.
    /// <p>
    /// This application also measures memory and CPU usage.
    /// <p>
    /// For more detailed information on the performance measurement applications,
    /// see the ETA Open Source Performance Tools Guide.
    /// <p>
    /// The following configuration files are required:
    /// <ul>
    /// <li>RDMFieldDictionary and enumtype.def
    /// <li>350k.xml file
    /// <li>MsgData.xml file
    /// </ul>
    public class NIProvPerf
    {
        private XmlMsgData m_XmlMsgData;                // message data information from XML file
        private volatile bool m_ShutdownApp = false;    // Indicates whether or not application should be shutdown
        private Provider m_Provider;                    // The provider used by this application. Handles the statistic for all threads used by this provider
        private Error m_Error;                          // error information
        private InitArgs m_InitArgs;                    // arguments for initializing transport

        private CountdownEvent m_RunLoopExited = new CountdownEvent(1);

        public NIProvPerf()
        {
            m_Provider = new Provider();
            m_Error = new();
            m_InitArgs = new();
            m_XmlMsgData = new XmlMsgData();
        }

        /// <summary>
        /// Run NIProvPerf
        /// </summary>
        public void Run()
        {
            long intervalSeconds = 0, currentRuntimeSec = 0;
            // Initialize runtime timer
            DateTime niProvRuntime = DateTime.Now + NIProvPerfConfig.RunTime;

            // this is the main loop
            while (!m_ShutdownApp)
            {
                try
                {
                    Thread.Sleep(1000);
                }
                catch (Exception)
                {
                    Console.WriteLine("Failed to sleep for 1 second");
                    Environment.Exit(-1);
                }

                ++currentRuntimeSec;
                ++intervalSeconds;

                // collect statistics at write statistics interval
                if (intervalSeconds == NIProvPerfConfig.WriteStatsInterval)
                {
                    m_Provider.CollectStats(true, NIProvPerfConfig.DisplayStats, currentRuntimeSec, NIProvPerfConfig.WriteStatsInterval);
                    intervalSeconds = 0;
                }

                // Handle runtime
                if (DateTime.Now >= niProvRuntime)
                {
                    Console.Write($"\nRun time of {NIProvPerfConfig.RunTime} seconds has expired.\n\n");
                    break;
                }
            }

            Stop();
            m_RunLoopExited.Signal(1);
        }

        private void Stop()
        {
            StopProviderThreads();
            m_Provider.CollectStats(false, false, 0, 0);
            m_Provider.PrintFinalStats();
            CleanUpAndExit();
        }

        /// <summary>
        /// Initializes NIProvPerf application
        /// </summary>
        /// <param name="args">input arguments</param>
        private void Initialize(string[] args)
        {
            // Read in configuration and echo it.
            NIProvPerfConfig.Init(args);
            Console.WriteLine(NIProvPerfConfig.ConvertToString);

            // Parse message data XML file
            if (m_XmlMsgData.ParseFile(NIProvPerfConfig.MsgFilename) == PerfToolsReturnCode.FAILURE)
            {
                Console.WriteLine($"Failed to load message data from file '{NIProvPerfConfig.MsgFilename}'.");
                Environment.Exit(-1);
            }

            // The application will exit if error happens during initialization
            m_Provider.Init(xmlMsgData => new NIProviderThread(xmlMsgData), ProviderType.PROVIDER_NONINTERACTIVE, m_XmlMsgData, NIProvPerfConfig.SummaryFilename!);

            if (!NIProvPerfConfig.UseReactor)
            {
                // Initialize ETA Transport
                m_InitArgs.Clear();
                m_InitArgs.GlobalLocking = NIProvPerfConfig.ThreadCount > 1;

                if (Transport.Initialize(m_InitArgs, out m_Error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Transport failed to initialize, error: {m_Error?.Text}");
                    Environment.Exit(-1);
                }
            }

            m_Provider.StartThreads();
        }

        /// <summary>
        /// Stop all provider threads
        /// </summary>
        private void StopProviderThreads()
        {
            for (int i = 0; i < ProviderPerfConfig.ThreadCount; i++)
            {
                m_Provider.ProviderThreadList![i].Shutdown = true;
            }

            for (int i = 0; i < ProviderPerfConfig.ThreadCount; i++)
            {

                // wait for provider thread cleanup or timeout
                while (!m_Provider.ProviderThreadList![i].ShutdownAck)
                {
                    try
                    {
                        Thread.Sleep(1000);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Thread.Sleep(1000) failed: {e.Message}");
                        Environment.Exit(-1);
                    }
                }
            }

            Console.Write("\nShutting down.\n\n");
        }

        /// <summary>
        /// Cleanup and exit NIProvPerf application
        /// </summary>
        private void CleanUpAndExit()
        {
            //cleanup provider
            m_Provider.Cleanup();

            //uninitialize ETA transport and exit
            Transport.Uninitialize();
            Console.WriteLine("Exiting.");
        }

        private void RegisterShutdownHook()
        {
            Console.CancelKeyPress += delegate
            {
                m_ShutdownApp = true;
                m_RunLoopExited.Wait();
            };
        }

        public static void Main(string[] args)
        {
            NIProvPerf niprovperf = new NIProvPerf();
            niprovperf.Initialize(args);
            niprovperf.RegisterShutdownHook();
            niprovperf.Run();
            Environment.Exit(0);
        }
    }
}
