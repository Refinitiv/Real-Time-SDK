/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access.Tests.ReactorSubmitOptionsExtensionsTests
{
    public class ApplyClientChannelConfigTests
    {
        [Theory]
        [InlineData(WriteFlags.NO_FLAGS, true, WriteFlags.DIRECT_SOCKET_WRITE)]
        [InlineData(WriteFlags.DO_NOT_COMPRESS, true, WriteFlags.DIRECT_SOCKET_WRITE | WriteFlags.DO_NOT_COMPRESS)]
        [InlineData(WriteFlags.DIRECT_SOCKET_WRITE, true, WriteFlags.DIRECT_SOCKET_WRITE)]
        [InlineData(WriteFlags.DIRECT_SOCKET_WRITE | WriteFlags.DO_NOT_COMPRESS, true, WriteFlags.DIRECT_SOCKET_WRITE | WriteFlags.DO_NOT_COMPRESS)]
        [InlineData(WriteFlags.NO_FLAGS, false, WriteFlags.NO_FLAGS)]
        [InlineData(WriteFlags.DO_NOT_COMPRESS, false, WriteFlags.DO_NOT_COMPRESS)]
        [InlineData(WriteFlags.DIRECT_SOCKET_WRITE, false, WriteFlags.DIRECT_SOCKET_WRITE)]
        [InlineData(WriteFlags.DIRECT_SOCKET_WRITE | WriteFlags.DO_NOT_COMPRESS, false, WriteFlags.DIRECT_SOCKET_WRITE | WriteFlags.DO_NOT_COMPRESS)]
        public void ShouldCorrectlyApplyDirectWriteFlag(WriteFlags initialWriteFlags, bool directWrite, WriteFlags expectedWriteFlags)
        {
            var opts = new ReactorSubmitOptions() { WriteArgs = { Flags = initialWriteFlags } };
            var channelConfig = new ClientChannelConfig { DirectWrite = directWrite };

            Assert.Equal(expectedWriteFlags, opts.ApplyClientChannelConfig(channelConfig).WriteArgs.Flags);
        }
    }
}
