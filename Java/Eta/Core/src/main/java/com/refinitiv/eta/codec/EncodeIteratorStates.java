/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/* Internal iterator states */
class EncodeIteratorStates
{
    public static final int NONE                        = 0;
    public static final int SET_DEFINITIONS             = 1;
    public static final int SUMMARY_DATA                = 2;
    public static final int SET_DATA                    = 3;
    public static final int SET_ENTRY_INIT              = 4;
    public static final int PRIMITIVE                   = 5;
    public static final int PRIMITIVE_U15               = 6;
    public static final int ENTRIES                     = 7;
    public static final int ENTRY_INIT                  = 8;
    public static final int ENTRY_WAIT_COMPLETE         = 9;
    public static final int SET_ENTRY_WAIT_COMPLETE     = 10;
    public static final int EXTENDED_HEADER             = 11;
    public static final int OPAQUE                      = 12;
    public static final int OPAQUE_AND_EXTENDED_HEADER  = 13;
    public static final int WAIT_COMPLETE               = 14;
    public static final int COMPLETE                    = 15;
    public static final int NON_RWF_DATA                = 16;
}
