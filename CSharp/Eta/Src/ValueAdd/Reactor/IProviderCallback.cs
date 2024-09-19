/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Callback used for processing all interactive provider events and messages.
    /// </summary>
    /// <seealso cref="IReactorChannelEventCallback"/>
    /// <seealso cref="IDefaultMsgCallback"/>
    /// <seealso cref="IRDMLoginMsgCallback"/>
    /// <seealso cref="IDirectoryMsgCallback"/>
    /// <seealso cref="IDictionaryMsgCallback"/>
    public interface IProviderCallback : IReactorChannelEventCallback, IDefaultMsgCallback,
        IRDMLoginMsgCallback, IDirectoryMsgCallback, IDictionaryMsgCallback
    {
    }
}
