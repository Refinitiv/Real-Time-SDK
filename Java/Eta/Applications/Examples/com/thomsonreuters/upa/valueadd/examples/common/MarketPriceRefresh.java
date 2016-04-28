package com.thomsonreuters.upa.valueadd.examples.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;

/**
 * The market price refresh message. Used by an OMM Consumer, OMM
 * Non-Interactive Provider and OMM Interactive provider to encode/decode a
 * market price refresh message.
 */
public class MarketPriceRefresh extends MarketPriceResponseBase
{
    private State state;
    private Qos qos;

    // used for encoding
    private RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

    public MarketPriceRefresh()
    {
        state = CodecFactory.createState();
        qos = CodecFactory.createQos();
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        state.clear();
        qos.clear();
        flags = 0;
    }

  
    /**
     * state of the market price.
     * 
     * @return {@link State}
     */
    public State state()
    {
        return state;
    }

    /**
     * qos value of the market price.
     * 
     * @return {@link Qos}
     */
    public Qos qos()
    {
        return qos;
    }

    /**
     * Checks the presence of refresh complete flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkRefreshComplete()
    {
        return (flags & MarketPriceRefreshFlags.REFRESH_COMPLETE) != 0;
    }

    /**
     * Applies refresh complete flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyRefreshComplete()
    {
        flags |= MarketPriceRefreshFlags.REFRESH_COMPLETE;
    }
    
    /**
     * Checks the presence of clear cache flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache()
    {
        return (flags & MarketPriceRefreshFlags.CLEAR_CACHE) != 0;
    }

    /**
     * Applies clear cache flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyClearCache()
    {
        flags |= MarketPriceRefreshFlags.CLEAR_CACHE;
    }

    /**
     * Checks the presence of solicited flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkSolicited()
    {
        return (flags & MarketPriceRefreshFlags.SOLICITED) != 0;
    }

    /**
     * Applies solicited flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applySolicited()
    {
        flags |= MarketPriceRefreshFlags.SOLICITED;
    }

    /**
     * Checks the presence of Qos flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasQos()
    {
        return (flags & MarketPriceRefreshFlags.HAS_QOS) != 0;
    }

    /**
     * Applies qos flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasQos()
    {
        flags |= MarketPriceRefreshFlags.HAS_QOS;
    }

    public Msg encodeMsg()
    {
        refreshMsg.clear();

        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.streamId(streamId());
        refreshMsg.applyHasMsgKey();

        if (checkPrivateStream())
        {
            refreshMsg.applyPrivateStream();
        }

        if (checkRefreshComplete())
        {
            refreshMsg.applyRefreshComplete();
        }
        
        if (checkClearCache())
        {
            refreshMsg.applyClearCache();
        }
        
        if (checkSolicited())
        {
            refreshMsg.applySolicited();
        }

        if (checkPrivateStream())
        {
            refreshMsg.applyPrivateStream();
        }

        if (checkHasServiceId())
        {
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(serviceId());
        }

        // Itemname
        refreshMsg.msgKey().name(itemName());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        refreshMsg.msgKey().applyHasNameType();

        // State
        refreshMsg.state().dataState(state().dataState());
        refreshMsg.state().streamState(state().streamState());
        refreshMsg.state().code(state().code());
        refreshMsg.state().text(state().text());

        if (checkHasQos())
        {
            refreshMsg.applyHasQos();
            refreshMsg.qos().dynamic(qos().isDynamic());
            refreshMsg.qos().rate(qos().rate());
            refreshMsg.qos().timeliness(qos().timeliness());
            refreshMsg.qos().rateInfo(qos().rateInfo());
            refreshMsg.qos().timeInfo(qos().timeInfo());
        }

        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.containerType(DataTypes.FIELD_LIST);

        return refreshMsg;
    }

    public String toString()
    {
        StringBuilder toStringBuilder = super.buildStringBuffer();
        toStringBuilder.insert(0, "MarketPriceRefresh: ");

        toStringBuilder.append(tabChar);
        toStringBuilder.append("state: ");
        toStringBuilder.append(state());
        toStringBuilder.append(eolChar);

        toStringBuilder.append(tabChar);
        toStringBuilder.append("solicited: ");
        toStringBuilder.append(checkSolicited());
        toStringBuilder.append(eolChar);

        toStringBuilder.append(tabChar);
        toStringBuilder.append("refreshComplete: ");
        toStringBuilder.append(checkRefreshComplete());
        toStringBuilder.append(eolChar);

        if (checkHasServiceId())
        {
            toStringBuilder.append(tabChar);
            toStringBuilder.append("serviceId: ");
            toStringBuilder.append(serviceId());
            toStringBuilder.append(eolChar);
        }
        if (checkHasQos())
        {
            toStringBuilder.append(tabChar);
            toStringBuilder.append("qos: ");
            toStringBuilder.append(refreshMsg.qos().toString());
            toStringBuilder.append(eolChar);
        }

        return toStringBuilder.toString();
    }

    protected int encodeRefreshFields(EncodeIterator encodeIter, DataDictionary dictionary)
    {
        MarketPriceItem mpItem = marketPriceItem();
        DictionaryEntry dictionaryEntry;
        int ret = CodecReturnCodes.SUCCESS;
        
        // RDNDISPLAY
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.RDNDISPLAY_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.RDNDISPLAY_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempUInt.value(mpItem.RDNDISPLAY);
            if ((ret = fieldEntry.encode(encodeIter, tempUInt)) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        // RDN_EXCHID
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.RDN_EXCHID_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.RDN_EXCHID_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            com.thomsonreuters.upa.codec.Enum enumValue = CodecFactory.createEnum();
            enumValue.value(mpItem.RDN_EXCHID);
            if ((ret = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        // DIVPAYDATE
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.DIVPAYDATE_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketPriceItem.DIVPAYDATE_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            return fieldEntry.encode(encodeIter, mpItem.DIVPAYDATE);
        }

        return ret;
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.MARKET_PRICE;
    }
}
