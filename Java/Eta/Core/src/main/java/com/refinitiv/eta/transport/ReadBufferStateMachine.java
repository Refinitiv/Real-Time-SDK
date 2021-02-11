package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.ProtocolFunctions;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Objects;

import com.refinitiv.eta.codec.Codec;


//                     +------------+
//                     |A    No     |
//        +------------|    Data    |------------+
//        |            +------------+            |
//        |                  |                   |
//        |                  |                   |
//        v                  v                   v
// +-------------+      +------------+      +------------+      +--------------+
// |B Unknown /  |----->|D  Known /  |<-----|B  Known /  |<=====|C  Known /    |
// |  Incomplete |      |  Complete  |      | Incomplete |      | Insufficient |
// +-------------+      +------------+      +------------+      +--------------+
// | |    ^^  |                                  ^                    ^
// | |    ||  |                                  |                    |
// | |    ||  v                                  |                    |
// | |   +-------------=+                        |                    |
// | |   |C Unknown /   |                        |                    |
// | |   | Insufficient |                        |                    |
// | |   +--------------+                        |                    |
// | |                                           |                    |
// | |                                           |                    |
// | +-------------------------------------------+                    |
// +------------------------------------------------------------------+
// 
// +---------- Legend ---------+  +-------- Operations ------+
// |                           |  | A: rewind() + read()     |
// |  -----> Read (I/O)        |  | B: read()                |
// |  =====> Compact (Buffer)  |  | C: compact()             |
// |                           |  | D: advance()             |
// +---------------------------+  +--------------------------+


/* RsslSocketChannel::read(RsslReadArgs, RsslError) reads RIPC messages
 * from the network (and returns complete messages to the caller).
 * This class maintains the state of the "read buffer".
 */
class ReadBufferStateMachine
{
    /* Indicates the length of the current message is unknown */
    protected static final int UNKNOWN_LENGTH = -1;

    /* Represents special codes returned by the java.nio.channels.SocketChannel::read(ByteBuffer[]) method */
    protected static class ReadReturnCodes
    {
        /* No data was read from the network when java.nio.channels.SocketChannel::read(ByteBuffer[]) was invoked */
        static final int NO_DATA_READ = 0;

        /* The last invocation of java.nio.channels.SocketChannel::read(ByteBuffer[]) indicated we reached the end of the data stream */
        static final int END_OF_STREAM = -1;
    }

    /* The minimum capacity of the buffer associated with this state machine */
    protected static final int MIN_BUFFER_CAPACITY = 32; // an arbitrary (and likely very inefficient) minimum "read IO" buffer size, in bytes

    /* The current state of the state machine. */
    protected ReadBufferState _state = ReadBufferState.NO_DATA;

    /* The current sub-state of the state machine. */
    protected ReadBufferSubState _subState = ReadBufferSubState.NORMAL;

    /* When network (read) IO is performed, RIPC messages will be read into this ByteBuffer */
    protected ByteBufferPair _readIoBuffer;

    /* This reference will point to the buffer that contains the "current" data to be read by the application.
     * When we process fragmented and compressed messages, the buffer referenced by this variable may not be the same as the "read IO buffer".
     */
    protected ByteBufferPair _dataBuffer;

    /* Maps a fragment ID to a ByteBuffer providing (temporary) storage for the fragmented message data. */
    protected final Map<Integer, ByteBufferPair> _fragmentedMessages = new HashMap<Integer, ByteBufferPair>();

    /* The position denoting the start of the current RIPC message in the "read IO" buffer.
     * Note the position where the next message starts may be known, even though the total length of said message is not (yet) known.
     * For example we may known the current message starts at position {code 0}, but we may not know the length of the next message,
     * because we have not read it's length (+ flags) yet.
     */
    protected int _currentMsgStartPos = 0;

    /* The length of the current message. */
    protected int _currentMsgRipcLen = UNKNOWN_LENGTH;

    /* The RIPC flags of the current message. */
    protected int _currentMsgRipcFlags = 0;

    /* The position of the "read IO" buffer after the last java.nio.channels.SocketChannel::read(ByteBuffer) was made. */
    protected int _lastReadPosition = 0;

    /* The position of the current data. */
    protected int _dataPosition = 0;

    /* The length of the current data. */
    protected int _dataLength = 0;

    /* The fragment ID of the last reassembled fragmented message returned to the user. */
    protected int _lastReassembledFragmentId = 0;
    protected final RsslSocketChannel _SocketChannelCallBack;

    /* For message decompression. */
    protected ByteBufferPair _decompressBuffer;

    /* Compressed fragment assembly buffer. */
    private ByteBufferPair _compressedFragmentAssemblyBuffer;

    protected int _fragmentationHeaderLength = 0;

    /* A compressed fragment is defined by two ripc messages:
     * (1) The first part is indicated by RipcFlag CompFragment (0x8)
     * (2) The final part is indicated by RipcFlag CompressedData (0x4)
     * The compressed fragment can only be decompressed when the two parts have been re-assembled.
     * Since other (non-compressed) messages can be received between parts 1 and 2,
     * this flag will be used to track when the reader is waiting for the second part.
     * Sub-state cannot be used for this at this time since can be changed on a per-message basis.
     */
    private boolean _compressedFragmentWaitingForSecondMsg = false;

    /* When processing a compressed fragment sequence within a fragmented message,
     * need to track the fragment Id so that the fragment re-assembly buffer can be found
     * when handling the second part of the sequence:
     * a normal compressed message which does not contain the fragment Id.
     */
    private int _compressedFragmentFragId = 0;

    /* Holds the RIPC version. */
    private int _ripcVersion;

    /* This is the length of the FragId and will be set/changed when ripcVersion() is called.
     * FragId is two bytes in RIPC13 and beyond. FradId is one byte in pre RIPC13.
     */
    int _fragIdLen = Ripc.Offsets.FRAGMENT_ID_LENGTH_RIPC13;

    // for HTTP tunneling
    int HTTP_HEADER3 = 0;  // HTTP chunk header size for ping
    int HTTP_HEADER3_CRLF_OFFSET = 1;  // offset of CRLF in HTTP_HEADER3
    int HTTP_HEADER6 = 0;  // HTTP chunk header size for login refresh/update, directory refresh/update,
    // dictionary fragment header, dictionary fragment, item refresh/update
    int HTTP_HEADER6_CRLF_OFFSET = 4;  // offset of CRLF in HTTP_HEADER6
    int CHUNKEND_SIZE = 0; // HTTP chunk ending size, which is bytes 0x0D and 0x0A
    int _httpOverhead = 0; // (HTTP_HEADER3 or HTTP_HEADER6) + CHUNKEND_SIZE

