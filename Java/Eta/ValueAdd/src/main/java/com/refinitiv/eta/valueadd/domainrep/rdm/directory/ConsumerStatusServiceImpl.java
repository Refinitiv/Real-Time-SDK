package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.ElementNames;


class ConsumerStatusServiceImpl implements ConsumerStatusService
{
    private long serviceId;
    private int action;
    private long sourceMirroringMode;

    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private UInt tmpUInt = CodecFactory.createUInt();
    private StringBuilder stringBuffer = new StringBuilder();

    
    ConsumerStatusServiceImpl()
    {
    }

    public void clear()
    {
        action = MapEntryActions.ADD;
        serviceId = 0;
        sourceMirroringMode = Directory.SourceMirroringMode.ACTIVE_NO_STANDBY;
    }

    public int copy(ConsumerStatusService destConsumerStatusService)
    {
        assert (destConsumerStatusService != null) : "destConsumerStatusService must be non-null";
        
        destConsumerStatusService.serviceId(serviceId());
        destConsumerStatusService.action(action());
        destConsumerStatusService.sourceMirroringMode(sourceMirroringMode());
        return CodecReturnCodes.SUCCESS;
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        elementList.clear();
        int ret = 0;
        if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
        {
             return ret;
        }
        elementEntry.clear();
        boolean foundSourceMirroringMode = false;
        while ((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (elementEntry.name().equals(ElementNames.SOURCE_MIRROR_MODE))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                {
                    return ret;
                }

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
                {
                    return ret;
                }
                sourceMirroringMode(tmpUInt.toLong());
                foundSourceMirroringMode = true;
            }
        }

        if (!foundSourceMirroringMode)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    public int encode(EncodeIterator encodeIter)
    {
        elementList.clear();
        elementList.applyHasStandardData();
        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        elementEntry.clear();
        elementEntry.name(ElementNames.SOURCE_MIRROR_MODE);
        elementEntry.dataType(DataTypes.UINT);
        tmpUInt.value(sourceMirroringMode);
        ret = elementEntry.encode(encodeIter, tmpUInt);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
    
        ret = elementList.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return ret;
    }

    public long serviceId()
    {
        return serviceId;
    }

    public void serviceId(long serviceId)
    {
        this.serviceId = serviceId;
    }

    public int action()
    {
        return action;
    }

    public void action(int action)
    {
        this.action = action;
    }

    public long sourceMirroringMode()
    {
        return sourceMirroringMode;
    }

    public void sourceMirroringMode(long sourceMirroringMode)
    {
        this.sourceMirroringMode = sourceMirroringMode;
    }

    StringBuilder buildStringBuf()
    {
        stringBuffer.setLength(0);
        stringBuffer.insert(0, "ConsumerStatusService: \n");
        stringBuffer.append(tab);

        stringBuffer.append(tab);
        stringBuffer.append("action: ");
        stringBuffer.append(action());
        stringBuffer.append(eol);
        stringBuffer.append(tab);
        stringBuffer.append("serviceId: ");
        stringBuffer.append(serviceId());
        stringBuffer.append(eol);
        stringBuffer.append(tab);
        stringBuffer.append("sourceMirroringMode: ");
        stringBuffer.append(sourceMirroringMode());
        stringBuffer.append(eol);

        return stringBuffer;
    }

    public String toString()
    {
        return buildStringBuf().toString();
    }

}
