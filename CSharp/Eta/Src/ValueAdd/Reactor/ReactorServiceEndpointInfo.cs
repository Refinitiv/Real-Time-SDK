/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This class represents endpoint information from the service discovery.
    /// </summary>
    sealed public class ReactorServiceEndpointInfo
    {
        internal ReactorServiceEndpointInfo()
        {
            DataFormatList = new List<string>();
            LocationList = new List<string>();
            Provider = string.Empty;
            Transport = string.Empty;
            Port = string.Empty;
        }

        /// <summary>
        /// Gets a list of data format of an endpoint
        /// </summary>
        public List<string> DataFormatList { get; internal set; }

        /// <summary>
        /// Gets a list of location of an endpoint
        /// </summary>
        public List<string> LocationList { get; internal set; }

        /// <summary>
        /// Gets a provider name of an endpoint
        /// </summary>
        public string Provider { get; internal set; }

        /// <summary>
        /// Gets a transport type of an endpoint
        /// </summary>
        public string Transport { get; internal set; }

        /// <summary>
        /// Gets an endpoint to establish a connection.
        /// </summary>
        public string? EndPoint { get; internal set; }

        /// <summary>
        /// Gets a port of an endpoint
        /// </summary>
        public string? Port { get; internal set; }
    }
}
