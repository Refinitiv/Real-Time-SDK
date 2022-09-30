/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Callback used for processing all consumer events and messages
    /// </summary>
    /// <see cref="IReactorChannelEventCallback"/>
    /// <see cref="IDefaultMsgCallback"/>
    /// <see cref="IDirectoryMsgCallback"/>
    /// <see cref="IDictionaryMsgCallback"/>
    public interface IConsumerCallback : IReactorChannelEventCallback, IDefaultMsgCallback,
        IDirectoryMsgCallback, IDictionaryMsgCallback, IRDMLoginMsgCallback
    {
    }
}
