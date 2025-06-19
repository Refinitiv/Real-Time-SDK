/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

import java.util.List;

public class AuthenticationSchemeFactory
{
    private static final String NEGOTIATEKERBEROS_SCHEME_NAME = "NEGOTIATE"; // SPNEGO
    private static final String KERBEROS_SCHEME_NAME = "KERBEROS"; // KERBEROS5
    private static final String NTLM_SCHEME_NAME = "NTLM";
    private static final String BASIC_SCHEME_NAME = "BASIC";

    private static boolean valid;

    /**
     * Creates the.
     *
     * @param authenticationSchemes the authentication schemes
     * @param authenticator the authenticator
     * @return the i authentication scheme
     * @throws ProxyAuthenticationException the proxy authentication exception
     */
    public static IAuthenticationScheme create(List<String> authenticationSchemes, IProxyAuthenticator authenticator)
            throws ProxyAuthenticationException
    {
        IAuthenticationScheme authenticatonScheme = null;
        valid = false;

        for (String authScheme : authenticationSchemes)
        {
            if (!authenticator.hasNegotiateFailed())
            {
                if (NEGOTIATEKERBEROS_SCHEME_NAME.compareToIgnoreCase(authScheme) == 0) // SPNEGO
                {
                    authenticatonScheme = new NegotiateKerberosAuthenticationScheme(authenticator);
                    if (!checkCredentials(authenticatonScheme))
                    {
                        authenticator.setNegotiateFailed();
                        continue;
                    }
                }
            }

            if (!authenticator.hasKerberosFailed())
            {
                if (KERBEROS_SCHEME_NAME.compareToIgnoreCase(authScheme) == 0) // Kerberos5
                {
                    authenticatonScheme = new KerberosAuthenticationScheme(authenticator);
                    if (!checkCredentials(authenticatonScheme))
                    {
                        authenticator.setKerberosFailed();
                        continue;
                    }
                }
            }

            if (!authenticator.hasNTLMfailed())
            {
                if (NTLM_SCHEME_NAME.compareToIgnoreCase(authScheme) == 0) // NTLM
                {
                    authenticatonScheme = new NtlmAuthenticationScheme(authenticator);
                    if (!checkCredentials(authenticatonScheme))
                    {
                        authenticator.setNTLMfailed();
                        continue;
                    }
                }
            }

            if (BASIC_SCHEME_NAME.compareToIgnoreCase(authScheme) == 0) // Basic
            {
                authenticatonScheme = new BasicAuthenticationScheme(authenticator);
                break; // for Basic authentication scheme don't do validateCredentials() below
            }

            if (valid)
                break;
        }

        if (authenticatonScheme == null)
        {
            StringBuilder sb = new StringBuilder();
            sb.append("Unable to instantiate an authtentication scheme from the list of schemes supported by the proxy ( ");
            for (String authScheme : authenticationSchemes)
            {
                sb.append(authScheme);
                sb.append(" ");
            }
            sb.append(").");

            throw new ProxyAuthenticationException(sb.toString());
        }

        return authenticatonScheme;
    }
	
    private static boolean checkCredentials(IAuthenticationScheme authenticatonScheme)
    {
        // check credentials and if valid then use the authentication Scheme 'authenticatonScheme'
        if (authenticatonScheme != null)
        {
            valid = true;
            try
            {
                authenticatonScheme.validateCredentials();
            }
            catch (ProxyAuthenticationException e)
            {
                valid = false;
                System.err.println(e.getMessage());
                return false;
            }
        }
        return true;
    }
}
