///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.nio.ReadOnlyBufferException;

import org.junit.Test;

import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportBufferImpl;
import com.refinitiv.eta.transport.TransportReturnCodes;

public class TransportBufferJunit
{

    /**
     * GIVEN an RsslTransportBuffer with a read-only view of a backing ByteBuffer 
     * WHEN the backing ByteBuffer is loaded with known data. 
     * THEN RsslTransportBuffer will correctly match the view of the backing ByteBuffer.
     */
    @Test
    public void basicReadBufferTest()
    {
        final int BUFFER_SIZE = 1000;
        final int POSITION = 250;
        final int LENGTH = 500;
        ByteBuffer backingByteBuffer = ByteBuffer.allocate(BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            backingByteBuffer.put((byte)(i % 256)); // 0-255
        }
        ByteBuffer asReadOnly = backingByteBuffer.asReadOnlyBuffer();
        assertNotNull(asReadOnly);

        asReadOnly.position(POSITION);
        asReadOnly.limit(POSITION + LENGTH);

        TransportBuffer transportBuffer = new TransportBufferImpl(6144);
        ((TransportBufferImpl)transportBuffer).data(asReadOnly);

        // now the RsslTransportBuffer is set and backed by known data.

        assertEquals(asReadOnly, transportBuffer.data());
        assertEquals(POSITION, transportBuffer.data().position());
        assertEquals(LENGTH, transportBuffer.data().limit()
                - transportBuffer.data().position());

        // perform relative gets.
        for (int i = POSITION; i < LENGTH; i++)
        {
            assertEquals((byte)(i % 256), transportBuffer.data().get());
        }

        // perform absolute gets.
        for (int i = POSITION; i < LENGTH; i++)
        {
            assertEquals((byte)(i % 256), transportBuffer.data().get(i));
        }

        // the backing ByteBuffer should be read-only.
        assertTrue(transportBuffer.data().isReadOnly());

        // attempt to write to the backing ByteBuffer,
        // expect to fail since this is read-only.
        try
        {
            transportBuffer.data().put(100, (byte)127);
            assertTrue("ReadOnlyBufferException should have happened and this statement should not have been reached.",
                       false);
        }
        catch (ReadOnlyBufferException e)
        {
            // as expected.
        }
    }
    
