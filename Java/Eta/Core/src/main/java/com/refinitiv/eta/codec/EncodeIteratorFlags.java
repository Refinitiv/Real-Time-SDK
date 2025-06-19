/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/* special flags for encode iterator use */
class EncodeIteratorFlags
{
    public static final int NONE               = 0x0000;
    public static final int LENU15             = 0x0001; /* Length needs to be encoded as u15 */
    public static final int HAS_PER_ENTRY_PERM = 0x0002; /* map, vector, or filter list needs per-entry perm flag set */
}
