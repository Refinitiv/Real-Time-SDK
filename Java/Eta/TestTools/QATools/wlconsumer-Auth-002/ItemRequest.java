/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRequestFlags;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.SymbolList;
import com.refinitiv.eta.rdm.ViewTypes;

public class ItemRequest
{
    private List<String> itemNames;
    private int streamId;
    private Qos qos;
    private Qos worstQos;
    private int priorityClass;
    private int priorityCount;
    private int serviceId;
    private int identifier;
    private List<Integer> viewFieldList;
    private List<String> viewElementNameList;
    private int domainType;
    private int flags;
    private boolean isSymbolListData;

    public static final Buffer VIEW_TYPE = CodecFactory.createBuffer();
    /** :ViewData */
    public static final Buffer VIEW_DATA = CodecFactory.createBuffer();

    RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
    private ElementList elementList = CodecFactory.createElementList();
    private ElementList behaviorList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private ElementEntry dataStreamEntry = CodecFactory.createElementEntry();
    private Array array = CodecFactory.createArray();
    private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
    private Buffer itemNameBuf = CodecFactory.createBuffer();

    private Int tempInt = CodecFactory.createInt();
    private UInt tempUInt = CodecFactory.createUInt();
    private Buffer elementNameBuf = CodecFactory.createBuffer();
    private Array viewArray = CodecFactory.createArray();

    public ItemRequest()
    {
        this(DomainTypes.MARKET_PRICE);
        this.isSymbolListData = false;
    }

    public ItemRequest(int domainType)
    {
        qos = CodecFactory.createQos();
        worstQos = CodecFactory.createQos();
        itemNames = new ArrayList<String>();

        priorityClass = 1;
        priorityCount = 1;
        flags = 0;
        identifier = -1;
        this.domainType = domainType;
    }

    public int streamId()
    {
        return streamId;
    }

    public void streamId(int streamId)
    {
        this.streamId = streamId;
    }

    public void symbolListData(boolean isSymbolListData)
    {
        this.isSymbolListData = isSymbolListData;
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        flags = 0;
        qos.clear();
        worstQos.clear();
        itemNames.clear();
        priorityClass = 1;
        priorityCount = 1;
        identifier = -1;
        isSymbolListData = false;
        if (viewFieldList != null)
            viewFieldList.clear();
        if (viewElementNameList != null)
            viewElementNameList.clear();
    }

    /**
     * Checks the presence of private stream flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream()
    {
        return (flags & MarketPriceRequestFlags.PRIVATE_STREAM) != 0;
    }

    /**
     * Applies private stream flag.
     */
    public void applyPrivateStream()
    {
        flags |= MarketPriceRequestFlags.PRIVATE_STREAM;
    }

    /**
     * 
     * @return service id
     */
    public int serviceId()
    {
        return serviceId;
    }

    /**
     * @param serviceId
     */
    public ItemRequest serviceId(int serviceId)
    {
        this.serviceId = serviceId;
        return this;
    }

    /**
     * 
     * @return list of item names
     */
    public List<String> itemNames()
    {
        return itemNames;
    }

    /**
     * 
     * @param itemNames list of item names
     */
    public void itemNames(List<String> itemNames)
    {
        this.itemNames = itemNames;
    }

    /**
     * 
     * @param itemName item name
     */
    public void addItem(String itemName)
    {
        itemNames.add(itemName);
    }

    /**
     * 
     * @return priority class used by request
     */
    public int priorityClass()
    {
        return priorityClass;
    }

    /**
     * 
     * @return priority count used by request
     */
    public int priorityCount()
    {
        return priorityCount;
    }

    /**
     * 
     * @param priorityClass
     * @param priorityCount
     * 
     */
    public void priority(int priorityClass, int priorityCount)
    {
        this.priorityClass = priorityClass;
        this.priorityCount = priorityCount;
    }

