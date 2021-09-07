package com.refinitiv.ema.perftools.common;

public class MarketPriceItem {

    private int _iMsg; // item data for message

    /**
     *  Item data for message.
     *
     * @return the int
     */
    public int iMsg()
    {
        return _iMsg;
    }

    /**
     *  Item data for message.
     *
     * @param iMsg the i msg
     */
    public void iMsg(int iMsg)
    {
        _iMsg = iMsg;
    }
}
