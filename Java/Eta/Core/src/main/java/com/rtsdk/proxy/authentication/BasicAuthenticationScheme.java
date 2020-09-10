package com.rtsdk.proxy.authentication;

import org.apache.commons.codec.binary.Base64;
import org.apache.http.util.EncodingUtils;

public class BasicAuthenticationScheme implements IAuthenticationScheme
{
    private final IProxyAuthenticator _proxyAuthenticator;
    private static final String[] RequiredCredentials = { CredentialName.USERNAME, CredentialName.PASSWORD };

    private static final String PROXY_AUTHORIZATION_PREFIX = "Proxy-Authorization: ";
    private static final String AUTHORIZATION_PREFIX = "Authorization: ";
    private static final String BASIC_RESPONSE_PREFIX = "BASIC ";
    private static final String EOL = "\r\n";
    static final String DEFAULT_CHARSET = "ASCII";

    @SuppressWarnings("unused")
    private int ntlmResponseCount = 0;
    boolean stopScheme = false;
	
    /**
     * Instantiates a new basic authentication scheme.
     *
     * @param proxyAuthenticator the proxy authenticator
     * @throws NullPointerException the null pointer exception
     */
    protected BasicAuthenticationScheme(IProxyAuthenticator proxyAuthenticator) throws NullPointerException
    {
        if (proxyAuthenticator == null)
        {
            throw new NullPointerException(String.format("%s: a valid proxyAuthenticator is required.", this.getClass().getName()));
        }

        _proxyAuthenticator = proxyAuthenticator;
    }
	
    @Override
    public IProxyAuthenticator getProxyAuthenicator()
    {
        return _proxyAuthenticator;
    }

    /* Processes a response from the proxy server 
     * and returns a (http) "Proxy-authorization: " value (e.g. "Basic dfdfakajas...") with a trailing \r\n
     * or returns an empty string if a "Proxy-authorization: " value does not need to be sent back to the proxy
     * 
     * httpResponseCode is the http response code to handle (e.g. 407)
     * proxyServerResponse is a response from the proxy server to process (may be null)
     * 
     * Throws ProxyAuthenticationException (an exception that halted the authentication process occurred)
     */
    @Override
    public String processResponse(int httpResponseCode, String proxyServerResponse) throws ProxyAuthenticationException
    {
        StringBuilder proxyAuthorizationValue = new StringBuilder();

        String credentails = String.format("%s:%s",
                                           _proxyAuthenticator.getCredentials().get(CredentialName.USERNAME),
                                           _proxyAuthenticator.getCredentials().get(CredentialName.PASSWORD));

        credentails = EncodingUtils.getAsciiString(Base64.encodeBase64(EncodingUtils.getBytes(credentails, DEFAULT_CHARSET)));

        validateCredentials(); // throws an exception if invalid

        if (httpResponseCode == 407)
        {
            proxyAuthorizationValue.append(PROXY_AUTHORIZATION_PREFIX);
        }
        else
        {
            proxyAuthorizationValue.append(AUTHORIZATION_PREFIX);
        }

        proxyAuthorizationValue.append(BASIC_RESPONSE_PREFIX);
        proxyAuthorizationValue.append(credentails);
        proxyAuthorizationValue.append(EOL);

        return proxyAuthorizationValue.toString();
    }
	
    public String name()
    {
        return "BASIC";
    }
	
    /* not useful in this scheme */
    public boolean stopScheme()
    {
        return stopScheme;
    }

    /* Throws a ProxyAuthenticationException if the credentials required for Basic authentication are invalid */
    @Override
    public void validateCredentials() throws ProxyAuthenticationException
    {
        for (String credentialName : RequiredCredentials)
        {
            if (!_proxyAuthenticator.getCredentials().isSet(credentialName))
            {
                StringBuilder sb = new StringBuilder();
                sb.append(this.getClass().getName());
                sb.append(": The \"");
                sb.append(credentialName);
                sb.append("\" credential is required for Basic authentication. ( The full list of required credentials is: ");

                for (String required : RequiredCredentials)
                {
                    sb.append(required);
                    sb.append(" ");
                }

                sb.append(")");

                throw new ProxyAuthenticationException(sb.toString());
            }
        }
    }

}
