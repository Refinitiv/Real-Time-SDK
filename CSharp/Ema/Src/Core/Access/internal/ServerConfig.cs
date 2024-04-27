/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System.Collections.Generic;
using System.Net.Security;

namespace LSEG.Ema.Access
{
    // This class represents the configuration of a single Server in the Server Group.
    internal class ServerConfig
	{

        public string Name { get; set; }
        public BindOptions BindOptions { get; set; }
        public bool DirectWrite { get; set; }
        public int HighWaterMark { get; set; }
        public int InitializationTimeout { get; set; }
        // Compression Threshold for this channel
        public int CompressionThreshold { get; set; }

        public bool CompressionThresholdSet { get; set; } = false;

        internal ServerConfig()
        {
            Name = string.Empty;
            BindOptions = new BindOptions();
            Clear();
        }

        internal ServerConfig(ServerConfig oldConfig)
        {
            Name = string.Empty;
            BindOptions = new BindOptions();
            Clear();

            oldConfig.BindOptions.Copy(BindOptions);
            DirectWrite = oldConfig.DirectWrite;
            HighWaterMark = oldConfig.HighWaterMark;
            InitializationTimeout = oldConfig.InitializationTimeout;
            CompressionThresholdSet = oldConfig.CompressionThresholdSet;
            CompressionThreshold = oldConfig.CompressionThreshold;
        }

        internal void Copy(ServerConfig destConfig)
        {
            BindOptions.Copy(destConfig.BindOptions);
            destConfig.Name = Name;
            destConfig.DirectWrite = DirectWrite;
            destConfig.HighWaterMark = HighWaterMark;
            destConfig.InitializationTimeout = InitializationTimeout;
            destConfig.CompressionThresholdSet = CompressionThresholdSet;
            destConfig.CompressionThreshold = CompressionThreshold;
        }

        internal void Clear()
        {
            Name = string.Empty;
            BindOptions.Clear();
            BindOptions.MajorVersion = Codec.MajorVersion();
            BindOptions.MinorVersion = Codec.MinorVersion();
            BindOptions.ComponentVersion = ComponentVersion.ProductInternalVersion;
            BindOptions.MinPingTimeout = 20;
            BindOptions.PingTimeout = 60;
            BindOptions.GuaranteedOutputBuffers = 100;
            BindOptions.MaxOutputBuffers = 150;
            BindOptions.SharedPoolSize = 50;
            BindOptions.ServiceName = "14002";
            BindOptions.SysRecvBufSize = 65535;
            BindOptions.SysSendBufSize = 65535;
            BindOptions.TcpOpts.TcpNoDelay = true;
            BindOptions.ForceCompression = true;
            CompressionThresholdSet = false;
            CompressionThreshold = 30;
            DirectWrite = false;
            HighWaterMark = 6000;
            InitializationTimeout = 60;

        }

        // This method converts a comma separated string of either cipher names or numbers into a List of TlsCipherSuite enums. 
        static internal List<TlsCipherSuite> StringToCipherList(string cipherString, ConfigErrorList error)
        {
            List<TlsCipherSuite> cipherList = new List<TlsCipherSuite>();

            string[] cipherArray = cipherString.Split(',');

            foreach (string cipher in cipherArray)
            {
                TlsCipherSuite cipherSuite;

                if (System.Enum.TryParse<TlsCipherSuite>(cipher.Trim(), out cipherSuite))
                {
                    cipherList.Add(cipherSuite);
                }
                else
                {
                    error.Add($"Unknown cipher suite string: {cipher.Trim()}", LoggerLevel.ERROR);
                }
            }

            return cipherList;
        }
    }
}