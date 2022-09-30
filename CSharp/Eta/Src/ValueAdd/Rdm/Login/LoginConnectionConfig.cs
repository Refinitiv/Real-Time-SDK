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
using Refinitiv.Eta.Rdm;

using Enum = Refinitiv.Eta.Codec.Enum;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Connection config is representation of login response payload
    /// and contains standby configuration information.
    /// </summary>
    public class LoginConnectionConfig
    {
        private List<ServerInfo> _serverList;

        private const string eol = "\n";
        private const string tab = "\t";
        private StringBuilder stringBuf = new();

        private Vector vector = new();
        private VectorEntry vectorEntry = new();
        private ElementList elementList = new();
        private ElementEntry elementEntry = new();
        private ElementList serverElementList = new();
        private ElementEntry serverElementEntry = new();
        private Enum tmpEnum = new();
        private UInt tmpUInt = new();
        private LocalElementSetDefDb setDb = new();

        /// <summary>
        /// Sets list of <see cref="ServerInfo"/> for standby connection configuration.
        /// </summary>
        public List<ServerInfo> ServerList
        {
            get => _serverList;
            set
            {
                Debug.Assert(value != null);
                _serverList.Clear();
                _serverList.AddRange(value);
            }
        }

        /// <summary>
        /// The number of standby servers.
        /// </summary>
        public long NumStandByServers { get; set; }

        public LoginConnectionConfig()
        {
            _serverList = new List<ServerInfo>();
        }

        /// Performs a deep copy of this object into <c>destConnectionConfig</c>.
        public CodecReturnCode Copy(LoginConnectionConfig destConnectionConfig)
        {
            Debug.Assert(destConnectionConfig != null);

            destConnectionConfig.NumStandByServers = NumStandByServers;

            CodecReturnCode ret;

            foreach (ServerInfo serverInfo in ServerList)
            {
                ServerInfo destServerInfo = new ServerInfo();
                ret = serverInfo.Copy(destServerInfo);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                destConnectionConfig.ServerList.Add(destServerInfo);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>Clears the current contents of the server info object and prepares it for re-use.</summary>
        public void Clear()
        {
            NumStandByServers = 0;
            _serverList.Clear();
        }

        public CodecReturnCode Encode(EncodeIterator EncodeIter)
        {
            // Encode Element ConnectionConfig
            elementList.Clear();
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            CodecReturnCode ret = elementList.EncodeInit(EncodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            elementEntry.Clear();

            elementEntry.DataType = DataTypes.VECTOR;
            elementEntry.Name = ElementNames.CONNECTION_CONFIG;

            ret = elementEntry.EncodeInit(EncodeIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // Encode Server Entries
            vector.Clear();
            vector.ContainerType = DataTypes.ELEMENT_LIST;
            vector.Flags = VectorFlags.HAS_SUMMARY_DATA;

            ret = vector.EncodeInit(EncodeIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // Encode numStandbyServers in summary data.
            elementList.Clear();
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            ret = elementList.EncodeInit(EncodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            elementEntry.Clear();

            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.NUM_STANDBY_SERVERS;
            tmpUInt.Value(NumStandByServers);
            ret = elementEntry.Encode(EncodeIter, tmpUInt);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            ret = elementList.EncodeComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            ret = vector.EncodeSummaryDataComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            foreach (ServerInfo serverInfo in ServerList)
            {
                vectorEntry.Clear();
                vectorEntry.Index = (uint)serverInfo.ServerIndex;
                vectorEntry.Action = VectorEntryActions.SET;
                ret = vectorEntry.EncodeInit(EncodeIter, 0);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                // Encode Element List describing server
                elementList.Clear();
                elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

                ret = elementList.EncodeInit(EncodeIter, null, 0);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                elementEntry.Clear();

                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.HOSTNAME;
                ret = elementEntry.Encode(EncodeIter, serverInfo.HostName);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.PORT;
                tmpUInt.Value(serverInfo.Port);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                if (serverInfo.HasLoadFactor)
                {
                    elementEntry.DataType = DataTypes.UINT;
                    elementEntry.Name = ElementNames.LOAD_FACT;
                    tmpUInt.Value(serverInfo.LoadFactor);
                    ret = elementEntry.Encode(EncodeIter, tmpUInt);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                if (serverInfo.HasType)
                {
                    elementEntry.DataType = DataTypes.ENUM;
                    elementEntry.Name = ElementNames.SERVER_TYPE;
                    tmpEnum.Value(serverInfo.ServerType);
                    ret = elementEntry.Encode(EncodeIter, tmpEnum);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                ret = elementList.EncodeComplete(EncodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                ret = vectorEntry.EncodeComplete(EncodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // Complete
            ret = vector.EncodeComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            ret = elementEntry.EncodeComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return elementList.EncodeComplete(EncodeIter, true);
        }

        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            Clear();
            CodecReturnCode ret = vector.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            if (vector.ContainerType != DataTypes.ELEMENT_LIST)
            {
                return CodecReturnCode.FAILURE;
            }

            if (vector.CheckHasSetDefs())
            {
                setDb.Clear();
                ret = setDb.Decode(dIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // Decode payload
            if (vector.CheckHasSummaryData())
            {
                ret = serverElementList.Decode(dIter, null);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                bool foundNumStandbyServers = false;
                // Decode each element entry in list
                while ((ret = serverElementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    if (ElementNames.NUM_STANDBY_SERVERS.Equals(serverElementEntry.Name))
                    {
                        if (serverElementEntry.DataType != DataTypes.UINT)
                            return CodecReturnCode.FAILURE;
                        ret = tmpUInt.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                        NumStandByServers = tmpUInt.ToLong();
                        foundNumStandbyServers = true;
                    }
                }

                if (!foundNumStandbyServers)
                    return CodecReturnCode.FAILURE;
            } // end summary data Decode

            vectorEntry.Clear();
            bool foundHostName = false;
            bool foundServerPort = false;
            if ((ret = vectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                do
                {
                    ret = serverElementList.Decode(dIter, setDb);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    ServerInfo serverInfo = new ServerInfo();
                    ServerList.Add(serverInfo);
                    serverInfo.ServerIndex = vectorEntry.Index;

                    // Decode each element entry in list
                    while ((ret = serverElementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }

                        if (serverElementEntry.Name.Equals(ElementNames.HOSTNAME))
                        {
                            foundHostName = true;
                            serverInfo.HostName = serverElementEntry.EncodedData;
                        }

                        if (serverElementEntry.Name.Equals(ElementNames.PORT))
                        {
                            ret = tmpUInt.Decode(dIter);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            foundServerPort = true;
                            serverInfo.Port = tmpUInt.ToLong();
                        }

                        if (serverElementEntry.Name.Equals(ElementNames.LOAD_FACT))
                        {
                            ret = tmpUInt.Decode(dIter);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            serverInfo.HasLoadFactor = true;
                            serverInfo.LoadFactor = tmpUInt.ToLong();
                        }

                        if (serverElementEntry.Name.Equals(ElementNames.SERVER_TYPE))
                        {
                            ret = tmpEnum.Decode(dIter);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            serverInfo.HasType = true;
                            serverInfo.ServerType = tmpEnum.ToInt();
                        }

                    }

                    if (!foundServerPort)
                        return CodecReturnCode.FAILURE;

                    if (!foundHostName)
                        return CodecReturnCode.FAILURE;
                }
                while ((ret = vectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER);
            }
            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            stringBuf.Clear();

            stringBuf.Append(tab);
            stringBuf.Append("numStandbyServers: ");
            stringBuf.Append(NumStandByServers);
            stringBuf.Append(eol);

            foreach (ServerInfo serverInfo in ServerList)
            {
                stringBuf.Append(serverInfo.ToString());
            }
            stringBuf.Append(eol);

            return stringBuf.ToString();
        }

        public void CopyReferences(LoginConnectionConfig srcConnectionConfig)
        {
            Debug.Assert(srcConnectionConfig != null);

            NumStandByServers = srcConnectionConfig.NumStandByServers;
            ServerList = srcConnectionConfig.ServerList;
        }
    }
}
