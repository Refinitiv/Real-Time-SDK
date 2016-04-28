package com.thomsonreuters.upa.examples.rdm.symbollist;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * The request message for symbol list domain.
 */
public class SymbolListRequest extends MsgBaseImpl
{
    private Qos qos;
    private int priorityClass;
    private int priorityCount;
    private Buffer symbolListName;
    private int serviceId;
    private int flags = 0;

    private RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
    private final static String eolChar = "\n";
    private final static String tabChar = "\t";
    
    public SymbolListRequest()
    {
        qos = CodecFactory.createQos();
        symbolListName = CodecFactory.createBuffer();
    }

    public Buffer symbolListName()
    {
        return symbolListName;
    }

    public int serviceId()
    {
        return serviceId;
    }

    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        qos.clear();
        priorityClass = 0;
        priorityCount = 0;
        flags = 0;
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
     * @return priority class used by request
     */
    public int priorityClass()
    {
        return priorityClass;
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
     * 
     * @return priority count used by request
     */
    public int priorityCount()
    {
        return priorityCount;
    }

    /**
     * Checks the presence of streaming.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkStreaming()
    {
        return (flags & SymbolListRequestFlags.STREAMING) != 0;
    }

    /**
     * Applies streaming flag.
     */
    public void applyStreaming()
    {
        flags |= SymbolListRequestFlags.STREAMING;
    }

    /**
     * Encodes the symbol list request.
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
        requestMsg.clear();

        /* set-up message */
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(streamId());
        requestMsg.domainType(DomainTypes.SYMBOL_LIST);
        requestMsg.containerType(DataTypes.NO_DATA);
        if (checkStreaming())
        {
            requestMsg.applyStreaming();
        }

        if (checkHasPriority())
        {
            requestMsg.applyHasPriority();
            requestMsg.priority().priorityClass(priorityClass());
            requestMsg.priority().count(priorityCount());
        }

        if (checkHasQos())
        {
            requestMsg.applyHasQos();
            requestMsg.qos().dynamic(qos().isDynamic());
            requestMsg.qos().rate(qos().rate());
            requestMsg.qos().rateInfo(qos().rateInfo());
            requestMsg.qos().timeliness(qos().timeliness());
            requestMsg.qos().timeInfo(qos().timeInfo());
        }

        /* set msgKey members */
        requestMsg.msgKey().applyHasNameType();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        requestMsg.msgKey().name().data(symbolListName.data(), symbolListName.position(), symbolListName.length());
        requestMsg.msgKey().serviceId(serviceId());

      
        return requestMsg.encode(encodeIter);
    }

    /**
     * Checks the presence of Qos.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasQos()
    {
        return (flags & SymbolListRequestFlags.HAS_QOS) != 0;
    }

    /**
     * Applies Qos flag.
     */
    public void applyHasQos()
    {
        flags |= SymbolListRequestFlags.HAS_QOS;
    }
    
    /**
     * Checks the presence of Priority flag.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPriority()
    {
        return (flags & SymbolListRequestFlags.HAS_PRIORITY) != 0;
    }

    /**
     * Applies Priority flag.
     */
    public void applyHasPriority()
    {
        flags |= SymbolListRequestFlags.HAS_PRIORITY;
    }
 
    /**
     * Applies service id flag.
     */
    public void applyHasServiceId()
    {
        flags |= SymbolListRequestFlags.HAS_SERVICE_ID;
    }
    
    /**
     * Checks the presence of service id flag.
     */
    public boolean checkHasServiceId()
    {
        return (flags & SymbolListRequestFlags.HAS_SERVICE_ID) != 0;
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "SymbolListRequest: \n");
        stringBuf.append(tabChar);
        stringBuf.append("streaming: ");
        stringBuf.append(checkStreaming());
        stringBuf.append(eolChar);

        stringBuf.append(tabChar);
        stringBuf.append("symbolListName: ");
        stringBuf.append(symbolListName());
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
    
    @Override
    public int domainType()
    {
        return DomainTypes.SYMBOL_LIST;
    }
}