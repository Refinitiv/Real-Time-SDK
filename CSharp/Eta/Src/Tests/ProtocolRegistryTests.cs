/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using Xunit;
using Xunit.Categories;

using LSEG.Eta.Internal;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports.Tests
{
    public class ProtocolRegistryTests
    {
        [Fact, Category("Unit")]
        public void ProtocolRegistryExists()
        {
            var registry = ProtocolRegistry.Instance;

            Assert.NotNull(registry);
        }

        [Fact, Category("Unit")]
        public void ProtocolRegistryRegisterSocketProtocol()
        {
            var registry = ProtocolRegistry.Instance;

            Assert.IsType<ProtocolRegistry>(registry.Register(ConnectionType.SOCKET, new SocketProtocol()));
        }


        [Fact, Category("Unit")]
        public void ProtocolRegistryReturnsSocketProtocolFactory()
        {
            var registry = ProtocolRegistry.Instance;

            Assert.IsType<SocketProtocol>(registry[ConnectionType.SOCKET]);
        }

        [Fact, Category("Unit")]
        public void ProtocolRegistryFailsToReturnUnregisteredFactory()
        {
            var registry = ProtocolRegistry.Instance;

            Assert.Null(registry[(ConnectionType)2]); /* Specify unregistered connection type */
        }

        [Fact, Category("Unit")]
        public void ProtocolRegistryExposesListAsEnumerable()
        {
            var registryEnumerable = ProtocolRegistry.Instance as IEnumerable<(ConnectionType connectionType, IProtocol protocol)>;
            Assert.NotNull(registryEnumerable);
        }

        [Fact, Category("Unit")]
        public void ProtocolRegistryContainsRegisteredProtocols()
        {
            var registryEnumerable = ProtocolRegistry.Instance;
            foreach (var item in registryEnumerable)
            {
                // There is at least one registration.
                if (item.connectionType == ConnectionType.SOCKET)
                {
                    Assert.IsAssignableFrom<SocketProtocol>(item.protocol);
                    return;
                }
            }
            Assert.True(false);
        }
    }
}
