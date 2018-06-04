package com.thomsonreuters.upa.codec;

/**
 * State Codes provide additional details about the current state.
 * 
 * @see State
 */
public class StateCodes
{
    // StateCodes class cannot be instantiated
    private StateCodes()
    {
        throw new AssertionError();
    }

    /**
     * code is none. Additional state code information is not required nor present.
     */
    public static final int NONE = 0;

    /**
     * Indicates that requested information was not found, though it might be
     * available at a later time or through changing some parameters used in the request.
     */
    public static final int NOT_FOUND = 1;

    /**
     * Indicates that a timeout occurred somewhere in the system while processing requested data.
     */
    public static final int TIMEOUT = 2;

    /**
     * Indicates that the request was denied due to permissioning. Typically
     * indicates that the requesting user does not have permission to request on
     * the service, to receive requested data, or to receive data at the requested QoS.
     */
    public static final int NOT_ENTITLED = 3;

    /**
     * Indicates that the request includes an invalid or unrecognized parameter.
     * Specific information should be contained in the text.
     */
    public static final int INVALID_ARGUMENT = 4;

    /**
     * Indicates invalid usage within the system. Specific information should be
     * contained in the text.
     */
    public static final int USAGE_ERROR = 5;

    /**
     * Indicates the stream was preempted, possibly by a caching device.
     * Typically indicates the user has exceeded an item limit, whether specific
     * to the user or a component in the system. Relevant information should be
     * contained in the text.
     */
    public static final int PREEMPTED = 6;

    /**
     * Indicates that JIT conflation has started on the stream. User is notified
     * when JIT Conflation ends via a {@link #REALTIME_RESUMED} code.
     */
    public static final int JIT_CONFLATION_STARTED = 7;

    /**
     * Indicates that JIT Conflation on the stream has finished.
     */
    public static final int REALTIME_RESUMED = 8;

    /**
     * Indicates that a component is recovering due to a failover condition.
     * User is notified when recovery finishes via a {@link #FAILOVER_COMPLETED} code.
     */
    public static final int FAILOVER_STARTED = 9;

    /**
     * Indicates that recovery from a failover condition has finished.
     */
    public static final int FAILOVER_COMPLETED = 10;

    /**
     * Indicates a gap was detected between messages. A gap might be detected
     * via an external reliability mechanism (e.g., transport) or using the
     * seqNum present in UPA messages.
     */
    public static final int GAP_DETECTED = 11;

    /**
     * Indicates that no resources are available to accommodate the stream.
     */
    public static final int NO_RESOURCES = 12;

    /**
     * Indicates that a request cannot be processed because too many other streams are already open.
     */
    public static final int TOO_MANY_ITEMS = 13;

    /**
     * Indicates that a stream is already open on the connection for the requested data.
     */
    public static final int ALREADY_OPEN = 14;

    /**
     * Indicates that the requested service is not known, though the service
     * might be available at a later point in time.
     */
    public static final int SOURCE_UNKNOWN = 15;

    /**
     * Indicates that the stream was not opened.
     * Additional information should be available in the text.
     */
    public static final int NOT_OPEN = 16;

    /* Reserved */
    static final int RESERVED17 = 17;

    /* Reserved */
    static final int RESERVED18 = 18;

    /** Indicates that a streaming request was made for non-updating data. */
    public static final int NON_UPDATING_ITEM = 19;

    /**
     * Indicates that the domain on which a request is made does not support the requested viewType.
     */
    public static final int UNSUPPORTED_VIEW_TYPE = 20;

    /**
     * code indicates that the requested view is invalid, possibly due to bad formatting.
     * Additional information should be available in the text.
     */
    public static final int INVALID_VIEW = 21;

    /**
     * Indicates that the full view (e.g., all available fields) is being
     * provided, even though only a specific view was requested.
     */
    public static final int FULL_VIEW_PROVIDED = 22;

    /**
     * Indicates that a batch request cannot be used for this request.
     * The user can instead split the batched items into individual requests.
     */
    public static final int UNABLE_TO_REQUEST_AS_BATCH = 23;
    
    /* Reserved */
    static final int RESERVED24 = 24;

    /* Reserved */
    static final int RESERVED25 = 25;

    /**
     * Request does not support batch and view
     */
    public static final int NO_BATCH_VIEW_SUPPORT_IN_REQ = 26;

    /**
     * Login rejected, exceeded maximum number of mounts per user
     */
    public static final int EXCEEDED_MAX_MOUNTS_PER_USER = 27;

    /**
     * Internal error from sender.
     */
    public static final int ERROR = 28;

    /**
     * A21: Connection to DACS down, users are not allowed to connect
     */
    public static final int DACS_DOWN = 29;

    /**
     * User unknown to permissioning system, it could be DACS, AAA or EED
     */
    public static final int USER_UNKNOWN_TO_PERM_SYS = 30;

    /**
     * Maximum logins reached.
     */
    public static final int DACS_MAX_LOGINS_REACHED = 31;

    /**
     * Application is denied access to the system
     */
    public static final int DACS_USER_ACCESS_TO_APP_DENIED = 32;
    
    /* Reserved */
    static final int RESERVED33 = 33;

    /**
     * Content is intended to fill a recognized gap
     */
    public static final int GAP_FILL = 34;
    
    /**
     * Application Authorization Failed
     */
    public static final int APP_AUTHORIZATION_FAILED = 35;

    /* Max reserved value */
    static final int MAX_RESERVED = 127;

