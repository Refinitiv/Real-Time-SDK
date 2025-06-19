/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;


/* Needed to check what HTTP Chunk Header size (6 bytes or 3 bytes).
 * Ping message starts with 3 byte HTTP header, while all others start with 6 byte HTTP header.
 */
class ReadBufferStateMachineHTTP extends ReadBufferStateMachine
{
    protected static final int CRLF = 0x0D0A; // value of bytes for /r/n
    protected static final int CRLFCRLF = 0x0D0A0D0A; // value of bytes for /r/n/r/n

    ReadBufferStateMachineHTTP(RsslSocketChannel callBackChannel)
    {
        super(callBackChannel);

        setHTTPheaders();
    }

    /* Advances the state machine to the next state. Invoke this method *after*
     * the user has processed a message.
     * IMPORTANT: if this method returns ReadBufferState.NO_DATA, the caller must rewind the buffer (invoke ByteBuffer::rewind())
     * ASSUMPTION: the current state is ReadBufferState.KNOWN_COMPLETE
     * 
     * Returns the current state
     */
    ReadBufferState advanceOnApplicationRead(ReadArgsImpl readArgs, Error error)
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        final int calculatedEndPos = (_currentMsgStartPos + _currentMsgRipcLen) + _httpOverhead;

        assert (calculatedEndPos <= _readIoBuffer.buffer().position());
        
        switch (_subState)
        {
            case NORMAL:
                break;
            case PROCESSING_COMPRESSED_MESSAGE:
                break;
            case PROCESSING_FRAGMENTED_MESSAGE:
                break;
            case PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE:
                break;
            case PROCESSING_PACKED_MESSAGE:
                if (_dataPosition != calculatedEndPos)
                {
                    advanceToNextPackedMessage(calculatedEndPos, readArgs);
                    if (_dataPosition != calculatedEndPos)
                    {
                        return _state; // we still have more data to process
                    }
                }
                break;
            case PROCESSING_PACKED_COMPRESSED_MESSAGE:
                if (_dataPosition != _decompressBuffer.buffer().limit())
                {
                    advanceToNextPackedMessage(_decompressBuffer.buffer().limit(), readArgs);
                    if (_dataPosition != _decompressBuffer.buffer().limit())
                    {
                        return _state; // we still have more data to process
                    }
                }
                break;
            case PROCESSING_COMPLETE_FRAGMENTED_MESSAGE:
                // return the fragmentation buffer to the pool
                ByteBufferPair toReturn = _fragmentedMessages.remove(_lastReassembledFragmentId);
                _SocketChannelCallBack.releasePair(toReturn);
                break;
            default:
                assert (false); // code should never reach here
                break; // do nothing
        }

        // have we reached the end of the data in the buffer?
        if (calculatedEndPos == _readIoBuffer.buffer().position())
        {
            _currentMsgStartPos = 0;
            _currentMsgRipcLen = UNKNOWN_LENGTH;
            _state = ReadBufferState.NO_DATA;
            // IMPORTANT: the caller MUST invoke _readIoBuffer.rewind()
        }
        else
        {
            _currentMsgStartPos += (_httpOverhead + _currentMsgRipcLen);
            _currentMsgRipcLen = UNKNOWN_LENGTH;
            updateStateCurrentLenUnknown(readArgs, error);
        }

