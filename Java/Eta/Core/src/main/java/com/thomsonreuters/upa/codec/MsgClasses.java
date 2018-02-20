package com.thomsonreuters.upa.codec;

/**
 * Identifies the specific type of a message (For example, {@link UpdateMsg}, {@link RequestMsg} etc).
 * 
 * @see Msg
 */
public class MsgClasses
{
    /**
     * This class is not instantiated
     */
    private MsgClasses()
    {
        throw new AssertionError();
    }

    /**
     * Consumers use {@link RequestMsg} to express interest in a new stream or
     * modify some parameters on an existing stream; typically results in the
     * delivery of a {@link RefreshMsg} or {@link StatusMsg}.
     */
    public static final int REQUEST = 1;

    /**
     * The Interactive Provider can use this class to respond to a consumer's
     * request for information (solicited) or provide a data resynchronization
     * point (unsolicited). The NIP can use this class to initiate a data flow
     * on a new item stream. Conveys state information, QoS, stream
     * permissioning information, and group information in addition to payload.
     */
    public static final int REFRESH = 2;

    /**
     * Indicates changes to the stream or data properties. A provider uses
     * {@link StatusMsg} to close streams and to indicate successful
     * establishment of a stream when there is no data to convey.
     * This message can indicate changes:
     * <ul>
     * <li>In streamState or dataState</li>
     * <li>In a stream's permissioning information</li>
     * <li>To the item group to which the stream belongs</li>
     * </ul>
     */
    public static final int STATUS = 3;

    /**
     * Interactive or NIPs use {@link UpdateMsg} to convey changes to information on a stream.
     * Update messages typically flow on a stream after delivery of a refresh
     */
    public static final int UPDATE = 4;

    /**
     * A consumer uses {@link CloseMsg} to indicate no further interest in a
     * stream. As a result, the stream should be closed.
     */
    public static final int CLOSE = 5;

    /**
     * A provider uses {@link AckMsg} to inform a consumer of success or failure
     * for a specific {@link PostMsg} or {@link CloseMsg}.
     */
    public static final int ACK = 6;

    /**
     * A bi-directional message that does not have any implicit interaction
     * semantics associated with it, thus the name generic. After a stream is
     * established via a request-refresh/status interaction:
     * <ul>
     * <li>A consumer can send this message to a provider.</li>
     * <li>A provider can send this message to a consumer.</li>
     * <li>NIPs can send this message to the ADH/</li>
     * </ul>
     */
    public static final int GENERIC = 7;

    /**
     * A consumer uses {@link PostMsg} to push content upstream. This information
     * can be applied to an Enterprise Platform cache or routed further upstream
     * to a data source. After receiving posted data, upstream components can
     * republish it to downstream consumers.
     */
    public static final int POST = 8;

    /**
     * String representation of a message class.
     * 
     * @param msgClass message class
     * 
     * @return the string representation of a message class
     */
    public static String toString(int msgClass)
    {
        String ret = "";

        switch (msgClass)
        {
            case UPDATE:
                ret = "UPDATE";
                break;
            case GENERIC:
                ret = "GENERIC";
                break;
            case REFRESH:
                ret = "REFRESH";
                break;
            case REQUEST:
                ret = "REQUEST";
                break;
            case POST:
                ret = "POST";
                break;
            case STATUS:
                ret = "STATUS";
                break;
            case CLOSE:
                ret = "CLOSE";
                break;
            case ACK:
                ret = "ACK";
                break;
            default:
                ret = Integer.toString(msgClass);
                break;
        }

        return ret;
    }
}