    /**
     * Provide string representation for a state code.
     * 
     * @param stateCode {@link StateCodes} enumeration to convert to string
     * 
     * @return string representation for a state code
     */
    public static String toString(int stateCode)
    {
        switch (stateCode)
        {
            case NONE:
                return "NONE";
            case TIMEOUT:
                return "TIMEOUT";
            case NOT_ENTITLED:
                return "NOT_ENTITLED";
            case INVALID_ARGUMENT:
                return "INVALID_ARGUMENT";
            case USAGE_ERROR:
                return "USAGE_ERROR";
            case PREEMPTED:
                return "PREEMPTED";
            case JIT_CONFLATION_STARTED:
                return "JIT_CONFLATION_STARTED";
            case REALTIME_RESUMED:
                return "REALTIME_RESUMED";
            case FAILOVER_STARTED:
                return "FAILOVER_STARTED";
            case FAILOVER_COMPLETED:
                return "FAILOVER_COMPLETED";
            case GAP_DETECTED:
                return "GAP_DETECTED";
            case NO_RESOURCES:
                return "NO_RESOURCES";
            case TOO_MANY_ITEMS:
                return "TOO_MANY_ITEMS";
            case ALREADY_OPEN:
                return "ALREADY_OPEN";
            case SOURCE_UNKNOWN:
                return "SOURCE_UNKNOWN";
            case NOT_OPEN:
                return "NOT_OPEN";
            case NON_UPDATING_ITEM:
                return "NON_UPDATING_ITEM";
            case UNSUPPORTED_VIEW_TYPE:
                return "UNSUPPORTED_VIEW_TYPE";
            case INVALID_VIEW:
                return "INVALID_VIEW";
            case FULL_VIEW_PROVIDED:
                return "FULL_VIEW_PROVIDED";
            case UNABLE_TO_REQUEST_AS_BATCH:
                return "UNABLE_TO_REQUEST_AS_BATCH";
            case NOT_FOUND:
                return "NOT_FOUND";
            case NO_BATCH_VIEW_SUPPORT_IN_REQ:
                return "NO_BATCH_VIEW_SUPPORT_IN_REQ";
            case EXCEEDED_MAX_MOUNTS_PER_USER:
                return "EXCEEDED_MAX_MOUNTS_PER_USER";
            case ERROR:
                return "ERROR";
            case DACS_DOWN:
                return "DACS_DOWN";
            case USER_UNKNOWN_TO_PERM_SYS:
                return "USER_UNKNOWN_TO_PERM_SYS";
            case DACS_MAX_LOGINS_REACHED:
                return "DACS_MAX_LOGINS_REACHED";
            case DACS_USER_ACCESS_TO_APP_DENIED:
                return "DACS_USER_ACCESS_TO_APP_DENIED";
            case GAP_FILL:
                return "GAP_FILL";
            case APP_AUTHORIZATION_FAILED:
                return "APP_AUTHORIZATION_FAILED";
            default:
                return Integer.toString(stateCode);
        }
    }

    /**
     * Provide string representation of meaning associated with state code.
     * 
     * @param stateCode {@link StateCodes} enumeration to get info for
     * 
     * @return string representation of meaning associated with state code
     */
    public static String info(int stateCode)
    {
        switch (stateCode)
        {
            case StateCodes.NONE:
                return "None";
            case StateCodes.NOT_FOUND:
                return "Not found";
            case StateCodes.TIMEOUT:
                return "Timeout";
            case StateCodes.NOT_ENTITLED:
                return "Not entitled";
            case StateCodes.INVALID_ARGUMENT:
                return "Invalid argument";
            case StateCodes.USAGE_ERROR:
                return "Usage error";
            case StateCodes.PREEMPTED:
                return "Preempted";
            case StateCodes.JIT_CONFLATION_STARTED:
                return "JIT conflation started";
            case StateCodes.REALTIME_RESUMED:
                return "Realtime resumed";
            case StateCodes.FAILOVER_STARTED:
                return "Failover started";
            case StateCodes.FAILOVER_COMPLETED:
                return "Failover completed";
            case StateCodes.GAP_DETECTED:
                return "Gap detected";
            case StateCodes.NO_RESOURCES:
                return "No resources";
            case StateCodes.TOO_MANY_ITEMS:
                return "Too many items";
            case StateCodes.ALREADY_OPEN:
                return "Already open";
            case StateCodes.SOURCE_UNKNOWN:
                return "Source unknown";
            case StateCodes.NOT_OPEN:
                return "Not open";
            case StateCodes.NON_UPDATING_ITEM:
                return "Non-updating item";
            case StateCodes.UNSUPPORTED_VIEW_TYPE:
                return "Unsupported view type";
            case StateCodes.INVALID_VIEW:
                return "Invalid view";
            case StateCodes.FULL_VIEW_PROVIDED:
                return "Full view provided";
            case StateCodes.UNABLE_TO_REQUEST_AS_BATCH:
                return "Unable to request as batch";
            case NO_BATCH_VIEW_SUPPORT_IN_REQ:
                return "Request does not support batch and view";
            case EXCEEDED_MAX_MOUNTS_PER_USER:
                return "Login rejected, exceeded maximum number of mounts per user";
            case ERROR:
                return "Internal error from sender";
            case DACS_DOWN:
                return "A21: Connection to DACS down, users are not allowed to connect";
            case USER_UNKNOWN_TO_PERM_SYS:
                return "User unknown to permissioning system, it could be DACS, AAA or EED";
            case DACS_MAX_LOGINS_REACHED:
                return "Maximum logins reached";
            case DACS_USER_ACCESS_TO_APP_DENIED:
                return "Application is denied access to the system";
            case GAP_FILL:
                return "Content is intended";
            case APP_AUTHORIZATION_FAILED:
                return "Application Authorization failed";
            default:
                return "";
        }
    }
}


