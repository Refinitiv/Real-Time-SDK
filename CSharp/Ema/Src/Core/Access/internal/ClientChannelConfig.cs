/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Diagnostics;
using System.Reflection;

namespace LSEG.Ema.Access
{
	internal class ClientChannelConfig
	{
        private static Assembly? _assembly = null;

        private static readonly FileVersionInfo? m_FileVersionInfo;

        private static string m_ProductInternalVersion = "EMA CSharp Edition";

        static ClientChannelConfig()
        {
            try
            {
                _assembly = Assembly.GetExecutingAssembly();
                m_FileVersionInfo = FileVersionInfo.GetVersionInfo(_assembly.Location);
            }
            catch (Exception) { }

            if (_assembly != null && m_FileVersionInfo != null && m_FileVersionInfo.ProductVersion != null)
            {
                string[] versionNumbers = m_FileVersionInfo.ProductVersion.Split('.');

                string productVersion = string.Empty;

                if (versionNumbers.Length >= 3)
                {
                    productVersion = $"{versionNumbers[0]}.{versionNumbers[1]}.{versionNumbers[2]}";
                }

                m_ProductInternalVersion = $"emacsharp{productVersion}.L1.all.beta";
            }
        }

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

        }

        // Reactor connection info for this channel.
        public ReactorConnectInfo ConnectInfo { get; set; }

        public int HighWaterMark { get; set; }

        // Name of this channel
        public string Name { get; set; } = string.Empty;

        // Direct Write config for this channel
        public bool DirectWrite { get; set; }

        // Clears the Channel info and sets the default options.
        public void Clear()
        {
            ConnectInfo.Clear();
            ConnectInfo.ConnectOptions.UserSpecObject = this;
            ConnectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            ConnectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();
            ConnectInfo.ConnectOptions.ProtocolType = (Eta.Transports.ProtocolType)Codec.ProtocolType();
            ConnectInfo.ConnectOptions.ComponentVersion = m_ProductInternalVersion;
            ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "";
            ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "";
            Name = string.Empty;
            HighWaterMark = 0;
            DirectWrite = false;
        }

        public void Copy(ClientChannelConfig destConfig)
        {
            ConnectInfo.Copy(destConfig.ConnectInfo);
            destConfig.ConnectInfo.ConnectOptions.UserSpecObject = destConfig;
            destConfig.Name = Name;
            destConfig.HighWaterMark = HighWaterMark;
            destConfig.DirectWrite = DirectWrite;
        }

        // Defaults used across different classes and helpers 
        internal const string DefaultHost = "localhost";
        internal const string DefaultPort = "14002";


        // StringToConnectionType and StringToCompressionType handled as Enums since the underlying ETA implementation also expects an enum.
        // For Programmatic configuration, they will be cast from an int after verifying that the value is in range.
        internal static ConnectionType StringToConnectionType(string connType) => connType.ToUpper() switch
        {
            "RSSL_SOCKET"    => ConnectionType.SOCKET,
            "RSSL_ENCRYPTED" => ConnectionType.ENCRYPTED,
            _                => throw new OmmInvalidConfigurationException("Connection Type: " + connType + " not recognized. Acceptable inputs: \"RSSL_SOCKET\", \"RSSL_ENCRYPTED\".")

        };

        internal static CompressionType StringToCompressionType(string compType) => compType switch
        {
            "None" => CompressionType.NONE,
            "ZLib" => CompressionType.ZLIB,
            "LZ4" => CompressionType.LZ4,
            _ => throw new OmmInvalidConfigurationException("Compression Type: " + compType + " not recognized. Acceptable inputs: \"None\", \"ZLib\", \"LZ4\".")
        };
    }
}