package com.refinitiv.eta.shared.rdm.marketbyprice;

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
 * Market by price response base class for market by price refresh and update
 * message.
 */
public abstract class MarketByPriceResponseBase extends MsgBaseImpl
{
    private Buffer itemName = CodecFactory.createBuffer();
    private MarketByPriceItem mbpInfo;
    private DataDictionary dictionary;
    private int partNo;

    protected final static String eolChar = "\n";
    protected final static String tabChar = "\t";

    protected final static int ENCODED_SET_DEF_SIZE = 60;
    protected Buffer tmpBuffer = CodecFactory.createBuffer();
    protected Map map = CodecFactory.createMap();
    protected MapEntry mapEntry = CodecFactory.createMapEntry();
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    protected Real tempReal = CodecFactory.createReal();
    protected UInt tempUInt = CodecFactory.createUInt();
    protected DateTime tempDateTime = CodecFactory.createDateTime();
    protected LocalFieldSetDefDb marketByPriceSetDefDb;

    /**
     * Instantiates a new market by price response base.
     */
    protected MarketByPriceResponseBase()
    {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.clear();
        setupMarketByPriceSetDefDb(encIter);
        partNo = -1;
    }

    private void setupMarketByPriceSetDefDb(EncodeIterator encIter)
    {
        FieldSetDefEntry[] fieldSetDefEntries = new FieldSetDefEntry[4];

        // Id=ORDER_PRC type=REAL
        fieldSetDefEntries[0] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[0].dataType(DataTypes.REAL);
        fieldSetDefEntries[0].fieldId(MarketByPriceItem.ORDER_PRC_FID);

        // Id=ORDER_SIZE type=REAL
        fieldSetDefEntries[1] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[1].dataType(DataTypes.REAL);
        fieldSetDefEntries[1].fieldId(MarketByPriceItem.ORDER_SIZE_FID);

        // Id=QUOTIM MS type=UINT_4
        fieldSetDefEntries[2] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[2].dataType(DataTypes.UINT_4);
        fieldSetDefEntries[2].fieldId(MarketByPriceItem.QUOTIM_MS_FID);

        // Number of Orders
        fieldSetDefEntries[3] = CodecFactory.createFieldSetDefEntry();
        fieldSetDefEntries[3].dataType(DataTypes.UINT_4);
        fieldSetDefEntries[3].fieldId(MarketByPriceItem.NO_ORD_FID);

        // Populate FieldSetDef array inside LocalFieldSetDefDb
        marketByPriceSetDefDb = CodecFactory.createLocalFieldSetDefDb();
        marketByPriceSetDefDb.clear();

        // index and ID must match
        marketByPriceSetDefDb.definitions()[0].setId(0); 
        marketByPriceSetDefDb.definitions()[0].count(4);
        marketByPriceSetDefDb.definitions()[0].entries(fieldSetDefEntries);
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        itemName.clear();
        partNo = -1;
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
     * Market by price item.
     *
     * @return market by price item data.
     */
    public MarketByPriceItem marketByPriceItem()
    {
        return mbpInfo;
    }

    /**
     * Market by price item.
     *
     * @param itemData market by price item data
     */
    public void marketByPriceItem(MarketByPriceItem itemData)
    {
        this.mbpInfo = itemData;
    }

    /**
     * Part no.
     *
     * @param partNo - current part number for multi-part refresh message.
     */
    public void partNo(int partNo)
    {
        this.partNo = partNo;
    }

    /**
     * Part no.
     *
     * @return the int
     */
    public int partNo()
    {
        return partNo;
    }

    /**
     * Encodes the market by price response. Returns success if encoding
     * succeeds or failure if encoding fails.
     * 
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

        // encode message
        int ret = msg.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        // encode map
        map.applyHasKeyFieldId();
        map.keyPrimitiveType(DataTypes.BUFFER);
        map.keyFieldId(MarketByPriceItem.ORDER_PRC_FID);
        map.containerType(DataTypes.FIELD_LIST);
        map.applyHasSetDefs();

        if (partNo() == 0)
        {
            map.applyHasSummaryData();
        }

        ret = map.encodeInit(encodeIter, 0, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        ret = marketByPriceSetDefDb.encode(encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        ret = map.encodeSetDefsComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        if (partNo() == 0)
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

        // complete encode field list
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
        stringBuf.append(mbpInfo.toString());
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
