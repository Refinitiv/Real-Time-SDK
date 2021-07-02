package com.refinitiv.ema.perftools.common;

public class ItemData {
    private int nextIndex;

    public int getNextIndex() {
        return nextIndex;
    }

    public void setNextIndex(int nextIndex) {
        this.nextIndex = nextIndex;
    }

    public void clear() {
        this.nextIndex = 0;
    }
}
