/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueRequest;

class QueueRequestImpl extends QueueMsgImpl implements QueueRequest
{
    final int SUBSTREAM_REQUEST_CLIENT_OPCODE = 1;

    Msg _msg = CodecFactory.createMsg();
    RequestMsg _requestMsg = (RequestMsg)CodecFactory.createMsg();
    DecodeIterator _decIter = CodecFactory.createDecodeIterator();

    Buffer _tmpBuffer = CodecFactory.createBuffer(); /* Used for encoding extended header. */
       
    Buffer _sourceName = CodecFactory.createBuffer();
    Buffer _destName = CodecFactory.createBuffer();

    @Override
    public QueueMsgType rdmMsgType()
    {
        return QueueMsgType.REQUEST;
    }

    @Override
    public int encode(EncodeIterator eIter)
    {
        int ret;    
       
        _requestMsg.clear();
        _requestMsg.msgClass(MsgClasses.REQUEST);
        _requestMsg.streamId(streamId());
        _requestMsg.domainType(domainType());
        _requestMsg.containerType(DataTypes.NO_DATA);

        _requestMsg.applyStreaming();

        _requestMsg.applyHasExtendedHdr();

        _requestMsg.msgKey().applyHasName();
        _requestMsg.msgKey().name().data(_sourceName.data());

        if ((ret = _requestMsg.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if ((ret = eIter.encodeNonRWFInit(_tmpBuffer)) != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if (_tmpBuffer.length() < 9)
        {
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        
        /* OpCode (Request uses DATA) */
        _tmpBuffer.data().put((byte)SUBSTREAM_REQUEST_CLIENT_OPCODE);
        _tmpBuffer.data().putInt(_lastOutSeqNum); // placeholder for lastSent, needs to be replaced immediately before sending
        _tmpBuffer.data().putInt(_lastInSeqNum); // placeholder for lastRecv, needs to be replaced immediately before sending

        if ((ret = eIter.encodeNonRWFComplete(_tmpBuffer,  true)) != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if ((ret = _requestMsg.encodeExtendedHeaderComplete(eIter,  true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _requestMsg.encodeComplete(eIter,  true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {    	                    
        _opCode = OpCodes.REQUEST;
        streamId(msg.streamId());
        domainType(msg.domainType());

		RequestMsg requestMsg = (RequestMsg)msg;

		if (requestMsg.checkHasExtendedHdr() == false
				|| requestMsg.extendedHeader().length() < 9)
			return CodecReturnCodes.INCOMPLETE_DATA;

		ByteBuffer extHdrByteBuf = requestMsg.extendedHeader().data();
		int startPos = requestMsg.extendedHeader().position();

		if (extHdrByteBuf.get(startPos) != OpCodes.DATA)
			return CodecReturnCodes.INCOMPLETE_DATA;

		
		_lastOutSeqNum = extHdrByteBuf.getInt(startPos + 1);
		_lastInSeqNum = extHdrByteBuf.getInt(startPos + 1 + 4);
		
   		_sourceName.data(msg.msgKey().name().data(),
				msg.msgKey().name().position(),
				msg.msgKey().name().length()
				);	

		return ReactorReturnCodes.SUCCESS;        
 
    }
    
    @Override
    public Buffer sourceName()
    {
        return _sourceName;
    }

    @Override
    public void sourceName(Buffer sourceName)
    {
        _sourceName.data(sourceName.data(), sourceName.position(), sourceName.length());
    }

    public void clear()
    {
        _opCode = OpCodes.REQUEST;    	
    }
    	
	public int requestMsgBufferSize()
	{
		return 128 + _sourceName.length();
	} 
}
