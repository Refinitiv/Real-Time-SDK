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

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Login Status. Used by an OMM Provider to indicate changes to the Login stream.
    /// </summary>
    sealed public class LoginStatus : MsgBase
    {
        #region Private Fields

        private State state = new();
        private Buffer userName = new();
        private Buffer authenticationErrorText = new();
        private string blankStringConst = string.Empty;

        private ElementList elementList = new();
        private ElementEntry element = new();
        private UInt tmpUInt = new();

        private IStatusMsg m_StatusMsg = (IStatusMsg)new Msg();

        #endregion
        #region Public Message Properties

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get; set; }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.LOGIN"/>.
        /// </summary>
        public override int DomainType { get => m_StatusMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.STATUS"/>
        /// </summary>
        public override int MsgClass { get => m_StatusMsg.MsgClass; }

        /// <summary>
        /// Flags for this message.  See <see cref="LoginStatusFlags"/>.
        /// </summary>
        public LoginStatusFlags Flags { get; set; }

        /// <summary>
        /// Checks the presence of the State field.
        /// </summary>
        public bool HasState
        {
            get => (Flags & LoginStatusFlags.HAS_STATE) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginStatusFlags.HAS_STATE;
                }
                else
                    Flags &= ~LoginStatusFlags.HAS_STATE;
            }
        }
        /// <summary>
        /// Checks the presence of user name field.
        /// </summary>
        public bool HasUserName
        {
            get => (Flags & LoginStatusFlags.HAS_USERNAME) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginStatusFlags.HAS_USERNAME;
                }
                else
                    Flags &= ~LoginStatusFlags.HAS_USERNAME;
            }
        }
        /// <summary>
        /// Checks the presence of UserName type field.
        /// </summary>
        public bool HasUserNameType
        {
            get => (Flags & LoginStatusFlags.HAS_USERNAME_TYPE) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginStatusFlags.HAS_USERNAME_TYPE;
                }
                else
                    Flags &= ~LoginStatusFlags.HAS_USERNAME_TYPE;
            }
        }
        /// <summary>
        /// Checks the presence of ClearCache flag.
        /// </summary>
        public bool ClearCache
        {
            get => (Flags & LoginStatusFlags.CLEAR_CACHE) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginStatusFlags.CLEAR_CACHE;
                }
                else
                    Flags &= ~LoginStatusFlags.CLEAR_CACHE;
            }
        }
        /// <summary>
        /// Checks the presence of AuthenticationErrorCode flag.
        /// </summary>
        public bool HasAuthenticationErrorCode
        {
            get => (Flags & LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE;
                }
                else
                    Flags &= ~LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE;
            }
        }
        /// <summary>
        /// Checks the presence of AuthenticationErrorText flag.
        /// </summary>
        public bool HasAuthenticationErrorText
        {
            get => (Flags & LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT;
                }
                else
                    Flags &= ~LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT;
            }
        }

        /// <summary>
        /// Gets / sets the authentication error text.
        /// </summary>
        public Buffer AuthenticationErrorText
        {
            get => authenticationErrorText;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, authenticationErrorText);
            }
        }

        /// <summary>
        /// The current state of the login stream.
        /// </summary>
        public State State
        {
            get => state;
            set
            {
                Debug.Assert(value != null);
                value.Copy(state);
            }
        }

        /// <summary>
        /// The type of the userName that was used with the Login Status. Populated by <see cref="Login.UserIdTypes"/>
        /// </summary>
        public Login.UserIdTypes UserNameType { get; set; }
        /// <summary>
        /// Gets / sets authentication error code.
        /// </summary>
        public long AuthenticationErrorCode { get; set; }

        /// <summary>
        /// The userName that was used when sending the Login Status.
        /// </summary>
        public Buffer UserName
        {
            get => userName;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, userName);
            }
        }

        #endregion

        /// <summary>
        /// Login Status Message constructor.
        /// </summary>
        public LoginStatus()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the login Status object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.LOGIN;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            Flags = default;
            StreamId = 1;
            State.Clear();
            UserName.Clear();
            authenticationErrorText.Clear();
            AuthenticationErrorCode = 0;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destStatusMsg</c>.
        /// </summary>
        /// <param name="destStatusMsg">LoginStatus object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(LoginStatus destStatusMsg)
        {
            Debug.Assert(destStatusMsg != null);
            destStatusMsg.StreamId = StreamId;
            if (HasUserName)
            {
                destStatusMsg.HasUserName = true;
                BufferHelper.CopyBuffer(UserName, destStatusMsg.UserName);
            }
            if (HasUserNameType)
            {
                destStatusMsg.HasUserNameType = true;
                destStatusMsg.UserNameType = UserNameType;
            }
            if (HasState)
            {
                destStatusMsg.HasState = true;
                destStatusMsg.State = State;
            }

            if (ClearCache)
            {
                destStatusMsg.ClearCache = true;
            }

            if (HasAuthenticationErrorCode)
            {
                destStatusMsg.HasAuthenticationErrorCode = true;
                destStatusMsg.AuthenticationErrorCode = AuthenticationErrorCode;
            }
            if (HasAuthenticationErrorText)
            {
                destStatusMsg.HasAuthenticationErrorText = true;
                BufferHelper.CopyBuffer(AuthenticationErrorText, destStatusMsg.AuthenticationErrorText);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this login status message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_StatusMsg.Clear();
            m_StatusMsg.StreamId = StreamId;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.LOGIN;

            if (HasUserName)
            {
                m_StatusMsg.ApplyHasMsgKey();
                m_StatusMsg.MsgKey.ApplyHasName();
                m_StatusMsg.MsgKey.Name = UserName;
                if (HasUserNameType)
                {
                    m_StatusMsg.MsgKey.ApplyHasNameType();
                    m_StatusMsg.MsgKey.NameType = (int)UserNameType;
                    if (UserNameType == Login.UserIdTypes.TOKEN)
                    {
                        m_StatusMsg.MsgKey.ApplyHasName();
                        m_StatusMsg.MsgKey.Name.Data(blankStringConst);
                    }
                }
            }

            if (ClearCache)
            {
                m_StatusMsg.ApplyClearCache();
            }
            if (HasState)
            {
                m_StatusMsg.ApplyHasState();
                m_StatusMsg.State.StreamState(State.StreamState());
                m_StatusMsg.State.DataState(State.DataState());
                m_StatusMsg.State.Code(State.Code());
                m_StatusMsg.State.Text(State.Text());
            }

            if (HasAuthenticationErrorCode || HasAuthenticationErrorText)
            {
                m_StatusMsg.ApplyHasMsgKey();
                m_StatusMsg.MsgKey.ApplyHasAttrib();
                m_StatusMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;

                CodecReturnCode ret = m_StatusMsg.EncodeInit(encodeIter, 0);
                if (ret != CodecReturnCode.ENCODE_MSG_KEY_ATTRIB)
                    return ret;
                ret = EncodeAttrib(encodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                ret = m_StatusMsg.EncodeKeyAttribComplete(encodeIter, true);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;

                ret = m_StatusMsg.EncodeComplete(encodeIter, true);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }
            else
            {
                CodecReturnCode ret = m_StatusMsg.Encode(encodeIter);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;

                return CodecReturnCode.SUCCESS;
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodeAttrib(EncodeIterator encodeIter)
        {
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            CodecReturnCode ret = elementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasAuthenticationErrorCode)
            {
                element.DataType = DataTypes.UINT;
                element.Name = ElementNames.AUTHN_ERROR_CODE;
                tmpUInt.Value(AuthenticationErrorCode);
                if ((ret = element.Encode(encodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }
            if (HasAuthenticationErrorText && authenticationErrorText.Length != 0)
            {
                element.DataType = DataTypes.ASCII_STRING;
                element.Name = ElementNames.AUTHN_ERROR_TEXT;
                ret = element.Encode(encodeIter, AuthenticationErrorText);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            return elementList.EncodeComplete(encodeIter, true);

        }

        /// <summary>
        /// Decodes this Login Status message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginStatus message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.STATUS)
                return CodecReturnCode.FAILURE;

            IStatusMsg statusMsg = (IStatusMsg)msg;
            StreamId = msg.StreamId;
            if (statusMsg.CheckHasState())
            {
                HasState = true;
                State = statusMsg.State;
            }

            if (statusMsg.CheckClearCache())
            {
                ClearCache = true;
            }

            IMsgKey msgKey = msg.MsgKey;
            if (msgKey != null)
            {
                if (msgKey.CheckHasName())
                {
                    HasUserName = true;
                    UserName = msgKey.Name;
                    if (msgKey.CheckHasNameType())
                    {
                        HasUserNameType = true;
                        UserNameType = (Login.UserIdTypes)msgKey.NameType;
                    }
                }
                if (msgKey.CheckHasAttrib())
                {
                    CodecReturnCode ret = msg.DecodeKeyAttrib(decodeIter, msgKey);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;

                    return DecodeAttrib(decodeIter);
                }
            }
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeAttrib(DecodeIterator dIter)
        {
            elementList.Clear();
            CodecReturnCode ret = elementList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            element.Clear();
            while ((ret = element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                else if (element.Name.Equals(ElementNames.AUTHN_ERROR_CODE))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAuthenticationErrorCode = true;
                    AuthenticationErrorCode = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.AUTHN_ERROR_TEXT))
                {
                    if (element.DataType != DataTypes.ASCII_STRING
                            && element.DataType != DataTypes.BUFFER)
                        return CodecReturnCode.FAILURE;
                    HasAuthenticationErrorText = true;
                    AuthenticationErrorText = element.EncodedData;
                }
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Login Status message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "LoginStatus: \n");

            if (HasUserNameType)
            {
                stringBuf.Append(tab);
                stringBuf.Append("nameType: ");
                stringBuf.Append(UserNameType);
                stringBuf.Append(eol);
            }

            if (HasUserName)
            {
                stringBuf.Append(tab);
                stringBuf.Append("name: ");
                stringBuf.Append(UserName);
                stringBuf.Append(eol);
            }

            if (HasState)
            {
                stringBuf.Append(tab);
                stringBuf.Append("state: ");
                stringBuf.Append(State);
                stringBuf.Append(eol);
            }

            if (HasAuthenticationErrorCode)
            {
                stringBuf.Append(tab);
                stringBuf.Append("authenticationErrorCode: ");
                stringBuf.Append(AuthenticationErrorCode);
                stringBuf.Append(eol);
            }
            if (HasAuthenticationErrorText)
            {
                stringBuf.Append(tab);
                stringBuf.Append("authenticationErrorText: ");
                stringBuf.Append(AuthenticationErrorText);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
