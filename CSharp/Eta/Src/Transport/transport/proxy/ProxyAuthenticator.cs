/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace Refinitiv.Eta.Transports.proxy
{
    internal class ProxyAuthenticator
    {
        public Credentials Credentials { get; private set; }
        public string ProxyHost { get; private set; }

        public bool IsAuthenticated { get; private set; } = false;
        private const string END_OF_CHUNK = "\r\n\r\n";

        private IAuthenticationScheme m_AuthScheme;
        private static readonly Regex HTTP_VERSION_PATTERN = new Regex("^\\s*HTTP/1.. (\\d+)");
        private static readonly Regex PROXY_AUTHENTICATE_PATTERN = new Regex("Proxy-[a|A]uthenticate: (\\w+)");
        private static readonly Regex WWW_AUTHENTICATE_PATTERN = new Regex("WWW-[a|A]uthenticate: (\\w+)");
        private static readonly Regex PROXY_CONNECTION_CLOSE_PATTERN = new Regex("Proxy-[c|C]onnection: [c|C]lose");
        private static readonly Regex CONNECTION_CLOSE_PATTERN = new Regex("Connection: [c|C]lose");
        private const int NOT_FOUND = -1;
        private const string NEGOTIATEKERBEROS_SCHEME_NAME = "NEGOTIATE"; // SPNEGO
        private const string KERBEROS_SCHEME_NAME = "KERBEROS"; // KERBEROS5
        private const string NTLM_SCHEME_NAME = "NTLM";
        private const string BASIC_SCHEME_NAME = "BASIC";

        public ProxyAuthenticator(Credentials credentials, string proxyHost)
        {
            Credentials = credentials;
            ProxyHost = proxyHost;
        }

        public ProxyAuthenticatorResponse ProcessResponse(string response)
        {
            bool isProxyConnectionClose = false;
            ProxyAuthenticatorResponse authResponse;

            if(IsAuthenticated || response is null || response.Length == 0)
            {
                authResponse = new ProxyAuthenticatorResponse(false, ""); // nothing to do
            }

            int httpResponseCode = ParseHttpResponseCode(response);

            if(httpResponseCode == 200)
            {
                IsAuthenticated = true;
                authResponse = new ProxyAuthenticatorResponse(false, ""); // nothing to do
            }
            else if (httpResponseCode == 407 || httpResponseCode == 401)
            {
                if(ResponseContainsProxyClose(response) || ResponseContainsConnectionClose(response))
                {
                    isProxyConnectionClose = true; // The proxy requested to close the connection.
                }

                if(IsAuthenticated)
                {
                    // we were authenticated, but we must restart the authentication process.
                    IsAuthenticated = false;
                    m_AuthScheme = null;
                }

                if(m_AuthScheme is null)
                {
                    CreateAuthenticationSchemeFromResponse(httpResponseCode, response);
                }

                authResponse = new ProxyAuthenticatorResponse(isProxyConnectionClose, m_AuthScheme.ProcessResponse(httpResponseCode, response));
            }
            else
            {
                throw new ResponseCodeException(GetType().Name + $": Unexpected http response code: {httpResponseCode}");
            }

            return authResponse;
        }

        private List<string> ParseAuthenticationSchemes(int httpResponseCode, string response)
        {
            List<string> authenticationSchemes = new List<string>();

            MatchCollection matcher;

            if (httpResponseCode == 407)
            {
                matcher = PROXY_AUTHENTICATE_PATTERN.Matches(response);
            }
            else
            {
                matcher = WWW_AUTHENTICATE_PATTERN.Matches(response);
            }

            int index = 0;

            while (matcher.Count > 0 && index != matcher.Count)
            {
                Match match = matcher[index];
                GroupCollection group = match.Groups;
                if (group.Count >= 1)
                {
                    authenticationSchemes.Add(group[1].ToString());
                }

                ++index;
            }

            return authenticationSchemes;
        }

        private void CreateAuthenticationSchemeFromResponse(int httpResponseCode, string response)
        {
            List<string> authenticationsSchemes = ParseAuthenticationSchemes(httpResponseCode, response);

            IAuthenticationScheme authenticatonScheme = null;

            foreach (string authScheme in authenticationsSchemes)
            {
                /* Supports only Basic proxy authentication. */
                if (string.Equals(BASIC_SCHEME_NAME, authScheme, System.StringComparison.OrdinalIgnoreCase))
                {
                    authenticatonScheme = new BasicAuthenticationSchem(this);
                    break;
                }
            }

            if (authenticatonScheme is null)
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("Unable to instantiate an authtentication scheme from the list of schemes supported by the proxy ( ");
                foreach(string authScheme in authenticationsSchemes)
                {
                    sb.Append(authScheme);
                    sb.Append(" ");
                }
                sb.Append(").");

                throw new ProxyAuthenticationException(sb.ToString());
            }

            m_AuthScheme = authenticatonScheme;
        }

        private bool ResponseContainsProxyClose(string response)
        {
            MatchCollection matcher = PROXY_CONNECTION_CLOSE_PATTERN.Matches(response);
            return matcher.Count > 0;
        }

        private bool ResponseContainsConnectionClose(string response)
        {
            MatchCollection matcher = CONNECTION_CLOSE_PATTERN.Matches(response);
            return matcher.Count > 0;
        }

        private int ParseHttpResponseCode(string response)
        {
            int httpResponseCode = NOT_FOUND;

            if(response is not null)
            {
                string[] responseChunks = response.Split(END_OF_CHUNK);
                if(responseChunks is not null)
                {
                    foreach(string chunk in responseChunks)
                    {
                        MatchCollection matcher = HTTP_VERSION_PATTERN.Matches(chunk);
                        
                        if(matcher.Count > 0)
                        {
                            Match match = matcher[0];
                            GroupCollection group = match.Groups;
                            httpResponseCode = int.Parse(group[1].ToString());
                            break;
                        }
                    }
                }
            }

            if(httpResponseCode == NOT_FOUND)
            {
                throw new ResponseCodeException("Unable to parse HTTP response code. (Authentication errors may have occurred.)");
            }

            return httpResponseCode;
        }
    }
}
