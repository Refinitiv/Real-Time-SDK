/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// Well known Element Names used by an <see cref="ElementEntry"/>.
    /// </summary>
    sealed public class ElementNames
	{
		// ElementNames class cannot be instantiated
		private ElementNames()
		{
            throw new System.NotImplementedException();
        }

		// RDMUser - Well known Element Names
		/// <summary>
		/// ApplicationId </summary>
		public static readonly Buffer APPID = new Buffer();
		/// <summary>
		/// ApplicationName </summary>
		public static readonly Buffer APPNAME = new Buffer();
		/// <summary>
		/// Position </summary>
		public static readonly Buffer POSITION = new Buffer();
		/// <summary>
		/// Password </summary>
		public static readonly Buffer PASSWORD = new Buffer();
		/// <summary>
		/// ProvidePermissionProfile </summary>
		public static readonly Buffer PROV_PERM_PROF = new Buffer();
		/// <summary>
		/// ProvidePermissionExpressions </summary>
		public static readonly Buffer PROV_PERM_EXP = new Buffer();
		/// <summary>
		/// AllowSuspectData </summary>
		public static readonly Buffer ALLOW_SUSPECT_DATA = new Buffer();
		/// <summary>
		/// SingleOpen </summary>
		public static readonly Buffer SINGLE_OPEN = new Buffer();
		/// <summary>
		/// SupportPauseResume </summary>
		public static readonly Buffer SUPPORT_PR = new Buffer();
		/// <summary>
		/// SupportOptimizedPauseResume </summary>
		public static readonly Buffer SUPPORT_OPR = new Buffer();
		/// <summary>
		/// SupportOMMPost </summary>
		public static readonly Buffer SUPPORT_POST = new Buffer();
		/// <summary>
		/// SupportBatchRequests </summary>
		public static readonly Buffer SUPPORT_BATCH = new Buffer();
		/// <summary>
		/// SupportViewRequests </summary>
		public static readonly Buffer SUPPORT_VIEW = new Buffer();
		/// <summary>
		/// SupportProviderDictionaryDownload </summary>
		public static readonly Buffer SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = new Buffer();
		/// <summary>
		/// InstanceId </summary>
		public static readonly Buffer INST_ID = new Buffer();
		/// <summary>
		/// Role </summary>
		public static readonly Buffer ROLE = new Buffer();
		/// <summary>
		/// PersistentMount </summary>
		public static readonly Buffer PERSISTENT_MOUNT = new Buffer();

		// Warm Standby - Well known Element Names
		/// <summary>
		/// SupportStandby </summary>
		public static readonly Buffer SUPPORT_STANDBY = new Buffer();
		/// <summary>
		/// WarmStandbyInfo </summary>
		public static readonly Buffer WARMSTANDBY_INFO = new Buffer();
		/// <summary>
		/// WarmStandbyMode </summary>
		public static readonly Buffer WARMSTANDBY_MODE = new Buffer();
		/// <summary>
		/// ConsumerConnectionStatus </summary>
		public static readonly Buffer CONS_CONN_STATUS = new Buffer();

		// Connection Load Balancing - Well known Element Names
		/// <summary>
		/// DownloadConnectionConfig </summary>
		public static readonly Buffer DOWNLOAD_CON_CONFIG = new Buffer();
		/// <summary>
		/// ConnectionConfig </summary>
		public static readonly Buffer CONNECTION_CONFIG = new Buffer();
		/// <summary>
		/// NumStandbyServers </summary>
		public static readonly Buffer NUM_STANDBY_SERVERS = new Buffer();
		/// <summary>
		/// Hostname </summary>
		public static readonly Buffer HOSTNAME = new Buffer();
		/// <summary>
		/// Port </summary>
		public static readonly Buffer PORT = new Buffer();
		/// <summary>
		/// ServerType </summary>
		public static readonly Buffer SERVER_TYPE = new Buffer();
		/// <summary>
		/// SystemID </summary>
		public static readonly Buffer SYSTEM_ID = new Buffer();

		// Authentication Support Feature - Well known Element Names
		/// <summary> AUTHN_TOKEN </summary>
		public static readonly Buffer AUTHN_TOKEN = new Buffer();
		/// <summary> AUTHN_EXTENDED </summary>
		public static readonly Buffer AUTHN_EXTENDED = new Buffer();
		/// <summary> AUTHN_TT_REISSUE  </summary>
		public static readonly Buffer AUTHN_TT_REISSUE  = new Buffer();
		/// <summary> AUTHN_EXTENDED_RESP </summary>
		public static readonly Buffer AUTHN_EXTENDED_RESP = new Buffer();
		/// <summary> AUTHN_ERROR_CODE </summary>
		public static readonly Buffer AUTHN_ERROR_CODE = new Buffer();
		/// <summary> AUTHN_ERROR_TEXT </summary>
		public static readonly Buffer AUTHN_ERROR_TEXT = new Buffer();

		// RDMService - Well known Element Names
		/// <summary>
		/// RDMService Name </summary>
		public static readonly Buffer NAME = new Buffer();
		/// <summary>
		/// RDMService Vendor </summary>
		public static readonly Buffer VENDOR = new Buffer();
		/// <summary>
		/// IsSource </summary>
		public static readonly Buffer IS_SOURCE = new Buffer();
		/// <summary>
		/// Capabilities </summary>
		public static readonly Buffer CAPABILITIES = new Buffer();
		/// <summary>
		/// DictionariesProvided </summary>
		public static readonly Buffer DICTIONARIES_PROVIDED = new Buffer();
		/// <summary>
		/// DictionariesUsed </summary>
		public static readonly Buffer DICTIONARIES_USED = new Buffer();
		/// <summary>
		/// Qos </summary>
		public static readonly Buffer QOS = new Buffer();
		/// <summary>
		/// SupportsQoSRange </summary>
		public static readonly Buffer SUPPS_QOS_RANGE = new Buffer();
		/// <summary>
		/// ItemList </summary>
		public static readonly Buffer ITEM_LIST = new Buffer();
		/// <summary>
		/// SupportsOutOfBandSnapshots </summary>
		public static readonly Buffer SUPPS_OOB_SNAPSHOTS = new Buffer();
		/// <summary>
		/// AcceptingConsumerStatus </summary>
		public static readonly Buffer ACCEPTING_CONS_STATUS = new Buffer();
		/// <summary>
		/// SourceMirroringMode </summary>
		public static readonly Buffer SOURCE_MIRROR_MODE = new Buffer();
		/// <summary>
		/// ConsumerStatus </summary>
		public static readonly Buffer CONS_STATUS = new Buffer();
		/// <summary>
		/// ServiceState </summary>
		public static readonly Buffer SVC_STATE = new Buffer();
		/// <summary>
		/// AcceptingRequests </summary>
		public static readonly Buffer ACCEPTING_REQS = new Buffer();
		/// <summary>
		/// Status </summary>
		public static readonly Buffer STATUS = new Buffer();
		/// <summary>
		/// Group </summary>
		public static readonly Buffer GROUP = new Buffer();
		/// <summary>
		/// MergedToGroup </summary>
		public static readonly Buffer MERG_TO_GRP = new Buffer();
		/// <summary>
		/// OpenLimit </summary>
		public static readonly Buffer OPEN_LIMIT = new Buffer();
		/// <summary>
		/// OpenWindow </summary>
		public static readonly Buffer OPEN_WINDOW = new Buffer();
		/// <summary>
		/// LoadFactor </summary>
		public static readonly Buffer LOAD_FACT = new Buffer();
		/// <summary>
		/// RDMService Type </summary>
		public static readonly Buffer TYPE = new Buffer();
		/// <summary>
		/// Data </summary>
		public static readonly Buffer DATA = new Buffer();
		/// <summary>
		/// LinkState </summary>
		public static readonly Buffer LINK_STATE = new Buffer();
		/// <summary>
		/// LinkCode </summary>
		public static readonly Buffer LINK_CODE = new Buffer();
		/// <summary>
		/// Text </summary>
		public static readonly Buffer TEXT = new Buffer();
		/// <summary>
		/// RDMService Version </summary>
		public static readonly Buffer VERSION = new Buffer();

		// Dictionary - Well known element names
		/// <summary>
		/// DictionaryId </summary>
		public static readonly Buffer DICTIONARY_ID = new Buffer();
		/// <summary>
		/// Dictionary Type </summary>
		public static readonly Buffer DICT_TYPE = new Buffer();
		/// <summary>
		/// Dictionary Version </summary>
		public static readonly Buffer DICT_VERSION = new Buffer();
		/// <summary>
		/// Field NAME </summary>
		public static readonly Buffer FIELD_NAME = new Buffer();
		/// <summary>
		/// FID </summary>
		public static readonly Buffer FIELD_ID = new Buffer();
		/// <summary>
		/// RIPPLETO </summary>
		public static readonly Buffer FIELD_RIPPLETO = new Buffer();
		/// <summary>
		/// TYPE </summary>
		public static readonly Buffer FIELD_TYPE = new Buffer();
		/// <summary>
		/// LENGTH </summary>
		public static readonly Buffer FIELD_LENGTH = new Buffer();
		/// <summary>
		/// RWFTYPE </summary>
		public static readonly Buffer FIELD_RWFTYPE = new Buffer();
		/// <summary>
		/// RWFLEN </summary>
		public static readonly Buffer FIELD_RWFLEN = new Buffer();
		/// <summary>
		/// ENUMLENGTH </summary>
		public static readonly Buffer FIELD_ENUMLENGTH = new Buffer();
		/// <summary>
		/// LONGNAME </summary>
		public static readonly Buffer FIELD_LONGNAME = new Buffer();

		// EnumType names
		/// <summary>
		/// FIDS </summary>
		public static readonly Buffer ENUM_FIDS = new Buffer();
		/// <summary>
		/// VALUE </summary>
		public static readonly Buffer ENUM_VALUE = new Buffer();
		/// <summary>
		/// DISPLAY </summary>
		public static readonly Buffer ENUM_DISPLAY = new Buffer();
		/// <summary>
		/// MEANING </summary>
		public static readonly Buffer ENUM_MEANING = new Buffer();

		// Enum Type Dictionary Tags
		/// <summary>
		/// RT_Version </summary>
		public static readonly Buffer ENUM_RT_VERSION = new Buffer();
		/// <summary>
		/// DT_Version </summary>
		public static readonly Buffer ENUM_DT_VERSION = new Buffer();

		// Field Set Definition names
		/// <summary>
		/// NUMENTRIES </summary>
		public static readonly Buffer SET_NUMENTRIES = new Buffer();
		/// <summary>
		/// FIDS </summary>
		public static readonly Buffer SET_FIDS = new Buffer();
		/// <summary>
		/// TYPES </summary>
		public static readonly Buffer SET_TYPES = new Buffer();
		/// <summary>
		/// NAMES </summary>
		public static readonly Buffer SET_NAMES = new Buffer();

		// Multicast info
		/// <summary>
		/// ReferenceDataServerHost </summary>
		public static readonly Buffer REFERENCE_DATA_SERVER_HOST = new Buffer();
		/// <summary>
		/// ReferenceDataServerPort </summary>
		public static readonly Buffer REFERENCE_DATA_SERVER_PORT = new Buffer();
		/// <summary>
		/// SnapshotServerHost </summary>
		public static readonly Buffer SNAPSHOT_SERVER_HOST = new Buffer();
		/// <summary>
		/// SnapshotServerPort </summary>
		public static readonly Buffer SNAPSHOT_SERVER_PORT = new Buffer();
		/// <summary>
		/// GapRecoveryServerHost </summary>
		public static readonly Buffer GAP_RECOVERY_SERVER_HOST = new Buffer();
		/// <summary>
		/// GapRecoveryServerPort </summary>
		public static readonly Buffer GAP_RECOVERY_SERVER_PORT = new Buffer();
		/// <summary>
		/// StreamingMulticastChannels </summary>
		public static readonly Buffer STREAMING_MCAST_CHANNELS = new Buffer();
		/// <summary>
		/// GapMulticastChannels </summary>
		public static readonly Buffer GAP_MCAST_CHANNELS = new Buffer();
		/// <summary>
		/// Address </summary>
		public static readonly Buffer ADDRESS = new Buffer();
		/// <summary>
		/// Domain </summary>
		public static readonly Buffer DOMAIN = new Buffer();
		/// <summary>
		/// MulticastGroup </summary>
		public static readonly Buffer MULTICAST_GROUP = new Buffer();
		/// <summary>
		/// Channel ID </summary>
		public static readonly Buffer CHANNEL_ID = new Buffer();

		/// <summary>
		/// RoundTripLatency
		/// </summary>
		public static readonly Buffer ROUND_TRIP_LATENCY = new Buffer();

		/// <summary>
		/// Ticks
		/// </summary>
		public static readonly Buffer TICKS = new Buffer();

		/// <summary>
		/// TCP Retransmissions
		/// </summary>
		public static Buffer TCP_RETRANS = new Buffer();

		// Request Message Payload - Well known Element Names
		// Because these span domains, they are namespaced
		// <namespace>:<element name>
		// Thomson Reuters claims empty namespace (e.g. :ItemList is TR namespace)
		// Customers can define and namespace using other values as they need
		/// <summary>
		/// :ItemList </summary>
		public static readonly Buffer BATCH_ITEM_LIST = new Buffer();
		/// <summary>
		/// :ViewType </summary>
		public static readonly Buffer VIEW_TYPE = new Buffer();
		/// <summary>
		/// :ViewData </summary>
		public static readonly Buffer VIEW_DATA = new Buffer();

		static ElementNames()
		{
            // ApplicationId
            APPID.Data("ApplicationId");

			// ApplicationName
			APPNAME.Data("ApplicationName");

			// Position
			POSITION.Data("Position");
			
            // Password
			PASSWORD.Data("Password");
			
            // ProvidePermissionProfile
			PROV_PERM_PROF.Data("ProvidePermissionProfile");
			
            // ProvidePermissionExpressions
			PROV_PERM_EXP.Data("ProvidePermissionExpressions");
			
            // AllowSuspectData
			ALLOW_SUSPECT_DATA.Data("AllowSuspectData");
		    
            // SingleOpen
			SINGLE_OPEN.Data("SingleOpen");
			
            // SupportPauseResume
			SUPPORT_PR.Data("SupportPauseResume");
			
            // SupportOptimizedPauseResume
			SUPPORT_OPR.Data("SupportOptimizedPauseResume");

			// SupportOMMPost
			SUPPORT_POST.Data("SupportOMMPost");
			
            // SupportBatchRequests
			SUPPORT_BATCH.Data("SupportBatchRequests");
			
            // SupportViewRequests
			SUPPORT_VIEW.Data("SupportViewRequests");
			
            // SupportProviderDictionaryDownload
			SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD.Data("SupportProviderDictionaryDownload");
			
            // InstanceId
			INST_ID.Data("InstanceId");
			
            // Role
			ROLE.Data("Role");
			
            // PersistentMount
			PERSISTENT_MOUNT.Data("PersistentMount");

			// Warm Standby - Well known Element Names
			SUPPORT_STANDBY.Data("SupportStandby");
			
            // WarmStandbyInfo
			WARMSTANDBY_INFO.Data("WarmStandbyInfo");
			
            // WarmStandbyMode
			WARMSTANDBY_MODE.Data("WarmStandbyMode");
			
            // ConsumerConnectionStatus
			CONS_CONN_STATUS.Data("ConsumerConnectionStatus");

			// Connection Load Balancing - Well known Element Names
			DOWNLOAD_CON_CONFIG.Data("DownloadConnectionConfig");
			
            // ConnectionConfig
			CONNECTION_CONFIG.Data("ConnectionConfig");
			
            // NumStandbyServers
			NUM_STANDBY_SERVERS.Data("NumStandbyServers");
			
            // Hostname
			HOSTNAME.Data("Hostname");
			
            // Port
			PORT.Data("Port");
			
            // ServerType
			SERVER_TYPE.Data("ServerType");
			
            // SystemID
			SYSTEM_ID.Data("SystemID");

            // Authentication Support Feature - Well known Element Names
            /* AUTHN_TOKEN */
            AUTHN_TOKEN.Data("AuthenticationToken");
            
            /* AUTHN_EXTENDED */
            AUTHN_EXTENDED.Data("AuthenticationExtended");
            
            /* AUTHN_TT_REISSUE  */
            AUTHN_TT_REISSUE.Data("AuthenticationTTReissue");
            
            /* AUTHN_EXTENDED_RESP */
            AUTHN_EXTENDED_RESP.Data("AuthenticationExtendedResp");
            
            /* AUTHN_ERROR_CODE */
            AUTHN_ERROR_CODE.Data("AuthenticationErrorCode");
            
            /* AUTHN_ERROR_TEXT */
            AUTHN_ERROR_TEXT.Data("AuthenticationErrorText");

            // RDMService - Well known Element Names
            NAME.Data("Name");
			
            // RDMService Vendor
			VENDOR.Data("Vendor");
			
            // IsSource
			IS_SOURCE.Data("IsSource");
			
            // Capabilities
			CAPABILITIES.Data("Capabilities");
			
            // DictionariesProvided
			DICTIONARIES_PROVIDED.Data("DictionariesProvided");
			
            // DictionariesUsed
			DICTIONARIES_USED.Data("DictionariesUsed");
			
            // Qos
			QOS.Data("QoS");
			
            // SupportsQoSRange 
			SUPPS_QOS_RANGE.Data("SupportsQoSRange");
			
            // ItemList 
			ITEM_LIST.Data("ItemList");
			
            // SupportsOutOfBandSnapshots 
			SUPPS_OOB_SNAPSHOTS.Data("SupportsOutOfBandSnapshots");
			
            // AcceptingConsumerStatus 
			ACCEPTING_CONS_STATUS.Data("AcceptingConsumerStatus");
			
            // SourceMirroringMode 
			SOURCE_MIRROR_MODE.Data("SourceMirroringMode");
			
            // ConsumerStatus 
			CONS_STATUS.Data("ConsumerStatus");
			
            // ServiceState 
			SVC_STATE.Data("ServiceState");
			
            // AcceptingRequests 
			ACCEPTING_REQS.Data("AcceptingRequests");
			
            // Status 
			STATUS.Data("Status");
			
            // Group 
			GROUP.Data("Group");
			
            // MergedToGroup 
			MERG_TO_GRP.Data("MergedToGroup");
			
            // OpenLimit 
			OPEN_LIMIT.Data("OpenLimit");
			
            // OpenWindow 
			OPEN_WINDOW.Data("OpenWindow");
			
            // LoadFactor 
			LOAD_FACT.Data("LoadFactor");
			
            // RDMService Type 
			TYPE.Data("Type");
			
            // Data 
			DATA.Data("Data");
			
            // LinkState 
			LINK_STATE.Data("LinkState");
			
            // LinkCode 
			LINK_CODE.Data("LinkCode");
			
            // Text 
			TEXT.Data("Text");
			
            // RDMService Version 
			VERSION.Data("Version");

			// Dictionary - Well known element names
			DICTIONARY_ID.Data("DictionaryId");
			
            // Dictionary Type
			DICT_TYPE.Data("Type");
			
            // Dictionary Version 
			DICT_VERSION.Data("Version");
			
            // Field NAME 
			FIELD_NAME.Data("NAME");
			
            // FID 
			FIELD_ID.Data("FID");
			
            // RIPPLETO 
			FIELD_RIPPLETO.Data("RIPPLETO");
			
            // TYPE 
			FIELD_TYPE.Data("TYPE");
			
            // LENGTH 
			FIELD_LENGTH.Data("LENGTH");
			
            // RWFTYPE 
			FIELD_RWFTYPE.Data("RWFTYPE");
			
            // RWFLEN 
			FIELD_RWFLEN.Data("RWFLEN");
			
            // ENUMLENGTH 
			FIELD_ENUMLENGTH.Data("ENUMLENGTH");
			
            // LONGNAME 
			FIELD_LONGNAME.Data("LONGNAME");

			// FIDS 
			ENUM_FIDS.Data("FIDS");
			
            // VALUE 
			ENUM_VALUE.Data("VALUE");
			
            // DISPLAY 
			ENUM_DISPLAY.Data("DISPLAY");
			
            // MEANING 
			ENUM_MEANING.Data("MEANING");

			// RT_Version 
			ENUM_RT_VERSION.Data("RT_Version");
			
            // DT_Version 
			ENUM_DT_VERSION.Data("DT_Version");

			// NUMENTRIES 
			SET_NUMENTRIES.Data("NUMENTRIES");
			
            // FIDS 
			SET_FIDS.Data("FIDS");
			
            // TYPES 
			SET_TYPES.Data("TYPES");
			
            // NAMES 
			SET_NAMES.Data("NAMES");

			// Multicast information
			// ReferenceDataServerHost
			 REFERENCE_DATA_SERVER_HOST.Data("ReferenceDataServerHost");
			
            // ReferenceDataServerPort 
			 REFERENCE_DATA_SERVER_PORT.Data("ReferenceDataServerPort");
			
            // SnapshotServerHost 
			 SNAPSHOT_SERVER_HOST.Data("SnapshotServerHost");
			
            // SnapshotServerPort 
			 SNAPSHOT_SERVER_PORT.Data("SnapshotServerPort");
			
            // GapRecoveryServerHost 
			 GAP_RECOVERY_SERVER_HOST.Data("GapRecoveryServerHost");
			
            // GapRecoveryServerPort 
			 GAP_RECOVERY_SERVER_PORT.Data("GapRecoveryServerPort");
			
            // StreamingMulticastChannels 
			 STREAMING_MCAST_CHANNELS.Data("StreamingMulticastChannels");
			
            // GapMulticastChannels 
			 GAP_MCAST_CHANNELS.Data("GapMulticastChannels");
			
            // Address 
			 ADDRESS.Data("Address");
			
            // Domain 
			 DOMAIN.Data("Domain");
			
            // MulticastGroup 
			 MULTICAST_GROUP.Data("MulticastGroup");
			 
            // Channel ID 
			 CHANNEL_ID.Data("ChannelId");

			// RoundTripLatency
			ROUND_TRIP_LATENCY.Data("RoundTripLatency");
			// Ticks
			TICKS.Data("Ticks");
			// TCP Retransmissions
			TCP_RETRANS.Data("TcpRetrans");

			// Request Message Payload - Well known Element Names
			// Because these span domains, they are namespaced
			// <namespace>:<element name>
			// Thomson Reuters claims empty namespace (e.g. :ItemList is TR namespace)
			// Customers can define and namespace using other values as they need
			// :ItemList 
			BATCH_ITEM_LIST.Data(":ItemList");
			
            // :ViewType 
			VIEW_TYPE.Data(":ViewType");
			
            // :ViewData 
			VIEW_DATA.Data(":ViewData");
		}
	}
}
