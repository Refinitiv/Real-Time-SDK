package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RequestMsg;

class TunnelStreamMsgImpl extends TunnelStreamMsg
    implements TunnelStreamMsg.TunnelStreamRequest,
    TunnelStreamMsg.TunnelStreamData,
    TunnelStreamMsg.TunnelStreamAck
{
    int _opCode;
    int _streamId;
    int _domainType;
    int _serviceId;
    String _name;
    ClassOfService _classOfService;

    int _seqNum, _seqNum2;
    int _actionOpCode;
    
    Buffer _tmpBuffer; /* Used for encoding protocol, such as in the extended header. */

    private int _recvWindow;

    static final boolean _traceXml = true;

    private Msg _encMsg;
    
    // tunnel stream fragmentation attributes for TunnelStreamData
    int _dataMsgFlag;
    long _totalMsgLength;
    long _fragmentNumber;
    int _messageId;
    int _containerType;
    
    // default stream version
    int _streamVersion = CosCommon.CURRENT_STREAM_VERSION;
    
    TunnelStreamMsgImpl()
    {
        _encMsg = CodecFactory.createMsg();
        _tmpBuffer = CodecFactory.createBuffer();
    }
    
    void opCode(int opCode) { _opCode = opCode; }

    int opCode() { return _opCode; }

    void streamId(int streamId) { _streamId = streamId; }

    int streamId() { return _streamId; }

    void domainType(int domainType) { _domainType = domainType; }

    int domainType() { return _domainType; }

    void serviceId(int serviceId) { _serviceId = serviceId; }

    int serviceId() { return _serviceId; }
    
    void name(String name) { _name = name; }

    String name() { return _name; }
    
    void classOfService(ClassOfService classOfService) { _classOfService = classOfService; }
    ClassOfService classOfService() { return _classOfService; }

    private void clearBase()
    {
        _streamId = 0;
        _domainType = 0;
        _serviceId = 0;
        _name = null;
        _classOfService = null;
    }
    
    /* Request Header ***/

    public void clearRequest()
    {
        clearBase();
        _opCode = OpCodes.REQUEST;
    }

    public int requestBufferSize()
    {
        return  _name.length() + 256;
    }

    /* Sets the message header members of a TunnelStream request. */
    private void setupRequestMsgHeader(RequestMsg requestMsg)
    {
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(_streamId);
        requestMsg.domainType(_domainType);
        requestMsg.containerType(DataTypes.FILTER_LIST);
        
        requestMsg.applyPrivateStream();
        requestMsg.applyQualifiedStream();
        requestMsg.applyStreaming();
        
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data(_name);
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().serviceId(_serviceId);

        requestMsg.msgKey().applyHasFilter();
        requestMsg.msgKey().filter(_classOfService.filterFlags());
    }

    @Override
    public int encodeRequest(EncodeIterator encIter, RequestMsg tmpRequestMsg)
    {
        int ret = CodecReturnCodes.SUCCESS;
        
        setupRequestMsgHeader(tmpRequestMsg);

        if ((ret = tmpRequestMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        // encode class of service as payload
        if ((ret = _classOfService.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = tmpRequestMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return ret;
    }

    @Override
    public int encodeRequestAsMsg(EncodeIterator encIter, Buffer classOfServiceBuffer, RequestMsg requestMsg)
    {
        int ret = CodecReturnCodes.SUCCESS;

        setupRequestMsgHeader(requestMsg);

        encIter.clear();
        encIter.setBufferAndRWFVersion(classOfServiceBuffer, 
                _classOfService.common().protocolMajorVersion(),
                _classOfService.common().protocolMinorVersion());

        // encode class of service as payload
        if ((ret = _classOfService.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        requestMsg.encodedDataBody(classOfServiceBuffer);

        return CodecReturnCodes.SUCCESS;
    }	

    /* Data Header ***/

    public void clearData()
    {
        clearBase();
        _opCode = OpCodes.DATA;
        _seqNum = 0;
        _dataMsgFlag = 0;
        _totalMsgLength = 0;
        _fragmentNumber = 0;
        _messageId = 0;
        _containerType = 0;
    }

    public void clearRetrans()
    {
        clearBase();
        _opCode = OpCodes.RETRANS;
        _seqNum = 0;
    }

    public int dataMsgBufferSize()
    {
        return 128;
    }

	public int dataMsgFlag()
	{
		return _dataMsgFlag;
	}

	public void dataMsgFlag(int flag)
	{
		_dataMsgFlag = flag;
	}

	public long totalMsgLength()
	{
		return _totalMsgLength;
	}

	public void totalMsgLength(long totalMsgLength)
	{
		_totalMsgLength = totalMsgLength;
	}

	public long fragmentNumber()
	{
		return _fragmentNumber;
	}

	public void fragmentNumber(long fragmentNumber)
	{
		_fragmentNumber = fragmentNumber;
	}

	public int messageId()
	{
		return _messageId;
	}

	public void messageId(int messageId)
	{
		_messageId = messageId;
	}

	public int containerType()
	{
		return _containerType;
	}

	public void containerType(int containerType)
	{
		_containerType = containerType;
	}	

    public int encodeDataInit(EncodeIterator encIter)
    {
        GenericMsg genericMsg = (GenericMsg)_encMsg;
        int ret;

        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.streamId(_streamId);
        genericMsg.domainType(_domainType);
        genericMsg.containerType(DataTypes.MSG);
        genericMsg.applyHasExtendedHdr();
        genericMsg.applyMessageComplete();

        genericMsg.applyHasSeqNum();
        genericMsg.seqNum(_seqNum);

        if ((ret = genericMsg.encodeInit(encIter,  0)) != CodecReturnCodes.ENCODE_EXTENDED_HEADER)
            return ret;
        
        if ((ret = encIter.encodeNonRWFInit(_tmpBuffer)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        int lengthRequired = 2 + ((_dataMsgFlag & Flags.FRAGMENTED) > 0 ? 11 : 0);
        if (_tmpBuffer.length() < lengthRequired)
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        
        _tmpBuffer.data().put((byte)_opCode);

    	if ((_classOfService != null && _classOfService.common().streamVersion() >= CosCommon.CURRENT_STREAM_VERSION) ||
        	(_classOfService == null && _streamVersion >= CosCommon.CURRENT_STREAM_VERSION)) // only current stream version or greater supports fragmentation
    	{
	        // fragmentation flag
	        _tmpBuffer.data().put((byte)_dataMsgFlag);
	        
	        // populate other fragmentation fields if fragmentation flag set
	        if ((_dataMsgFlag & Flags.FRAGMENTED) > 0)
	        {
		        _tmpBuffer.data().putInt((int)_totalMsgLength);
		
		        _tmpBuffer.data().putInt((int)_fragmentNumber);
		
		        _tmpBuffer.data().putShort((short)_messageId);
		
		        _tmpBuffer.data().put((byte)_containerType);
	        }
    	}

        if ((ret = encIter.encodeNonRWFComplete(_tmpBuffer, true)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        if ((ret = _encMsg.encodeExtendedHeaderComplete(encIter,  true))
                < CodecReturnCodes.SUCCESS)
            return ret;
        
        return CodecReturnCodes.SUCCESS;
    }

    public int encodeDataComplete(EncodeIterator encIter)
    {
        return _encMsg.encodeComplete(encIter, true);
    }

    // JUnit test method
    int encodeDataInitOpaque(EncodeIterator encIter)
    {
        GenericMsg genericMsg = (GenericMsg)_encMsg;
        int ret;

        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.streamId(_streamId);
        genericMsg.domainType(_domainType);
        genericMsg.containerType(DataTypes.OPAQUE);
        genericMsg.applyHasExtendedHdr();
        genericMsg.applyMessageComplete();

        genericMsg.applyHasSeqNum();
        genericMsg.seqNum(_seqNum);

        if ((ret = genericMsg.encodeInit(encIter,  0)) != CodecReturnCodes.ENCODE_EXTENDED_HEADER)
            return ret;
        
        if ((ret = encIter.encodeNonRWFInit(_tmpBuffer)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        int lengthRequired = 2 + ((_dataMsgFlag & Flags.FRAGMENTED) > 0 ? 11 : 0);
        if (_tmpBuffer.length() < lengthRequired)
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        
        _tmpBuffer.data().put((byte)_opCode);

    	if ((_classOfService != null && _classOfService.common().streamVersion() >= CosCommon.CURRENT_STREAM_VERSION) ||
        	(_classOfService == null && _streamVersion >= CosCommon.CURRENT_STREAM_VERSION)) // only current stream version or greater supports fragmentation
    	{
	        // fragmentation flag
	        _tmpBuffer.data().put((byte)_dataMsgFlag);
	        
	        // populate other fragmentation fields if fragmentation flag set
	        if ((_dataMsgFlag & Flags.FRAGMENTED) > 0)
	        {
		        _tmpBuffer.data().putInt((int)_totalMsgLength);
		
		        _tmpBuffer.data().putInt((int)_fragmentNumber);
		
		        _tmpBuffer.data().putShort((short)_messageId);
		
		        _tmpBuffer.data().put((byte)(_containerType - DataTypes.CONTAINER_TYPE_MIN));
	        }
    	}

        if ((ret = encIter.encodeNonRWFComplete(_tmpBuffer, true)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        if ((ret = _encMsg.encodeExtendedHeaderComplete(encIter,  true))
                < CodecReturnCodes.SUCCESS)
            return ret;
        
        return CodecReturnCodes.SUCCESS;
    }

    /* Ack Header ***/

    public void clearAck()
    {
        clearBase();
        _opCode = OpCodes.ACK;
        _actionOpCode = 0;
        _recvWindow = 0;
    }

    public void seqNum(int seqNum)
    {
        _seqNum = seqNum;
    }
    
    public int seqNum()
    {
        return _seqNum;
    }
    

    public int recvWindow()
    {
        return _recvWindow;
    }

    public void recvWindow(int recvWindow)
    {
        _recvWindow = recvWindow;
    }

    public int ackBufferSize(AckRangeList nakRangeList)
    {
        return 128 + ((nakRangeList != null) ? nakRangeList.count() * 2 * 4 : 0);
    }

    public int encodeAck(EncodeIterator encIter, AckRangeList ackRangeList, AckRangeList nakRangeList, int actionOpCode)
    {
        GenericMsg genericMsg = (GenericMsg)_encMsg;
        ByteBuffer byteBuffer;
        int ret;

        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.streamId(_streamId);
        genericMsg.domainType(_domainType);
        genericMsg.containerType(DataTypes.NO_DATA);
        genericMsg.applyHasExtendedHdr();
        genericMsg.applyMessageComplete();

        if ((ret = genericMsg.encodeInit(encIter,  0)) != CodecReturnCodes.ENCODE_EXTENDED_HEADER)
            return ret;

        if ((ret = encIter.encodeNonRWFInit(_tmpBuffer)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        if (_tmpBuffer.length() < 1 /* OpCode */ + 4 /* seqNum */ 
                + 1 + ((nakRangeList != null) ? nakRangeList.count() * 2 * 4: 0) /* Nak ranges */
                + 1 + ((ackRangeList != null) ? ackRangeList.count() * 2 * 4: 0) /* Ack ranges */
                + 4 /* RecvWindow */)
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        
        byteBuffer = _tmpBuffer.data();
        
        /* OpCode */
        byteBuffer.put((byte)OpCodes.ACK);
      
        if (actionOpCode < 0x80)
        {
        	byteBuffer.put((byte)actionOpCode);
 
        }
        else
        {
        	short v1 = (short) (actionOpCode | 0x8000);
        	byteBuffer.putShort(v1);
        }

        /* SeqNum */
        byteBuffer.putInt(_seqNum);
        
        /* Nak Ranges */
        if (nakRangeList == null)
            byteBuffer.put((byte)0);
        else
        {
            int[] nakRanges = nakRangeList.rangeArray();

            byteBuffer.put((byte)nakRangeList.count());
            for (int i = 0; i < nakRangeList.count() * 2; ++i)
            {
                byteBuffer.putInt(nakRanges[i]);
            }
        }
        
        if (ackRangeList == null)
            byteBuffer.put((byte)0);
        else
        {
            int[] ackRanges = ackRangeList.rangeArray();

            byteBuffer.put((byte)ackRangeList.count());
            for (int i = 0; i < ackRangeList.count() * 2; ++i)
            {
                byteBuffer.putInt(ackRanges[i]);
            }
        }
        
        /* RecvWindow */
        byteBuffer.putInt(_recvWindow);
        
        if ((ret = encIter.encodeNonRWFComplete(_tmpBuffer, true)) != CodecReturnCodes.SUCCESS)
            return ret;

        if ((ret = _encMsg.encodeExtendedHeaderComplete(encIter,  true))
                < CodecReturnCodes.SUCCESS)
            return ret;

        if ((ret = _encMsg.encodeComplete(encIter, true)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        return CodecReturnCodes.SUCCESS;
    }

    int decode(DecodeIterator decIter, GenericMsg genericMsg,
            AckRangeList ackRangeList, AckRangeList nakRangeList)
    {
        int i;
        int tmpPos, tmpLimit;
        ByteBuffer byteBuffer;

        if (genericMsg.checkHasExtendedHdr() == false)
            return CodecReturnCodes.INCOMPLETE_DATA;
        
        /* Decode extendedHeader. */
        byteBuffer = genericMsg.extendedHeader().data();
        tmpPos = byteBuffer.position();
        tmpLimit = byteBuffer.limit();
        
        byteBuffer.position(genericMsg.extendedHeader().position());
        byteBuffer.limit(byteBuffer.position() + genericMsg.extendedHeader().length());
        
        _streamId = genericMsg.streamId();
        _domainType = genericMsg.domainType();
        try
        {
            _opCode = byteBuffer.get() & 0xff;

            switch(_opCode)
            {
                case OpCodes.DATA:
                case OpCodes.RETRANS:
                	if ((_classOfService != null && _classOfService.common().streamVersion() >= CosCommon.CURRENT_STREAM_VERSION) ||
                		(_classOfService == null && _streamVersion >= CosCommon.CURRENT_STREAM_VERSION)) // only current stream version or greater supports fragmentation
                	{
	                	_dataMsgFlag = byteBuffer.get();
	                    // decode other fragmentation fields if fragmentation flag set
	                    if ((_dataMsgFlag & Flags.FRAGMENTED) > 0)
	                    {
	                    	_totalMsgLength = byteBuffer.getInt();
	                    	_fragmentNumber = byteBuffer.getInt();
	                    	_messageId = byteBuffer.getShort();
	                    	_containerType = byteBuffer.get() + DataTypes.CONTAINER_TYPE_MIN;
	                    }
                	}
                	
                    if (genericMsg.checkHasSeqNum() == false)
                        return CodecReturnCodes.INCOMPLETE_DATA;

                    _seqNum = (int)genericMsg.seqNum();

                    break;

                case OpCodes.ACK:
                {
                    int ranges[];
                    int count;  
                    _actionOpCode = readUShort15rb(byteBuffer);   
                    
                    _seqNum = byteBuffer.getInt();

                    /* Nak ranges */
                    count = byteBuffer.get() & 0xff;
                    nakRangeList.count(count);

                    ranges = nakRangeList.rangeArray();
                    count *= 2;
                    for (i = 0; i < count; ++i)
                        ranges[i] = byteBuffer.getInt();

                    /* Selective ack ranges */
                    count = byteBuffer.get() & 0xff;
                    ackRangeList.count(count);

                    ranges = ackRangeList.rangeArray();
                    count *= 2;
                    for (i = 0; i < count; ++i)
                        ranges[i] = byteBuffer.getInt();    
                    
                    _recvWindow = byteBuffer.getInt();

                    break;
                }

                default:
                    return CodecReturnCodes.INCOMPLETE_DATA;

            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        byteBuffer.limit(tmpLimit);
        byteBuffer.position(tmpPos);
        
        return CodecReturnCodes.SUCCESS;
    }

    String xmlDumpBegin(AckRangeList ackRangeList, AckRangeList nakRangeList)
    {
        switch(_opCode)
        {
            case OpCodes.DATA:
                return "<TunnelData streamId=\"" + _streamId + "\" seqNum=\"" + _seqNum + "\" >\n";

            case OpCodes.RETRANS:
                return "<TunnelRetrans streamId=\"" + _streamId + "\" seqNum=\"" + _seqNum + "\" >\n";
                
            case OpCodes.ACK:
            {
                String str = "<TunnelAck streamId=\"" + _streamId + "\" seqNum=\"" + _seqNum + "\" recvWindow=\"" + _recvWindow;
                if (ackRangeList != null && ackRangeList.count() > 0)
                {
                    int count = ackRangeList.count() * 2;
                    str += "\" ackRanges=\"";
                    for (int i = 0; i < count; i += 2)
                    {
                        str += ackRangeList.rangeArray()[i] + "-" + ackRangeList.rangeArray()[i + 1] ;
                        if (i < count - 2) str += ",";
                    }
                }

                if (nakRangeList != null && nakRangeList.count() > 0)
                {
                    int count = nakRangeList.count() * 2;
                    str += "\" nakRanges=\"";
                    for (int i = 0; i < count; i += 2)
                    {
                        str += nakRangeList.rangeArray()[i] + "-" + nakRangeList.rangeArray()[i + 1] ;
                        if (i < count - 2) str += ",";
                    }
                }

                str += "\" />\n";

                return str;
            }
                
            default:
                return "<TunnelUnknown streamId=\"" + _streamId + "\"opCode=\"" + _opCode + "\" />\n";
        }
    }
    
    public int flag()
    {
    	return _actionOpCode;
    }
    
    String xmlDumpEnd()
    {
        switch(_opCode)
        {
            case OpCodes.DATA:
                return "</TunnelData>\n";
            case OpCodes.RETRANS:
                return "</TunnelRetrans>\n";
            default:
                return "";
        }
    }
    
	short readUShort15rb(ByteBuffer byteBuffer) throws Exception
    {
    	int b = readUnsignedByte(byteBuffer);
        if ((b & 0x80) != 0)
        {
        	return (short) (((b & 0x7F) << 8)  + readUnsignedByte(byteBuffer));
        }
        return (short) b;
    }
	
	   int readUnsignedByte(ByteBuffer byteBuffer) throws Exception
	    {
	        short val = byteBuffer.get();
	        if (val < 0)
	            val &= 0xFF;
	        return val;
	    }	
}
