/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;

using static Refinitiv.Eta.Rdm.Login;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Login Refresh.
    /// This message is used to respond to a Login Request message after the user's login is accepted.
    /// </summary>
    public class LoginRefresh : MsgBase
    {
        #region Private Fields

        private IRefreshMsg m_RefreshMsg = new Msg();

        private Buffer userName = new();
        private LoginAttrib attrib = new();
        private LoginSupportFeatures features = new();
        private LoginConnectionConfig connectionConfig = new();
        private Buffer authenticationExtendedResp = new();
        private Buffer authenticationErrorText = new();
        private string blankStringConst = string.Empty;

        private ElementList elementList = new();
        private ElementEntry element = new();
        private UInt tmpUInt = new();

        #endregion
        #region Public Message Properties

        public override int StreamId { get; set; }

        public override int MsgClass { get => m_RefreshMsg.MsgClass; }

        public override int DomainType { get => m_RefreshMsg.DomainType; }

        public LoginRefreshFlags Flags { get; set; }

        public bool Solicited
        {
            get => (Flags & LoginRefreshFlags.SOLICITED) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.SOLICITED;
                else
                    Flags &= ~LoginRefreshFlags.SOLICITED;
            }
        }

        /// <summary>
        /// Whether connection config is present.
        /// </summary>
        /// <seealso cref="ConnectionConfig"/>
        public bool HasConnectionConfig
        {
            get => (Flags & LoginRefreshFlags.HAS_CONN_CONFIG) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_CONN_CONFIG;
                else
                    Flags &= ~LoginRefreshFlags.HAS_CONN_CONFIG;
            }
        }

        public bool HasAttrib
        {
            get => (Flags & LoginRefreshFlags.HAS_ATTRIB) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_ATTRIB;
                else
                    Flags &= ~LoginRefreshFlags.HAS_ATTRIB;
            }
        }
        public bool HasFeatures
        {
            get => (Flags & LoginRefreshFlags.HAS_FEATURES) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_FEATURES;
                else
                    Flags &= ~LoginRefreshFlags.HAS_FEATURES;
            }
        }
        /// <summary>
        /// Checks the presence of user name field.
        /// </summary>
        public bool HasUserName
        {
            get => (Flags & LoginRefreshFlags.HAS_USERNAME) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_USERNAME;
                else
                    Flags &= ~LoginRefreshFlags.HAS_USERNAME;
            }
        }

        /// <summary>
        /// Checks the presence of user name type field.
        /// </summary>
        public bool HasUserNameType
        {
            get => (Flags & LoginRefreshFlags.HAS_USERNAME_TYPE) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_USERNAME_TYPE;
                else
                    Flags &= ~LoginRefreshFlags.HAS_USERNAME_TYPE;
            }
        }

        /// <summary>
        /// Indicates whether <see cref="AuthenticationTTReissue"/> is present.
        /// </summary>
        public bool HasAuthenicationTTReissue
        {
            get => (Flags & LoginRefreshFlags.HAS_AUTHENTICATION_TT_REISSUE) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_AUTHENTICATION_TT_REISSUE;
                else
                    Flags &= ~LoginRefreshFlags.HAS_AUTHENTICATION_TT_REISSUE;
            }
        }

        public bool HasAuthenticationExtendedResp
        {
            get => (Flags & LoginRefreshFlags.HAS_AUTHENTICATION_EXTENDED_RESP) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_AUTHENTICATION_EXTENDED_RESP;
                else
                    Flags &= ~LoginRefreshFlags.HAS_AUTHENTICATION_EXTENDED_RESP;
            }
        }

        public bool HasAuthenticationErrorText
        {
            get => (Flags & LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT;
                else
                    Flags &= ~LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT;
            }
        }

        public bool HasAuthenticationErrorCode
        {
            get => (Flags & LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE;
                else
                    Flags &= ~LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE;
            }
        }

        public bool HasSequenceNumber
        {
            get => (Flags & LoginRefreshFlags.HAS_SEQ_NUM) != 0;
            set
            {
                if (value)
                    Flags |= LoginRefreshFlags.HAS_SEQ_NUM;
                else
                    Flags &= ~LoginRefreshFlags.HAS_SEQ_NUM;
            }
        }

        public bool ClearCache
        {
            get => (Flags & LoginRefreshFlags.CLEAR_CACHE) != 0;
            set
            {
                if (value)
                {
                    Flags |= LoginRefreshFlags.CLEAR_CACHE;
                }
                else
                    Flags &= ~LoginRefreshFlags.CLEAR_CACHE;
            }
        }

        /// <summary>
        /// Sets userName for login to the user specified buffer.
        /// Data and position of userName buffer will be set to passed in buffer's data and position.
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

        public Buffer AuthenticationExtendedResp
        {
            get => authenticationExtendedResp;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, authenticationExtendedResp);
            }
        }

        public Buffer AuthenticationErrorText
        {
            get => authenticationErrorText;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, authenticationErrorText);
            }
        }

        public LoginAttrib LoginAttrib
        {
            get => attrib;
            set
            {
                Debug.Assert(value != null);
                value.Copy(attrib);
            }
        }

        /// <summary>
        /// Connection configuration information.
        /// </summary>
        /// <seealso cref="HasConnectionConfig"/>
        public LoginConnectionConfig ConnectionConfig
        {
            get => connectionConfig;
            set
            {
                Debug.Assert(value != null);
                value.Copy(connectionConfig);
            }
        }

        public long SequenceNumber { get; set; }

        /// <summary>
        /// The type of the userName that was used with the Login Refresh. Populated by <see cref="Login.UserIdTypes"/>
        /// </summary>
        public UserIdTypes UserNameType { get; set; }

        /// <summary>
        /// Indicates when a new authentication token needs to be reissued (in UNIX Epoch time).
        /// </summary>
        /// <seealso cref="HasAuthenicationTTReissue"/>
        public long AuthenticationTTReissue { get; set; }

        public long AuthenticationErrorCode { get; set; }

        public LoginSupportFeatures SupportedFeatures
        {
            get => features;
            set
            {
                Debug.Assert(value != null);
                value.Copy(features);
            }
        }

        public State State { get; set; } = new State();

        #endregion

        public LoginRefresh()
        {
            Clear();
        }

        public CodecReturnCode Copy(LoginRefresh destRefreshMsg)
        {
            Debug.Assert(destRefreshMsg != null);
            destRefreshMsg.StreamId = StreamId;
            if (HasUserName)
            {
                destRefreshMsg.HasUserName = true;
                BufferHelper.CopyBuffer(UserName, destRefreshMsg.UserName);
            }
            if (HasUserNameType)
            {
                destRefreshMsg.HasUserNameType = true;
                destRefreshMsg.UserNameType = UserNameType;
            }
            if (HasSequenceNumber)
            {
                destRefreshMsg.HasSequenceNumber = true;
                destRefreshMsg.SequenceNumber = SequenceNumber;
            }

            if (HasAuthenicationTTReissue)
            {
                destRefreshMsg.HasAuthenicationTTReissue = true;
                destRefreshMsg.AuthenticationTTReissue = AuthenticationTTReissue;
            }
            if (HasAuthenticationExtendedResp)
            {
                destRefreshMsg.HasAuthenticationExtendedResp = true;
                BufferHelper.CopyBuffer(AuthenticationExtendedResp, destRefreshMsg.AuthenticationExtendedResp);
            }
            if (HasAuthenticationErrorCode)
            {
                destRefreshMsg.HasAuthenticationErrorCode = true;
                destRefreshMsg.AuthenticationErrorCode = AuthenticationErrorCode;
            }
            if (HasAuthenticationErrorText)
            {
                destRefreshMsg.HasAuthenticationErrorText = true;
                BufferHelper.CopyBuffer(AuthenticationErrorText, destRefreshMsg.AuthenticationErrorText);
            }

            if (ClearCache)
            {
                destRefreshMsg.ClearCache = true;
            }
            if (Solicited)
            {
                destRefreshMsg.Solicited = true;
            }

            State.Copy(destRefreshMsg.State);

            if (HasConnectionConfig)
            {
                destRefreshMsg.HasConnectionConfig = true;
                ConnectionConfig.Copy(destRefreshMsg.ConnectionConfig);
            }

            if (HasAttrib)
            {
                destRefreshMsg.HasAttrib = true;
                LoginAttrib.Copy(destRefreshMsg.LoginAttrib);
            }
            if (HasFeatures)
            {
                destRefreshMsg.HasFeatures = true;
                SupportedFeatures.Copy(destRefreshMsg.SupportedFeatures);
            }

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator EncodeIter)
        {
            m_RefreshMsg.Clear();

            // message header
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
            m_RefreshMsg.StreamId = StreamId;
            m_RefreshMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.LOGIN;
            m_RefreshMsg.ContainerType = DataTypes.NO_DATA;
            m_RefreshMsg.ApplyHasMsgKey();
            m_RefreshMsg.ApplyRefreshComplete();
            m_RefreshMsg.State.DataState(State.DataState());
            m_RefreshMsg.State.StreamState(State.StreamState());
            m_RefreshMsg.State.Code(State.Code());
            m_RefreshMsg.State.Text(State.Text());

            if (ClearCache)
                m_RefreshMsg.ApplyClearCache();
            if (Solicited)
                m_RefreshMsg.ApplySolicited();

            if (HasSequenceNumber)
            {
                m_RefreshMsg.ApplyHasSeqNum();
                m_RefreshMsg.SeqNum = SequenceNumber;
            }

            if (HasUserName)
            {
                m_RefreshMsg.MsgKey.ApplyHasName();
                m_RefreshMsg.MsgKey.Name = UserName;
                m_RefreshMsg.MsgKey.NameType = (int)UserNameType;
            }

            if (HasUserNameType)
            {
                if (UserNameType == UserIdTypes.TOKEN)
                {
                    m_RefreshMsg.MsgKey.ApplyHasName();
                    m_RefreshMsg.MsgKey.Name.Data(blankStringConst);
                }
                m_RefreshMsg.MsgKey.ApplyHasNameType();
                m_RefreshMsg.MsgKey.NameType = (int)UserNameType;
            }

            // key attrib
            m_RefreshMsg.MsgKey.ApplyHasAttrib();
            m_RefreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            if (HasConnectionConfig)
            {
                m_RefreshMsg.ContainerType = DataTypes.ELEMENT_LIST;
            }

            CodecReturnCode ret = m_RefreshMsg.EncodeInit(EncodeIter, 0);
            if (ret != CodecReturnCode.ENCODE_MSG_KEY_ATTRIB)
                return ret;
            ret = EncodeAttrib(EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = m_RefreshMsg.EncodeKeyAttribComplete(EncodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            // Encode conn config now, if specified
            if (HasConnectionConfig)
            {
                ret = ConnectionConfig.Encode(EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            ret = m_RefreshMsg.EncodeComplete(EncodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodeAttrib(EncodeIterator EncodeIter)
        {
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            CodecReturnCode ret = elementList.EncodeInit(EncodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasAuthenicationTTReissue)
            {
                element.DataType = DataTypes.UINT;
                element.Name = ElementNames.AUTHN_TT_REISSUE;
                tmpUInt.Value(AuthenticationTTReissue);
                if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }
            if (HasAuthenticationExtendedResp && AuthenticationExtendedResp.Length != 0)
            {
                element.DataType = DataTypes.ASCII_STRING;
                element.Name = ElementNames.AUTHN_EXTENDED_RESP;
                ret = element.Encode(EncodeIter, AuthenticationExtendedResp);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            if (HasAuthenticationErrorCode)
            {
                element.DataType = DataTypes.UINT;
                element.Name = ElementNames.AUTHN_ERROR_CODE;
                tmpUInt.Value(AuthenticationErrorCode);
                if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }
            if (HasAuthenticationErrorText && AuthenticationErrorText.Length != 0)
            {
                element.DataType = DataTypes.ASCII_STRING;
                element.Name = ElementNames.AUTHN_ERROR_TEXT;
                ret = element.Encode(EncodeIter, AuthenticationErrorText);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib)
            {
                if (LoginAttrib.HasApplicationId && LoginAttrib.ApplicationId.Length != 0)
                {
                    element.DataType = DataTypes.ASCII_STRING;
                    element.Name = ElementNames.APPID;
                    ret = element.Encode(EncodeIter, LoginAttrib.ApplicationId);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasApplicationName && LoginAttrib.ApplicationName.Length != 0)
                {
                    element.DataType = DataTypes.ASCII_STRING;
                    element.Name = ElementNames.APPNAME;
                    ret = element.Encode(EncodeIter, LoginAttrib.ApplicationName);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasPosition && LoginAttrib.Position.Length != 0)
                {
                    element.DataType = DataTypes.ASCII_STRING;
                    element.Name = ElementNames.POSITION;
                    ret = element.Encode(EncodeIter, LoginAttrib.Position);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasProvidePermissionProfile)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.PROV_PERM_PROF;
                    tmpUInt.Value(LoginAttrib.ProvidePermissionProfile);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasProvidePermissionExpressions)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.PROV_PERM_EXP;
                    tmpUInt.Value(LoginAttrib.ProvidePermissionExpressions);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasSingleOpen)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SINGLE_OPEN;
                    tmpUInt.Value(LoginAttrib.SingleOpen);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasAllowSuspectData)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.ALLOW_SUSPECT_DATA;
                    tmpUInt.Value(LoginAttrib.AllowSuspectData);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (LoginAttrib.HasSupportRoundTripLatencyMonitoring)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.ROUND_TRIP_LATENCY;
                    tmpUInt.Value(LoginAttrib.SupportConsumerRTTMonitoring);
                    ret = element.Encode(EncodeIter, tmpUInt);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            if (HasFeatures)
            {
                if (SupportedFeatures.HasSupportPost)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SUPPORT_POST;
                    tmpUInt.Value(SupportedFeatures.SupportOMMPost);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (SupportedFeatures.HasSupportBatchRequests
                    || SupportedFeatures.HasSupportBatchReissues
                    || SupportedFeatures.HasSupportBatchCloses)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SUPPORT_BATCH;
                    int temp = 0;
                    if (SupportedFeatures.HasSupportBatchRequests)
                        temp |= Login.BatchSupportFlags.SUPPORT_REQUESTS;
                    if (SupportedFeatures.HasSupportBatchReissues)
                        temp |= Login.BatchSupportFlags.SUPPORT_REISSUES;
                    if (SupportedFeatures.HasSupportBatchCloses)
                        temp |= Login.BatchSupportFlags.SUPPORT_CLOSES;
                    tmpUInt.Value(temp);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (SupportedFeatures.HasSupportViewRequests)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SUPPORT_VIEW;
                    tmpUInt.Value(SupportedFeatures.SupportViewRequests);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (SupportedFeatures.HasSupportStandby)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SUPPORT_STANDBY;
                    tmpUInt.Value(SupportedFeatures.SupportStandby);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (SupportedFeatures.HasSupportOptimizedPauseResume)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SUPPORT_OPR;
                    tmpUInt.Value(SupportedFeatures.SupportOptimizedPauseResume);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }

                if (SupportedFeatures.HasSupportProviderDictionaryDownload)
                {
                    element.DataType = DataTypes.UINT;
                    element.Name = ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
                    tmpUInt.Value(SupportedFeatures.SupportProviderDictionaryDownload);
                    if ((ret = element.Encode(EncodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }

            return elementList.EncodeComplete(EncodeIter, true);
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();

            if (msg.MsgClass != MsgClasses.REFRESH)
                return CodecReturnCode.FAILURE;
            StreamId = msg.StreamId;

            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            if (refreshMsg.CheckSolicited())
                Solicited = true;
            if (refreshMsg.CheckClearCache())
                ClearCache = true;

            refreshMsg.State.Copy(State);

            if (refreshMsg.CheckHasSeqNum())
            {
                HasSequenceNumber = true;
                SequenceNumber = refreshMsg.SeqNum;
            }

            IMsgKey msgKey = msg.MsgKey;
            if (msgKey == null
                || (msgKey.CheckHasAttrib()
                    && msgKey.AttribContainerType != DataTypes.ELEMENT_LIST))
            {
                return CodecReturnCode.FAILURE;
            }

            if (msgKey.CheckHasName() && msgKey.Name != null)
            {
                HasUserName = true;
                UserName = msgKey.Name;
            }

            if (msgKey.CheckHasNameType())
            {
                HasUserNameType = true;
                UserNameType = (Login.UserIdTypes)msgKey.NameType;
            }

            if (msg.ContainerType == DataTypes.ELEMENT_LIST)
            {
                CodecReturnCode ret = DecodePayload(dIter, refreshMsg);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (msgKey.CheckHasAttrib())
            {
                CodecReturnCode ret = msg.DecodeKeyAttrib(dIter, msgKey);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                return DecodeAttrib(dIter);
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodePayload(DecodeIterator dIter, IRefreshMsg msg)
        {
            Debug.Assert(msg.ContainerType == DataTypes.ELEMENT_LIST);

            // Decode payload containing connection config in login refresh
            CodecReturnCode ret = elementList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // Decode each element entry in list
            while ((ret = element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                //connectionconfig
                if (element.Name.Equals(ElementNames.CONNECTION_CONFIG))
                {
                    HasConnectionConfig = true;
                    if (element.DataType != DataTypes.VECTOR)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    ret = ConnectionConfig.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
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

                if (element.Name.Equals(ElementNames.ALLOW_SUSPECT_DATA))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    attrib.HasAllowSuspectData = true;
                    attrib.AllowSuspectData = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.APPID))
                {
                    if (element.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;

                    HasAttrib = true;
                    LoginAttrib.HasApplicationId = true;
                    LoginAttrib.ApplicationId = element.EncodedData;

                }
                else if (element.Name.Equals(ElementNames.APPNAME))
                {
                    if (element.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;

                    HasAttrib = true;
                    LoginAttrib.HasApplicationName = true;
                    LoginAttrib.ApplicationName = element.EncodedData;

                }
                else if (element.Name.Equals(ElementNames.POSITION))
                {
                    if (element.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;

                    HasAttrib = true;
                    LoginAttrib.HasPosition = true;
                    LoginAttrib.Position = element.EncodedData;

                }
                else if (element.Name.Equals(ElementNames.PROV_PERM_EXP))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasProvidePermissionExpressions = true;
                    LoginAttrib.ProvidePermissionExpressions = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.PROV_PERM_PROF))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasProvidePermissionProfile = true;
                    LoginAttrib.ProvidePermissionProfile = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.SINGLE_OPEN))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasSingleOpen = true;
                    LoginAttrib.SingleOpen = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.SUPPORT_POST))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasFeatures = true;
                    SupportedFeatures.HasSupportPost = true;
                    SupportedFeatures.SupportOMMPost = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.SUPPORT_STANDBY))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasFeatures = true;
                    SupportedFeatures.HasSupportStandby = true;
                    SupportedFeatures.SupportStandby = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.SUPPORT_BATCH))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasFeatures = true;
                    if ((tmpUInt.ToLong() & Login.BatchSupportFlags.SUPPORT_REQUESTS) > 0)
                    {
                        SupportedFeatures.HasSupportBatchRequests = true;
                        SupportedFeatures.SupportBatchRequests = 1;
                    }
                    if ((tmpUInt.ToLong() & Login.BatchSupportFlags.SUPPORT_REISSUES) > 0)
                    {
                        SupportedFeatures.HasSupportBatchReissues = true;
                        SupportedFeatures.SupportBatchReissues = 1;
                    }
                    if ((tmpUInt.ToLong() & Login.BatchSupportFlags.SUPPORT_CLOSES) > 0)
                    {
                        SupportedFeatures.HasSupportBatchCloses = true;
                        SupportedFeatures.SupportBatchCloses = 1;
                    }
                }
                else if (element.Name.Equals(ElementNames.SUPPORT_VIEW))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasFeatures = true;
                    SupportedFeatures.HasSupportViewRequests = true;
                    SupportedFeatures.SupportViewRequests = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.SUPPORT_OPR))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasFeatures = true;
                    SupportedFeatures.HasSupportOptimizedPauseResume = true;
                    SupportedFeatures.SupportOptimizedPauseResume = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasFeatures = true;
                    SupportedFeatures.HasSupportProviderDictionaryDownload = true;
                    SupportedFeatures.SupportProviderDictionaryDownload = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.AUTHN_TT_REISSUE))
                {
                    if (element.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAuthenicationTTReissue = true;
                    AuthenticationTTReissue = tmpUInt.ToLong();
                }
                else if (element.Name.Equals(ElementNames.AUTHN_EXTENDED_RESP))
                {
                    if (element.DataType != DataTypes.ASCII_STRING
                            && element.DataType != DataTypes.BUFFER)
                        return CodecReturnCode.FAILURE;

                    HasAuthenticationExtendedResp = true;
                    AuthenticationExtendedResp = element.EncodedData;

                }
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
                    if (element.DataType != DataTypes.ASCII_STRING && element.DataType != DataTypes.BUFFER)
                        return CodecReturnCode.FAILURE;

                    HasAuthenticationErrorText = true;
                    AuthenticationErrorText = element.EncodedData;

                }
                else if (element.Name.Equals(ElementNames.ROUND_TRIP_LATENCY))
                {
                    if (element.DataType != DataTypes.UINT)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    HasAttrib = true;
                    LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
                    LoginAttrib.SupportConsumerRTTMonitoring = tmpUInt.ToLong();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "LoginRefresh: \n");
            stringBuf.Append(tab);
            stringBuf.Append("name: ");
            stringBuf.Append(UserName);
            stringBuf.Append(eol);
            stringBuf.Append(tab);
            stringBuf.Append("nameType: ");
            stringBuf.Append(UserNameType);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(State);
            stringBuf.Append(eol);

            if (Solicited)
            {
                stringBuf.Append(tab);
                stringBuf.Append("isSolicited: ");
                stringBuf.Append(true);
                stringBuf.Append(eol);
            }

            if (HasAuthenicationTTReissue)
            {
                stringBuf.Append(tab);
                stringBuf.Append("authenticationTTReissue: ");
                stringBuf.Append(AuthenticationTTReissue);
                stringBuf.Append(eol);
            }
            if (HasAuthenticationExtendedResp)
            {
                stringBuf.Append(tab);
                stringBuf.Append("authenticationExtendedResp: ");
                stringBuf.Append(AuthenticationExtendedResp);
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
                stringBuf.Append(HasAuthenticationErrorText);
                stringBuf.Append(eol);
            }

            if (HasAttrib)
            {
                stringBuf.Append(LoginAttrib.ToString());
            }

            if (HasFeatures)
            {
                stringBuf.Append(SupportedFeatures.ToString());
            }
            if (HasConnectionConfig)
            {
                stringBuf.Append(ConnectionConfig.ToString());
            }
            return stringBuf.ToString();
        }

        public override void Clear()
        {
            m_RefreshMsg.Clear();
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
            m_RefreshMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.LOGIN;
            m_RefreshMsg.ContainerType = DataTypes.NO_DATA;
            m_RefreshMsg.ApplyHasMsgKey();
            m_RefreshMsg.ApplyRefreshComplete();

            Flags = default;
            LoginAttrib.Clear();
            features.Clear();

            State.Clear();
            State.StreamState(StreamStates.OPEN);
            State.DataState(DataStates.OK);
            State.Code(StateCodes.NONE);

            UserNameType = Login.UserIdTypes.NAME;
            userName.Clear();
            AuthenticationTTReissue = 0;
            AuthenticationErrorCode = 0;
            authenticationExtendedResp.Clear();
            authenticationErrorText.Clear();
            connectionConfig.Clear();
        }
    }
}
