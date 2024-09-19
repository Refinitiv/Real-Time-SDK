/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;

namespace LSEG.Ema.PerfTools.EMA_NiProvPerf
{
    public class EMANiProvPerf
    {
        private EmaNiProviderPerfConfig config;
        private XmlMsgData xmlMsgData;
        private ProviderStats providerStats;

        private XmlItemInfoList? xmlItemInfoList;

        public EMANiProvPerf()
        {
            config = new EmaNiProviderPerfConfig();
            xmlMsgData = new XmlMsgData();
            providerStats = new ProviderStats();
        }

        public static void Main(string[] args)
        {
            EMANiProvPerf niProvPerf = new EMANiProvPerf();
            niProvPerf.Initialize(args);
            niProvPerf.Run();
        }

        private void Initialize(string[] args)
        {
            config.Init(args);
            if (xmlMsgData.ParseFile(config.MsgFilename!) == PerfToolsReturnCode.FAILURE)
            {
                Console.Error.WriteLine($"Failed to load message data from file {config.MsgFilename}.\n");
                Environment.Exit(-1);
            }
            xmlItemInfoList = new XmlItemInfoList(config.ItemRequestCount);
            if (xmlItemInfoList.ParseFile(config.ItemFilename!) == PerfToolsReturnCode.FAILURE)
            {
                Console.Error.WriteLine($"Failed to load item list from file '{config.ItemFilename}'.\n");
                Environment.Exit(-1);
            }

            providerStats.Initialize(config, () => new NIProviderThread(config, xmlMsgData));
        }

        private void Run()
        {
            providerStats.Run();
        }
    }
}
