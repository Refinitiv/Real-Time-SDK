package com.rtsdk.eta.codec;

/**
 * Combination of bit values to indicate special behaviors and the presence of
 * optional {@link RequestMsg} content.
 * 
 * @see RequestMsg
 */
public class RequestMsgFlags
{
    /**
     * This class is not instantiated
     */
    private RequestMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x000) No Flags set */
    public static final int NONE = 0x000;

    /** (0x001) Request Message has Extended Header */
    public static final int HAS_EXTENDED_HEADER = 0x001;

    /** (0x002) Has Priority */
    public static final int HAS_PRIORITY = 0x002;

    /**
     * (0x004) Indicates whether the request is for streaming data.
     * <ul>
     * <li>If present, the OMM consumer wants to continue to receive changes to
     * information after the initial refresh is complete.</li>
     * <li>If absent, the OMM consumer wants to receive only the refresh, after
     * which the OMM Provider should close the stream. Such a request is
     * typically referred to as a non-streaming or snapshot data request.</li>
     * </ul>
     * Because a refresh can be split into multiple parts, it is possible for
     * updates to occur between the first and last part of the refresh, even as
     * part of a non-streaming request.
     */
    public static final int STREAMING = 0x004;

    /**
     * (0x008) Indicates that the consumer wants to receive the full msgKey in
     * update messages. This flag does not control whether the msgKey is present
     * in an update message. Instead, the provider application determines
     * whether this information is present (the consumer should be capable of
     * handling the msgKey in any {@link UpdateMsg}). When specified on a
     * request to ADS, the ADS fulfils the request.
     */
    public static final int MSG_KEY_IN_UPDATES = 0x008;

    /**
     * (0x010) Indicates that the consumer wants to receive conflation
     * information in update messages delivered on this stream. This flag does
     * not control whether conflation information is present in update messages.
     * Instead, the provider application determines whether this information is
     * present,(the consumer should be capable of handling conflation
     * information in any {@link UpdateMsg}).
     */
    public static final int CONF_INFO_IN_UPDATES = 0x010;

    /**
     * (0x020) Indicates that the consumer application does not require a
     * refresh for this request. This typically occurs after an initial request
     * handshake is completed, usually to change stream attributes (e.g.,
     * priority). In some instances, a Provider may still deliver a refresh
     * message (but if the consumer does not explicitly ask for it, the message
     * should be unsolicited).
     */
    public static final int NO_REFRESH = 0x020;

    /** (0x040) Indicates that Request has Qos */
    public static final int HAS_QOS = 0x040;

    /** (0x080) Indicates that Request has Worst Qos */
    public static final int HAS_WORST_QOS = 0x080;

    /** (0x100) Requests that the stream be opened as private */
    public static final int PRIVATE_STREAM = 0x100;

    /**
     * (0x200) Indicates that the consumer would like to pause the stream,
     * though this does not guarantee that the stream will pause. To resume data
     * flow, the consumer must send a subsequent request message with the
     * {@link #STREAMING} flag set.
     */
    public static final int PAUSE = 0x200;

    /**
     * (0x400) Indicates that the request message payload may contain a Dynamic
     * View, specifying information the application wishes to receive (or that
     * the application wishes to continue receiving a previously specified
     * View). If this flag is not present, any previously specified view is
     * discarded and the full View is provided.
     */
    public static final int HAS_VIEW = 0x400;

    /**
     * (0x800) Indicates that the request message payload contains a list of
     * items of interest, all with matching msgKey information.
     */
    public static final int HAS_BATCH = 0x800;
    
    /** (0x1000) Requests that the stream be opened as qualified */
    public static final int QUALIFIED_STREAM = 0x1000;
       
}
