package com.refinitiv.ema.perftools.emajniprovperf;

import com.refinitiv.ema.access.*;

public class NIProviderPerfClient implements OmmProviderClient {
    private final NIProviderThread niProviderThread;
    private boolean connectionUp;

    public NIProviderPerfClient() {
        this.niProviderThread = (NIProviderThread) Thread.currentThread();
    }

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {
        niProviderThread.providerThreadStats().refreshCount().increment();
        setConnectionStatus(refreshMsg.state());
    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {
        niProviderThread.providerThreadStats().statusCount().increment();
        setConnectionStatus(statusMsg.state());
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {
        niProviderThread.providerThreadStats().genMsgRecvCount().increment();
    }

    @Override
    public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {
        niProviderThread.providerThreadStats().postCount().increment();
    }

    @Override
    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
        niProviderThread.providerThreadStats().requestCount().increment();
    }

    @Override
    public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {

    }

    @Override
    public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
        niProviderThread.providerThreadStats().closeCount().increment();
        connectionUp = false;
    }

    @Override
    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {
    }

    public boolean isConnectionUp() {
        return connectionUp;
    }

    private void setConnectionStatus(OmmState state) {
        connectionUp = state.streamState() == OmmState.StreamState.OPEN && state.dataState() == OmmState.DataState.OK;
    }
}
