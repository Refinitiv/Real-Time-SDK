/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.BufferImpl;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.transport.TransportBuffer;

class DecodeIteratorImpl implements DecodeIterator
{
    // for utility methods
    private static final int MSG_CLASS_POS = 2;
    private static final int MSG_TYPE_POS = 3;
    private static final int MSG_STREAMID_POS = 4;
    private static final int MSG_FLAGS_POS = 8;
    private static final int MSG_CLASS_SIZE = 1;
    private static final int MSG_TYPE_SIZE = 1;
    private static final int MSG_STREAMID_SIZE = 4;

    static final int DEC_ITER_MAX_LEVELS = 16;

    private final BufferReader[] _readerByVersion = new BufferReader[1]; // increase the size of the array when supporting new version
    // create all readers the current release supports and put them in the array
    {
        _readerByVersion[0] = new DataBufferReaderWireFormatV1();
        // add other readers as they are supported
    }
    BufferReader         _reader;
	
    /* Current level of decoding.
     * This is incremented when decoding a new container and decremented when _currentEntryCount==_totalEntryCount */
    int                 _decodingLevel;
	
    /* Parsing internals, current position */
    int                 _curBufPos;
	
    /* RsslBuffer which holds the ByteBuffer to decode */
    Buffer _buffer = CodecFactory.createBuffer();
	
    /* All decode iterator information */
    final DecodingLevel _levelInfo[] = new DecodingLevel[DEC_ITER_MAX_LEVELS];
	
    GlobalFieldSetDefDb _fieldSetDefDb;
    GlobalElementSetDefDb _elementSetDefDb;
	
    private boolean _isVersionSet; /* flag that tracks whether or not version is set */
    {
        _decodingLevel = -1;
        for (int i = 0; i < DEC_ITER_MAX_LEVELS; i++)
        {
            _levelInfo[i] = new DecodingLevel();
        }
    }

    @Override
    public void clear()
    {
        if (_reader != null)
        {
            _reader.clear();
        }
        _decodingLevel = -1;
        _buffer.clear();
        _isVersionSet = false;
        _fieldSetDefDb = null;
        _elementSetDefDb = null;
    }
	
