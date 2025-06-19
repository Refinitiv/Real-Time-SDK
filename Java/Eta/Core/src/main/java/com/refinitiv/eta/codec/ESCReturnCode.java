/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/* Used to know the Return Code based on the ESC sequence used in RMTES. */

class ESCReturnCode
{
    static final int ESC_ERROR = -1;
    static final int ESC_SUCCESS = 0;
    static final int UTF_ENC = 1;
    static final int RHPA_CMD = 2;
    static final int RREP_CMD = 3;
    static final int END_CHAR = 4;
}
