package com.rtsdk.eta.codec;

import java.nio.ByteBuffer;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.transport.Transport;

class BufferImpl implements Buffer
{
    // This class field will be used to bypass asserts when running junits.
    static boolean _runningInJunits = false;

    /* The length of the buffer. */
    private int _length;
    /* The position of the data. */
    private int _position;
    /* The actual data represented as a ByteBuffer. */
    private ByteBuffer _data;
    /* The actual data represented as a String. */
    private String _dataString;

    boolean _isBlank;

    BufferImpl()
    {
    }

    @Override
    public int data(ByteBuffer data)
    {
        if (data != null)
        {
            data_internal(data);
            return CodecReturnCodes.SUCCESS;
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }
    
    void data_internal(ByteBuffer data)
    {
        _data = data;
        _dataString = null;
        _position = data.position();
        _length = data.limit() - _position;
        _isBlank = false;
    }

    @Override
    public int data(ByteBuffer data, int position, int length)
    {
        if (data != null)
        {
            assert (position >= 0 || _runningInJunits) : "position must be positive";
            assert (position <= data.limit() || _runningInJunits) : "position must be less than or equal to the ByteBuffer's limit";
            assert (length >= 0 || _runningInJunits) : "length must be positive";
            assert ((position + length) <= data.limit() || _runningInJunits) :
                "length + position must be less than or equal to the ByteBuffer's limit";

            if (position >= 0 && position <= data.limit() && length >= 0 && (position + length) <= data.limit())
            {
                data_internal(data, position, length);
                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    void data_internal(ByteBuffer data, int position, int length)
    {
        _data = data;
        _dataString = null;
        _position = position;
        _length = length;
        _isBlank = false;
    }

    @Override
    public int copy(ByteBuffer destBuffer)
    {
        int length = length();
        if (length != 0)
        {
            if (destBuffer != null)
            {
                assert (length <= (destBuffer.limit() - destBuffer.position()) || _runningInJunits) :
                    "destBuffer is too small, use this.length() for proper length";

                if (length <= (destBuffer.limit() - destBuffer.position()))
                {
                    // save the destBuffer's position
                    int origDesPos = destBuffer.position();

                    if (_dataString == null)
                    {
                        // ByteBuffer to ByteBuffer.
                        int origSrcPos = _data.position();
                        int origSrcLim = _data.limit();
                        _data.position(_position);
                        _data.limit(_position + length);
                        destBuffer.put(_data);
                        _data.limit(origSrcLim); // set limit first, temp limit may be less than orgSrcPos which can cause exception
                        _data.position(origSrcPos);
                    }
                    else
                    {
                        // no need to check return code since we did range checking already.
                        copyStringToByteBuffer(_dataString, destBuffer);
                    }
                    destBuffer.position(origDesPos);
                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else
        {
            // length is zero, no copy needed for blank
            return CodecReturnCodes.SUCCESS;
        }
    }

    @Override
    public int copy(byte[] destBuffer)
    {
        return copy(destBuffer, 0);
    }

    @Override
    public int copy(byte[] destBuffer, int destOffset)
    {
        int length = length();
        if (length != 0)
        {
            if (destBuffer != null)
            {
                assert (length <= (destBuffer.length - destOffset) || _runningInJunits) :
                    "destBuffer is too small, use this.length() for proper length";

                if (length <= (destBuffer.length - destOffset))
                {
                    if (_dataString == null)
                    {
                        // ByteBuffer to byte[].
                        int origSrcPos = _data.position();
                        int origSrcLim = _data.limit();
                        _data.position(_position);
                        _data.limit(_position + length);
                        _data.get(destBuffer, destOffset, length);
                        _data.limit(origSrcLim);
                        _data.position(origSrcPos);
                    }
                    else
                    {
                        // no need to check return code since we did range checking already.
                        copyStringToByteArray(_dataString, destBuffer, destOffset);
                    }
                }
                else
                {
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else
        {
            // length is zero, no copy needed for blank
            return CodecReturnCodes.SUCCESS;
        }

        return 0; // success
    }

    @Override
    public int copy(Buffer destBuffer)
    {
        int length = length();
        if (length != 0)
        {
            if (destBuffer != null)
            {
                assert (((BufferImpl)destBuffer).dataString() == null || _runningInJunits) : "destBuffer must be backed by a ByteBuffer";
                if (((BufferImpl)destBuffer).dataString() == null)
                {
                    ByteBuffer destByteBuffer = destBuffer.data();
                    assert (destByteBuffer != null || _runningInJunits) : "destBuffer's backing buffer must be non-null";
                    if (destByteBuffer != null)
                    {
                        assert (length <= destBuffer.length() || _runningInJunits) : "destBuffer is too small, use this.length() for proper length";
                        if (length <= destBuffer.length())
                        {
                            // save the destByteBuffer's position and limit
                            int origDesPos = destByteBuffer.position();
                            int origDesLim = destByteBuffer.limit();
                            destByteBuffer.position(destBuffer.position());
                            destByteBuffer.limit(destBuffer.position() + destBuffer.length());

                            if (_dataString == null)
                            {
                                // ByteBuffer to ByteBuffer.
                                int origSrcPos = _data.position();
                                int origSrcLim = _data.limit();
                                _data.limit(_position + length);
                                _data.position(_position);
                                destByteBuffer.put(_data);
                                _data.limit(origSrcLim);
                                _data.position(origSrcPos);
                            }
                            else
                            {
                                // no need to check return code since we did range checking already.
                                copyStringToByteBuffer(_dataString, destByteBuffer);
                            }

                            // restore the destByteBuffer's position and limit
                            destByteBuffer.position(origDesPos);
                            destByteBuffer.limit(origDesLim);

                            return CodecReturnCodes.SUCCESS; // success
                        }
                        else
                        {
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                    }
                    else
                    {
                        return CodecReturnCodes.INVALID_ARGUMENT;
                    }
                }
                else
                {
                    return CodecReturnCodes.INVALID_ARGUMENT;
                }
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else
        {
            // length is zero, no copy needed for blank
            return CodecReturnCodes.SUCCESS;
        }
    }

    int copyWithOrWithoutByteBuffer(Buffer destBufferInt)
    {
        BufferImpl buffer = (BufferImpl)destBufferInt;
        if (buffer._data == null || length() > buffer.length())
        {
            ByteBuffer newByteBuffer = ByteBuffer.allocate(length());
            buffer.data(newByteBuffer);
        }
        return copy(buffer);
    }

    private ByteBuffer copy()
    {
        return ByteBuffer.wrap(dataBytes());
    }

    @Override
    public int length()
    {
        int len = _length;

        if (_data != null && _data.position() > _position)
        {
            if (_data.position() - _position < len)
            {
                len = _data.position() - _position;
            }
        }

        return len;
    }

    @Override
    public int position()
    {
        return _position;
    }

    @Override
    public String toString()
    {
        String retStr = null;

        if (_data != null)
        {
            retStr = new String(dataBytes());
        }
        else if (_dataString != null)
        {
            retStr = _dataString;
        }

        return retStr;
    }

    @Override
    public int data(String str)
    {
        if (str != null)
        {
            data_internal(str);
            return CodecReturnCodes.SUCCESS;
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    void data_internal(String str)
    {
        _dataString = str;
        _data = null;
        _position = 0;
        _length = str.length();
        _isBlank = false;
    }
    
    /**
     * Append byte.
     *
     * @param b the b
     */
    /* Appends a byte to the buffer. */
    public void appendByte(byte b)
    {
        _data.put(b);
    }

    @Override
    public void clear()
    {
        _data = null;
        _dataString = null;
        _length = 0;
        _position = 0;
    }

    void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
    
    private byte[] dataBytes()
    {
        byte[] bufferBytes = new byte[_length];

        for (int i = 0; i < _length; i++)
        {
            bufferBytes[i] = dataByte(i + _position);
        }

        return bufferBytes;
    }

    /* Gets a data byte from a position in the buffer. */
    byte dataByte(int position)
    {
        byte retByte = 0;

        if (_data != null)
        {
            retByte = _data.get(position);
        }
        else if (_dataString != null)
        {
            retByte = (byte)_dataString.charAt(position);
        }

        return retByte;
    }

    @Override
    public ByteBuffer data()
    {
        ByteBuffer buffer = null;

        if (_data != null)
        {
            buffer = _data;
        }
        else if (_dataString != null)
        {
            buffer = copy();
        }

        return buffer;
    }

    String dataString()
    {
        return _dataString;
    }

    // copy the references from srcBuffer to this buffer, without creating garbage.
    void copyReferences(Buffer srcBuffer)
    {
        assert (srcBuffer != null || _runningInJunits) : "srcBuffer must be non-null";

        BufferImpl srcBuf = (BufferImpl)srcBuffer;

        _dataString = srcBuf.dataString();
        if (_dataString == null)
        {
            // backing buffer is not a String, safe to call data()
            _data = srcBuf.data();
        }
        else
        {
            _data = null;
        }

        _position = srcBuf.position();
        _length = srcBuf.length();
    }

    @Override
    public boolean equals(Buffer buffer)
    {
        if (buffer != null && buffer.length() == _length)
        {
            // determine the backing buffers, in order to perform the compare.
            BufferImpl thatBuffer = (BufferImpl)buffer;
            String thatString = thatBuffer.dataString();
            if (thatString != null)
            {
                // thatBuffer is backed by a String
                if (_dataString != null)
                    return _dataString.equals(thatString); // both Strings
                return compareByteBufferToString(_data, _position, _length, thatString);
            }
            else
            {
                // thatBuffer is backed by a ByteBuffer
                if (_dataString != null)
                    return compareByteBufferToString(thatBuffer.data(), thatBuffer.position(), thatBuffer.length(), _dataString);

                // this is backed by a ByteBuffer

                // compare byte by byte, since RsslBuffer's pos and len may be separate from ByteBuffer.
                int thisPosition = _position;
                int thatPosition = thatBuffer.position();
                ByteBuffer thatByteBuffer = thatBuffer.data();
                for (int idx = 0; idx < _length; idx++)
                {
                    if (_data.get(thisPosition + idx) != thatByteBuffer.get(thatPosition + idx))
                        return false;
                }

                return true;
            }
        }
        else
        {
            return false;
        }
    }
    
    @Override
    public boolean equals(Object buffer)
    {
        return equals((Buffer)buffer);
    }
    
    public int hashCode()
    {
        int hashCode;

        if (_dataString != null)
        {
            if (!_dataString.equals(""))
            {
                hashCode = _dataString.charAt(0) + 31;
                int multiplier = 1;
                for (int i = 1; i < _dataString.length(); ++i)
                {
                    multiplier *= 31;
                    hashCode += (_dataString.charAt(i) + 30) * multiplier;
                }
            }
            else
            {
                hashCode = _data.get(_position) + 31;
            }
        }
        else
        {
            hashCode = _data.get(_position) + 31;
            int multiplier = 1;
            for (int i = _position + 1; i < _position + _length; ++i)
            {
                multiplier *= 31;
                hashCode += (_data.get(i) + 30) * multiplier;
            }
        }

        return hashCode;
    }
    	
    private boolean compareByteBufferToString(ByteBuffer bb, int position, int length, String s)
    {
        if (length == s.length())
        {

            for (int idx = 0; idx < length; idx++)
            {
                if (bb.get(position + idx) != ((byte)(s.charAt(idx) & 0xFF)))
                    return false;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    // returns false if the ByteBuffer is too small.
    private boolean copyStringToByteBuffer(String s, ByteBuffer bb)
    {
        int len = s.length();
        if (len != 0)
        {
            if (len <= (bb.limit() - bb.position()))
            {

                for (int idx = 0; idx < len; idx++)
                {
                    bb.put(bb.position() + idx, ((byte)(s.charAt(idx) & 0xFF)));
                }

                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    }

    // returns false if the ByteBuffer is too small.
    private boolean copyStringToByteArray(String s, byte[] ba, int offset)
    {
        int len = s.length();
        if (len != 0)
        {

            if (len <= (ba.length + offset))
            {
                for (int idx = 0; idx < len; idx++)
                {
                    ba[offset + idx] = (byte)(s.charAt(idx) & 0xFF);
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    }

    @Override
    public String toHexString()
    {
        if (_data == null && _dataString == null)
            return null;

        if (_dataString == null)
            return Transport.toHexString(_data, _position, _length);
        else
        {
            // convert the _dataString into a ByteBuffer
            ByteBuffer buf = ByteBuffer.wrap(_dataString.getBytes());
            return Transport.toHexString(buf, 0, (buf.limit() - buf.position()));
        }
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeBuffer((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeBuffer(iter, this);
    }
    
    @Override
    public int capacity()
    {
        if (_data != null)
            return _data.limit() - _position;
        return _length;
    }
}
