package com.rtsdk.eta.transport;

import java.nio.ByteBuffer;

class HTTPTransportBufferImpl extends TransportBufferImpl
{
    private int http_write_position = 6;
    private int http_chunk_end_size = 2;
	
    HTTPTransportBufferImpl()
    {
        super();
    }

    HTTPTransportBufferImpl(int size)
    {
        super(size);
    }

    HTTPTransportBufferImpl(Pool pool, int size)
    {
        super(pool, size);
    }

    HTTPTransportBufferImpl(Pool pool)
    {
        super(pool);
    }

    int getHttp_write_position()
    {
        return http_write_position;
    }

    void setHttp_write_position(int http_write_position)
    {
        this.http_write_position = http_write_position;
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
            int dataStart = 0;
            // in the first fragment the length is equal to the buffer capacity

            _data.position(dataStart + http_write_position);
            _data.putShort((short)(_data.capacity() - http_write_position - http_chunk_end_size));
            _data.put((byte)flags); // add flags in the third byte of header
            _data.put(FRAGMENT_HEADER_RIPC_FLAGS); // add Ext flags (08) in the fourth byte of header
            _data.putInt(limit); // add the length of the payload

            // add fragment ID in the ninth byte of header
            if (bigBuffer.ripcVersion() >= Ripc.RipcVersions.VERSION13)
            {
                // two byte fragId
                _data.putShort(bigBuffer.fragmentId());
            }
            else
            {
                // one byte fragId
                _data.put((byte)bigBuffer.fragmentId());
            }

            _data.limit(_data.capacity());

            bigBuffer._data.position(0);
            // update bytesCopied - leave extra bytes for compression
            bytesCopied = _data.capacity() - _firstFragmentHeaderLength - http_write_position - http_chunk_end_size;
            bigBuffer._data.limit(bytesCopied);
            // check the line below later, include http overhead?
            _length = _data.capacity();
            // update uncompressed bytes written
            ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(uncompressedBytesWritten + bytesCopied + _firstFragmentHeaderLength
                                                                + http_write_position + http_chunk_end_size);
        }
        else
        {
            _data.position(0 + http_write_position);
            int bytesToCopy = limit - position;
            if (_data.capacity() <= (bytesToCopy + _nextFragmentHeaderLength + http_write_position + http_chunk_end_size))
            {
                bytesCopied = _data.capacity() - _nextFragmentHeaderLength - http_write_position - http_chunk_end_size;
            }
            else
            {
                bytesCopied = bytesToCopy;
            }
            // check the line below later, include http overhead?
            _length = bytesCopied + _nextFragmentHeaderLength + http_write_position + http_chunk_end_size;
            _data.putShort((short)(bytesCopied + _nextFragmentHeaderLength));
            _data.put((byte)flags); // add flags in the third byte of header
            _data.put(FRAGMENT_RIPC_FLAGS); // add Ext flags (04) in the fourth byte of header

            // add fragment ID in the fifth byte of header
            if (bigBuffer.ripcVersion() >= Ripc.RipcVersions.VERSION13)
            {
                _data.putShort(bigBuffer.fragmentId()); // two byte fragId starting version 13
            }
            else
            {
                _data.put((byte)bigBuffer.fragmentId()); // one byte fragId older versions
            }

            _data.limit(_length);
            // update uncompressed bytes written
            ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(uncompressedBytesWritten + bytesCopied + _nextFragmentHeaderLength
                                                                + http_write_position + http_chunk_end_size);

            bigBuffer._data.limit(position + bytesCopied);
        }

        // copy the data from bigBuffer to this buffer
        _data.put(bigBuffer._data);

        populateHTTPOverhead();
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
     * If the input data will not fit in the fragment, returns TransportReturnCodes.FAILURE before any data is copied.
     */
    protected int populateFirstFragment(int flags, int optFlags, int fragId, int totalLength, byte[] inData, int offset, int inDataLength)
    {
        int msgLength = _firstFragmentHeaderLength + inDataLength;

        if (msgLength > _data.capacity())
            return TransportReturnCodes.FAILURE;

        _data.position(_startPosition + http_write_position);
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

        populateHTTPOverhead();

        _length = msgLength + http_write_position + http_chunk_end_size; // also: _data.position() - _startPosition;

        return (msgLength - _firstFragmentHeaderLength);
    }

