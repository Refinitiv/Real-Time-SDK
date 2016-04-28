package com.thomsonreuters.upa.rdm;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.ElementEntry;

/**
 * Well known Element Names used by an {@link ElementEntry}.
 */
public class ElementNames
{
    // ElementNames class cannot be instantiated
    private ElementNames()
    {
        throw new AssertionError();
    }

    // RDMUser - Well known Element Names
    /** ApplicationId */
    public static final Buffer APPID = CodecFactory.createBuffer();
    /** ApplicationName */
    public static final Buffer APPNAME = CodecFactory.createBuffer();
    /** Position */
    public static final Buffer POSITION = CodecFactory.createBuffer();
    /** Password */
    public static final Buffer PASSWORD = CodecFactory.createBuffer();
    /** ProvidePermissionProfile */
    public static final Buffer PROV_PERM_PROF = CodecFactory.createBuffer();
    /** ProvidePermissionExpressions */
    public static final Buffer PROV_PERM_EXP = CodecFactory.createBuffer();
    /** AllowSuspectData */
    public static final Buffer ALLOW_SUSPECT_DATA = CodecFactory.createBuffer();
    /** SingleOpen */
    public static final Buffer SINGLE_OPEN = CodecFactory.createBuffer();
    /** SupportPauseResume */
    public static final Buffer SUPPORT_PR = CodecFactory.createBuffer();
    /** SupportOptimizedPauseResume */
    public static final Buffer SUPPORT_OPR = CodecFactory.createBuffer();
    /** SupportOMMPost */
    public static final Buffer SUPPORT_POST = CodecFactory.createBuffer();
    /** SupportBatchRequests */
    public static final Buffer SUPPORT_BATCH = CodecFactory.createBuffer();
    /** SupportViewRequests */
    public static final Buffer SUPPORT_VIEW = CodecFactory.createBuffer();
    /** SupportProviderDictionaryDownload*/
    public static final Buffer SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = CodecFactory.createBuffer();
    /** InstanceId */
    public static final Buffer INST_ID = CodecFactory.createBuffer();
    /** Role */
    public static final Buffer ROLE = CodecFactory.createBuffer();
    /** PersistentMount */
    public static final Buffer PERSISTENT_MOUNT = CodecFactory.createBuffer();

    // Warm Standby - Well known Element Names
    /** SupportStandby */
    public static final Buffer SUPPORT_STANDBY = CodecFactory.createBuffer();
    /** WarmStandbyInfo */
    public static final Buffer WARMSTANDBY_INFO = CodecFactory.createBuffer();
    /** WarmStandbyMode */
    public static final Buffer WARMSTANDBY_MODE = CodecFactory.createBuffer();
    /** ConsumerConnectionStatus */
    public static final Buffer CONS_CONN_STATUS = CodecFactory.createBuffer();

    // Connection Load Balancing - Well known Element Names
    /** DownloadConnectionConfig */
    public static final Buffer DOWNLOAD_CON_CONFIG = CodecFactory.createBuffer();
    /** ConnectionConfig */
    public static final Buffer CONNECTION_CONFIG = CodecFactory.createBuffer();
    /** NumStandbyServers */
    public static final Buffer NUM_STANDBY_SERVERS = CodecFactory.createBuffer();
    /** Hostname */
    public static final Buffer HOSTNAME = CodecFactory.createBuffer();
    /** Port */
    public static final Buffer PORT = CodecFactory.createBuffer();
    /** ServerType */
    public static final Buffer SERVER_TYPE = CodecFactory.createBuffer();
    /** SystemID */
    public static final Buffer SYSTEM_ID = CodecFactory.createBuffer();

