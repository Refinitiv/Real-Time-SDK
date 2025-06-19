/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

public interface IProxyAuthenticator
{
    /* Sets the credentials used to authenticate */
    void setCredentials(ICredentials credentials) throws NullPointerException;

    /* Returns the credentials used to authenticate */
    ICredentials getCredentials();

    /* Returns the proxy hostname used to authenticate */
    String getProxyHost();

    /* Processes a response from the proxy server 
     * and returns a (http) "Proxy-authorization" value (e.g. "NTLM TlRMTVNTUA...") with NO TRAILING \r\n
     * or returns an empty string if a "Proxy-authorization" value does not need to be sent back to the proxy
     * 
     * proxyServerResponse is a response from the proxy server to process (may be null)
     * 
     * Throws ResponseCodeException (the proxy server sent a response containing a missing, invalid, or unexpected response code)
     * Throws ProxyAuthenticationException (an exception that halted the authentication process occurred)
     */
    IProxyAuthenticatorResponse processResponse(String proxyServerResponse)
            throws ResponseCodeException, ProxyAuthenticationException;

    /* Returns true if the proxy has returned a response indicating authentication was successful */
    boolean isAuthenticated();
	
    /* Set if the Negotiate authentication scheme has failed. */
    void setNegotiateFailed();
	
    /* Set if the Negotiate authentication scheme has failed. */
    void setKerberosFailed();
	
    /* Set if the Negotiate authentication scheme has failed. */
    void setNTLMfailed();

    /* Returns true if Negotiate authentication scheme has failed. */
    boolean hasNegotiateFailed();

    /* Returns true if Kerberos authentication scheme has failed. */
    boolean hasKerberosFailed();
	
    /* Returns true if NTLM authentication scheme has failed. */
    boolean hasNTLMfailed();
}
