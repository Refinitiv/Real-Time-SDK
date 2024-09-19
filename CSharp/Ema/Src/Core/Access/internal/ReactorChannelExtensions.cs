/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access
{
    internal static class ReactorChannelExtensions
    {
        public static ClientChannelConfig? GetClientChannelConfig(this ReactorChannel? reactorChannel) =>
            ((ChannelInfo?)reactorChannel?.UserSpecObj)?.ChannelConfig;
    }
}
