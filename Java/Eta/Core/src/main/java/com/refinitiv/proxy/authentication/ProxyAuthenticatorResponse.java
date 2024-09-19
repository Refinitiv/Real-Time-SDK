/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

public class ProxyAuthenticatorResponse implements IProxyAuthenticatorResponse
{
    private final boolean _isProxyConnectionClose;
    private final String _proxyAuthorization;
	
    /**
     * Instantiates a new proxy authenticator response.
     *
     * @param isProxyConnectionClose the is proxy connection close
     * @param proxyAuthorization the proxy authorization
     */
    protected ProxyAuthenticatorResponse(boolean isProxyConnectionClose, String proxyAuthorization)
    {
        _isProxyConnectionClose = isProxyConnectionClose;
        _proxyAuthorization = (proxyAuthorization != null) ? proxyAuthorization : "";
    }

    @Override
    public boolean isProxyConnectionClose()
    {
        return _isProxyConnectionClose;
    }

    @Override
    public String getProxyAuthorization()
    {
        return _proxyAuthorization;
    }

}
