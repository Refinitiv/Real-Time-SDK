package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import java.util.HashMap;
import java.util.Map;

public class MarketPriceTableItem {
    private String id;

    private Long handle;

    private final Map<String, String> columns = new HashMap<>();

    private boolean isStreaming;

    private int itemIndex;

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public Long getHandle() {
        return handle;
    }

    public void setHandle(Long handle) {
        this.handle = handle;
    }

    public Map<String, String> getColumns() {
        return columns;
    }

    public boolean isStreaming() {
        return isStreaming;
    }

    public void setStreaming(boolean streaming) {
        isStreaming = streaming;
    }

    public int getItemIndex() {
        return itemIndex;
    }

    public void setItemIndex(int itemIndex) {
        this.itemIndex = itemIndex;
    }
}
