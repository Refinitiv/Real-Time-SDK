/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Rdm;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Class representing the role of an OMM Non-Interactive Provider.
    /// </summary>
    /// <see cref="ReactorRole"/>
    /// <see cref="ReactorRoleType"/>
    sealed public class NIProviderRole : ReactorRole
    {
        const int OPEN_LIMIT = 5;
        const string VENDOR = "Refinitiv";
        const string LINK_NAME = "NI_PUB";
        const string FIELD_DICTIONARY_NAME = "RWFFld";
        const string ENUM_TYPE_DICTIONARY_NAME = "RWFEnum";
        internal const int LOGIN_STREAM_ID = 1;
        internal const int DIRECTORY_STREAM_ID = -1;
        internal const int FIELD_DICTIONARY_STREAM_ID = -2;
        internal const int ENUM_DICTIONARY_STREAM_ID = -3;

        internal const long FILTER_TO_REFRESH = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.LOAD | ServiceFilterFlags.LINK;

        private LoginRequest? m_LoginRequest = null;
        private DirectoryRefresh? m_DirectoryRefresh = null;
        private DirectoryClose? m_DirectoryClose = null;
        private DictionaryClose? m_FieldDictionaryClose = null;
        private DictionaryClose? m_EnumDictionaryClose = null;

        private Buffer m_StateText = new Buffer();
        private Service m_Service = new Service();

        /// <summary>
        /// The <see cref="LoginRequest"/> to be sent during the connection establishment
        /// process.
        /// </summary>
        ///
        /// <remarks>
        /// This can be populated with a user's specific information or
        /// invoke <see cref="InitDefaultRDMLoginRequest()"/> to populate with default
        /// information. If this parameter is left empty no login will be sent to
        /// the system; useful for systems that do not require a login.
        /// </remarks>
        public LoginRequest? RdmLoginRequest { get => m_LoginRequest; set { CopyLoginRequest(value); } }

        /// <summary>
        /// Gets or sets a class which implements the <see cref="IRDMLoginMsgCallback"/> interface for processing
        /// <see cref="RDMLoginMsgEvent"/>s received. If not present, the received message will be passed
        /// to the <see cref="IDefaultMsgCallback"/>.
        /// </summary>
        public IRDMLoginMsgCallback? LoginMsgCallback { get; set; }

        /// <summary>
        /// A Directory Refresh to be sent during the setup of a Non-Interactive Provider session.
        /// This can be populated with a user's specific information or invoke
        /// <see cref="InitDefaultRDMDirectoryRefresh(string, int)"/> to populate with default information.
        /// Requires LoginRequest to be set.
        /// </summary>
        public DirectoryRefresh? RdmDirectoryRefresh { get => m_DirectoryRefresh; set { CopyDirectoryRefresh(value); } }

        /// <summary>
        /// The DirectoryClose to be sent to close the Directory stream.
        /// This corresponds to the DirectoryRefresh sent during the connection establishment process.
        /// </summary>
        public DirectoryClose? RdmDirectoryClose
        {
            get
            {
                if (m_DirectoryRefresh == null)
                {
                    return null;
                }

                if (m_DirectoryClose == null)
                {
                    m_DirectoryClose = new DirectoryClose();
                }

                m_DirectoryClose.StreamId = m_DirectoryRefresh.StreamId;

                return m_DirectoryClose;
            }
            set
            {
                m_DirectoryClose = value;
            }
        }

        /// <summary>
        /// Callback for propagating Dictionary message events
        /// </summary>
        public IDictionaryMsgCallback? DictionaryMsgCallback { get; set; } = null;

        /// <summary>
        /// A Field Dictionary Request to be sent during the setup of a Consumer-Provider session.
        /// Requires DirectoryRequest to be set.
        /// </summary>
        public DictionaryRequest? RdmFieldDictionaryRequest { get; set; }

        /// <summary>
        /// The DictionaryClose to be sent to close the Field Dictionary stream.
        /// This corresponds to the Field DictionaryRequest sent
        /// during the connection establishment process.
        /// </summary>
        public DictionaryClose? RdmFieldDictionaryClose
        {
            get
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
            set { m_FieldDictionaryClose = value; }
        }

        /// <summary>
        /// A EnumType Dictionary Request to be sent during the setup of a Consumer-Provider session.
        /// Requires Field DictionaryRequest to be set.
        /// </summary>
        public DictionaryRequest? RdmEnumDictionaryRequest { get; set; }

        /// <summary>
        /// The DictionaryClose to be sent to close the EnumType Dictionary stream.
        /// This corresponds to the EnumType DictionaryRequest sent
        /// during the connection establishment process.
        /// </summary>
        public DictionaryClose? RdmEnumDictionaryClose
        {
            get
            {
                if (RdmEnumDictionaryRequest == null)
                {
                    return null;
                }

                if (m_EnumDictionaryClose == null)
                {
                    m_EnumDictionaryClose = new DictionaryClose();
                }

                m_EnumDictionaryClose.StreamId = RdmEnumDictionaryRequest.StreamId;

                return m_EnumDictionaryClose;
            }
            set { m_EnumDictionaryClose = value; }
        }

        /// <summary>
        /// the Dictionary download mode
        /// </summary>
        public DictionaryDownloadMode DictionaryDownloadMode { get; set; } = DictionaryDownloadMode.NONE;

        /// <summary>
        /// FieldDictionary name
        /// </summary>
        public Buffer FieldDictionaryName { get; private set; } = new Buffer();

        /// <summary>
        /// EnumType dicitonary name
        /// </summary>
        public Buffer EnumTypeDictionaryName { get; private set; } = new Buffer();

        /// <summary>
        /// Indicates whether Field Dictionary responce was received
        /// </summary>
        public bool ReceivedFieldDictionaryResp { get; set; } = false;

        /// <summary>
        /// Indicates whether EnumType Dictionary response was received
        /// </summary>
        public bool ReceivedEnumDictionaryResp { get; set; } = false;

        /// <summary>
        /// Instantiates a new NI provider role.
        /// </summary>
        public NIProviderRole()
        {
            Type = ReactorRoleType.NIPROVIDER;
            FieldDictionaryName.Data(FIELD_DICTIONARY_NAME);
            EnumTypeDictionaryName.Data(ENUM_TYPE_DICTIONARY_NAME);
        }

        /// <summary>
        /// Performs a deep copy from a specified NIProviderRole into this NIProviderRole.
        /// Only public facing attributes are copied.
        /// </summary>
        /// <param name="role">role to copy from</param>
        internal void Copy(NIProviderRole role)
        {
            base.Copy(role);
            CopyLoginRequest(role.RdmLoginRequest);
            LoginMsgCallback = role.LoginMsgCallback;
            CopyDirectoryRefresh(role.RdmDirectoryRefresh);
            DictionaryMsgCallback = role.DictionaryMsgCallback;
            DictionaryDownloadMode = role.DictionaryDownloadMode;
        }

        /// <summary>
        /// Performs a deep copy from a specified DirectoryRefresh
        /// into the DirectoryRefresh associated with this NIProviderRole
        /// </summary>
        /// <param name="directoryRefresh">source DirectoryRefresh message</param>
        void CopyDirectoryRefresh(DirectoryRefresh? directoryRefresh)
        {
            if (directoryRefresh != null)
            {
                if (m_DirectoryRefresh == null)
                {
                    m_DirectoryRefresh = new DirectoryRefresh();
                }
                directoryRefresh.Copy(m_DirectoryRefresh);
            }
        }

        /// <summary>
        /// Performs a deep copy from a specified LoginRequest 
        /// into the LoginRequest associated with this NIProviderRole.
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

        /// <summary>Initializes the RDM LoginRequest with default information.</summary>
        ///
        /// <remarks>If the <see cref="RdmLoginRequest"/> has already been defined it will
        /// be reused.</remarks>
        public void InitDefaultRDMLoginRequest()
        {
            string userName = String.Empty;
            int streamId;

            if (RdmLoginRequest == null)
            {
                streamId = LOGIN_STREAM_ID;
                RdmLoginRequest = new();
            }
            else
            {
                streamId = (RdmLoginRequest.StreamId == 0)
                    ? LOGIN_STREAM_ID
                    : RdmLoginRequest.StreamId;
                userName = RdmLoginRequest.UserName.ToString();
                RdmLoginRequest.Clear();
            }

            // RdmLoginRequest.rdmMsgType(LoginMsgType.REQUEST);
            RdmLoginRequest.InitDefaultRequest(streamId);
            RdmLoginRequest.HasAttrib = true;
            if (!string.IsNullOrEmpty(userName))
            {
                RdmLoginRequest.UserName.Data(userName);
            }
            RdmLoginRequest.HasRole = true;
            RdmLoginRequest.Role = Login.RoleTypes.PROV;
        }

        /// <summary>
        /// Initializes the RDM DirectoryRefresh with default information.
        /// If the DirectoryRefresh has already been defined (due to a previous call to <see cref="RdmDirectoryRefresh"/>),
        /// the DirectoryRefresh object will be reused.
        /// </summary>
        /// <param name="serviceName">the serviceName for this source directory refresh</param>
        /// <param name="serviceId">the serviceId for this source directory refresh</param>
        public void InitDefaultRDMDirectoryRefresh(string serviceName, int serviceId)
        {
            int streamId;

            if (m_DirectoryRefresh == null)
            {
                streamId = DIRECTORY_STREAM_ID;
                m_DirectoryRefresh = new DirectoryRefresh();
            }
            else
            {
                streamId = m_DirectoryRefresh.StreamId == 0 ? DIRECTORY_STREAM_ID : m_DirectoryRefresh.StreamId;
                m_DirectoryRefresh.Clear();
            }

            // stream id
            m_DirectoryRefresh.StreamId = streamId;

            // state information
            m_DirectoryRefresh.State.StreamState(StreamStates.OPEN);
            m_DirectoryRefresh.State.DataState(DataStates.OK);
            m_DirectoryRefresh.State.Code(StateCodes.NONE);
            m_DirectoryRefresh.State.Text(m_StateText);

            //clear cache
            m_DirectoryRefresh.ClearCache = true;

            //attribInfo information
            m_DirectoryRefresh.Filter = FILTER_TO_REFRESH;

            //_service
            m_Service.Clear();
            m_Service.Action = MapEntryActions.ADD;

            //set the _service Id (map key)
            m_Service.ServiceId = serviceId;

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.INFO) != 0)
            {
                m_Service.HasInfo = true;
                m_Service.Info.Action = FilterEntryActions.SET;

                //vendor
                m_Service.Info.HasVendor = true;
                m_Service.Info.Vendor.Data(VENDOR);

                //_service name - required
                m_Service.Info.ServiceName.Data(serviceName);

                //Qos Range is not supported
                m_Service.Info.HasSupportQosRange = true;
                m_Service.Info.SupportsQosRange = 0;

                //capabilities - required
                m_Service.Info.CapabilitiesList.Add((long)DomainType.MARKET_PRICE);
                m_Service.Info.CapabilitiesList.Add((long)DomainType.MARKET_BY_ORDER);

                //qos
                m_Service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                m_Service.Info.QosList.Add(qos);

                //dictionary used
                m_Service.Info.HasDictionariesUsed = true;
                m_Service.Info.DictionariesUsedList.Add(FIELD_DICTIONARY_NAME);
                m_Service.Info.DictionariesUsedList.Add(ENUM_TYPE_DICTIONARY_NAME);

                //isSource = Service is provided directly from original publisher
                m_Service.Info.HasIsSource = true;
                m_Service.Info.IsSource = 1;

                // itemList - Name of SymbolList that includes all of the items that
                // the publisher currently provides. Blank for this example
                m_Service.Info.HasItemList = true;
                m_Service.Info.ItemList.Data("");

                m_Service.Info.HasAcceptingConsStatus = true;
                //accepting customer status = no
                m_Service.Info.AcceptConsumerStatus = 0;

                m_Service.Info.HasSupportOOBSnapshots = true;
                //supports out of band snapshots = no
                m_Service.Info.SupportsOOBSnapshots = 0;
            }

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.STATE) != 0)
            {
                m_Service.HasState = true;
                m_Service.State.Action = FilterEntryActions.SET;

                //_service state
                m_Service.State.ServiceStateVal = 1;

                //accepting requests
                m_Service.State.HasAcceptingRequests = true;
                m_Service.State.AcceptingRequests = 1;

                //status
                m_Service.State.HasStatus = true;
                m_Service.State.Status.DataState(DataStates.OK);
                m_Service.State.Status.StreamState(StreamStates.OPEN);
                m_Service.State.Status.Code(StateCodes.NONE);
                m_Service.State.Status.Text().Data("OK");
            }

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.LOAD) != 0)
            {
                m_Service.HasLoad = true;
                m_Service.Load.Action = FilterEntryActions.SET;

                //open limit
                m_Service.Load.HasOpenLimit = true;
                m_Service.Load.OpenLimit = OPEN_LIMIT;

                //load factor
                m_Service.Load.HasLoadFactor = true;
                m_Service.Load.LoadFactor = 1;
            }

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.LINK) != 0)
            {
                m_Service.HasLink = true;
                m_Service.Link.Action = FilterEntryActions.SET;

                ServiceLink serviceLink = new ServiceLink();

                //link name - Map Entry Key
                serviceLink.Name.Data(LINK_NAME);

                //link type
                serviceLink.HasType = true;
                serviceLink.Type = LinkTypes.INTERACTIVE;

                //link state
                serviceLink.LinkState = LinkStates.UP;

                //link code
                serviceLink.HasCode = true;
                serviceLink.LinkCode = LinkCodes.OK;

                //link text
                serviceLink.HasText = true;
                serviceLink.Text.Data("Link state is up");

                m_Service.Link.LinkList.Add(serviceLink);
            }

            m_DirectoryRefresh.ServiceList.Add(m_Service);
        }

        /// <summary>
        /// Initializes the RDM Field DictionaryRequest with default information.
        /// If the FieldDictionaryRequest has already been defined,
        /// the FieldDictionaryRequest object will be reused.
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
            while (RdmLoginRequest != null && streamId == RdmLoginRequest.StreamId
                || m_DirectoryRefresh != null && streamId == m_DirectoryRefresh.StreamId)
            {
                streamId--;
            }
            RdmFieldDictionaryRequest.StreamId = streamId;
            RdmFieldDictionaryRequest.Streaming = true;
            RdmFieldDictionaryRequest.Verbosity = Dictionary.VerbosityValues.NORMAL;
            RdmFieldDictionaryRequest.DictionaryName = FieldDictionaryName;

            return;
        }

        /// <summary>
        /// Initializes the RDM EnumType DictionaryRequest with default information.
        /// If the EnumDictionaryRequest has already been defined,
        /// the EnumDictionaryRequest object will be reused.
        /// </summary>
        internal void InitDefaultRDMEnumDictionaryRequest()
        {
            int streamId;

            if (RdmEnumDictionaryRequest == null)
            {
                streamId = ENUM_DICTIONARY_STREAM_ID;
                RdmEnumDictionaryRequest = new DictionaryRequest();
            }
            else
            {
                streamId = RdmEnumDictionaryRequest.StreamId == 0 ? ENUM_DICTIONARY_STREAM_ID : RdmEnumDictionaryRequest.StreamId;
                RdmEnumDictionaryRequest.Clear();
            }

            // make sure stream id isn't already being used
            while (RdmLoginRequest != null && streamId == RdmLoginRequest.StreamId
                || m_DirectoryRefresh != null && streamId == m_DirectoryRefresh.StreamId
                || RdmFieldDictionaryRequest != null && streamId == RdmFieldDictionaryRequest.StreamId)
            {
                streamId--;
            }

            RdmEnumDictionaryRequest.StreamId = streamId;
            RdmEnumDictionaryRequest.Streaming = true;
            RdmEnumDictionaryRequest.Verbosity = Dictionary.VerbosityValues.NORMAL;
            RdmEnumDictionaryRequest.DictionaryName = EnumTypeDictionaryName;

            return;
        }
    }
}
