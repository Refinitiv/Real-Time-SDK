/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
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
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueAck;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsgType;

class QueueAckImpl extends QueueMsgImpl implements QueueAck
{
	final int SUBSTREAM_ACK_CLIENT_OPCODE = 2;
   
    GenericMsg _genericMsg = (GenericMsg)CodecFactory.createMsg();

    Buffer _sourceName = CodecFactory.createBuffer();
    Buffer _destName = CodecFactory.createBuffer();

    Buffer _tmpBuffer = CodecFactory.createBuffer(); 

    @Override
    public QueueMsgType rdmMsgType()
    {
        return QueueMsgType.ACK;
    }
    
    @Override
    public int encode(EncodeIterator eIter)
    {
        int ret;     
        _genericMsg.clear();		
		_genericMsg.msgClass(MsgClasses.GENERIC);
		_genericMsg.streamId(streamId());
		_genericMsg.containerType(DataTypes.NO_DATA);
		_genericMsg.domainType(domainType());
		_genericMsg.applyMessageComplete();
		_genericMsg.applyHasExtendedHdr();
		_genericMsg.applyHasSecondarySeqNum();		
		_genericMsg.secondarySeqNum(_lastOutSeqNum);
		_genericMsg.secondarySeqNum(_seqNum);
		_genericMsg.applyHasMsgKey();

		/* toName */
		_genericMsg.msgKey().applyHasName();
		_genericMsg.msgKey().name().data(_destName.data(), _destName.position(), _destName.length());
		
		if ((ret = _genericMsg.encodeInit(eIter,  0)) != CodecReturnCodes.ENCODE_EXTENDED_HEADER)
			return ret;

		_tmpBuffer.clear();
		
		if ((ret = eIter.encodeNonRWFInit(_tmpBuffer)) != CodecReturnCodes.SUCCESS)
			return ret;
				
		if (_tmpBuffer.length() < 2 + 9 /* Identifier */ + _sourceName.length())
			return CodecReturnCodes.BUFFER_TOO_SMALL;

		/* Opcode */
		_tmpBuffer.data().put((byte)SUBSTREAM_ACK_CLIENT_OPCODE);
		
		/* Write fromName length */
		_tmpBuffer.data().put((byte)_sourceName.length());
				
		/* Write fromName */
		int tmpPos = _sourceName.data().position();
		int tmpLimit = _sourceName.data().limit();
		_sourceName.data().position(_sourceName.position());
		_sourceName.data().limit(_sourceName.position() + _sourceName.length());		
		_tmpBuffer.data().put(_sourceName.data());
		_sourceName.data().limit(tmpLimit);
		_sourceName.data().position(tmpPos);	

		/* Identifier */		
		/* Reserve byte for length */
		int encodedLengthPos = _tmpBuffer.data().position();
		_tmpBuffer.data().position(encodedLengthPos + 1);
		
		/* Encode */
		if ((ret = TunnelStreamUtil.writeLong64ls(_identifier, _tmpBuffer.data())) != CodecReturnCodes.SUCCESS)
			return ret;
		
		/* Fill in the encoded length */
		assert (_tmpBuffer.data().position() - (encodedLengthPos + 1)) <= 255;
		_tmpBuffer.data().put(encodedLengthPos, (byte)(_tmpBuffer.data().position() - (encodedLengthPos + 1)));

		if ((ret = eIter.encodeNonRWFComplete(_tmpBuffer,  true)) != CodecReturnCodes.SUCCESS)
			return ret;
				
		if ((ret = _genericMsg.encodeExtendedHeaderComplete(eIter,  true))
				!= CodecReturnCodes.SUCCESS)
			return ret;
		
		if ((ret = _genericMsg.encodeComplete(eIter, true)) != CodecReturnCodes.SUCCESS)
			return ret;
		                        
        return CodecReturnCodes.SUCCESS;
    }
    
  
    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {                           
       	GenericMsg genericMsg = (GenericMsg)msg;
       	
		ByteBuffer tmpByteBuf;
		int tmpPos, tmpLimit;

		try
		{			
			if (genericMsg.checkHasExtendedHdr() == false)
				return CodecReturnCodes.INCOMPLETE_DATA;

			/* Decode extended header. */
			tmpByteBuf = genericMsg.extendedHeader().data();
			tmpPos = tmpByteBuf.position();
			tmpLimit = tmpByteBuf.limit();			
			
			if (genericMsg.checkHasSecondarySeqNum() == false)
				return CodecReturnCodes.INCOMPLETE_DATA;

			if (genericMsg.checkHasMsgKey() == false)
				return CodecReturnCodes.INCOMPLETE_DATA;

			if (genericMsg.msgKey().checkHasName() == false)
				return CodecReturnCodes.INCOMPLETE_DATA;
			
			/* seqNum */
			_secondarySeqNum = (int)genericMsg.secondarySeqNum();	
			_seqNum = (int)genericMsg.secondarySeqNum();	

			tmpByteBuf.position(genericMsg.extendedHeader().position());
			_opCode = tmpByteBuf.get();
			streamId(msg.streamId());
            domainType(msg.domainType());

			/* toQueue */
			_destName.data(genericMsg.msgKey().name().data(),
					genericMsg.msgKey().name().position(),
					genericMsg.msgKey().name().length()
					);

			/* fromQueue */
			int msgLength = tmpByteBuf.get() & 0xff;
			_sourceName.data(tmpByteBuf, tmpByteBuf.position(), msgLength);
			tmpByteBuf.position(tmpByteBuf.position() + msgLength);

			/* Identifier */
			msgLength = tmpByteBuf.get() & 0xff;
			_identifier = TunnelStreamUtil.readLong64ls(msgLength, tmpByteBuf);

			tmpByteBuf.limit(tmpLimit);
			tmpByteBuf.position(tmpPos);

			return CodecReturnCodes.SUCCESS;   
		}   	
		catch(Exception e)
		{
			return CodecReturnCodes.INCOMPLETE_DATA;
		}
    }
            
    @Override
    public void identifier(long identifier)
    {
        _identifier = identifier;
    }

    @Override
    public long identifier()
    {
        return _identifier;
    }
    
    public void clear()
    {
        _opCode = OpCodes.ACK;
        _sourceName.clear();
        _destName.clear();
        _identifier = 0;
    }
        
    @Override
    public Buffer sourceName()
    {
    	return _sourceName;
    }

    @Override
    public Buffer destName()
    {
    	return _destName;
    }

    @Override
    public void sourceName(Buffer sourceName)
    {
        _sourceName.data(sourceName.data(), sourceName.position(), sourceName.length());
    }

    @Override
    public void destName(Buffer destName)
    {
        _destName.data(destName.data(), destName.position(), destName.length());
    }    
        
	int ackMsgBufferSize()
	{
		return 128 + _sourceName.length() + _destName.length();
	}
	
}
