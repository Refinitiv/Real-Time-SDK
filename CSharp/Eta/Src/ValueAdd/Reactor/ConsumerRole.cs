/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Buffer = Refinitiv.Eta.Codec.Buffer;
using Refinitiv.Eta.ValueAdd.Rdm;
using static Refinitiv.Eta.Rdm.Dictionary;
using static Refinitiv.Eta.Rdm.Directory;
using static Refinitiv.Eta.Rdm.Login;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Class representing the role of an OMM Consumer.
    /// </summary>
    /// <see cref="ReactorRole"/>
    /// <see cref="ReactorRoleType"/>
    public class ConsumerRole : ReactorRole
    {
        private LoginRequest? m_LoginRequest;
        DirectoryRequest? m_DirectoryRequest = null;

        DictionaryClose? m_FieldDictionaryClose = null;
        DictionaryClose? m_EnumDictionaryClose = null;

        public const int LOGIN_STREAM_ID = 1;
        public const int DIRECTORY_STREAM_ID = 2;
        public const int FIELD_DICTIONARY_STREAM_ID = 3;
        public const int ENUM_DICTIONARY_STREAM_ID = 4;

        public bool ReceivedFieldDictionaryResp { get; set; } = false;
        public bool ReceivedEnumDictionaryResp { get; set; } = false;
        /// <summary>
        /// The <see cref="LoginRequest"/> to be sent during the connection establishment process.
        /// </summary>
        ///
        /// <remarks>This can be populated with a user's specific information or invoke
        /// <see cref="InitDefaultRDMLoginRequest()"/> to populate with default information. If
        /// this parameter is left empty no login will be sent to the system; useful for
        /// systems that do not require a login.</remarks>
        public LoginRequest? RdmLoginRequest
        {
            get => m_LoginRequest;
            set
            {
                CopyLoginRequest(value);
            }
        }

        public LoginRTT? RdmLoginRTT { get; set; }

        public bool RTTEnabled { get; set; }

        public IRDMLoginMsgCallback? LoginMsgCallback { get; set; }

        public const long FILTER_TO_REQUEST = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE |  ServiceFilterFlags.GROUP;

        /// <summary>
        /// A Directory Request to be sent during the setup of a Consumer-Provider session.
        /// This can be populated with a user's specific information or invoke <see cref="InitDefaultRDMDirectoryRequest"/>
        /// to populate with default information. Requires LoginRequest to be set.
        /// </summary>
        public DirectoryRequest? RdmDirectoryRequest
        {
            get => m_DirectoryRequest;
            set { CopyDirectoryRequest(value); }
        }

        /// <summary>
        /// Field dictionary name
        /// </summary>
        public Buffer FieldDictionaryName { get; private set; } = new Buffer();

        /// <summary>
        /// Enum type dictionary name
        /// </summary>
        public Buffer EnumTypeDictionaryName { get; private set; } = new Buffer();

        /// <summary>
        /// A Field Dictionary Request to be sent during the setup of a Consumer-Provider session.
        /// Requires DirectoryRequest to be set.
        /// </summary>
        public DictionaryRequest? RdmFieldDictionaryRequest { get; set; }

        /// <summary>
        /// A EnumType Dictionary Request to be sent during the setup of a Consumer-Provider session.
        /// Requires FieldDictionaryRequest to be set.
        /// </summary>
        public DictionaryRequest? RdmEnumTypeDictionaryRequest { get; set; }

        /// <summary>
        /// A callback function for processing RDMDirectoryMsgEvents received.
        /// If not present, the received message will be passed to the DefaultMsgCallback.
        /// </summary>
        public IDirectoryMsgCallback? DirectoryMsgCallback { get; set; }

        /// <summary>
        /// A callback function for processing RDMDictionaryMsgEvents received.
        /// If not present, the received message will be passed to the DefaultMsgCallback.
        /// </summary>
        public IDictionaryMsgCallback? DictionaryMsgCallback { get; set; }

        public DictionaryDownloadMode DictionaryDownloadMode { get; set; } = DictionaryDownloadMode.NONE;

        /// <summary>
        /// Gets or sets <see cref="ReactorOAuthCredential"/> to specify OAuth credential for authentication with the
        /// token service. 
        /// </summary>
        public ReactorOAuthCredential? ReactorOAuthCredential { get; set; }

        /// <summary>
        /// Instantiates a new consumer role.
        /// </summary>
        public ConsumerRole()
        {
            Type = ReactorRoleType.CONSUMER;
            FieldDictionaryName.Data("RWFFld");
            EnumTypeDictionaryName.Data("RWFEnum");
        }

        /// <summary>
        /// Initializes the RDM DirectoryRequest with default information.
        /// If the rdmDirectoryRequest has already been defined (due to a previous call to
        /// <see cref="RdmDirectoryRequest"/>, the DirectoryRequest object will be reused.
        /// </summary>
        public void InitDefaultRDMDirectoryRequest()
        {
            int streamId;

            if (m_DirectoryRequest == null)
            {
                streamId = DIRECTORY_STREAM_ID;
                m_DirectoryRequest = new DirectoryRequest();
            }
            else
            {
                streamId = m_DirectoryRequest.StreamId == 0 ? DIRECTORY_STREAM_ID : m_DirectoryRequest.StreamId;
                m_DirectoryRequest.Clear();
            }

            m_DirectoryRequest.StreamId = streamId;
            m_DirectoryRequest.Filter = FILTER_TO_REQUEST;
            m_DirectoryRequest.Streaming = true;

            return;
        }

        /// <summary>
        /// Initializes the RDM Field DictionaryRequest with default information.
        /// If the rdmFieldDictionaryRequest has already been defined,
        /// the rdmFieldDictionaryRequest object will be reused.
        /// </summary>
        internal void InitDefaultRDMFieldDictionaryRequest()
        {
            int streamId;

            if (RdmFieldDictionaryRequest == null)
            {
                streamId = FIELD_DICTIONARY_STREAM_ID;
                RdmFieldDictionaryRequest = new DictionaryRequest();
            }
            else
            {
                streamId = RdmFieldDictionaryRequest.StreamId == 0 ? FIELD_DICTIONARY_STREAM_ID : RdmFieldDictionaryRequest.StreamId;
                RdmFieldDictionaryRequest.Clear();
            }

            // make sure stream id isn't already being used
            while (m_LoginRequest != null && streamId == m_LoginRequest.StreamId
                || m_DirectoryRequest != null && streamId == m_DirectoryRequest.StreamId)
            {
                streamId++;
            }
            RdmFieldDictionaryRequest.StreamId = streamId;
            RdmFieldDictionaryRequest.Streaming = true;
            RdmFieldDictionaryRequest.Verbosity = VerbosityValues.NORMAL;
            RdmFieldDictionaryRequest.DictionaryName = FieldDictionaryName;

            return;
        }

        /// <summary>
        /// Initializes the RDM EnumType DictionaryRequest with default information.
        /// If the rdmEnumDictionaryRequest has already been defined,
        /// the rdmEnumDictionaryRequest object will be reused.
        /// </summary>
        internal void InitDefaultRDMEnumDictionaryRequest()
        {
            int streamId;

            if (RdmEnumTypeDictionaryRequest == null)
            {
                streamId = ENUM_DICTIONARY_STREAM_ID;
                RdmEnumTypeDictionaryRequest = new DictionaryRequest();
            }
            else
            {
                streamId = RdmEnumTypeDictionaryRequest.StreamId == 0 ? ENUM_DICTIONARY_STREAM_ID : RdmEnumTypeDictionaryRequest.StreamId;
                RdmEnumTypeDictionaryRequest.Clear();
            }

            // make sure stream id isn't already being used
            while (m_LoginRequest != null && streamId == m_LoginRequest.StreamId
                || m_DirectoryRequest != null && streamId == m_DirectoryRequest.StreamId
                || RdmFieldDictionaryRequest != null && streamId == RdmFieldDictionaryRequest.StreamId)
            {
                streamId++;
            }

            RdmEnumTypeDictionaryRequest.StreamId = streamId;
            RdmEnumTypeDictionaryRequest.Streaming = true;
            RdmEnumTypeDictionaryRequest.Verbosity = VerbosityValues.NORMAL;
            RdmEnumTypeDictionaryRequest.DictionaryName = EnumTypeDictionaryName;

            return;
        }

        /// <summary>
        /// The DictionaryClose to be sent to close the EnumType Dictionary stream.
        /// This corresponds to the EnumType DictionaryRequest sent
        /// during the connection establishment process.
        /// </summary>
        /// <returns><see cref="DictionaryClose"/> message</returns>
        public DictionaryClose? EnumDictionaryClose()
        {
            if (RdmEnumTypeDictionaryRequest == null)
                return null;

            if (m_EnumDictionaryClose == null)
            {
                m_EnumDictionaryClose = new DictionaryClose();
            }

            m_EnumDictionaryClose.StreamId = RdmEnumTypeDictionaryRequest.StreamId;

            return m_EnumDictionaryClose;
        }

        /// <summary>
        /// The DictionaryClose to be sent to close the Field Dictionary stream.
        /// This corresponds to the Field DictionaryRequest sent
        /// during the connection establishment process.
        /// </summary>
        /// <returns><see cref="DictionaryClose"/> message</returns>
        public DictionaryClose? FieldDictionaryClose()
        {
            if (RdmFieldDictionaryRequest == null)
            {
                return null;
            }

            if (m_FieldDictionaryClose == null)
            {
                m_FieldDictionaryClose = new DictionaryClose();
            }

            m_FieldDictionaryClose.StreamId = RdmFieldDictionaryRequest.StreamId;

            return m_FieldDictionaryClose;
        }

        /// <summary>
        /// Performs a deep copy from a specified ConsumerRole into this ConsumerRole.
        /// Only public facing attributes are copied.
        /// </summary>
        /// <param name="role">role to copy from</param>
        internal void Copy(ConsumerRole role)
        {
            base.Copy(role);
            CopyLoginRequest(role.RdmLoginRequest);
            LoginMsgCallback = role.LoginMsgCallback;
            CopyDirectoryRequest(role.RdmDirectoryRequest);
            DirectoryMsgCallback = role.DirectoryMsgCallback;
            DictionaryMsgCallback = role.DictionaryMsgCallback;
            DictionaryDownloadMode = role.DictionaryDownloadMode;
        }

        /// <summary>
        /// Performs a deep copy from a specified DirectoryRequest
        /// into the DirectoryRequest associated with this ConsumerRole
        /// </summary>
        /// <param name="source"></param>
        internal void CopyDirectoryRequest(DirectoryRequest? source)
        {
            if (source is not null)
            {
                if (m_DirectoryRequest is null)
                {
                    m_DirectoryRequest = new DirectoryRequest();
                }
                source.Copy(m_DirectoryRequest);
            }
        }

        /// <summary>
        /// Performs a deep copy from a specified LoginRequest
        /// into the LoginRequest associated with this ConsumerRole.
        /// </summary>
        /// <param name="loginRequest">source LoginRequest</param>
        void CopyLoginRequest(LoginRequest? loginRequest)
        {
            if (loginRequest != null)
            {
                if (loginRequest != null)
                {
                    if (m_LoginRequest == null)
                    {
                        m_LoginRequest = new LoginRequest();
                    }
                    loginRequest.Copy(m_LoginRequest);
                }
            }
        }


        /// <summary>
        /// Initializes the RDM LoginRequest with default information.
        /// </summary>
        ///
        /// <remarks>
        /// If the <see cref="RdmLoginRequest"/> has already been defined it will be reused.
        /// </remarks>
        public void InitDefaultRDMLoginRequest()
        {
            string userName = "";
            int streamId;

            if (m_LoginRequest is null)
            {
                streamId = LOGIN_STREAM_ID;
                m_LoginRequest = new LoginRequest();
            }
            else
            {
                streamId = (m_LoginRequest.StreamId == 0)
                    ? LOGIN_STREAM_ID
                    : m_LoginRequest.StreamId;
                userName = m_LoginRequest.UserName.ToString();
                m_LoginRequest.Clear();
            }

            // replaced with MsgClass, setup by the LoginRequest: RdmLoginRequest.MsgType = LoginMsgType.REQUEST;
            m_LoginRequest.InitDefaultRequest(streamId);
            m_LoginRequest.HasAttrib = true;
            if (!userName.Equals(""))
            {
                m_LoginRequest.UserName.Data(userName);
            }
            m_LoginRequest.HasRole = true;
            m_LoginRequest.Role = RoleTypes.CONS;

            return;
        }

        public void InitDefaultLoginRTT()
        {
            int streamId;

            if (RdmLoginRTT == null)
            {
                streamId = LOGIN_STREAM_ID;
                RdmLoginRTT = new();
            }
            else
            {
                streamId = (RdmLoginRTT.StreamId == 0 ? LOGIN_STREAM_ID : RdmLoginRTT.StreamId);
                RdmLoginRTT.Clear();
            }

            RdmLoginRTT.InitRTT(streamId);
        }
    }
}
