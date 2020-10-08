///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.refinitiv.eta.transport.Lz4Compressor;
import com.refinitiv.eta.transport.TransportBufferImpl;


public class Lz4CompressorJunit
{
    static Lz4Compressor _compressor;
    static java.util.Random _gen;
    
    enum MessageContentType
    {
        UNIFORM, SEQUENCE, RANDOM
    };


    @BeforeClass
    public static void setUpBeforeClass() throws Exception
    {
        _gen = new java.util.Random(2289374);
        _compressor = new Lz4Compressor();
    }

    @AfterClass
    public static void tearDownAfterClass() throws Exception
    {
    }

    @Before
    public void setUp() throws Exception
    {
    }

    @After
    public void tearDown() throws Exception
    {
    }

    /**
     * testCompress1 compresses a small buffer with poor compression
     * results: the compression is larger than the original, so it
     * will not fit in the given buffer. This verifies that the compress 
     * method returns failure since the buffer is too small for the
     * compressed bytes.
     */
    @Test
    public final void testCompress1()
    {
        
        // Part 1: buffer size matches bytes to compress: if compression
        // grows the data, then buffer will be too small to hold result.
        // Verify compression fails.
        TransportBufferImpl tbuf = new TransportBufferImpl(20);
        tbuf.data().position(0);
        tbuf.data().limit(20);
        for (int i = 0; i < 20; i++)
        {
            // This data is not expected to compress well
            tbuf.data().put((byte)(i % 255));
        }

        int compressedBytes = _compressor.compress(tbuf, 0, 20);
        assertTrue(compressedBytes > 0);
        assertTrue(_compressor.compressedData() != null);
        assertTrue(_compressor.compressedDataLength() == compressedBytes);
        
        // Part 2: size the buffer to maximum compressed length for the 
        // given input buffer. Verify compression OK since buffer is large enough.
        int maxCompressedLen = _compressor.getMaxCompressedLength(20);
        TransportBufferImpl tbuf2 = new TransportBufferImpl(maxCompressedLen);
        tbuf2.data().position(0);
        tbuf2.data().limit(20);
        for (int i = 0; i < 20; i++)
        {
            // This data is not expected to compress well
            tbuf2.data().put((byte)(i % 255));
        }
        
        compressedBytes = _compressor.compress(tbuf2, 0, 20);
        assertTrue(compressedBytes > 0);
        assertTrue(_compressor.compressedData() != null);
        assertTrue(_compressor.compressedDataLength() == compressedBytes);

    }

