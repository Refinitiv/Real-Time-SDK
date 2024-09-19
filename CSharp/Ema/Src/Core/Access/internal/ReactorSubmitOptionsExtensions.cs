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
    internal static class ReactorSubmitOptionsExtensions
    {
        public static ReactorSubmitOptions ApplyClientChannelConfig(this ReactorSubmitOptions options, ClientChannelConfig channelConfig)
        {
            // There is no need to unset the flag as the ReactorSubmitOptions must be cleared before calling this method.
            if (channelConfig.DirectWrite)
                options.WriteArgs.Flags |= Eta.Transports.WriteFlags.DIRECT_SOCKET_WRITE;

            return options;
        }
    }
}
