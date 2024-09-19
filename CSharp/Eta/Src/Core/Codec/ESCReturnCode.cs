/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    enum ESCReturnCode
    {
        ESC_ERROR = -1,
        ESC_SUCCESS = 0,
        UTF_ENC = 1,
        RHPA_CMD = 2,
        RREP_CMD = 3,
        END_CHAR = 4
    }
}

