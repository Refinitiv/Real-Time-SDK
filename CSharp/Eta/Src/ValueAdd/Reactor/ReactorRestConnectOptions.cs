/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorRestConnectOptions
    {
        public ReactorDiscoveryTransportProtocol Transport { get; set; }

        public ReactorDiscoveryDataFormatProtocol DataFormat { get; set; }

        public ProxyOptions ProxyOptions { get; set; } = new ProxyOptions();

        //public string? TokenServiceURL { get; set; }

        public ReactorOptions ReactorOptions { get; private set; }

        // Temporary redirect flags/locations
        public bool AuthRedirect { get; set; }

        public string? AuthRedirectLocation { get; set; }

        public bool DiscoveryRedirect { get; set; }

        public string? DiscoveryRedirectLocation { get; set; }

        public ReactorRestLogginHandler RestLoggingHandler { get; private set; }

        public ReactorRestConnectOptions(ReactorOptions options)
        {
            ReactorOptions = options;

            Clear();

            if (ReactorOptions.RestProxyOptions.IsHostAndPortSet)
            {
                ReactorOptions.RestProxyOptions.CopyTo(ProxyOptions);
            }

            RestLoggingHandler = new ReactorRestLogginHandler(options);
        }

        public void ClearAuthRedirectParamerters()
        {
            AuthRedirect = false;
            AuthRedirectLocation = null;
        }

        public void ClearDiscoveryRedirectParameters()
        {
            DiscoveryRedirect = false;
            DiscoveryRedirectLocation = null;
        }

        public void Clear()
        {
            Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
            DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_RWF;
            ProxyOptions.Clear();

            AuthRedirect = false;
            AuthRedirectLocation = null;
            DiscoveryRedirect = false;
            DiscoveryRedirectLocation = null;
        }

        public void ApplyServiceDiscoveryOptions(ReactorServiceDiscoveryOptions options)
        {
            DataFormat = options.DataFormat;
            Transport = options.Transport;

            if (!ProxyOptions.IsHostAndPortSet)
            {
                ProxyOptions.ProxyHostName = options.ProxyHostName.ToString();
                ProxyOptions.ProxyPort = options.ProxyPort.ToString();
                ProxyOptions.ProxyUserName = options.ProxyUserName.ToString();
                ProxyOptions.ProxyPassword = options.ProxyPassword.ToString(); 
            }
        }

        public void ApplyProxyInfo(ConnectOptions connectOptions)
        {
            if (ProxyOptions.IsHostAndPortSet)
                return;

            connectOptions.ProxyOptions.CopyTo(ProxyOptions);
        }
    }
}
