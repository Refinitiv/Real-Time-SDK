/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

public class ProxyAuthenticatorFactory
{
    
    /**
     * Creates the proxy authenticator.
     *
     * @return the i proxy authenticator
     */
    public static IProxyAuthenticator create()
    {
        return new ProxyAuthenticatorImpl();
    }

    /**
     * Creates the proxy authenticator.
     *
     * @param credentials the credentials
     * @return the i proxy authenticator
     */
    public static IProxyAuthenticator create(ICredentials credentials)
    {
        return new ProxyAuthenticatorImpl(credentials);
    }

    /**
     * Creates the proxy authenticator.
     *
     * @param credentials the credentials
     * @param proxyHost the proxy host
     * @return the i proxy authenticator
     */
    public static IProxyAuthenticator create(ICredentials credentials, String proxyHost)
    {
        return new ProxyAuthenticatorImpl(credentials, proxyHost);
    }
}