    /**
     * Checks the presence of streaming.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkStreaming()
    {
        return (flags & MarketPriceRequestFlags.STREAMING) != 0;
    }

    /**
     * Applies streaming flag.
     */
    public void applyStreaming()
    {
        flags |= MarketPriceRequestFlags.STREAMING;
    }

    /**
     * Checks the presence of Qos.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasQos()
    {
        return (flags & MarketPriceRequestFlags.HAS_QOS) != 0;
    }

    /**
     * Applies Qos flag.
     */
    public void applyHasQos()
    {
        flags |= MarketPriceRequestFlags.HAS_QOS;
    }

    /**
     * Checks the presence of WorstQos.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasWorstQos()
    {
        return (flags & MarketPriceRequestFlags.HAS_WORST_QOS) != 0;
    }

    /**
     * Applies WorstQos flag.
     */
    public void applyHasWorstQos()
    {
        flags |= MarketPriceRequestFlags.HAS_WORST_QOS;
    }

    /**
     * Checks the presence of Priority flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPriority()
    {
        return (flags & MarketPriceRequestFlags.HAS_PRIORITY) != 0;
    }

    /**
     * Applies Priority flag.
     */
    public void applyHasPriority()
    {
        flags |= MarketPriceRequestFlags.HAS_PRIORITY;
    }

    /**
     * Applies View flag.
     */
    public void applyHasView()
    {
        flags |= MarketPriceRequestFlags.HAS_VIEW;
    }

    /**
     * Applies Pause flag.
     */
    public void applyPause()
    {
        flags |= RequestMsgFlags.PAUSE;
    }

    // APIQA: apply/check NoRefresh flag methods
    public void applyNoRefresh()
    {
        flags |= 0x2000;
    }

    public boolean checkHasNoRefresh()
    {
        return (flags & 0x2000) != 0;
    }

    // END APIQA

    /**
     * Checks the presence of Pause.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPause()
    {
        return (flags & RequestMsgFlags.PAUSE) != 0;
    }

    /**
     * Checks the presence of View flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasView()
    {
        return (flags & MarketPriceRequestFlags.HAS_VIEW) != 0;
    }

    /**
     * 
     * @return Qos used by request
     */
    public Qos qos()
    {
        return qos;
    }

    /**
     * 
     * @return WorstQos used by request
     */
    public Qos worstQos()
    {
        return worstQos;
    }

    /**
     * 
     * @return list of view fields
     */
    public List<Integer> viewFields()
    {
        return viewFieldList;
    }

    /**
     * 
     * @return list of view elementNames
     */
    public List<String> viewElementNames()
    {
        return viewElementNameList;
    }

    /**
     * 
     * @param viewList list of view fields
     */
    public void viewFields(List<Integer> viewList)
    {
        viewFieldList = viewList;
    }

    /**
     * 
     * @param elementNameList list of element names
     */
    public void viewElementNames(List<String> elementNameList)
    {
        viewElementNameList = elementNameList;
    }

    /**
     * Checks the presence of service id flag.
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId()
    {
        return (flags & MarketPriceRequestFlags.HAS_SERVICE_ID) != 0;
    }

    /**
     * Applies service id flag.
     */
    public void applyHasServiceId()
    {
        flags |= MarketPriceRequestFlags.HAS_SERVICE_ID;
    }

    /**
     * 
     * @return Domain type
     */
    public int domainType()
    {
        return domainType;
    }

    /**
     * 
     * @param domainType Domain type
     */
    public void domainType(int domainType)
    {
        this.domainType = domainType;
    }

    /**
     * Set the identifier for the msg Key
     */

    public void identifier(int setIdentifier)
    {
        identifier = setIdentifier;
    }

    /** 
     * @return Identifier
     */
    public int identifier()
    {
        return identifier;
    }

    /**
     * Checks the presence of an identifier
     * @return true - if exists; false if does not exist;
     */
    public boolean checkHasIdentifier()
    {
        if (identifier >= 0)
            return true;
        else
            return false;
    }