    protected ProtocolFunctions _protocolFunctions;

    protected final byte[] _maskBytes = new byte[4];

    /* callBackChannel is an RsslSocketChannel used to access the buffer pool maintained by the RsslSocketChannel. */
    ReadBufferStateMachine(RsslSocketChannel callBackChannel)
    {
        assert (callBackChannel != null);
        _SocketChannelCallBack = callBackChannel;
    }

    /* Initializes the state machine.
     *
     * readIoBuffer is the ByteBuffer used for all all network I/O (reads).
     * The current position of this buffer must be 0.
     *
     * Throws IllegalArgumentException
     * Thrown if buffer is null or in an invalid state.
     */
    void initialize(ByteBufferPair readIoBuffer, ProtocolFunctions protocolFunctions) throws IllegalArgumentException
    {
        validateBuffer(readIoBuffer);
        _readIoBuffer = readIoBuffer;

        _state = ReadBufferState.NO_DATA;
        _subState = ReadBufferSubState.NORMAL;
        _currentMsgStartPos = 0;
        _currentMsgRipcLen = UNKNOWN_LENGTH;
        _currentMsgRipcFlags = 0;
        _lastReadPosition = 0;
        _dataBuffer = null;
        _dataPosition = 0;
        _dataLength = 0;
        _lastReassembledFragmentId = 0;

        // release the buffers back to the pool.
        for (Entry<Integer, ByteBufferPair> entrySet : _fragmentedMessages.entrySet())
        {
            ByteBufferPair buf = entrySet.getValue();
            _SocketChannelCallBack.releasePair(buf);
        }

        // clear out the _fragmentedMessages map
        _fragmentedMessages.clear();

        _protocolFunctions = protocolFunctions;
    }

    void initialize(ByteBufferPair readIoBuffer) throws IllegalArgumentException
    {
        initialize(readIoBuffer,  new RipcProtocolFunctions(_SocketChannelCallBack));
    }

    /* Returns the current state of the machine. */
    ReadBufferState state()
    {
        return _state;
    }

    /* Returns the current state of the machine */
    ReadBufferSubState subState()
    {
        return _subState;
    }

    /* Returns true if the current buffer has remaining packed data
     * ASSUMPTION: The current state is ReadBufferState.KNOWN_COMPLETE
     */
    boolean hasRemainingPackedData()
    {
        boolean retVal = false;

        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        if (_subState == ReadBufferSubState.PROCESSING_PACKED_MESSAGE)
        {
            retVal = (_dataPosition + _dataLength) != (_currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + _currentMsgRipcLen);
        }
        else if (_subState == ReadBufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
        {
            retVal = (_dataPosition + _dataLength) != _decompressBuffer.buffer().limit();
        }

        return retVal;
    }

    /* Returns the position of the read buffer after the last call to java.nio.channels.SocketChannel::read(ByteBuffer) */
    int lastReadPos()
    {
        return _lastReadPosition;
    }

    /* Transitions from the current state to the next one, because data was read from the java.nio.channels.SocketChannel
     *
     * readReturnCode is the return code from java.nio.channels.SocketChannel::read(ByteBuffer)
     *
     * Returns the new ReadBufferState
     */
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

    /* Advances the state machine because ByteBuffer::compact() was invoked on the ByteBuffer used for network IO (reads).
     *
     * Returns the new ReadBufferState.
     * ASSUMPTION: the current state is ReadBufferState.KNOWN_INSUFFICENT or ReadBufferState.UNKNOWN_INSUFFICIENT
     */
    ReadBufferState advanceOnCompact()
    {
        _lastReadPosition = _readIoBuffer.buffer().position();
        _currentMsgStartPos = 0;

        switch (_state)
        {
            case KNOWN_INSUFFICENT:
                _state = ReadBufferState.KNOWN_INCOMPLETE; // we assume compact() was called before the socket channel was read
                break;
            case UNKNOWN_INSUFFICIENT:
                _state = ReadBufferState.UNKNOWN_INCOMPLETE; // we assume compact() was called before the socket channel was read
                break;
            default:
                assert (false); // code should never reach here
                break;
        }

        return _state;
    }

    /* Advances the state machine to the next state.
     * Invoke this method *after* the user has processed a message.
     *
     * IMPORTANT: if this method returns ReadBufferState.NO_DATA,
     * the caller must rewind the buffer (invoke ByteBuffer::rewind()
     *
     * ASSUMPTION: the current state is ReadBufferState.KNOWN_COMPLETE
     *
     * Returns the current state
     */
    ReadBufferState advanceOnApplicationRead(ReadArgsImpl readArgs, Error error)
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        final int calculatedEndPos = (_currentMsgStartPos + _protocolFunctions.messageLength()) + _httpOverhead;

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
            case PROCESSING_COMPLETE_FRAGMENTED_JSON_MESSAGE:
                // return the fragmentation buffer to the pool
                if(_SocketChannelCallBack.getWsSession() != null && _SocketChannelCallBack.getWsSession().reassemblyBuffer != null)
                {
                    _SocketChannelCallBack.releasePair(_SocketChannelCallBack.getWsSession().reassemblyBuffer);
                    _SocketChannelCallBack.getWsSession().reassemblyBuffer = null;
                }
                break;
            default:
                assert (false); // code should never reach here
                break; // do nothing
        }

        // have we reached the end of the data in the buffer?
        if (calculatedEndPos == _readIoBuffer.buffer().position())
        {
            _currentMsgStartPos = 0;
            _protocolFunctions.unsetMessageLength();
            _state = ReadBufferState.NO_DATA;
            // IMPORTANT: the caller MUST invoke _readIoBuffer.rewind()
        }
        else
        {
            // Not adding an HTTP_HEADER here,
            // since the 'http' or 'encrypted' cases will use ReadBufferStateMachineHTTP::advanceOnApplicationRead()
            _currentMsgStartPos += _protocolFunctions.messageLength();
            _protocolFunctions.unsetMessageLength();
            updateStateCurrentLenUnknown(readArgs, error);
        }

