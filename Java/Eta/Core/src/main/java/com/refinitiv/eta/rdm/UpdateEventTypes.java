/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

/**
 * Indicates the general content of the update.
 */
public class UpdateEventTypes
{
    // UpdateEventTypes class cannot be instantiated
    private UpdateEventTypes()
    {
        throw new AssertionError();
    }

    /** Unspecified Update Event */
    public static final int UNSPECIFIED = 0;
    /** Update Event Quote */
    public static final int QUOTE = 1;
    /** Update Event Trade */
    public static final int TRADE = 2;
    /** Update Event News Alert */
    public static final int NEWS_ALERT = 3;
    /** Update Event Volume Alert */
    public static final int VOLUME_ALERT = 4;
    /** Update Event Order Indication */
    public static final int ORDER_INDICATION = 5;
    /** Update Event Closing Run */
    public static final int CLOSING_RUN = 6;
    /** Update Event Correction */
    public static final int CORRECTION = 7;
    /** Update Event Market Digest */
    public static final int MARKET_DIGEST = 8;
    /** Update Event Quotes followed by a Trade */
    public static final int QUOTES_TRADE = 9;
    /** Update Event with filtering and conflation applied */
    public static final int MULTIPLE = 10;
    /** Fields may have changed */
    public static final int VERIFY = 11;
    /* Maximum reserved update event type */
    static final int MAX_RESERVED = 127;
    
    /**
     * String representation of an update event type name.
     * 
     * @param updateEventType update event type value
     * 
     * @return the string representation of an update event type name
     */
    public static String toString(int updateEventType)
    {
        String ret = "";

        switch (updateEventType)
        {
            case UNSPECIFIED:
                ret = "UNSPECIFIED";
                break;
            case QUOTE:
                ret = "QUOTE";
                break;
            case TRADE:
                ret = "TRADE";
                break;
            case NEWS_ALERT:
                ret = "NEWS_ALERT";
                break;
            case VOLUME_ALERT:
                ret = "VOLUME_ALERT";
                break;
            case ORDER_INDICATION:
                ret = "ORDER_INDICATION";
                break;
            case CLOSING_RUN:
                ret = "CLOSING_RUN";
                break;
            case CORRECTION:
                ret = "CORRECTION";
                break;
            case MARKET_DIGEST:
                ret = "MARKET_DIGEST";
                break;
            case QUOTES_TRADE:
                ret = "QUOTES_TRADE";
                break;
            case MULTIPLE:
                ret = "MULTIPLE";
                break;
            case VERIFY:
                ret = "VERIFY";
                break;
            default:
                ret = Integer.toString(updateEventType);
                break;
        }

        return ret;
    }

}
