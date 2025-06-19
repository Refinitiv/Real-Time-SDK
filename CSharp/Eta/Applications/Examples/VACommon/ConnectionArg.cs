/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.VACommon
{

    /// <summary>
    /// Connection argument class for the Value Add consumer and non-interactive provider applications.
    /// </summary>
    public class ConnectionArg
    {
        /// <summary>
        /// Type of the connection.
        /// </summary>
        public ConnectionType ConnectionType { get; set; }

        /// <summary>
        /// Tls protocol.
        /// </summary>
        public EncryptionProtocolFlags EncryptionProtocolFlags { get; set; }

        /// <summary>
        /// Name of service to request items from on this connection.
        /// </summary>
        public string Service { get; set; }

        /* non-segmented connection */

        /// <summary>
        /// Hostname of provider to connect to.
        /// </summary>
        public string Hostname { get; set; }

        /// <summary>
        /// Port of provider to connect to.
        /// </summary>
        public string Port { get; set; }

        /// <summary>
        /// Interface that the provider will be using.  This is optional
        /// </summary>
        public string? InterfaceName { get; set; }

        /// <summary>
        /// Item list for this connection.
        /// </summary>
        public List<ItemArg> ItemList { get; set; }

        /// <summary>
        /// Instantiates a new connection arg.
        /// </summary>
        ///
        /// <param name="connectionType">the connection type</param>
        /// <param name="service">the service</param>
        /// <param name="hostname">the hostname</param>
        /// <param name="port">the port</param>
        /// <param name="itemList">the item list</param>
        public ConnectionArg(ConnectionType connectionType, string service, string hostname, string port, List<ItemArg> itemList, EncryptionProtocolFlags encryptionProtocolFlags)
        {
            this.ConnectionType = connectionType;
            this.Service = service;
            this.Hostname = hostname;
            this.Port = port;
            this.ItemList = itemList;
            EncryptionProtocolFlags = encryptionProtocolFlags;
        }


        /// <summary>
        /// Instantiates a new connection arg.
        /// </summary>
        public ConnectionArg()
        {
            ItemList = new();
            Service = string.Empty;
            Hostname = string.Empty;
            Port = string.Empty;
            EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_NONE;
        }
    }
}
