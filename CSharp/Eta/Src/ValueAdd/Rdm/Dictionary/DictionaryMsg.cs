/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Text;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>The RDM Dictionary Base Message.</summary>
    ///
    /// <remarks>This RDM Dictionary messages may be reused or pooled in a single collection via
    /// their common <c>DictionaryMsg</c> base class and re-used as a different
    /// <see cref="DictionaryMsgType"/>.</remarks>
    ///
    /// <seealso cref="DictionaryClose"/>
    /// <seealso cref="DictionaryRefresh"/>
    /// <seealso cref="DictionaryRequest"/>
    /// <seealso cref="DictionaryStatus"/>
    sealed public class DictionaryMsg
    {
        private DictionaryClose? m_DictionaryClose = new DictionaryClose();
        private DictionaryStatus? m_DictionaryStatus = new DictionaryStatus();
        private DictionaryRequest? m_DictionaryRequest = new DictionaryRequest();
        private DictionaryRefresh? m_DictionaryRefresh = new DictionaryRefresh();

        private DictionaryMsgType m_DictionaryMsgType;

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
        /// <seealso cref="DictionaryRefreshFlags"/>
        /// <seealso cref="DictionaryRequestFlags"/>
        /// <seealso cref="DictionaryStatusFlags"/>
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

        /// <summary>
        /// The type of this Dictionary Message.  See <see cref="DictionaryMsgType"/>
        /// </summary>
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

        /// <summary>
		/// Returns a <see cref="DictionaryClose"/> RDM message if this LoginMsg is set to  <see cref="DictionaryMsgType.CLOSE"/>, null otherwise.
		/// </summary>
		/// <returns>The DictionaryClose RDM Message </returns>
        public DictionaryClose? DictionaryClose
        {
            get => DictionaryMsgType == DictionaryMsgType.CLOSE ? m_DictionaryClose : null;
            private set { m_DictionaryClose = value; }
        }

        /// <summary>
		/// Returns a <see cref="DictionaryRequest"/> RDM message if this LoginMsg is set to  <see cref="DictionaryMsgType.REQUEST"/>, null otherwise.
		/// </summary>
		/// <returns>The DictionaryRequest RDM Message </returns>
        public DictionaryRequest? DictionaryRequest
        {
            get => DictionaryMsgType == DictionaryMsgType.REQUEST ? m_DictionaryRequest : null;
            private set { m_DictionaryRequest = value; }
        }

        /// <summary>
        /// Returns a <see cref="DictionaryRefresh"/> RDM message if this LoginMsg is set to  <see cref="DictionaryMsgType.REFRESH"/>, null otherwise.
        /// </summary>
        /// <returns>The DictionaryRefresh RDM Message </returns>
        public DictionaryRefresh? DictionaryRefresh
        {
            get => DictionaryMsgType == DictionaryMsgType.REFRESH ? m_DictionaryRefresh : null;
            private set { m_DictionaryRefresh = value; }
        }

        /// <summary>
        /// Returns a <see cref="DictionaryStatus"/> RDM message if this LoginMsg is set to  <see cref="DictionaryMsgType.STATUS"/>, null otherwise.
        /// </summary>
        /// <returns>The DictionaryStatus RDM Message </returns>
        public DictionaryStatus? DictionaryStatus
        {
            get => DictionaryMsgType == DictionaryMsgType.STATUS ? m_DictionaryStatus : null;
            private set { m_DictionaryStatus = value; }
        }

        /// <summary>
        /// Clears the current contents of the Dictionary Message object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            GetMsg()!.Clear();
        }

        /// <summary>
        /// Encodes this Dictionary message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return GetMsg()!.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Directory message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// LoginMsgType needs to be set prior to calling Decode.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginClose message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            return GetMsg()!.Decode(decodeIter, msg);
        }

        /// <summary>
        /// Returns a human readable string representation of the Dictionary message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
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