    /* Populates the Nth fragment in the set (N>1) with the given data
     * 
     * flags is the flags for the RipcFlags field
     * optFlags is the flags for the RipcOptionalFlags field
     * fragId is the fragment Id for this fragment
     * inData is the array containing the source data to be added to this fragment
     * offset is the starting position of input data in the inData array
     * inDataLength is the number of bytes of input data to be added to the fragment
     * 
     * Returns the number of payload bytes copied to the fragment.
     *         If the input data will not fit in the fragment, returns TransportReturnCodes.FAILURE before any data is copied.
     */
    protected int populateNextFragment(int flags, int optFlags, int fragId, byte[] inData, int offset, int inDataLength)
    {
        int msgLength = _nextFragmentHeaderLength + inDataLength;

        if (msgLength > _data.capacity())
            return TransportReturnCodes.FAILURE;

        _data.position(_startPosition + http_write_position);
        _data.putShort((short)msgLength);
        _data.put((byte)flags); // add flags in the third byte of header
        _data.put((byte)optFlags); // add Ext flags (04) in the fourth byte of
                                   // header

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

        _length = msgLength + http_write_position + http_chunk_end_size; // also: _data.position() - _startPosition;

        populateHTTPOverhead();

        return (msgLength - _nextFragmentHeaderLength);
    }	
	
    void populateRipcHeader(int flags)
    {
        int lastPosition = _data.position();

        _length = lastPosition - _startPosition - http_write_position;
        _data.position(_startPosition + http_write_position);
        _data.putShort((short)_length); // RIPC msg length
        _data.put((byte)flags); // RIPC flag indicating data
        _data.position(lastPosition);
    }

    void populateHTTPOverhead()
    {
        int dataStart = _startPosition;
        _data.put(dataStart + 4, (byte)0x0D);
        _data.put(dataStart + 5, (byte)0x0A);

        int end = _data.position();
        int dataLength = end - dataStart - 6;

        // put data chunk length in ASCII
        byte[] dataLengthBytesTemp = Integer.toHexString(dataLength).getBytes();
        byte[] dataLengthBytes = { 0x3B, 0x3B, 0x3B, 0x3B }; // initialize (3Bh is what C++ uses, why?)
        for (int i = 0; i < dataLengthBytesTemp.length; i++)
            dataLengthBytes[i] = dataLengthBytesTemp[i];

        // length will not take up more than 4 bytes
        _data.put(dataStart, dataLengthBytes[0]);
        _data.put(dataStart + 1, dataLengthBytes[1]);
        _data.put(dataStart + 2, dataLengthBytes[2]);
        _data.put(dataStart + 3, dataLengthBytes[3]);

        // this is the /r/n at the end of HTTP chunked header
        _data.put((byte)0x0D);
        _data.put((byte)0x0A);

        int lastPosition = _data.position();
        _length = lastPosition - _startPosition;

        _data.position(dataStart);
        _data.limit(lastPosition);
    }		
	
    // should be called only if buffer is packed
    int packedLen()
    {
        return (_data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position);
    }

    @Override
    public int length()
    {
        int len = _length; // default to buffer capacity if nothing yet written

        if (_isWriteBuffer) // write buffer
        {
            if (!_isPacked) // normal buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position;
                }
            }
            else
            // packed buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position - PACKED_HDR > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position - PACKED_HDR;
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
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position;
                }
                else
                    len = 0;
            }
            else
            // packed buffer
            {
                if (_data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position - PACKED_HDR > 0)
                {
                    len = _data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position - PACKED_HDR;
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
                len = _data.position() - _startPosition - RIPC_WRITE_POSITION - http_write_position;
                srcStartPos = _startPosition + RIPC_WRITE_POSITION + http_write_position;
            }
            else
            // read buffer
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
            dataStartPos = _startPosition + RIPC_WRITE_POSITION + http_write_position;
        }
        else
        // read buffer
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
            return _data.limit() - (_startPosition + RIPC_WRITE_POSITION + http_write_position);
        }
        else
        // read buffer
        {
            return _data.limit() - _startPosition;
        }
    }
	
}
