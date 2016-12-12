package com.thomsonreuters.proxy.authentication;

public class ProxyAuthenticatorResponse implements IProxyAuthenticatorResponse
{
    private final boolean _isProxyConnectionClose;
    private final String _proxyAuthorization;
	
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
