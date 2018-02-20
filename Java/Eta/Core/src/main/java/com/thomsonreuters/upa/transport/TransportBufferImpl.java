package com.thomsonreuters.upa.transport;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

class TransportBufferImpl extends UpaNode implements TransportBuffer
{
    static final int PACKED_HDR = 2;
    static final int RIPC_WRITE_POSITION = 3;
    static final byte FRAGMENT_RIPC_FLAGS = 0x04;
    static final byte FRAGMENT_HEADER_RIPC_FLAGS = 0x08;

    /* RIPC Version dependent.
     * FragId is one-byte Pre RIPC13, two bytes RIPC13 and beyond.
     * RsslSocketChannel.BigBuffer will change these when RipcVersion is RIPC13 and beyond. */
    static int _ripcVersion = 0;
    static int _firstFragmentHeaderLength = 0;
    static int _nextFragmentHeaderLength = 0;

    /* The actual data buffer. */
    ByteBuffer _data;

    /* This position is set in the write buffer when the buffer is obtained via getBuffer method. */
    int _startPosition;

    /* The total length of the buffer (including header). */
    int _length;

    /* this will be used for packed buffers */
    boolean _isPacked = false;
    int _packedMsgLengthPosition;

    /* Remembers whether the application owns this buffer
     * (got it from getBuffer() and has not successfully written it) */
    boolean _isOwnedByApp = true;

    boolean _isWriteBuffer = false;

    TransportBufferImpl()
    {
    }

    TransportBufferImpl(int size)
    {
        _data = ByteBuffer.allocateDirect(size);
        _startPosition = 0;
        _length = size;
    }

    TransportBufferImpl(Pool pool, int size)
    {
        _data = ByteBuffer.allocateDirect(size);
        _startPosition = 0;
        pool(pool);
        _length = size;
    }

    TransportBufferImpl(Pool pool)
    {
        pool(pool);
        _isWriteBuffer = true;
    }

    void data(ByteBuffer data)
    {
        _data = data;
    }

    @Override
    public ByteBuffer data()
    {
        return _data;
    }

    boolean isBigBuffer()
    {
        return false;
    }

    /* Update the packed header with the length of the message being packed.
     * 
     * reserveNextPackedHdr is true, if this method will attempt to reserve space (i.e. the packed header) for another packed message.
     * 
     * Returns the amount of user available bytes remaining for packing.
     */
    int pack(boolean reserveNextPackedHdr, Channel chnl, Error error)
    {
        // immediately return failure if packing not enabled on buffer
        if (!_isPacked)
        {
            error.channel(chnl);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("cannot pack a buffer with packing disabled");
            return TransportReturnCodes.FAILURE;
        }

        if (_data.position() > (_packedMsgLengthPosition + PACKED_HDR))
        {
            // write the length of the message being packed
            int len = _data.position() - _packedMsgLengthPosition - PACKED_HDR;
            _data.putShort(_packedMsgLengthPosition, (short)len);

            // set new mark for the next message
            _packedMsgLengthPosition = _data.position();
        }

        if (reserveNextPackedHdr)
        {
            // attempt to reserve space for the next packed_hdr.
            if ((_packedMsgLengthPosition + PACKED_HDR) > _data.limit())
                return 0;

            _data.position(_packedMsgLengthPosition + PACKED_HDR);
        }
        else
        {
            // don't reserve space for the next packed_hdr, we're done packing.
            _data.position(_packedMsgLengthPosition);
            return 0;
        }

        return _data.limit() - _data.position();
    }

    short fragmentId()
    {
        return -1;
    }

