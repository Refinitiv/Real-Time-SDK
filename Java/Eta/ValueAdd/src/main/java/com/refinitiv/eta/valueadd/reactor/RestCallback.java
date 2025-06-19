/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The default message callback is used to communicate message events
 * to the application.
 */
interface RestCallback
{
    public int RestResponseCallback(RestResponse response, RestEvent event);
    public int RestErrorCallback(RestEvent event, String errorText);
}
