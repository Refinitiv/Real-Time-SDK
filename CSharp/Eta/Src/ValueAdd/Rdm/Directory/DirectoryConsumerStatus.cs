/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Net.NetworkInformation;
using System.Text;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The Directory Consumer Status is sent by OMM Consumer applications to inform a service of how it is 
    /// being used for Source Mirroring.This message is primarily informational.
    /// </summary>
    sealed public class DirectoryConsumerStatus : MsgBase
    {
        private List<ConsumerStatusService> m_ConsumerServiceStatusList;
        private IGenericMsg m_GenericMsg;
        private Map m_Map;
        private MapEntry m_MapEntry;
        private UInt m_TempUInt;

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_GenericMsg.StreamId; set => m_GenericMsg.StreamId = value; }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.SOURCE"/>.
        /// </summary>
        public override int DomainType { get => m_GenericMsg.DomainType; }
        
        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.GENERIC"/>
        /// </summary>
        public override int MsgClass { get => m_GenericMsg.MsgClass; }

        /// <summary>
        /// List of <see cref="ConsumerStatusService"/> objects.
        /// </summary>
        public List<ConsumerStatusService> ConsumerServiceStatusList 
        { 
            get { return m_ConsumerServiceStatusList; } 
            set 
            {
                Debug.Assert(value != null);
                m_ConsumerServiceStatusList.Clear();
                m_ConsumerServiceStatusList.AddRange(value);
            } 
        }

        /// <summary>
        /// Directory Consumer Status Message constructor.
        /// </summary>
        public DirectoryConsumerStatus()
        {
            m_ConsumerServiceStatusList = new List<ConsumerStatusService>();
            m_GenericMsg = (IGenericMsg)new Msg();
            m_Map = new Map();
            m_MapEntry = new MapEntry();
            m_TempUInt = new UInt();
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Directory Service Status object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            base.Clear();
            m_ConsumerServiceStatusList.Clear();
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_GenericMsg.ContainerType = DataTypes.MAP;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destConsumerStatus</c>.
        /// </summary>
        /// <param name="destConsumerStatus">DirectoryConsumerStatus object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(DirectoryConsumerStatus destConsumerStatus)
        {
            Debug.Assert(destConsumerStatus != null);
            destConsumerStatus.Clear();
            destConsumerStatus.StreamId = StreamId;
            destConsumerStatus.ConsumerServiceStatusList.Clear();

            if (ConsumerServiceStatusList != null && ConsumerServiceStatusList.Count != 0)
            {
                CodecReturnCode ret = CodecReturnCode.SUCCESS;
                foreach (ConsumerStatusService consStatusService in ConsumerServiceStatusList)
                {
                    ConsumerStatusService destConsStatusService = new ConsumerStatusService();
                    ret = consStatusService.Copy(destConsStatusService);
                    if (ret < CodecReturnCode.SUCCESS)
                        return ret;
                    destConsumerStatus.ConsumerServiceStatusList.Add(destConsStatusService);
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Directory Consumer Status message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_GenericMsg.Flags = GenericMsgFlags.HAS_MSG_KEY;
            m_GenericMsg.MsgKey.Flags = MsgKeyFlags.HAS_NAME;
            m_GenericMsg.MsgKey.Name = ElementNames.CONS_CONN_STATUS;

            CodecReturnCode codecReturnCode = m_GenericMsg.EncodeInit(encodeIter, 0);
            if (codecReturnCode < CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }

            m_Map.Clear();
            m_Map.Flags = MapFlags.NONE;
            m_Map.KeyPrimitiveType = DataTypes.UINT;
            m_Map.ContainerType = DataTypes.ELEMENT_LIST;
            codecReturnCode = m_Map.EncodeInit(encodeIter, 0, 0);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }

            foreach (ConsumerStatusService serviceStatus in m_ConsumerServiceStatusList)
            {
                m_MapEntry.Clear();
                m_MapEntry.Flags = MapEntryFlags.NONE;
                m_MapEntry.Action = serviceStatus.Action;
                m_TempUInt.Value(serviceStatus.ServiceId);
                if (m_MapEntry.Action != MapEntryActions.DELETE)
                {
                    codecReturnCode = m_MapEntry.EncodeInit(encodeIter, m_TempUInt, 0);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }

                    codecReturnCode = serviceStatus.Encode(encodeIter);

                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }

                    codecReturnCode = m_MapEntry.EncodeComplete(encodeIter, true);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }
                }
                else
                {
                    codecReturnCode = m_MapEntry.Encode(encodeIter, m_TempUInt);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }
                }
            }

            codecReturnCode = m_Map.EncodeComplete(encodeIter, true);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }

            return m_GenericMsg.EncodeComplete(encodeIter, true);
        }

        /// <summary>
        /// Decodes this Directory Consumer Status message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this DirectoryConsumerStatus message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            if (msg.MsgClass != MsgClasses.GENERIC || msg.DomainType != (int)Eta.Rdm.DomainType.SOURCE)
            {
                return CodecReturnCode.FAILURE;
            }

            IMsgKey key = msg.MsgKey;

            if (key == null || !key.CheckHasName())
            {
                return CodecReturnCode.FAILURE;
            }

            if (!key.Name.Equals(ElementNames.CONS_CONN_STATUS))
            {
                // Unknown generic msg name
                return CodecReturnCode.FAILURE;
            }

            if (msg.ContainerType != DataTypes.MAP)
                return CodecReturnCode.FAILURE;

            Clear();
            StreamId = msg.StreamId;

            m_Map.Clear();

            CodecReturnCode codecReturnCode = m_Map.Decode(decodeIter);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }

            if (m_Map.KeyPrimitiveType != DataTypes.UINT)
            {
                return CodecReturnCode.FAILURE;
            }

            if (m_Map.ContainerType != DataTypes.ELEMENT_LIST)
            {
                return CodecReturnCode.FAILURE;
            }

            m_MapEntry.Clear();
            m_TempUInt.Clear();

            while ((codecReturnCode = m_MapEntry.Decode(decodeIter, m_TempUInt)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    return codecReturnCode;
                }

                ConsumerStatusService serviceStatus = new ConsumerStatusService();

                if (m_MapEntry.Action != MapEntryActions.DELETE)
                {
                    codecReturnCode = serviceStatus.Decode(decodeIter, msg);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }
                }

                serviceStatus.Action = m_MapEntry.Action;
                serviceStatus.ServiceId = m_TempUInt.ToLong();

                m_ConsumerServiceStatusList.Add(serviceStatus);
            }

            return CodecReturnCode.SUCCESS;
        }


        /// <summary>
        /// Returns a human readable string representation of the Directory Consumer Status.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, $"DirectoryConsumerStatus: {NewLine}");
            stringBuf.Append(tab);
            stringBuf.Append("streamId: ");
            stringBuf.Append(StreamId);
            stringBuf.AppendLine();

            if (ConsumerServiceStatusList != null && ConsumerServiceStatusList.Count != 0)
            {
                stringBuf.Append(tab);
                stringBuf.Append("ConsumerServiceStatusList: ");
                foreach (ConsumerStatusService consStatusService in ConsumerServiceStatusList)
                {
                    stringBuf.Append(consStatusService.BuildStringBuf());
                }
                stringBuf.AppendLine();
            }

            return stringBuf.ToString();
        }
    }
}
