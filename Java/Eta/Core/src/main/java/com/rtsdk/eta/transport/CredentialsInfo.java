package com.rtsdk.eta.transport;

/**
 * Options used for configuring proxy credentials, which might be needed during a tunneling connection.
 * 
 * Supported authentication protocols are: Negotiate/Kerberos, Kerberos, NTLM, and Basic.
 * 
 * Protocols Negotiate/Kerberos or Kerberos require the following options:
 *             HTTPproxyUsername, HTTPproxyPasswd, HTTPproxyDomain, and HTTPproxyKRB5configFile  
 *                                                             
 * Protocol NTLM requires the following options:
 *             HTTPproxyUsername, HTTPproxyPasswd, HTTPproxyDomain, and HTTPproxyLocalHostname
 *             
 * Protocol Basic requires the following options:
 *             HTTPproxyUsername and HTTPproxyPasswd
 * 
 * @see ConnectOptions
 */
public interface CredentialsInfo
{
    
    /**
     * The username to authenticate.
     * Needed for all authentication protocols
     *
     * @param HTTPproxyUsername the HTT pproxy username
     */
    public void HTTPproxyUsername(String HTTPproxyUsername);

    /**
     * The username to authenticate.
     * Needed for all authentication protocols.
     * 
     * @return the HTTPproxyUsername
     */
    public String HTTPproxyUsername();

    /**
     * The password to authenticate.
     * Needed for all authentication protocols.
     *
     * @param HTTPproxyPasswd the HTT pproxy passwd
     */
    public void HTTPproxyPasswd(String HTTPproxyPasswd);

    /**
     * The password to authenticate.
     * Needed for all authentication protocols.
     * 
     * @return the HTTPproxyPasswd
     */
    public String HTTPproxyPasswd();
    
    /**
     * The domain of the user to authenticate.
     * Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.
     * 
     * For Negotiate/Kerberos or for Kerberos authentication protocols, HTTPproxyDomain
     * should be the same as the domain in the 'realms' and 'domain_realm' sections of
     * the Kerberos configuration file ({@link #HTTPproxyKRB5configFile()}).
     *
     * @param HTTPproxyDomain the HTT pproxy domain
     */
    public void HTTPproxyDomain(String HTTPproxyDomain);

    /**
     * 
     * The domain of the user to authenticate.
     * Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.
     * 
     * For Negotiate/Kerberos or for Kerberos authentication protocols, HTTPproxyDomain
     * should be the same as the domain in the 'realms' and 'domain_realm' sections of
     * the Kerberos configuration file ({@link #HTTPproxyKRB5configFile()}).
     * 
     * @return the HTTPproxyDomain
     */
    public String HTTPproxyDomain();
    
    /**
     * The local hostname of the client.
     * Needed for NTLM authentication protocol only.
     *
     * @param HTTPproxyLocalHostname the HTT pproxy local hostname
     */
    public void HTTPproxyLocalHostname(String HTTPproxyLocalHostname);

    /**
     * 
     * The local hostname of the client.
     * Needed for NTLM authentication protocol only.
     * 
     * @return the HTTPproxyLocalHostname
     */
    public String HTTPproxyLocalHostname();
    
    /**
     * The complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or custom file).
     * Needed for Negotiate/Kerberos and Kerberos authentications.
     * 
     * The default locations could be the following:
     * Windows: c:\winnt\krb5.ini or c:\windows\krb5.ini
     * Linux: /etc/krb5.conf 
     * Other Unix: /etc/krb5/krb5.conf
     *
     * @param HTTPproxyKRB5configFile the HTT pproxy KRB 5 config file
     */
    public void HTTPproxyKRB5configFile(String HTTPproxyKRB5configFile);

    /**
     * 
     * The complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or custom file).
     * Needed for Negotiate/Kerberos and Kerberos authentications.
     * 
     * The default locations could be the following:
     * Windows: c:\winnt\krb5.ini or c:\windows\krb5.ini
     * Linux: /etc/krb5.conf
     * Other Unix: /etc/krb5/krb5.conf
     * 
     * @return the HTTPproxyKRB5configFile
     */
    public String HTTPproxyKRB5configFile();
}