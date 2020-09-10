package com.rtsdk.proxy.authentication;

public interface IAuthenticationScheme
{
    /* Returns the IProxyAuthenticator associated with this authentication scheme */
    IProxyAuthenticator getProxyAuthenicator();

    /* Processes a response from the proxy server
     * and returns a (http) "Proxy-authorization: " value (e.g. "NTLM TlRMTVNTUA...") with a trailing \r\n
     * or returns an empty string if a "Proxy-authorization: " value does not need to be sent back to the proxy
     * 
     * httpResponseCode is the http response code to handle (e.g. 407)
     * proxyServerResponse is a response from the proxy server to process (may be null)
     *            
     * Throws ProxyAuthenticationException (an exception that halted the authentication process occurred)
     */
    String processResponse(int httpResponseCode, String proxyServerResponse) throws ProxyAuthenticationException;
	
    /* Validate credentials to be used for this authentication scheme. */
    void validateCredentials() throws ProxyAuthenticationException;

    /* Returns the name of this authentication scheme (e.g. "NEGOTIATE", "KERBEROS", "NTLM", BASIC") */
    String name();

    /* Returns true if we have stopped processing to responses from the proxy server.
     * This is useful in the NTLM scheme, where for example if the domain credential is incorrect,
     * then after a certain number of attempts you will stop processing responses from the proxy server.
     * 
     * Returns true if the scheme has reached the maximum number of attempts to process a response from the proxy server.
     */
    boolean stopScheme();
}
