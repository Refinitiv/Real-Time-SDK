package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRefresh;

class QueueRefreshImpl extends QueueMsgImpl implements QueueRefresh
{
    Msg _msg = CodecFactory.createMsg();
    RefreshMsg _refreshMsg = (RefreshMsg)CodecFactory.createMsg();
    DecodeIterator _decIter = CodecFactory.createDecodeIterator();
    
    Buffer _sourceName = CodecFactory.createBuffer();
    Buffer _destName = CodecFactory.createBuffer();

    Buffer _tmpBuffer = CodecFactory.createBuffer(); /* Used for encoding extended header. */    
    
    int _queueDepth = 0;
            
    @Override
    public QueueMsgType rdmMsgType()
    {
        return QueueMsgType.REFRESH;
    }

    @Override
    public int encode(EncodeIterator eIter)
    {
        int ret = CodecReturnCodes.SUCCESS;      
        _refreshMsg.clear();
        _refreshMsg.msgClass(MsgClasses.REFRESH);
        _refreshMsg.streamId(streamId());
        _refreshMsg.domainType(domainType());
        _refreshMsg.containerType(DataTypes.NO_DATA);
        _refreshMsg.applyDoNotCache();
        _refreshMsg.applyRefreshComplete();
        _refreshMsg.applySolicited();
        _refreshMsg.applyHasMsgKey();
        _refreshMsg.applyHasExtendedHdr();
        _refreshMsg.msgKey().applyHasName();
        _refreshMsg.msgKey().name(_sourceName);
        _refreshMsg.state().dataState(DataStates.OK);
        _refreshMsg.state().streamState(StreamStates.OPEN);
        _refreshMsg.state().code(StateCodes.NONE);
        
        if ((ret = _refreshMsg.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if ((ret = eIter.encodeNonRWFInit(_tmpBuffer)) != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if (_tmpBuffer.length() < 11)
        {
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        
        _tmpBuffer.data().putInt(_lastOutSeqNum);
        _tmpBuffer.data().putInt(_lastInSeqNum);

        /* QueueDepth */
        _tmpBuffer.data().putShort((short)_queueDepth);

        if ((ret = eIter.encodeNonRWFComplete(_tmpBuffer,  true)) != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if ((ret = _refreshMsg.encodeExtendedHeaderComplete(eIter,  true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _refreshMsg.encodeComplete(eIter,  true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }        
        return ret;
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {    
        _opCode = OpCodes.REFRESH;
        streamId(msg.streamId());
        domainType(msg.domainType());
    
        RefreshMsg refreshMsg = (RefreshMsg)msg;
    	if (refreshMsg.checkHasExtendedHdr() == false
    			|| refreshMsg.extendedHeader().length() < 8)
    	{
    	    // for unit test
    	    _lastInSeqNum = 0;
    		_lastOutSeqNum = 0;
    		_queueDepth = 0;
    	}
    	else
    	{
    		ByteBuffer extHdrByteBuf = refreshMsg.extendedHeader().data();
    		int startPos = refreshMsg.extendedHeader().position();

    		_lastOutSeqNum = extHdrByteBuf.getInt(startPos);
    		_lastInSeqNum = extHdrByteBuf.getInt(startPos + 4);
    		if (refreshMsg.extendedHeader().length() == 10)
    		{
    		    _queueDepth = extHdrByteBuf.getShort(startPos + 8);
    		}
    		else
    		{
    		    // for unit test
    		    _queueDepth = 0;
    		}
    	}
        _sourceName.data(refreshMsg.msgKey().name().toString());
        refreshMsg.state().copy(_state);

    	return ReactorReturnCodes.SUCCESS;        
    }
    
    @Override
    public State state()
    {
    	return _state;
    }
    
    void queueDepth(int queueDepth)
    {
        _queueDepth = queueDepth;
    }

    @Override
    public int queueDepth()
    {
        return _queueDepth;
    }    
    
    @Override
    public Buffer sourceName()
    {
        return _sourceName;
    }

    void sourceName(Buffer sourceName)
    {
        _sourceName.data(sourceName.data(), sourceName.position(), sourceName.length());
    }

    public void clear()
    {
    	_opCode = OpCodes.REFRESH;
    	_queueDepth = 0;
    }
}