    // RDMService - Well known Element Names
    /** RDMService Name */
    public static final Buffer NAME = CodecFactory.createBuffer();
    /** RDMService Vendor */
    public static final Buffer VENDOR = CodecFactory.createBuffer();
    /** IsSource */
    public static final Buffer IS_SOURCE = CodecFactory.createBuffer();
    /** Capabilities */
    public static final Buffer CAPABILITIES = CodecFactory.createBuffer();
    /** DictionariesProvided */
    public static final Buffer DICTIONARIES_PROVIDED = CodecFactory.createBuffer();
    /** DictionariesUsed */
    public static final Buffer DICTIONARIES_USED = CodecFactory.createBuffer();
    /** Qos */
    public static final Buffer QOS = CodecFactory.createBuffer();
    /** SupportsQoSRange */
    public static final Buffer SUPPS_QOS_RANGE = CodecFactory.createBuffer();
    /** ItemList */
    public static final Buffer ITEM_LIST = CodecFactory.createBuffer();
    /** SupportsOutOfBandSnapshots */
    public static final Buffer SUPPS_OOB_SNAPSHOTS = CodecFactory.createBuffer();
    /** AcceptingConsumerStatus */
    public static final Buffer ACCEPTING_CONS_STATUS = CodecFactory.createBuffer();
    /** SourceMirroringMode */
    public static final Buffer SOURCE_MIROR_MODE = CodecFactory.createBuffer();
    /** ConsumerStatus */
    public static final Buffer CONS_STATUS = CodecFactory.createBuffer();
    /** ServiceState */
    public static final Buffer SVC_STATE = CodecFactory.createBuffer();
    /** AcceptingRequests */
    public static final Buffer ACCEPTING_REQS = CodecFactory.createBuffer();
    /** Status */
    public static final Buffer STATUS = CodecFactory.createBuffer();
    /** Group */
    public static final Buffer GROUP = CodecFactory.createBuffer();
    /** MergedToGroup */
    public static final Buffer MERG_TO_GRP = CodecFactory.createBuffer();
    /** OpenLimit */
    public static final Buffer OPEN_LIMIT = CodecFactory.createBuffer();
    /** OpenWindow */
    public static final Buffer OPEN_WINDOW = CodecFactory.createBuffer();
    /** LoadFactor */
    public static final Buffer LOAD_FACT = CodecFactory.createBuffer();
    /** RDMService Type */
    public static final Buffer TYPE = CodecFactory.createBuffer();
    /** Data */
    public static final Buffer DATA = CodecFactory.createBuffer();
    /** LinkState */
    public static final Buffer LINK_STATE = CodecFactory.createBuffer();
    /** LinkCode */
    public static final Buffer LINK_CODE = CodecFactory.createBuffer();
    /** Text */
    public static final Buffer TEXT = CodecFactory.createBuffer();
    /** RDMService Version */
    public static final Buffer VERSION = CodecFactory.createBuffer();

    // Dictionary - Well known element names
    /** DictionaryId */
    public static final Buffer DICTIONARY_ID = CodecFactory.createBuffer();
    /** Dictionary Type */
    public static final Buffer DICT_TYPE = CodecFactory.createBuffer();
    /** Dictionary Version */
    public static final Buffer DICT_VERSION = CodecFactory.createBuffer();
    /** Field NAME */
    public static final Buffer FIELD_NAME = CodecFactory.createBuffer();
    /** FID */
    public static final Buffer FIELD_ID = CodecFactory.createBuffer();
    /** RIPPLETO */
    public static final Buffer FIELD_RIPPLETO = CodecFactory.createBuffer();
    /** TYPE */
    public static final Buffer FIELD_TYPE = CodecFactory.createBuffer();
    /** LENGTH */
    public static final Buffer FIELD_LENGTH = CodecFactory.createBuffer();
    /** RWFTYPE */
    public static final Buffer FIELD_RWFTYPE = CodecFactory.createBuffer();
    /** RWFLEN */
    public static final Buffer FIELD_RWFLEN = CodecFactory.createBuffer();
    /** ENUMLENGTH */
    public static final Buffer FIELD_ENUMLENGTH = CodecFactory.createBuffer();
    /** LONGNAME */
    public static final Buffer FIELD_LONGNAME = CodecFactory.createBuffer();

