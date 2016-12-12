package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

/**
 * This buffer is used by the RMTES Decoding interface to store data that needs
 * to be decoded, and used for partial data storage.
 * 
 * @see RmtesBuffer
 * @see RmtesDecoder
 */

public interface RmtesCacheBuffer
{
    /**
     * Return length of RmtesCacheBuffer
     * 
     * @return int Length
     */

    public int length();

    /**
     * set length of RmtesCacheBuffer
     * 
     * @param x int Length
     */

    public void length(int x);

    /**
     * set CharBuffer data of RmtesCacheBuffer
     * 
     * @param x CharBuffer Data
     * 
     * @deprecated Use data(ByteBuffer x) instead. We now leverage ByteBuffer instead of CharBuffer.
     */
    @Deprecated
    public void data(CharBuffer x);
    
    /**
     * Return a view of data of RmtesCacheBuffer as a CharBuffer
     * 
     * @return CharBuffer Data
     * 
     * @deprecated Use byteData() instead. We now leverage ByteBuffer instead of CharBuffer.
     */
    @Deprecated
    public CharBuffer data();
    
    /**
     * Return ByteBuffer data of RmtesCacheBuffer
     * 
     * @return ByteBuffer Data
     */

    public ByteBuffer byteData();

    /**
     * set ByteBuffer data of RmtesCacheBuffer
     * 
     * @param x ByteBuffer Data
     */

    public void data(ByteBuffer x);

    /**
     * Return allocated Length of RmtesCacheBuffer
     * 
     * @return int Allocated Length
     */

    public int allocatedLength();

    /**
     * set allocated length of RmtesCacheBuffer
     * 
     * @param x int Allocated Length
     */

    public void allocatedLength(int x);

    /**
     * Clears an RmtesCacheBuffer of info in its data, keeps allocatedLength and
     * resets length to zero.
     * 
     */

    public void clear();

}