package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

import javafx.beans.property.IntegerProperty;
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

    public BlockingQueue<ItemNotificationModel> getNotificationQueue() {
        return notificationQueue;
    }

    public int getMessageCounter() {
        return messageCounter.get();
    }

    public IntegerProperty messageCounterProperty() {
        return messageCounter;
    }

    public void incrementCounter() {
        this.messageCounter.set(this.messageCounter.get() + 1);
    }
}