        return _state;
    }

    ReadBufferState advanceOnSocketChannelRead(int readReturnCode, ReadArgsImpl readArgs, Error error)
    {
        _lastReadPosition = _readIoBuffer.buffer().position();

        if (readReturnCode != ReadReturnCodes.END_OF_STREAM)
        {
            // update ReadArgs.bytesRead here
            readArgs._bytesRead = readReturnCode;

            // first, some housekeeping: if the previous state was KNOWN_INCOMPLETE or UNKNOWN_INCOMPLETE,
            // we ASSUME compact() was called on the read buffer before java.nio.channels.SocketChannel() read() was called,
            // and update our state accordingly
            switch (_state)
            {
                case KNOWN_INCOMPLETE:
                    if (readReturnCode != ReadReturnCodes.NO_DATA_READ)
                    {
                        updateStateCurrentLenKnown(readArgs, error);
                    }
                    break;
                case UNKNOWN_INCOMPLETE: // fall through
                case NO_DATA:
                    if (readReturnCode != ReadReturnCodes.NO_DATA_READ)
                    {
                        updateStateCurrentLenUnknown(readArgs, error); // we only update state if we actually read some data
                    }
                    break;
                default:
                    assert (false); // code should never reach here
                    break;
            }
        }
        else
        {
            _state = ReadBufferState.END_OF_STREAM;
        }

        return _state;
    }

    // case 0:
    // first check for end of 5-byte message after end of chunk in oldScktChannel during httpReconnectState
    // if not case 0, then do case 1 or case 2
    //
    // case 1:
    // the smallest possible message (i.e. ping) is
    // (3-byte ASCII chunk header length) + 2-byte RIPC header + 1-byte RIPC flag + CHUNKEND_SIZE (total: 3 + 2 +1 + 2 = 8 bytes),
    // these 8 bytes are: 0x33 0x0D 0x0A 00 03 02 0x0D 0x0D
    //
    // case 2:
    // all other messages are: (4-byte ASCII chunk header length) + 0x0D + 0x0A
    // +(whole RIPC message) + 0x0D + 0x0A
    //
    //
    // so we first check if (5th and 6th bytes are 0x0D and 0x0A),
    // if yes, then we have case 2,
    // if no,
    // then we check if (2nd and 3rd bytes are 0x0D and 0x0A) and (5th and 6th bytes are 0x00 and 0x03), if yes then we have a ping
    // if no, then we have an invalid HTTP data chunk
    private void updateStateCurrentLenUnknown(ReadArgsImpl readArgs, Error error)
    {
        assert (_currentMsgRipcLen == UNKNOWN_LENGTH);

        // System.out.println("ReadBufferStateMachineHTTP::updateStateCurrentLenUnknown() start:  "
        // +Transport.toHexString(_readIoBuffer.buffer(), _currentMsgStartPos, _readIoBuffer.buffer().position()));

        // check for end of 5-byte message after end of chunk in oldScktChannel during httpReconnectState
        // 0x30 0x0D 0x0A 0x0D 0x0A ('0'/r/n/r/n)
        if (readUInt(_readIoBuffer.buffer(), _currentMsgStartPos + 1) == CRLFCRLF)
        {// case 0 (end of chunk: 0x30 0x0D 0x0A 0x0D 0x0A)

            // if in httpReconnectState, then set flag that will be checked on next read from application
            // and then httpReconnectState will be turned off, READ_FD_CHANGE will be returned and will be reading from new scktChannel
            if (((RsslHttpSocketChannel)_SocketChannelCallBack)._httpReconnectState)
                ((RsslHttpSocketChannel)_SocketChannelCallBack).rcvEndOfResponseOldChannel = true;

            // _currentMsgStartPos will be set back later, so prepare here (see httpReconnectState logic)
            _currentMsgStartPos = -(HTTP_HEADER6 + CHUNKEND_SIZE);

            _currentMsgRipcLen = 0;
            _subState = ReadBufferSubState.NORMAL;

            return;
        }

        // Check to see if a PING can fit: 3 byte HTTP header + 3 byte RIPC header + 2 byte chunk end
        // This is 0 indexed, so we're looking for index _currentMsgStartPos+7 for the overflow.
        if ((_currentMsgStartPos + HTTP_HEADER3 + Ripc.Offsets.MSG_FLAG + CHUNKEND_SIZE) < _readIoBuffer.buffer().position())
        { 
        	
            if ((readUShort(_readIoBuffer.buffer(), _currentMsgStartPos + HTTP_HEADER3_CRLF_OFFSET) == CRLF)
                    & readUShort(_readIoBuffer.buffer(), _currentMsgStartPos + HTTP_HEADER3) == Ripc.Lengths.HEADER)
            {// case 1 (ping)
                _currentMsgRipcLen = 3;
                _currentMsgRipcFlags = _readIoBuffer.buffer().get(_currentMsgStartPos + HTTP_HEADER3 + Ripc.Offsets.MSG_FLAG);
                _httpOverhead = HTTP_HEADER3 + CHUNKEND_SIZE;
                updateStateCurrentLenKnownPossiblePing();
            }
            // For a non-ping message, the minimum number of bytes is 9:
            // HTTP_HEADER6 + 3 byte RIPC header, again 0 index so looking for index 8.
            else if((_currentMsgStartPos + HTTP_HEADER6 + Ripc.Offsets.MSG_FLAG) < _readIoBuffer.buffer().position())
            {
            	// check for all other messages
                if (readUShort(_readIoBuffer.buffer(), _currentMsgStartPos + HTTP_HEADER6_CRLF_OFFSET) == CRLF)
                {// case 2 (messages with 6-byte http header)
                    decodeRipcHeader();
                    _httpOverhead = HTTP_HEADER6 + CHUNKEND_SIZE;
                    updateStateCurrentLenKnown(readArgs, null);
                }
                else
                {
                    System.out.println("Invalid HTTP data chunk");
                    // should never get here
                }
            }
            else if (_currentMsgStartPos + HTTP_HEADER6 + Ripc.Offsets.MSG_FLAG < _readIoBuffer.buffer().limit())
            {
                _state = ReadBufferState.UNKNOWN_INCOMPLETE;
            }
            else
            {
                _state = ReadBufferState.UNKNOWN_INSUFFICIENT;
            }
        }
        else if (_currentMsgStartPos + HTTP_HEADER6 + Ripc.Offsets.MSG_FLAG < _readIoBuffer.buffer().limit())
        {
        	_state = ReadBufferState.UNKNOWN_INCOMPLETE;
        }
        else
        {
            _state = ReadBufferState.UNKNOWN_INSUFFICIENT;
        }
    }
      
    private void updateStateCurrentLenKnownPossiblePing()
    {
        final int calculatedEndPos = _currentMsgStartPos + HTTP_HEADER3 + _currentMsgRipcLen + CHUNKEND_SIZE; // 0x33 0x0D 0x0A in front
                                                                                                              // 0x0D 0x0A in the end

        if (calculatedEndPos <= _readIoBuffer.buffer().position())
        {
            _state = ReadBufferState.KNOWN_COMPLETE;

            if (_currentMsgRipcLen != Ripc.Lengths.HEADER)
            {
                // should not get here (with HTTP_HEADER3 we only have a ping, so we should have _currentMsgRipcLen set to Ripc.Lengths.HEADER
            }
            else
            {
                // this is a ping message (0x33 0x0D 0x0A 00 03 02 0x0D 0x0A)
                _dataLength = 0;
                _dataPosition = calculatedEndPos;
                _subState = ReadBufferSubState.NORMAL;
            }
        }
        else if (calculatedEndPos <= _readIoBuffer.buffer().limit())
        {
            _state = ReadBufferState.KNOWN_INCOMPLETE;
        }
        else
        {
            _state = ReadBufferState.KNOWN_INSUFFICENT;
        }
    }

    private void setHTTPheaders()
    {
        HTTP_HEADER3 = 3;
        HTTP_HEADER6 = 6;
        CHUNKEND_SIZE = 2;
    }

}

