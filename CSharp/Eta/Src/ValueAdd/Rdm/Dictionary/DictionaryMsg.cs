/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System.Text;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    public class DictionaryMsg
    {
        protected const string eol = "\n";
        protected const string tab = "\t";

        private DictionaryClose? m_DictionaryClose = new DictionaryClose();
        private DictionaryStatus? m_DictionaryStatus = new DictionaryStatus();
        private DictionaryRequest? m_DictionaryRequest = new DictionaryRequest();
        private DictionaryRefresh? m_DictionaryRefresh = new DictionaryRefresh();

        private DictionaryMsgType m_DictionaryMsgType;

        public DictionaryMsgType DictionaryMsgType
        {
            get => m_DictionaryMsgType;
            set
            {
                m_DictionaryMsgType = value;
                switch (m_DictionaryMsgType)
                {
                    case DictionaryMsgType.STATUS:
                        if (m_DictionaryStatus is null)
                        {
                            m_DictionaryStatus = new DictionaryStatus();
                        }
                        break;
                    case DictionaryMsgType.REQUEST:
                        if (m_DictionaryRequest is null)
                        {
                            m_DictionaryRequest = new DictionaryRequest();
                        }
                        break;
                    case DictionaryMsgType.CLOSE:
                        if (m_DictionaryClose is null)
                        {
                            m_DictionaryClose = new DictionaryClose();
                        }
                        break;
                    case DictionaryMsgType.REFRESH:
                        if (m_DictionaryRefresh is null)
                        {
                            m_DictionaryRefresh = new DictionaryRefresh();
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        public DictionaryClose? DictionaryClose
        {
            get => DictionaryMsgType == DictionaryMsgType.CLOSE ? m_DictionaryClose : null;
            private set { m_DictionaryClose = value; }
        }

        public DictionaryRequest? DictionaryRequest
        {
            get => DictionaryMsgType == DictionaryMsgType.REQUEST ? m_DictionaryRequest : null;
            private set { m_DictionaryRequest = value; }
        }

        public DictionaryRefresh? DictionaryRefresh
        {
            get => DictionaryMsgType == DictionaryMsgType.REFRESH ? m_DictionaryRefresh : null;
            private set { m_DictionaryRefresh = value; }
        }

        public DictionaryStatus? DictionaryStatus
        {
            get => DictionaryMsgType == DictionaryMsgType.STATUS ? m_DictionaryStatus : null;
            private set { m_DictionaryStatus = value; }
        }

        public void Clear()
        {
            GetMsg()!.Clear();
        }

        public int StreamId
        {
            get => GetMsg()!.StreamId;
            set { GetMsg()!.StreamId = value; }
        }

        public int Flags
        {
            get
            {
                switch (m_DictionaryMsgType)
                {
                    case DictionaryMsgType.REQUEST:
                        return (int)m_DictionaryRequest!.Flags;
                    case DictionaryMsgType.REFRESH:
                        return (int)m_DictionaryRefresh!.Flags;
                    case DictionaryMsgType.STATUS:
                        return (int)m_DictionaryStatus!.Flags;
                    default:
                        return 0;
                }
            }
            set
            {
                switch (m_DictionaryMsgType)
                {
                    case DictionaryMsgType.REQUEST:
                        m_DictionaryRequest!.Flags = (DictionaryRequestFlags)value;
                        break;
                    case DictionaryMsgType.REFRESH:
                        m_DictionaryRefresh!.Flags = (DictionaryRefreshFlags)value;
                        break;
                    case DictionaryMsgType.STATUS:
                        m_DictionaryStatus!.Flags = (DictionaryStatusFlags)value;
                        break;
                    default:
                        break;
                }
            }
        }

        public CodecReturnCode Decode(DecodeIterator decodeIterator, Msg msg)
        {
            return GetMsg()!.Decode(decodeIterator, msg);
        }

        public CodecReturnCode Encode(EncodeIterator encodeIterator)
        {
            return GetMsg()!.Encode(encodeIterator);
        }

        public override string? ToString()
        {
            return GetMsg()!.ToString();
        }

        private MsgBase? GetMsg()
        {
            switch (m_DictionaryMsgType)
            {
                case DictionaryMsgType.REQUEST:
                    return m_DictionaryRequest;
                case DictionaryMsgType.REFRESH:
                    return m_DictionaryRefresh;
                case DictionaryMsgType.STATUS:
                    return m_DictionaryStatus;
                case DictionaryMsgType.CLOSE:
                    return m_DictionaryClose;
                default:
                    return null;
            }
        }
    }
}
