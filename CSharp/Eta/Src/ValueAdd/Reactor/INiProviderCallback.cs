/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Callback used for processing all non-interactive provider events and messages
    /// </summary>
    /// <see cref="IReactorChannelEventCallback"/>
    /// <see cref="IDefaultMsgCallback"/>
    public interface INiProviderCallback : IReactorChannelEventCallback, IDefaultMsgCallback, IRDMLoginMsgCallback
    {
    }
}
