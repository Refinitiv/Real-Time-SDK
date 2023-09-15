﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This is the option to be used in the <see cref="Reactor.CreateReactor(ReactorOptions, out ReactorErrorInfo?)"/> call.
    /// </summary>
    sealed public class ReactorOptions
    {
        string m_ServiceDiscoveryUrl = string.Empty;
        string m_TokenServiceUrl = string.Empty;
        int m_RestRequestTimeout;
        double m_TokenExpireRatio;

        /// <summary>
        /// Gets or sets a user defined object that can be useful for identifying a specific instance
        /// of a <see cref="Reactor"/> or coupling this <see cref="Reactor"/> with other user created
        /// information.
        /// </summary>
        public Object? UserSpecObj { get; set; }

        /// <summary>
        /// Gets or sets whether to enable XML tracing for the <see cref="Reactor"/>.
        /// </summary>
        public bool XmlTracing { get; set; }

        /// <summary>
        /// Gets or sets an output stream for logging REST request and response. Defaults to standard output.
        /// </summary>
        public Stream? RestLogOutputStream { get; set; }

        /// <summary>
        /// Gets or sets to enable logging REST request and response to an output stream. Defaults to <c>false</c>
        /// </summary>
        public bool EnableRestLogStream { get; set; }

        /// <summary>
        /// Create <see cref="ReactorOptions"/>
        /// </summary>
        public ReactorOptions()
        {
            Clear();
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            UserSpecObj = null;
            XmlTracing = false;
            m_TokenServiceUrl = "https://api.refinitiv.com/auth/oauth2/v2/token";
            m_ServiceDiscoveryUrl = "https://api.refinitiv.com/streaming/pricing/v1/";
            m_RestRequestTimeout = 45000; // 45 seconds
            m_TokenExpireRatio = 0.01;
            EnableRestLogStream = false;
        }

        /// <summary>
        /// Sets a URL for the RDP service discovery
        /// </summary>
        /// <param name="serviceDiscoveryURL">The URL for the RDP service discovery</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success, if the specified URL is null 
        /// or empty <see cref="ReactorReturnCode.PARAMETER_INVALID"/></returns>
        public ReactorReturnCode SetServiceDiscoveryURL(string serviceDiscoveryURL)
        {
            if(serviceDiscoveryURL is null || serviceDiscoveryURL.Length == 0)
            {
                return ReactorReturnCode.PARAMETER_INVALID;
            }

            m_ServiceDiscoveryUrl = serviceDiscoveryURL;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Gets a URL for the RDP service discovery
        /// </summary>
        /// <returns>The service discovery URL</returns>
        public string GetServiceDiscoveryURL()
        {
            return m_ServiceDiscoveryUrl;
        }

        /// <summary>
        /// Sets a URL of the token service to get an access token. 
        /// This is used for querying RDP service discovery and subscribing data from RDP.
        /// </summary>
        /// <param name="tokenServiceURL">The URL for the token service</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success, if the specified URL is null 
        /// or empty <see cref="ReactorReturnCode.PARAMETER_INVALID"/></returns>
        public ReactorReturnCode SetTokenServiceURL(string tokenServiceURL)
        {
            if(tokenServiceURL is null || tokenServiceURL.Length == 0)
            {
                return ReactorReturnCode.PARAMETER_INVALID;
            }

            m_TokenServiceUrl = tokenServiceURL;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Gets a URL of the token service to get an access token.
        /// This is used for querying RDP service discovery and subscribing data from RDP.
        /// </summary>
        /// <returns>The token service URL</returns>
        public string GetTokenServiceURL()
        {
            return m_TokenServiceUrl;
        }

        /// <summary>
        /// Sets a maximum time(millisecond) for the REST requests is allowed to take for token service and service discovery.
        /// The default request timeout is 45000 milliseconds.
        /// </summary>
        /// <param name="requestTimeout">The request timeout in millisecond</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success, if the request timeout is less than 
        /// or equal to zero <see cref="ReactorReturnCode.PARAMETER_INVALID"/></returns>
        public ReactorReturnCode SetRestRequestTimeout(int requestTimeout)
        {
            if(requestTimeout <= 0)
            {
                return ReactorReturnCode.PARAMETER_INVALID;
            }

            m_RestRequestTimeout = requestTimeout;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Gets the maximum time for the REST requests is allowed to take for token service and service discovery.
        /// </summary>
        /// <returns>The REST request timeout(millisecond)</returns>
        public int GetRestRequestTimeout()
        {
            return m_RestRequestTimeout;
        }

        /// <summary>
        /// Sets a ratio to multiply with access token validity time(second) to specify when the access is about to expire.
        /// The default token exipred ratio is 0.01. The valid range is between 0.01 to 0.90.
        /// </summary>
        /// <param name="tokenExpireRatio">The token expire ratio</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success, otherwise <see cref="ReactorReturnCode.PARAMETER_INVALID"/>
        /// if the specified ratio is out of range.</returns>
        public ReactorReturnCode SetTokenExpireRatio(double tokenExpireRatio)
        {
            if(tokenExpireRatio < 0.01 || tokenExpireRatio > 0.90)
            {
                return ReactorReturnCode.PARAMETER_INVALID;
            }

            m_TokenExpireRatio = tokenExpireRatio;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns the ratio to multiply with access token validity time(second) to specify when the access is about to expire.
        /// </summary>
        /// <returns>The token expire ratio</returns>
        public double GetTokenExpireRatio()
        {
            return m_TokenExpireRatio;
        }

        /// <summary>
        /// Performs a deep copy from a specified ReactorOptions into this ReactorOptions.
        /// </summary>
        /// <param name="options">The option to copy from</param>
        internal void Copy(ReactorOptions options)
        {
            UserSpecObj = options.UserSpecObj;
            XmlTracing = options.XmlTracing;
            m_TokenServiceUrl = options.m_TokenServiceUrl;
            m_ServiceDiscoveryUrl = options.m_ServiceDiscoveryUrl;
            m_RestRequestTimeout = options.m_RestRequestTimeout;
            m_TokenExpireRatio = options.m_TokenExpireRatio;
            EnableRestLogStream = options.EnableRestLogStream;
            RestLogOutputStream = options.RestLogOutputStream;
        }
    }
}
