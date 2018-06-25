package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.BufferImpl;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

class EncodeIteratorImpl implements EncodeIterator
{
    private static final int MSG_CLASS_POS = 2;
    private static final int MSG_STREAMID_POS = 4;
    private static final int MSG_FLAGS_POS = 8;

    static final int ENC_ITER_MAX_LEVELS = 16;
    
    GlobalFieldSetDefDb _fieldSetDefDb;
    GlobalElementSetDefDb _elementSetDefDb;

    private final BufferWriter [] _writerByVersion = new BufferWriter[1]; // increase the size of the array when supporting new version
    // create all writers the current release supports and put them in the array
    {
        _writerByVersion[0] = new DataBufferWriterWireFormatV1();
        // add other readers as they are supported
    }
    BufferWriter _writer;
    int          _startBufPos;   /* Initial position of the buffer */
    int          _curBufPos;     /* The current position in the buffer */
    int          _endBufPos;     /* The end position in the buffer */
    int          _encodingLevel; /* Current nesting level */
    final EncodingLevel _levelInfo[] = new EncodingLevel[ENC_ITER_MAX_LEVELS];
    ByteBuffer _buffer;
    Buffer _clientBuffer;
    TransportBuffer _clientTransportBuffer;	
	
    private boolean _isVersionSet; /* flag that tracks whether or not version is set */

    {
        _encodingLevel = -1;
        for (int i = 0; i < ENC_ITER_MAX_LEVELS; i++)
        {
            _levelInfo[i] = new EncodingLevel();
        }
    }
	
    @Override
    public void clear()
    {
        if (_writer != null)
        {
            _writer._buffer = null;
            _writer._reservedBytes = 0;
        }
        _encodingLevel = -1;
        _buffer = null;
        _isVersionSet = false;

        _clientBuffer = null;
        _clientTransportBuffer = null;
        _fieldSetDefDb = null;
        _elementSetDefDb = null;
    }
	
