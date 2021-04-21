package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

import com.refinitiv.ema.access.Msg;

public class ItemNotificationModel {

    private ItemRequestModel request;

    private long parentHandle;

    private long handle;

    private Msg msg;

    public long getParentHandle() {
        return parentHandle;
    }

    public void setParentHandle(long parentHandle) {
        this.parentHandle = parentHandle;
    }

    public long getHandle() {
        return handle;
    }

    public void setHandle(long handle) {
        this.handle = handle;
    }

    public Msg getMsg() {
        return msg;
    }

    public void setMsg(Msg msg) {
        this.msg = msg;
    }

    public boolean isBatchPart() {
        return ( parentHandle != 0 );
    }

    public long getRequestHandle() {
        return isBatchPart() ? parentHandle : handle;
    }

    public ItemRequestModel getRequest() {
        return request;
    }

    public void setRequest(ItemRequestModel request) {
        this.request = request;
    }
}
