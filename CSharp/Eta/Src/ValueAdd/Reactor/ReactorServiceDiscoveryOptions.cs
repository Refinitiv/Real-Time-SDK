﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This is the service discovery options to be used in the <see cref="Reactor.QueryServiceDiscovery(ReactorServiceDiscoveryOptions, out ReactorErrorInfo?)"/>
    /// to get endpoint informaton from the service discovery.
    /// </summary>
    public class ReactorServiceDiscoveryOptions
    {
        private Buffer m_ClientId = new Buffer();
        private Buffer m_ClientSecret = new Buffer();
        private Buffer m_ProxyHostName = new Buffer();
        private Buffer m_ProxyPort = new Buffer();
        private Buffer m_ProxyUserName = new Buffer();
        private Buffer m_ProxyPassword = new Buffer();

        public ReactorServiceDiscoveryOptions()
        {
            Clear();
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void Clear()
        {
            m_ClientId.Clear();
            m_ClientSecret.Clear();
            Transport = ReactorDiscoveryTransportProtocol.RD_TP_INIT;
            DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_INIT;
            m_ProxyHostName.Clear();
            m_ProxyPort.Clear();
            m_ProxyUserName.Clear();
            m_ProxyPassword.Clear();
        }

        /// <summary>
        /// Gets or sets the unique ID defined for an application making a request to the token service.
        /// </summary>
        public Buffer ClientId 
        {
            get { return m_ClientId; }

            set { m_ClientId.Data(value.Data(), value.Position, value.Length); }
        }

        /// <summary>
        /// Gets or sets the client secret defined for an application making a request to the token service.
        /// </summary>
        public Buffer ClientSecret
        {
            get { return m_ClientSecret; }

            set { m_ClientSecret.Data(value.Data(), value.Position, value.Length); }
        }

        /// <summary>
        /// Gets or sets an optional parameter to specify the desired transport protocol to get
        /// service endpoints from the service discovery. 
        /// </summary>
        public ReactorDiscoveryTransportProtocol Transport { get; set; }

        /// <summary>
        /// Gets or sets an optional parameter to specify the desired data format protocol to get
        /// service endpoints from the service discovery. 
        /// </summary>
        public ReactorDiscoveryDataFormatProtocol DataFormat { get; set; }

        /// <summary>
        /// Gets or sets the IP address or hostname of the HTTP proxy server.
        /// </summary>
        public Buffer ProxyHostName
        {
            get { return m_ProxyHostName; }

            set { m_ProxyHostName.Data(value.Data(), value.Position, value.Length); }
        }

        /// <summary>
        /// Gets or sets the Port Number of the HTTP proxy server.
        /// </summary>
        public Buffer ProxyPort
        {
            get { return m_ProxyPort; }

            set { m_ProxyPort.Data(value.Data(), value.Position, value.Length); }
        }

        /// <summary>
        /// Gests or sets a proxy user name to authentication with a proxy server
        /// </summary>
        public Buffer ProxyUserName
        {
            get { return m_ProxyUserName; }

            set { m_ProxyUserName.Data(value.Data(), value.Position, value.Length); }
        }

        /// <summary>
        /// Gests or sets a proxy password to authentication with a proxy server
        /// </summary>
        public Buffer ProxyPassword
        {
            get { return m_ProxyPassword; }

            set { m_ProxyPassword.Data(value.Data(), value.Position, value.Length); }
        }

        /// <summary>
        /// Gets or sets a service dicovery endpoint callback to get endpoint information.
        /// </summary>
        public IReactorServiceEndpointEventCallback? ReactorServiceEndpointEventCallback { get; set; }

        /// <summary>
        /// Gets or sets a user defined object.
        /// </summary>
        public object? UserSpecObject { get; set; }
    }
}
