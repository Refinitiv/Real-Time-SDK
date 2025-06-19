/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
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
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class DirectoryRefreshImpl extends MsgBaseImpl
{
    private List<Service> serviceList = new ArrayList<Service>();
    private long sequenceNumber;
    private int serviceId;
    private long filter;
    private int flags;
    private State state;

    private final static String eol = "\n";
    private final static String tab = "\t";

    private Map map = CodecFactory.createMap();
    private MapEntry mEntry = CodecFactory.createMapEntry();
    private UInt tmpUInt = CodecFactory.createUInt();
    private RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

    DirectoryRefreshImpl()
    {
        state = CodecFactory.createState();
    }

    public void clear()
    {
        super.clear();
        serviceList.clear();
        sequenceNumber = 0;
        serviceId = 0;
        filter = 0;
        flags = 0;
        state.clear();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
    }

    public State state()
    {
        return state;
    }
    
    public void state(State state)
    {
        state().streamState(state.streamState());
        state().dataState(state.dataState());
        state().code(state.code());
        state().text(state.text());
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
        if (msg.msgClass() != MsgClasses.REFRESH)
        {
            return CodecReturnCodes.FAILURE;
        }
        streamId(msg.streamId());

        MsgKey msgKey = msg.msgKey();
        if (msgKey != null)
        {
            if (msgKey.checkHasFilter())
            {
                filter(msg.msgKey().filter());
            }
            if (msgKey.checkHasServiceId())
            {
                applyHasServiceId();
                serviceId(msgKey.serviceId());
            }
        }
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        if (refreshMsg.checkSolicited())
            applySolicited();
        if (refreshMsg.checkClearCache())
            applyClearCache();

        state().streamState(refreshMsg.state().streamState());
        state().dataState(refreshMsg.state().dataState());
        state().code(refreshMsg.state().code());
        if (refreshMsg.state().text().length() > 0)
        {
            Buffer text = refreshMsg.state().text();
            this.state.text().data(text.data(), text.position(), text.length());
        }
        if (refreshMsg.checkHasSeqNum())
        {
            applyHasSequenceNumber();
            sequenceNumber(refreshMsg.seqNum());
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

    public int copy(DirectoryRefresh destRefreshMsg)
    {
        assert (destRefreshMsg != null) : "destRefreshMsg must be non-null";

        destRefreshMsg.streamId(streamId());

        destRefreshMsg.filter(filter());

        destRefreshMsg.state().streamState(state().streamState());
        destRefreshMsg.state().dataState(state().dataState());
        destRefreshMsg.state().code(state().code());

        if (checkClearCache())
            destRefreshMsg.applyClearCache();
        if (checkSolicited())
            destRefreshMsg.applySolicited();

        if (state().text().length() > 0)
        {
            Buffer stateText = CodecFactory.createBuffer();
            ByteBuffer byteBuffer = ByteBuffer.allocate(state().text().length());
            state().text().copy(byteBuffer);
            stateText.data(byteBuffer);
            destRefreshMsg.state().text(stateText);
        }

        if (checkHasServiceId())
        {
            destRefreshMsg.applyHasServiceId();
            destRefreshMsg.serviceId(serviceId());
        }

        if (checkHasSequenceNumber())
        {
            destRefreshMsg.applyHasSequenceNumber();
            destRefreshMsg.sequenceNumber(sequenceNumber());
        }

        int ret = CodecReturnCodes.SUCCESS;
        for (Service rdmService : serviceList())
        {
            ServiceImpl destRDMService = new ServiceImpl();
            ret = rdmService.copy(destRDMService);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destRefreshMsg.serviceList().add(destRDMService);
        }

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int encode(EncodeIterator encodeIter)
    {
        refreshMsg.clear();
        refreshMsg.streamId(streamId());
        refreshMsg.containerType(DataTypes.MAP);
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SOURCE);
        refreshMsg.applyHasMsgKey();
        refreshMsg.applyRefreshComplete();
        refreshMsg.state().dataState(state().dataState());
        refreshMsg.state().streamState(state().streamState());
        refreshMsg.state().code(state().code());
        refreshMsg.state().text(state().text());

        if (checkClearCache())
            refreshMsg.applyClearCache();
        if (checkSolicited())
            refreshMsg.applySolicited();

        refreshMsg.msgKey().applyHasFilter();
        refreshMsg.msgKey().filter(filter);

        if (checkHasServiceId())
        {
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(serviceId);
        }

        if (checkHasSequenceNumber())
        {
            refreshMsg.applyHasSeqNum();
            refreshMsg.seqNum(sequenceNumber());
        }

        int ret = refreshMsg.encodeInit(encodeIter, 0);
        if (ret != CodecReturnCodes.ENCODE_CONTAINER)
            return ret;
        ret = encodeServiceList(encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = refreshMsg.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        return CodecReturnCodes.SUCCESS;
    }

    public long filter()
    {
        return filter;
    }

    public void filter(long filter)
    {
        this.filter = filter;
    }

    public long sequenceNumber()
    {
        return sequenceNumber;
    }

    public void sequenceNumber(long sequenceNumber)
    {
        assert(checkHasSequenceNumber());
        this.sequenceNumber = sequenceNumber;
    }

    public boolean checkHasSequenceNumber()
    {
        return (flags & DirectoryRefreshFlags.HAS_SEQ_NUM) != 0;
    }

    public void applyHasSequenceNumber()
    {
        flags |= DirectoryRefreshFlags.HAS_SEQ_NUM;
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
        return (flags & DirectoryRefreshFlags.HAS_SERVICE_ID) != 0;
    }

    public void applyHasServiceId()
    {
        flags |= DirectoryRefreshFlags.HAS_SERVICE_ID;
    }

    public boolean checkClearCache()
    {
        return (flags & DirectoryRefreshFlags.CLEAR_CACHE) != 0;
    }

    public void applyClearCache()
    {
        flags |= DirectoryRefreshFlags.CLEAR_CACHE;
    }

    public boolean checkSolicited()
    {
        return (flags & DirectoryRefreshFlags.SOLICITED) != 0;
    }

    public void applySolicited()
    {
        flags |= DirectoryRefreshFlags.SOLICITED;
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DirectoryRefresh: \n");

        stringBuf.append(tab);
        stringBuf.append(state());
        stringBuf.append(eol);

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
        stringBuf.append("clearCache: ");
        stringBuf.append(checkClearCache());
        stringBuf.append(eol);

        stringBuf.append(tab);
        stringBuf.append("solicited: ");
        stringBuf.append(checkSolicited());
        stringBuf.append(eol);

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