/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

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
