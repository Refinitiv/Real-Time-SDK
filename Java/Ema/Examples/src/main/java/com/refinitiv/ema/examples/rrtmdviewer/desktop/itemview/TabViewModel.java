/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

import javafx.beans.property.BooleanProperty;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleIntegerProperty;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Used for storing current state of the tab view.
 */
public class TabViewModel {

    private static final int QUEUE_CAPACITY = 1024;

    private final BlockingQueue<ItemNotificationModel> notificationQueue = new LinkedBlockingQueue<>(QUEUE_CAPACITY);

    private final IntegerProperty messageCounter = new SimpleIntegerProperty(this, this.getClass().getName());

    private final BooleanProperty connectionProperty = new SimpleBooleanProperty(this, this.getClass().getName());

    public BlockingQueue<ItemNotificationModel> getNotificationQueue() {
        return notificationQueue;
    }

    public int getMessageCounter() {
        return messageCounter.get();
    }

    public IntegerProperty messageCounterProperty() {
        return messageCounter;
    }

    public BooleanProperty getConnectionProperty() { return connectionProperty;}

    public void incrementCounter() {
        this.messageCounter.set(this.messageCounter.get() + 1);
    }
}
