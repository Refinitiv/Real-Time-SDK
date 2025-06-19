/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

public class ProxyAuthenticationException extends Exception
{
    private static final long serialVersionUID = 5209362189686162178L;

    /**
     * Instantiates a new proxy authentication exception.
     *
     * @param message the message
     */
    public ProxyAuthenticationException(String message)
    {
        super(message);
    }

    /**
     * Instantiates a new proxy authentication exception.
     *
     * @param message the message
     * @param cause the cause
     */
    public ProxyAuthenticationException(String message, Throwable cause)
    {
        super(message, cause);
    }
}
