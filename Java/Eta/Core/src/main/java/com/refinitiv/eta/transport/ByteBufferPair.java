/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

/* Represents a ByteBuffer paired with a read-only version of itself */
final class ByteBufferPair extends EtaNode
{
    private final ByteBuffer _buffer;
    private final ByteBuffer _readOnly;
    
    /* Initializes a ByteBufferPair
     * 
     * capacity is the capacity of the ByteBuffer
     * 
     * If allocateDirect is true, the ByteBuffer will be created via ByteBuffer::allocateDirect(int)
     */
    ByteBufferPair(Pool pool, int capacity, boolean allocateDirect)
    {
        _pool = pool;
        if (allocateDirect)
        {
            _buffer = ByteBuffer.allocateDirect(capacity);
        }
        else
        {
            _buffer = ByteBuffer.allocate(capacity);
        }

        _readOnly = _buffer.asReadOnlyBuffer();
    }
    
    /* Returns the (mutable) ByteBuffer in the pair */
    ByteBuffer buffer()
    {
        return _buffer;
    }

    /* Returns the read-only ByteBuffer in the pair */
    ByteBuffer readOnly()
    {
        return _readOnly;
    }

}
