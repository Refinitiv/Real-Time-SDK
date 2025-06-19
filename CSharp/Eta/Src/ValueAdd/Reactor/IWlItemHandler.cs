/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal interface IWlItemHandler : IWlHandler
    {
        void Init(ConsumerRole consumerRole);

        bool IsRequestRecoverable(WlRequest wlRequest, int streamState);
        ReactorReturnCode PauseAll();
        ReactorReturnCode ResumeAll();

        ReactorReturnCode LoginStreamClosed(State? state);

        ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo? errorInfo);
    }
}