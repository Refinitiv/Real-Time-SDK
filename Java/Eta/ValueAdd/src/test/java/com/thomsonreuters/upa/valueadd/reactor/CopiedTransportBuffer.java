///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

import static org.junit.Assert.assertEquals; 
/** Used to copy the TransportBuffers of received ReactorEvents, since we cannot
 * create TransportBuffers externally. Provides very simple implementations of
 * the TransportBuffer's interfaces. 
 * This is only meant for inspecting the ReactorEvent's buffer.
 */
public class CopiedTransportBuffer implements TransportBuffer
{
    ByteBuffer _data;
    int _length;
    
    public CopiedTransportBuffer(TransportBuffer transportBuffer)
    {
        _data = ByteBuffer.allocate(transportBuffer.length());
        _length = transportBuffer.length();
        assertEquals(TransportReturnCodes.SUCCESS, transportBuffer.copy(_data));
        _data.position(0);
    }
    
    @Override
    public ByteBuffer data()
    {
        return _data;
    }

    @Override
    public int length()
    {
        return _length;
    }

    @Override
    public int copy(ByteBuffer destBuffer)
    {
        /* Simple copy, taken from Codec Buffer copy. */
        int origSrcPos = _data.position();
        int origSrcLim = _data.limit();
        _data.position(0);
        _data.limit(0 + _length);
        destBuffer.put(_data);
        _data.limit(origSrcLim); //set limit first, temp limit may be less than orgSrcPos which can cause exception
        _data.position(origSrcPos);
        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public int capacity()
    {
        return _length;
    }

    @Override
    public int dataStartPosition()
    {
        return 0;
    }

    @Override
    /** Return the buffer contents as a string. */
    public String toString()
    {
        return new String(_data.array());
    }

}
