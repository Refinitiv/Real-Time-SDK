/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

namespace LSEG.Ema.Access
{
    internal interface IOmmProviderImpl
    {
        string ProviderName { get; }
        OmmProviderConfig.ProviderRoleEnum ProviderRole { get; }
        void Uninitialize();
        long RegisterClient(RequestMsg reqMsg, IOmmProviderClient client, object? closure);
        void Reissue(RequestMsg reqMsg, long handle);
        void Submit(GenericMsg genericMsg, long handle);
        void Submit(RefreshMsg refreshMsg, long handle);
        void Submit(UpdateMsg updateMsg, long handle);
        void Submit(StatusMsg statusMsg, long handle);
        void Submit(AckMsg ackMsg, long handle);
        void Submit(PackedMsg packedMsg);
        int Dispatch(int dispatchTimeout);
        void Unregister(long handle);
        void ChannelInformation(ChannelInformation channelInfo);
        void ConnectedClientChannelInfo(List<ChannelInformation> clientInfoList);
        void ModifyIOCtl(IOCtlCode code, int val);
        void ModifyIOCtl(IOCtlCode code, int val, long handle);
        void CloseChannel(long clientHnadle);
    }
}
