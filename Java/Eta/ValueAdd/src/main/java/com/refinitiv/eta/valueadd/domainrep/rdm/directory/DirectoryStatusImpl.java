/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class DirectoryStatusImpl extends MsgBaseImpl
{
    private final State state;
    private long filter;
    private int serviceId;
    private int flags;
    
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();           
    
    DirectoryStatusImpl()
    {
        super();
        state = CodecFactory.createState();
    }
    
    public void applyHasFilter()
    {
        flags |= DirectoryStatusFlags.HAS_FILTER;
    }
    
    public boolean checkHasFilter()
    {
        return (flags & DirectoryStatusFlags.HAS_FILTER) != 0;
    }
    
    public void applyHasServiceId()
    {
        flags |= DirectoryStatusFlags.HAS_SERVICE_ID;
    }
    
    public boolean checkHasServiceId()
    {
        return (flags & DirectoryStatusFlags.HAS_SERVICE_ID) != 0;
    }
    
    public int copy(DirectoryStatus destStatusMsg)
    {
        assert (destStatusMsg != null) : "destStatusMsg must be non-null";
        destStatusMsg.streamId(streamId());
        if(checkHasFilter())
        {
            destStatusMsg.applyHasFilter();
            destStatusMsg.filter(filter);
        }
        if(checkHasServiceId())
        {
            destStatusMsg.applyHasServiceId();
            destStatusMsg.serviceId(serviceId);
        }
        
        if (checkClearCache())
        {
            destStatusMsg.applyClearCache();
        }
        
        if (checkHasState())
        {
            destStatusMsg.applyHasState();
            destStatusMsg.state().streamState(this.state.streamState());
            destStatusMsg.state().dataState(this.state.dataState());
            destStatusMsg.state().code(this.state.code());

            if (this.state.text().length() > 0)
            {
                Buffer stateText = CodecFactory.createBuffer();
                ByteBuffer byteBuffer = ByteBuffer.allocate(this.state.text().length());
                this.state.text().copy(byteBuffer);
                stateText.data(byteBuffer);
                destStatusMsg.state().text(stateText);
            }
        }
       
        return CodecReturnCodes.SUCCESS;
    }
    
    public void flags(int flags)
    {
        this.flags = flags;
    }
    
    public int flags()
    {
        return flags;
    }
    
    public void clear()
    {
        super.clear();
        serviceId = 0;
        filter = 0;
        flags = 0;
        state.clear();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
    }

    public int encode(EncodeIterator encodeIter)
    {
        statusMsg.clear();
        statusMsg.streamId(streamId());
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(DomainTypes.SOURCE);
        
        if(checkHasFilter())
        {
           statusMsg.applyHasMsgKey();
           statusMsg.msgKey().applyHasFilter();
           statusMsg.msgKey().filter(filter);
        }
        
        if (checkClearCache())
        {
            statusMsg.applyClearCache();
        }
        
        if(checkHasServiceId())
        {
           statusMsg.applyHasMsgKey();
           statusMsg.msgKey().applyHasServiceId();
           statusMsg.msgKey().serviceId(serviceId);
        }
        
        if(checkHasState())
        {
            statusMsg.applyHasState();
            statusMsg.state().dataState(state().dataState());
            statusMsg.state().streamState(state().streamState());
            statusMsg.state().code(state().code());
            statusMsg.state().text(state().text());
        }
       
        return statusMsg.encode(encodeIter);
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.STATUS)
             return CodecReturnCodes.FAILURE;
        
        StatusMsg statusMsg = (StatusMsg) msg;
        streamId(msg.streamId());
        if(statusMsg.checkHasState())
        {
            state.code(statusMsg.state().code());
            state.streamState(statusMsg.state().streamState());
            state.dataState(statusMsg.state().dataState());
            
            if (statusMsg.state().text().length() > 0)
            {
                Buffer buf = statusMsg.state().text();
                this.state.text().data(buf.data(), buf.position(), buf.length());
            }
            
           applyHasState();
        }
        
        if (statusMsg.checkClearCache())
        {
            applyClearCache();
        }
        
        MsgKey key = msg.msgKey();
        if(key != null)
        {
            if(key.checkHasFilter())
            {
                applyHasFilter();
                filter(key.filter());
            }
            
            if(key.checkHasServiceId())
            {
                applyHasServiceId();
                serviceId(key.serviceId());
            }
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    public boolean checkHasState()
    {
        return (flags & DirectoryStatusFlags.HAS_STATE) != 0;
    }
    
    public void applyHasState()
    {
        flags |= DirectoryStatusFlags.HAS_STATE;
    }
    
    public void applyClearCache()
    {
        flags |= DirectoryStatusFlags.CLEAR_CACHE;
    }

    public boolean checkClearCache()
    {
        return (flags & DirectoryStatusFlags.CLEAR_CACHE) != 0;
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

    public void serviceId(int serviceId)
    {
        assert(checkHasServiceId());
        this.serviceId = serviceId;
    }

    public int serviceId()
    {
        return serviceId;
    }
    
    public State state()
    {
        return state;
    }
    
    public void state(State state)
    {
        this.state().streamState(state.streamState());
        this.state().dataState(state.dataState());
        this.state().code(state.code());
        this.state().text(state.text());
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DirectoryStatus: \n");
     
        if (checkHasServiceId())
        {
            stringBuf.append(tab);
            stringBuf.append("serviceId: ");
            stringBuf.append(serviceId());
            stringBuf.append(eol);
        }
        
        if (checkHasFilter())
        {
            stringBuf.append(tab);
            stringBuf.append("filter: ");
            stringBuf.append(filter());
            stringBuf.append(eol);
        }

        if (checkHasState())
        {
            stringBuf.append(tab);
            stringBuf.append("state: ");
            stringBuf.append(state());
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