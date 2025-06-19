/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.marketbyorder;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RealHints;

/**
 * Market by order item data.
 */
public class MarketByOrderItem
{
    public static final int ORDER_ID_FID = 3426;
    public static final int ORDER_PRC_FID = 3427;
    public static final int ORDER_SIDE_FID = 3428;
    public static final int ORDER_SIZE_FID = 3429;
    public static final int MKT_MKR_ID_FID = 212;
    public static final int QUOTIM_MS_FID = 3855;
    public static final int CURRENCY_FID = 15;
    public static final int MKT_STATUS_IND_FID = 133;
    public static final int ACTIVE_DATE_FID = 17;
    public static final int MKOASK_VOL_FID = 3839;
    public static final int MKOBID_VOL_FID = 3840;

    public static final int ORDER_BID = 1;
    public static final int ORDER_ASK = 2;
    public static final int MAX_ORDERS = 3;

    public static Enum USD_ENUM;
    public static Enum BBO_ENUM;

    public String itemName;
    public boolean isInUse;

    /* market by order item data */
    public List<OrderInfo> orderInfoList;

    public boolean includePrivateStream = false;

    private StringBuilder stringBuf = new StringBuilder();
    private final static String eolChar = "\n";
    private final static String tabChar = "\t";
    
    /* market by order fields for each order */
    public class OrderInfo
    {
        public Real ORDER_PRC;
        public String MKT_MKR_ID;
        public Real ORDER_SIZE;
        public String ORDER_ID;
        public Enum ORDER_SIDE;
        public long QUOTIM_MS;
        public Real MKOASK_VOL;
        public Real MKOBID_VOL;

        /*
         * To demonstrate the container actions "Add," "Update," and "Delete,"
         * Each order follows a cycle where it is first added, then updated some
         * number of times, then deleted, then added again. The 'lifetime'
         * member is used to control this cycle.
         */
        public int lifetime;

        {
            ORDER_PRC = CodecFactory.createReal();
            ORDER_SIZE = CodecFactory.createReal();
            ORDER_SIDE = CodecFactory.createEnum();
            MKOASK_VOL = CodecFactory.createReal();
            MKOBID_VOL = CodecFactory.createReal();
        }
    }

    static
    {
        USD_ENUM = CodecFactory.createEnum();
        USD_ENUM.value(840);
        BBO_ENUM = CodecFactory.createEnum();
        BBO_ENUM.value(20);
    }

    {
        orderInfoList = new ArrayList<OrderInfo>(MAX_ORDERS);
        for (int i = 0; i < MAX_ORDERS; i++)
        {
            orderInfoList.add(new OrderInfo());
        }
    }

    /**
     * Clears the contents of this object.
     */
    public void clear()
    {
        isInUse = false;
        for (OrderInfo orderInfo : orderInfoList)
        {
            orderInfo.ORDER_ID = null;
            orderInfo.MKT_MKR_ID = null;
            orderInfo.ORDER_PRC.clear();
            orderInfo.ORDER_SIZE.clear();
            orderInfo.ORDER_SIDE.clear();
            orderInfo.QUOTIM_MS = 0;
            orderInfo.MKOASK_VOL.clear();
            orderInfo.MKOBID_VOL.clear();
            orderInfo.lifetime = 0;
        }
    }
    
    /**
     * Initializes market by order item fields.
     */
    public void initFields()
    {
        isInUse = true;
        int idx = -1;
        for (OrderInfo orderInfo : orderInfoList)
        {
            ++idx;
            orderInfo.ORDER_ID = String.valueOf(idx + 100);
            orderInfo.MKT_MKR_ID = "MarketMaker" + String.valueOf(idx + 1);
            orderInfo.ORDER_PRC.value(3459 + 100 * idx, RealHints.EXPONENT_2);
            orderInfo.ORDER_SIZE.value(5 + idx, RealHints.EXPONENT0);
            orderInfo.ORDER_SIDE.value((idx >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID);
            orderInfo.QUOTIM_MS = 500 * idx;
            orderInfo.MKOASK_VOL.value(2 + idx, RealHints.EXPONENT0);
            orderInfo.MKOBID_VOL.value(3 + idx, RealHints.EXPONENT0);
            orderInfo.lifetime = 5 + idx;
        }
    }

    /**
     * Updates any item that's currently in use.
     */
    public void updateFields()
    {
        int idx = -1;

        for (OrderInfo orderInfo : orderInfoList)
        {
            ++idx;
            orderInfo.ORDER_PRC.value(orderInfo.ORDER_PRC.toLong() + 1, orderInfo.ORDER_PRC.hint());
            orderInfo.QUOTIM_MS += 1000;

            orderInfo.MKOASK_VOL.value(orderInfo.MKOASK_VOL.toLong() + 1,
                                       orderInfo.ORDER_PRC.hint());
            orderInfo.MKOBID_VOL.value(orderInfo.MKOASK_VOL.toLong() + 1,
                                       orderInfo.ORDER_PRC.hint());

            if (orderInfo.lifetime == 0)
                orderInfo.lifetime = 5 + idx;
            else
                --orderInfo.lifetime;
        }
    }

    public String toString()
    {
        stringBuf.setLength(0);
        for (OrderInfo orderInfo : orderInfoList)
        {
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER ID: ");
            stringBuf.append(orderInfo.ORDER_ID);
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("MRKT MAKER ID: ");
            stringBuf.append(orderInfo.MKT_MKR_ID);
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER PRICE: ");
            stringBuf.append(orderInfo.ORDER_PRC.toDouble());
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER SIZE: ");
            stringBuf.append(orderInfo.ORDER_SIZE.toDouble());
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER SIDE: ");
            stringBuf.append((orderInfo.ORDER_SIDE.toInt() == ORDER_BID) ? "BID" : "ASK");
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("QUOTIM MS: ");
            stringBuf.append(orderInfo.QUOTIM_MS);
            stringBuf.append(eolChar);
            if (includePrivateStream)
            {
                stringBuf.append(tabChar);
                stringBuf.append(tabChar);
                stringBuf.append("MKT ORDER ASK VOL: ");
                stringBuf.append(orderInfo.MKOASK_VOL);
                stringBuf.append(eolChar);
                stringBuf.append(tabChar);
                stringBuf.append(tabChar);
                stringBuf.append("MKT ORDER BID VOL: ");
                stringBuf.append(orderInfo.MKOBID_VOL);
                stringBuf.append(eolChar);
                stringBuf.append(tabChar);
                stringBuf.append(tabChar);
                stringBuf.append("Lifetime: ");
                stringBuf.append(orderInfo.lifetime);
                stringBuf.append(eolChar);
            }
        }

        return stringBuf.toString();
    }

}
