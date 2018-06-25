package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueData;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataTimeoutCode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgType;

class QueueDataImpl extends QueueMsgImpl implements QueueData
{
    final int QUEUE_DATA_OPCODE = 1; 
      
    Buffer _tempBuffer = CodecFactory.createBuffer();
    Buffer _tempBuffer2 = CodecFactory.createBuffer();
    GenericMsg _genericMsg = (GenericMsg)CodecFactory.createMsg();
    int _flags;
    int _queueDepth;
    
    Buffer _sourceName = CodecFactory.createBuffer();
    Buffer _destName = CodecFactory.createBuffer();
    Buffer _dataBuffer = CodecFactory.createBuffer();   

    public QueueDataImpl()
    {
        _timeout = QueueDataTimeoutCode.INFINITE;
    }
    
    @Override
    public QueueMsgType rdmMsgType()
    {
        return QueueMsgType.DATA;
    }

    @Override
    public int encode(EncodeIterator eIter)
    {
        int ret = CodecReturnCodes.SUCCESS;
        
        try
        {
            _genericMsg.clear();
            _genericMsg.msgClass(MsgClasses.GENERIC);
            _genericMsg.streamId(streamId());
            _genericMsg.domainType(_domainType);
            _genericMsg.containerType(_containerType);
            _genericMsg.applyHasExtendedHdr();
                       
            _genericMsg.applyMessageComplete();

            _genericMsg.applyHasSeqNum();
            _genericMsg.seqNum(_seqNum); 

            _genericMsg.applyHasMsgKey();
            _genericMsg.msgKey().applyHasName();
            _genericMsg.msgKey().name().data( _destName.data(), _destName.position(), _destName.length());
            
            if ((ret = _genericMsg.encodeInit(eIter,  0)) != CodecReturnCodes.ENCODE_EXTENDED_HEADER)
                return ret;

            _tempBuffer.clear();
            
            if ((ret = eIter.encodeNonRWFInit(_tempBuffer)) != CodecReturnCodes.SUCCESS)
                return ret;            
            
            if (_tempBuffer.length() < 2 + _sourceName.length()
                    + 2 /* Flags */
                    + 1 + 8 /* Timeout */
                    + 1 + 8 /* Identifier */)
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            
            /* Opcode */
            _tempBuffer.data().put((byte)QUEUE_DATA_OPCODE);

            /* Flags - always set to 0 (will be internally replaced if needed) */
            TunnelStreamUtil.writeResBit15(0, _tempBuffer.data());

            /* From queue */
            _tempBuffer.data().put((byte)_sourceName.length());
            
            int tmpPos = _sourceName.data().position();
            int tmpLimit = _sourceName.data().limit();
            
            _tempBuffer.data().put(_sourceName.data());
            _sourceName.data().position(tmpPos);
            _sourceName.data().limit(tmpLimit);

            /* Timeout */		
            /* Reserve byte for length */
            int encodedLengthPos = _tempBuffer.data().position();
            _tempBuffer.data().position(encodedLengthPos + 1);		

            /* Encode */
            if ((ret = TunnelStreamUtil.writeLong64ls(_timeout, _tempBuffer.data())) != CodecReturnCodes.SUCCESS)
                return ret;		
            /* Fill in the encoded length */
            assert (_tempBuffer.data().position() - (encodedLengthPos + 1)) <= 255;
            _tempBuffer.data().put(encodedLengthPos, (byte)(_tempBuffer.data().position() - (encodedLengthPos + 1)));

            /* Identifier */		
            /* Reserve byte for length */
            encodedLengthPos = _tempBuffer.data().position();
            _tempBuffer.data().position(encodedLengthPos + 1);
            
            /* Encode */
            if ((ret = TunnelStreamUtil.writeLong64ls(_identifier, _tempBuffer.data())) != CodecReturnCodes.SUCCESS)
                return ret;
            
            /* Fill in the encoded length */
            assert (_tempBuffer.data().position() - (encodedLengthPos + 1)) <= 255;
            _tempBuffer.data().put(encodedLengthPos, (byte)(_tempBuffer.data().position() - (encodedLengthPos + 1)));
            
            /* QueueDepth */
            _tempBuffer.data().putShort((short)_queueDepth);
                                              
            if ((ret = eIter.encodeNonRWFComplete(_tempBuffer,  true)) != CodecReturnCodes.SUCCESS)
                return ret;
                    
            if ((ret = _genericMsg.encodeExtendedHeaderComplete(eIter,  true)) != CodecReturnCodes.ENCODE_CONTAINER)
                return ret;
             
            // payload
            if ((ret = eIter.encodeNonRWFInit(_tempBuffer2)) != CodecReturnCodes.SUCCESS)
                return ret;
            
            tmpPos = _dataBuffer.data().position();
            tmpLimit = _dataBuffer.data().limit();
            _dataBuffer.data().limit(_dataBuffer.position() + _dataBuffer.length());
            _dataBuffer.data().position(_dataBuffer.position());
            _tempBuffer2.data().put(_dataBuffer.data());
            _dataBuffer.data().limit(tmpLimit);
            _dataBuffer.data().position(tmpPos);
            
            if ((ret = eIter.encodeNonRWFComplete(_tempBuffer2,  true)) != CodecReturnCodes.SUCCESS)
                return ret;
                
            return _genericMsg.encodeComplete(eIter, true);
        }
        catch (BufferOverflowException e)
        {
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        catch (Exception e)
        {
            return CodecReturnCodes.FAILURE;
        }
    }
    
    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
    	int msgLength;
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

    		tmpByteBuf.position(genericMsg.extendedHeader().position());
			_opCode = tmpByteBuf.get();
			
			_flags = TunnelStreamUtil.readResBit15(tmpByteBuf);
			
			streamId(genericMsg.streamId());
	        domainType(genericMsg.domainType());

    		if (genericMsg.checkHasMsgKey() == false)
    			return CodecReturnCodes.INCOMPLETE_DATA;
					
    		if (genericMsg.msgKey().checkHasName() == false)
    			return CodecReturnCodes.INCOMPLETE_DATA;
						
    		if (genericMsg.checkHasSeqNum() == false)
    			return CodecReturnCodes.INCOMPLETE_DATA;
					
    		_destName.data(genericMsg.msgKey().name().data(),
							genericMsg.msgKey().name().position(),
							genericMsg.msgKey().name().length()
							);			
    		
            _dataBuffer.data(genericMsg.encodedDataBody().data(), genericMsg.encodedDataBody().position(),
                             genericMsg.encodedDataBody().length());
    		
    		_containerType = genericMsg.containerType();

			/* SeqNum */
    		_seqNum = (int)genericMsg.seqNum();
					
			/* fromQueue length */
    		msgLength = tmpByteBuf.get() & 0xff;

			/* fromQueue */
    		_sourceName.data(tmpByteBuf, tmpByteBuf.position(), msgLength);
    		tmpByteBuf.position(tmpByteBuf.position() + msgLength);
					
    		msgLength = tmpByteBuf.get() & 0xff;
    		_timeout = TunnelStreamUtil.readLong64ls(msgLength, tmpByteBuf);
    	
    		/* Identifier */
    		msgLength = tmpByteBuf.get() & 0xff;
    		_identifier = TunnelStreamUtil.readLong64ls(msgLength, tmpByteBuf);
					
            /* QueueDepth */
    		_queueDepth = tmpByteBuf.getShort();

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
    public int encodeInit(EncodeIterator eIter)
    {
        int ret = CodecReturnCodes.SUCCESS;

        _genericMsg.clear();
        _genericMsg.msgClass(MsgClasses.GENERIC);
        _genericMsg.streamId(streamId());
        _genericMsg.domainType(_domainType);
        _genericMsg.containerType(_containerType);
        _genericMsg.applyHasExtendedHdr();
                   
        _genericMsg.applyMessageComplete();

        _genericMsg.applyHasSeqNum();
        _genericMsg.seqNum(1);  // place holder

        _genericMsg.applyHasMsgKey();
        _genericMsg.msgKey().applyHasName();
        _genericMsg.msgKey().name().data( _destName.data(), _destName.position(), _destName.length());   
                
        if ((ret = _genericMsg.encodeInit(eIter,  0)) != CodecReturnCodes.ENCODE_EXTENDED_HEADER)
            return ret;

        _tempBuffer.clear();
        if ((ret = eIter.encodeNonRWFInit(_tempBuffer)) != CodecReturnCodes.SUCCESS)
            return ret;
                
		if (_tempBuffer.length() < 2 + _sourceName.length() 
				+ 1 + 8 /* Timeout */
				+ 1 + 8 /* Identifier */)
			return CodecReturnCodes.BUFFER_TOO_SMALL;
		
		/* Opcode */
		_tempBuffer.data().put((byte)QUEUE_DATA_OPCODE);

        /* Flags - always set to 0 (will be internally replaced if needed) */
        TunnelStreamUtil.writeResBit15(0, _tempBuffer.data());

		/* From queue */
		_tempBuffer.data().put((byte)_sourceName.length());
		
		int tmpPos = _sourceName.data().position();
		int tmpLimit = _sourceName.data().limit();
		_tempBuffer.data().put(_sourceName.data());
		_sourceName.data().position(tmpPos);
		_sourceName.data().limit(tmpLimit);

		/* Timeout */		
		/* Reserve byte for length */
		int encodedLengthPos = _tempBuffer.data().position();
		_tempBuffer.data().position(encodedLengthPos + 1);		
		/* Encode */
		if ((ret = TunnelStreamUtil.writeLong64ls(_timeout, _tempBuffer.data())) != CodecReturnCodes.SUCCESS)
			return ret;		
		/* Fill in the encoded length */
		assert (_tempBuffer.data().position() - (encodedLengthPos + 1)) <= 255;
		_tempBuffer.data().put(encodedLengthPos, (byte)(_tempBuffer.data().position() - (encodedLengthPos + 1)));

		/* Identifier */		
		/* Reserve byte for length */
		encodedLengthPos = _tempBuffer.data().position();
		_tempBuffer.data().position(encodedLengthPos + 1);
		
		/* Encode */
		if ((ret = TunnelStreamUtil.writeLong64ls(_identifier, _tempBuffer.data())) != CodecReturnCodes.SUCCESS)
			return ret;
		
		/* Fill in the encoded length */
		assert (_tempBuffer.data().position() - (encodedLengthPos + 1)) <= 255;
		_tempBuffer.data().put(encodedLengthPos, (byte)(_tempBuffer.data().position() - (encodedLengthPos + 1)));
				               
        /* QueueDepth */
        _tempBuffer.data().putShort((short)_queueDepth);

        if ((ret = eIter.encodeNonRWFComplete(_tempBuffer,  true)) != CodecReturnCodes.SUCCESS)
            return ret;
                
        if ((ret = _genericMsg.encodeExtendedHeaderComplete(eIter,  true)) != CodecReturnCodes.ENCODE_CONTAINER)
            return ret;
       
        return CodecReturnCodes.SUCCESS;
                        
    }
    
    @Override
    public int encodeComplete(EncodeIterator eIter, boolean success)
    {
        return _genericMsg.encodeComplete(eIter, success);
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

    @Override
    public void timeout(long timeout)
    {
        _timeout = timeout;
    }

    @Override
    public long timeout()
    {
        return _timeout;
    }

    @Override
    public void encodedDataBody(Buffer data)
    {
        _dataBuffer.data(data.data(), data.position(), data.length());
    }

    @Override
    public Buffer encodedDataBody()
    {
        return _dataBuffer;
    }    

    @Override
    public void containerType(int containerType)
    {
        _containerType = containerType;
    }

    @Override
    public int containerType()
    {
        return _containerType;
    }
        	
	int dataMsgBufferSize()
	{
		return 128 + _sourceName.length() + _destName.length();
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
    public void queueDepth(int queueDepth)
    {
        _queueDepth = queueDepth;
    }

    @Override
    public int queueDepth()
    {
        return _queueDepth;
    }

    @Override
    public boolean checkPossibleDuplicate()
    {
        return (flags() & QueueDataFlags.POSSIBLE_DUPLICATE) != 0;
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

    public void clear()
    {
        _opCode = OpCodes.DATA;
        _sourceName.clear();
        _destName.clear();
        _identifier = 0;
        _timeout = QueueDataTimeoutCode.INFINITE;
        _dataBuffer.clear();
        _flags = 0;
        _queueDepth = 0;
    }
}
