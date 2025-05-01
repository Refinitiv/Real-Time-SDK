/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

/**
 * RDM Instrument Name Types.
 *
 */
public class InstrumentNameTypes
{
    // InstrumentNameTypes class cannot be instantiated
    private InstrumentNameTypes()
    {
        throw new AssertionError();
    }

    /** Symbology is not specified or not applicable */
    public static final int UNSPECIFIED     = 0;
    /** Instrument Code */
    public static final int RIC             = 1;
    /** Contributor */
    public static final int CONTRIBUTOR     = 2;
    /* Maximum reserved Quote Symbology */
    static final int MAX_RESERVED   = 127;
}
