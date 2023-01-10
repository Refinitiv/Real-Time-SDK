/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /**
 * Callback used for processing all interactive provider events and messages.
 * 
 * @see ReactorChannelEventCallback
 * @see DefaultMsgCallback
 * @see RDMLoginMsgCallback
 * @see RDMDirectoryMsgCallback
 * @see RDMDictionaryMsgCallback
 */
    public interface IProviderCallback : IReactorChannelEventCallback, IDefaultMsgCallback,
        IRDMLoginMsgCallback, IDirectoryMsgCallback, IDictionaryMsgCallback
    {
    }
}
