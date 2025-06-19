/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
