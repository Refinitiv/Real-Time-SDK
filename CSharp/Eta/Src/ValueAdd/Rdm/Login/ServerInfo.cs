/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;

using static Refinitiv.Eta.Rdm.Login;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Information about available servers.
    /// A list of ServerInfo is used by an LoginRefresh to list servers available for connecting
    /// to, and whether to use them as Standby servers.
    /// </summary>
    public class ServerInfo
    {
        private const string eol = "\n";
        private const string tab = "\t";
        private StringBuilder stringBuf = new();

        private Buffer _hostName;

        /// <summary>
        /// Port for the server.
        /// </summary>
        public long Port { get; set; }

        /// <summary>
        /// Server's load factor.
        /// </summary>
        public long LoadFactor { get; set; }

        /// <summary>
        /// Server's type
        /// </summary>
        public int ServerType { get; set; }

        /// <summary>
        /// HostName for the server.
        /// </summary>
        public Buffer HostName
        {
            get => _hostName;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, _hostName);
            }
        }

        /// <summary>
        /// An index for the server.
        /// </summary>
        public long ServerIndex { get; set; }

        /// <summary>
        /// The server info flags. Populated by <see cref="ServerInfoFlags"/>
        /// </summary>
        public ServerInfoFlags Flags { get; set; }

        /// <summary>
        /// Gets / sets HAS_LOAD_FACTOR flag.
        /// </summary>
        public bool HasLoadFactor
        {
            get => (Flags & ServerInfoFlags.HAS_LOAD_FACTOR) != 0;
            set
            {
                if (value)
                    Flags |= ServerInfoFlags.HAS_LOAD_FACTOR;
                else
                    Flags &= ~ServerInfoFlags.HAS_LOAD_FACTOR;
            }
        }

        /// <summary>
        /// Gets / sets HAS_TYPE flag.
        /// </summary>
        public bool HasType
        {
            get => (Flags & ServerInfoFlags.HAS_TYPE) != 0;
            set
            {
                if (value)
                    Flags |= ServerInfoFlags.HAS_TYPE;
                else
                    Flags &= ~ServerInfoFlags.HAS_TYPE;
            }
        }

        /// <summary>
        /// Instantiates a new server info.
        /// </summary>
        public ServerInfo()
        {
            Flags = default;
            _hostName = new Buffer();
            LoadFactor = 65535;
            ServerType = ServerTypes.STANDBY;
            ServerIndex = 0;
        }

        public CodecReturnCode Copy(ServerInfo destServerInfo)
        {
            Debug.Assert(destServerInfo != null);
            destServerInfo.Flags = Flags;
            BufferHelper.CopyBuffer(HostName, destServerInfo.HostName);
            destServerInfo.Port = Port;
            destServerInfo.ServerIndex = ServerIndex;

            if (HasLoadFactor)
                destServerInfo.LoadFactor = LoadFactor;

            if (HasType)
                destServerInfo.ServerType = ServerType;

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            stringBuf.Clear();
            stringBuf.Append(tab);
            stringBuf.Append("Server:");
            stringBuf.Append(eol);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("hostName: ");
            stringBuf.Append(HostName.ToString());
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("port: ");
            stringBuf.Append(Port);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("serverIndex: ");
            stringBuf.Append(ServerIndex);
            stringBuf.Append(eol);

            if (HasLoadFactor)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("loadFactor: ");
                stringBuf.Append(LoadFactor);
                stringBuf.Append(eol);
            }
            if (HasType)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("serverType: ");
                stringBuf.Append(ServerType);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }

        CodecReturnCode CopyReferences(ServerInfo srcServerInfo)
        {
            Debug.Assert(srcServerInfo != null);

            HostName = srcServerInfo.HostName;
            Flags = srcServerInfo.Flags;
            if (srcServerInfo.HasLoadFactor)
            {
                HasLoadFactor = true;
                LoadFactor = srcServerInfo.LoadFactor;
            }

            if (srcServerInfo.HasType)
            {
                HasType = true;
                ServerType = srcServerInfo.ServerType;
            }

            Port = srcServerInfo.Port;
            ServerIndex = srcServerInfo.ServerIndex;

            return CodecReturnCode.SUCCESS;
        }

        public void Clear()
        {
            HostName.Clear();
            Port = 0;
            LoadFactor = 65535;
            ServerType = ServerTypes.STANDBY;
            ServerIndex = 0;
            Flags = default;
        }
    }
}
