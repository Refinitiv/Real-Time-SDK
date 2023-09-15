/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.Common;

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
    DOMAIN_NOT_SUPPORTED,
    //APIQA
    TEMPORARY_REJECTED
    //END APIQA
}

public static class ItemRejectReasonExtensions
{
    public static string GetAsString(this ItemRejectReason rejectReason)
    {
        switch (rejectReason)
        {
            case ItemRejectReason.NONE:
                return "NONE";
            case ItemRejectReason.ITEM_COUNT_REACHED:
                return "ITEM_COUNT_REACHED";
            case ItemRejectReason.INVALID_SERVICE_ID:
                return "INVALID_SERVICE_ID";
            case ItemRejectReason.ITEM_ALREADY_OPENED:
                return "ITEM_ALREADY_OPENED";
            case ItemRejectReason.STREAM_ALREADY_IN_USE:
                return "STREAM_ALREADY_IN_USE";
            case ItemRejectReason.QOS_NOT_SUPPORTED:
                return "QOS_NOT_SUPPORTED";
            case ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED:
                return "KEY_ENC_ATTRIB_NOT_SUPPORTED";
            case ItemRejectReason.ITEM_NOT_SUPPORTED:
                return "ITEM_NOT_SUPPORTED";
            case ItemRejectReason.REQUEST_DECODE_ERROR:
                return "REQUEST_DECODE_ERROR";
            case ItemRejectReason.BATCH_ITEM_REISSUE:
                return "BATCH_ITEM_REISSUE";
            case ItemRejectReason.PRIVATE_STREAM_REDIRECT:
                return "PRIVATE_STREAM_REDIRECT";
            case ItemRejectReason.PRIVATE_STREAM_MISMATCH:
                return "PRIVATE_STREAM_MISMATCH";
            case ItemRejectReason.DOMAIN_NOT_SUPPORTED:
                return "DOMAIN_NOT_SUPPORTED";
            //APIQA
            case ItemRejectReason.TEMPORARY_REJECTED:
                return "TEMPORARY_REJECTED";
            // END APIQA
            default:
                return "Unknown reason";
        }
    }
}
