/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    /* Internal iterator states */
    internal enum EncodeIteratorStates
    {
        NONE = 0,
        SET_DEFINITIONS = 1,
        SUMMARY_DATA = 2,
        SET_DATA = 3,
        SET_ENTRY_INIT = 4,
        PRIMITIVE = 5,
        PRIMITIVE_U15 = 6,
        ENTRIES = 7,
        ENTRY_INIT = 8,
        ENTRY_WAIT_COMPLETE = 9,
        SET_ENTRY_WAIT_COMPLETE = 10,
        EXTENDED_HEADER = 11,
        OPAQUE = 12,
        OPAQUE_AND_EXTENDED_HEADER = 13,
        WAIT_COMPLETE = 14,
        COMPLETE = 15,
        NON_RWF_DATA = 16
    }

}