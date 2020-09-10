package com.rtsdk.proxy.authentication;

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
