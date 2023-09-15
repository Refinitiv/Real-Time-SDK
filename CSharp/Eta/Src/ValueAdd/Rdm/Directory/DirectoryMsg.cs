/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>The RDM Source Directory Base Message.</summary>
    ///
    /// <remarks>This RDM Source Directory messages may be reused or pooled in a single collection via
    /// their common <c>DirectoryMsg</c> base class and re-used as a different
    /// <see cref="DirectoryMsgType"/>.</remarks>
    ///
    /// <seealso cref="DirectoryClose"/>
    /// <seealso cref="DirectoryConsumerStatus"/>
    /// <seealso cref="DirectoryRefresh"/>
    /// <seealso cref="DirectoryRequest"/>
    /// <seealso cref="DirectoryStatus"/>
    /// <seealso cref="DirectoryUpdate"/>

    sealed public class DirectoryMsg : IRdmMsg
    {
        private DirectoryClose? m_DirectoryClose = new DirectoryClose();
        private DirectoryStatus? m_DirectoryStatus = new DirectoryStatus();
        private DirectoryRequest? m_DirectoryRequest = new DirectoryRequest();
        private DirectoryRefresh? m_DirectoryRefresh = new DirectoryRefresh();
        private DirectoryUpdate? m_DirectoryUpdate = new DirectoryUpdate();
        private DirectoryConsumerStatus? m_DirectoryConsumerStatus = new DirectoryConsumerStatus();

        private DirectoryMsgType m_DirectoryMsgType;

        /// <summary>
        /// The domain type of the message
        /// </summary>
        public Eta.Rdm.DomainType DomainType { get => Eta.Rdm.DomainType.SOURCE; }

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public int StreamId
        {
            get => GetMsg()!.StreamId;
            set { GetMsg()!.StreamId = value; }
        }

        /// <summary>
        /// Flags for this message.
        /// </summary>
        /// <seealso cref="DirectoryRefreshFlags"/>
        /// <seealso cref="DirectoryRequestFlags"/>
        /// <seealso cref="DirectoryStatusFlags"/>
        /// <seealso cref="DirectoryUpdateFlags"/>
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

        /// <summary>
        /// The type of this Directory Message.  See <see cref="DirectoryMsgType"/>
        /// </summary>
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

        /// <summary>
		/// Returns a <see cref="DirectoryClose"/> RDM message if this LoginMsg is set to  <see cref="DirectoryMsgType.CLOSE"/>, null otherwise.
		/// </summary>
		/// <returns>The DirectoryClose RDM Message </returns>
        public DirectoryClose? DirectoryClose 
        { 
            get => DirectoryMsgType == DirectoryMsgType.CLOSE ? m_DirectoryClose : null; 
            private set { m_DirectoryClose = value; } 
        }

        /// <summary>
		/// Returns a <see cref="DirectoryRequest"/> RDM message if this LoginMsg is set to  <see cref="DirectoryMsgType.REQUEST"/>, null otherwise.
		/// </summary>
		/// <returns>The DirectoryRequest RDM Message </returns>
        public DirectoryRequest? DirectoryRequest
        {
            get => DirectoryMsgType == DirectoryMsgType.REQUEST ? m_DirectoryRequest : null;
            private set { m_DirectoryRequest = value; }
        }

        /// <summary>
		/// Returns a <see cref="DirectoryRefresh"/> RDM message if this LoginMsg is set to  <see cref="DirectoryMsgType.REFRESH"/>, null otherwise.
		/// </summary>
		/// <returns>The DirectoryRefresh RDM Message </returns>
        public DirectoryRefresh? DirectoryRefresh
        {
            get => DirectoryMsgType == DirectoryMsgType.REFRESH ? m_DirectoryRefresh : null;
            private set { m_DirectoryRefresh = value; }
        }

        /// <summary>
		/// Returns a <see cref="DirectoryStatus"/> RDM message if this LoginMsg is set to  <see cref="DirectoryMsgType.STATUS"/>, null otherwise.
		/// </summary>
		/// <returns>The DirectoryStatus RDM Message </returns>
        public DirectoryStatus? DirectoryStatus
        {
            get => DirectoryMsgType == DirectoryMsgType.STATUS ? m_DirectoryStatus : null;
            private set { m_DirectoryStatus = value; }
        }

        /// <summary>
		/// Returns a <see cref="DirectoryUpdate"/> RDM message if this LoginMsg is set to  <see cref="DirectoryMsgType.UPDATE"/>, null otherwise.
		/// </summary>
		/// <returns>The DirectoryUpdate RDM Message </returns>
        public DirectoryUpdate? DirectoryUpdate
        {
            get => DirectoryMsgType == DirectoryMsgType.UPDATE ? m_DirectoryUpdate : null;
            private set { m_DirectoryUpdate = value; }
        }

        /// <summary>
		/// Returns a <see cref="DirectoryConsumerStatus"/> RDM message if this LoginMsg is set to  <see cref="DirectoryMsgType.CONSUMER_STATUS"/>, null otherwise.
		/// </summary>
		/// <returns>The DirectoryConsumerStatus RDM Message </returns>
        public DirectoryConsumerStatus? DirectoryConsumerStatus
        {
            get => DirectoryMsgType == DirectoryMsgType.CONSUMER_STATUS ? m_DirectoryConsumerStatus : null;
            private set { m_DirectoryConsumerStatus = value; }
        }

        /// <summary>
        /// Clears the current contents of the Directory Message object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            GetMsg()?.Clear();
        }

        /// <summary>
        /// Encodes this Directory message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return GetMsg()!.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Directory message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// DirectoryMsgType needs to be set prior to calling Decode.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this Directory message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            return GetMsg()!.Decode(decodeIter, msg);
        }

        /// <summary>
        /// Decodes this Directory message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// DirectoryMsgType needs to be set prior to calling Decode.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded IMsg instance for this Directory message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, IMsg msg)
        {
            return GetMsg()!.Decode(decodeIter, (Msg)msg);
        }


        /// <summary>
        /// Returns a human readable string representation of the Directory message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string? ToString()
        {
            return GetMsg()?.ToString() ?? String.Empty;
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
