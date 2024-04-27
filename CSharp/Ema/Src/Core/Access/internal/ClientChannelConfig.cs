/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access
{
    internal class ClientChannelConfig
    {
        // Reactor connection info for this channel.
        public ReactorConnectInfo ConnectInfo { get; set; }

        public int HighWaterMark { get; set; }

        // Name of this channel
        public string Name { get; set; } = string.Empty;

        // Direct Write config for this channel
        public bool DirectWrite { get; set; }

        // Compression Threshold for this channel
        public int CompressionThreshold { get; set; }

        public bool CompressionThresholdSet { get; set; } = false;

        public ClientChannelConfig()
        {
            ConnectInfo = new ReactorConnectInfo();
            Clear();
        }

        public ClientChannelConfig(ClientChannelConfig oldConfig)
        {
            ConnectInfo = new ReactorConnectInfo();

            oldConfig.ConnectInfo.Copy(ConnectInfo);
            ConnectInfo.ConnectOptions.UserSpecObject = this;
            Name = oldConfig.Name;
            HighWaterMark = oldConfig.HighWaterMark;
            DirectWrite = oldConfig.DirectWrite;
            CompressionThresholdSet = oldConfig.CompressionThresholdSet;
            CompressionThreshold = oldConfig.CompressionThreshold;
        }

        // Clears the Channel info and sets the default options.
        public void Clear()
        {
            ConnectInfo.Clear();
            ConnectInfo.ConnectOptions.UserSpecObject = this;
            ConnectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            ConnectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();
            ConnectInfo.ConnectOptions.ProtocolType = (Eta.Transports.ProtocolType)Codec.ProtocolType();
            ConnectInfo.ConnectOptions.ComponentVersion = ComponentVersion.ProductInternalVersion;
            ConnectInfo.ConnectOptions.NumInputBuffers = 100;
            ConnectInfo.ConnectOptions.GuaranteedOutputBuffers = 100;
            Name = string.Empty;
            HighWaterMark = 0;
            DirectWrite = false;
            CompressionThresholdSet = false;
            CompressionThreshold = 30;
        }

        public void Copy(ClientChannelConfig destConfig)
        {
            ConnectInfo.Copy(destConfig.ConnectInfo);
            destConfig.ConnectInfo.ConnectOptions.UserSpecObject = destConfig;
            destConfig.Name = Name;
            destConfig.HighWaterMark = HighWaterMark;
            destConfig.DirectWrite = DirectWrite;
            destConfig.CompressionThresholdSet = CompressionThresholdSet;
            destConfig.CompressionThreshold = CompressionThreshold;
        }

        // StringToConnectionType and StringToCompressionType handled as Enums since the underlying ETA implementation also expects an enum.
        // For Programmatic configuration, they will be cast from an int after verifying that the value is in range.
        internal static Eta.Transports.ConnectionType StringToConnectionType(string connType) => connType.ToUpper() switch
        {
            "RSSL_SOCKET" => Eta.Transports.ConnectionType.SOCKET,
            "RSSL_ENCRYPTED" => Eta.Transports.ConnectionType.ENCRYPTED,
            _ => throw new OmmInvalidConfigurationException("Connection Type: " + connType + " not recognized. Acceptable inputs: \"RSSL_SOCKET\", \"RSSL_ENCRYPTED\".")
        };

        internal static Eta.Transports.CompressionType StringToCompressionType(string compType) => compType switch
        {
            "None" => Eta.Transports.CompressionType.NONE,
            "ZLib" => Eta.Transports.CompressionType.ZLIB,
            "LZ4" => Eta.Transports.CompressionType.LZ4,
            _ => throw new OmmInvalidConfigurationException("Compression Type: " + compType + " not recognized. Acceptable inputs: \"None\", \"ZLib\", \"LZ4\".")
        };
    }
}