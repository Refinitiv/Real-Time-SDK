package com.refinitiv.ema.perftools.common;

public class MarketPriceMsg {
    private MarketField[] fieldEntries;   // List of fields.
    private int fieldEntryCount;          // Number of fields in list.
    private int arrayCount;               // count for field entry array

    /**
     * Instantiates a new market price msg.
     *
     * @param count the count
     */
    public MarketPriceMsg(int count) {
        arrayCount = count;
        fieldEntries = new MarketField[arrayCount];
    }

    /**
     * List of fields.
     *
     * @return the market field[]
     */
    public MarketField[] fieldEntries() {
        return fieldEntries;
    }

    /**
     * Number of fields in list.
     *
     * @return the int
     */
    public int fieldEntryCount() {
        return fieldEntryCount;
    }

    /**
     * Number of fields in list.
     *
     * @param count the count
     */
    public void fieldEntryCount(int count) {
        fieldEntryCount = count;
    }
}
