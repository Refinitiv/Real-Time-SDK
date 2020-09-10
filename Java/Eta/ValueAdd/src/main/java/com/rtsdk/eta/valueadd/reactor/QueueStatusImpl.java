package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.State;
import com.rtsdk.eta.codec.StatusMsg;
import com.rtsdk.eta.valueadd.domainrep.rdm.queue.QueueMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.queue.QueueStatus;
import com.rtsdk.eta.valueadd.domainrep.rdm.queue.QueueStatusFlags;

class QueueStatusImpl extends QueueMsgImpl implements QueueStatus
{
    int _flags;
    State _state = CodecFactory.createState();
    StatusMsg _statusMsg = (StatusMsg)CodecFactory.createMsg();

    @Override
    public QueueMsgType rdmMsgType()
    {
        return QueueMsgType.STATUS;
    }

    @Override
    public int encode(EncodeIterator eIter)
    {
    	int ret = CodecReturnCodes.SUCCESS;
    
    	_statusMsg.clear();
    	_statusMsg.msgClass(MsgClasses.STATUS);
    	_statusMsg.streamId(streamId());
    	_statusMsg.domainType(domainType());
    	_statusMsg.containerType(DataTypes.NO_DATA);
    	_statusMsg.flags(_flags);

    	_statusMsg.applyHasState();
    	_statusMsg.state().code(_state.code());
    	_statusMsg.state().streamState(_state.streamState());
    	_statusMsg.state().dataState(_state.dataState());
    	      
    	if ((ret = _statusMsg.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
    	{
    		return ret;
    	}
    
    	if ((ret = _statusMsg.encodeComplete(eIter,  true)) < CodecReturnCodes.SUCCESS)
    	{
    		return ret;
    	}
    	return ret;  
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        int ret = CodecReturnCodes.SUCCESS;
        _opCode = OpCodes.STATUS;
        streamId(msg.streamId());
        domainType(msg.domainType());
        
        if (msg.msgClass() == MsgClasses.STATUS && msg.containerType() == DataTypes.NO_DATA)
        {
            StatusMsg statusMsg = (StatusMsg)msg;
            if (statusMsg.checkHasState())
            {
                statusMsg.state().copy(_state);
                applyHasState();
            }
            else
            {
            	ret = CodecReturnCodes.FAILURE;
            }
        }
        else
        {
            ret = CodecReturnCodes.FAILURE;
        }
        
        return ret;        
    }
    
    @Override
    public void flags(int flags)
    {
        _flags = flags;
    }
    
    @Override
    public int flags()
    {
        return _flags;
    }

    @Override
    public boolean checkHasState()
    {
        return (_flags & QueueStatusFlags.HAS_STATE) != 0;
    }
    
    @Override
    public void applyHasState()
    {
        _flags |= QueueStatusFlags.HAS_STATE;
    }
    
    @Override
    public void state(State state)
    {
        _state = state;
    }

    @Override
    public State state()
    {
        return _state;
    }

    public void clear()
    {
        _state.clear();
    }
}