    @Override
    public int setBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion)
    {
        assert (buffer != null) : "buffer must be non-null";
        assert (buffer.data() != null) : "byte buffer must be non-null";

        int ret = setWriter(rwfMajorVersion, rwfMinorVersion);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        _clientBuffer = buffer;
        return setBuffer(buffer.data(), ((BufferImpl)buffer).position(), buffer.length());
    }
	
    @Override
    public int setBufferAndRWFVersion(TransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion)
    {
        assert (buffer != null) : "buffer must be non-null";

        int ret = setWriter(rwfMajorVersion, rwfMinorVersion);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        _clientTransportBuffer = buffer;

        int length = buffer.data().limit() - buffer.data().position();
        return setBuffer(buffer.data(), buffer.data().position(), length);
    }
	
    public int setGlobalFieldSetDefDb(GlobalFieldSetDefDb setDefDb)
    {
        _fieldSetDefDb = setDefDb;
        return CodecReturnCodes.SUCCESS;
    }
    
    public int setGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb)
    {
        _elementSetDefDb = setDefDb;
        return CodecReturnCodes.SUCCESS;
    }
	
    private int setWriter(int rwfMajorVersion, int rwfMinorVersion)
    {
        int ret = CodecReturnCodes.VERSION_NOT_SUPPORTED;
        for (int i = 0; i < _writerByVersion.length; i++)
        {
            // the readers are non null
            if (_writerByVersion[i].majorVersion() == rwfMajorVersion)
            {
                // for now do not check the minor version
                _writer = _writerByVersion[i];
                ret = CodecReturnCodes.SUCCESS;
                break;
            }
        }
        return ret;
    }
    
    private int setBuffer(ByteBuffer buffer, int position, int length)
    {
        if (buffer.limit() < position + length)
            return TransportReturnCodes.FAILURE;

        _startBufPos = position;
        _curBufPos = position;
        _endBufPos = length + position;
        buffer.position(position);

        _writer._buffer = buffer;
        _buffer = buffer;

        _isVersionSet = true;

        return TransportReturnCodes.SUCCESS;
    }

    /* Returns the end of the encoded content in the buffer.
     * If the position is at the start, don't know the actual encoded length,
     * so everything up to _endBufPos may be encoded content.
     * If not, assume content is encoded only up to the buffer's current position.
     */
    private int endOfEncodedBuffer()
    {
        int position = _buffer.position();
        if (position > _startBufPos)
            return position;
        else
            return _endBufPos;
    }
	
    private int realignBuffer(ByteBuffer newEncodeBuffer)
    {
        // save current buffer attributes to restore them later
        int oldPosition = _buffer.position();
        int oldLimit = _buffer.limit();

        int endBufPos = newEncodeBuffer.limit();
        int offset = newEncodeBuffer.position() - _startBufPos;
        int encodedLength = _curBufPos - _startBufPos;

        if ((newEncodeBuffer.limit() - newEncodeBuffer.position()) < _buffer.limit() - _startBufPos)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        // set the source
        _buffer.position(_startBufPos);
        _buffer.limit(_startBufPos + encodedLength);

        // modify _startBufPos to that of newEncodeBuffer
        _startBufPos = newEncodeBuffer.position();

        // copy ByteBuffer to ByteBuffer.
        newEncodeBuffer.put(_buffer);

        // restore the old buffer attributes
        _buffer.limit(oldLimit);
        _buffer.position(oldPosition);

        // realign iterator data
        _curBufPos += offset;
        _endBufPos = endBufPos;
        _buffer = newEncodeBuffer;
        _writer._buffer = _buffer;

        // modify the iterator marks
        int reservedBytes = 0;
        for (int i = 0; i <= _encodingLevel; i++)
        {
            _levelInfo[i].realighBuffer(offset);
            reservedBytes += _levelInfo[i]._reservedBytes;
        }

        // Back off endBufPos to account for any bytes reserved
        _endBufPos -= reservedBytes;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int realignBuffer(TransportBuffer newEncodeBuffer)
    {
        assert (newEncodeBuffer != null);

        return realignBuffer(newEncodeBuffer.data());
    }

    @Override
    public int realignBuffer(Buffer newEncodeBuffer)
    {
        assert (newEncodeBuffer != null);

        return realignBuffer(newEncodeBuffer.data());
    }

    @Override
    public int encodeNonRWFInit(Buffer buffer)
    {
        return Encoders.encodeNonRWFInit(this, buffer);
    }

    @Override
    public int encodeNonRWFComplete(Buffer buffer, boolean success)
    {
        return Encoders.encodeNonRWFComplete(this, buffer, success);
    }
	
    boolean isIteratorOverrun(int length)
    {
        return ((_curBufPos + length) > _endBufPos) ? true : false;
    }
	
    @Override
    public int majorVersion()
    {
        if (_isVersionSet)
        {
            return _writer.majorVersion();
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public int minorVersion()
    {
        if (_isVersionSet)
        {
            return _writer.minorVersion();
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public int replaceStreamId(int streamId)
    {
        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int streamIDPos = _startBufPos + MSG_STREAMID_POS;
        if (endPos < streamIDPos + 4)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* Store streamId as Int32 */
        _buffer.putInt(streamIDPos, streamId);

        _buffer.position(position);
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int replaceSeqNum(long seqNum)
    {
        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int ret = pointToSeqNum(endPos);
        if (ret == CodecReturnCodes.SUCCESS)
        {
            if (endPos < _buffer.position() + 4)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            // write over seqNum
            _buffer.putInt((int)seqNum);
        }

        _buffer.position(position);
        return ret;
    }
	
    private int pointToSeqNum(int endPos)
    {
        int startPos = _startBufPos;
        if (endPos < _startBufPos + 10)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        byte msgClass = _buffer.get(startPos + MSG_CLASS_POS);
        _buffer.position(startPos + MSG_FLAGS_POS);
        short flags = _writer.getUShort15rb(); // position is set to next after the flags

        switch (msgClass)
        {
            case MsgClasses.UPDATE:
                if ((flags & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 2)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 2);
                break;

            case MsgClasses.REFRESH:
                if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 1)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 1);
                break;

            case MsgClasses.GENERIC:
                if ((flags & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 1)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 1);
                break;

            case MsgClasses.POST:
                if ((flags & PostMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 9)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 9);
                break;

            case MsgClasses.ACK:
                if ((flags & AckMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 5)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 5);

                if ((flags & AckMsgFlags.HAS_NAK_CODE) > 0)
                {
                    _buffer.position(_buffer.position() + 1);
                }
                if ((flags & AckMsgFlags.HAS_TEXT) > 0)
                {
                    return skipText(endPos);
                }
                break;

            default:
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    private int skipText(int endPos)
    {
        if (endPos < _buffer.position() + 1)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        short len = _buffer.get();
        if (len < 0)
            len &= 0xFF;

        if (len == 0xFE)
        {
            if (endPos < _buffer.position() + 2)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            len = _buffer.getShort();

            if (len < 0)
                len &= 0xFFFF;
        }

        if (endPos < _buffer.position() + len)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        _buffer.position(_buffer.position() + len);
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int replaceStreamState(int streamState)
    {
        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int ret = pointToState(endPos);
        if (ret == CodecReturnCodes.SUCCESS)
        {
            if (endPos < _buffer.position() + 1)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            // write over streamState
            byte state = _buffer.get();
            state = (byte)(state & 0x07);
            state |= (streamState << 3);
            _buffer.put(_buffer.position() - 1, state);
        }

        _buffer.position(position);
        return ret;
    }

    @Override
    public int replaceDataState(int dataState)
    {
        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int ret = pointToState(endPos);
        if (ret == CodecReturnCodes.SUCCESS)
        {
            if (endPos < _buffer.position() + 1)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            // write over dataState
            byte state = _buffer.get();
            state = (byte)(state & 0xF8);
            state |= dataState;
            _buffer.put(_buffer.position() - 1, state);
        }

        _buffer.position(position);
        return ret;
    }

    @Override
    public int replaceStateCode(int stateCode)
    {
        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int ret = pointToState(endPos);
        if (ret == CodecReturnCodes.SUCCESS)
        {
            if (endPos < _buffer.position() + 2)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            // write over code
            _buffer.put(_buffer.position() + 1, (byte)stateCode);
        }

        _buffer.position(position);
        return ret;
    }

    private int pointToState(int endPos)
    {
        int startPos = _startBufPos;
        if (endPos < _startBufPos + 10)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        byte msgClass = _buffer.get(startPos + MSG_CLASS_POS);
        _buffer.position(startPos + MSG_FLAGS_POS);
        short flags = _writer.getUShort15rb(); // position is set to next after the flags

        switch (msgClass)
        {
            case MsgClasses.REFRESH:
                if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    if (endPos < _buffer.position() + 1)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    _buffer.position(_buffer.position() + 1);
                }
                else
                {
                    if (endPos < _buffer.position() + 5)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    _buffer.position(_buffer.position() + 5);
                }
                break;

            case MsgClasses.STATUS:
                if ((flags & StatusMsgFlags.HAS_STATE) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 1)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 1);
                break;

            default:
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int replaceGroupId(Buffer groupId)
    {
        assert (groupId != null);

        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int ret = pointToGroupId(endPos);
        if (ret == CodecReturnCodes.SUCCESS)
        {
            if (endPos < _buffer.position() + 1)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            int len = _buffer.get();
            if (len != groupId.length())
                return CodecReturnCodes.FAILURE;
            if (endPos < _buffer.position() + len)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            // write over groupId
            groupId.data().position(groupId.position());
            for (int i = 0; i < len; i++)
            {
                _buffer.put(groupId.data().get());
            }
        }

        _buffer.position(position);
        return ret;
    }

    private int pointToGroupId(int endPos)
    {
        int startPos = _startBufPos;
        if (endPos < _startBufPos + 10)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        byte msgClass = _buffer.get(startPos + MSG_CLASS_POS);
        _buffer.position(startPos + MSG_FLAGS_POS);
        short flags = _writer.getUShort15rb(); // position is set to next after the flags

        switch (msgClass)
        {
            case MsgClasses.REFRESH:
                if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    if (endPos < _buffer.position() + 1)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    _buffer.position(_buffer.position() + 1);
                }
                else
                {
                    if (endPos < _buffer.position() + 5)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    _buffer.position(_buffer.position() + 5);
                }
                return skipState(endPos);

            case MsgClasses.STATUS:
                if ((flags & StatusMsgFlags.HAS_GROUP_ID) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if (endPos < _buffer.position() + 1)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                _buffer.position(_buffer.position() + 1);
                if ((flags & StatusMsgFlags.HAS_STATE) > 0)
                {
                    return skipState(endPos);
                }
                break;

            default:
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }
	
    private int skipState(int endPos)
    {
        // assuming the position is on the beginning of State
        // set the position after state
        if (endPos < _buffer.position() + 3)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        _buffer.position(_buffer.position() + 2);
        return skipText(endPos);
    }

    @Override
    public int replacePostId(int postId)
    {
        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int ret = pointToPostId(endPos);
        if (ret == CodecReturnCodes.SUCCESS)
        {
            if (endPos < _buffer.position() + 4)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            // write over seqNum
            _buffer.putInt(postId);
        }

        _buffer.position(position);
        return ret;
    }

    private int pointToPostId(int endPos)
    {
        int startPos = _startBufPos;
        if (endPos < _buffer.position() + 10)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        byte msgClass = _buffer.get(startPos + MSG_CLASS_POS);
        _buffer.position(startPos + MSG_FLAGS_POS);
        short flags = _writer.getUShort15rb(); // position is set to next after the flags

        switch (msgClass)
        {
            case MsgClasses.POST:
                if ((flags & PostMsgFlags.HAS_POST_ID) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                if ((flags & PostMsgFlags.HAS_SEQ_NUM) > 0)
                {
                    if (endPos < _buffer.position() + 13)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    _buffer.position(_buffer.position() + 13);
                }
                else
                {
                    if (endPos < _buffer.position() + 9)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    _buffer.position(_buffer.position() + 9);
                }
                break;

            default:
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }
	
    private int setMsgFlag(int msgClass, int mFlag)
    {
        assert (_buffer != null) : "encodedMessageBuffer must be non-null";

        int position = _buffer.position();
        int endPos = endOfEncodedBuffer();
        int startPos = _startBufPos;
        if (endPos - _startBufPos < 9)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        byte encodedMsgClass = _buffer.get(startPos + MSG_CLASS_POS);
        if (encodedMsgClass != msgClass)
            return CodecReturnCodes.FAILURE;

        if (_buffer.get(startPos + MSG_CLASS_POS) >= 0x80)
            if (endPos < _startBufPos + 10)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

        _buffer.position(startPos + MSG_FLAGS_POS);
        short flags = _writer.getUShort15rb(); // position is set to next after the flags
        short newFlags = (short)(flags | mFlag);
        if ((flags < 0x80) && (newFlags >= 0x80))
            return CodecReturnCodes.INVALID_DATA;

        _buffer.position(startPos + MSG_FLAGS_POS);
        if (newFlags >= 0x80)
            _writer.writeUShort15rbLong(newFlags);
        else
            _buffer.put((byte)newFlags);

        _buffer.position(position);
        return CodecReturnCodes.SUCCESS;
    }

    private int unSetMsgFlag(int msgClass, int mFlag)
    {
        assert (_buffer != null) : "encodedMessageBuffer must be non-null";

        int position = _buffer.position();
        int startPos = _startBufPos;
        int endPos = endOfEncodedBuffer();
        if (endPos < _startBufPos + 9)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        byte encodedMsgClass = _buffer.get(startPos + MSG_CLASS_POS);
        if (encodedMsgClass != msgClass)
            return CodecReturnCodes.FAILURE;

        if (_buffer.get(startPos + MSG_CLASS_POS) >= 0x80)
            if (endPos < _startBufPos + 10)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

        _buffer.position(startPos + MSG_FLAGS_POS);
        short flags = _writer.getUShort15rb(); // position is set to next after the flags
        short newFlags = (short)(flags & ~mFlag);
        if ((flags >= 0x80) && (newFlags < 0x80))
            return CodecReturnCodes.INVALID_DATA;

        _buffer.position(startPos + MSG_FLAGS_POS);
        if (newFlags >= 0x80)
            _writer.writeUShort15rbLong(newFlags);
        else
            _buffer.put((byte)newFlags);

        _buffer.position(position);
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int setSolicitedFlag()
    {
        return setMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.SOLICITED);
    }

    @Override
    public int unsetSolicitedFlag()
    {
        return unSetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.SOLICITED);
    }

    @Override
    public int setRefreshCompleteFlag()
    {
        return setMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.REFRESH_COMPLETE);
    }

    @Override
    public int unsetRefreshCompleteFlag()
    {
        return unSetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.REFRESH_COMPLETE);
    }

	@Override
	public int setGenericCompleteFlag()
	{
		return setMsgFlag(MsgClasses.GENERIC, GenericMsgFlags.MESSAGE_COMPLETE);
	}

	@Override
	public int unsetGenericCompleteFlag()
	{
		return unSetMsgFlag(MsgClasses.GENERIC, GenericMsgFlags.MESSAGE_COMPLETE);
	}

    @Override
    public int setStreamingFlag()
    {
        return setMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.STREAMING);
    }

    @Override
    public int unsetStreamingFlag()
    {
        return unSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.STREAMING);
    }

    @Override
    public int setNoRefreshFlag()
    {
        return setMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.NO_REFRESH);
    }

    @Override
    public int unsetNoRefreshFlag()
    {
        return unSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.NO_REFRESH);
    }

    @Override
    public int setMsgKeyInUpdatesFlag()
    {
        return setMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.MSG_KEY_IN_UPDATES);
    }

    @Override
    public int unsetMsgKeyInUpdatesFlag()
    {
        return unSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.MSG_KEY_IN_UPDATES);
    }

    @Override
    public int setConfInfoInUpdatesFlag()
    {
        return setMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.CONF_INFO_IN_UPDATES);
    }

    @Override
    public int unsetConfInfoInUpdatesFlag()
    {
        return unSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.CONF_INFO_IN_UPDATES);
    }

    @Override
    public Buffer buffer()
    {
        return _clientBuffer;
    }

    @Override
    public TransportBuffer transportBuffer()
    {
        return _clientTransportBuffer;
    }
}
