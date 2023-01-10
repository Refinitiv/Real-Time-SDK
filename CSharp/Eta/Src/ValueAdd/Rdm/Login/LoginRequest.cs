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
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;

using static LSEG.Eta.Rdm.Login;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM login request. Used by an OMM Consumer or OMM Non-Interactive Provider to
    /// request a login.
    /// </summary>
    /// <remarks>
    ///
    /// <para>A Login Request message is encoded and sent by OMM consumer and
    /// non-interactive provider applications. This message registers a user with the
    /// system. After receiving a successful login response, applications can then begin
    /// consuming or providing additional content. An OMM provider can use the login
    /// request information to authenticate users with the Data Access Control
    /// System.</para>
    ///
    /// <para>The LoginRequest represents all members of a login request message and
    /// allows for simplified use in OMM applications that leverage RDMs.</para>
    ///
    /// </remarks>
    public class LoginRequest : MsgBase
    {
        #region Private Fields

        private IRequestMsg m_RequestMsg = new Msg();

        private Buffer userName = new();
        private LoginAttrib attrib = new();
        private Buffer instanceId = new();
        private Buffer password = new();
        private Buffer authenticationToken = new();
        private Buffer authenticationExtended = new();

        // private string blankStringConst = string.Empty;
        private readonly string blankStringConst = "\0";
        private static string? defaultUsername;

        private ElementEntry elementEntry = new();
        private ElementList elementList = new();
        private UInt tmpUInt = new();

        #endregion
        #region Public Message Properties

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get; set; }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.LOGIN"/>.
        /// </summary>
        public override int DomainType { get => m_RequestMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.REQUEST"/>
        /// </summary>
        public override int MsgClass { get => m_RequestMsg.MsgClass; }

        /// <summary>
        /// Flags for this message.  See <see cref="LoginRequestFlags"/>.
        /// </summary>
        public LoginRequestFlags Flags { get; set; }

        /// <summary>
        /// Gets / sets login attrib information.
        /// </summary>
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
        /// The userName that was used when sending the Login Request.
        /// </summary>
        ///
        /// <remarks><para><em>Required</em>. Populate this member with the username, email address,
        /// or user token based on the <see cref="UserNameType"/> specification.</para>
        ///
        /// <para>If you initialize LoginRequest using <see cref="InitDefaultRequest(int)"/>, it
        /// uses the name of the user currently logged into the system on which the application
        /// runs.</para></remarks>
        public Buffer UserName
        {
            get => userName;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, userName);
            }
        }

        /// <summary>
        /// Gets / sets instance id. InstanceId can be used to differentiate applications running on the same machine.
        /// </summary>
        public Buffer InstanceId
        {
            get => instanceId;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, instanceId);
            }
        }

        /// <summary>
        /// Gets / sets the password.
        /// </summary>
        public Buffer Password
        {
            get => password;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, password);
            }
        }

        /// <summary>
        /// Gets / sets the authentication token.
        /// </summary>
        public Buffer AuthenticationToken
        {
            get => authenticationToken;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, authenticationToken);
            }
        }

        /// <summary>
        /// Gets / sets the authentication extended data.
        /// </summary>
        public Buffer AuthenticationExtended
        {
            get => authenticationExtended;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, authenticationExtended);
            }
        }

        /// <summary>
        /// Indicates the role of the application. Populated by <see cref="RoleTypes"/>
        /// </summary>
        public long Role { get; set; }

        /// <summary>
        /// Indicates whether the Consumer desires connection information.
        /// If available, a list of servers will be present
        /// in the serverList field of the <see cref="LoginRefresh"/>
        /// </summary>
        public long DownloadConnectionConfig { get; set; }

        /// <summary>
        /// The type of the <see cref="UserName"/> that was used with the Login Request. Populated by <see cref="UserIdTypes"/>
        /// </summary>
        public Login.UserIdTypes UserNameType { get; set; }

        /// <summary>
        /// Checks the presence of user name type.
        /// </summary>
        public bool HasUserNameType
        {
            get => (Flags & LoginRequestFlags.HAS_USERNAME_TYPE) != 0;

            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_USERNAME_TYPE;
                else
                    Flags &= ~LoginRequestFlags.HAS_USERNAME_TYPE;
            }
        }

        /// <summary>
        /// Checks if login request is pause.
        /// </summary>
        public bool Pause
        {
            get => (Flags & LoginRequestFlags.PAUSE_ALL) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.PAUSE_ALL;
                else
                    Flags &= ~LoginRequestFlags.PAUSE_ALL;
            }
        }

        /// <summary>
        /// Checks if no refresh required flag is set.
        /// </summary>
        public bool NoRefresh
        {
            get => (Flags & LoginRequestFlags.NO_REFRESH) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.NO_REFRESH;
                else
                    Flags &= ~LoginRequestFlags.NO_REFRESH;
            }
        }

        /// <summary>
        /// Checks the presence of attrib field.
        /// </summary>
        /// <see cref="LoginAttrib"/>
        public bool HasAttrib
        {
            get => (Flags & LoginRequestFlags.HAS_ATTRIB) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_ATTRIB;
                else
                    Flags &= ~LoginRequestFlags.HAS_ATTRIB;
            }
        }

        /// <summary>
        /// Checks the presence of download connection config field.
        /// </summary>
        public bool HasDownloadConnectionConfig
        {
            get => (Flags & LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG;
                else
                    Flags &= ~LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG;
            }
        }

        /// <summary>
        /// Checks the presence of instance id field.
        /// </summary>
        public bool HasInstanceId
        {
            get => (Flags & LoginRequestFlags.HAS_INSTANCE_ID) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_INSTANCE_ID;
                else
                    Flags &= ~LoginRequestFlags.HAS_INSTANCE_ID;
            }
        }

        /// <summary>
        /// Checks the presence of application role field.
        /// </summary>
        public bool HasRole
        {
            get => (Flags & LoginRequestFlags.HAS_ROLE) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_ROLE;
                else
                    Flags &= ~LoginRequestFlags.HAS_ROLE;
            }
        }

        /// <summary>
        /// Checks the presence of the authenticationExtended data field.
        /// </summary>
        public bool HasAuthenticationExtended
        {
            get => (Flags & LoginRequestFlags.HAS_AUTHENTICATION_EXTENDED) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_AUTHENTICATION_EXTENDED;
                else
                    Flags &= ~LoginRequestFlags.HAS_AUTHENTICATION_EXTENDED;
            }
        }

        /// <summary>
        /// Checks the presence of password field.
        /// </summary>
        public bool HasPassword
        {
            get => (Flags & LoginRequestFlags.HAS_PASSWORD) != 0;
            set
            {
                if (value)
                    Flags |= LoginRequestFlags.HAS_PASSWORD;
                else
                    Flags &= ~LoginRequestFlags.HAS_PASSWORD;
            }
        }

        #endregion

        /// <summary>
        /// Login Request Message constructor.
        /// </summary>
        public LoginRequest()
        {
            Clear();
            try
            {
                defaultUsername = Environment.UserName;
            }
            catch (Exception)
            {
                defaultUsername = "eta";
            }

            InitDefaultRequest(1);
        }

        /// <summary>
        /// Clears the current contents of the login request object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.LOGIN;
            m_RequestMsg.ApplyStreaming();
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;

            Flags = default;
            InstanceId.Clear();
            UserName.Clear();
            Password.Clear();
            LoginAttrib.Clear();
            Role = 0;
            DownloadConnectionConfig = 0;
            UserNameType = 0;
            AuthenticationToken.Clear();
            AuthenticationExtended.Clear();
        }

        /// <summary>
        /// Initializes a LoginRequest with default information, clearing it and filling in
        /// a typical userName, applicationName and position.
        /// </summary>
        /// <param name="streamId">Stream ID to be used for this login Request.</param>
        public void InitDefaultRequest(int streamId)
        {

            StreamId = streamId;
            UserName.Data(defaultUsername);
            HasUserNameType = true;
            UserNameType = UserIdTypes.NAME;
            HasAttrib = true;
            LoginAttrib.InitDefaultAttrib();
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destRequestMsg</c>.
        /// </summary>
        /// <param name="destRequestMsg">LoginRequest object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(LoginRequest destRequestMsg)
        {
            Debug.Assert(destRequestMsg != null);

            destRequestMsg.StreamId = StreamId;
            destRequestMsg.Flags = Flags;

            BufferHelper.CopyBuffer(UserName, destRequestMsg.UserName);
            if (HasUserNameType)
            {
                destRequestMsg.HasUserNameType = true;
                destRequestMsg.UserNameType = UserNameType;
            }
            if (HasAttrib)
            {
                destRequestMsg.HasAttrib = true;
                LoginAttrib.Copy(destRequestMsg.LoginAttrib);
            }
            if (HasDownloadConnectionConfig)
            {
                destRequestMsg.HasDownloadConnectionConfig = true;
                destRequestMsg.DownloadConnectionConfig = DownloadConnectionConfig;
            }
            if (HasInstanceId)
            {
                destRequestMsg.HasInstanceId = true;
                BufferHelper.CopyBuffer(InstanceId, destRequestMsg.InstanceId);
            }
            if (HasPassword)
            {
                destRequestMsg.HasPassword = true;
                BufferHelper.CopyBuffer(Password, destRequestMsg.Password);
            }
            if (HasRole)
            {
                destRequestMsg.HasRole = true;
                destRequestMsg.Role = Role;
            }

            if (HasAuthenticationExtended)
            {
                destRequestMsg.HasAuthenticationExtended = true;
                BufferHelper.CopyBuffer(AuthenticationExtended, destRequestMsg.AuthenticationExtended);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this login request message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_RequestMsg.Clear();

            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.StreamId = StreamId;
            m_RequestMsg.DomainType = (int)Eta.Rdm.DomainType.LOGIN;
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;

            m_RequestMsg.ApplyStreaming();

            if (NoRefresh)
                m_RequestMsg.ApplyNoRefresh();

            if (Pause)
                m_RequestMsg.ApplyPause();

            m_RequestMsg.MsgKey.ApplyHasName();
            m_RequestMsg.MsgKey.Name = UserName;

            if (HasUserNameType)
            {
                m_RequestMsg.MsgKey.ApplyHasNameType();
                m_RequestMsg.MsgKey.NameType = (int)UserNameType;
                if (UserNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    m_RequestMsg.MsgKey.Name.Data(blankStringConst);
            }

            m_RequestMsg.MsgKey.ApplyHasAttrib();
            m_RequestMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;

            CodecReturnCode ret = m_RequestMsg.EncodeInit(encodeIter, 0);
            if (ret != CodecReturnCode.ENCODE_MSG_KEY_ATTRIB)
                return ret;

            ret = EncodeAttrib(encodeIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if ((ret = m_RequestMsg.EncodeKeyAttribComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                return ret;

            if ((ret = m_RequestMsg.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodeAttrib(EncodeIterator EncodeIter)
        {
            elementEntry.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            CodecReturnCode ret;

            if ((ret = elementList.EncodeInit(EncodeIter, null, 0)) != CodecReturnCode.SUCCESS)
                return ret;

            if (HasAttrib
                && LoginAttrib.HasApplicationId
                && LoginAttrib.ApplicationId.Length != 0)
            {
                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.APPID;
                if ((ret = elementEntry.Encode(EncodeIter, LoginAttrib.ApplicationId)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib
                && LoginAttrib.HasApplicationName
                && LoginAttrib.ApplicationName.Length != 0)
            {
                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.APPNAME;
                if ((ret = elementEntry.Encode(EncodeIter, LoginAttrib.ApplicationName)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib
                && LoginAttrib.HasPosition
                && LoginAttrib.Position.Length != 0)
            {
                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.POSITION;
                if ((ret = elementEntry.Encode(EncodeIter, LoginAttrib.Position)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasPassword && Password.Length != 0)
            {
                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.PASSWORD;
                if ((ret = elementEntry.Encode(EncodeIter, Password)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib && LoginAttrib.HasProvidePermissionProfile)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.PROV_PERM_PROF;
                tmpUInt.Value(LoginAttrib.ProvidePermissionProfile);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib && LoginAttrib.HasProvidePermissionExpressions)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.PROV_PERM_EXP;
                tmpUInt.Value(LoginAttrib.ProvidePermissionExpressions);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib && LoginAttrib.HasSingleOpen)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.SINGLE_OPEN;
                tmpUInt.Value(LoginAttrib.SingleOpen);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib && LoginAttrib.HasAllowSuspectData)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.ALLOW_SUSPECT_DATA;
                tmpUInt.Value(LoginAttrib.AllowSuspectData);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib && LoginAttrib.HasProviderSupportDictDownload)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
                tmpUInt.Value(LoginAttrib.SupportProviderDictionaryDownload);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasUserNameType
                && UserNameType == Login.UserIdTypes.AUTHN_TOKEN
                && UserName.Length != 0)
            {
                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.AUTHN_TOKEN;
                if ((ret = elementEntry.Encode(EncodeIter, UserName)) != CodecReturnCode.SUCCESS)
                    return ret;

                if (HasAuthenticationExtended && AuthenticationExtended.Length != 0)
                {
                    elementEntry.DataType = DataTypes.BUFFER;
                    elementEntry.Name = ElementNames.AUTHN_EXTENDED;
                    if ((ret = elementEntry.Encode(EncodeIter, AuthenticationExtended)) != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }

            if (HasInstanceId && InstanceId.Length != 0)
            {
                elementEntry.DataType = DataTypes.ASCII_STRING;
                elementEntry.Name = ElementNames.INST_ID;
                ret = elementEntry.Encode(EncodeIter, InstanceId);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasRole)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.ROLE;
                tmpUInt.Value(Role);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasDownloadConnectionConfig)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.DOWNLOAD_CON_CONFIG;
                tmpUInt.Value(DownloadConnectionConfig);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAttrib && LoginAttrib.HasSupportRoundTripLatencyMonitoring)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.ROUND_TRIP_LATENCY;
                tmpUInt.Value(LoginAttrib.SupportConsumerRTTMonitoring);
                ret = elementEntry.Encode(EncodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if ((ret = elementList.EncodeComplete(EncodeIter, true)) != CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decodes this Login Refresh message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginRefresh message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.REQUEST)
                return CodecReturnCode.FAILURE;

            IRequestMsg requestMsg = (IRequestMsg)msg;

            //All login requests should be streaming
            if ((requestMsg.Flags & (int)RequestMsgFlags.STREAMING) == 0)
                return CodecReturnCode.FAILURE;

            if ((requestMsg.Flags & (int)RequestMsgFlags.NO_REFRESH) != 0)
                NoRefresh = true;

            if ((requestMsg.Flags & (int)RequestMsgFlags.PAUSE) != 0)
                Pause = true;

            StreamId = msg.StreamId;

            IMsgKey msgKey = msg.MsgKey;
            if (msgKey == null
                || !msgKey.CheckHasName()
                || (msgKey.CheckHasAttrib()
                    && msgKey.AttribContainerType != DataTypes.ELEMENT_LIST))
            {
                return CodecReturnCode.FAILURE;
            }

            Buffer userName = msgKey.Name;
            UserName = userName;
            if (msgKey.CheckHasNameType())
            {
                HasUserNameType = true;
                UserNameType = (Login.UserIdTypes)msgKey.NameType;
            }

            if (msgKey.CheckHasAttrib())
            {
                CodecReturnCode ret = msg.DecodeKeyAttrib(decodeIter, msgKey);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                return DecodeAttrib(decodeIter);
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeAttrib(DecodeIterator dIter)
        {
            elementList.Clear();
            CodecReturnCode ret = elementList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            elementEntry.Clear();
            while ((ret = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (elementEntry.Name.Equals(ElementNames.ALLOW_SUSPECT_DATA))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasAllowSuspectData = true;
                    LoginAttrib.AllowSuspectData = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.APPID))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;

                    HasAttrib = true;
                    LoginAttrib.HasApplicationId = true;
                    LoginAttrib.ApplicationId = elementEntry.EncodedData;

                }
                else if (elementEntry.Name.Equals(ElementNames.APPNAME))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;

                    HasAttrib = true;
                    LoginAttrib.HasApplicationName = true;
                    LoginAttrib.ApplicationName = elementEntry.EncodedData;

                }
                else if (elementEntry.Name.Equals(ElementNames.POSITION))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;

                    HasAttrib = true;
                    LoginAttrib.HasPosition = true;
                    LoginAttrib.Position = elementEntry.EncodedData;
                }
                else if (elementEntry.Name.Equals(ElementNames.PASSWORD))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;
                    HasAttrib = true;
                    HasPassword = true;
                    Password = elementEntry.EncodedData;
                }
                else if (elementEntry.Name.Equals(ElementNames.AUTHN_TOKEN))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING && elementEntry.DataType != DataTypes.BUFFER)
                        return CodecReturnCode.FAILURE;
                    UserName = elementEntry.EncodedData;
                }
                else if (elementEntry.Name.Equals(ElementNames.AUTHN_EXTENDED))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING && elementEntry.DataType != DataTypes.BUFFER)
                        return CodecReturnCode.FAILURE;
                    HasAuthenticationExtended = true;
                    AuthenticationExtended = elementEntry.EncodedData;
                }
                else if (elementEntry.Name.Equals(ElementNames.INST_ID))
                {
                    if (elementEntry.DataType != DataTypes.ASCII_STRING)
                        return CodecReturnCode.FAILURE;
                    HasInstanceId = true;
                    InstanceId = elementEntry.EncodedData;
                }
                else if (elementEntry.Name.Equals(ElementNames.DOWNLOAD_CON_CONFIG))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    HasDownloadConnectionConfig = true;
                    DownloadConnectionConfig = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.PROV_PERM_EXP))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasProvidePermissionExpressions = true;
                    LoginAttrib.ProvidePermissionExpressions = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.PROV_PERM_PROF))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasProvidePermissionProfile = true;
                    LoginAttrib.ProvidePermissionProfile = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.SINGLE_OPEN))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasSingleOpen = true;
                    LoginAttrib.SingleOpen = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.ROLE))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasRole = true;
                    Role = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasAttrib = true;
                    LoginAttrib.HasProviderSupportDictDownload = true;
                    LoginAttrib.SupportProviderDictionaryDownload = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.ROUND_TRIP_LATENCY))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
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

        /// <summary>
        /// Returns a human readable string representation of the Login Request message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();

            stringBuf.Insert(0, "LoginRequest: \n");
            stringBuf.Append(tab);
            stringBuf.Append("userName: ");
            stringBuf.Append(UserName.ToString());
            stringBuf.Append(eol);
            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append("true");
            stringBuf.Append(eol);

            if (HasUserNameType)
            {
                stringBuf.Append(tab);
                stringBuf.Append("nameType: ");
                stringBuf.Append(UserNameType);
                stringBuf.Append(eol);
            }

            if (Pause)
            {
                stringBuf.Append(tab);
                stringBuf.Append("pauseAll:");
                stringBuf.Append("true");
                stringBuf.Append(eol);
            }
            if (NoRefresh)
            {
                stringBuf.Append(tab);
                stringBuf.Append("noRefresh:");
                stringBuf.Append("true");
                stringBuf.Append(eol);
            }

            if (HasAttrib)
            {
                stringBuf.Append(LoginAttrib.ToString());
            }

            if (HasDownloadConnectionConfig)
            {
                stringBuf.Append(tab);
                stringBuf.Append("downloadConnectionConfig: ");
                stringBuf.Append(DownloadConnectionConfig);
                stringBuf.Append(eol);
            }
            if (HasInstanceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("instanceId: ");
                stringBuf.Append(InstanceId);
                stringBuf.Append(eol);
            }

            if (HasRole)
            {
                stringBuf.Append(tab);
                stringBuf.Append("role: ");
                stringBuf.Append(Role);
                stringBuf.Append(eol);
            }

            if (HasAuthenticationExtended)
            {
                stringBuf.Append(tab);
                stringBuf.Append("authenticationExtended: ");
                stringBuf.Append(AuthenticationExtended);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }       
    }
}