    public void encode()
    {
        requestMsg.clear();

        /* set-up message */
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(streamId());
        requestMsg.domainType(domainType);

        requestMsg.containerType(DataTypes.NO_DATA);

        if (checkHasQos())
        {
            requestMsg.applyHasQos();
            requestMsg.qos().dynamic(qos.isDynamic());
            requestMsg.qos().rate(qos.rate());
            requestMsg.qos().timeliness(qos.timeliness());
            requestMsg.qos().rateInfo(qos.rateInfo());
            requestMsg.qos().timeInfo(qos.timeInfo());
        }

        if (checkHasWorstQos())
        {
            requestMsg.applyHasWorstQos();
            requestMsg.worstQos().dynamic(worstQos.isDynamic());
            requestMsg.worstQos().rate(worstQos.rate());
            requestMsg.worstQos().timeliness(worstQos.timeliness());
            requestMsg.worstQos().rateInfo(worstQos.rateInfo());
            requestMsg.worstQos().timeInfo(worstQos.timeInfo());
        }

        if (checkHasPriority())
        {
            requestMsg.applyHasPriority();
            requestMsg.priority().priorityClass(priorityClass());
            requestMsg.priority().count(priorityCount());
        }

        if (checkStreaming())
        {
            requestMsg.applyStreaming();
        }

        if (checkHasPause())
        {
            requestMsg.applyPause();
        }

        // APIQA: apply NoRefresh flag on requestMsg if set
        if (checkHasNoRefresh())
        {
            requestMsg.applyNoRefresh();
        }
        // END APIQA

        boolean isBatchRequest = itemNames.size() > 1;
        applyFeatureFlags(isBatchRequest);

        /* specify msgKey members */
        requestMsg.msgKey().applyHasNameType();
        requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);

        /* If user set Identifier */
        if (checkHasIdentifier())
        {
            requestMsg.msgKey().applyHasIdentifier();
            requestMsg.msgKey().identifier(identifier);
        }

