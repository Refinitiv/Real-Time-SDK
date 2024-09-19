/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;

namespace LSEG.Ema.PerfTools.EMA_IProvPerf
{
    public class EmaIProvPerf
    {
        private EmaIProvPerfConfig config;
        private XmlMsgData xmlMsgData;
        private ProviderStats providerStats;

        public EmaIProvPerf()
        {
            config = new EmaIProvPerfConfig();
            xmlMsgData = new XmlMsgData();
            providerStats = new ProviderStats();
        }

        public static void Main(string[] args)
        {
            EmaIProvPerf iProvPerf = new EmaIProvPerf();
            iProvPerf.Initialize(args);
            iProvPerf.Run();
        }

        private void Initialize(string[] args)
        {
            config.Init(args);
            if (xmlMsgData.ParseFile(config.MsgFilename!) == PerfToolsReturnCode.FAILURE)
            {
                Console.WriteLine($"Failed to load message data from file '{config.MsgFilename}'.\n");
                Environment.Exit(-1);
            }
            providerStats.Initialize(config, () => new IProviderThread(config, xmlMsgData));
        }

        private void Run()
        {
            providerStats.Run();
        }
    }
}
