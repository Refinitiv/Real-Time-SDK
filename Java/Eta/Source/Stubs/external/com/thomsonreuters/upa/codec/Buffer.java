package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;

/**
 * UPA Buffer represents some type of user-provided content along with the
 * content's length.
 * 
 * <p>
 * Used by data encoder/decoder and message packages. The <code>Buffer</code>
 * has a position and length. When accessing the backing data, use
 * {@link Buffer#position()} for the position and {@link Buffer#length()} for
 * the length, not the position and limit of the {@link ByteBuffer} returned
 * from {@link #data()}.Blank buffers are conveyed as a {@link Buffer#length()}
 * of 0.
 * 
 * <p>
 * {@link Buffer} can:
 * <ul>
 * <li>
 * Represent various buffer and string types, such as ASCII, RMTES, or UTF8
 * strings.</li>
 * <li>
 * Contain or reference encoded data on both container and message header
 * structures.</li>
 * </ul>
 */
public interface Buffer
{
    /**
     * Sets the Buffer data to the ByteBuffer. This buffer's position and length
     * will be taken from the specified ByteBuffer.
     * 
     * @param data the {@link java.nio.ByteBuffer ByteBuffer} data to set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success, or
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if data is null.
     */
    public int data(ByteBuffer data);

    /**
     * Sets the Buffer data to the ByteBuffer. This buffer's position and length
     * will be set to the specified position and length.
     * 
     * @param data the {@link java.nio.ByteBuffer ByteBuffer} to set.
     * @param position the data's starting position.
     * @param length the data's length.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if data is null, or if
     *         position or length is outside of the data's capacity.
     */
    public int data(ByteBuffer data, int position, int length);

    /**
     * Gets the underlying ByteBuffer. Do not use the position and limit from
     * the {@link java.nio.ByteBuffer ByteBuffer}. Use {@link #position()} and
     * {@link #length()} from the {@link Buffer}.
     * 
     * @return <ol>
     *         <li>If this Buffer is backed by a {@link java.nio.ByteBuffer
     *         ByteBuffer} then the {@link java.nio.ByteBuffer ByteBuffer}'s
     *         reference will be returned.</li>
     *         <li>If this Buffer is backed by a String, then a new
     *         {@link java.nio.ByteBuffer ByteBuffer} will be created from a new
     *         byte[] from the backing String. Note that this creates garbage.</li>
     *         </ol>
     */
    public ByteBuffer data();

    /**
     * Copies this Buffer's data starting at this Buffer's position, for this
     * Buffer's length, into the destBuffer starting at the destBuffer's position.
     * 
     * @param destBuffer A {@link java.nio.ByteBuffer ByteBuffer} large enough
     *            to hold the contents of this {@link Buffer}.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destBuffer is
     *         null, or {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the
     *         destBuffer is too small.
     * 
     * @see Buffer#length()
     */
    public int copy(ByteBuffer destBuffer);

    /**
     * Copies this Buffer's data starting at this Buffer's position, for this
     * Buffer's length, into the destBuffer.
     * 
     * @param destBuffer A byte[] large enough to hold the contents of this Buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destBuffer is
     *         null, or {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the
     *         destBuffer is too small.
     * 
     * @see Buffer#length()
     */
    public int copy(byte[] destBuffer);

    /**
     * Copies this Buffer's data starting at this Buffer's position, for this
     * Buffer's length, into the destBuffer.
     * 
     * @param destBuffer A byte[] large enough to hold the contents of this Buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destBuffer is
     *         null, or {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the
     *         destBuffer is too small.
     * 
     * @see Buffer#length()
     */
    public int copy(byte[] destBuffer, int destOffset);

    /**
     * Copies this Buffer's data starting at this Buffer's position, for this
     * Buffer's length, into the destBuffer starting at the destBuffer's
     * position.
     * 
     * @param destBuffer A {@link Buffer} backed by a
     *            {@link java.nio.ByteBuffer ByteBuffer} large enough to hold
     *            the contents of this {@link Buffer}.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destBuffer is
     *         null, the backing buffer is null or the backing buffer is a
     *         String, or {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the
     *         destBuffer is too small.
     * 
     * @see Buffer#length()
     * @see Buffer#position()
     */
    public int copy(Buffer destBuffer);

    /**
     * The backing ByteBuffer is initially set along with initial position and length. 
     * This method returns the initial length if there was no operation on the backing 
     * ByteBuffer that would change the position (such as get or put).
     * If the backing ByteBuffer position has been changed by reading or writing to 
     * the buffer, this method returns the change in position, i.e. difference between 
     * current position and initial position.
     * 
     * @return the length
     */
    public int length();

    /**
     * Returns the initial position of the buffer.
     * 
     * @return the position
     */
    public int position();

    /**
     * Sets the Buffer data to the contents of the string. This buffer's
     * position will be set to zero and length will be set to the specified string's length.
     * 
     * @param str the string to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success, or
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if data is null.
     */
    public int data(String str);

    /**
     * Converts the underlying buffer into a String. This should only be called
     * when the Buffer is known to contain ASCII data. This method creates
     * garbage unless the underlying buffer is a String.
     * 
     * @return the String representation
     */
    public String toString();

    /**
     * Converts the underlying buffer into a formatted hexidecimal String. This
     * method creates garbage.
     * 
     * @return the formatted hexadecimal String representation.
     */
    public String toHexString();

    /**
     * Clears the buffer by setting its underlying ByteBuffer to null and its
     * length and position to 0. If the underlying ByteBuffer is not referenced 
     * in any other place, it will be a subject to GC.
     */
    public void clear();
    
    /**
     * Returns true if the Buffer has been decoded as blank. 
     */
    public boolean isBlank();

    /**
     * Tests if this {@link Buffer} is equal to another {@link Buffer}. The two
     * objects are equals if they have the same length and the two sequence of
     * elements are equal.
     * <p>
     * If one buffer is backed by a String and the other buffer is backed by a
     * ByteBuffer, the String will be compared as 8-bit ASCII.
     * 
     * @param buffer A Buffer.
     * 
     * @return true if equals, otherwise false.
     */
    public boolean equals(Buffer buffer);

    /**
     * Encodes a Buffer.
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Decodes a Buffer.
     * 
     * @param iter The decoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);
    
    /**
     * Returns the capacity of this buffer. If underlying data is backed by
     * ByteBuffer, capacity is the difference between limit and initial position
     * of the backing ByteBuffer. For String backed buffer, capacity is the string length.
     * 
     * @return the capacity. Number of bytes that this buffer can hold.
     */
    public int capacity();
}