/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    public class DirectoryMsg
    {
        private DirectoryClose? m_DirectoryClose = new DirectoryClose();
        private DirectoryStatus? m_DirectoryStatus = new DirectoryStatus();
        private DirectoryRequest? m_DirectoryRequest = new DirectoryRequest();
        private DirectoryRefresh? m_DirectoryRefresh = new DirectoryRefresh();
        private DirectoryUpdate? m_DirectoryUpdate = new DirectoryUpdate();
        private DirectoryConsumerStatus? m_DirectoryConsumerStatus = new DirectoryConsumerStatus();

        private DirectoryMsgType m_DirectoryMsgType;
        
        public DirectoryMsgType DirectoryMsgType 
        { 
            get => m_DirectoryMsgType; 
            set
            {
                m_DirectoryMsgType = value;
                switch (m_DirectoryMsgType)
                {
                    case DirectoryMsgType.STATUS:
                        if (m_DirectoryStatus is null)
                        {
                            m_DirectoryStatus = new DirectoryStatus();
                        }
                        break;
                    case DirectoryMsgType.REQUEST:
                        if (m_DirectoryRequest is null)
                        {
                            m_DirectoryRequest = new DirectoryRequest();
                        }
                        break;
                    case DirectoryMsgType.UPDATE:
                        if (m_DirectoryUpdate is null)
                        {
                            m_DirectoryUpdate = new DirectoryUpdate();
                        }
                        break;
                    case DirectoryMsgType.CLOSE:
                        if (m_DirectoryClose is null)
                        {
                            m_DirectoryClose = new DirectoryClose();
                        }
                        break;
                    case DirectoryMsgType.REFRESH:
                        if (m_DirectoryRefresh is null)
                        {
                            m_DirectoryRefresh = new DirectoryRefresh();
                        }
                        break;
                    default:
                        break;
                }
            } 
        }

        public DirectoryClose? DirectoryClose 
        { 
            get => DirectoryMsgType == DirectoryMsgType.CLOSE ? m_DirectoryClose : null; 
            private set { m_DirectoryClose = value; } 
        }

        public DirectoryRequest? DirectoryRequest
        {
            get => DirectoryMsgType == DirectoryMsgType.REQUEST ? m_DirectoryRequest : null;
            private set { m_DirectoryRequest = value; }
        }

        public DirectoryRefresh? DirectoryRefresh
        {
            get => DirectoryMsgType == DirectoryMsgType.REFRESH ? m_DirectoryRefresh : null;
            private set { m_DirectoryRefresh = value; }
        }

        public DirectoryStatus? DirectoryStatus
        {
            get => DirectoryMsgType == DirectoryMsgType.STATUS ? m_DirectoryStatus : null;
            private set { m_DirectoryStatus = value; }
        }

        public DirectoryUpdate? DirectoryUpdate
        {
            get => DirectoryMsgType == DirectoryMsgType.UPDATE ? m_DirectoryUpdate : null;
            private set { m_DirectoryUpdate = value; }
        }

        public DirectoryConsumerStatus? DirectoryConsumerStatus
        {
            get => DirectoryMsgType == DirectoryMsgType.CONSUMER_STATUS ? m_DirectoryConsumerStatus : null;
            private set { m_DirectoryConsumerStatus = value; }
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
                switch (m_DirectoryMsgType)
                {
                    case DirectoryMsgType.REQUEST:
                        return (int)m_DirectoryRequest!.Flags;
                    case DirectoryMsgType.REFRESH:
                        return (int)m_DirectoryRefresh!.Flags;
                    case DirectoryMsgType.UPDATE:
                        return (int)m_DirectoryUpdate!.Flags;
                    case DirectoryMsgType.STATUS:
                        return (int)m_DirectoryStatus!.Flags;
                    default:
                        return 0;
                }
            }
            set 
            {
                switch (m_DirectoryMsgType)
                {
                    case DirectoryMsgType.REQUEST:
                        m_DirectoryRequest!.Flags = (DirectoryRequestFlags)value;
                        break;
                    case DirectoryMsgType.REFRESH:
                        m_DirectoryRefresh!.Flags = (DirectoryRefreshFlags)value;
                        break;
                    case DirectoryMsgType.UPDATE:
                        m_DirectoryUpdate!.Flags = (DirectoryUpdateFlags)value;
                        break;
                    case DirectoryMsgType.STATUS:
                        m_DirectoryStatus!.Flags = (DirectoryStatusFlags)value;
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
            switch (m_DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    return m_DirectoryRequest;
                case DirectoryMsgType.REFRESH:
                    return m_DirectoryRefresh;
                case DirectoryMsgType.UPDATE:
                    return m_DirectoryUpdate;
                case DirectoryMsgType.STATUS:
                    return m_DirectoryStatus;
                case DirectoryMsgType.CLOSE:
                    return m_DirectoryClose;
                case DirectoryMsgType.CONSUMER_STATUS:
                    return m_DirectoryConsumerStatus;
                default:
                    return null;
            }
        }
    }
}
