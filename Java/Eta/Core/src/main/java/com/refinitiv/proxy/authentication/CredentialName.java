/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

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
