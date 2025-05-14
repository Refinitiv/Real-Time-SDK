/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Security.Authentication;
using System.Text;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// The ChannelInformation interface provides channel information to an application
    /// </summary>
    /// <remarks>
    /// For Consumer applications, this channel information is about the out bound channel
    /// (e.g., the channel used by a Consumer application to connect to a RTDS or RTC) used
    /// to connect for receiving data (Consumer)
    /// </remarks>
    public sealed class ChannelInformation
    {
        private StringBuilder m_stringBuilder = new ();

        /// <summary>
        /// Constructir for ChannelInformation
        /// </summary>
        public ChannelInformation() 
        {
            Clear();
        }

        /// <summary>
        /// Constructor for ChannelInformation
        /// </summary>
        /// <param name="reactorChannel"><see cref="ReactorChannel"/> to initialize current instant of Channelinformation with.</param>
        internal ChannelInformation(ReactorChannel reactorChannel)
        {
            Set(reactorChannel);
        }

        /// <summary>
        /// Gets the reactor channel state from the ChannelState class.
        /// </summary>
        public ChannelState ChannelState { get; set; }

        /// <summary>
        /// The connection type from the ConnectionTypes class.
        /// </summary>
        public ConnectionType ConnectionType { get; set; }

        /// <summary>
        /// The host name as a string.
        /// </summary>
        public string? Hostname { get; set; }

        /// <summary>
        /// Gets the IP address of the connected client. 
        /// This is set only for IProvider applications.
        /// </summary>
        public string? IpAddress { get; set; }

        /// <summary>
        /// Port being used by channel. Valid for SOCKET connection type.
        /// </summary>
        public int Port { get; set; }

        /// <summary>
        /// Gets the protocol type.
        /// </summary>
        public ProtocolType ProtocolType { get; set; }

        /// <summary>
        /// Gets the major version.
        /// </summary>
        public int MajorVersion { get; set; }

        /// <summary>
        /// Gets the minor version.
        /// </summary>
        public int MinorVersion { get; set; }

        /// <summary>
        /// Gets the ping timeout.
        /// </summary>
        public int PingTimeout { get; set; }

        /// <summary>
        /// Gets the max fragment size.
        /// </summary>
        public int MaxFragmentSize { get; set; }

        /// <summary>
        /// Gets the maximum number of output buffers.
        /// </summary>
        public int MaxOutputBuffers { get; set; }

        /// <summary>
        /// Gets the guaranteed number of output buffers.
        /// </summary>
        public int GuaranteedOutputBuffers { get; set; }

        /// <summary>
        /// Gets the number of input buffers.
        /// </summary>
        public int NumInputBuffers { get; set; }

        /// <summary>
        /// Gets the systems send Buffer size
        /// </summary>
        public int SysSendBufSize { get; set; }

        /// <summary>
        /// Gets the systems receive Buffer size.
        /// </summary>
        public int SysRecvBufSize { get; set; }

        /// <summary>
        /// Gets the compression type, if it is enabled.
        /// </summary>
        public CompressionType CompressionType { get; set; }

        /// <summary>
        /// Gets the compression threshold
        /// </summary>
        public int CompressionThreshold { get; set; }

        /// <summary>
        /// The encrypted connection type from the ConnectionTypes class. 
        /// This property is used when <see cref="ConnectionType"/> is <see cref="ConnectionType.ENCRYPTED"/>.
        /// </summary>
        public ConnectionType EncryptedConnectionType { get; set; }

        /// <summary>
        /// The string representation of the connected component information.
        /// </summary>
        public string? ComponentInfo { get; set; }

        /// <summary>
        /// The EMA's configuration session channel name
        /// </summary>
        public string SessionChannelName { get; set; } = string.Empty;

        /// <summary>
        ///  The EMA's configuration channel name
        /// </summary>
        public string ChannelName { get; set; } = string.Empty;

        /// <summary>
        /// Gets the encryption protocol type used to authenticate the the <see cref="ConnectionType.ENCRYPTED"/> connection type.
        /// </summary>
        /// <remarks>This property is valid only for the <see cref="ConnectionType.ENCRYPTED"/> connection type.</remarks>
        /// <value>The negotiated protocol for the Channel; otherwise <see cref="SslProtocols.None"/></value>
        public SslProtocols EncryptionProtocol { get; internal set; }

        /// <summary>
        /// Clears the ChannelInformation. 
        /// Invoking Clear() resets all member variables to their default values.
        /// </summary>
        public void Clear()
        {
            ChannelState = Access.ChannelState.CLOSED;
            ConnectionType = Access.ConnectionType.UNIDENTIFIED;
            ProtocolType = Access.ProtocolType.UNKNOWN;
            PingTimeout = 0;
            MajorVersion = 0;
            MinorVersion = 0;
            IpAddress = null;
            Hostname = null;
            ComponentInfo = null;
            Port = 0;
            MaxFragmentSize = 0;
            MaxOutputBuffers = 0;
            GuaranteedOutputBuffers = 0;
            NumInputBuffers = 0;
            SysSendBufSize = 0;
            SysRecvBufSize = 0;
            CompressionType = 0;
            CompressionThreshold = 0;
            EncryptedConnectionType = ConnectionType.UNIDENTIFIED;
            EncryptionProtocol = SslProtocols.None;
            SessionChannelName  = string.Empty;
            ChannelName  = string.Empty;
        }

        internal void Set(ReactorChannel reactorChannel)
        {
            Clear();

            if (reactorChannel == null)
                return;

            ReactorChannelInfo channelInfo = new ReactorChannelInfo();
            if (reactorChannel.Info(channelInfo, out _) != ReactorReturnCode.SUCCESS)
            {
                ComponentInfo = "unavailable";
            }
            else
            {
                if (channelInfo.ChannelInfo == null || channelInfo.ChannelInfo.ComponentInfoList == null || channelInfo.ChannelInfo.ComponentInfoList.Count == 0)
                    ComponentInfo = "unavailable";
                else
                {
                    ComponentInfo = channelInfo.ChannelInfo.ComponentInfoList[0].ComponentVersion.ToString();

                    // the ClientHostname and ClientIP methods will return non-null values only for IProvider clients
                    Hostname = channelInfo.ChannelInfo.ClientHostname;
                    IpAddress = channelInfo.ChannelInfo.ClientIP;
                    MaxFragmentSize = channelInfo.ChannelInfo.MaxFragmentSize;
                    MaxOutputBuffers = channelInfo.ChannelInfo.MaxOutputBuffers;
                    GuaranteedOutputBuffers = channelInfo.ChannelInfo.GuaranteedOutputBuffers;
                    NumInputBuffers = channelInfo.ChannelInfo.NumInputBuffers;
                    SysSendBufSize = channelInfo.ChannelInfo.SysSendBufSize;
                    SysRecvBufSize = channelInfo.ChannelInfo.SysRecvBufSize;
                    CompressionType = EtaToEmaCompressionType(channelInfo.ChannelInfo.CompressionType);
                    CompressionThreshold = channelInfo.ChannelInfo.CompressionThresHold;
                    EncryptionProtocol = channelInfo.ChannelInfo.EncryptionProtocol;
                }
            }

            // _hostname will be null for Consumer and NiProvider applications.
            if (Hostname == null)
            {
                Hostname = reactorChannel.HostName;
            }

            Port = reactorChannel.Port;

            IChannel? channel = reactorChannel.Channel;

            if (channel != null)
            {
                ConnectionType = EtaToEmaConnectionType(channel.ConnectionType);

                if(ConnectionType == ConnectionType.ENCRYPTED)
                {
                    EncryptedConnectionType = ConnectionType.SOCKET;
                }

                ProtocolType = EtaToEmaProtocolType(channel.ProtocolType);
                MajorVersion = channel.MajorVersion;
                MinorVersion = channel.MinorVersion;
                PingTimeout = channel.PingTimeOut;
                ChannelState = EtaToEmaChannelState(channel.State);
            }
            else
            {
                ConnectionType = ConnectionType.UNIDENTIFIED;
                ProtocolType = Access.ProtocolType.UNKNOWN;
                MajorVersion = MinorVersion = PingTimeout = 0;
            }
        }

        /// <summary>
        /// String representation of the current <see cref="ChannelInformation"/> instance.
        /// </summary>
        /// <returns>string value</returns>
        public override string ToString()
        {
            m_stringBuilder.Length = 0;

            if(!string.IsNullOrEmpty(ChannelName))
            {
                m_stringBuilder.AppendLine($"channelName: {ChannelName}");
            }

            if(!string.IsNullOrEmpty(SessionChannelName))
            {
                m_stringBuilder.AppendLine($"sessionChannelName: {SessionChannelName}");
            }

            m_stringBuilder.Append("hostname: ").Append(Hostname).AppendLine()
                .Append("\tIP address: ").Append(IpAddress).AppendLine()
                .Append("\tport: ").Append(Port).AppendLine()
                .Append("\tconnected component info: ").Append(ComponentInfo).AppendLine()
                .Append("\tchannel state: ");

            switch (ChannelState)
            {
                case Access.ChannelState.CLOSED:
                    m_stringBuilder.Append("closed");
                    break;
                case Access.ChannelState.INACTIVE:
                    m_stringBuilder.Append("inactive");
                    break;
                case Access.ChannelState.INITIALIZING:
                    m_stringBuilder.Append("initializing");
                    break;
                case Access.ChannelState.ACTIVE:
                    m_stringBuilder.Append("active");
                    break;
                default:
                    m_stringBuilder.Append(ChannelState);
                    break;
            }

            if (ConnectionType == Access.ConnectionType.UNIDENTIFIED)
                m_stringBuilder.AppendLine().Append("\tconnection type: unknown")
                    .AppendLine().Append("\tprotocol type: ");
            else
                m_stringBuilder.AppendLine().Append("\tconnection type: ")
                    .Append(ConnectionType.ToString())
                    .AppendLine().Append("\tprotocol type: ");

            if (ProtocolType == Access.ProtocolType.RWF)
                m_stringBuilder.Append("Rssl wire format");
            else if (ProtocolType == Access.ProtocolType.JSON)
                m_stringBuilder.Append("Rssl JSON format");
            else
                m_stringBuilder.Append("unknown wire format");

            if (ConnectionType == Access.ConnectionType.ENCRYPTED)
            {
                m_stringBuilder.AppendLine()
                    .Append("\tencrypted connection type: ")
                    .Append(EncryptedConnectionType.ToString());
            }

            m_stringBuilder.AppendLine().Append("\tmajor version: ")
                .Append(MajorVersion).AppendLine()
                .Append("\tminor version: ").Append(MinorVersion)
                .AppendLine().Append("\tping timeout: ").Append(PingTimeout);

            m_stringBuilder.AppendLine().Append("\tmax fragmentation size: ").Append(MaxFragmentSize)
            .AppendLine().Append("\tmax output buffers: ").Append(MaxOutputBuffers)
            .AppendLine().Append("\tguaranteed output buffers: ").Append(GuaranteedOutputBuffers)
            .AppendLine().Append("\tnumber input buffers: ").Append(NumInputBuffers)
            .AppendLine().Append("\tsystem send buffer size: ").Append(SysSendBufSize)
            .AppendLine().Append("\tsystem receive buffer size: ").Append(SysRecvBufSize)
            .AppendLine().Append("\tcompression type: ");

            switch (CompressionType)
            {
                case Access.CompressionType.ZLIB:
                    m_stringBuilder.Append("ZLIB");
                    break;
                case Access.CompressionType.LZ4:
                    m_stringBuilder.Append("LZ4");
                    break;
                case Access.CompressionType.NONE:
                default:
                    m_stringBuilder.Append("none");
                    break;
            }

            m_stringBuilder.AppendLine().Append("\tcompression threshold: ").Append(CompressionThreshold);
            m_stringBuilder.AppendLine().Append($"\tencryptionProtocol: {EncryptionProtocol}");

            return m_stringBuilder.ToString();
        }

        internal static ConnectionType EtaToEmaConnectionType(Eta.Transports.ConnectionType type)
        {
            switch (type)
            {
                case Eta.Transports.ConnectionType.SOCKET:
                    return ConnectionType.SOCKET;
                case Eta.Transports.ConnectionType.ENCRYPTED:
                    return ConnectionType.ENCRYPTED;
                default:
                    return ConnectionType.UNIDENTIFIED;
            }
        }

        internal static CompressionType EtaToEmaCompressionType(Eta.Transports.CompressionType type)
        {
            switch (type)
            {
                case Eta.Transports.CompressionType.NONE:
                    return CompressionType.NONE;
                case Eta.Transports.CompressionType.ZLIB:
                    return CompressionType.ZLIB;
                case Eta.Transports.CompressionType.LZ4:
                    return CompressionType.LZ4;
                default:
                    return CompressionType.NONE;
            }
        }

        internal static ProtocolType EtaToEmaProtocolType(Eta.Transports.ProtocolType type)
        {
            switch (type)
            {
                case Eta.Transports.ProtocolType.RWF:
                    return ProtocolType.RWF;
                default:
                    return ProtocolType.UNKNOWN;
            }
        }

        internal static ChannelState EtaToEmaChannelState(Eta.Transports.ChannelState type)
        {
            switch (type)
            {
                case Eta.Transports.ChannelState.CLOSED:
                    return ChannelState.CLOSED;
                case Eta.Transports.ChannelState.ACTIVE:
                    return ChannelState.ACTIVE;
                case Eta.Transports.ChannelState.INITIALIZING:
                    return ChannelState.INITIALIZING;
                case Eta.Transports.ChannelState.INACTIVE:
                    return ChannelState.INACTIVE;
                default:
                    return 0;
            }
        }

        internal static Eta.Transports.ConnectionType EmaToEtaConnectionType(ConnectionType type)
        {
            switch (type)
            {
                case ConnectionType.SOCKET:
                    return Eta.Transports.ConnectionType.SOCKET;
                case ConnectionType.ENCRYPTED:
                    return Eta.Transports.ConnectionType.ENCRYPTED;
                default:
                    return Eta.Transports.ConnectionType.SOCKET;
            }
        }

        internal static Eta.Transports.CompressionType EmaToEtaCompressionType(CompressionType type)
        {
            switch (type)
            {
                case CompressionType.NONE:
                    return Eta.Transports.CompressionType.NONE;
                case CompressionType.ZLIB:
                    return Eta.Transports.CompressionType.ZLIB;
                case CompressionType.LZ4:
                    return Eta.Transports.CompressionType.LZ4;
                default:
                    return 0;
            }
        }

        internal static Eta.Transports.ProtocolType EmaToEtaProtocolType(ProtocolType type)
        {
            return Eta.Transports.ProtocolType.RWF;
        }

        internal static Eta.Transports.ChannelState EmaToEtaChannelState(ChannelState type)
        {
            switch (type)
            {
                case ChannelState.CLOSED:
                    return Eta.Transports.ChannelState.CLOSED;
                case ChannelState.ACTIVE:
                    return Eta.Transports.ChannelState.ACTIVE;
                case ChannelState.INITIALIZING:
                    return Eta.Transports.ChannelState.INITIALIZING;
                case ChannelState.INACTIVE:
                    return Eta.Transports.ChannelState.INACTIVE;
                default:
                    return 0;
            }
        }
    }
}
