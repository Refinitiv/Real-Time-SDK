/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.marketbyorder;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.FieldSetDefEntry;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * Market by order response base class for market by order refresh and update
 * message.
 */
public abstract class MarketByOrderResponseBase extends MsgBaseImpl
{
    private Buffer itemName = CodecFactory.createBuffer();
    private MarketByOrderItem mboInfo;
    private DataDictionary dictionary;

    protected final static String eolChar = "\n";
    protected final static String tabChar = "\t";
    
    protected Buffer tmpBuffer = CodecFactory.createBuffer();
    protected Map map = CodecFactory.createMap();
    protected MapEntry mapEntry = CodecFactory.createMapEntry();
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    protected Real tempReal = CodecFactory.createReal();
    protected UInt tempUInt = CodecFactory.createUInt();
    protected DateTime tempDateTime = CodecFactory.createDateTime(); 
    protected LocalFieldSetDefDb marketByOrderSetDefDb;

    /**
     * Instantiates a new market by order response base.
     */
    protected MarketByOrderResponseBase()
    {
        setupMarketByOrderSetDefDb();
    }

    /**
     * Setup market by order set def db.
     */
    protected void setupMarketByOrderSetDefDb()
    {
        FieldSetDefEntry[] fieldSetDefEntries = new FieldSetDefEntry[3];

        // Id=ORDER_PRC type=REAL
        fieldSetDefEntries[0] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[0].dataType(DataTypes.REAL);
        fieldSetDefEntries[0].fieldId(MarketByOrderItem.ORDER_PRC_FID);

        // Id=ORDER_SIZE type=REAL
        fieldSetDefEntries[1] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[1].dataType(DataTypes.REAL);
        fieldSetDefEntries[1].fieldId(MarketByOrderItem.ORDER_SIZE_FID);

        // Id=QUOTIM MS type=UINT_4
        fieldSetDefEntries[2] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[2].dataType(DataTypes.UINT_4);
        fieldSetDefEntries[2].fieldId(MarketByOrderItem.QUOTIM_MS_FID);

        // populate FieldSetDef array inside LocalFieldSetDefDb      
        marketByOrderSetDefDb = CodecFactory.createLocalFieldSetDefDb();
        marketByOrderSetDefDb.clear();
        marketByOrderSetDefDb.definitions()[0].setId(0);
        marketByOrderSetDefDb.definitions()[0].count(3);
        marketByOrderSetDefDb.definitions()[0].entries(fieldSetDefEntries);
    }

    /**
     * Checks if is refresh type.
     *
     * @return true, if is refresh type
     */
    protected boolean isRefreshType()
    {
        return false;
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        itemName.clear();
    }

    /**
     * Dictionary for encoding market by order refresh/update message.
     * 
     * @return dictionary
     */
    public DataDictionary dictionary()
    {
        return dictionary;
    }
    
    /**
     * Dictionary for encoding market by order refresh/update message.
     *
     * @param dictionary the dictionary
     */
    public void dictionary(DataDictionary dictionary)
    {
        this.dictionary = dictionary;
    }

    /**
     * Item name.
     *
     * @return item name
     */
    public Buffer itemName()
    {
        return itemName;
    }

    /**
     * Market by order item.
     *
     * @return market by order item data.
     */
    public MarketByOrderItem marketByOrderItem()
    {
        return mboInfo;
    }

    /**
     * Market by order item.
     *
     * @param itemData market by order item data
     */
    public void marketByOrderItem(MarketByOrderItem itemData)
    {
        this.mboInfo = itemData;
    }

    /**
     * Encodes the market by order response. 
     * Returns success if encoding succeeds or failure if encoding fails.
     * @param encodeIter - encode iterator
     * @return {@link CodecReturnCodes}
     */
    public int encode(EncodeIterator encodeIter)
    {
        map.clear();
        fieldList.clear();
        fieldEntry.clear();

        tempUInt.clear();
        tempReal.clear();

        // set-up message
        Msg msg = encodeMsg();
        int ret = msg.encodeInit(encodeIter, 0);
        
        // encode message
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        // encode map
        map.applyHasKeyFieldId();
        map.keyPrimitiveType(DataTypes.BUFFER);
        map.keyFieldId(MarketByOrderItem.ORDER_ID_FID);
        map.containerType(DataTypes.FIELD_LIST);
        map.applyHasSetDefs();
      
        if (isRefreshType())
        {
            map.applyHasSummaryData();
        }

        ret = map.encodeInit(encodeIter, 0, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
        
        ret = marketByOrderSetDefDb.encode(encodeIter);
        if(ret < CodecReturnCodes.SUCCESS)
        	return ret;
        ret = map.encodeSetDefsComplete(encodeIter, true);
        if(ret < CodecReturnCodes.SUCCESS)
        	return ret;
        
        if (isRefreshType())
        {
            // encode summary data
        	ret = encodeSummaryData(encodeIter, dictionary, map);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
            ret = map.encodeSummaryDataComplete(encodeIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        // encode map entries 
        ret = encodeMapEntries(encodeIter, dictionary);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        // complete encode map
        ret = map.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        // complete encode message
        return msg.encodeComplete(encodeIter, true);
    }

    /**
     * Encode msg.
     *
     * @return the msg
     */
    public abstract Msg encodeMsg();

    /**
     * Encode summary data.
     *
     * @param encodeIter the encode iter
     * @param dictionary the dictionary
     * @param map the map
     * @return the int
     */
    protected int encodeSummaryData(EncodeIterator encodeIter, DataDictionary dictionary, Map map)
    {
        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encode map entries.
     *
     * @param encodeIter the encode iter
     * @param dictionary the dictionary
     * @return the int
     */
    protected abstract int encodeMapEntries(EncodeIterator encodeIter, DataDictionary dictionary);

    /* (non-Javadoc)
     * @see com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl#buildStringBuffer()
     */
    public StringBuilder buildStringBuffer()
    {
        StringBuilder stringBuf = super.buildStringBuffer();

        stringBuf.append(tabChar);
        stringBuf.append("itemName: ");
        stringBuf.append(itemName());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("item info: ");
        stringBuf.append(mboInfo.toString());
        stringBuf.append(eolChar);

        return stringBuf;
    }

    /* (non-Javadoc)
     * @see com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase#decode(com.refinitiv.eta.codec.DecodeIterator, com.refinitiv.eta.codec.Msg)
     */
    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        throw new UnsupportedOperationException();
    }
}
