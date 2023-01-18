/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using System.Diagnostics;

namespace LSEG.Eta.ValueAdd.Rdm
{

    /// <summary>The RDM Login Base Message.</summary>
    ///
    /// <remarks>This RDM Login messages may be reused or pooled in a single collection via
    /// their common <c>LoginMsg</c> base class and re-used as a different
    /// <see cref="LoginMsgType"/>.</remarks>
    ///
    /// <seealso cref="LoginClose"/>
    /// <seealso cref="LoginRefresh"/>
    /// <seealso cref="LoginRequest"/>
    /// <seealso cref="LoginStatus"/>
    /// <seealso cref="LoginConsumerConnectionStatus"/>
    sealed public class LoginMsg
    {
        private LoginClose? m_LoginClose;
        private LoginRefresh? m_LoginRefresh;
        private LoginRequest? m_LoginRequest;
        private LoginStatus? m_LoginStatus;
        private LoginConsumerConnectionStatus? m_LoginConsumerConnectionStatus;
        private LoginRTT? m_LoginRTT;

        private LoginMsgType m_LoginMsgType = LoginMsgType.UNKNOWN;

        /// <summary>
        /// Login message type. These are defined per-message class basis for login messages.
        /// </summary>
        public LoginMsgType LoginMsgType
        {
            get => m_LoginMsgType;
            set
            {
                m_LoginMsgType = value;
                switch (value)
                {
                    case LoginMsgType.CLOSE:
                        if (m_LoginClose == null)
                            m_LoginClose = new LoginClose();
                        break;
                    case LoginMsgType.REFRESH:
                        if (m_LoginRefresh == null)
                            m_LoginRefresh = new LoginRefresh();
                        break;
                    case LoginMsgType.REQUEST:
                        if (m_LoginRequest == null)
                            m_LoginRequest = new LoginRequest();
                        break;
                    case LoginMsgType.STATUS:
                        if (m_LoginStatus == null)
                            m_LoginStatus = new LoginStatus();
                        break;
                    case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                        if (m_LoginConsumerConnectionStatus == null)
                            m_LoginConsumerConnectionStatus = new LoginConsumerConnectionStatus();
                        break;
                    case LoginMsgType.RTT:
                        if (m_LoginRTT == null)
                            m_LoginRTT = new LoginRTT();
                        break;
                    default:
                        break;
                }
            }
        }

        /// <summary>
		/// Returns a <see cref="LoginClose"/> RDM message if this LoginMsg is set to  <see cref="LoginMsgType.CLOSE"/>, null otherwise.
		/// </summary>
		/// <returns>The LoginClose RDM Message </returns>
        public LoginClose? LoginClose
        {
            get => (LoginMsgType == LoginMsgType.CLOSE) ? m_LoginClose : null;
        }

        /// <summary>
		/// Returns a <see cref="LoginRefresh"/>  RDM message if this LoginMsg is set to  <see cref="LoginMsgType.REFRESH"/>, null otherwise.
		/// </summary>
		/// <returns>The LoginRefresh RDM Message.</returns>
        public LoginRefresh? LoginRefresh
        {
            get => (LoginMsgType == LoginMsgType.REFRESH) ? m_LoginRefresh : null;
        }

        /// <summary>
		/// Returns a <see cref="LoginRequest"/>  RDM message  if this LoginMsg is set to  <see cref="LoginMsgType.REQUEST"/>, null otherwise.
		/// </summary>
		/// <returns>The LoginRequest RDM Message.</returns>
        public LoginRequest? LoginRequest
        {
            get => (LoginMsgType == LoginMsgType.REQUEST) ? m_LoginRequest : null;
        }

        /// <summary>
		/// Returns a <see cref="LoginStatus"/>  RDM message if this LoginMsg is set to  <see cref="LoginMsgType.STATUS"/>, null otherwise.
		/// </summary>
		/// <returns>The LoginStatus RDM Message.</returns>
        public LoginStatus? LoginStatus
        {
            get => (LoginMsgType == LoginMsgType.STATUS) ? m_LoginStatus : null;
        }

        /// <summary>
        /// Returns a <see cref="LoginConsumerConnectionStatus"/>  RDM message if this LoginMsg is set to  <see cref="LoginMsgType.CONSUMER_CONNECTION_STATUS"/>, null otherwise.
        /// </summary>
        /// <returns>The LoginConsumerConnectionStatus RDM Message.</returns>
        public LoginConsumerConnectionStatus? LoginConsumerConnectionStatus
        {
            get => (LoginMsgType == LoginMsgType.CONSUMER_CONNECTION_STATUS)
                ? m_LoginConsumerConnectionStatus
                : null;
        }


        /// <summary>
        /// Returns a <see cref="LoginRTT"/>  RDM message if this LoginMsg is set to <see cref="LoginMsgType.RTT"/>, null otherwise.
        /// </summary>
        /// <returns>The LoginRTT RDM Message.</returns>
        public LoginRTT? LoginRTT
        {
            get => (LoginMsgType == LoginMsgType.RTT)
                ? m_LoginRTT
                : null;
        }

        /// <summary>
        /// Returns the Login RDM message that this LoginMsg is set to with <see cref="LoginMsgType"/> .
        /// </summary>
        /// <returns>The RDM Message.</returns>
        private MsgBase? GetMsg()
        {
            switch (m_LoginMsgType)
            {
                case LoginMsgType.CLOSE:
                    return m_LoginClose;

                case LoginMsgType.REFRESH:
                    return m_LoginRefresh;

                case LoginMsgType.REQUEST:
                    return m_LoginRequest;

                case LoginMsgType.STATUS:
                    return m_LoginStatus;

                case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                    return m_LoginConsumerConnectionStatus;

                case LoginMsgType.RTT:
                    return m_LoginRTT;

                default:
                    return null;
            }
        }

        /// <summary>
        /// Returns a human readable string representation of the Login message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            return GetMsg()!.ToString() ?? String.Empty;
        }

        /// <summary>
        /// Clears the current contents of the Directory Message object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            GetMsg()!.Clear();
        }

        /// <summary>
        /// Encodes this Login message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return GetMsg()!.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Login RDM message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// LoginMsgType needs to be set prior to calling Decode.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginClose message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            switch (LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    return LoginRequest!.Decode(decodeIter, msg);
                case LoginMsgType.REFRESH:
                    return LoginRefresh!.Decode(decodeIter, msg);
                case LoginMsgType.STATUS:
                    return LoginStatus!.Decode(decodeIter, msg);
                case LoginMsgType.CLOSE:
                    return LoginClose!.Decode(decodeIter, msg);
                case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                    return LoginStatus!.Decode(decodeIter, msg);
                case LoginMsgType.RTT:
                    return LoginRTT!.Decode(decodeIter, msg);
                default:
                    Debug.Assert (false); // not supported on this message class
                    return CodecReturnCode.FAILURE;
            }
        }
    }
}
