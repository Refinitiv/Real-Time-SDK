/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Transports.proxy
{
    internal class ProxyAuthenticatorResponse
    {
        public ProxyAuthenticatorResponse(bool isProxyConnectionClose, string proxyAuthorization)
        {
            IsProxyConnectionClose = isProxyConnectionClose;
            ProxyAuthorization = proxyAuthorization;
        }

        public bool IsProxyConnectionClose { get; private set; }

        public string ProxyAuthorization { get ; private set; } 
    }
}
