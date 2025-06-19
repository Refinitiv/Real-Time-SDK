/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

/**
 * This buffer is used by the RMTES Decoding interface to store and display the decoded strings.
 * 
 * @see RmtesCacheBuffer
 * @see RmtesDecoder
 */

public interface RmtesBuffer
{
    /**
     * Return length of RmtesBuffer
     * 
     * @return int Length
     */

    public int length();

    /**
     * set length of RmtesBuffer
     * 
     * @param x int Length
     */

    public void length(int x);

    /**
     * set CharBuffer data of RmtesBuffer
     * 
     * @param x CharBuffer Data
     * 
     * @deprecated Use data(ByteBuffer x) instead. We now leverage ByteBuffer instead of CharBuffer.
     */
    @Deprecated
    public void data(CharBuffer x);
    
    /**
     * Return a view of data of RmtesBuffer as a CharBuffer
     * 
     * @return CharBuffer Data
     * 
     * @deprecated Use byteData() instead. We now leverage ByteBuffer instead of CharBuffer.
     */
    @Deprecated
    public CharBuffer data();
    
    /**
     * Return ByteBuffer data of RmtesBuffer
     * 
     * @return ByteBuffer Data
     */

    public ByteBuffer byteData();

    /**
     * set ByteBuffer data of RmtesBuffer
     * 
     * @param x ByteBuffer Data
     */

    public void data(ByteBuffer x);

    /**
     * Return allocated Length of RmtesBuffer
     * 
     * @return int Allocated Length
     */

    public int allocatedLength();

    /**
     * set allocated length of RmtesBuffer
     * 
     * @param x int Allocated Length
     */

    public void allocatedLength(int x);

    /**
     * toString() method to return string of decoded data
     * 
     * @return String Decoded output of RmtesBuffer
     */

    public String toString();
    
    /**
     * Clears an RmtesBuffer of info in its data, keeps allocatedLength and resets length to zero.
     * 
     */
    public void clear();
}
