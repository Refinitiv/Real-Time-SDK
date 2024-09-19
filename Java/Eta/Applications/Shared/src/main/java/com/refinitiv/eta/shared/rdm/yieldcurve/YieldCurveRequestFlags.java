/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.yieldcurve;

/**
 * Yield curve request flags. 
 */
public class YieldCurveRequestFlags
{
    public static final int NONE = 0;
    public static final int HAS_QOS = 0x001;
    public static final int HAS_PRIORITY = 0x002;
    public static final int HAS_SERVICE_ID = 0x004;
    public static final int HAS_VIEW = 0x010;
    public static final int STREAMING = 0x020;
    public static final int PRIVATE_STREAM = 0x040;
    
    private YieldCurveRequestFlags()
    {
    }
}
