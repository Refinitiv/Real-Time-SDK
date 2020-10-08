package com.refinitiv.proxy.authentication;

public class CredentialName
{
    /* The domain of the user to authenticate */
    public static final String DOMAIN = "domain";

    /* The username to authenticate */
    public static final String USERNAME = "username";

    /* The password to authenticate */
    public static final String PASSWORD = "password";

    /* The local hostname (server name) */
    public static final String LOCAL_HOSTNAME = "localhostname";

    /* Complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or ?).
     * Needed for Negotiate/Kerberos and Kerberos authentications.
     * The default locations could be the following:
     * Windows:    c:\winnt\krb5.ini or c:\windows\krb5.ini
     * Linux:      /etc/krb5.conf
     * Other Unix: /etc/krb5/krb5.conf
     */
    public static final String KRB5_CONFIG_FILE = "krb5.ini";
}
