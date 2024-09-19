/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;

namespace LSEG.Ema.PerfTools.EMA_NiProvPerf
{
    public class NiProviderPerfClient : IOmmProviderClient
    {
        private NIProviderThread niProviderThread;
        private bool connectionUp;

        public NiProviderPerfClient(NIProviderThread niProviderThread)
        {
            this.niProviderThread = niProviderThread;
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
        {
            SetConnectionStatus(refreshMsg.State());
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
        {
            niProviderThread.ProviderThreadStats.StatusCount.Increment();
            SetConnectionStatus(statusMsg.State());
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
        {
            niProviderThread.ProviderThreadStats.GenMsgRecvCount.Increment();
        }

        public void OnPostMsg(PostMsg postMsg, IOmmProviderEvent providerEvent)
        {
            niProviderThread.ProviderThreadStats.PostCount.Increment();
        }

        public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            niProviderThread.ProviderThreadStats.RequestCount.Increment();
        }

        public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            niProviderThread.ProviderThreadStats.CloseCount.Increment();
            connectionUp = false;
        }

        public bool IsConnectionUp()
        {
            return connectionUp;
        }

        private void SetConnectionStatus(OmmState state)
        {
            connectionUp = state.StreamState == OmmState.StreamStates.OPEN && state.DataState == OmmState.DataStates.OK;
        }
    }
}
