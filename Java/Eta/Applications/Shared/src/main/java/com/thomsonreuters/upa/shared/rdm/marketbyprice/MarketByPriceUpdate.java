package com.thomsonreuters.upa.shared.rdm.marketbyprice;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRefreshFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;

/**
 * The market by price update message. Used by an OMM Interactive provider to
 * encode/decode a market by price update message.
 */
public class MarketByPriceUpdate extends MarketByPriceResponseBase
{
    private int flags;
    private int serviceId;

    private UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();

    /**
     * Instantiates a new market by price update.
     */
    public MarketByPriceUpdate()
    {
        super();
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        flags = 0;
        serviceId = 0;
    }


    /**
     * Checks the presence of service id flag.
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId()
    {
        return (flags & MarketPriceRefreshFlags.HAS_SERVICE_ID) != 0;
    }

    /**
     * Sets service id flag.
     */
    public void applyHasServiceId()
    {
        flags |= MarketPriceRefreshFlags.HAS_SERVICE_ID;
    }
    
    /**
     * Sets Private stream flag.
     */
    public void applyPrivateStream()
    {
        flags |= MarketPriceRefreshFlags.PRIVATE_STREAM;
    }

    /**
     * Checks the presence of private stream flag.
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream()
    {
        return (flags & MarketPriceRefreshFlags.PRIVATE_STREAM) != 0;
    }
    

    /**
     * Sets Private stream flag.
     */
    public void applyRefreshComplete()
    {
        flags |= MarketPriceRefreshFlags.REFRESH_COMPLETE;
    }

    /**
     * Checks the presence of refresh complete flag.
     * @return true - if exists; false if does not exist.
     */
    public boolean checkRefreshComplete()
    {
        return (flags & MarketPriceRefreshFlags.REFRESH_COMPLETE) != 0;
    }

    
    /**
     * Service id.
     *
     * @return serviceId
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

    /* (non-Javadoc)
     * @see com.thomsonreuters.upa.shared.rdm.marketbyprice.MarketByPriceResponseBase#encodeMsg()
     */
    @Override
    public Msg encodeMsg()
    {
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(streamId());
        updateMsg.domainType(DomainTypes.MARKET_BY_PRICE);
        updateMsg.containerType(DataTypes.MAP);

        /*
         * although msg key is not required, it is provided here for
         * completeness
         */
        if (checkHasServiceId())
        {
            /* attrib info */
            updateMsg.applyHasMsgKey();
            /* service Id */
            updateMsg.msgKey().serviceId(serviceId);
            updateMsg.msgKey().applyHasServiceId();
            /* name */
            updateMsg.msgKey().name(itemName());
            updateMsg.msgKey().applyHasName();
            /* name type */
            updateMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            updateMsg.msgKey().applyHasNameType();
        }

        return updateMsg;
    }

