package com.thomsonreuters.proxy.authentication;

import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class ProxyAuthenticatorImpl implements IProxyAuthenticator
{
    protected ICredentials _credentials;

    protected String _proxyHost;
    private boolean _isAuthenticated = false;
    private static final String END_OF_CHUNK = "\r\n\r\n";

    private IAuthenticationScheme _authScheme;
    private final Pattern _httpVersionPattern = Pattern.compile("^\\s*HTTP/1.. (\\d+)");
    private final Pattern _proxyAuthenticatePattern = Pattern.compile("Proxy-[a|A]uthenticate: (\\w+)");
    private final Pattern _wwwAuthenticatePattern = Pattern.compile("WWW-[a|A]uthenticate: (\\w+)");
    private final Pattern _proxyConnectionClosePattern = Pattern.compile("Proxy-[c|C]onnection: close");
    private final Pattern _connectionClosePattern = Pattern.compile("Connection: [c|C]lose");
	
    boolean negotiateHasFailed = false;
    boolean kerberosHasFailed = false;
    boolean ntlmHasFailed = false;	
	
    /**
     * Instantiates a new proxy authenticator impl.
     *
     * @param credentials the credentials
     * @param ProxyHost the proxy host
     */
    protected ProxyAuthenticatorImpl(ICredentials credentials, String ProxyHost)
    {
        _credentials = credentials;
        _proxyHost = ProxyHost;
    }

    /**
     * Instantiates a new proxy authenticator impl.
     *
     * @param credentials the credentials
     */
    protected ProxyAuthenticatorImpl(ICredentials credentials)
    {
        _credentials = credentials;
    }

    /**
     * Instantiates a new proxy authenticator impl.
     */
    protected ProxyAuthenticatorImpl()
    {
        _credentials = CredentialsFactory.create();
    }

    @Override
    public void setCredentials(ICredentials credentials) throws NullPointerException
    {
        if (credentials == null)
        {
            throw new NullPointerException();
        }

        _credentials = credentials;
    }

    @Override
    public ICredentials getCredentials()
    {
        return _credentials;
    }

    @Override
    public String getProxyHost()
    {
        return _proxyHost;
    }

    @Override
    public IProxyAuthenticatorResponse processResponse(String response) throws ResponseCodeException, ProxyAuthenticationException
    {
        // the proxy may indicate the connection to it will be closed:
        boolean isProxyConnectionClose = false;
        IProxyAuthenticatorResponse authResponse; // the response we (may) generate

        if (_isAuthenticated || response == null || response.isEmpty())
        {
            authResponse = new ProxyAuthenticatorResponse(false, ""); // nothing to do
        }

        int httpResponseCode = parseHttpResponseCode(response);

        if (httpResponseCode == 200)
        {
            _isAuthenticated = true;
            authResponse = new ProxyAuthenticatorResponse(false, ""); // nothing to do
        }
        else if (httpResponseCode == 407 || httpResponseCode == 401)
        {
            if (responseContainsProxyClose(response) || responseContainsConnectionClose(response))
            {
                isProxyConnectionClose = true; // The proxy requested to close the connection.
            }

            if (_isAuthenticated)
            {
                // we were authenticated, but we must restart the authentication process
                _isAuthenticated = false;
                _authScheme = null;
            }

            if (_authScheme == null)
            {
                // the next call will throw a ProxyAuthenticationException if we don't
                // have an implementation that can authenticate against this proxy
                createAuthenticationSchemeFromResponse(httpResponseCode, response);
            }

            authResponse = new ProxyAuthenticatorResponse(isProxyConnectionClose, _authScheme.processResponse(httpResponseCode, response));

            if (_authScheme != null)
            {
                if (_authScheme.stopScheme())
                {
                    if (_authScheme.name().equalsIgnoreCase("NEGOTIATE"))
                        negotiateHasFailed = true;
                    else if (_authScheme.name().equalsIgnoreCase("KERBEROS"))
                        kerberosHasFailed = true;
                    else if (_authScheme.name().equalsIgnoreCase("NTLM"))
                        ntlmHasFailed = true;

                    createAuthenticationSchemeFromResponse(httpResponseCode, response);
                    authResponse = new ProxyAuthenticatorResponse(isProxyConnectionClose, _authScheme.processResponse(httpResponseCode, response));
                }
            }
        }
        else
        {
            throw new ResponseCodeException(this.getClass().getName() + ": Unexpected http response code: " + httpResponseCode);
        }

        return authResponse;
    }

    private void createAuthenticationSchemeFromResponse(int httpResponseCode, String response) throws ProxyAuthenticationException
    {
        List<String> authenticationSchemes = parseAuthenticationSchemes(httpResponseCode, response);

        _authScheme = AuthenticationSchemeFactory.create(authenticationSchemes, this);
    }

    public boolean hasNegotiateFailed()
    {
        return negotiateHasFailed;
    }

    public boolean hasKerberosFailed()
    {
        return kerberosHasFailed;
    }

    public boolean hasNTLMfailed()
    {
        return ntlmHasFailed;
    }
	
    public void setNegotiateFailed()
    {
        negotiateHasFailed = true;
    }

    public void setKerberosFailed()
    {
        kerberosHasFailed = true;
    }

    public void setNTLMfailed()
    {
        ntlmHasFailed = true;
    }

    private List<String> parseAuthenticationSchemes(int httpResponseCode, String response)
    {
        List<String> authenticationSchemes = new LinkedList<String>();

        Matcher matcher;

        if (httpResponseCode == 407)
        {
            matcher = _proxyAuthenticatePattern.matcher(response);
        }
        else
        {
            matcher = _wwwAuthenticatePattern.matcher(response);
        }

        while (matcher.find())
        {
            if (matcher.groupCount() >= 1)
            {
                authenticationSchemes.add(matcher.group(1));
            }
        }

        return authenticationSchemes;
    }

    private boolean responseContainsProxyClose(String response)
    {
        Matcher matcher = _proxyConnectionClosePattern.matcher(response);
        return matcher.find();
    }
	
    private boolean responseContainsConnectionClose(String response)
    {
        Matcher matcher = _connectionClosePattern.matcher(response);
        return matcher.find();
    }

    @Override
    public boolean isAuthenticated()
    {
        return _isAuthenticated;
    }

    private int parseHttpResponseCode(String response) throws ResponseCodeException
    {
        final int NOT_FOUND = -1;
        int httpResponseCode = NOT_FOUND;

        if (response != null)
        {
            // The proxy response may be in multiple "chunks".
            // For example, FreeProxy sends both a htlm header response (with a response code),
            // plus an html message formatted for web browsers (that we will ignore).
            String[] responseChunks = response.split(END_OF_CHUNK);
            if (responseChunks != null)
            {
                for (String chunk : responseChunks)
                {
                    Matcher matcher = _httpVersionPattern.matcher(chunk);

                    if (matcher.find() && matcher.groupCount() >= 1)
                    {
                        httpResponseCode = Integer.parseInt(matcher.group(1));
                        break;
                    }
                }
            }
        }

        if (httpResponseCode == NOT_FOUND)
        {
            throw new ResponseCodeException("Unable to parse HTTP response code. (Authentication errors may have occurred.)");
        }

        return httpResponseCode;
    }
}