    @Override
    public int setBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion)
    {
        assert (buffer != null) : "buffer must be non-null";
        assert (buffer.data() != null) : "byte buffer must be non-null";

        int ret = setReader(rwfMajorVersion, rwfMinorVersion);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return setBuffer(buffer.data(), ((BufferImpl)buffer).position(), buffer.length());
    }

    @Override
    public int setBufferAndRWFVersion(TransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion)
    {
        assert (buffer != null) : "buffer must be non-null";

        int ret = setReader(rwfMajorVersion, rwfMinorVersion);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return setBuffer(buffer.data(), buffer.dataStartPosition(), buffer.length());
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

    private int setReader(int rwfMajorVersion, int rwfMinorVersion)
    {
        int ret = CodecReturnCodes.VERSION_NOT_SUPPORTED;
        for (int i = 0; i < _readerByVersion.length; i++)
        {
            // the readers are non null
            if (_readerByVersion[i].majorVersion() == rwfMajorVersion)
            {
                // for now do not check the minor version
                _reader = _readerByVersion[i];
                ret = CodecReturnCodes.SUCCESS;
                break;
            }
        }
        return ret;
    }
    
    private int setBuffer(ByteBuffer buffer, int position, int length)
    {
        _curBufPos = position;

        int val;
        if ((val = _buffer.data(buffer, position, length)) != CodecReturnCodes.SUCCESS)
            return val; // return error code to caller.

        _reader._buffer = buffer;
        try
        {
            _reader.position(position);
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        _levelInfo[0]._endBufPos = length + position;

        _isVersionSet = true;

        return CodecReturnCodes.SUCCESS;
    }
	
    void setup(DecodingLevel _levelInfo, int cType, Object container)
    {
        _levelInfo._itemCount = 0;
        _levelInfo._nextItemPosition = 0;
        _levelInfo._nextSetPosition = 0;
        _levelInfo._containerType = cType;
        _levelInfo._listType = container;
    }

    @Override
    public int finishDecodeEntries()
    {
        Decoders.endOfList(this);

        return CodecReturnCodes.END_OF_CONTAINER;
    }

    @Override
    public int majorVersion()
    {
        if (_isVersionSet)
        {
            return _reader.majorVersion();
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
            return _reader.minorVersion();
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public int extractMsgClass()
    {
        if (MSG_CLASS_POS + MSG_CLASS_SIZE > _buffer.length())
            return CodecReturnCodes.INCOMPLETE_DATA;

        int position = _buffer.data().position();
        byte msgClass = _buffer.data().get(_buffer.position() + MSG_CLASS_POS);
        _buffer.data().position(position);
        return msgClass;
    }
	
    @Override
    public int extractDomainType()
    {
        if (MSG_TYPE_POS + MSG_TYPE_SIZE > _buffer.length())
            return CodecReturnCodes.INCOMPLETE_DATA;

        int position = _buffer.data().position();
        byte msgDomain = _buffer.data().get(_buffer.position() + MSG_TYPE_POS);
        _buffer.data().position(position);
        return msgDomain;
    }

    @Override
    public int extractStreamId()
    {
        if (MSG_STREAMID_POS + MSG_STREAMID_SIZE > _buffer.length())
            return CodecReturnCodes.INCOMPLETE_DATA;

        int position = _buffer.data().position();
        int streamId = _buffer.data().getInt(_buffer.position() + MSG_STREAMID_POS);
        _buffer.data().position(position);
        return streamId;
    }

    @Override
    public int extractSeqNum()
    {
        int position = _buffer.data().position();
        int ret = CodecReturnCodes.SUCCESS;

        try
        {
            ret = pointToSeqNum();
            if (ret == CodecReturnCodes.SUCCESS)
                ret = (int)_reader.readRelativeUnsignedInt();
        }
        catch (Exception e)
        {
            ret = CodecReturnCodes.INVALID_DATA;
        }

        _buffer.data().position(position);
        return ret;
    }
	
    private int pointToSeqNum() throws Exception
    {
        byte msgClass = _buffer.data().get(_buffer.position() + MSG_CLASS_POS);
        _buffer.data().position(_buffer.position() + MSG_FLAGS_POS);
        short flags = _reader.readRelativeUShort15rb(); // position is set to next after the flags

        switch (msgClass)
        {
            case MsgClasses.UPDATE:
                if ((flags & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                _buffer.data().position(_buffer.data().position() + 2);
                break;

            case MsgClasses.REFRESH:
                if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                _buffer.data().position(_buffer.data().position() + 1);
                break;

            case MsgClasses.GENERIC:
                if ((flags & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                _buffer.data().position(_buffer.data().position() + 1);
                break;

            case MsgClasses.POST:
                if ((flags & PostMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                _buffer.data().position(_buffer.data().position() + 9);
                break;

            case MsgClasses.ACK:
                if ((flags & AckMsgFlags.HAS_SEQ_NUM) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                _buffer.data().position(_buffer.data().position() + 5);

                if ((flags & AckMsgFlags.HAS_NAK_CODE) > 0)
                {
                    _buffer.data().position(_buffer.data().position() + 1);
                }
                if ((flags & AckMsgFlags.HAS_TEXT) > 0)
                {
                    int len = _reader.readRelativeUShort16ob();
                    _buffer.data().position(_buffer.data().position() + len);
                }
                break;

            default:
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int extractGroupId(Buffer groupId)
    {
        assert (groupId != null);

        int position = _buffer.data().position();
        int ret = CodecReturnCodes.SUCCESS;

        try
        {
            ret = pointToGroupId();
            if (ret == CodecReturnCodes.SUCCESS)
                ret = getGroupId(groupId);
        }
        catch (Exception e)
        {
            ret = CodecReturnCodes.INVALID_DATA;
        }

        _buffer.data().position(position);
        return ret;
    }
	
    private int pointToGroupId() throws Exception
    {
        byte msgClass = _buffer.data().get(_buffer.position() + MSG_CLASS_POS);
        _buffer.data().position(_buffer.position() + MSG_FLAGS_POS);
        short flags = _reader.readRelativeUShort15rb(); // position is set to next after the flags

        switch (msgClass)
        {
            case MsgClasses.STATUS:
                if ((flags & StatusMsgFlags.HAS_GROUP_ID) == 0)
                {
                    return CodecReturnCodes.FAILURE;
                }
                // skip 2 data format, update type
                _buffer.data().position(_buffer.data().position() + 1);
                if ((flags & StatusMsgFlags.HAS_STATE) > 0)
                {
                    skipState();
                }
                break;

            case MsgClasses.REFRESH:
                // skip 2 data format, refresh state
                _buffer.data().position(_buffer.data().position() + 1);
                if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                {
                    // skip 4
                    _buffer.data().position(_buffer.data().position() + 4);
                }
                skipState();
                break;

            default:
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    private void skipState() throws Exception
    {
        // assuming the position is on the beginning of State
        // set the position after state
        _buffer.data().position(_buffer.data().position() + 2);
        int len = _reader.readRelativeUShort15rb();
        _buffer.data().position(_buffer.data().position() + len);
    }

    private int getGroupId(Buffer groupId) throws Exception
    {
        // assuming the position is at the beginning of GroupId
        short groupLen = _buffer.data().get();
        if (groupLen < 0)
            groupLen &= 0xFF;

        if (groupLen > groupId.length())
        {
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        groupId.data().position(groupId.position());
        for (int i = 0; i < groupLen; i++)
        {
            groupId.data().put(_buffer.data().get());
        }

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int extractPostId()
    {
        int position = _buffer.data().position();
        int ret = CodecReturnCodes.SUCCESS;

        try
        {
            ret = pointToPostId();
            if (ret == CodecReturnCodes.SUCCESS)
                ret = (int)_reader.readRelativeUnsignedInt();
        }
        catch (Exception e)
        {
            ret = CodecReturnCodes.INVALID_DATA;
        }

        _buffer.data().position(position);
        return ret;
    }
	
    private int pointToPostId() throws Exception
    {
        byte msgClass = _buffer.data().get(_buffer.position() + MSG_CLASS_POS);
        _buffer.data().position(_buffer.position() + MSG_FLAGS_POS); // jump to flag position
        short flags = _reader.readRelativeUShort15rb(); // position is set to next after the flags

        if (msgClass != MsgClasses.POST)
            return CodecReturnCodes.INVALID_DATA;

        if ((flags & PostMsgFlags.HAS_POST_ID) == 0)
            return CodecReturnCodes.FAILURE;

        if ((flags & PostMsgFlags.HAS_SEQ_NUM) > 0)
        {
            _buffer.data().position(_buffer.data().position() + 13);
        }
        else
        {
            _buffer.data().position(_buffer.data().position() + 9);
        }
        return CodecReturnCodes.SUCCESS;
    }
}
