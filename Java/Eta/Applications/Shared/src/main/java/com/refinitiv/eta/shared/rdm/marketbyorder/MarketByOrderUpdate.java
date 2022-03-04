/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.marketbyorder;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRefreshFlags;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;

/**
 * The market by order update message. Used by an OMM Non-Interactive Provider
 * and OMM Interactive provider to encode/decode a market by order update message.
 */
public class MarketByOrderUpdate extends MarketByOrderResponseBase
{
    private int flags;
    private int serviceId;

    private UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();

    /**
     * Instantiates a new market by order update.
     */
    public MarketByOrderUpdate()
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

    /* (non-Javadoc)
     * @see com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase#encodeMsg()
     */
    @Override
    public Msg encodeMsg()
    {
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(streamId());
        updateMsg.domainType(DomainTypes.MARKET_BY_ORDER);
        updateMsg.containerType(DataTypes.MAP);

        if (checkHasServiceId())
        {
            updateMsg.applyHasMsgKey();
            //service Id
            updateMsg.msgKey().applyHasServiceId();
            updateMsg.msgKey().serviceId(serviceId);

            //name
            updateMsg.msgKey().applyHasName();
            updateMsg.msgKey().name(itemName());

            //name type
            updateMsg.msgKey().applyHasNameType();
            updateMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        }

        return updateMsg;
    }
    
    /* (non-Javadoc)
     * @see com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase#encodeMapEntries(com.refinitiv.eta.codec.EncodeIterator, com.refinitiv.eta.codec.DataDictionary)
     */
    @Override
    protected int encodeMapEntries(EncodeIterator encodeIter, DataDictionary dictionary)
    {
        DictionaryEntry dictionaryEntry;
        int ret = CodecReturnCodes.SUCCESS;

        // encode the order book
        mapEntry.clear();
        
        int idx = -1;
        for (MarketByOrderItem.OrderInfo orderInfo : marketByOrderItem().orderInfoList)
        {
            ++idx;
            
            if (orderInfo.lifetime != 0)
            {
                // determine if this is an ADD or UPDATE. This logic matches the
                // logic used in MarketByOrderItem.
                if (orderInfo.lifetime == (5 + idx))
                    mapEntry.action(MapEntryActions.ADD);
                else
                    mapEntry.action(MapEntryActions.UPDATE);
                
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

                if ((ret = fieldList.encodeInit(encodeIter, marketByOrderSetDefDb, 0)) < CodecReturnCodes.SUCCESS)
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
                    if ((ret = fieldEntry.encode(encodeIter, orderInfo.ORDER_PRC)) < CodecReturnCodes.SUCCESS)
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
                    if ((ret = fieldEntry.encode(encodeIter, orderInfo.ORDER_SIZE)) < CodecReturnCodes.SUCCESS)
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
                        if ((ret = fieldEntry.encode(encodeIter, orderInfo.MKOASK_VOL)) < CodecReturnCodes.SUCCESS)
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
                        if ((ret = fieldEntry.encode(encodeIter, orderInfo.MKOBID_VOL)) < CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                    }
                }

                
                /*
                 * Since the field list specified FieldListFlags.HAS_STANDARD_DATA as
                 * well, standard entries can be encoded after the set is finished.
                 * The Add actions will additionally include the ORDER_SIDE and
                 * MKT_MKR_ID fields.
                 */
                
                if (orderInfo.lifetime == (5 + idx))
                {
                    //ORDER_SIDE
                    fieldEntry.clear();
                    dictionaryEntry = dictionary.entry(MarketByOrderItem.ORDER_SIDE_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.fieldId(MarketByOrderItem.ORDER_SIDE_FID);
                        fieldEntry.dataType(dictionaryEntry.rwfType());
                        if ((ret = fieldEntry.encode(encodeIter, orderInfo.ORDER_SIDE)) < CodecReturnCodes.SUCCESS)
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
                        if ((ret = fieldEntry.encode(encodeIter, tmpBuffer)) < CodecReturnCodes.SUCCESS)
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
                tmpBuffer.data(orderInfo.ORDER_ID);
                if ((ret = mapEntry.encode(encodeIter, tmpBuffer)) != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
        }

        return ret;
    }

    /* (non-Javadoc)
     * @see com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl#toString()
     */
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "MarketByOrderUpdate: \n");
        
        if (checkHasServiceId())
        {
            stringBuf.append(tabChar);
            stringBuf.append("serviceId: ");
            stringBuf.append(serviceId());
            stringBuf.append(eolChar);
        }
        
        return stringBuf.toString();
        
        
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