    // EnumType names
    /** FIDS */
    public static final Buffer ENUM_FIDS = CodecFactory.createBuffer();
    /** VALUE */
    public static final Buffer ENUM_VALUE = CodecFactory.createBuffer();
    /** DISPLAY */
    public static final Buffer ENUM_DISPLAY = CodecFactory.createBuffer();
    /** MEANING */
    public static final Buffer ENUM_MEANING = CodecFactory.createBuffer();

    // Enum Type Dictionary Tags
    /** RT_Version */
    public static final Buffer ENUM_RT_VERSION = CodecFactory.createBuffer();
    /** DT_Version */
    public static final Buffer ENUM_DT_VERSION = CodecFactory.createBuffer();
    
    // Field Set Definition names
    /** NUMENTRIES */
    public static final Buffer SET_NUMENTRIES = CodecFactory.createBuffer();
    /** FIDS */
    public static final Buffer SET_FIDS = CodecFactory.createBuffer();
    /** TYPES */
    public static final Buffer SET_TYPES = CodecFactory.createBuffer();
    /** NAMES */
    public static final Buffer SET_NAMES = CodecFactory.createBuffer();
    

    
    // Multicast info
    /** ReferenceDataServerHost */
    public static final Buffer REFERENCE_DATA_SERVER_HOST = CodecFactory.createBuffer();
    /** ReferenceDataServerPort */
    public static final Buffer REFERENCE_DATA_SERVER_PORT = CodecFactory.createBuffer();
    /** SnapshotServerHost */
    public static final Buffer SNAPSHOT_SERVER_HOST = CodecFactory.createBuffer();
    /** SnapshotServerPort */
    public static final Buffer SNAPSHOT_SERVER_PORT = CodecFactory.createBuffer();
    /** GapRecoveryServerHost */
    public static final Buffer GAP_RECOVERY_SERVER_HOST = CodecFactory.createBuffer();
    /** GapRecoveryServerPort */
    public static final Buffer GAP_RECOVERY_SERVER_PORT = CodecFactory.createBuffer();
    /** StreamingMulticastChannels */
    public static final Buffer STREAMING_MCAST_CHANNELS = CodecFactory.createBuffer();
    /** GapMulticastChannels */
    public static final Buffer GAP_MCAST_CHANNELS = CodecFactory.createBuffer();
    /** Address */
    public static final Buffer ADDRESS = CodecFactory.createBuffer();
    /** Domain */
    public static final Buffer DOMAIN = CodecFactory.createBuffer();
    /** MulticastGroup */
    public static final Buffer MULTICAST_GROUP = CodecFactory.createBuffer();
    /** Channel ID */
    public static final Buffer CHANNEL_ID = CodecFactory.createBuffer();

    // Request Message Payload - Well known Element Names
    // Because these span domains, they are namespaced
    // <namespace>:<element name>
    // Thomson Reuters claims empty namespace (e.g. :ItemList is TR namespace)
    // Customers can define and namespace using other values as they need
    /** :ItemList */
    public static final Buffer BATCH_ITEM_LIST = CodecFactory.createBuffer();
    /** :ViewType */
    public static final Buffer VIEW_TYPE = CodecFactory.createBuffer();
    /** :ViewData */
    public static final Buffer VIEW_DATA = CodecFactory.createBuffer();