    /* Populates a fragment when not using compression.
     * 
     * bigBuffer is the source of data to be fragmented
     * firstFragment is true if this is first fragment of the set
     * flags is the RipcFlags for the fragment
     * 
     * Returns the number of payload bytes written to this fragment
     */
    protected int populateFragment(BigBuffer bigBuffer, boolean firstFragment, int flags, WriteArgs writeArgs)
    {
        int bytesCopied;
        int position = bigBuffer._data.position();
        int limit = bigBuffer._data.limit();
        int uncompressedBytesWritten = writeArgs.uncompressedBytesWritten();
        int bytesWritten = writeArgs.bytesWritten();

        if (firstFragment)
        {
            limit = position;
            // in the first fragment the length is equal to the buffer capacity
            _data.position(0);
            _data.putShort((short)_data.capacity());
            _data.put((byte)flags); // add flags in the third byte of header
            _data.put((byte)FRAGMENT_HEADER_RIPC_FLAGS); // add Ext flags (08) in the fourth byte of header
            _data.putInt(limit); // add the length of the payload

            // add fragment ID in the ninth byte of header
            if (bigBuffer.ripcVersion() >= Ripc.RipcVersions.VERSION13)
            {
                // two byte fragId
                _data.putShort((short)bigBuffer.fragmentId());
            }
            else
            {
                // one byte fragId
                _data.put((byte)bigBuffer.fragmentId());
            }

            _data.limit(_data.capacity());

            bigBuffer._data.position(0);
            // update bytesCopied - leave extra bytes for compression
            bytesCopied = _data.capacity() - _firstFragmentHeaderLength;
            bigBuffer._data.limit(bytesCopied);
            _length = _data.capacity();

            // update uncompressed bytes written
            ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(uncompressedBytesWritten + bytesCopied + _firstFragmentHeaderLength);
        }
        else
        {
            _data.position(0);
            int bytesToCopy = limit - position;
            if (_data.capacity() <= (bytesToCopy + _nextFragmentHeaderLength))
            {
                bytesCopied = _data.capacity() - _nextFragmentHeaderLength;
            }
            else
            {
                bytesCopied = bytesToCopy;
            }

            _length = bytesCopied + _nextFragmentHeaderLength;
            _data.putShort((short)(bytesCopied + _nextFragmentHeaderLength));
            _data.put((byte)flags); // add flags in the third byte of header
            _data.put((byte)FRAGMENT_RIPC_FLAGS); // add Ext flags (04) in the fourth byte of header

            // add fragment ID in the fifth byte of header
            if (bigBuffer.ripcVersion() >= Ripc.RipcVersions.VERSION13)
            {
                _data.putShort((short)bigBuffer.fragmentId()); // two byte fragId starting version 13
            }
            else
            {
                _data.put((byte)bigBuffer.fragmentId()); // one byte fragId older versions
            }

            _data.limit(_length);
            // update uncompressed bytes written
            ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(uncompressedBytesWritten + bytesCopied + _nextFragmentHeaderLength);

            bigBuffer._data.limit(position + bytesCopied);

        }

        // copy the data from bigBuffer to this buffer
        _data.put(bigBuffer._data);
        _data.position(0);
        bigBuffer._data.limit(limit);

        // update bytes written
        ((WriteArgsImpl)writeArgs).bytesWritten(bytesWritten + _length);

        return bytesCopied;
    }

    /* Populates the first (header) fragment with the given data.
     * 
     * flags is the flags for the RipcFlags field
     * optFlags is the flags for the RipcOptionalFlags field
     * fragId is the fragment Id for this fragment
     * totalLength is the total (uncompressed) length as written in the first fragment
     * inData is the array containing the source data to be added to this fragment
     * offset is the offset in the source data array
     * inDataLength is the number of bytes to be copied from the inData array (starting from offset) and added to this fragment
     * 
     * Returns the number of payload bytes copied to the fragment.
     * Returns TransportReturnCodes.FAILURE before any data is copied, if the input data will not fit in the fragment.
     */
    protected int populateFirstFragment(int flags, int optFlags, int fragId, int totalLength, byte[] inData, int offset, int inDataLength)
    {
        int msgLength = _firstFragmentHeaderLength + inDataLength;

        if (msgLength > _data.capacity())
            return TransportReturnCodes.FAILURE;

        _data.position(_startPosition);
        _data.putShort((short)msgLength);
        _data.put((byte)flags);
        _data.put((byte)optFlags); // add Ext flags (08) in the fourth byte of header
        _data.putInt(totalLength); // add the payload length of all fragments

        // add fragment ID in the ninth byte of header
        if (_ripcVersion >= Ripc.RipcVersions.VERSION13)
        {
            _data.putShort((short)fragId);
        }
        else
        {
            _data.put((byte)fragId);
        }

        _data.put(inData, offset, inDataLength);

        _length = msgLength; // also: _data.position() - _startPosition;
        _data.position(_startPosition);
        _data.limit(_startPosition + _length);

        return (msgLength - _firstFragmentHeaderLength);
    }