        if (!isBatchRequest && !checkHasIdentifier())
        {
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data(itemNames.get(0));
        }
    }

    /**
     * Encodes the item request.
     * 
     * @param encodeIter The Encode Iterator
     * @return {@link CodecReturnCodes#SUCCESS} if encoding succeeds or failure
     *         if encoding fails.
     * 
     *         This method is only used within the Market By Price Handler and
     *         each handler has its own implementation, although much is similar
     */
    public int encode(EncodeIterator encodeIter)
    {
        boolean isBatchRequest = itemNames.size() > 1;
        int ret = CodecReturnCodes.SUCCESS;

        encode();

        /* encode request message payload */
        if (checkHasView() || isSymbolListData || isBatchRequest)
        {
            ret = encodeRequestPayload(isBatchRequest, encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int encodeRequestPayload(boolean isBatchRequest, EncodeIterator encodeIter)
    {
        elementList.clear();
        elementList.applyHasStandardData();

        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (isBatchRequest && (encodeBatchRequest(encodeIter) < CodecReturnCodes.SUCCESS))
        {
            return CodecReturnCodes.FAILURE;
        }

        if (checkHasView())
        {
            if (viewFieldList != null && viewFieldList.size() > 0)
            {
                if (encodeViewRequest(encodeIter, ViewTypes.FIELD_ID_LIST) < CodecReturnCodes.SUCCESS)
                {
                    return CodecReturnCodes.FAILURE;
                }
            }
            else if (viewElementNameList != null && viewElementNameList.size() > 0)
            {
                if (encodeViewRequest(encodeIter, ViewTypes.ELEMENT_NAME_LIST) < CodecReturnCodes.SUCCESS)
                {
                    return CodecReturnCodes.FAILURE;
                }
            }
        }

        if (isSymbolListData && (encodeSymbolListData(encodeIter) < CodecReturnCodes.SUCCESS))
        {
            return CodecReturnCodes.FAILURE;
        }

        ret = elementList.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        return CodecReturnCodes.SUCCESS;
    }

    private void applyFeatureFlags(boolean isBatchRequest)
    {
        if (checkPrivateStream())
        {
            requestMsg.applyPrivateStream();
        }

        if (checkHasView() || isSymbolListData || isBatchRequest)
        {
            requestMsg.containerType(DataTypes.ELEMENT_LIST);
            if (checkHasView())
            {
                requestMsg.applyHasView();
            }
            if (isBatchRequest)
            {
                requestMsg.applyHasBatch();
            }
        }
    }

    private int encodeBatchRequest(EncodeIterator encodeIter)
    {
        /*
         * For Batch requests, the message has a payload of an element list that
         * contains an array of the requested items
         */

        elementEntry.clear();
        array.clear();
        itemNameBuf.clear();

        elementEntry.name(ElementNames.BATCH_ITEM_LIST);
        elementEntry.dataType(DataTypes.ARRAY);
        int ret = elementEntry.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        /* Encode the array of requested item names */
        array.primitiveType(DataTypes.ASCII_STRING);
        array.itemLength(0);

        ret = array.encodeInit(encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        for (String itemName : itemNames)
        {
            arrayEntry.clear();
            itemNameBuf.data(itemName);
            ret = arrayEntry.encode(encodeIter, itemNameBuf);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        ret = array.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = elementEntry.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Encodes the View Element Entry. This entry contains an array of FIDs that
     * the consumer wishes to receive from the provider.
     * 
     * This method is only used within the Market Price Handler
     */
    private int encodeViewRequest(EncodeIterator encodeIter, int viewType)
    {
        elementEntry.clear();
        tempUInt.clear();
        elementEntry.name(ElementNames.VIEW_TYPE);
        elementEntry.dataType(DataTypes.UINT);
        if (viewType == ViewTypes.FIELD_ID_LIST)
            tempUInt.value(ViewTypes.FIELD_ID_LIST);
        else
            tempUInt.value(ViewTypes.ELEMENT_NAME_LIST);
        int ret = elementEntry.encode(encodeIter, tempUInt);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        elementEntry.clear();
        elementEntry.name(ElementNames.VIEW_DATA);
        elementEntry.dataType(DataTypes.ARRAY);
        if ((ret = elementEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (viewType == ViewTypes.FIELD_ID_LIST)
        {
            viewArray.primitiveType(DataTypes.INT);
            viewArray.itemLength(2);
        }
        else
            viewArray.primitiveType(DataTypes.ASCII_STRING);

        if ((ret = viewArray.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (viewType == ViewTypes.FIELD_ID_LIST)
        {
            for (Integer viewField : viewFieldList)
            {
                arrayEntry.clear();
                tempInt.value(viewField);
                ret = arrayEntry.encode(encodeIter, tempInt);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
        }
        else
        {
            for (String elementName : viewElementNameList)
            {
                arrayEntry.clear();
                elementNameBuf.data(elementName);
                ret = arrayEntry.encode(encodeIter, elementNameBuf);

                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
        }
        ret = viewArray.encodeComplete(encodeIter, true);

        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = elementEntry.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    public int encodeSymbolListData(EncodeIterator encodeIter)
    {
        int ret = CodecReturnCodes.SUCCESS;
        elementEntry.clear();
        elementEntry.name(SymbolList.ElementNames.SYMBOL_LIST_BEHAVIORS);
        elementEntry.dataType(DataTypes.ELEMENT_LIST);

        ret = elementEntry.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        behaviorList.clear();
        behaviorList.applyHasStandardData();
        ret = behaviorList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        dataStreamEntry.clear();
        dataStreamEntry.name(SymbolList.ElementNames.SYMBOL_LIST_DATA_STREAMS);
        dataStreamEntry.dataType(DataTypes.UINT);

        if (checkStreaming())
            tempUInt.value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
        else
            tempUInt.value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS);
        if ((ret = dataStreamEntry.encode(encodeIter, tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = behaviorList.encodeComplete(encodeIter, true);

        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = elementEntry.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

}
