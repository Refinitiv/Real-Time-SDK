/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2025 LSEG. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 *  ETA Reactor Channel IOCtl codes for {@link ReactorChannel#ioctl(int, Object value, ReactorErrorInfo)}
 *
 */
public class ReactorChannelIOCtlCode
{
    /**
     * This option is used to dynamically change the preferred host options.
    */
    public static final int FALLBACK_PREFERRED_HOST_OPTIONS = 201;
}