/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access.Tests.ReactorChannelExtensions
{
    public class GetClientChannelConfigTests
    {
        [Fact]
        public void ShouldGetAttachedChannelConfig()
        {
            var channelConfig = new ClientChannelConfig();
            var reactorChannel = CreateReactorChannelWithConfig(channelConfig);

            Assert.Same(channelConfig, reactorChannel.GetClientChannelConfig());
        }

        [Fact]
        public void ShouldReturnNullWhenNoChannelInfoAttached()
        {
            var reactorChannel = new ReactorChannel();

            Assert.Null(reactorChannel.GetClientChannelConfig());
        }

        [Fact]
        public void ShouldReturnNullWhenNoChannelConfigAttached()
        {
            var reactorChannel = CreateReactorChannelWithConfig(null);

            Assert.Null(reactorChannel.GetClientChannelConfig());
        }

        private ReactorChannel CreateReactorChannelWithConfig(ClientChannelConfig? clientChannelConfig) =>
            new ReactorChannel() { UserSpecObj = CreateChannelInfo(clientChannelConfig) };

        private ChannelInfo CreateChannelInfo(ClientChannelConfig? clientChannelConfig) =>
            new ChannelInfo(clientChannelConfig, Reactor.CreateReactor(new ReactorOptions(), out _));
    }
}
