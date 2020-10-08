///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.eta.transport.ByteBufferPair;
import com.refinitiv.eta.transport.ReadBufferState;
import com.refinitiv.eta.transport.ReadBufferStateMachine;
import com.refinitiv.eta.transport.RsslSocketChannel;

/**
 * These are some *very* basic tests of the
 * {@link ReadBufferStateMachine} to test some corner cases not handled by
 * the real JUnit tests in {@link SocketChannelJunitTest} 
 */
public class ReadBufferStateMachineJunit
{
    /**
     * Given a valid {@link ByteBuffer}
     * When I create a {@link ReadBufferStateMachine} using said {@link ByteBuffer} 
     * the initial state of the machine is "no data"
     */
    @Test
    public void createValidReadBufferStateMachine()
    {
        final RsslSocketChannel chnl = new RsslSocketChannel();
        final ReadBufferStateMachine stateMachine = new ReadBufferStateMachine(chnl);
        assertEquals(ReadBufferState.NO_DATA, stateMachine.state());
    }
        
    /**
     * Given a {@link ByteBuffer} with a capacity less than the minimum
     * required by the state machine
     * When I create a {@link ReadBufferStateMachine} using said {@link ByteBuffer} 
     * an {@link IllegalArgumentException} is thrown
     */
    @Test
    public void byteBufferLessThanMinCapacity()
    {
        try
        {
            final ByteBufferPair buf =  new ByteBufferPair(null, (ReadBufferStateMachine.MIN_BUFFER_CAPACITY - 1), true);
            
            final RsslSocketChannel chnl = new RsslSocketChannel();
            final ReadBufferStateMachine stateMachine = new ReadBufferStateMachine(chnl);
            stateMachine.initialize(buf);
            assertTrue(false); // code should never reach here
        }
        catch (IllegalArgumentException e)
        {
            assertTrue(true);
        }
    }
    
    /**
     * Given a {@link ByteBuffer} with a non-zero initial {@link ByteBuffer#position()} 
     * When I create a {@link ReadBufferStateMachine} using said {@link ByteBuffer} 
     * an {@link IllegalArgumentException} is thrown
     */
    @Test
    public void nonZeroInitalByteBufferPos()
    {
        try
        {
            final int aNonZeroInitalPosition = 1;
            final ByteBufferPair buf = new ByteBufferPair(null, ReadBufferStateMachine.MIN_BUFFER_CAPACITY, true);
            buf.buffer().position(aNonZeroInitalPosition);
            
            final RsslSocketChannel chnl = new RsslSocketChannel();
            final ReadBufferStateMachine stateMachine = new ReadBufferStateMachine(chnl);
            stateMachine.initialize(buf);
            assertTrue(false); // code should never reach here
        }
        catch (IllegalArgumentException e)
        {
            assertTrue(true);
        }
    } 

    /**
     * 
     * Given a {@link ReadBufferStateMachine} 
     * when the initial state of the machine is "no data"
     * and the first call to {@link ReadBufferStateMachine#advanceOnSocketChannelRead(int)}
     * passed the value {@code 0}
     * then the state of {@link ReadBufferStateMachine} will still be "no data"
     * 
     */
    @Test
    public void firstCallToProcessReadHasNodata()
    {
        final ByteBufferPair buf = new ByteBufferPair(null, ReadBufferStateMachine.MIN_BUFFER_CAPACITY, true);
        final ReadArgsImpl readArgs = (ReadArgsImpl) TransportFactory.createReadArgs();
        final RsslSocketChannel chnl = new RsslSocketChannel();
        final ReadBufferStateMachine stateMachine = new ReadBufferStateMachine(chnl);
        stateMachine.initialize(buf);
        assertEquals(ReadBufferState.NO_DATA, stateMachine.state());
        
        assertEquals(ReadBufferState.NO_DATA, stateMachine.advanceOnSocketChannelRead(0, readArgs));
    }
}
