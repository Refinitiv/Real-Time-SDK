/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    enum RMTESParseState
    {
        ERROR = -1,
        NORMAL = 0,
        ESC = 1,
        LBRKT = 3,
        RHPA = 4,
        RREP = 5,
        ESC_21 = 6,
        ESC_22 = 7,
        ESC_24 = 8,
        ESC_24_28 = 9,
        ESC_24_29 = 10,
        ESC_24_2A = 11,
        ESC_24_2B = 12,
        ESC_25 = 13,
        ESC_26 = 14,
        ESC_26_40 = 15,
        ESC_26_40_ESC = 16,
        ESC_26_40_ESC_24 = 17,
        ESC_26_40_ESC_24_29 = 18,
        ESC_26_40_ESC_24_2A = 19,
        ESC_26_40_ESC_24_2B = 20,
        ESC_28 = 21,
        ESC_29 = 22,
        ESC_2A = 23,
        ESC_2B = 24
    }
}
