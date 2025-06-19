/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.marketbyorder;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRefreshFlags;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;

/**
 * The market by order refresh message. Used by an OMM Non-Interactive Provider
 * and OMM Interactive provider to encode/decode a market by order refresh message.
 */
public class MarketByOrderRefresh extends MarketByOrderResponseBase
{
    private int serviceId;
    private int flags;
    private State state;
    private Qos qos;

    // used for encoding
    private RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

    /**
     * Instantiates a new market by order refresh.
     */
    public MarketByOrderRefresh()
    {
        state = CodecFactory.createState();
        qos = CodecFactory.createQos();
    }

    /* (non-Javadoc)
     * @see com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase#isRefreshType()
     */
    @Override
    protected boolean isRefreshType()
    {
        return true;
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
     * The market by order refresh flags. Populated by {@link MarketPriceRefreshFlags}.
     *
     * @return the int
     */
    public int flags()
    {
        return flags;
    }

    /**
     * The market by order refresh flags. Populated by {@link MarketPriceRefreshFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags)
    {
        this.flags = flags;
    }

    /**
     * state of the market by order.
     * 
     * @return {@link State}
     */
    public State state()
    {
        return state;
    }

    /**
     * qos value of the market by order.
     * 
     * @return {@link Qos}
     */
    public Qos qos()
    {
        return qos;
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
     * 
     */
    public void applyHasServiceId()
    {
        flags |= MarketPriceRefreshFlags.HAS_SERVICE_ID;
    }

    /* (non-Javadoc)
     * @see com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl#toString()
     */
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "MarketByOrderRefresh: \n");

        stringBuf.append(tabChar);
        stringBuf.append("state: ");
        stringBuf.append(state());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("solicited: ");
        stringBuf.append(checkSolicited());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("refreshComplete: ");
        stringBuf.append(checkRefreshComplete());
        stringBuf.append(eolChar);

        if (checkHasServiceId())
        {
            stringBuf.append(tabChar);
            stringBuf.append("serviceId: ");
            stringBuf.append(serviceId());
            stringBuf.append(eolChar);
        }
        if (checkHasQos())
        {
            stringBuf.append(tabChar);
            stringBuf.append("qos: ");
            stringBuf.append(refreshMsg.qos().toString());
            stringBuf.append(eolChar);
        }

        return stringBuf.toString();
    }

