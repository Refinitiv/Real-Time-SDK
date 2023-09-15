/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;

using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Connection config is representation of login response payload
    /// and contains standby configuration information.
    /// </summary>
    sealed public class LoginConnectionConfig
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

        /// <summary>
        /// Login Connection Config constructor.
        /// </summary>
        public LoginConnectionConfig()
        {
            _serverList = new List<ServerInfo>();
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destConnectionConfig</c>.
        /// </summary>
        /// <param name="destConnectionConfig">LoginConnectionConfig that will be copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
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

        /// <summary>
        /// Shallow copies the information and references contained in <c>srcConnectionConfig</c> into this object.
        /// </summary>
        /// <param name="srcConnectionConfig">LoginConnectionConfig that will be copied from.</param>
        public void CopyReferences(LoginConnectionConfig srcConnectionConfig)
        {
            Debug.Assert(srcConnectionConfig != null);

            NumStandByServers = srcConnectionConfig.NumStandByServers;
            ServerList = srcConnectionConfig.ServerList;
        }

        /// <summary>
        /// Clears the current contents of the login connection config info object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            NumStandByServers = 0;
            _serverList.Clear();
        }

        /// <summary>
        /// Encodes the connection config info using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            // Encode Element ConnectionConfig
            elementList.Clear();
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            CodecReturnCode ret = elementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            elementEntry.Clear();

            elementEntry.DataType = DataTypes.VECTOR;
            elementEntry.Name = ElementNames.CONNECTION_CONFIG;

            ret = elementEntry.EncodeInit(encodeIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // Encode Server Entries
            vector.Clear();
            vector.ContainerType = DataTypes.ELEMENT_LIST;
            vector.Flags = VectorFlags.HAS_SUMMARY_DATA;

            ret = vector.EncodeInit(encodeIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // Encode numStandbyServers in summary data.
            elementList.Clear();
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            ret = elementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            elementEntry.Clear();

            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.NUM_STANDBY_SERVERS;
            tmpUInt.Value(NumStandByServers);
            ret = elementEntry.Encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            ret = elementList.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            ret = vector.EncodeSummaryDataComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            foreach (ServerInfo serverInfo in ServerList)
            {
                vectorEntry.Clear();
                vectorEntry.Index = (uint)serverInfo.ServerIndex;
                vectorEntry.Action = VectorEntryActions.SET;
                ret = vectorEntry.EncodeInit(encodeIter, 0);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                // Encode Element List describing server
                elementList.Clear();
                elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

                ret = elementList.EncodeInit(encodeIter, null, 0);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                elementEntry.Clear();

                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.HOSTNAME;
                ret = elementEntry.Encode(encodeIter, serverInfo.HostName);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.PORT;
                tmpUInt.Value(serverInfo.Port);
                ret = elementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                if (serverInfo.HasLoadFactor)
                {
                    elementEntry.DataType = DataTypes.UINT;
                    elementEntry.Name = ElementNames.LOAD_FACT;
                    tmpUInt.Value(serverInfo.LoadFactor);
                    ret = elementEntry.Encode(encodeIter, tmpUInt);
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
                    ret = elementEntry.Encode(encodeIter, tmpEnum);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                ret = elementList.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                ret = vectorEntry.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // Complete
            ret = vector.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            ret = elementEntry.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return elementList.EncodeComplete(encodeIter, true);
        }

        /// <summary>
        /// Decodes the connection config info using the provided <c>dIter</c>.  The decode iterator must be set to decode the vector containing the login connection config info.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message and is ready to decode the Login Connection Config Vector.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter)
        {
            Clear();
            CodecReturnCode ret = vector.Decode(decodeIter);
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
                ret = setDb.Decode(decodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // Decode payload
            if (vector.CheckHasSummaryData())
            {
                ret = serverElementList.Decode(decodeIter, null);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                bool foundNumStandbyServers = false;
                // Decode each element entry in list
                while ((ret = serverElementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    if (ElementNames.NUM_STANDBY_SERVERS.Equals(serverElementEntry.Name))
                    {
                        if (serverElementEntry.DataType != DataTypes.UINT)
                            return CodecReturnCode.FAILURE;
                        ret = tmpUInt.Decode(decodeIter);
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
            if ((ret = vectorEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                do
                {
                    ret = serverElementList.Decode(decodeIter, setDb);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    ServerInfo serverInfo = new ServerInfo();
                    ServerList.Add(serverInfo);
                    serverInfo.ServerIndex = vectorEntry.Index;

                    // Decode each element entry in list
                    while ((ret = serverElementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
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
                            ret = tmpUInt.Decode(decodeIter);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            foundServerPort = true;
                            serverInfo.Port = tmpUInt.ToLong();
                        }

                        if (serverElementEntry.Name.Equals(ElementNames.LOAD_FACT))
                        {
                            ret = tmpUInt.Decode(decodeIter);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            serverInfo.HasLoadFactor = true;
                            serverInfo.LoadFactor = tmpUInt.ToLong();
                        }

                        if (serverElementEntry.Name.Equals(ElementNames.SERVER_TYPE))
                        {
                            ret = tmpEnum.Decode(decodeIter);
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
                while ((ret = vectorEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER);
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string containing the connection configuration info.
        /// </summary>        
        /// <returns>String containing the string representation.</returns>
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
    }
}
