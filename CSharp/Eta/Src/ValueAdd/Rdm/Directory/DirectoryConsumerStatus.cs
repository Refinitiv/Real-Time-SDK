/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System.Diagnostics;
using System.Text;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    public class DirectoryConsumerStatus : MsgBase
    {
        private List<ConsumerStatusService> m_ConsumerServiceStatusList;
        private IGenericMsg m_GenericMsg;
        private Map m_Map;
        private MapEntry m_MapEntry;
        private UInt m_TempUInt;

        public DirectoryConsumerStatus()
        {
            m_ConsumerServiceStatusList = new List<ConsumerStatusService>();
            m_GenericMsg = (IGenericMsg)new Msg();
            m_Map = new Map();
            m_MapEntry = new MapEntry();
            m_TempUInt = new UInt();
            Clear();
        }

        public override int StreamId { get => m_GenericMsg.StreamId; set => m_GenericMsg.StreamId = value; }
        public override int MsgClass { get => m_GenericMsg.MsgClass; }
        public override int DomainType { get => m_GenericMsg.DomainType; }

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

        public override void Clear()
        {
            base.Clear();
            m_ConsumerServiceStatusList.Clear();
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_GenericMsg.ContainerType = DataTypes.MAP;
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
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

            CodecReturnCode codecReturnCode = m_Map.Decode(decIter);
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

            while ((codecReturnCode = m_MapEntry.Decode(decIter, m_TempUInt)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    return codecReturnCode;
                }

                ConsumerStatusService serviceStatus = new ConsumerStatusService();

                if (m_MapEntry.Action != MapEntryActions.DELETE)
                {
                    codecReturnCode = serviceStatus.Decode(decIter, msg);
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

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_GenericMsg.Flags = GenericMsgFlags.HAS_MSG_KEY;
            m_GenericMsg.MsgKey.Flags = MsgKeyFlags.HAS_NAME;
            m_GenericMsg.MsgKey.Name = ElementNames.CONS_CONN_STATUS;

            CodecReturnCode codecReturnCode = m_GenericMsg.EncodeInit(encIter, 0);
            if (codecReturnCode < CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }

            m_Map.Clear();
            m_Map.Flags = MapFlags.NONE;
            m_Map.KeyPrimitiveType = DataTypes.UINT;
            m_Map.ContainerType = DataTypes.ELEMENT_LIST;
            codecReturnCode = m_Map.EncodeInit(encIter, 0, 0);
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
                    codecReturnCode = m_MapEntry.EncodeInit(encIter, m_TempUInt, 0);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }

                    codecReturnCode = serviceStatus.Encode(encIter);

                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }

                    codecReturnCode = m_MapEntry.EncodeComplete(encIter, true);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }
                }
                else
                {
                    codecReturnCode = m_MapEntry.Encode(encIter, m_TempUInt);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }
                }
            }

            codecReturnCode = m_Map.EncodeComplete(encIter, true);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }

            return m_GenericMsg.EncodeComplete(encIter, true);
        }

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


        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DirectoryConsumerStatus: \n");
            stringBuf.Append(tab);
            stringBuf.Append("streamId: ");
            stringBuf.Append(StreamId);
            stringBuf.Append(eol);

            if (ConsumerServiceStatusList != null && ConsumerServiceStatusList.Count != 0)
            {
                stringBuf.Append(tab);
                stringBuf.Append("ConsumerServiceStatusList: ");
                foreach (ConsumerStatusService consStatusService in ConsumerServiceStatusList)
                {
                    stringBuf.Append(consStatusService.BuildStringBuf());
                }
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
