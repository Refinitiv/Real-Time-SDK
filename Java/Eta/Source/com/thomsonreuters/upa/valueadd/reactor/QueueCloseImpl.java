package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgType;

class QueueCloseImpl extends QueueMsgImpl implements QueueClose
{
    CloseMsg _closeMsg = (CloseMsg)CodecFactory.createMsg();
    
    @Override
    public QueueMsgType rdmMsgType()
    {
        return QueueMsgType.CLOSE;
    }

    @Override
    public int encode(EncodeIterator eIter)
    {
       	int ret = CodecReturnCodes.SUCCESS;
            	
    	_closeMsg.clear();
    	_closeMsg.msgClass(MsgClasses.CLOSE);
    	_closeMsg.streamId(streamId());
    	_closeMsg.domainType(domainType());
    	_closeMsg.containerType(DataTypes.NO_DATA);
    	      
    	if ((ret = _closeMsg.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
    	{
    		return ret;
    	}
    
    	if ((ret = _closeMsg.encodeComplete(eIter,  true)) < CodecReturnCodes.SUCCESS)
    	{
    		return ret;
    	}
    	return ret;
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
       int ret = CodecReturnCodes.SUCCESS;
        
       _opCode = OpCodes.CLOSE;
       streamId(msg.streamId());
       domainType(msg.domainType());
       
       if (msg.msgClass() == MsgClasses.CLOSE && msg.containerType() == DataTypes.NO_DATA)
       {
    	   CloseMsg closeMsg = (CloseMsg)msg;
    	   
    	   streamId(closeMsg.streamId()); 	
       }
       else
       {
    	   ret = CodecReturnCodes.FAILURE;
       }  	      
       return ret;            
    }
    
	int closeMsgBufferSize()
	{
		return 64;
	}
	
    public void clear()
    {
        _opCode = OpCodes.CLOSE;
    }
	
  

}
