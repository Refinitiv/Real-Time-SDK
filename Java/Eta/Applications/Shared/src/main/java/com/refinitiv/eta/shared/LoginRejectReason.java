/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

/** reasons a login request is rejected */
public enum LoginRejectReason
{
    MAX_LOGIN_REQUESTS_REACHED,
    NO_USER_NAME_IN_REQUEST,
    LOGIN_RDM_DECODER_FAILED;
}