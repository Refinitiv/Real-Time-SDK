/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;

import java.util.*;
import java.util.concurrent.locks.ReentrantLock;

public class ServiceInfoResponseModel {

    private ReentrantLock serviceInfoLock = new ReentrantLock();

    private final BooleanProperty dictionaryLoaded = new SimpleBooleanProperty(this, this.getClass().getName());
    private final BooleanProperty sourceRefreshCompleted = new SimpleBooleanProperty(this, this.getClass().getName(), true);

    private java.util.Map<Long, ServiceInfoModel> serviceInfoMap = new HashMap<>();
    private Map<String, Integer> fids = new HashMap<>();

    public Map<Long, ServiceInfoModel> getServiceInfoMap() {
        return serviceInfoMap;
    }

    public boolean getDictionaryLoaded() {
        return dictionaryLoaded.get();
    }

    public boolean sourceRefreshCompletedPropertyValue() {
        return sourceRefreshCompleted.get();
    }

    public BooleanProperty dictionaryLoadedProperty() {
        return dictionaryLoaded;
    }

    public BooleanProperty sourceRefreshCompletedProperty() {
        return sourceRefreshCompleted;
    }

    public void setDictionaryLoaded(boolean dictionaryLoaded) {
        this.dictionaryLoaded.set(dictionaryLoaded);
    }

    public Map<String, Integer> getFids() {
        return fids;
    }

    public void setFids(Map<String, Integer> fids) {
        this.fids = fids;
    }

    public void acquireServiceInfoLock() {
        serviceInfoLock.lock();
    }

    public void releaseServiceInfoLock() {
        serviceInfoLock.unlock();
    }

    @Override
    public String toString() {
        return "ServiceInfoResponseModel{" +
                "serviceInfoLoaded=" + dictionaryLoaded +
                ", fids=" + fids +
                '}';
    }
}
