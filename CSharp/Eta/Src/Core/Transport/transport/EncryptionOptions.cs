/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Net.Security;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Options used for configuring an encrypted tunneled connection with the <see cref="ConnectionType.ENCRYPTED"/> connection type.
    /// </summary>
    /// <seealso cref="ConnectOptions"/>
    public class EncryptionOptions
    {
        /// <summary>
        /// Default constructor
        /// </summary>
        public EncryptionOptions()
        {
            Clear();
        }

        /// <summary>
        /// Gets or sets TLS protocol(s) used by this connection.
        /// </summary>
        /// <value>The encryption protocol flags.</value>
        public EncryptionProtocolFlags EncryptionProtocolFlags { get; set; }

        /// <summary>
        /// Gets or sets the protocol used for this connection.
        /// </summary>
        /// <value>The encrypted protocol</value>
        public ConnectionType EncryptedProtocol { get; set; }

        /// <summary>
        /// Gets or sets the collection of cipher suites allowed for TLS negotiation.
        /// The operating system default is used if not specified.
        /// </summary>
        /// <remarks>
        /// This option supports only on a Linux system with OpenSSL 1.1.1 or higher or a macOS.
        /// </remarks>
        /// <value>The collection of <c>TlsCipherSuite</c></value>
        public IEnumerable<TlsCipherSuite> TlsCipherSuites { get; set; }

        /// <summary>
        /// Gets or sets the timeout in millisecond for client to complete the authetication with the server.
        /// Defaults to 10000 milliseconds.
        /// </summary>
        public int AuthenticationTimeout { get; set; }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_NONE;
            EncryptedProtocol = ConnectionType.SOCKET;
            TlsCipherSuites = null;
            AuthenticationTimeout = 10000;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"EncryptionProtocolFlags: {EncryptionProtocolFlags}, EncryptedProtocol: {EncryptedProtocol}," +
                $" TlsCipherSuites: {TlsCipherSuites}, AuthenticationTimeout: {AuthenticationTimeout}";
        }
    }
}
