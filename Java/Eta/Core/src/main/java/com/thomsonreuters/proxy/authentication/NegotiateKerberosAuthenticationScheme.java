package com.thomsonreuters.proxy.authentication;

import javax.security.auth.Subject;
import javax.security.auth.login.LoginContext;

import java.security.PrivilegedActionException;
import java.util.Base64;

import org.apache.http.util.EncodingUtils;

public class NegotiateKerberosAuthenticationScheme extends KerberosAuthenticationScheme
{
    // Negotiate/Kerberos is called SPNEGO(=Simple and Protected GSSAPI Negotiation)
    
    // Example krb5 config file:
    //		[libdefaults]
    //			default_realm = EXAMPLE.COM
    //			default_tkt_enctypes = aes128-cts rc4-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc
    //			default_tgs_enctypes = aes128-cts rc4-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc
    //			permitted_enctypes   = aes128-cts rc4-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc
    //
    //		[realms]
    //			EXAMPLE.COM  = {
    //				kdc = kdc1.example.com 
    //				default_domain = EXAMPLE.COM 
    //		}
    //
    //		[domain_realm]
    //			.EXAMPLE.COM = EXAMPLE.COM

    private static final String NEGOTIATEKERBEROS_RESPONSE_PREFIX = "NEGOTIATE ";	

    /**
     * Instantiates a new negotiate kerberos authentication scheme.
     *
     * @param proxyAuthenticator the proxy authenticator
     * @throws NullPointerException the null pointer exception
     */
    protected NegotiateKerberosAuthenticationScheme(IProxyAuthenticator proxyAuthenticator)
            throws NullPointerException
    {
        super(proxyAuthenticator);
    }

    /* Processes a response from the proxy server
     * and returns a (http) "Proxy-authorization: " value (e.g. "NEGOTIATE dfdfakajas...") with a trailing \r\n
     * or returns an empty string if a "Proxy-authorization: " value does not need to be sent back to the proxy
     * 
     * httpResponseCode is the http response code to handle (e.g. 407)
     * proxyServerResponse is a response from the proxy server to process (may be null)
     * 
     * Throws ProxyAuthenticationException (an exception that halted the authentication process occurred)
     */
    @Override
    public String processResponse(int httpResponseCode, String proxyServerResponse)
            throws ProxyAuthenticationException
    {
        StringBuilder proxyAuthorizationValue = new StringBuilder();
        String credentials = EncodingUtils.getAsciiString(Base64.getEncoder().encode(serviceTicket));

        if (httpResponseCode == 407)
            proxyAuthorizationValue.append(PROXY_AUTHORIZATION_PREFIX);
        else
            proxyAuthorizationValue.append(AUTHORIZATION_PREFIX);

        proxyAuthorizationValue.append(NEGOTIATEKERBEROS_RESPONSE_PREFIX);
        proxyAuthorizationValue.append(credentials);
        proxyAuthorizationValue.append(EOL);
        return proxyAuthorizationValue.toString();
    }
	
    public String name()
    {
        return "NEGOTIATE";
    }
	
    /* Throws a ProxyAuthenticationException if the credentials required for Negotiate/Kerberos authentication are invalid */
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
                sb.append("\" credential is required for Negotiate/Kerberos authentication. ( The full list of required credentials is: ");

                for (String required : RequiredCredentials)
                {
                    sb.append(required);
                    sb.append(" ");
                }

                sb.append(")");

                throw new ProxyAuthenticationException(sb.toString());
            }
        }

        // do AS and TGS exchanges upfront here
        try
        {
            // the final outcome of this method is getting the Kerberos service ticket
            // after doing the necessary AS and TGS exchanges according to the Kerberos specification
            doASandTGSexchanges();
        }
        catch (Exception e)
        {
            stopScheme = true;
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("Error while attempting to get Negotiate/Kerberos service ticket:  " + e.toString());
            throw new ProxyAuthenticationException("Error while attempting to get Negotiate/Kerberos service ticket:  " + e.toString());
        }
    }

    public byte[] getKerberosServiceTicket(String userName, String domain, String service, String server, LoginContext loginContext)
            throws PrivilegedActionException
    {
        //
        // 1st: get TGT (Ticket Granting Ticket that should now be in loginContext)
        Subject subject = loginContext.getSubject(); // TGT (Ticket Granting Ticket)
        // output TGT info
        // System.out.println("\nTicket Granting Ticket ==" + subject);

        //
        // 2nd: get Kerberos service ticket, i.e. perform Kerberos
        // Ticket-Granting Server (TGS) exchange
        byte[] serviceTicket = null;
        @SuppressWarnings("unchecked")
        Object doAs = Subject.doAs(subject, new KerberosServiceTicketGenerator("SPNEGO", userName, domain, service, server));
        serviceTicket = (byte[])doAs; // Subject.doAs returns the Kerberos service ticket as an array of encrypted bytes

        return serviceTicket;
    }

}
