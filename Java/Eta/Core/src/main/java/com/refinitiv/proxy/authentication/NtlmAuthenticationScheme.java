/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

class NtlmAuthenticationScheme implements IAuthenticationScheme
{
    private static enum State
    {
        UnInitalized, Initalized, WaitingForType2, Type3Generated
    }

    private static final String PROXY_AUTHORIZATION_PREFIX = "Proxy-Authorization: ";
    private static final String AUTHORIZATION_PREFIX = "Authorization: ";
    private static final String NTLM_RESPONSE_PREFIX = "NTLM "; // "Negotiate ";
    private static final String EOL = "\r\n";

    private final IProxyAuthenticator _proxyAuthenticator;
    private State _state = State.UnInitalized;
    private static final String[] RequiredCredentials = {
        CredentialName.DOMAIN, CredentialName.USERNAME, CredentialName.PASSWORD, CredentialName.LOCAL_HOSTNAME };
    private final NTLMEngineImpl _ntlmEgine = new NTLMEngineImpl();
    private static final Pattern PROXY_AUTHENTICATE_PATTERN = Pattern.compile("Proxy-Authenticate: " + NTLM_RESPONSE_PREFIX + "(\\S+)");
    private static final Pattern WWW_AUTHENTICATE_PATTERN = Pattern.compile("WWW-Authenticate: " + NTLM_RESPONSE_PREFIX + "(\\S+)");

    private int ntlmResponseCount = 0;
    boolean stopScheme = false;

    /**
     * Instantiates a new ntlm authentication scheme.
     *
     * @param proxyAuthenticator the proxy authenticator
     * @throws NullPointerException the null pointer exception
     */
    protected NtlmAuthenticationScheme(IProxyAuthenticator proxyAuthenticator) throws NullPointerException
    {
        if (proxyAuthenticator == null)
        {
            throw new NullPointerException(String.format("%s: a valid proxyAuthenticator is required.", this.getClass().getName()));
        }

        _proxyAuthenticator = proxyAuthenticator;
        _state = State.Initalized;
    }

    @Override
    public IProxyAuthenticator getProxyAuthenicator()
    {
        return _proxyAuthenticator;
    }

    /* Processes a response from the proxy server
     * and returns a (http) "Proxy-authorization: " value (e.g. "NTLM TlRMTVNTUA...") with a trailing \r\n
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
        String proxyAuthorizationValue;
        ntlmResponseCount++;
        if (ntlmResponseCount > 24)
        {
            // this could have occurred for a case where the credentials were wrong (e.g. wrong domain)
            // so stop trying anymore
            stopScheme = true;
        }

        validateCredentials(); // throws an exception if invalid

        if (_state == State.Initalized || _state == State.Type3Generated)
        {
            // generate a type1 NTLM message
            String type1Message = generateNtlmMessage(null, // always null for type 1 messages
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.USERNAME),
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.PASSWORD),
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.LOCAL_HOSTNAME),
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.DOMAIN));

            if (httpResponseCode == 407)
            {
                proxyAuthorizationValue = PROXY_AUTHORIZATION_PREFIX + NTLM_RESPONSE_PREFIX + type1Message + EOL;
            }
            else
            {
                proxyAuthorizationValue = AUTHORIZATION_PREFIX + NTLM_RESPONSE_PREFIX + type1Message + EOL;
            }

            _state = State.WaitingForType2;
        }
        else if (_state == State.WaitingForType2)
        {
            String type2Message = parseType2Message(httpResponseCode, proxyServerResponse);

            String type3Message = generateNtlmMessage(type2Message,
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.USERNAME),
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.PASSWORD),
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.LOCAL_HOSTNAME),
                                                      _proxyAuthenticator.getCredentials().get(CredentialName.DOMAIN));

            if (httpResponseCode == 407)
            {
                proxyAuthorizationValue = PROXY_AUTHORIZATION_PREFIX + NTLM_RESPONSE_PREFIX + type3Message + EOL;
            }
            else
            {
                proxyAuthorizationValue = AUTHORIZATION_PREFIX + NTLM_RESPONSE_PREFIX + type3Message + EOL;
            }

            _state = State.Type3Generated;
        }
        else if (_state == State.Type3Generated)
        {
            throw new ProxyAuthenticationException(this.getClass().getName() + ": A type3 response was already generated.");
        }
        else
        {
            throw new ProxyAuthenticationException(this.getClass().getName() + ": Unexpected state: " + _state.toString());
        }

        return proxyAuthorizationValue;
    }

    public String name()
    {
        return "NTLM";
    }

    public boolean stopScheme()
    {
        return stopScheme;
    }

    /* Throws a ProxyAuthenticationException if the credentials required for NTLM authentication are invalid */
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
                sb.append("\" credential is required for NTLM authentication. ( The full list of required credentials is: ");

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

