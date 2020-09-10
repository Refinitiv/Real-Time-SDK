package com.rtsdk.eta.valueadd.domainrep.rdm.directory;

import java.util.ArrayList;
import java.util.List;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.GenericMsg;
import com.rtsdk.eta.codec.GenericMsgFlags;
import com.rtsdk.eta.codec.Map;
import com.rtsdk.eta.codec.MapEntry;
import com.rtsdk.eta.codec.MapEntryActions;
import com.rtsdk.eta.codec.MapEntryFlags;
import com.rtsdk.eta.codec.MapFlags;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.MsgKey;
import com.rtsdk.eta.codec.MsgKeyFlags;
import com.rtsdk.eta.codec.UInt;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.rdm.ElementNames;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class DirectoryConsumerStatusImpl extends MsgBaseImpl
{
    private final List<ConsumerStatusService> consumerServiceStatusList;
    private GenericMsg genericMsg;
    private Map map;
    private MapEntry mapEntry;
    private UInt tempUInt;
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";

    DirectoryConsumerStatusImpl()
    {
        consumerServiceStatusList = new ArrayList<ConsumerStatusService>();
        genericMsg = (GenericMsg)CodecFactory.createMsg();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.LOGIN);
        map = CodecFactory.createMap();
        mapEntry = CodecFactory.createMapEntry();
        tempUInt = CodecFactory.createUInt();
    }

    public void clear()
    {
        super.clear();
        consumerServiceStatusList.clear();
    }

    public int copy(DirectoryConsumerStatus destConsumerStatus)
    {
        assert (destConsumerStatus != null) : "destConsumerStatus must be non-null";
        destConsumerStatus.streamId(streamId());

        if (consumerServiceStatusList() != null && !consumerServiceStatusList().isEmpty())
        {
            int ret = CodecReturnCodes.SUCCESS;
            for (ConsumerStatusService consStatusService : consumerServiceStatusList())
            {
                ConsumerStatusService destConsStatusService = new ConsumerStatusServiceImpl();
                ret = consStatusService.copy(destConsStatusService);
                if (ret < CodecReturnCodes.SUCCESS)
                    return ret;
                destConsumerStatus.consumerServiceStatusList().add(destConsStatusService);
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    public int encode(EncodeIterator encodeIter)
    {
        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.SOURCE);
        genericMsg.streamId(streamId());
        genericMsg.containerType(DataTypes.MAP);
        genericMsg.flags(GenericMsgFlags.HAS_MSG_KEY);
        genericMsg.msgKey().flags(MsgKeyFlags.HAS_NAME);
        genericMsg.msgKey().name(ElementNames.CONS_CONN_STATUS);
        int ret = genericMsg.encodeInit(encodeIter, 0);
        if (ret != CodecReturnCodes.ENCODE_CONTAINER)
            return CodecReturnCodes.FAILURE;

        map.clear();
        map.flags(MapFlags.NONE);
        map.keyPrimitiveType(DataTypes.UINT);
        map.containerType(DataTypes.ELEMENT_LIST);
        ret = map.encodeInit(encodeIter, 0, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;
        
        for(ConsumerStatusService serviceStatus : consumerServiceStatusList())
        {
            mapEntry.clear();
            mapEntry.flags(MapEntryFlags.NONE);
            mapEntry.action(serviceStatus.action());
            tempUInt.value(serviceStatus.serviceId());
            if (mapEntry.action() != MapEntryActions.DELETE)
            {
                ret = mapEntry.encodeInit(encodeIter, tempUInt, 0);
                if(ret != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
                
                serviceStatus.encode(encodeIter);
                
                if(ret != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
                
                ret = mapEntry.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
            else
            {
                ret = mapEntry.encode(encodeIter, tempUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
        }
        
        ret = map.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        ret = genericMsg.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        return CodecReturnCodes.SUCCESS;
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        if (msg.msgClass() != MsgClasses.GENERIC || msg.domainType() != DomainTypes.SOURCE)
            return CodecReturnCodes.FAILURE;

        MsgKey key = msg.msgKey();

        if (key == null || !key.checkHasName())
            return CodecReturnCodes.FAILURE;

        if (!key.name().equals(ElementNames.CONS_CONN_STATUS))
        {
            // Unknown generic msg name
            return CodecReturnCodes.FAILURE;
        }

        if (msg.containerType() != DataTypes.MAP)
            return CodecReturnCodes.FAILURE;
        
        clear();
        streamId(msg.streamId());

        map.clear();

        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        if (map.keyPrimitiveType() != DataTypes.UINT)
            return CodecReturnCodes.FAILURE;

        if (map.containerType() != DataTypes.ELEMENT_LIST)
            return CodecReturnCodes.FAILURE;

        mapEntry.clear();
        tempUInt.clear();

        while ((ret = mapEntry.decode(dIter, tempUInt)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
                return CodecReturnCodes.FAILURE;
            
            ConsumerStatusService serviceStatus = new ConsumerStatusServiceImpl();
        
            if (mapEntry.action() != MapEntryActions.DELETE)
            {
                ret = serviceStatus.decode(dIter, msg);
                if (ret != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
            
            serviceStatus.action(mapEntry.action());
            serviceStatus.serviceId(tempUInt.toLong());
           
            consumerServiceStatusList().add(serviceStatus);
        }

        return CodecReturnCodes.SUCCESS;
    }

    public List<ConsumerStatusService> consumerServiceStatusList()
    {
        return consumerServiceStatusList;
    }

    public void consumerServiceStatusList(List<ConsumerStatusService> consumerServiceStatusList)
    {
        assert (consumerServiceStatusList != null) : "consumerServiceStatusList must be non-null";

        consumerServiceStatusList().clear();
       
        for (ConsumerStatusService consStatusService : consumerServiceStatusList)
        {
            consumerServiceStatusList().add(consStatusService);
        }
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DirectoryConsumerStatus: \n");
        stringBuf.append(tab);
        stringBuf.append("streamId: ");
        stringBuf.append(streamId());
        stringBuf.append(eol);

        if (consumerServiceStatusList() != null && !consumerServiceStatusList().isEmpty())
        {
            stringBuf.append(tab);
            stringBuf.append("ConsumerServiceStatusList: ");
            for (ConsumerStatusService consStatusService : consumerServiceStatusList())
            {
                stringBuf.append(((ConsumerStatusServiceImpl)consStatusService).buildStringBuf());
            }
            stringBuf.append(eol);
        }

        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.SOURCE;
    }
}
