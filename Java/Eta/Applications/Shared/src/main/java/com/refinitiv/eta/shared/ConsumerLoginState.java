/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

/**
 * Consumer login states.
 */
public enum ConsumerLoginState
{
    PENDING_LOGIN, /** initial state before request is sent*/ 
    OK_SOLICITED, 
    OK_UNSOLICITED,
    CLOSED, 
    CLOSED_RECOVERABLE, 
    SUSPECT; 
}
