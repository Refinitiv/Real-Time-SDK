/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

import java.util.HashMap;

/***
 * Indicates Update Type Filter value
 */
public class UpdateTypeFilter {

    // Update type is Unspecified
    public static final long RDM_UPT_UNSPECIFIED = 0x001;

    // Update Type is Quote
    public static final long RDM_UPT_QUOTE = 0x002;

    // Update Type is Trade
    public static final long RDM_UPT_TRADE = 0x004;

    // Update Type is News Alert
    public static final long RDM_UPT_NEWS_ALERT = 0x008;

    // Update Type is Volume Alert
    public static final long RDM_UPT_VOLUME_ALERT = 0x010;

    // Update Type is Order Indication
    public static final long RDM_UPT_ORDER_INDICATION = 0x020;

    // Update Type is Closing Run
    public static final long RDM_UPT_CLOSING_RUN = 0x040;

    // Update Type is Correction
    public static final long RDM_UPT_CORRECTION = 0x080;

    // Update Type is Market Digest
    public static final long RDM_UPT_MARKET_DIGEST = 0x100;

    // Update Type is Quotes followed by a Trade
    public static final long RDM_UPT_QUOTES_TRADE = 0x200;

    // Update with filtering and conflation applied
    public static final long RDM_UPT_MULTIPLE = 0x400;

    // Fields may have changed
    public static final long RDM_UPT_VERIFY = 0x800;

    private static final long[] TYPE_FILTER_ENTRIES = {
            RDM_UPT_UNSPECIFIED,
            RDM_UPT_QUOTE,
            RDM_UPT_TRADE,
            RDM_UPT_NEWS_ALERT,
            RDM_UPT_VOLUME_ALERT,
            RDM_UPT_ORDER_INDICATION,
            RDM_UPT_CLOSING_RUN,
            RDM_UPT_CORRECTION,
            RDM_UPT_MARKET_DIGEST,
            RDM_UPT_QUOTES_TRADE,
            RDM_UPT_MULTIPLE,
            RDM_UPT_VERIFY
    };

    private static final HashMap<Long, String> TYPE_FILTER_VAL_TO_NAME = new HashMap<>();

    static
    {
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_UNSPECIFIED, "Unspecified");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_QUOTE, "Quote");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_TRADE, "Trade");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_NEWS_ALERT, "News Alert");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_VOLUME_ALERT, "Volume Alert");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_ORDER_INDICATION, "Order Indication");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_CLOSING_RUN, "Closing Run");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_CORRECTION, "Correction");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_MARKET_DIGEST, "Market Digest");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_QUOTES_TRADE, "Quotes Trade");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_MULTIPLE, "Multiple");
        TYPE_FILTER_VAL_TO_NAME.put(RDM_UPT_VERIFY, "Verify");
    }

    public static String updateTypeFilterToString(long value)
    {
        StringBuilder resBuilder = new StringBuilder();

        for (int i = 0; i < TYPE_FILTER_ENTRIES.length; i++) {
            if ((value & TYPE_FILTER_ENTRIES[i]) > 0) {
                resBuilder.append(TYPE_FILTER_VAL_TO_NAME.get(TYPE_FILTER_ENTRIES[i]));
                resBuilder.append(" | ");
            }
        }
        resBuilder.delete(resBuilder.length() - 3, resBuilder.length());

        return resBuilder.toString();
    }
}
