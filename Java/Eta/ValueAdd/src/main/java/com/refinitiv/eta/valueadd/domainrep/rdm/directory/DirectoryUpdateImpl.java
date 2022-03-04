/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.MapEntryFlags;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class DirectoryUpdateImpl extends MsgBaseImpl
{
    private List<Service> serviceList = new ArrayList<Service>();
    private long sequenceNumber;
    private int serviceId;
    private long filter;
    private int flags;

    private final static String eol = "\n";
    private final static String tab = "\t";

    private Map map = CodecFactory.createMap();
    private MapEntry mEntry = CodecFactory.createMapEntry();
    private UInt tmpUInt = CodecFactory.createUInt();
    private UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
    
    DirectoryUpdateImpl()
    {
    }

    public void clear()
    {
        super.clear();
        serviceList.clear();
        sequenceNumber = 0;
        serviceId = 0;
        filter = 0;
        flags = 0;
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }

    private Service service(int serviceId)
    {
        for (Service service : serviceList)
        {
            if (service.serviceId() == serviceId)
                return service;
        }
        return null;
    }

    public List<Service> serviceList()
    {
        return serviceList;
    }

    public void serviceList(List<Service> serviceList)
    {
        assert (serviceList != null) : "serviceList must be non-null";

        serviceList().clear();
       
        for (Service service : serviceList)
        {
            serviceList().add(service);
        }
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.UPDATE)
        {
            return CodecReturnCodes.FAILURE;
        }
        streamId(msg.streamId());

        MsgKey msgKey = msg.msgKey();
        if (msgKey != null)
        {
            if (msgKey.checkHasFilter())
            {
                applyHasFilter();
                filter(msg.msgKey().filter());
            }
            if (msgKey.checkHasServiceId())
            {
                applyHasServiceId();
                serviceId(msgKey.serviceId());
            }
        }
        UpdateMsg updateMsg = (UpdateMsg)msg;
        if (updateMsg.checkHasSeqNum())
        {
            applyHasSequenceNumber();
            sequenceNumber(updateMsg.seqNum());
        }

        if (msg.containerType() != DataTypes.MAP)
        {
            return CodecReturnCodes.FAILURE;
        }

        return decodeServiceList(dIter);
    }

    private int encodeServiceList(EncodeIterator encIter)
    {
        map.clear();
        map.flags(MapEntryFlags.NONE);
        map.keyPrimitiveType(DataTypes.UINT);
        map.containerType(DataTypes.FILTER_LIST);
        int ret = map.encodeInit(encIter, 0, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        for (Service service : serviceList())
        {
            mEntry.clear();
            mEntry.flags(MapEntryFlags.NONE);
            mEntry.action(service.action());
            tmpUInt.value(service.serviceId());
            ret = mEntry.encodeInit(encIter, tmpUInt, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            if (mEntry.action() != MapEntryActions.DELETE)
            {
                ret = service.encode(encIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = mEntry.encodeComplete(encIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

        }
        return map.encodeComplete(encIter, true);
    }

    private int decodeServiceList(DecodeIterator dIter)
    {
        int ret = 0;
        if ((ret = map.decode(dIter)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (map.containerType() != DataTypes.FILTER_LIST || map.keyPrimitiveType() != DataTypes.UINT)
        {
            return CodecReturnCodes.FAILURE;
        }

        Service service = null;
        while ((ret = mEntry.decode(dIter, tmpUInt)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
            {
                return ret;
            }

            service = service((int)tmpUInt.toLong());
            if (service == null)
            {
                service = new ServiceImpl();
                service.serviceId((int)tmpUInt.toLong());
                serviceList.add(service);
            }
            if (mEntry.action() != MapEntryActions.DELETE)
            {
                ret = service.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            service.action(mEntry.action());

        }

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int encode(EncodeIterator encodeIter)
    {
        updateMsg.clear();
        updateMsg.streamId(streamId());
        updateMsg.containerType(DataTypes.MAP);
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.domainType(DomainTypes.SOURCE);
        updateMsg.applyDoNotConflate();
        if (checkHasFilter())
        {
            updateMsg.applyHasMsgKey();
            updateMsg.msgKey().applyHasFilter();
            updateMsg.msgKey().filter(filter);
        }

        if (checkHasServiceId())
        {
            updateMsg.applyHasMsgKey();
            updateMsg.msgKey().applyHasServiceId();
            updateMsg.msgKey().serviceId(serviceId);
        }

        if (checkHasSequenceNumber())
        {
            updateMsg.applyHasSeqNum();
            updateMsg.seqNum(sequenceNumber());
        }

        int ret = updateMsg.encodeInit(encodeIter, 0);
        if (ret != CodecReturnCodes.ENCODE_CONTAINER)
            return ret;
        ret = encodeServiceList(encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = updateMsg.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        return CodecReturnCodes.SUCCESS;
    }

    public int copy(DirectoryUpdate destUpdateMsg)
    {
        assert (destUpdateMsg != null) : "destUpdateMsg must be non-null";
        
        destUpdateMsg.streamId(streamId());

        if (checkHasFilter())
        {
            destUpdateMsg.applyHasFilter();
            destUpdateMsg.filter(filter());
        }

        if (checkHasServiceId())
        {
            destUpdateMsg.applyHasServiceId();
            destUpdateMsg.serviceId(serviceId());
        }


        if (checkHasSequenceNumber())
        {
            destUpdateMsg.applyHasSequenceNumber();
            destUpdateMsg.sequenceNumber(sequenceNumber());
        }

        int ret = CodecReturnCodes.SUCCESS;
        for(Service rdmService : serviceList())
        {
            ServiceImpl destRDMService = new ServiceImpl();
            ret = rdmService.copy(destRDMService);
            if(ret != CodecReturnCodes.SUCCESS)
                return ret;
            destUpdateMsg.serviceList().add(destRDMService);
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    public long filter()
    {
        return filter;
    }

    public void filter(long filter)
    {
        assert(checkHasFilter());
        this.filter = filter;
    }

    public boolean checkHasFilter()
    {
        return (flags & DirectoryUpdateFlags.HAS_FILTER) != 0;
    }

    public void applyHasFilter()
    {
        flags |= DirectoryUpdateFlags.HAS_FILTER;
    }

    public long sequenceNumber()
    {
        return sequenceNumber;
    }

    public void sequenceNumber(long seqNum)
    {
        assert(checkHasSequenceNumber());
        this.sequenceNumber = seqNum;
    }

    public boolean checkHasSequenceNumber()
    {
        return (flags & DirectoryUpdateFlags.HAS_SEQ_NUM) != 0;
    }

    public void applyHasSequenceNumber()
    {
        flags |= DirectoryUpdateFlags.HAS_SEQ_NUM;
    }

    public int serviceId()
    {
        return serviceId;
    }

    public void serviceId(int serviceId)
    {
        assert(checkHasServiceId());
        this.serviceId = serviceId;
    }

    public boolean checkHasServiceId()
    {
        return (flags & DirectoryUpdateFlags.HAS_SERVICE_ID) != 0;
    }

    public void applyHasServiceId()
    {
        flags |= DirectoryUpdateFlags.HAS_SERVICE_ID;
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DirectoryUpdate: \n");

        if (checkHasServiceId())
        {
            stringBuf.append(tab);
            stringBuf.append("serviceId: ");
            stringBuf.append(serviceId());
            stringBuf.append(eol);
        }

        if (checkHasSequenceNumber())
        {
            stringBuf.append(tab);
            stringBuf.append("sequenceNumber: ");
            stringBuf.append(sequenceNumber());
            stringBuf.append(eol);
        }

        stringBuf.append(tab);
        stringBuf.append("filter: ");
        boolean addOr = false;
        long filter = filter();
        if ((filter & Directory.ServiceFilterFlags.INFO) != 0)
        {
            stringBuf.append("INFO");
            addOr = true;
        }
        if ((filter & Directory.ServiceFilterFlags.DATA) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("DATA");
            addOr = true;
        }
        if ((filter & Directory.ServiceFilterFlags.GROUP) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("GROUP");
            addOr = true;
        }
        if ((filter & Directory.ServiceFilterFlags.LINK) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("LINK");
            addOr = true;
        }
        if ((filter & Directory.ServiceFilterFlags.LOAD) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("LOAD");
            addOr = true;
        }
        if ((filter & Directory.ServiceFilterFlags.STATE) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("STATE");
            addOr = true;
        }
        stringBuf.append(eol);

        for(Service service : serviceList)
        {
            stringBuf.append(service.toString());
        }

        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.SOURCE;
    }
 }