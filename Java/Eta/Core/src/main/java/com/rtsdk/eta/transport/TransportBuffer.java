package com.rtsdk.eta.transport;

import java.nio.ByteBuffer;

/**
 * UPA buffer used by transport layer.
 */
public interface TransportBuffer
{
    /**
     * Gets the underlying ByteBuffer.
     * 
     * @return the data
     */
    public ByteBuffer data();

    /**
     * Gets the length of the buffer.
     * If the TransportBuffer is a read buffer (i.e. a TransportBuffer that contains
     * data read from network, typically associated with DecodeIterator), the length is 
     * the difference between limit() and position() of the Buffer's ByteBuffer data.
     * If the TransportBuffer is a write buffer (i.e. a TransportBuffer that has been
     * obtained through Channel.getBuffer() call and is used to encode data, typically 
     * associated with EncodeIterator), the length is the encoded content size after
     * encoding has started. Initially (before encoding has started), the length is
     * equal to the size passed into the Channel.getBuffer() call.
     * 
     * @return the length
     */
    public int length();

    /**
     * Copies the buffer contents into memory passed by the user.
     * 
     * @param destBuffer destination buffer to copy into
     * 
     * @return {@link TransportReturnCodes} (FAILURE returned for insufficient memory)
     */
    public int copy(ByteBuffer destBuffer);
    
    /**
     * Returns the capacity of this buffer, which is the difference between
     * limit and initial position of the backing ByteBuffer.
     * 
     * @return the capacity. Maximum number of bytes that can be stored in the
     *         write buffer. In case of read only buffer, this is maximum number
     *         of bytes UPAJ fills in to the buffer.
     */
    public int capacity();

    /**
     * Returns the position where content starts in this buffer
     * 
     * @return the position where content starts in this buffer. 
     */
    public int dataStartPosition();
}