    /* (non-Javadoc)
     * @see com.thomsonreuters.upa.shared.rdm.marketbyprice.MarketByPriceResponseBase#encodeMapEntries(com.thomsonreuters.upa.codec.EncodeIterator, com.thomsonreuters.upa.codec.DataDictionary)
     */
    @Override
    protected int encodeMapEntries(EncodeIterator encodeIter, DataDictionary dictionary)
    {
        DictionaryEntry dictionaryEntry;
        int ret = CodecReturnCodes.SUCCESS;

        //encode the order book
        mapEntry.clear();

        int idx = -1;
        for (MarketByPriceItem.PriceInfo priceInfo : marketByPriceItem().priceInfoList)
        {
            ++idx;

            if (priceInfo.lifetime != 0)
            {
                /*
                 * determine if this is an ADD or UPDATE. This logic matches the
                 * logic used in MarketByOrderItem.
                 */
                if (priceInfo.lifetime == (5 + idx) && checkRefreshComplete())
                    mapEntry.action(MapEntryActions.ADD);
                else
                    mapEntry.action(MapEntryActions.UPDATE);

                tmpBuffer.data(priceInfo.PRICE_POINT);
                if ((ret = mapEntry.encodeInit(encodeIter, tmpBuffer, 0)) != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }

                fieldList.clear();
                fieldList.applyHasStandardData();
                
                fieldList.applyHasSetData();
                fieldList.applyHasSetId();
                fieldList.setId(0);
                
                if ((ret = fieldList.encodeInit(encodeIter, marketByPriceSetDefDb, 0)) < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                
                //encode fields

                //ORDER_PRC
                fieldEntry.clear();
                dictionaryEntry = dictionary.entry(MarketByPriceItem.ORDER_PRC_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.fieldId(MarketByPriceItem.ORDER_PRC_FID);
                    fieldEntry.dataType(dictionaryEntry.rwfType());
                    if ((ret = fieldEntry.encode(encodeIter, priceInfo.ORDER_PRC)) < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }

                //ORDER_SIZE
                fieldEntry.clear();
                dictionaryEntry = dictionary.entry(MarketByPriceItem.ORDER_SIZE_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.fieldId(MarketByPriceItem.ORDER_SIZE_FID);
                    fieldEntry.dataType(dictionaryEntry.rwfType());
                    if ((ret = fieldEntry.encode(encodeIter, priceInfo.ORDER_SIZE)) < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }

                // QUOTIM_MS
                fieldEntry.clear();
                dictionaryEntry = dictionary.entry(MarketByPriceItem.QUOTIM_MS_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.fieldId(MarketByPriceItem.QUOTIM_MS_FID);
                    fieldEntry.dataType(dictionaryEntry.rwfType());
                    tempUInt.value(priceInfo.QUOTIM_MS);
                    ret = fieldEntry.encode(encodeIter, tempUInt);
                    if (ret < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }

                // NO_ORD
                fieldEntry.clear();
                dictionaryEntry = dictionary.entry(MarketByPriceItem.NO_ORD_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.fieldId(MarketByPriceItem.NO_ORD_FID);
                    fieldEntry.dataType(dictionaryEntry.rwfType());
                    tempUInt.value(priceInfo.NO_ORD);
                    /*
                     * This encoding completes the encoding of the ORDER_PRC,
                     * ORDER_SIZE, QUOTIM_MS, NO_ORD set. When a set is
                     * completed, a success code, CodecReturnCodes.SET_COMPLETE,
                     * is returned to indicate this. This may be used to ensure
                     * that the set is being used as intended.
                     */
                    ret = fieldEntry.encode(encodeIter, tempUInt);
                    if (ret != CodecReturnCodes.SET_COMPLETE)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                }
               

                //encode items for private stream
                if (checkPrivateStream())
                {
                    //MKOASK_VOL
                    fieldEntry.clear();
                    dictionaryEntry = dictionary.entry(MarketByPriceItem.MKOASK_VOL_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.fieldId(MarketByPriceItem.MKOASK_VOL_FID);
                        fieldEntry.dataType(dictionaryEntry.rwfType());
                        if ((ret = fieldEntry.encode(encodeIter, priceInfo.MKOASK_VOL)) < CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                    }

                    //MKOBID_VOL
                    fieldEntry.clear();
                    dictionaryEntry = dictionary.entry(MarketByPriceItem.MKOBID_VOL_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.fieldId(MarketByPriceItem.MKOBID_VOL_FID);
                        fieldEntry.dataType(dictionaryEntry.rwfType());
                        if ((ret = fieldEntry.encode(encodeIter, priceInfo.MKOBID_VOL)) < CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                    }
                }

                /*
                 * Since the field list specified
                 * FieldListFlags.HAS_STANDARD_DATA as well, standard entries
                 * can be encoded after the set is finished. The Add actions
                 * will additionally include the ORDER_SIDE and MKT_MKR_ID
                 * fields.
                 */

                if (priceInfo.lifetime == (5 + idx) && checkRefreshComplete())
                {
                    //ORDER_SIDE
                    fieldEntry.clear();
                    dictionaryEntry = dictionary.entry(MarketByPriceItem.ORDER_SIDE_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.fieldId(MarketByPriceItem.ORDER_SIDE_FID);
                        fieldEntry.dataType(dictionaryEntry.rwfType());
                        if ((ret = fieldEntry.encode(encodeIter, priceInfo.ORDER_SIDE)) < CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                    }
                }
                if ((ret = fieldList.encodeComplete(encodeIter, true)) != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }

                if ((ret = mapEntry.encodeComplete(encodeIter, true)) != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                //delete the order
                mapEntry.action(MapEntryActions.DELETE);
                tmpBuffer.data(priceInfo.PRICE_POINT);
                if ((ret = mapEntry.encode(encodeIter, tmpBuffer)) != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
        }

        return ret;
    }

    /* (non-Javadoc)
     * @see com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl#toString()
     */
    public String toString()
    {
        StringBuilder toStringBuilder = super.buildStringBuffer();
        toStringBuilder.insert(0, "MarketByPriceUpdate: ");

        if (checkHasServiceId())
        {
            toStringBuilder.append(tabChar);
            toStringBuilder.append("serviceId: ");
            toStringBuilder.append(serviceId());
            toStringBuilder.append(eolChar);
        }

        return toStringBuilder.toString();

    }
    
    /* (non-Javadoc)
     * @see com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase#domainType()
     */
    @Override
    public int domainType()
    {
        return DomainTypes.MARKET_BY_PRICE;
    }
}