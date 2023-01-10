/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.Transports.Internal;
using System.Collections.Generic;

namespace LSEG.Eta.Internal
{
    class IpcProtocolOptions
    {
        public bool ServerForceCompression { get; set; }
        public CompressionType SessionInDecompress { get; set; }
        public CompressionType SessionOutCompression { get; set; }
        public byte SessionCompLevel { get; set; }
        public ushort SessionCompType { get; set; }
        public byte CompressionBitmap { get; set; }
        public bool ForceCompression { get; set; }

        public ProtocolType ProtocolType { get; set; }
        public int MajorVersion { get; set; }
        public int MinorVersion { get; set; }
        public bool NakMount { get; set; }
        public RipcSessionFlags ServerSessionFlags { get; set; }
        public int PingTimeout { get; set; }

        public bool KeyExchange { get; set; }
        public long _P { get; set; }
        public long _G { get; set; }
        public long Send_Key { get; set; }
        public long Random_Key { get; set; }
        public long Shared_Key { get; set; } // this is the key used for encryption/decryption; the other things are needed to calculate this
        public byte EncryptionType { get; set; }

        public int MaxUserMsgSize { get; set; }

        public string ClientHostName { get; set; }
        public string ClientIpAddress { get; set; }

        /* This is the Component Info that we are going to encode on ConnectReq and ConnectAck */
        public ComponentInfo ComponentInfo { get; set; } = new ComponentInfo();

        /* This is the Component Info(s) that we receive when decoding ConnectReq and ConnectAck. */
        public List<ComponentInfo> ReceivedComponentVersionList = new List<ComponentInfo>(1);

        public void Options(ConnectOptions options)
        {
            SessionInDecompress = options.CompressionType;
            SessionOutCompression = 0;
            SessionCompLevel = (byte)0; // value from ConnectAck
            CompressionBitmap = (byte)SessionInDecompress;
            PingTimeout = options.PingTimeout;
            ProtocolType = options.ProtocolType;
            MajorVersion = options.MajorVersion;
            MinorVersion = options.MinorVersion;
            ServerSessionFlags = (byte)0; // value from ConnectAck
            ForceCompression = false;
            KeyExchange = false;
            _P = 0;
            _G = 0;
            Send_Key = 0;
            Random_Key = 0;
            Shared_Key = 0;
            EncryptionType = 0;
        }

        public void Options(BindOptions bindOptions)
        {
            ServerForceCompression = bindOptions.ForceCompression;
            SessionInDecompress = bindOptions.CompressionType;
            SessionOutCompression = 0;
            SessionCompLevel = (byte)bindOptions.CompressionLevel;
            CompressionBitmap = (byte)SessionInDecompress;
            ForceCompression = bindOptions.ForceCompression;
            ProtocolType = bindOptions.ProtocolType;
            MajorVersion = bindOptions.MajorVersion;
            MinorVersion = bindOptions.MinorVersion;
            MaxUserMsgSize = bindOptions.MaxFragmentSize;
            KeyExchange = false;
            _P = 0;
            _G = 0;
            Send_Key = 0;
            Random_Key = 0;
            Shared_Key = 0;
            EncryptionType = 0;

            /* PingTimeout is not set for bindOptions since it will be negotiated when the ConnectReq is received. */

            /* ServerSessionFlags will be the logical OR of the bindOptions server
             * to client pings and client to server pings value, OR'ed with the sessionFlags read from the ConnectReq.
             * The value will be used/sent in the sessionFlags for the ConnectAck. */
            if (bindOptions.ServerToClientPings)
                ServerSessionFlags |= RipcSessionFlags.SERVER_TO_CLIENT_PING;
            if (bindOptions.ClientToServerPings)
                ServerSessionFlags |= RipcSessionFlags.CLIENT_TO_SERVER_PING;
        }
    }
}
