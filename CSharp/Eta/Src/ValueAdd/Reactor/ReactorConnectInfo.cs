/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Reactor connection information for use in <see cref="ReactorConnectOptions"/>
    /// </summary>
    public class ReactorConnectInfo
    {
        const int DEFAULT_TIMEOUT = 60;

        private int m_InitTimeout;

        /// <summary>
        /// Creates ReactorConnectInfo
        /// </summary>
        public ReactorConnectInfo()
        {
            ConnectOptions = new ConnectOptions();
            m_InitTimeout = DEFAULT_TIMEOUT;
            EnableSessionManagement = false;
            Location = "us-east-1";
        }

        /// <summary>
        /// Gets the <see cref="Transports.ConnectOptions"/>, which is the ConnectOptions
        /// associated with the underlying <see cref="Transport.Connect(ConnectOptions, out Error)"/> method.
        /// This includes information about the host or network to connect to, the type of connection to use,
        /// and other transport specific configuration information.
        /// </summary>
        public ConnectOptions ConnectOptions { get; private set; }

        /// <summary>
        /// Gets or sets the amount of time (in seconds) to wait for the successful initialization
        /// <see cref="ReactorChannel"/>. If initialization does not complete in time, an event is dispatched 
        /// to the application to indicate that the ReactorChannel is down. Timeout must be greater than zero.
        /// Default is 60 seconds.
        /// </summary>
        /// <param name="timeout">the initialization timeout in seconds</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if the timeout is valid, otherwise 
        /// <see cref="ReactorReturnCode.PARAMETER_OUT_OF_RANGE"/> if timeout is out of range</returns>
        public ReactorReturnCode SetInitTimeout(int timeout)
        {
            if (timeout < 1)
                return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;

            m_InitTimeout = timeout;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns the initialization timeout value.
        /// </summary>
        /// <returns>the initialization timeout value</returns>
        public int GetInitTimeout()
        {
            return m_InitTimeout;
        }

        /// <summary>
        /// Gets or sets whether the session management is enabled.
        /// <para>
        /// Enabling this option indicates Reactor to get an endpoint from the RDP service discovery
        /// if both host and port are not spcified by users. 
        /// </para>
        /// </summary>
        public bool EnableSessionManagement { get; set; }

        /// <summary>
        /// Gets or sets <see cref="IReactorAuthTokenEventCallback"/> that receives <see cref="ReactorAuthTokenEvent"/>.
        /// The token is requested by the Reactor for Consumer applications to send login request and reissue with the token.
        /// </summary>
        public IReactorAuthTokenEventCallback? ReactorAuthTokenEventCallback { get; set; }

        /// <summary>
        /// Gets or sets the location to get a service endpoint to establish a connection with service provider.
        /// Defaults to "us-east-1" if not specified. 
        /// </summary>
        public string Location {get; set; }

        /// <summary>
        /// Clears this object for resuse
        /// </summary>
        public void Clear()
        {
            ConnectOptions.Clear();
            m_InitTimeout = DEFAULT_TIMEOUT;
            EnableSessionManagement = false;
            Location = "us-east-1";
            ReactorAuthTokenEventCallback = null;
        }

        /// <summary>
        /// This method will perform a deep copy into the passed in parameter's members from the ReactorConnectInfo
        /// calling this method.
        /// </summary>
        /// <param name="destInfo">the value getting populated with the values of the calling ReactorConnectInfo</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success, <see cref="ReactorReturnCode.FAILURE"/> if the
        /// destInfo is null.</returns>
        public ReactorReturnCode Copy(ReactorConnectInfo destInfo)
        {
            if (destInfo is null)
                return ReactorReturnCode.FAILURE;

            ConnectOptions.Copy(destInfo.ConnectOptions);
            destInfo.m_InitTimeout = m_InitTimeout;
            destInfo.EnableSessionManagement = EnableSessionManagement;
            destInfo.Location = Location;
            destInfo.ReactorAuthTokenEventCallback = ReactorAuthTokenEventCallback;
            return ReactorReturnCode.SUCCESS;
        }
    }
}
