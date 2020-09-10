package com.rtsdk.eta.codec;

/**
 * The Ack Message nakCodes, used to indicate a reason for a negative acknowledgment.
 * 
 * @see AckMsg
 */
public class NakCodes
{
    /**
     * This class is not instantiated
     */
    private NakCodes()
    {
        throw new AssertionError();
    }

    /** No Nak Code */
    public static final int NONE = 0;

    /**
     * Access Denied (user not properly permissioned for posting on the item or service)
     */
    public static final int ACCESS_DENIED = 1;

    /**
     * Denied by source (source being posted to has denied accepting this post message)
     */
    public static final int DENIED_BY_SRC = 2;

    /** Source Down (source being posted to is down or not available) */
    public static final int SOURCE_DOWN = 3;

    /** Source Unknown (source being posted to is unknown and is unreachable) */
    public static final int SOURCE_UNKNOWN = 4;

    /**
     * No Resources (some component along the path of the post message does not
     * have appropriate resources available to continue processing the post)
     */
    public static final int NO_RESOURCES = 5;

    /**
     * No Response (may mean that the source is unavailable or there is a delay
     * in processing the posted information)
     */
    public static final int NO_RESPONSE = 6;

    /**
     * Gateway is down (gateway device for handling posted or contributed
     * information is down or not available)
     */
    public static final int GATEWAY_DOWN = 7;

    /* Reserved */
    static final int RESERVED8 = 8;
    /* Reserved */
    static final int RESERVED9 = 9;

    /**
     * Unknown Symbol (item information provided with the post message is not
     * recognized by the system)
     */
    public static final int SYMBOL_UNKNOWN = 10;

    /**
     * Item not open (item being posted to does not have an available stream open)
     */
    public static final int NOT_OPEN = 11;

    /**
     * Nak being sent due to invalid content (content of the post message is
     * invalid and cannot be posted, it does not match the expected formatting for this post)
     */
    public static final int INVALID_CONTENT = 12;

    public static String toString(int nakCode)
    {
        String ret = "";

        switch (nakCode)
        {
            case NONE:
                ret = "NONE";
                break;
            case ACCESS_DENIED:
                ret = "ACCESS_DENIED";
                break;
            case DENIED_BY_SRC:
                ret = "DENIED_BY_SRC";
                break;
            case SOURCE_DOWN:
                ret = "SOURCE_DOWN";
                break;
            case SOURCE_UNKNOWN:
                ret = "SOURCE_UNKNOWN";
                break;
            case NO_RESOURCES:
                ret = "NO_RESOURCES";
                break;
            case NO_RESPONSE:
                ret = "NO_RESPONSE";
                break;
            case GATEWAY_DOWN:
                ret = "GATEWAY_DOWN";
                break;
            case SYMBOL_UNKNOWN:
                ret = "SYMBOL_UNKNOWN";
                break;
            case NOT_OPEN:
                ret = "NOT_OPEN";
                break;
            case INVALID_CONTENT:
                ret = "INVALID_CONTENT";
                break;
            default:
                ret = Integer.toString(nakCode);
                break;
        }

        return ret;
    }
}
