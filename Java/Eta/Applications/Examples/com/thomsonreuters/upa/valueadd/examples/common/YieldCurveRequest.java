package com.thomsonreuters.upa.valueadd.examples.common;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * The request message for yield curve domain.
 */
public class YieldCurveRequest extends MsgBaseImpl
{
    private final List<String> itemNames;
    private final Qos qos;
    private int priorityClass;
    private int priorityCount;
    private int serviceId;
    private final List<String> viewFieldList;
    private final int domainType;
    private int flags;

    private final static String eolChar = "\n";
    private final static String tabChar = "\t";

    private RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private Array array = CodecFactory.createArray();
    private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
    private Buffer itemNameBuf = CodecFactory.createBuffer();

    public YieldCurveRequest()
    {
        this(DomainTypes.YIELD_CURVE);
    }

    public YieldCurveRequest(int domainType)
    {
        qos = CodecFactory.createQos();
        itemNames = new ArrayList<String>();
        viewFieldList = new ArrayList<String>();
        priorityClass = 1;
        priorityCount = 1;
        flags = 0;
        this.domainType = domainType;
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        flags = 0;
        qos.clear();
        itemNames.clear();
        priorityClass = 1;
        priorityCount = 1;
    }

    /**
     * Checks the presence of private stream flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream()
    {
        return (flags & YieldCurveRequestFlags.PRIVATE_STREAM) != 0;
    }

    /**
     * Applies private stream flag.
     */
    public void applyPrivateStream()
    {
        flags |= YieldCurveRequestFlags.PRIVATE_STREAM;
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
    public YieldCurveRequest serviceId(int serviceId)
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
        return (flags & YieldCurveRequestFlags.STREAMING) != 0;
    }

    /**
     * Applies streaming flag.
     */
    public void applyStreaming()
    {
        flags |= YieldCurveRequestFlags.STREAMING;
    }

    /**
     * Checks the presence of Qos.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasQos()
    {
        return (flags & YieldCurveRequestFlags.HAS_QOS) != 0;
    }

    /**
     * Applies Qos flag.
     */
    public void applyHasQos()
    {
        flags |= YieldCurveRequestFlags.HAS_QOS;
    }

    /**
     * Checks the presence of Priority flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPriority()
    {
        return (flags & YieldCurveRequestFlags.HAS_PRIORITY) != 0;
    }

    /**
     * Applies Priority flag.
     */
    public void applyHasPriority()
    {
        flags |= YieldCurveRequestFlags.HAS_PRIORITY;
    }

    /**
     * Applies View flag.
     */
    public void applyHasView()
    {
        flags |= YieldCurveRequestFlags.HAS_VIEW;
    }

    /**
     * Checks the presence of View flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasView()
    {
        return (flags & YieldCurveRequestFlags.HAS_VIEW) != 0;
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
     * @return list of view fields
     */
    public List<String> viewFields()
    {
        return viewFieldList;
    }

    /**
     * Checks the presence of service id flag.
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId()
    {
        return (flags & YieldCurveRequestFlags.HAS_SERVICE_ID) != 0;
    }

    /**
     * Applies service id flag.
     */
    public void applyHasServiceId()
    {
        flags |= YieldCurveRequestFlags.HAS_SERVICE_ID;
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
     * Encodes the item request.
     * 
     * @param encodeIter The Encode Iterator
     * @return {@link CodecReturnCodes#SUCCESS} if encoding succeeds or failure
     *         if encoding fails.
     * 
     *         This method is only used within the Yield Curve Handler and
     *         each handler has its own implementation, although much is similar
     */
    public int encode(EncodeIterator encodeIter)
    {
        requestMsg.clear();
        elementList.clear();
        elementEntry.clear();
        array.clear();
        itemNameBuf.clear();

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

        boolean isBatchRequest = itemNames.size() > 1;
        applyFeatureFlags(isBatchRequest);

        /* specify msgKey members */
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().serviceId(serviceId());

        requestMsg.msgKey().applyHasNameType();
        requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);

        if (!isBatchRequest)
        {
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data(itemNames.get(0));
        }

        int ret = requestMsg.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        /* encode request message payload */
        if (checkHasView() || isBatchRequest)
        {
            ret = encodeRequestPayload(isBatchRequest, encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }
        ret = requestMsg.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int encodeRequestPayload(boolean isBatchRequest, EncodeIterator encodeIter)
    {
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        elementList.applyHasStandardData();

        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (isBatchRequest
                && (encodeBatchRequest(encodeIter) < CodecReturnCodes.SUCCESS))
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

        if (checkHasView() || isBatchRequest)
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

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "YieldCurveRequest: \n");
        stringBuf.append(tabChar);
        stringBuf.append("streaming: ");
        stringBuf.append(checkStreaming());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("domain: ");
        stringBuf.append(DomainTypes.toString(domainType));
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("isPrivateStream: ");
        stringBuf.append(checkPrivateStream());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("hasView: ");
        stringBuf.append(checkHasView());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("itemNames: ");
        stringBuf.append(itemNames());
        stringBuf.append(eolChar);
        if (checkHasServiceId())
        {
            stringBuf.append(tabChar);
            stringBuf.append("serviceId: ");
            stringBuf.append(serviceId());
            stringBuf.append(eolChar);
        }
        if (checkHasPriority())
        {
            stringBuf.append(tabChar);
            stringBuf.append("priority class: ");
            stringBuf.append(priorityClass());
            stringBuf.append(", priority count: ");
            stringBuf.append(priorityCount());
            stringBuf.append(eolChar);
        }
        if (checkHasQos())
        {
            stringBuf.append(tabChar);
            stringBuf.append("qos: ");
            stringBuf.append(qos.toString());
            stringBuf.append(eolChar);
        }

        return stringBuf.toString();
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        throw new UnsupportedOperationException();
    }
}
