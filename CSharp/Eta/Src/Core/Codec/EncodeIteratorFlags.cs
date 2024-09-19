/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    /* special flags for encode iterator use */
    internal enum EncodeIteratorFlags
    {
        NONE = 0x0000,
        LENU15 = 0x0001, // Length needs to be encoded as u15
        HAS_PER_ENTRY_PERM = 0x0002, // map, vector, or filter list needs per-entry perm flag set
    }

}