    /**
     * GIVEN an RsslTransportBuffer with a writable view of the backing ByteBuffer
     * WHEN data is written with relative writes into the buffer
     * THEN the backing ByteBuffer will reflect the data written
     * AND writing past the length of the RsslTransportBuffer will throw
     * an exception. 
     */
    @Test
    public void basicWriteBufferRelativeTest()
    {
        final int BUFFER_SIZE = 1000;
        final int POSITION = 250;
        final int LENGTH = 500;
        ByteBuffer backingByteBuffer = ByteBuffer.allocate(BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            backingByteBuffer.put((byte)0); // ensure the contents of the
                                            // buffer.
        }
        ByteBuffer writable = backingByteBuffer.duplicate();
        assertNotNull(writable);

        writable.position(POSITION);
        writable.limit(POSITION + LENGTH);

        TransportBuffer transportBuffer = new TransportBufferImpl(6144);
        ((TransportBufferImpl)transportBuffer).data(writable);

        // now the RsslTransportBuffer is set.

        assertEquals(writable, transportBuffer.data());
        assertEquals(POSITION, transportBuffer.data().position());
        assertEquals(LENGTH, transportBuffer.data().limit()
                - transportBuffer.data().position());

        // write data into the RsslTransportBuffer, using relative writes.
        for (int i = POSITION; i < (POSITION + LENGTH); i++)
        {
            transportBuffer.data().put((byte)((i % 255) + 1)); // 1-255
        }

        // attempt to (relative) write past the set limit
        try
        {
            transportBuffer.data().put((byte)1);
            assertTrue("The ByteBuffer.put() is expected to throw a BufferOverFlowException", false);
        }
        catch (BufferOverflowException e)
        {
            // This exception is expected.
        }

        // Since relative writes were used, the transportBuffer's buffer
        // position should be at the limit.
        assertTrue(transportBuffer.data().position() == transportBuffer.data().limit());

        // verify the backing buffer contains only changes made in the
        // RsslTransportBuffer
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            if (i >= POSITION && i < (POSITION + LENGTH))
            {
                assertEquals((byte)((i % 255) + 1), backingByteBuffer.get(i));
            }
            else
            {
                assertEquals(0, backingByteBuffer.get(i));
            }
        }
    }

    /**
     * GIVEN an RsslTransportBuffer with a writable view of the backing ByteBuffer
     * WHEN data is written with absolute writes into the buffer
     * THEN the backing ByteBuffer will reflect the data written
     * AND writing past the length of the RsslTransportBuffer will throw
     * an exception. 
     */
    @Test
    public void basicWriteBufferAbsoluteTest()
    {
        final int BUFFER_SIZE = 1000;
        final int POSITION = 250;
        final int LENGTH = 500;
        ByteBuffer backingByteBuffer = ByteBuffer.allocate(BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            backingByteBuffer.put((byte)0); // ensure the contents of the
                                            // buffer.
        }
        ByteBuffer writable = backingByteBuffer.duplicate();
        assertNotNull(writable);

        writable.position(POSITION);
        writable.limit(POSITION + LENGTH);

        TransportBuffer transportBuffer = new TransportBufferImpl(6144);
        ((TransportBufferImpl)transportBuffer).data(writable);

        // now the RsslTransportBuffer is set.

        assertEquals(writable, transportBuffer.data());
        assertEquals(POSITION, transportBuffer.data().position());
        assertEquals(LENGTH, transportBuffer.data().limit()
                - transportBuffer.data().position());

        // write data into the RsslTransportBuffer, using relative writes.
        for (int i = POSITION; i < (POSITION + LENGTH); i++)
        {
            transportBuffer.data().put(i, (byte)((i % 255) + 1)); // 1-255
        }

        // attempt to (absolute) write past the set limit
        try
        {
            transportBuffer.data().put((POSITION + LENGTH), (byte)1);
            assertTrue("The ByteBuffer.put() is expected to throw a IndexOutOfBoundsException",
                       false);
        }
        catch (IndexOutOfBoundsException e)
        {
            // This exception is expected.
        }

        // Since absolute writes were used, the transportBuffer's buffer
        // position should be at the transportBuffer's position.
        assertTrue(transportBuffer.data().position() == ((TransportBufferImpl)transportBuffer)
                .data().position());

        // verify the backing buffer contains only changes made in the
        // RsslTransportBuffer
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            if (i >= POSITION && i < (POSITION + LENGTH))
            {
                assertEquals((byte)((i % 255) + 1), backingByteBuffer.get(i));
            }
            else
            {
                assertEquals(0, backingByteBuffer.get(i));
            }
        }
    }
    
    /**
     * This tests the getLength() method. It contains three test cases.<BR>
     * 
     * 1. Positive test for read buffer length<BR>
     * 2. Positive test for non-fragmented write buffer length<BR>
     * 3. Positive test for fragmented write buffer length<BR>
     */
    @Test
    public void getLengthTest()
    {
    	// 1. Positive test for read buffer length
    	
    	TransportBufferImpl readBuf = new TransportBufferImpl(100);
        assertEquals(readBuf.length(), 100);
        
    	// 2. Positive test for non-fragmented write buffer length
        
    	TransportBufferImpl writeBufNonFrag = new TransportBufferImpl(100);
    	writeBufNonFrag._isWriteBuffer = true;
    	assertEquals(writeBufNonFrag.length(), 100);
    	for (int i = 0; i < 100; i++)
    	{
    		if (i == 50)
    		{
    			assertEquals(writeBufNonFrag.length(), 47);
    		}
    		writeBufNonFrag._data.put((byte)i);
    	}
        assertEquals(writeBufNonFrag.length(), 97);
        
        // 3. Positive test for fragmented write buffer length
        
    	TransportBufferImpl writeBufFrag = new BigBuffer(null, 6145);
    	writeBufFrag._isWriteBuffer = true;
    	assertEquals(6145, writeBufFrag.length());
    	for (int i = 0; i < 6145; i++)
    	{
    		if (i == 50)
    		{
    			assertEquals(writeBufFrag.length(), 50);
    		}
    		writeBufFrag._data.put((byte)i);
    	}
        assertEquals(6145, writeBufFrag.length());
    }

    /**
     * This tests the copy() method. It contains four test cases.<BR>
     * 
     * 1. Positive test for read buffer copy<BR>
     * 2. Positive test for non-fragmented write buffer copy<BR>
     * 3. Positive test for fragmented write buffer copy<BR>
     * 4. Negative test case for insufficient memory for copy
     */
    @Test
    public void copyTest()
    {
    	// 1. Positive test for read buffer copy
    	
    	TransportBufferImpl readBuf = new TransportBufferImpl(100);
    	for (int i = 0; i < 100; i++)
    	{
    		readBuf._data.put((byte)i);
    	}
    	readBuf._data.position(0);
    	ByteBuffer readBufCopy = ByteBuffer.allocate(100);
    	assertEquals(readBuf.copy(readBufCopy), TransportReturnCodes.SUCCESS);
    	for (int i = 0; i < 100; i++)
    	{
    		assertEquals(readBufCopy.get(i), i);
    	}
        
    	// 2. Positive test for non-fragmented write buffer copy
        
    	TransportBufferImpl writeBufNonFrag = new TransportBufferImpl(100);
    	writeBufNonFrag._isWriteBuffer = true;
    	for (int i = 0; i < 100; i++)
    	{
    		writeBufNonFrag._data.put((byte)i);
    	}
    	ByteBuffer writeBufNonFragCopy = ByteBuffer.allocate(100);
    	assertEquals(writeBufNonFrag.copy(writeBufNonFragCopy), TransportReturnCodes.SUCCESS);
    	for (int i = 0; i < 97; i++)
    	{
    		assertEquals(writeBufNonFragCopy.get(i), i + 3);
    	}
        
        // 3. Positive test for fragmented write buffer copy
        
    	TransportBufferImpl writeBufFrag = new BigBuffer(null, 6145);
    	writeBufFrag._isWriteBuffer = true;
    	for (int i = 0; i < 6145; i++)
    	{
    		writeBufFrag._data.put((byte)i);
    	}
    	ByteBuffer writeBufFragCopy = ByteBuffer.allocate(6145);
    	assertEquals(writeBufFrag.copy(writeBufFragCopy), TransportReturnCodes.SUCCESS);
    	for (int i = 0; i < 6145; i++)
    	{
    		assertEquals(writeBufFragCopy.get(i), (byte)i);
    	}
    	
    	// 4. Negative test case for insufficient memory for copy
    	
    	assertEquals(writeBufFrag.copy(writeBufNonFragCopy), TransportReturnCodes.FAILURE);
	}
}
