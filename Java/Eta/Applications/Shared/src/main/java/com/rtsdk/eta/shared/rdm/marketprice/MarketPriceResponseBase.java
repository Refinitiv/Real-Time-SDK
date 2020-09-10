package com.rtsdk.eta.shared.rdm.marketprice;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.DictionaryEntry;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.FieldEntry;
import com.rtsdk.eta.codec.FieldList;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.Real;
import com.rtsdk.eta.codec.RealHints;
import com.rtsdk.eta.codec.UInt;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * Market price response base class for market price refresh and update
 * message.
 */
public abstract class MarketPriceResponseBase extends MsgBaseImpl
{
    private Buffer itemName;
    private MarketPriceItem mpInfo;
    private DataDictionary dictionary;
    protected int flags;
    private int serviceId;
    
    protected final static String eolChar = "\n";
    protected final static String tabChar = "\t";
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    protected Real tempReal = CodecFactory.createReal();
    protected UInt tempUInt = CodecFactory.createUInt();
    
    /**
     * Instantiates a new market price response base.
     */
    protected MarketPriceResponseBase()
    {
        itemName = CodecFactory.createBuffer();
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        serviceId = 0;
        flags = 0;
        itemName.clear();
    }
    
    /**
     * Dictionary for encoding market price refresh/update message.
     * 
     * @return dictionary
     */
    public DataDictionary dictionary()
    {
        return dictionary;
    }

    /**
     * Dictionary for encoding market price refresh/update message.
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
     * Market price item.
     *
     * @return market price item data.
     */
    public MarketPriceItem marketPriceItem()
    {
        return mpInfo;
    }
    
    /**
     * Service id.
     *
     * @return service id
     */
    public int serviceId()
    {
        return serviceId;
    }

    /**
     * Service id.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /**
     * The market price refresh flags. Populated by
     * {@link MarketPriceRefreshFlags}.
     *
     * @return the int
     */
    public int flags()
    {
        return flags;
    }

    /**
     * The market price refresh flags. Populated by
     * {@link MarketPriceRefreshFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags)
    {
        this.flags = flags;
    }
    
    /**
     * Market price item.
     *
     * @param itemData - market price item data.
     */
    public void marketPriceItem(MarketPriceItem itemData)
    {
        this.mpInfo = itemData;
    }
    
    /**
     * Checks the presence of private stream flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream()
    {
        return (flags & MarketPriceRefreshFlags.PRIVATE_STREAM) != 0;
    }

    /**
     * Applies private stream flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyPrivateStream()
    {
        flags |= MarketPriceRefreshFlags.PRIVATE_STREAM;
    }

    /**
     * Checks the presence of service id flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId()
    {
        return (flags & MarketPriceRefreshFlags.HAS_SERVICE_ID) != 0;
    }

    /**
     * Applies service id flag.
     */
    public void applyHasServiceId()
    {
        flags |= MarketPriceRefreshFlags.HAS_SERVICE_ID;
    }
    
    
    /**
     * Encodes the market price response. 
     * Returns success if encoding succeeds or failure if encoding fails.
     * @param encodeIter - encode iterator
     * @return {@link CodecReturnCodes}
     */
    public int encode(EncodeIterator encodeIter)
    {
        fieldList.clear();
        fieldEntry.clear();

        tempUInt.clear();
        tempReal.clear();

        MarketPriceItem mpItem = marketPriceItem();
        
        // set-up message
        Msg msg = encodeMsg();

        // encode message
        int ret = msg.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        // encode field list
        fieldList.applyHasStandardData();
        ret = fieldList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        // encode fields specific to refresh
        ret = encodeRefreshFields(encodeIter, dictionary);
        if(ret < CodecReturnCodes.SUCCESS)
            return ret;
        
        // encode fields common to refresh and updates
        
        // TRDPRC_1
        fieldEntry.clear();
        DictionaryEntry dictionaryEntry = dictionary.entry(MarketPriceItem.TRDPRC_1_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.TRDPRC_1_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(mpItem.TRDPRC_1, RealHints.EXPONENT_2);
            ret = fieldEntry.encode(encodeIter, tempReal);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        // BID
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.BID_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.BID_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(mpItem.BID, RealHints.EXPONENT_2);
            ret = fieldEntry.encode(encodeIter, tempReal);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        // ASK
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.ASK_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.ASK_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(mpItem.ASK, RealHints.EXPONENT_2);
            ret = fieldEntry.encode(encodeIter, tempReal);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        // ACVOL_1
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.ACVOL_1_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.ACVOL_1_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(mpItem.ACVOL_1, RealHints.EXPONENT_2);
            ret = fieldEntry.encode(encodeIter, tempReal);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        // NETCHNG_1
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.NETCHNG_1_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.NETCHNG_1_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(mpItem.NETCHNG_1, RealHints.EXPONENT_2);
            ret = fieldEntry.encode(encodeIter, tempReal);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        // ASK_TIME
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.ASK_TIME_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.ASK_TIME_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            ret = fieldEntry.encode(encodeIter, mpItem.ASK_TIME.time());
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        // encode items for private stream
        if (checkPrivateStream())
        {
            // PERATIO
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketPriceItem.PERATIO_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketPriceItem.PERATIO_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                tempReal.clear();
                tempReal.value(mpItem.PERATIO, RealHints.EXPONENT_2);
                ret = fieldEntry.encode(encodeIter, tempReal);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            // SALTIME
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketPriceItem.SALTIME_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketPriceItem.SALTIME_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                ret = fieldEntry.encode(encodeIter, mpItem.SALTIME.time());
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
        }

        // complete encode field list
        ret = fieldList.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

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
     * Encode refresh fields.
     *
     * @param encodeIter the encode iter
     * @param dictionary the dictionary
     * @return the int
     */
    protected int encodeRefreshFields(EncodeIterator encodeIter, DataDictionary dictionary)
    {
       return CodecReturnCodes.SUCCESS;
    }

    /* (non-Javadoc)
     * @see com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl#buildStringBuffer()
     */
    public StringBuilder buildStringBuffer()
    {
        StringBuilder stringBuf = super.buildStringBuffer();

        stringBuf.append(tabChar);
        stringBuf.append("itemName: ");
        stringBuf.append(itemName());
        stringBuf.append(eolChar);
        
        if(checkHasServiceId())
        {
            stringBuf.append(tabChar);
            stringBuf.append("serviceId: ");
            stringBuf.append(serviceId());
            stringBuf.append(eolChar);
        }
        stringBuf.append(tabChar);
        stringBuf.append("item info: ");
        stringBuf.append(eolChar);
        stringBuf.append(mpInfo.toString());
        stringBuf.append(eolChar);
        return stringBuf;
    }
   

    /* (non-Javadoc)
     * @see com.rtsdk.eta.valueadd.domainrep.rdm.MsgBase#decode(com.rtsdk.eta.codec.DecodeIterator, com.rtsdk.eta.codec.Msg)
     */
    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        throw new UnsupportedOperationException();
    }
   
}