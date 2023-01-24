/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Transports
{
    /// <summary>
    ///  Options used for configuring a unified/fully connected mesh network.
    /// <seealso cref="ConnectOptions"/>
    /// </summary>
    sealed public class UnifiedNetworkInfo
    {
        private string _serviceName;

        /// <summary>
        /// Address or hostname to connect to/join for all inbound and outbound data.
        /// All data is exchanged on this hostName:serviceName combination.
        /// </summary>
        /// <value>The address</value>
        public string Address { get; set; }

        /// <summary>
        /// Port number or service name to connect to/join for all inbound and outbound data.
        /// All data is exchanged on this hostName:serviceName combination.
        /// </summary>
        /// <value>The service name</value>
        public string ServiceName { get => _serviceName;
            set
            {
                if (value != null &&_serviceName != value)
                {
                    _serviceName = value;
                }
            }
        }

        /// <summary>
        /// A character representation of an IP address or hostname associated with
        /// the local network interface to use for sending and receiving content.
        /// This value is intended for use in systems which have multiple network
        /// interface cards, and if not specified the default network interface will be used.
        /// </summary>
        /// <value>The interface name</value>
        public string InterfaceName { get; set; }

        /// <summary>
        /// Gets the port
        /// </summary>
        /// <value>The port</value>
        internal int Port { get; set; }

        /// <summary>
        /// Clears the values
        /// </summary>
        public void Clear()
        {
            Address = null;
            ServiceName = null;
            InterfaceName = null;
        }

        /// <summary>
        /// Gets the string representation of this object. 
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"Address: {Address}, ServiceName: {ServiceName}";
        }

        /// <summary>
        /// Determines whether the specified object is equal to the current object.
        /// </summary>
        /// <param name="obj">The object to compare</param>
        /// <returns><c>true</c> if the specified object is equal to the current object; otherwise, <c>false</c></returns>
        public override bool Equals(object obj)
        {
            var networkInfo = obj as UnifiedNetworkInfo;
            if (networkInfo is null)
                return false;

            return Address.Equals(networkInfo.Address, StringComparison.OrdinalIgnoreCase)
                && ServiceName.Equals(networkInfo.ServiceName, StringComparison.OrdinalIgnoreCase);
        }

        /// <summary>
        /// Gets the hash code
        /// </summary>
        /// <returns>The hash code</returns>
        public override int GetHashCode()
        {
            return Address.GetHashCode() ^ ServiceName.GetHashCode();
        }
    }
}