    /* (non-Javadoc)
     * @see com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase#encodeSummaryData(com.refinitiv.eta.codec.EncodeIterator, com.refinitiv.eta.codec.DataDictionary, com.refinitiv.eta.codec.Map)
     */
    protected int encodeSummaryData(EncodeIterator encodeIter, DataDictionary dictionary, Map map)
    {
        // encode fields for summary data
        fieldList.clear();
        fieldList.applyHasStandardData();
        int ret = fieldList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        //CURRENCY 
        fieldEntry.clear();
        DictionaryEntry dictionaryEntry = dictionary.entry(MarketByOrderItem.CURRENCY_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketByOrderItem.CURRENCY_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            ret = fieldEntry.encode(encodeIter, MarketByOrderItem.USD_ENUM);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        //MARKET STATUS INDICATOR
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketByOrderItem.MKT_STATUS_IND_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketByOrderItem.MKT_STATUS_IND_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            ret = fieldEntry.encode(encodeIter, MarketByOrderItem.BBO_ENUM);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        //ACTIVE DATE
        fieldEntry.clear();
        dictionaryEntry = dictionary.entry(MarketByOrderItem.ACTIVE_DATE_FID);
        if (dictionaryEntry != null)
        {
            fieldEntry.fieldId(MarketByOrderItem.ACTIVE_DATE_FID);
            fieldEntry.dataType(dictionaryEntry.rwfType());
            tempDateTime.localTime();
            if ((ret =
                    fieldEntry.encode(encodeIter, tempDateTime.date())) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
         

        return fieldList.encodeComplete(encodeIter, true);
    }

    /* (non-Javadoc)
     * @see com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase#encodeMapEntries(com.refinitiv.eta.codec.EncodeIterator, com.refinitiv.eta.codec.DataDictionary)
     */
    protected int encodeMapEntries(EncodeIterator encodeIter, DataDictionary dictionary)
    {
        DictionaryEntry dictionaryEntry;
        int ret = CodecReturnCodes.SUCCESS;

        // encode the order book
        for (MarketByOrderItem.OrderInfo orderInfo : marketByOrderItem().orderInfoList)
        {
            mapEntry.clear();
            mapEntry.action(MapEntryActions.ADD);
            tmpBuffer.clear();
            tmpBuffer.data(orderInfo.ORDER_ID);
            if ((ret = mapEntry.encodeInit(encodeIter, tmpBuffer, 0)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            fieldList.clear();
            fieldList.applyHasStandardData();
            fieldList.applyHasSetData();
            fieldList.applyHasSetId();
            fieldList.setId(0);
            ret = fieldList.encodeInit(encodeIter, marketByOrderSetDefDb, 0);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            //encode fields

            //ORDER_PRC
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketByOrderItem.ORDER_PRC_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketByOrderItem.ORDER_PRC_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                ret = fieldEntry.encode(encodeIter, orderInfo.ORDER_PRC);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            //ORDER_SIZE
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketByOrderItem.ORDER_SIZE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketByOrderItem.ORDER_SIZE_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                ret = fieldEntry.encode(encodeIter, orderInfo.ORDER_SIZE);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            //QUOTIM_MS
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketByOrderItem.QUOTIM_MS_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketByOrderItem.QUOTIM_MS_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                tempUInt.value(orderInfo.QUOTIM_MS);
                /*
                 * This encoding completes the encoding of the ORDER_PRC,
                 * ORDER_SIZE, QUOTIM_MS set. When a set is completed, a success
                 * code, CodecReturnCodes.SET_COMPLETE, is returned to indicate
                 * this. This may be used to ensure that the set is being used
                 * as intended.
                 */
                ret = fieldEntry.encode(encodeIter, tempUInt);
                if (ret != CodecReturnCodes.SET_COMPLETE)
                {
                    return CodecReturnCodes.FAILURE;
                }
            }

            if (checkPrivateStream())
            {
                //MKOASK_VOL
                fieldEntry.clear();
                dictionaryEntry = dictionary.entry(MarketByOrderItem.MKOASK_VOL_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.fieldId(MarketByOrderItem.MKOASK_VOL_FID);
                    fieldEntry.dataType(dictionaryEntry.rwfType());
                    ret = fieldEntry.encode(encodeIter, orderInfo.MKOASK_VOL);
                    if (ret < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }

                //MKOBID_VOL
                fieldEntry.clear();
                dictionaryEntry = dictionary.entry(MarketByOrderItem.MKOBID_VOL_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.fieldId(MarketByOrderItem.MKOBID_VOL_FID);
                    fieldEntry.dataType(dictionaryEntry.rwfType());
                    ret = fieldEntry.encode(encodeIter, orderInfo.MKOBID_VOL);
                    if (ret < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            /*
             * Since the field list specified FieldListFlags.HAS_STANDARD_DATA
             * as well, standard entries can be encoded after the set is
             * finished. The Add actions will additionally include the
             * ORDER_SIDE and MKT_MKR_ID fields.
             */

            //ORDER_SIDE
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketByOrderItem.ORDER_SIDE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketByOrderItem.ORDER_SIDE_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                ret = fieldEntry.encode(encodeIter, orderInfo.ORDER_SIDE);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            //MKT_MKR_ID 
            fieldEntry.clear();
            dictionaryEntry = dictionary.entry(MarketByOrderItem.MKT_MKR_ID_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.fieldId(MarketByOrderItem.MKT_MKR_ID_FID);
                fieldEntry.dataType(dictionaryEntry.rwfType());
                tmpBuffer.data(orderInfo.MKT_MKR_ID);
                ret = fieldEntry.encode(encodeIter, tmpBuffer);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            ret = fieldList.encodeComplete(encodeIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            ret = mapEntry.encodeComplete(encodeIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return ret;
    }
    
    /* (non-Javadoc)
     * @see com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase#encodeMsg()
     */
    public Msg encodeMsg()
    {
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.streamId(streamId());
        refreshMsg.domainType(DomainTypes.MARKET_BY_ORDER);
        refreshMsg.containerType(DataTypes.MAP);
        refreshMsg.applyHasMsgKey();

        if (checkRefreshComplete())
        {
            refreshMsg.applyRefreshComplete();
        }
        if (checkSolicited())
        {
            refreshMsg.applySolicited();
        }
        
        if(checkClearCache())
        {
            refreshMsg.applyClearCache();
        }
        
        if (checkHasServiceId())
        {
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(serviceId());
        }
        
        if (checkPrivateStream())
        {
            refreshMsg.applyPrivateStream();
        }
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().applyHasNameType();

        refreshMsg.state().dataState(state().dataState());
        refreshMsg.state().streamState(state().streamState());
        refreshMsg.state().code(state().code());
        refreshMsg.state().text(state().text());

        //Itemname
        refreshMsg.msgKey().name(itemName());
        refreshMsg.msgKey().nameType(InstrumentNameTypes.RIC);

        if (checkHasQos())
        {
            refreshMsg.applyHasQos();
            refreshMsg.qos().dynamic(qos().isDynamic());
            refreshMsg.qos().rate(qos().rate());
            refreshMsg.qos().timeliness(qos().timeliness());
            refreshMsg.qos().rateInfo(qos().rateInfo());
            refreshMsg.qos().timeInfo(qos().timeInfo());
        }

        return refreshMsg;
    }
    
    /* (non-Javadoc)
     * @see com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase#domainType()
     */
    @Override
    public int domainType()
    {
        return DomainTypes.MARKET_BY_ORDER;
    }
}
