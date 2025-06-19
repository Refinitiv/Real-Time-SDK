/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

using LSEG.Eta.Transports;

namespace LSEG.Eta.Internal
{
    /// <summary>
    /// Singleton Registry of all registered protocols.
    /// </summary>
    internal sealed class ProtocolRegistry : IEnumerable<(ConnectionType connectionType, IProtocol protocol)>
    {
        // Ignore Resharper...this pattern designed to create singleton correctly.
        private static readonly Lazy<ProtocolRegistry> _instance = new Lazy<ProtocolRegistry>(CreateRegistry);

        /// <summary>
        /// Create Registry; register all knon Protocols
        /// </summary>
        /// <returns></returns>
        private static ProtocolRegistry CreateRegistry()
        {
            var registry = new ProtocolRegistry()
                   .Register(ConnectionType.SOCKET, new SocketProtocol())
                   .Register(ConnectionType.ENCRYPTED, new SocketProtocol())
                   ;

            return registry;
        }

        public static ProtocolRegistry Instance => _instance.Value;

        private ProtocolRegistry()
        {

        }

        /// <summary>
        /// Maps ProtocolFlag => Protocol
        /// </summary>
        List<(ConnectionType connectionType, IProtocol protocol)> _registry 
            = new List<(ConnectionType connectionType, IProtocol protocol)>();

        /// <summary>
        /// throws InvalidOperation fault if more thn one registry entry found with same ProtoclFlag.
        /// </summary>
        /// <param name="connectionType"></param>
        /// <param name="protocol"></param>
        /// <returns></returns>
        public ProtocolRegistry Register(ConnectionType connectionType, IProtocol protocol)
        {
            lock (_registry)
            {
                var entry = _registry.SingleOrDefault((item) => item.connectionType == connectionType);
                if (entry.Equals(default((ConnectionType connectionType, IProtocol protocol))))
                {
                    _registry.Add(new ValueTuple<ConnectionType, IProtocol>(connectionType, protocol));
                }
                else
                {
                    entry = new ValueTuple<ConnectionType, IProtocol>(connectionType, protocol);
                }
            }
            return this;
        }

        public IEnumerator<(ConnectionType connectionType, IProtocol protocol)> GetEnumerator()
        {
            return _registry.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return _registry.GetEnumerator();
        }

        /// <summary>
        /// Get protocol for a specified connection type
        /// </summary>
        /// <param name="connectionType"></param>
        /// <returns>Protocol registerd to the ConnectionType, or null</returns>
        public IProtocol this[ConnectionType connectionType] 
                => _registry.SingleOrDefault((item) => item.connectionType == connectionType)
                            .protocol;
    }
}