        return _state;
    }

    /* Returns the position of the current RIPC or Websocket message. */
    int currentMessagePosition()
    {
        return _currentMsgStartPos;
    }

    /* Returns the length of the current RIPC or Websocket message (length includes the header)
     * ASSUMPTION: this method is only invoked when the current state is ReadBufferState.KNOWN_COMPLETE
     *
     * Returns the length of the current RIPC message (length includes the RIPC header)
     */
    int currentMessageLength()
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        return _protocolFunctions.messageLength();
    }

    /* Returns a reference to the buffer containing the current data
     * ASSUMPTION: The current state is ReadBufferState.KNOWN_COMPLETE
     *
     * Returns a reference to the buffer containing the current data
     */
    ByteBufferPair dataBuffer()
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        return _dataBuffer;
    }

    /* Returns the position of the current data
     * ASSUMPTION: The current state is ReadBufferState.KNOWN_COMPLETE
     *
     * Returns the position of the current data
     */
    int dataPosition()
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        return _dataPosition;
    }

    /* Returns the length of the current data
     * ASSUMPTION: The current state is ReadBufferState.KNOWN_COMPLETE
     *
     * Returns the length of the current data
     */
    int dataLength()
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        return _dataLength;
    }

    private void resizeReassemblyBuffer(WebSocketSession websocketSession, ByteBufferPair bufferPair, int position, int length)
    {
        // Increase the buffer's length
        websocketSession.reassemblyBufferLength *= 2;
        ByteBufferPair tempReassemblyBuffer = _SocketChannelCallBack.acquirePair(websocketSession.reassemblyBufferLength);

        websocketSession.reassemblyBuffer.buffer().limit(websocketSession.reassemblyBuffer.buffer().position());
        websocketSession.reassemblyBuffer.buffer().position(0);
        tempReassemblyBuffer.buffer().put(websocketSession.reassemblyBuffer.buffer());
        _SocketChannelCallBack.releasePair(websocketSession.reassemblyBuffer);  // Returns the old buffer back to the pool.

        websocketSession.reassemblyBuffer = tempReassemblyBuffer;
        copyMessageData(websocketSession.reassemblyBuffer, bufferPair, position, length);
        websocketSession.reassemblyBufferDataLength += length;
    }

    private void handleFragmentedJSONMessages(ReadArgsImpl readArgs, WebSocketSession websocketSession, ByteBufferPair bufferPair, int position, int length)
    {
        /* Checks whether this function needs to handle WS fragmented messages */
        if (websocketSession.wsFrameHdr.fragment || websocketSession.wsFrameHdr.opcode == WebSocketFrameParser._WS_OPC_CONT)
        {
            readArgs._uncompressedBytesRead = length;

            if(websocketSession.wsFrameHdr.fragment)
            {
                if(websocketSession.wsFrameHdr.opcode != WebSocketFrameParser._WS_OPC_CONT) /* This is the first fragmented message */
                {
                    /* preallocates buffer to assemble fragmented messages and resizes if needed. */
                    websocketSession.reassemblyBufferLength = length * 10;
                    websocketSession.reassemblyBuffer = _SocketChannelCallBack.acquirePair(websocketSession.reassemblyBufferLength);
                    copyMessageData(websocketSession.reassemblyBuffer, bufferPair, position, length);
                    websocketSession.reassemblyBufferDataLength = length;
                }
                else
                {
                    if(websocketSession.reassemblyBufferLength >= (websocketSession.reassemblyBufferDataLength + length)  )
                    {
                        copyMessageData(websocketSession.reassemblyBuffer, bufferPair, position, length);
                        websocketSession.reassemblyBufferDataLength += length;
                    }
                    else
                    {
                        resizeReassemblyBuffer(websocketSession, bufferPair, position, length);
                    }
                }

                _dataLength = 0;
                _subState = ReadBufferSubState.PROCESSING_FRAGMENTED_MESSAGE;
            }
            else
            {	/* This is the last fragmented message */

                if(websocketSession.reassemblyBufferLength >= (websocketSession.reassemblyBufferDataLength + length)  )
                {
                    copyMessageData(websocketSession.reassemblyBuffer, bufferPair, position, length);
                    websocketSession.reassemblyBufferDataLength += length;
                }
                else
                {
                    resizeReassemblyBuffer(websocketSession, bufferPair, position, length);
                }

                _dataBuffer = websocketSession.reassemblyBuffer;
                _dataBuffer.buffer().limit(_dataBuffer.buffer().position());
                _dataLength = _dataBuffer.buffer().limit();
                _dataPosition = 0;
                _subState = ReadBufferSubState.PROCESSING_COMPLETE_FRAGMENTED_JSON_MESSAGE;
            }
        }
    }

    void initializeDecompressBuffer() {
        if (Objects.isNull(_decompressBuffer)) {
            _decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
        }
    }

    void executeDecompressing(ReadArgsImpl readArgs) {
        _SocketChannelCallBack._compressor.appendCompressTrailing();
        int uncompressedLength = _SocketChannelCallBack._compressor.decompress(_readIoBuffer, _decompressBuffer, _dataPosition, _dataLength);
        _dataLength = uncompressedLength;
        _dataBuffer = _decompressBuffer;
        _dataPosition = 0; /* Set position at the beginning of the decoded compress buffer */
        readArgs._uncompressedBytesRead = _protocolFunctions.additionalHdrLength() + uncompressedLength;
    }

    /* Updates the state machine when the length of the current message is unknown. */
    private void updateStateCurrentLenUnknown(ReadArgsImpl readArgs, Error error)
    {
        /* Read additional transport header if any */
        if( _protocolFunctions.readPrependTransportHdr(readArgs._bytesRead, readArgs, error) == 1 )
        {
            return; // return to read more data from network
        }

        if (_protocolFunctions.isRWFProtocol()) // Handling for RWF protocol
        {
            assert (_currentMsgRipcLen == UNKNOWN_LENGTH);

            // Not adding an HTTP_HEADER here, since the 'http' or 'encrypted' cases will use ReadBufferStateMachineHTTP::updateStateCurrentLenUnknown()

            if (_currentMsgStartPos + _protocolFunctions.additionalHdrLength() + Ripc.Offsets.MSG_FLAG < _readIoBuffer.buffer().position())
            {
                decodeRipcHeader();

                updateStateCurrentLenKnown(readArgs, error);
            }
            else if (_currentMsgStartPos + _protocolFunctions.additionalHdrLength() + Ripc.Offsets.MSG_FLAG < _readIoBuffer.buffer().limit())
            {
                _state = ReadBufferState.UNKNOWN_INCOMPLETE;
            }
            else
            {
                _state = ReadBufferState.UNKNOWN_INSUFFICIENT;
            }
        }
        else
        { 	// Handling for JSON protocol and control frames.
            _state = ReadBufferState.KNOWN_COMPLETE;
            _subState = ReadBufferSubState.NORMAL;
            _dataLength = (int) _SocketChannelCallBack.getWsSession().wsFrameHdr.payloadLen;
            _dataPosition = _currentMsgStartPos + _SocketChannelCallBack.getWsSession().wsFrameHdr.hdrLen;
            _dataBuffer = _readIoBuffer;

            /* Checks whether the payload is compressed */
            if(_SocketChannelCallBack.getWsSession().wsFrameHdr.compressed)
            {
                if (_decompressBuffer == null)
                {
                    _decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
                }

                int uncompressedLength = _SocketChannelCallBack._compressor.decompress(_readIoBuffer, _decompressBuffer, _dataPosition, _dataLength );

                _dataLength = uncompressedLength;

                _dataBuffer = _decompressBuffer;

                _dataPosition = 0; /* Set position at the beginning of the decoded compress buffer */

                readArgs._uncompressedBytesRead = _protocolFunctions.additionalHdrLength() + uncompressedLength;

                handleFragmentedJSONMessages(readArgs, _SocketChannelCallBack.getWsSession(), _decompressBuffer, _dataPosition, _dataLength);
            }
            else
            {
                readArgs._uncompressedBytesRead = readArgs._bytesRead;

                handleFragmentedJSONMessages(readArgs, _SocketChannelCallBack.getWsSession(), _readIoBuffer, _dataPosition, _dataLength);
            }
        }
    }

    /* Updates the state machine when the length of the current message is known. */
    protected void updateStateCurrentLenKnown(ReadArgsImpl readArgs, Error error)
    {
    	/* Read additional transport header if any for JSON protocol only. */
    	if(_SocketChannelCallBack.protocolType() == Codec.JSON_PROTOCOL_TYPE)
    	{
    		_protocolFunctions.readPrependTransportHdr(readArgs._bytesRead, readArgs, error);
    	}
    	
        // calculatedEndPos is the end of the RIPC or JSON message
        // (if we have http, then calculatedEndPos is at the position of /r/n at the end of the http chunk)
        final int calculatedEndPos = (_currentMsgStartPos + HTTP_HEADER6 + _protocolFunctions.messageLength());

        if (calculatedEndPos <= _readIoBuffer.buffer().position())
        {
            _state = ReadBufferState.KNOWN_COMPLETE;

            if (_protocolFunctions.isRWFProtocol()) // Handling for RWF protocol
        	{
	            if (_currentMsgRipcLen != Ripc.Lengths.HEADER)
	            {
	                // this is NOT a ping message
	
	                if ((_currentMsgRipcFlags & Ripc.Flags.PACKING) > 0)
	                {
	                    if ((_currentMsgRipcFlags & Ripc.Flags.COMPRESSION) == 0)
	                    {
	                        readArgs._uncompressedBytesRead = _currentMsgRipcLen + _httpOverhead;
	                        _subState = ReadBufferSubState.PROCESSING_PACKED_MESSAGE;
	                    }
	                    else
	                    {
	                        _subState = ReadBufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE;
	                    }
	                    processPackedMessage(calculatedEndPos, readArgs);
	                }
	                else if ((_currentMsgRipcFlags & Ripc.Flags.HAS_OPTIONAL_FLAGS) > 0)
	                {
	                    // Only fragmented messages are using these flags.
	                    // And packed and fragmented messages are mutually exclusive
	                    // but fragmented messages can be compressed.
	                    if ((_currentMsgRipcFlags & Ripc.Flags.COMPRESSION) == 0)
	                    {
	                        _subState = ReadBufferSubState.PROCESSING_FRAGMENTED_MESSAGE;
	                    }
	                    else
	                    {
	                        _subState = ReadBufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE;
	                    }
	                    processExtendedMessage(calculatedEndPos, readArgs);
	                }
	                else if ((_currentMsgRipcFlags & Ripc.Flags.COMPRESSION) > 0)
	                {
	
	                    if ((_currentMsgRipcFlags & Ripc.Flags.COMP_FRAGMENT) == 0 && !_compressedFragmentWaitingForSecondMsg)
	                    {
	                        // normal compressed message: not part of a COMP_FRAGMENT pair
	                        processCompressedMessage(calculatedEndPos);
	                    }
	                    else
	                    // compressed fragmented message
	                    {
	                        if (_subState == ReadBufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE)
	                        {
	                            // This is not a fragment, but part of the fragment group.
	                            // The second part of the CompFragment sequence that we are waiting for has arrived.
	                            processExtendedMessageNonFragment(calculatedEndPos, readArgs);
	                        }
	                        else if (_subState == ReadBufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
	                        {
	                            processPackedMessage(calculatedEndPos, readArgs);
	                        }
	                        else
	                        {
	                            // Non-fragmented handling of CompFragment sequence:
	                            // This will handle first and last message in the sequence (CompFragment x8 followed by CompressedData x4)
	                            processCompFragmentSequence(calculatedEndPos, readArgs);
	                        }
	                    }
	                }
	                else
	                {
	                    _subState = ReadBufferSubState.NORMAL;
	                    _dataLength = _protocolFunctions.messageLength() - _protocolFunctions.entireHeaderLength();
	                    _dataPosition = _currentMsgStartPos + _protocolFunctions.entireHeaderLength() + HTTP_HEADER6;
	                    _dataBuffer = _readIoBuffer;
	
	                    readArgs._uncompressedBytesRead = _protocolFunctions.messageLength() + _httpOverhead;
	                }
	            }
	            else
	            {
	                // this is a ping message
	                _dataLength = 0;
	                _dataPosition = calculatedEndPos;
	                _subState = ReadBufferSubState.NORMAL;
	            }
        	}
            else
            {
            	// Handling for JSON protocol and control frames.
        		_state = ReadBufferState.KNOWN_COMPLETE;
        		_subState = ReadBufferSubState.NORMAL;
        		_dataLength = (int) _SocketChannelCallBack.getWsSession().wsFrameHdr.payloadLen;
        		_dataPosition = _currentMsgStartPos + _SocketChannelCallBack.getWsSession().wsFrameHdr.hdrLen;
        		_dataBuffer = _readIoBuffer;

        		/* Checks whether the payload is compressed */
        		if(_SocketChannelCallBack.getWsSession().wsFrameHdr.compressed)
        		{
        			if (_decompressBuffer == null)
        			{
        				_decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
        			}
        			
        			int uncompressedLength = _SocketChannelCallBack._compressor.decompress(_readIoBuffer, _decompressBuffer, _dataPosition, _dataLength );
        			
                    _dataLength = uncompressedLength;
                    
                    _dataBuffer = _decompressBuffer;
                    
                    _dataPosition = 0; /* Set position at the beginning of the decoded compress buffer */

                    readArgs._uncompressedBytesRead = _protocolFunctions.additionalHdrLength() + uncompressedLength;
                    
                    handleFragmentedJSONMessages(readArgs, _SocketChannelCallBack.getWsSession(), _decompressBuffer, _dataPosition, _dataLength);
        		}
        		else
        		{
        			readArgs._uncompressedBytesRead = readArgs._bytesRead;
        			
        			handleFragmentedJSONMessages(readArgs, _SocketChannelCallBack.getWsSession(), _readIoBuffer, _dataPosition, _dataLength);
        		}
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

    /* Processes the start of a packed message.
     *
     * calculatedEndPos is the calculated end position (in the read IO buffer) of the message
     */
    private void processPackedMessage(final int calculatedEndPos, ReadArgsImpl readArgs)
    {
        // read the length of the first packed message
        if (_subState == ReadBufferSubState.PROCESSING_PACKED_MESSAGE)
        {
            _dataLength = readUShort(_readIoBuffer.buffer(), _currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Lengths.HEADER);
            if (_dataLength > 0)
            {
                _dataPosition = _currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Lengths.HEADER + Ripc.Offsets.PACKED_MSG_DATA;
            }
            else
            {
                _dataPosition = calculatedEndPos;
            }
            _dataBuffer = _readIoBuffer;
        }
        else if (_subState == ReadBufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
        {
            if (_decompressBuffer == null)
            {
                _decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
            }

            final int startOfDataPos = _currentMsgStartPos + HTTP_HEADER6 + _protocolFunctions.additionalHdrLength() + Ripc.Lengths.HEADER;
            final int ripcFlags = _readIoBuffer.buffer().get(_currentMsgStartPos + HTTP_HEADER6 + _protocolFunctions.additionalHdrLength() + Ripc.Offsets.MSG_FLAG);
            if ((ripcFlags & Ripc.Flags.COMP_FRAGMENT) > 0)
            {
                compFragmentAssembly(calculatedEndPos, startOfDataPos);

                // the packed message is not yet complete, update _dataPosition with respect to _decompressBuffer
                _dataLength = 0;
                _dataPosition = _decompressBuffer.buffer().limit(); // advance to the end of the message
                _dataBuffer = _readIoBuffer;

                // Since this is the first part of a split compressed message, we only know the bytesRead.
                // Just add this part's RIPC Header to uncompressedBytesRead.
                readArgs._uncompressedBytesRead = _protocolFunctions.additionalHdrLength() + Ripc.Lengths.HEADER;
            }
            else if (_compressedFragmentWaitingForSecondMsg && (ripcFlags & Ripc.Flags.COMPRESSION) > 0)
            {
                if (compFragmentAssembly(calculatedEndPos, startOfDataPos))
                {
                    // Now have the re-assembled and de-compressed split packed message:
                    // data has been decompressed into _decompressBuffer.
                    // Proceeding with unpacking the message.
                    _dataLength = readUShort(_decompressBuffer.buffer(), 0);
                    if (_dataLength > 0)
                    {
                        _dataPosition = Ripc.Offsets.PACKED_MSG_DATA;
                    }
                    else
                    {
                        _dataPosition = _decompressBuffer.buffer().limit();
                    }
                    _dataBuffer = _decompressBuffer;

                    // Add the entire compressed message here to bytesRead,
                    // advancedToNextPackedMessage() will only update the uncompressedBytesRead,
                    // as it processes each packed message.
                    int headerLength = _protocolFunctions.additionalHdrLength() + Ripc.Lengths.HEADER;
                    readArgs._uncompressedBytesRead = headerLength + _httpOverhead + _decompressBuffer.buffer().position();
                }
            }
            else
            {
                int uncompressedLength = _SocketChannelCallBack._compressor.decompress(_readIoBuffer, _decompressBuffer, _currentMsgStartPos
                        + HTTP_HEADER6 + _protocolFunctions.additionalHdrLength() + Ripc.Lengths.HEADER, calculatedEndPos - 
                        (_currentMsgStartPos + HTTP_HEADER6 + _protocolFunctions.additionalHdrLength() + Ripc.Lengths.HEADER));
                _dataLength = readUShort(_decompressBuffer.buffer(), 0);
                if (_dataLength > 0)
                {
                    _dataPosition = Ripc.Offsets.PACKED_MSG_DATA;
                }
                else
                {
                    _dataPosition = _decompressBuffer.buffer().limit();
                }
                _dataBuffer = _decompressBuffer;

                // Add the entire compressed message here,
                // advancedToNextPackedMessage() will only update the
                // uncompressedBytesRead, as it processes each packed message.
                readArgs._uncompressedBytesRead = _protocolFunctions.additionalHdrLength() + Ripc.Lengths.HEADER + _httpOverhead + uncompressedLength;
            }
        }
    }

    /* Process the final (second) part of a CompFragment sequence within a fragmented message.
     * This part will be added to the fragment assembly buffer identified by the _compressedFragmentFragId.
     */
    private void processExtendedMessageNonFragment(final int calculatedEndPos, ReadArgsImpl readArgs)
    {
        ByteBufferPair data = null;

        assert (_compressedFragmentWaitingForSecondMsg == true);

        compFragmentAssembly(calculatedEndPos, _currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Lengths.HEADER);

        data = _fragmentedMessages.get(_compressedFragmentFragId);

        // add _decompressBuffer fragment assembly
        copyMessageData(data, _decompressBuffer, 0, _decompressBuffer.buffer().limit());

        readArgs._uncompressedBytesRead = Ripc.Lengths.HEADER + _httpOverhead + _fragmentationHeaderLength + _dataLength;

        updateFragmentHandler(calculatedEndPos, data, _compressedFragmentFragId);
    }

    private void updateFragmentHandler(final int calculatedEndPos, ByteBufferPair data, int fragmentId)
    {
        if (data != null)
        {
            // have we assembled the entire message yet?
            if (data.buffer().position() == data.buffer().limit())
            {
                _lastReassembledFragmentId = fragmentId; // we reassembled the entire message
            }
            else
            {
                data = null; // still need more fragments
            }
        }

        _dataBuffer = data;

        if (_dataBuffer != null)
        {
            // we now have a complete fragmented message
            _dataLength = _dataBuffer.buffer().limit();
            _dataPosition = 0;
            _subState = ReadBufferSubState.PROCESSING_COMPLETE_FRAGMENTED_MESSAGE;
        }
        else
        {
            // we don't have a complete fragmented message
            _dataLength = 0;
            _dataPosition = calculatedEndPos; // advance to the end of the message
            _dataBuffer = _readIoBuffer;
            if (_subState != ReadBufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE
                && _subState != ReadBufferSubState.PROCESSING_FRAGMENTED_MESSAGE)
            {
                _subState = ReadBufferSubState.NORMAL;
            }
        }
    }

    /* Process compFragment message in the normal case: no fragmentation */
    void processCompFragmentSequence(int calculatedEndPos, ReadArgsImpl readArgs)
    {
        if (!compFragmentAssembly(calculatedEndPos, _currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Lengths.HEADER))
        {
            // waiting for second part of the fragmented compression
            _dataBuffer = _readIoBuffer;
            _dataLength = 0;
            _dataPosition = calculatedEndPos; // advance to end of message

            /* _currentMsgStartPos includes RIPC Header and HTTP header,
             * remove HTTP header uncompressedBytesRead will be calculated when the split message is combined.
             */
            readArgs._uncompressedBytesRead = Ripc.Lengths.HEADER;
        }
        else
        {
            // fragmented compression is re-assembled
            _dataBuffer = _decompressBuffer;
            _dataLength = _dataBuffer.buffer().limit();
            _dataPosition = 0;
            _subState = ReadBufferSubState.NORMAL;

            readArgs._uncompressedBytesRead = Ripc.Lengths.HEADER + _dataLength + _httpOverhead;
        }
    }

    /* Processes the start of a compressed message
     *
     * calculatedEndPos is the calculated end position (in the read IO buffer) of the message
     */
    private void processCompressedMessage(final int calculatedEndPos)
    {
        _subState = ReadBufferSubState.PROCESSING_COMPRESSED_MESSAGE;
        _dataLength = _currentMsgRipcLen - Ripc.Lengths.HEADER;
        _dataPosition = _currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Lengths.HEADER;
        _dataBuffer = _readIoBuffer;
    }

    /* Re-assembles fragmented compression.
     * The first message will contain CompFrag flag (x8).
     * The second part will be a normal CompressedData flag (x4).
     * A fragmented message can occur with normal messages, or fragmented messages
     * (in which case the CompFrag flag will be combined with the fragmentation flag (x1)).
     *
     * Returns true when the second part of the fragmented compression is received and re-assembled with the first.
     * Returns false after processing the first part of the fragmented compression (compFrag flag).
     */
    private boolean compFragmentAssembly(final int calculatedEndPos, final int startOfDataPos)
    {
        if (!_compressedFragmentWaitingForSecondMsg)
        {
            // first compressed fragment
            _compressedFragmentWaitingForSecondMsg = true;

            // copy into assembly buffer and wait for next compressed fragment
            // to decompress
            if (_compressedFragmentAssemblyBuffer != null)
            {
                _compressedFragmentAssemblyBuffer.buffer().clear();
            }
            else
            {
                _compressedFragmentAssemblyBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize() + 100);
            }
            copyMessageData(_compressedFragmentAssemblyBuffer, _readIoBuffer, startOfDataPos, calculatedEndPos - startOfDataPos);

            return false;
        }
        else
        {
            // last compressed fragment
            _compressedFragmentWaitingForSecondMsg = false;

            // copy into assembly buffer and decompress entire assembled buffer
            copyMessageData(_compressedFragmentAssemblyBuffer, _readIoBuffer, startOfDataPos, calculatedEndPos - startOfDataPos);

            if (_decompressBuffer == null)
            {
                _decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
            }
            _dataLength = _SocketChannelCallBack._compressor.decompress(_compressedFragmentAssemblyBuffer, _decompressBuffer, 0,
                    _compressedFragmentAssemblyBuffer.buffer().position());

            // we now have a complete compressed fragmented message
            return true;
        }
    }

    /* Validates the specified ByteBufferPair,
     * and throws an IllegalArgumentException if said ByteBuffer is insufficient in size, or in an invalid initial state
     *
     * pair is the ByteBuffer to validate
     */
    private void validateBuffer(ByteBufferPair pair)
    {

        if (pair.buffer().capacity() < MIN_BUFFER_CAPACITY)
        {
            throw new IllegalArgumentException("The inital position of the buffer must be zero.");
        }

        if (pair.buffer().position() != 0)
        {
            throw new IllegalArgumentException("The inital position of the buffer must be zero.");
        }
    }

    /* Decodes the RIPC length.
     * Only invoke this method if RipcReadIoStateMachine::canDecodeRipcHeader() returns true
     */
    protected void decodeRipcHeader()
    {
        assert (_currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Offsets.MSG_FLAG < _readIoBuffer.buffer().position());

        _currentMsgRipcLen = readUShort(_readIoBuffer.buffer(), _currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6);
        _currentMsgRipcFlags = _readIoBuffer.buffer().get(_currentMsgStartPos + _protocolFunctions.additionalHdrLength() + HTTP_HEADER6 + Ripc.Offsets.MSG_FLAG);
    }

    /* Processes the "extended header" in the current RIPC message.
     * An extended header is present in all fragmented messages.
     * Returns a non-null ByteBufferPair if the *complete* fragmented message has been received,
     * or null if the complete fragmented message has not been received
     * (or if we cannot correctly process the fragmented message).
     * ASSUMPTION: The current state is ReadBufferState.KNOWN_COMPLETE
     *
     * calculatedEndPos is the calculated end position of the current message (in the read IO buffer)
     */
    private void processExtendedMessage(final int calculatedEndPos, ReadArgsImpl readArgs)
    {
        assert (_state == ReadBufferState.KNOWN_COMPLETE);

        ByteBufferPair data = null;

        int currentStartPos = _currentMsgStartPos + _protocolFunctions.additionalHdrLength();
        final int ripcExtendedFlags = _readIoBuffer.buffer().get(currentStartPos + HTTP_HEADER6 + Ripc.Offsets.EXTENDED_FLAGS);
        final int ripcFlags = _readIoBuffer.buffer().get(currentStartPos + HTTP_HEADER6 + Ripc.Offsets.MSG_FLAG);

        int fragmentId = 0;

        if ((ripcExtendedFlags & Ripc.Flags.Optional.FRAGMENT_HEADER) > 0)
        {
            // this message contains a fragmented header (the start of a fragmented message)
            final long totalMessageLength = readUInt(_readIoBuffer.buffer(), currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENTED_MSG_LENGTH);

            /* RIPC13 and beyond use a two byte fragId, pre RIPC13 was a one byte fragId */
            if (_ripcVersion >= Ripc.RipcVersions.VERSION13)
                fragmentId = _readIoBuffer.buffer().getShort(currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENT_HEADER_FRAGMENT_ID);
            else
                fragmentId = _readIoBuffer.buffer().get(currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENT_HEADER_FRAGMENT_ID);

            data = acquireFragmentBuffer(fragmentId, totalMessageLength);
            if (data != null)
            {
                // the capacity of the frag buffer may be larger than the frag message (to keep capacity sizes unique).
                // adjust the limit to the match the frag message size.
                data.buffer().limit((int)totalMessageLength);
            }
            int startOfDataPos = (currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENT_HEADER_FRAGMENT_ID + _fragIdLen);
            _fragmentationHeaderLength = Ripc.Lengths.FIRST_FRAGMENT_WITHOUT_FRAGID + _fragIdLen;
            if (_subState == ReadBufferSubState.PROCESSING_FRAGMENTED_MESSAGE)
            {
                copyMessageData(data, _readIoBuffer, startOfDataPos, calculatedEndPos - startOfDataPos);

                int headerAndDataLength = _fragmentationHeaderLength + (calculatedEndPos - startOfDataPos);
                readArgs._uncompressedBytesRead = headerAndDataLength + _httpOverhead;
            }
            // decompress if compressed
            else if (_subState == ReadBufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE)
            {
                if ((ripcFlags & Ripc.Flags.COMP_FRAGMENT) > 0)
                {
                    _compressedFragmentFragId = fragmentId;
                    compFragmentAssembly(calculatedEndPos, startOfDataPos);
                }
                else
                {
                    if (_decompressBuffer == null)
                    {
                        _decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
                    }

                    _SocketChannelCallBack._compressor.decompress(_readIoBuffer, _decompressBuffer,
                            startOfDataPos, calculatedEndPos - startOfDataPos);

                    copyMessageData(data, _decompressBuffer, _decompressBuffer.buffer().position(), _decompressBuffer.buffer().limit());

                    readArgs._uncompressedBytesRead = _fragmentationHeaderLength + _httpOverhead
                                                      + (_decompressBuffer.buffer().limit() - _decompressBuffer.buffer().position());
                }
            }
        }
        else if ((ripcExtendedFlags & Ripc.Flags.Optional.FRAGMENT) > 0)
        {
            // this message is a continuation of a fragmented message

            // RIPC13 and beyond use a two byte fragId, pre RIPC13 was a one byte fragId
            if (_ripcVersion >= Ripc.RipcVersions.VERSION13)
                fragmentId = _readIoBuffer.buffer().getShort(currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENT_ID);
            else
                fragmentId = _readIoBuffer.buffer().get(currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENT_ID);

            data = _fragmentedMessages.get(fragmentId); // returns null if we don't know about this fragment id
            int startOfDataPos = (currentStartPos + HTTP_HEADER6 + Ripc.Offsets.FRAGMENT_ID + _fragIdLen);
            _fragmentationHeaderLength = Ripc.Lengths.ADDITIONAL_FRAGMENT_WITHOUT_FRAGID + _fragIdLen;
            if (_subState == ReadBufferSubState.PROCESSING_FRAGMENTED_MESSAGE)
            {
                copyMessageData(data, _readIoBuffer, startOfDataPos, calculatedEndPos - startOfDataPos);

                int headerAndDataLength = _fragmentationHeaderLength + (calculatedEndPos - startOfDataPos);
                readArgs._uncompressedBytesRead = headerAndDataLength + _httpOverhead;
            }
            // decompress if compressed
            else if (_subState == ReadBufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE)
            {
                if ((ripcFlags & Ripc.Flags.COMP_FRAGMENT) > 0)
                {
                    _compressedFragmentFragId = fragmentId;
                    compFragmentAssembly(calculatedEndPos, startOfDataPos);
                }
                else
                {
                    if (_decompressBuffer == null)
                    {
                        _decompressBuffer = _SocketChannelCallBack.acquirePair(maxFragmentSize());
                    }

                    _SocketChannelCallBack._compressor.decompress(_readIoBuffer, _decompressBuffer,
                            startOfDataPos, calculatedEndPos - startOfDataPos);

                    copyMessageData(data, _decompressBuffer, _decompressBuffer.buffer().position(), _decompressBuffer.buffer().limit());

                    readArgs._uncompressedBytesRead = _fragmentationHeaderLength +
                                                      _httpOverhead +
                                                      (_decompressBuffer.buffer().limit() - _decompressBuffer.buffer().position());
                }
            }
        }

        updateFragmentHandler(calculatedEndPos, data, fragmentId);
    }

    /* Copies data from the specified src ByteBuffer to the specified dest  ByteBuffer,
     * if both are non-null, and the specified source offset and source length are valid.
     *
     * dest is the data that will be copied to this ByteBuffer, starting at the current destination ByteBuffer::position().
     *      If this parameter is null, no action will be taken.
     *
     * src is the data that will be copied from this ByteBuffer, starting from the specified offset.
     *      If this parameter is null, no action will be taken.
     *
     * srcOffset is the data that will be copied, starting from this offset in the source ByteBuffer.
     *      If this parameter exceeds the source ByteBuffer::limit(), no action will be taken.
     *
     * If there is sufficient space remaining in the destination ByteBuffer,
     * the number of bytes specified by the srcLength value will be copied from the source ByteBuffer to the destination source ByteBuffer.
     * If the source offset + the source length exceeds the limit of the source ByteBuffer, no action will be taken.
     */
    private void copyMessageData(ByteBufferPair dest, ByteBufferPair src, final int srcOffset, final int srcLength)
    {
        if (dest != null && src != null && srcOffset <= src.buffer().limit() && srcOffset + srcLength <= src.buffer().limit())
        {
            // We aim to copy the data between buffers *efficiently*.
            // The (JDK 1.7) JavaDoc for the ByteBuffer.put(ByteBuffer) method
            // states that it may (potentially) copy bytes from one ByteBuffer
            // to another more efficiently than "manually" copying the bytes
            // using other methods, BUT in order to use it, we need to adjust
            // and remember the current position and limit of the source buffer so we can restore them:

            // remember the current (source) position and limit
            int tempSrcLimit = src.buffer().limit();
            int tempSrcPos = src.buffer().position();

            // adjust the source position and limit so we only copy the current message data:
            src.buffer().position(srcOffset);
            src.buffer().limit(srcOffset + srcLength);

            dest.buffer().put(src.buffer()); // per Javadoc, potentially more efficient than a "manual copy"

            // restore (source) limit and position
            src.buffer().limit(tempSrcLimit);
            src.buffer().position(tempSrcPos);
        }
    }

    /* Returns a buffer used to store the data in a fragmented message,
     * or null if a buffer could not be acquired (i.e. because the requested length is invalid )
     *
     * fragmentId is the ID of the fragment
     *
     * totalMessageLength is the total length of fragmented message, after all the fragments are received.
     *           (This length is "just the data", and does not include the overhead of the RIPC headers.)
     *
     * Returns a buffer used to store the data in a fragmented message,
     *         or null if a buffer could not be acquired (i.e. because the requested length is invalid)
     */
    private ByteBufferPair acquireFragmentBuffer(final int fragmentId, final long totalMessageLength)
    {
        final ByteBufferPair messageData;

        // DANGER: RWF protocol states that the total message length is
        // a 32-bit unsigned integer, but we cannot allocate a single byte-buffer
        // that is larger than a (signed) Java int
        if (totalMessageLength >= 0 && totalMessageLength <= Integer.MAX_VALUE)
        {
            // find the smallest existing fragmentation buffer less than or equal to the message:
            messageData = _SocketChannelCallBack.acquirePair((int)totalMessageLength);

            if (_fragmentedMessages.containsKey(fragmentId))
            {
                // We never received all parts of the previous message with this fragment ID.
                // Return it's buffer to the pool.
                ByteBufferPair incomplete = _fragmentedMessages.remove(fragmentId);
                _SocketChannelCallBack.releasePair(incomplete);
            }

            _fragmentedMessages.put(fragmentId, messageData);
        }
        else
        {
            // we cannot fit this message in Java's ByteBuffer (see DANGER comment above)
            messageData = null;
        }

        return messageData;
    }


    /* Advances the data position to the next packed message (or to the end of the entire RIPC message)
     * ASSUMPTION: the current state is KNOWN_COMPLETE
     * ASSUMPTION: the current sub-state is PROCESSING_PACKED_MESSAGE or PROCESSING_PACKED_COMPRESSED_MESSAGE
     * ASSUMPTION: we are not already at the end of the RIPC message
     *
     * calculatedEndPos is the calculated end of the entire RIPC message
     */
    protected void advanceToNextPackedMessage(final int calculatedEndPos, ReadArgsImpl readArgs)
    {
        assert ((_state == ReadBufferState.KNOWN_COMPLETE)
                && (_subState == ReadBufferSubState.PROCESSING_PACKED_MESSAGE || _subState == ReadBufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
                && (_dataPosition != calculatedEndPos));

        // advance to the next packed message (or the end of the entire RIPC message)
        _dataPosition += _dataLength;

        // advance if we have not reached the end of the entire (RIPC) message
        if (_dataPosition != calculatedEndPos)
        {

            if (_subState == ReadBufferSubState.PROCESSING_PACKED_MESSAGE)
            {
                _dataLength = readUShort(_readIoBuffer.buffer(), _dataPosition);
            }
            else if (_subState == ReadBufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
            {
                _dataLength = readUShort(_decompressBuffer.buffer(), _dataPosition);
            }

            if (_dataLength > 0)
            {
                _dataPosition += Ripc.Offsets.PACKED_MSG_DATA;
            }
            else
            {
                _dataPosition = calculatedEndPos;
            }
        }
    }

    protected int readUShort(ByteBuffer buffer, int idx)
    {
        int val = buffer.getShort(idx);
        if (val < 0)
        {
            // convert to unsigned.
            val &= 0xFFFF;
        }
        return val;
    }

    protected long readUInt(ByteBuffer buffer, int idx)
    {
        long val = buffer.getInt(idx);
        if (val < 0)
        {
            // convert to unsigned.
            val &= 0xFFFFFFFF;
        }
        return val;
    }

    private int maxFragmentSize()
    {
        return _SocketChannelCallBack._internalMaxFragmentSize;
    }

    /* This will be called from RsslSocketChannel::initChnlWaitingConnectAck() (as a client)
     * or from RsslSocketChannel::initChnlFinishSession() (as a server).
     * It will set the _fradIdLen based on the ripcVersion.
     */
    void ripcVersion(int ripcVersion)
    {
        _ripcVersion = ripcVersion;
        if (_ripcVersion >= Ripc.RipcVersions.VERSION13)
        {
            _fragIdLen = Ripc.Offsets.FRAGMENT_ID_LENGTH_RIPC13;
        }
        else
        {
            _fragIdLen = Ripc.Offsets.FRAGMENT_ID_LENGTH_RIPC12;
        }
    }

    ProtocolFunctions protocolFunctions()
    {
        return _protocolFunctions;
    }
}