    /* Generates a (type1 or type 3) NTLM message (note: this is *not* the entire HTTP message)
     * 
     * ntlmMessage is the type2 message that was received from the server
     *             or is null to generate a type 1 message
     * username is the username to authenticate with
     * password is the password to authenticate with
     * localHostname is the local hostname (must be all caps per the NTLM specification)
     * domain is the NT domain to authenticate in
     * 
     * Returns the response.
     * 
     * Throws ProxyAuthenticationException if the messages cannot be retrieved.
     */
    private final String generateNtlmMessage(String ntlmMessage, String username, String password, String localHostname, String domain)
            throws ProxyAuthenticationException
    {
        try
        {
            if (localHostname == null)
            {
                localHostname = "LOCALHOST";
            }

            // note: per the NTLM specification, localHostname is always uppercase
            return _ntlmEgine.getResponseFor(ntlmMessage, username, password, localHostname.toUpperCase(), domain);
        }
        catch (Exception ex)
        {
            throw new ProxyAuthenticationException("Error generating NTLM response message.", ex);
        }
    }

    private String parseType2Message(int httpResponseCode, String response) throws ResponseCodeException
    {
        String type2Msg;

        Matcher matcher;

        if (httpResponseCode == 407)
        {
            matcher = PROXY_AUTHENTICATE_PATTERN.matcher(response);
        }
        else
        {
            matcher = WWW_AUTHENTICATE_PATTERN.matcher(response);
        }

        if (matcher.find() && matcher.groupCount() >= 1)
        {
            type2Msg = matcher.group(1);
        }
        else
        {
            throw new ResponseCodeException("Unable to parse type2 message from response.");
        }

        return type2Msg;
    }

    /* Demonstrates *just* the generation of valid NTLM messages, without actually performing any authentication */
    private static void messageGenerationDemo() 
    {
        // The NTLM Handshake is documented here:
        // http://www.innovation.ch/personal/ronald/ntlm.html
        //
        // NTLM Handshake
        // When a client needs to authenticate itself to a proxy or server using
        // the NTLM scheme then the following 4-way handshake takes place
        // (only parts of the request and status line and the relevant headers
        // are shown here; "C" is the client, "S" the server):
        //
        // 1: C --> S GET ...
        //
        // 2: C <-- S 401 Unauthorized
        // WWW-Authenticate: NTLM
        //
        // 3: C --> S GET ...
        // Authorization: NTLM <base64-encoded type-1-message>
        //
        // 4: C <-- S 401 Unauthorized
        // WWW-Authenticate: NTLM <base64-encoded type-2-message>
        //
        // 5: C --> S GET ...
        // Authorization: NTLM <base64-encoded type-3-message>
        //
        // 6: C <-- S 200 Ok

        try
        {
            final NTLMEngineImpl ntlmEgine = new NTLMEngineImpl();

            // generate the type 1 message
            String type1Msg = ntlmEgine.getResponseFor(null, "john.doe", "1234", "MyMachineName", "MyDomain");

            System.out.println("NLTM " + type1Msg);

            // the type 2 response contains the challenge
            String type2Msg = "TlRMTVNTUAACAAAACgAKADgAAAAFgomiOn57Is8wwS8AAAAAAAAAAMYAxgBCAAAABgGxHQAAAA9BAE0ARQBSAFMAAgAKAEEATQBFAFIAUwABABIATwBBAEsATABQAEMAMQAwADEABAAqAGEAbQBlAHIAcwAuAGkAbQBlAC4AcgBlAHUAdABlAHIAcwAuAGMAbwBtAAMAPgBvAGEAawBsAHAAYwAxADAAMQAuAGEAbQBlAHIAcwAuAGkAbQBlAC4AcgBlAHUAdABlAHIAcwAuAGMAbwBtAAUAHgBpAG0AZQAuAHIAZQB1AHQAZQByAHMALgBjAG8AbQAHAAgA0aaWQ1RyzAEAAAAA";

            String type3Msg = ntlmEgine.getResponseFor(type2Msg, "john.doe", "1234", "MyMachineName", "MyDomain");

            System.out.println("NLTM " + type3Msg);
        }
        catch (Exception e)
        {
            System.out.println(e);
        }
    }


    /**
     * The main method.
     *
     * @param args the arguments
     */
    public static void main(String[] args)
    {
        messageGenerationDemo(); // no authentication, just message generation
    }

}
