/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsgType;

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
