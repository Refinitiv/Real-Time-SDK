/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Represents Dictionary Refresh message.
    /// </summary>
    public class DictionaryRefresh : MsgBase
    {
        private IRefreshMsg m_DictionaryRefresh = new Msg();
        private Int tmpInt = new Int();
        UInt tmpUInt = new UInt();
        private DecodeIterator m_SeriesDecodeIterator = new DecodeIterator();
        private Series m_Series = new Series();

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_ElementEntry = new ElementEntry();
        private long m_SeqNum;

        public DictionaryRefreshFlags Flags { get; set; }

        /// <summary>
        /// Checks the presence of sequence number field.
        /// </summary>
        public bool HasSequenceNumber
        { 
            get => (Flags & DictionaryRefreshFlags.HAS_SEQ_NUM) != 0; 
            set 
            { 
                if (value)
                {
                    Flags |= DictionaryRefreshFlags.HAS_SEQ_NUM;
                } else
                {
                    Flags &= ~DictionaryRefreshFlags.HAS_SEQ_NUM;
                }
            } 
        }
        /// <summary>
        /// Checks the presence of solicited flag.
        /// </summary>
        public bool Solicited
        {
            get => (Flags & DictionaryRefreshFlags.SOLICITED) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryRefreshFlags.SOLICITED;
                }
                else
                {
                    Flags &= ~DictionaryRefreshFlags.SOLICITED;
                }
            }
        }
        /// <summary>
        /// Checks presence of the dictionaryId, version, and type fields.
        /// </summary>
        public bool HasInfo
        {
            get => (Flags & DictionaryRefreshFlags.HAS_INFO) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryRefreshFlags.HAS_INFO;
                }
                else
                {
                    Flags &= ~DictionaryRefreshFlags.HAS_INFO;
                }
            }
        }
        /// <summary>
        /// Checks the presence of refresh complete flag.
        /// </summary>
        public bool RefreshComplete
        {
            get => (Flags & DictionaryRefreshFlags.IS_COMPLETE) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryRefreshFlags.IS_COMPLETE;
                }
                else
                {
                    Flags &= ~DictionaryRefreshFlags.IS_COMPLETE;
                }
            }
        }
        /// <summary>
        /// Checks the presence of clear cache flag.
        /// </summary>
        public bool ClearCache
        {
            get => (Flags & DictionaryRefreshFlags.CLEAR_CACHE) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryRefreshFlags.CLEAR_CACHE;
                }
                else
                {
                    Flags &= ~DictionaryRefreshFlags.CLEAR_CACHE;
                }
            }
        }

        public override int StreamId { get => m_DictionaryRefresh.StreamId; set { m_DictionaryRefresh.StreamId = value; } }
        public override int MsgClass { get => m_DictionaryRefresh.MsgClass; }
        public override int DomainType { get => m_DictionaryRefresh.DomainType; }

        /// <summary>
        /// The ID of the service providing the dictionary.
        /// </summary>
        public int ServiceId { get => m_DictionaryRefresh.MsgKey.ServiceId; set { m_DictionaryRefresh.MsgKey.ServiceId = value; } }

        /// <summary>
        /// Sets the the dictionaryName field for this message to the user specified buffer. 
        /// Buffer used by this object's dictionaryName field will be set 
        /// to passed in buffer's data and position. Note that this creates garbage if buffer is backed by String object.
        /// </summary>
        public Buffer DictionaryName { get => m_DictionaryRefresh.MsgKey.Name; set { m_DictionaryRefresh.MsgKey.Name = value; } }

        /// <summary>
        /// The current state of the stream.
        /// </summary>
        public State State
        {
            get => m_DictionaryRefresh.State;
            set
            {
                Debug.Assert(value != null);
                value.Copy(m_DictionaryRefresh.State);              
            }
        }

        /// <summary>
        /// The verbosity of the dictionary being provided. Populated by <see cref="Dictionary.VerbosityValues"/>
        /// </summary>
        public long Verbosity { get => m_DictionaryRefresh.MsgKey.Filter; set { m_DictionaryRefresh.MsgKey.Filter = value; } }

        /// <summary>
        /// When encoding the message, this points to the dictionary object that is being encoded.
        /// </summary>
        public DataDictionary? DataDictionary { get; set; }

        /// <summary>
        /// The sequence number of this message.
        /// </summary>
        public long SequenceNumber { get => m_SeqNum; set { m_SeqNum = value; } }

        /// <summary>
        /// The type of the dictionary. Populated by <see cref="Dictionary.Types"/>
        /// </summary>
        public int DictionaryType { get; set; }

        /// <summary>
        /// When decoding, points to the payload of the message. The application should set 
        /// the iterator to this buffer and call the appropriate decode method. This will add the data 
        /// present to the <see cref="DataDictionary"/> object. When encoding, this member is not used.
        /// </summary>
        public Buffer DataBody { get; set; } = new Buffer();

        /// <summary>
        /// This field is initialized with dictionary-&gt;minFid and after encoding each part, 
        /// updated with the start Fid for next encoded part. When decoding, this is not used.
        /// </summary>
        public int StartFid { get; set; }

        /// <summary>
        /// This field is initialized with 0 and after encoding each part, 
        /// updated with the start Enum Table Count for next encoded part. When decoding, this is not used.
        /// </summary>
        public int StartEnumTableCount { get; set; }

        /// <summary>
        /// Clears current DictionaryRefresh instance.
        /// </summary>
        public override void Clear()
        {
            Flags = 0;
            m_DictionaryRefresh.Clear();
            m_DictionaryRefresh.MsgClass = MsgClasses.REFRESH;
            m_DictionaryRefresh.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
            m_DictionaryRefresh.ContainerType = DataTypes.SERIES;
            m_DictionaryRefresh.ApplyHasMsgKey();
            m_DictionaryRefresh.MsgKey.ApplyHasName();
            m_DictionaryRefresh.MsgKey.ApplyHasFilter();
            m_DictionaryRefresh.MsgKey.ApplyHasServiceId();
            m_SeqNum = 0;
            ServiceId = 0;
            Verbosity = Dictionary.VerbosityValues.NORMAL;
            DictionaryType = 0;
            DictionaryName.Clear();
            StartFid = -32768; // MIN_FID
            StartEnumTableCount = 0;
            DataBody.Clear();
        }

        public DictionaryRefresh()
        {
            Clear();
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
        {
            if (msg.MsgClass != MsgClasses.REFRESH)
            {
                return CodecReturnCode.FAILURE;
            }

            Clear();
            StreamId = msg.StreamId;
            
            IMsgKey key = msg.MsgKey;          
            if (key == null || !key.CheckHasFilter() || !key.CheckHasName() || !key.CheckHasServiceId())
            {
                return CodecReturnCode.FAILURE;
            }                

            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            DictionaryName = key.Name;
            if (key.CheckHasServiceId())
            {
                ServiceId = key.ServiceId;
            }                
            if (refreshMsg.CheckHasSeqNum())
            {
                HasSequenceNumber = true;
                SequenceNumber = refreshMsg.SeqNum;
            }            
            Verbosity = (int)key.Filter;           
            if (refreshMsg.CheckRefreshComplete())
            {
                RefreshComplete = true;
            }               
            if (refreshMsg.CheckSolicited())
            {
                Solicited = true;
            }               
            if (refreshMsg.CheckClearCache())
            {
                ClearCache = true;
            }
            State = refreshMsg.State;

            // payload
            m_SeriesDecodeIterator.Clear();
            m_Series.Clear();
            if (msg.EncodedDataBody.Length > 0)
            {
                Buffer encodedDataBody = msg.EncodedDataBody;
                DataBody.Data(msg.EncodedDataBody.Data(), msg.EncodedDataBody.Position, msg.EncodedDataBody.Length);
                m_SeriesDecodeIterator.SetBufferAndRWFVersion(encodedDataBody, decIter.MajorVersion(), decIter.MinorVersion());
                CodecReturnCode ret = m_Series.Decode(m_SeriesDecodeIterator);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                if (m_Series.CheckHasSummaryData())
                {
                    HasInfo = true;
                    return DecodeDictionaryInfo();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeDictionaryInfo()
        {
            m_ElementList.Clear();
            CodecReturnCode ret = m_ElementList.Decode(m_SeriesDecodeIterator, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            //If there is no summary data present, don't go looking for info.
            if (!(m_ElementList.CheckHasStandardData() || m_ElementList.CheckHasSetData()))
            {
                return CodecReturnCode.FAILURE;
            }
                
            m_ElementEntry.Clear();
            bool foundVersion = false;
            bool foundType = false;
            while ((ret = m_ElementEntry.Decode(m_SeriesDecodeIterator)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                if (m_ElementEntry.Name.Equals(ElementNames.DICT_VERSION))
                {
                    foundVersion = true;
                }
                if (m_ElementEntry.Name.Equals(ElementNames.DICT_TYPE))
                {
                    ret = tmpUInt.Decode(m_SeriesDecodeIterator);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    DictionaryType = (int)tmpUInt.ToLong();
                    foundType = true;
                }
                if (!(foundVersion || foundType))
                    return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            if (ClearCache)
            {
                m_DictionaryRefresh.ApplyClearCache();
            }
            if (Solicited)
            {
                m_DictionaryRefresh.ApplySolicited();
            }
            if (RefreshComplete)
            {
                m_DictionaryRefresh.ApplyRefreshComplete();
            }
            if (HasSequenceNumber)
            {
                m_DictionaryRefresh.ApplyHasSeqNum();
                m_DictionaryRefresh.SeqNum = m_SeqNum;
            }
          
            CodecReturnCode ret = m_DictionaryRefresh.EncodeInit(encIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            CodecError error;

            //encode dictionary into message
            switch (DictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    {
                        tmpInt.Value(StartFid);
                        CodecReturnCode dictEncodeRet = DataDictionary!.EncodeFieldDictionary(encIter, tmpInt, (int)Verbosity, out error);
                        if (dictEncodeRet != CodecReturnCode.SUCCESS)
                        {
                            if (dictEncodeRet != CodecReturnCode.DICT_PART_ENCODED)
                            {
                                // dictionary encode failed
                                return dictEncodeRet;
                            }
                        }
                        else
                        {
                            // set refresh complete flag
                            ret = encIter.SetRefreshCompleteFlag();
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }

                        //complete encode message
                        ret = m_DictionaryRefresh.EncodeComplete(encIter, true);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }

                        StartFid = (int)tmpInt.ToLong();
                        return dictEncodeRet;
                    }
                case Dictionary.Types.ENUM_TABLES:
                    {
                        tmpInt.Value(StartEnumTableCount);

                        //encode dictionary into message
                        CodecReturnCode dictEncodeRet = DataDictionary!.EncodeEnumTypeDictionaryAsMultiPart(encIter, tmpInt, (int)Verbosity, out error);
                        if (dictEncodeRet != CodecReturnCode.SUCCESS)
                        {
                            if (dictEncodeRet != CodecReturnCode.DICT_PART_ENCODED)
                            {
                                // dictionary encode failed
                                return dictEncodeRet;
                            }
                        }
                        else
                        {
                            // set refresh complete flag
                            ret = encIter.SetRefreshCompleteFlag();
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }

                        //complete encode message
                        ret = m_DictionaryRefresh.EncodeComplete(encIter, true);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                        StartEnumTableCount = (int)tmpInt.ToLong();
                        return dictEncodeRet;
                    }
                default:
                    return CodecReturnCode.FAILURE;
            }
        }

        public CodecReturnCode Copy(DictionaryRefresh destRefreshMsg)
        {
            destRefreshMsg.Clear();
            destRefreshMsg.Flags = Flags;
            destRefreshMsg.StreamId = StreamId;
            destRefreshMsg.ServiceId = ServiceId;
            destRefreshMsg.Verbosity = Verbosity;
            destRefreshMsg.DictionaryType = DictionaryType;
            destRefreshMsg.StartFid = StartFid;
            destRefreshMsg.StartEnumTableCount = StartEnumTableCount;
            BufferHelper.CopyBuffer(DictionaryName, destRefreshMsg.DictionaryName);
            BufferHelper.CopyBuffer(DataBody, destRefreshMsg.DataBody);
            State.Copy(destRefreshMsg.State);
            if (HasSequenceNumber)
            {
                destRefreshMsg.SequenceNumber = SequenceNumber;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuilder = PrepareStringBuilder();
            stringBuilder.Insert(0, "DictionaryRefresh: \n");

            stringBuilder.Append(tab);
            stringBuilder.Append("dictionaryName: ");
            stringBuilder.Append(DictionaryName);
            stringBuilder.Append(eol);

            stringBuilder.Append(tab);
            stringBuilder.Append(State);
            stringBuilder.Append(eol);

            stringBuilder.Append(tab);
            stringBuilder.Append("serviceId: ");
            stringBuilder.Append(ServiceId);
            stringBuilder.Append(eol);

            stringBuilder.Append(tab);
            stringBuilder.Append("isRefreshComplete: " + RefreshComplete);
            stringBuilder.Append(eol);

            stringBuilder.Append(tab);
            stringBuilder.Append("isClearCache: " + ClearCache);
            stringBuilder.Append(eol);

            stringBuilder.Append(tab);
            stringBuilder.Append("isSolicited: " + Solicited);
            stringBuilder.Append(eol);

            stringBuilder.Append(tab);
            stringBuilder.Append("verbosity: ");
            bool addOr;

            stringBuilder.Append("INFO");
            addOr = true;

            if ((Verbosity & Dictionary.VerbosityValues.MINIMAL) != 0)
            {
                if (addOr)
                    stringBuilder.Append(" | ");
                stringBuilder.Append("MINIMAL");
                addOr = true;
            }
            if ((Verbosity & Dictionary.VerbosityValues.NORMAL) != 0)
            {
                if (addOr)
                    stringBuilder.Append(" | ");
                stringBuilder.Append("NORMAL");
                addOr = true;
            }
            if ((Verbosity & Dictionary.VerbosityValues.VERBOSE) != 0)
            {
                if (addOr)
                    stringBuilder.Append(" | ");
                stringBuilder.Append("VERBOSE");
            }

            stringBuilder.Append(eol);
            return stringBuilder.ToString();
        }
    }
}
