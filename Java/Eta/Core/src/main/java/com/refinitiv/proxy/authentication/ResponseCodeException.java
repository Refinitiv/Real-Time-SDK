/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

public class ResponseCodeException extends ProxyAuthenticationException
{
    private static final long serialVersionUID = -6455982052563237834L;

    /**
     * Instantiates a new response code exception.
     *
     * @param message the message
     */
    public ResponseCodeException(String message)
    {
        super(message);
    }

    /**
     * Instantiates a new response code exception.
     *
     * @param message the message
     * @param cause the cause
     */
    public ResponseCodeException(String message, Throwable cause)
    {
        super(message, cause);
    }

}