    @Test
    public final void testEncodeDecodeTransportBuffer()
    {
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM,20, true, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE,20, true, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM, 6144, true, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE, 6144, true, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.RANDOM, 6000, true, 0);
        
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM,20, false, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE,20, false, 0);
        
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM, 6144, false, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE, 6144, false, 0);
        runEncodeDecodeTransportBuffer(MessageContentType.RANDOM, 6000, false, 0);

        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM,20, true, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE,20, true, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM, 6144, true, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE, 6144, true, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.RANDOM, 6000, true, 10);
        
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM,20, false, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE,20, false, 10);
        
        runEncodeDecodeTransportBuffer(MessageContentType.UNIFORM, 6144, false, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.SEQUENCE, 6144, false, 10);
        runEncodeDecodeTransportBuffer(MessageContentType.RANDOM, 6000, false, 10);

    }
    
    private final void runEncodeDecodeTransportBuffer(MessageContentType dataType, int size, boolean allocateDirect, int offset)
    {
        int maxCompressedLen = _compressor.getMaxCompressedLength(size);
        TransportBufferImpl tbuf = new TransportBufferImpl(maxCompressedLen + offset);
        TransportBufferImpl decodeBuf = new TransportBufferImpl(maxCompressedLen);
        ByteBuffer buf = getTestData(dataType, size, maxCompressedLen);
        ByteBuffer original = null;
        if (allocateDirect)
            original = ByteBuffer.allocateDirect(maxCompressedLen);
        else
            original = ByteBuffer.allocate(maxCompressedLen);
        for(int n=0; n < size; n++)
        {
            original.put(buf.get(n));
        }
        original.limit(size);

        tbuf._data.position(offset);
        for(int n=0; n < size; n++)
        {
            tbuf._data.put(buf.get(n));
        }
        
        int compressedBytes = _compressor.compress(tbuf, offset, size);
        assertTrue(compressedBytes > 0);
        assertTrue(compressedBytes == _compressor.compressedDataLength());
        
        byte[] tmpCompressedData = _compressor.compressedData();
        TransportBufferImpl compressedBuf = new TransportBufferImpl(_compressor.compressedDataLength());
        for (int i=0; i < compressedBytes; i++)
        {
            compressedBuf.data().put(tmpCompressedData[i]);
        }
        compressedBuf.data().position(0);

        original.position(0);
        int decompressedBytes = _compressor.decompress(compressedBuf, decodeBuf, compressedBytes);
        assertTrue(decompressedBytes == size);
        assertTrue(original.compareTo(decodeBuf.data()) == 0);
    }
    
    @Test
    public final void testEncodeDecodeByteBufferPair()
    {
        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 20, true, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 20, true, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 6144, true, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 6144, true, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.RANDOM, 6000, true, 0);


        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 20, false, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 20, false, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 6144, false, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 6144, false, 0);
        
        runEncodeDecodeByteBufferPair(MessageContentType.RANDOM, 6000, false, 0);
        

        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 20, true, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 20, true, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 6144, true, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 6144, true, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.RANDOM, 6000, true, 10);


        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 20, false, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 20, false, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.UNIFORM, 6144, false, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.SEQUENCE, 6144, false, 10);
        
        runEncodeDecodeByteBufferPair(MessageContentType.RANDOM, 6000, false, 10);

    }
    
    
    private final void runEncodeDecodeByteBufferPair(MessageContentType dataType, int size, boolean allocateDirect, int offset)
    {
        int maxCompressedLen = _compressor.getMaxCompressedLength(size);
        ByteBufferPair pairin = new ByteBufferPair(null, maxCompressedLen, true);
        ByteBufferPair pairout = new ByteBufferPair(null, maxCompressedLen, true);

        ByteBufferPair tocompress = new ByteBufferPair(null, maxCompressedLen + offset, allocateDirect);
        ByteBuffer buf = getTestData(dataType, size, maxCompressedLen);
        ByteBuffer original = null;
        if (allocateDirect == true)
            original = ByteBuffer.allocate(maxCompressedLen);
        else
            original = ByteBuffer.allocateDirect(maxCompressedLen);
        
        for(int n=0; n < size; n++)
        {
            original.put(buf.get(n));
        }
        original.limit(size);
        
        tocompress.buffer().position(offset);
        for(int n=0; n < size; n++)
        {
            tocompress.buffer().put(buf.get(n));
        }
        
        int compressedBytes = _compressor.compress(tocompress.buffer(), offset, size);
        System.out.println("In=" + size + " Compressed-out:" + compressedBytes);
        assertTrue(compressedBytes > 0);
        
        byte[] tmpCompressedData = _compressor.compressedData();
        for (int i=0; i < compressedBytes; i++)
        {
            pairin.buffer().put(tmpCompressedData[i]);
        }
        pairin.buffer().position(0);
        int decompressedBytes = _compressor.decompress(pairin, pairout, 0, compressedBytes);
        assertTrue(decompressedBytes == size);
        
        pairout.buffer().position(0);
        original.position(0);
        assertTrue(original.compareTo(pairout.buffer()) == 0);
        
    }
    
    private final ByteBuffer getTestData(MessageContentType dataType, int size, int capacity)
    {
        ByteBuffer buf = null;
        assertTrue(size <= capacity);
        switch (dataType)
        {
            case SEQUENCE:
            {
                buf = ByteBuffer.allocateDirect(capacity);
                for (int i=0; i < size; i++)
                {
                    buf.put((byte)(i % 255));
                }
                buf.position(0);
                buf.limit(size);
            }
            break;
             
            case UNIFORM:
            {
                buf = ByteBuffer.allocateDirect(capacity);
                for (int i=0; i < size; i++)
                {
                    buf.put((byte) 3);
                }
                buf.position(0);
                buf.limit(size);
            }
            break;
            
            case RANDOM:
            {
                buf = ByteBuffer.allocateDirect(capacity);
                for (int i=0; i < size; i++)
                {
                    buf.put((byte)(_gen.nextInt() % 255));
                }
                buf.position(0);
                buf.limit(size);
            }
            break;
            
            default:
                break;
        }
        
        return buf;
    }
    
    @SuppressWarnings("all")
    @Test
    public final void testCompressionTypes()
    {
        // caution here, internal Ripc.CompressionType refers to the RWF
        // external CompressionTypes happens to be the same, but isn't the same.
        assertTrue(CompressionTypes.ZLIB == Ripc.CompressionTypes.ZLIB);
        // Public compression types include LZ4
        assertTrue(CompressionTypes.LZ4 == Ripc.CompressionTypes.LZ4);
        // Public interface defines zlib and lz4
        assertTrue(CompressionTypes.MAX_DEFINED == CompressionTypes.LZ4);
    }
}
