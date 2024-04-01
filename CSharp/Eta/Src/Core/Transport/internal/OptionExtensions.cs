/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.Internal
{
    internal static class OptionExtensions
    {
        internal static TcpOpts CopyTo(this TcpOpts source, TcpOpts dest)
        {
            dest.TcpNoDelay = source.TcpNoDelay;
            return source;
        }

        internal static UnifiedNetworkInfo CopyTo(this UnifiedNetworkInfo source, UnifiedNetworkInfo dest)
        {
            dest.ServiceName = source.ServiceName;
            dest.Address = source.Address;
            dest.InterfaceName = source.InterfaceName;
            dest.Port = source.Port;

            return source;
        }

        internal static ConnectOptions CopyTo(this ConnectOptions source, ConnectOptions dest)
        {
            dest.Blocking = source.Blocking;
            dest.ChannelReadLocking = source.ChannelReadLocking;
            dest.ChannelWriteLocking = source.ChannelWriteLocking;
            dest.ComponentVersion = source.ComponentVersion;
            dest.CompressionType = source.CompressionType;
            dest.ConnectionType = source.ConnectionType;
            dest.GuaranteedOutputBuffers = source.GuaranteedOutputBuffers;
            dest.MajorVersion = source.MajorVersion;
            dest.MinorVersion = source.MinorVersion;
            dest.NumInputBuffers = source.NumInputBuffers;
            dest.PingTimeout = source.PingTimeout;
            dest.ProtocolType = source.ProtocolType;
            dest.SysRecvBufSize = source.SysRecvBufSize;
            dest.SysSendBufSize = source.SysSendBufSize;
            source.TcpOpts.CopyTo(dest.TcpOpts);
            source.UnifiedNetworkInfo.CopyTo(dest.UnifiedNetworkInfo);
            dest.UserSpecObject = source.UserSpecObject;
            source.EncryptionOpts.CopyTo(dest.EncryptionOpts);
            source.ProxyOptions.CopyTo(dest.ProxyOptions);
            dest.ConnectTimeout = source.ConnectTimeout;
            dest.SendTimeout = source.SendTimeout;
            dest.ReceiveTimeout = source.ReceiveTimeout;

            return dest;
        }

        internal static BindOptions CopyTo(this BindOptions source, BindOptions dest)
        {
            dest.ChannelIsBlocking = source.ChannelIsBlocking;
            dest.ClientToServerPings = source.ClientToServerPings;
            dest.ComponentVersion = source.ComponentVersion;
            dest.CompressionLevel = source.CompressionLevel;
            dest.CompressionType = source.CompressionType;
            dest.ConnectionType = source.ConnectionType;
            dest.ForceCompression = source.ForceCompression;
            dest.GuaranteedOutputBuffers = source.GuaranteedOutputBuffers;
            dest.InterfaceName = source.InterfaceName;
            dest.MajorVersion = source.MajorVersion;
            dest.MaxFragmentSize = source.MaxFragmentSize;
            dest.MaxOutputBuffers = source.MaxOutputBuffers;
            dest.MinorVersion = source.MinorVersion;
            dest.MinPingTimeout = source.MinPingTimeout;
            dest.NumInputBuffers = source.NumInputBuffers;
            dest.PingTimeout = source.PingTimeout;
            dest.ProtocolType = source.ProtocolType;
            dest.ServerBlocking = source.ServerBlocking;
            dest.ServerToClientPings = source.ServerToClientPings;
            dest.ServiceName = source.ServiceName;
            dest.SharedPoolLock = source.SharedPoolLock;
            dest.SharedPoolSize = source.SharedPoolSize;
            source.TcpOpts.CopyTo(dest.TcpOpts);
            dest.UserSpecObject = source.UserSpecObject;
            source.BindEncryptionOpts.CopyTo(dest.BindEncryptionOpts);
            dest.SysSendBufSize = source.SysSendBufSize;
            dest.SysRecvBufSize = source.SysRecvBufSize;

            return dest;
        }

        internal static AcceptOptions CopyTo(this AcceptOptions source, AcceptOptions dest)
        {
            dest.ChannelReadLocking = source.ChannelReadLocking;
            dest.ChannelWriteLocking = source.ChannelWriteLocking;
            dest.NakMount = source.NakMount;
            dest.SysRecvBufSize = source.SysRecvBufSize;
            dest.SysSendBufSize = source.SysSendBufSize;
            dest.SendTimeout = source.SendTimeout;
            dest.ReceiveTimeout = source.ReceiveTimeout;
            dest.UserSpecObject = source.UserSpecObject;

            return dest;
        }

        internal static EncryptionOptions CopyTo(this EncryptionOptions source, EncryptionOptions dest)
        {
            dest.EncryptionProtocolFlags = source.EncryptionProtocolFlags;
            dest.EncryptedProtocol = source.EncryptedProtocol;
            dest.TlsCipherSuites = source.TlsCipherSuites;
            dest.AuthenticationTimeout = source.AuthenticationTimeout;

            return dest;
        }

        internal static BindEncryptionOptions CopyTo(this BindEncryptionOptions source, BindEncryptionOptions dest)
        {
            dest.EncryptionProtocolFlags = source.EncryptionProtocolFlags;
            dest.ServerCertificate = source.ServerCertificate;
            dest.ServerPrivateKey = source.ServerPrivateKey;
            dest.TlsCipherSuites = source.TlsCipherSuites;
            dest.AuthenticationTimeout = source.AuthenticationTimeout;

            return dest;
        }

        internal static bool IsProxyEnabled(this ConnectOptions connectOptions)
        {
            if( !string.IsNullOrEmpty(connectOptions.ProxyOptions.ProxyHostName) &&
                !string.IsNullOrEmpty(connectOptions.ProxyOptions.ProxyPort))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