    static
    {
        APPID.data("ApplicationId");
        /** ApplicationName */
        APPNAME.data("ApplicationName");
        /** Position */
        POSITION.data("Position");
        /** Password */
        PASSWORD.data("Password");
        /** ProvidePermissionProfile */
        PROV_PERM_PROF.data("ProvidePermissionProfile");
        /** ProvidePermissionExpressions */
        PROV_PERM_EXP.data("ProvidePermissionExpressions");
        /** AllowSuspectData */
        ALLOW_SUSPECT_DATA.data("AllowSuspectData");
        /** SingleOpen */
        SINGLE_OPEN.data("SingleOpen");
        /** SupportPauseResume */
        SUPPORT_PR.data("SupportPauseResume");
        /** SupportOptimizedPauseResume */
        SUPPORT_OPR.data("SupportOptimizedPauseResume");
        /** SupportOMMPost */
        SUPPORT_POST.data("SupportOMMPost");
        /** SupportBatchRequests */
        SUPPORT_BATCH.data("SupportBatchRequests");
        /** SupportViewRequests */
        SUPPORT_VIEW.data("SupportViewRequests");
        /** SupportProviderDictionaryDownload */
        SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD.data("SupportProviderDictionaryDownload");
        /** InstanceId */
        INST_ID.data("InstanceId");
        /** Role */
        ROLE.data("Role");
        /** PersistentMount */
        PERSISTENT_MOUNT.data("PersistentMount");

        // Warm Standby - Well known Element Names
        /** SupportStandby */
        SUPPORT_STANDBY.data("SupportStandby");
        /** WarmStandbyInfo */
        WARMSTANDBY_INFO.data("WarmStandbyInfo");
        /** WarmStandbyMode */
        WARMSTANDBY_MODE.data("WarmStandbyMode");
        /** ConsumerConnectionStatus */
        CONS_CONN_STATUS.data("ConsumerConnectionStatus");

        // Connection Load Balancing - Well known Element Names
        /** DownloadConnectionConfig */
        DOWNLOAD_CON_CONFIG.data("DownloadConnectionConfig");
        /** ConnectionConfig */
        CONNECTION_CONFIG.data("ConnectionConfig");
        /** NumStandbyServers */
        NUM_STANDBY_SERVERS.data("NumStandbyServers");
        /** Hostname */
        HOSTNAME.data("Hostname");
        /** Port */
        PORT.data("Port");
        /** ServerType */
        SERVER_TYPE.data("ServerType");
        /** SystemID */
        SYSTEM_ID.data("SystemID");

        // RDMService - Well known Element Names
        /** RDMService Name */
        NAME.data("Name");
        /** RDMService Vendor */
        VENDOR.data("Vendor");
        /** IsSource */
        IS_SOURCE.data("IsSource");
        /** Capabilities */
        CAPABILITIES.data("Capabilities");
        /** DictionariesProvided */
        DICTIONARIES_PROVIDED.data("DictionariesProvided");
        /** DictionariesUsed */
        DICTIONARIES_USED.data("DictionariesUsed");
        /** Qos */
        QOS.data("QoS");
        /** SupportsQoSRange */
        SUPPS_QOS_RANGE.data("SupportsQoSRange");
        /** ItemList */
        ITEM_LIST.data("ItemList");
        /** SupportsOutOfBandSnapshots */
        SUPPS_OOB_SNAPSHOTS.data("SupportsOutOfBandSnapshots");
        /** AcceptingConsumerStatus */
        ACCEPTING_CONS_STATUS.data("AcceptingConsumerStatus");
        /** SourceMirroringMode */
        SOURCE_MIROR_MODE.data("SourceMirroringMode");
        /** ConsumerStatus */
        CONS_STATUS.data("ConsumerStatus");
        /** ServiceState */
        SVC_STATE.data("ServiceState");
        /** AcceptingRequests */
        ACCEPTING_REQS.data("AcceptingRequests");
        /** Status */
        STATUS.data("Status");
        /** Group */
        GROUP.data("Group");
        /** MergedToGroup */
        MERG_TO_GRP.data("MergedToGroup");
        /** OpenLimit */
        OPEN_LIMIT.data("OpenLimit");
        /** OpenWindow */
        OPEN_WINDOW.data("OpenWindow");
        /** LoadFactor */
        LOAD_FACT.data("LoadFactor");
        /** RDMService Type */
        TYPE.data("Type");
        /** Data */
        DATA.data("Data");
        /** LinkState */
        LINK_STATE.data("LinkState");
        /** LinkCode */
        LINK_CODE.data("LinkCode");
        /** Text */
        TEXT.data("Text");
        /** RDMService Version */
        VERSION.data("Version");

        // Dictionary - Well known element names
        /** DictionaryId */
        DICTIONARY_ID.data("DictionaryId");
        /** Dictionary Type */
        DICT_TYPE.data("Type");
        /** Dictionary Version */
        DICT_VERSION.data("Version");
        /** Field NAME */
        FIELD_NAME.data("NAME");
        /** FID */
        FIELD_ID.data("FID");
        /** RIPPLETO */
        FIELD_RIPPLETO.data("RIPPLETO");
        /** TYPE */
        FIELD_TYPE.data("TYPE");
        /** LENGTH */
        FIELD_LENGTH.data("LENGTH");
        /** RWFTYPE */
        FIELD_RWFTYPE.data("RWFTYPE");
        /** RWFLEN */
        FIELD_RWFLEN.data("RWFLEN");
        /** ENUMLENGTH */
        FIELD_ENUMLENGTH.data("ENUMLENGTH");
        /** LONGNAME */
        FIELD_LONGNAME.data("LONGNAME");

        // EnumType names
        /** FIDS */
        ENUM_FIDS.data("FIDS");
        /** VALUE */
        ENUM_VALUE.data("VALUE");
        /** DISPLAY */
        ENUM_DISPLAY.data("DISPLAY");
        /** MEANING */
        ENUM_MEANING.data("MEANING");

        // Enum Type Dictionary Tags
        /** RT_Version */
        ENUM_RT_VERSION.data("RT_Version");
        /** DT_Version */
        ENUM_DT_VERSION.data("DT_Version");
        
        // Field Set Definition names
        /** NUMENTRIES */
        SET_NUMENTRIES.data("NUMENTRIES");
        /** FIDS */
        SET_FIDS.data("FIDS");
        /** TYPES */
        SET_TYPES.data("TYPES");
        /** NAMES */
        SET_NAMES.data("NAMES");
        
        // Multicast information
        /** ReferenceDataServerHost */
         REFERENCE_DATA_SERVER_HOST.data("ReferenceDataServerHost");
        /** ReferenceDataServerPort */
         REFERENCE_DATA_SERVER_PORT.data("ReferenceDataServerPort");
        /** SnapshotServerHost */
         SNAPSHOT_SERVER_HOST.data("SnapshotServerHost");
        /** SnapshotServerPort */
         SNAPSHOT_SERVER_PORT.data("SnapshotServerPort");
        /** GapRecoveryServerHost */
         GAP_RECOVERY_SERVER_HOST.data("GapRecoveryServerHost");
        /** GapRecoveryServerPort */
         GAP_RECOVERY_SERVER_PORT.data("GapRecoveryServerPort");
        /** StreamingMulticastChannels */
         STREAMING_MCAST_CHANNELS.data("StreamingMulticastChannels");
        /** GapMulticastChannels */
         GAP_MCAST_CHANNELS.data("GapMulticastChannels");
        /** Address */
         ADDRESS.data("Address");
        /** Domain */
         DOMAIN.data("Domain");
        /** MulticastGroup */
         MULTICAST_GROUP.data("MulticastGroup");
         /** Channel ID */
         CHANNEL_ID.data("ChannelId");


        // Request Message Payload - Well known Element Names
        // Because these span domains, they are namespaced
        // <namespace>:<element name>
        // Thomson Reuters claims empty namespace (e.g. :ItemList is TR
        // namespace)
        // Customers can define and namespace using other values as they need
        /** :ItemList */
        BATCH_ITEM_LIST.data(":ItemList");
        /** :ViewType */
        VIEW_TYPE.data(":ViewType");
        /** :ViewData */
        VIEW_DATA.data(":ViewData");

    }
}
