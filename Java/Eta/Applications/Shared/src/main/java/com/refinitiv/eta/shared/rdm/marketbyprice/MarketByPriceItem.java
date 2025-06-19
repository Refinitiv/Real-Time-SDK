/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.marketbyprice;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RealHints;

/**
 * Market by price item data.
 */
public class MarketByPriceItem
{
    public static final int ORDER_PRC_FID = 3427; /* Order Price */
    public static final int ORDER_SIDE_FID = 3428; /* Order Side */
    public static final int ORDER_SIZE_FID = 3429; /* Order Size */ 
    public static final int QUOTIM_MS_FID = 3855; /* Quote Time */
    public static final int NO_ORD_FID = 3430; /* Number of Orders */
    public static final int MKOASK_VOL_FID = 3839;
    public static final int MKOBID_VOL_FID = 3840;
    public static final int CURRENCY_FID = 15;
   
    public static final int ORDER_BID = 1;
    public static final int ORDER_ASK = 2;
    public static final int MAX_ORDERS = 3;

    public static Enum USD_ENUM;
    public static Enum BBO_ENUM;

    public String itemName;
    public boolean isInUse;
    
    /* market by order item data */
    public List<PriceInfo> priceInfoList;
    
    public boolean includePrivateStream = false;

    private StringBuilder stringBuf = new StringBuilder();
    private final static String eolChar = "\n";
    private final static String tabChar = "\t";

    
    /* market by order fields for each order */
    public class PriceInfo
    {
        public Real ORDER_PRC;
        public String MKT_MKR_ID;
        public Real ORDER_SIZE;
        public String PRICE_POINT;
        public Enum ORDER_SIDE;
        public long QUOTIM_MS;
        public long NO_ORD;
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
        priceInfoList = new ArrayList<PriceInfo>(MAX_ORDERS);
        for (int i = 0; i < MAX_ORDERS; i++)
        {
            priceInfoList.add(new PriceInfo());
        }
    }

    /**
     * Clears the contents of this object.
     */
    public void clear()
    {
        isInUse = false;
        for (PriceInfo priceInfo : priceInfoList)
        {
            priceInfo.MKT_MKR_ID = null;
            priceInfo.ORDER_PRC.clear();
            priceInfo.ORDER_SIZE.clear();
            priceInfo.ORDER_SIDE.clear();
            priceInfo.QUOTIM_MS = 0;
            priceInfo.PRICE_POINT = null;
            priceInfo.NO_ORD = 0;
            priceInfo.MKOASK_VOL.clear();
            priceInfo.MKOBID_VOL.clear();
            priceInfo.lifetime = 0;
        }
    }

    /**
     * Initializes market by price item fields.
     */
    public void initFields()
    {
        isInUse = true;
        int idx = -1;
        for (PriceInfo priceInfo : priceInfoList)
        {
            ++idx;
            priceInfo.MKT_MKR_ID = "MarketMaker" + String.valueOf(idx + 1);
            priceInfo.ORDER_PRC.value(3459 + 100 * idx, RealHints.EXPONENT_2);
            priceInfo.ORDER_SIZE.value(5 + idx, RealHints.EXPONENT0);
            priceInfo.ORDER_SIDE.value((idx >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID);
            priceInfo.QUOTIM_MS = 500 * idx;
            priceInfo.PRICE_POINT = priceInfo.ORDER_PRC + ((idx >= MAX_ORDERS / 2) ? "a" : "b");
            priceInfo.NO_ORD = 1+idx;
            priceInfo.MKOASK_VOL.value(2 + idx, RealHints.EXPONENT0);
            priceInfo.MKOBID_VOL.value(3 + idx, RealHints.EXPONENT0);
            priceInfo.lifetime = 5 + idx;
        }
    }
    
    /**
     * Updates any item that's currently in use.
     */
    public void updateFields()
    {
        int idx = -1;

        for (PriceInfo priceInfo : priceInfoList)
        {
            ++idx;
            priceInfo.ORDER_PRC.value(priceInfo.ORDER_PRC.toLong() + 1, priceInfo.ORDER_PRC.hint());
            priceInfo.QUOTIM_MS += 1000;

            priceInfo.MKOASK_VOL.value(priceInfo.MKOASK_VOL.toLong() + 1,
                                       priceInfo.ORDER_PRC.hint());
            priceInfo.MKOBID_VOL.value(priceInfo.MKOASK_VOL.toLong() + 1,
                                       priceInfo.ORDER_PRC.hint());

            if (priceInfo.lifetime == 0)
                priceInfo.lifetime = 5 + idx;
            else
                --priceInfo.lifetime;
        }
    }

    public String toString()
    {
        stringBuf.setLength(0);
        for (PriceInfo priceInfo : priceInfoList)
        {
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("MRKT MAKER ID: ");
            stringBuf.append(priceInfo.MKT_MKR_ID);
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER PRICE: ");
            stringBuf.append(priceInfo.ORDER_PRC.toDouble());
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER SIZE: ");
            stringBuf.append(priceInfo.ORDER_SIZE.toDouble());
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("ORDER SIDE: ");
            stringBuf.append((priceInfo.ORDER_SIDE.toInt() == ORDER_BID) ? "BID" : "ASK");
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("QUOTIM MS: ");
            stringBuf.append(priceInfo.QUOTIM_MS);
            stringBuf.append(eolChar);
            stringBuf.append(tabChar);
            stringBuf.append(tabChar);
            stringBuf.append("No ORDERS: ");
            stringBuf.append(priceInfo.NO_ORD);
            stringBuf.append(eolChar);
            if (includePrivateStream)
            {
                stringBuf.append(tabChar);
                stringBuf.append(tabChar);
                stringBuf.append("MKT ORDER ASK VOL: ");
                stringBuf.append(priceInfo.MKOASK_VOL);
                stringBuf.append(eolChar);
                stringBuf.append(tabChar);
                stringBuf.append(tabChar);
                stringBuf.append("MKT ORDER BID VOL: ");
                stringBuf.append(priceInfo.MKOBID_VOL);
                stringBuf.append(eolChar);
                stringBuf.append(tabChar);
                stringBuf.append(tabChar);
                stringBuf.append("Lifetime: ");
                stringBuf.append(priceInfo.lifetime);
                stringBuf.append(eolChar);
            }
        }

        return stringBuf.toString();
    }

}