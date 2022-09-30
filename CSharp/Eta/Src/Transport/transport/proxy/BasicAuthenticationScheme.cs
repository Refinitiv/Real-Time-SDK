/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Text;

namespace Refinitiv.Eta.Transports.proxy
{
    internal class BasicAuthenticationSchem : IAuthenticationScheme
    {
        private ProxyAuthenticator m_ProxyAuthenticator;
        private static readonly string[] RequiredCredentials = { CredentialName.USERNAME, CredentialName.PASSWORD};

        private const string PROXY_AUTHORIZATION_PREFIX = "Proxy-Authorization: ";
        private const string AUTHORIZATION_PREFIX = "Authorization: ";
        private const string BASIC_RESPONSE_PREFIX = "BASIC ";
        private const string EOL= "\r\n";

        public BasicAuthenticationSchem(ProxyAuthenticator proxyAuthenticator)
        {
            m_ProxyAuthenticator = proxyAuthenticator;
        }

        public string GetName()
        {
            return "BASIC";
        }

        public ProxyAuthenticator GetProxyAuthenticator()
        {
            return m_ProxyAuthenticator;
        }

        public string ProcessResponse(int httpResponseCode, string proxyServerResponse)
        {
            StringBuilder proxyAuthorizationValue = new StringBuilder();

            string credentails = $"{m_ProxyAuthenticator.Credentials.GetKeyValue(CredentialName.USERNAME)}" +
                $":{m_ProxyAuthenticator.Credentials.GetKeyValue(CredentialName.PASSWORD)}";

            credentails = System.Convert.ToBase64String(Encoding.ASCII.GetBytes(credentails));

            ValidateCredentials();

            if(httpResponseCode == 407)
            {
                proxyAuthorizationValue.Append(PROXY_AUTHORIZATION_PREFIX);
            }
            else
            {
                proxyAuthorizationValue.Append(AUTHORIZATION_PREFIX);
            }

            proxyAuthorizationValue.Append(BASIC_RESPONSE_PREFIX);
            proxyAuthorizationValue.Append(credentails);
            proxyAuthorizationValue.Append(EOL);

            return proxyAuthorizationValue.ToString();
        }

        public void ValidateCredentials()
        {
            foreach(string credentialName in RequiredCredentials)
            {
                if(!m_ProxyAuthenticator.Credentials.HasKey(credentialName))
                {
                    StringBuilder sb = new StringBuilder();
                    sb.Append(GetType().Name);
                    sb.Append(": The \"");
                    sb.Append(credentialName);
                    sb.Append("\" credential is required for Basic authentication. ( The full list of required credentials is: ");

                    foreach (string required in RequiredCredentials)
                    {
                        sb.Append(required);
                        sb.Append(" ");
                    }

                    sb.Append(")");

                    throw new ProxyAuthenticationException(sb.ToString());
                }
            }
        }
    }
}
