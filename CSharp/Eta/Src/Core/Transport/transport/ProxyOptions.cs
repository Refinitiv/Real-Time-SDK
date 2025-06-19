/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Proxy Options used in <see cref="ConnectOptions"/>
    /// </summary>
    sealed public class ProxyOptions
    {
        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            ProxyHostName = null;
            ProxyPort = null;
            ProxyUserName = null;
            ProxyPassword = null;
        }

        /// <summary>
        /// Gets or set a proxy server host name.
        /// </summary>
        public string ProxyHostName { get; set; }

        /// <summary>
        /// Gets or sets a proxy server port.
        /// </summary>
        public string ProxyPort { get; set; }

        /// <summary>
        /// Gets or sets a proxy user name
        /// </summary>
        public string ProxyUserName { get; set; }

        /// <summary>
        /// Gets or sets a proxy password.
        /// </summary>
        public string ProxyPassword { get; set; }

        /// <summary>
        /// Indicates that current ProxyOptions is set.
        /// </summary>
        public bool IsHostAndPortSet => !(string.IsNullOrEmpty(ProxyHostName) && string.IsNullOrEmpty(ProxyPort));

        /// <summary>
        /// Gets the port
        /// </summary>
        /// <value>The port</value>
        internal int Port { get; set; }

        /// <summary>
        /// Gets the string representation of this object. 
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"ProxyHostName: {ProxyHostName}, ProxyPort: {ProxyPort}, " +
                $"ProxyUserName: {ProxyUserName}, ProxyPassword: {ProxyPassword}";
        }

        /// <summary>
        /// Determines whether the specified object is equal to the current object.
        /// </summary>
        /// <param name="obj">The object to compare</param>
        /// <returns><c>true</c> if the specified object is equal to the current object; otherwise, <c>false</c></returns>
        public override bool Equals(object obj)
        {
            var proxyOptions = obj as ProxyOptions;
            if (proxyOptions is null)
                return false;

            if (ProxyHostName is null)
            {
                if (proxyOptions.ProxyHostName is not null)
                {
                    return false;
                }
            }
            else
            {
                if (!ProxyHostName.Equals(proxyOptions.ProxyHostName))
                {
                    return false;
                }
            }

            if (ProxyPort is null)
            {
                if (proxyOptions.ProxyPort is not null)
                {
                    return false;
                }
            }
            else
            {
                if (!ProxyPort.Equals(proxyOptions.ProxyPort))
                {
                    return false;
                }
            }

            if (ProxyUserName is null)
            {
                if (proxyOptions.ProxyUserName is not null)
                {
                    return false;
                }
            }
            else
            {
                if (!ProxyUserName.Equals(proxyOptions.ProxyUserName))
                {
                    return false;
                }
            }

            if (ProxyPassword is null)
            {
                if (proxyOptions.ProxyPassword is not null)
                {
                    return false;
                }
            }
            else
            {
                if (!ProxyPassword.Equals(proxyOptions.ProxyPassword))
                {
                    return false;
                }
            }

            return true;
        }

        /// <summary>
        /// Gets a hash code for this object
        /// </summary>
        /// <returns>The hash code</returns>
        public override int GetHashCode()
        {
            return (ProxyHostName ?? string.Empty).GetHashCode() ^ (ProxyPort ?? string.Empty).GetHashCode()
                ^ (ProxyUserName ?? string.Empty).GetHashCode() ^ (ProxyPassword ?? string.Empty).GetHashCode();
        }

        /// <summary>
        /// Performs a deep copy to a specified ProxyOptions from this ProxyOptions.
        /// </summary>
        /// <param name="dest">The proxy options to copy to.</param>
        public void CopyTo(ProxyOptions dest)
        {
            dest.ProxyHostName = ProxyHostName;
            dest.ProxyPort = ProxyPort;
            dest.ProxyUserName = ProxyUserName;
            dest.ProxyPassword = ProxyPassword;
        }
    }
}

