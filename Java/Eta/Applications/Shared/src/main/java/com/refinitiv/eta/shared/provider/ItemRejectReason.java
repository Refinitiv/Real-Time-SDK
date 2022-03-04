/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.provider;

/**
 *  Reasons an item request is rejected.
 */
public enum ItemRejectReason
{
    NONE,
    ITEM_COUNT_REACHED, 
    INVALID_SERVICE_ID, 
    QOS_NOT_SUPPORTED, 
    ITEM_ALREADY_OPENED, 
    STREAM_ALREADY_IN_USE, 
    KEY_ENC_ATTRIB_NOT_SUPPORTED, 
    PRIVATE_STREAM_REDIRECT,
    PRIVATE_STREAM_MISMATCH,
    ITEM_NOT_SUPPORTED,
    REQUEST_DECODE_ERROR,
    BATCH_ITEM_REISSUE,
    DOMAIN_NOT_SUPPORTED;

    
    /*
     * Clears the item information.
     * itemInfo - The item information to be cleared
     */
    public static String toString(ItemRejectReason rejectReason)
    {
        switch (rejectReason)
        {
            case NONE:
                return "NONE";
            case ITEM_COUNT_REACHED:
                return "ITEM_COUNT_REACHED";
            case INVALID_SERVICE_ID:
                return "INVALID_SERVICE_ID";
            case ITEM_ALREADY_OPENED:
                return "ITEM_ALREADY_OPENED";
            case STREAM_ALREADY_IN_USE:
                return "STREAM_ALREADY_IN_USE";
            case QOS_NOT_SUPPORTED:
                return "QOS_NOT_SUPPORTED";
            case KEY_ENC_ATTRIB_NOT_SUPPORTED:
                return "KEY_ENC_ATTRIB_NOT_SUPPORTED";
            case ITEM_NOT_SUPPORTED:
                return "ITEM_NOT_SUPPORTED";
            case REQUEST_DECODE_ERROR:
                return "REQUEST_DECODE_ERROR";
            case BATCH_ITEM_REISSUE:
                return "BATCH_ITEM_REISSUE";
            case PRIVATE_STREAM_REDIRECT:
                return "PRIVATE_STREAM_REDIRECT";
            case PRIVATE_STREAM_MISMATCH:
                return "PRIVATE_STREAM_MISMATCH";
            case DOMAIN_NOT_SUPPORTED:
                return "DOMAIN_NOT_SUPPORTED";
            default:
                return "Unknown reason";
        }
    }
}
