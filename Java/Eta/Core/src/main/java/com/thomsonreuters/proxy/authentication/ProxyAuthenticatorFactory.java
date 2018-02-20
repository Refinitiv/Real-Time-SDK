package com.thomsonreuters.proxy.authentication;

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
