package com.thomsonreuters.proxy.authentication;

public class ProxyAuthenticatorFactory
{
    public static IProxyAuthenticator create()
    {
        return new ProxyAuthenticatorImpl();
    }

    public static IProxyAuthenticator create(ICredentials credentials)
    {
        return new ProxyAuthenticatorImpl(credentials);
    }

    public static IProxyAuthenticator create(ICredentials credentials, String proxyHost)
    {
        return new ProxyAuthenticatorImpl(credentials, proxyHost);
    }
}