    /* Populates the Nth fragment in the set (N>1) with the given data.
     * 
     * flags is the flags for the RipcFlags field
     * optFlags is the flags for the RipcOptionalFlags field
     * fragId is the fragment Id for this fragment
     * inData is the array containing the source data to be added to this fragment
     * offset is the starting position of input data in the inData array
     * inDataLength is the number of bytes of input data to be added to the fragment
     * 
     * Returns the number of payload bytes copied to the fragment.
     * Returns TransportReturnCodes.FAILURE before any data is copied, if the input data will not fit in the fragment.
     */
    protected int populateNextFragment(int flags, int optFlags, int fragId, byte[] inData, int offset, int inDataLength)
    {
        int msgLength = _nextFragmentHeaderLength + inDataLength;

        if (msgLength > _data.capacity())
            return TransportReturnCodes.FAILURE;

        _data.position(_startPosition);
        _data.putShort((short)msgLength);
        _data.put((byte)flags); // add flags in the third byte of header
        _data.put((byte)optFlags); // add Ext flags (04) in the fourth byte of header

        // add fragment ID in the fifth byte of header
        if (_ripcVersion >= Ripc.RipcVersions.VERSION13)
        {
            _data.putShort((short)fragId);
        }
        else
        {
            _data.put((byte)fragId);
        }

        _data.put(inData, offset, inDataLength);

        _length = msgLength; // also: _data.position() - _startPosition;
        _data.position(_startPosition);
        _data.limit(_startPosition + _length);

        return (msgLength - _nextFragmentHeaderLength);
    }

    void populateRipcHeader(int flags)
    {
        int lastPosition = _data.position();

        _length = lastPosition - _startPosition;
        _data.position(_startPosition);
        _data.putShort((short)_length); // RIPC msg length
        _data.put((byte)flags);         // RIPC flag indicating data
        _data.position(_startPosition);
        _data.limit(lastPosition);
    }

    /* Should be called only if buffer is packed. */
    int packedLen()
    {
        return (_data.position() - _startPosition - RIPC_WRITE_POSITION);
    }

    @Override
    public int length()
    {
        int len = _length; // default to buffer capacity if nothing yet written

        if (_isWriteBuffer) // write buffer
        {
            if (!_isPacked) // normal buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION;
                }
            }
            else
            // packed buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION - PACKED_HDR > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION - PACKED_HDR;
                }
            }
        }
        else
        // read buffer
        {
            len = _data.limit() - _data.position();
        }

        return len;
    }

    int encodedLength()
    {
        int len = _length;
        // if nothing has been written, will return 0 for write buffer

        if (_isWriteBuffer) // write buffer
        {
            if (!_isPacked) // normal buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION;
                }
                else
                    len = 0;
            }
            else
            // packed buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION - PACKED_HDR > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION - PACKED_HDR;
                }
                else
                    len = 0;
            }
        }
        else
        // read buffer
        {
            len = _data.limit() - _data.position();
        }

        return len;
    }

    @Override
    public int copy(ByteBuffer destBuffer)
    {
        int retVal = TransportReturnCodes.SUCCESS;
        int len = 0;
        int srcStartPos = 0, destStartPos = destBuffer.position();

        try
        {
            // determine length and positions
            if (_isWriteBuffer) // write buffer
            {
                len = _data.position() - _startPosition - RIPC_WRITE_POSITION;
                srcStartPos = _startPosition + RIPC_WRITE_POSITION;
            }
            else // read buffer
            {
                len = _data.limit() - _data.position();
                srcStartPos = _data.position();
            }

            // copy contents
            for (int i = 0; i < len; i++)
            {
                destBuffer.put(destStartPos + i, _data.get(srcStartPos + i));
            }
        }
        catch (Exception e)
        {
            retVal = TransportReturnCodes.FAILURE;
        }

        return retVal;
    }

    public int dataStartPosition()
    {
        int dataStartPos = 0;

        if (_isWriteBuffer) // write buffer
        {
            dataStartPos = _startPosition + RIPC_WRITE_POSITION;
        }
        else // read buffer
        {
            dataStartPos = _data.position();
        }

        return dataStartPos;
    }

    @Override
    public int capacity()
    {
        if (_isWriteBuffer) // write buffer
        {
            return _data.limit() - (_startPosition + RIPC_WRITE_POSITION);
        }
        else // read buffer
        {
            return _data.limit() - _startPosition;
        }
    }

}
