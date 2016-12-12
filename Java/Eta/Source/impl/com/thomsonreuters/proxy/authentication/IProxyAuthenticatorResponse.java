package com.thomsonreuters.proxy.authentication;

public interface IProxyAuthenticatorResponse
{
    public boolean isProxyConnectionClose();
	
    public String getProxyAuthorization();
}
