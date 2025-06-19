/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

import net.jpountz.lz4.LZ4Compressor;
import net.jpountz.lz4.LZ4Exception;
import net.jpountz.lz4.LZ4Factory;
import net.jpountz.lz4.LZ4SafeDecompressor;

class Lz4Compressor extends Compressor
{
    private byte[] _bytesToCompress;
    private byte[] _compressedBytes;
    private int _compressedBytesLen;
    private byte[] _bytesToDecompress;
    private byte[] _decompressedBytes;
    private int _maxCompressionInLen;
    private LZ4Compressor _lz4Compressor;
    private LZ4SafeDecompressor _lz4Decompressor;

    {
    	_lz4Compressor = LZ4Factory.safeInstance().fastCompressor();
    	_lz4Decompressor =  LZ4Factory.safeInstance().safeDecompressor();
        _maxCompressionInLen = 6144;

        _compressedBytes = null;
        _bytesToCompress = null;
        _decompressedBytes = null;
        _bytesToDecompress = null;
    }

    /* Wraps the lz4 library compress method.
     * Compresses the bufferToCompress data into _compressedBytes owned by this object.
     * 
     * The method will have the capacity to store the compressed data internally
     * if lenToCompress is <= the size passed to maxCompressionLength.
     * 
     * Returns the compressed size
     */
    @Override
    int compress(TransportBufferImpl bufferToCompress, int dataStartPos, int lenToCompress)
    {
        // The _compressedBytes buffer is sized to hold the maximum size of
        // compressed data for this compressor, based on the given maximum
        // number of bytes to compress, as defined in the call to maxCompressionLength().
        if (_compressedBytes == null)
            _compressedBytes = new byte[getMaxCompressedLength(_maxCompressionInLen)];

        byte[] bytesToCompress;
        if (!bufferToCompress.data().hasArray())
        {
            // Size of _bytesToCompress is based on the maximum size
            // of data to compress as set by call to maxCompressionLength().
            if (_bytesToCompress == null)
                _bytesToCompress = new byte[_maxCompressionInLen];

            bytesToCompress = _bytesToCompress;

            for (int i = 0; i < lenToCompress; i++)
            {
                bytesToCompress[i] = bufferToCompress.data().get(dataStartPos + i);
            }
            // reset dataStartPos since copied to bytesToCompress at position 0
            dataStartPos = 0;
        }
        else
        {
            bytesToCompress = bufferToCompress.data().array();
        }
        
        try
        {
        	_compressedBytesLen = _lz4Compressor.compress(bytesToCompress, dataStartPos, lenToCompress,
                                                      _compressedBytes, 0, _compressedBytes.length);
        } catch(LZ4Exception e)
        {
        	throw new CompressorException("LZ4 compress(TransportBufImpl, int, int) exception: " + e.getMessage());
        }

        //System.out.println("[LZ4 Cv2] in=" + lenToCompress + " compressed=" + _compressedBytesLen);
        return _compressedBytesLen;
    }

    /* Wraps the lz4 library compress method.
     * Compresses the bufferToCompress data into _compressedBytes owned by this object.
     * 
     * The method will have the capacity to store the compressed data internally
     * if lenToCompress is <= the size passed to maxCompressionLength.
     * 
     * Returns the compressed size
     */
    @Override
    int compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress)
    {
        // The _compressedBytes buffer is sized to hold the maximum size of
        // compressed data for this compressor, based on the given maximum
        // number of bytes to compress, as defined in the call to maxCompressionLength().
        if (_compressedBytes == null)
            _compressedBytes = new byte[getMaxCompressedLength(_maxCompressionInLen)];

        byte[] bytesToCompress;
        if (!bufferToCompress.hasArray())
        {
            // Size of _bytesToCompress is based on the maximum size
            // of data to compress as set by call to maxCompressionLength().
            if (_bytesToCompress == null)
                _bytesToCompress = new byte[_maxCompressionInLen];

            bytesToCompress = _bytesToCompress;

            for (int i = 0; i < lenToCompress; i++)
            {
                bytesToCompress[i] = bufferToCompress.get(dataStartPos + i);
            }
            // reset dataStartPos since copied to bytesToCompress at position 0
            dataStartPos = 0;
        }
        else
        {
            bytesToCompress = bufferToCompress.array();
        }
        
        try
        {
        	_compressedBytesLen = _lz4Compressor.compress(bytesToCompress, dataStartPos, lenToCompress,
                                                      _compressedBytes, 0, _compressedBytes.length);
        } catch(LZ4Exception e)
        {
        	throw new CompressorException("LZ4 compress(ByteBuffer, int, int) exception: " + e.getMessage());
        }
        
        //System.out.println("[LZ4 Cv3] in=" + lenToCompress + " compressed=" + _compressedBytesLen);
        return _compressedBytesLen;
    }

    @Override
    byte[] compressedData()
    {
        return _compressedBytes;
    }

    @Override
    int compressedDataLength()
    {
        return _compressedBytesLen;
    }

    @Override
    int decompress(TransportBufferImpl bufferToDecompress, TransportBufferImpl decompressedBuffer, int lenToDecompress)
    {
    	
        //System.out.println("[LZ4-decompress-TransportBufferImpl] Start decompressing " + lenToDecompress);
        int uncompressedBytesLen = 0;
        byte[] bytesToDecompress;

        if (_decompressedBytes == null)
            _decompressedBytes = new byte[_maxCompressionInLen];

        if (!bufferToDecompress.data().hasArray())
        {
            if (_bytesToDecompress == null)
                _bytesToDecompress = new byte[getMaxCompressedLength(_maxCompressionInLen)];

            bytesToDecompress = _bytesToDecompress;

            int contentStartPos = bufferToDecompress.dataStartPosition();
            for (int i = 0; i < lenToDecompress; i++)
            {
                bytesToDecompress[i] = bufferToDecompress.data().get(contentStartPos + i);
            }
        }
        else
        {
            bytesToDecompress = bufferToDecompress.data().array();
        }
        
        try
        {
        	uncompressedBytesLen = _lz4Decompressor.decompress(bytesToDecompress, 0, // src offset
                                                           lenToDecompress, _decompressedBytes, 0); // dst offset
        } catch(LZ4Exception e)
        {
        	throw new CompressorException("LZ4 decompress(TransportBufImpl, TransportBufImpl, int) exception: " + e.getMessage());
        }

        decompressedBuffer.data().clear();
        decompressedBuffer.data().put(_decompressedBytes, 0, uncompressedBytesLen);
        decompressedBuffer.data().limit(decompressedBuffer.data().position());
        decompressedBuffer.data().position(0);

        // System.out.println("[LZ4] in=" + lenToDecompress + " decompressed=" + uncompressedBytesLen);
        return uncompressedBytesLen;
    }

    @Override
    int decompress(ByteBufferPair bufferToDecompress, ByteBufferPair decompressedBuffer, int dataStartPos, int lenToDecompress)
    {
    	//System.out.println("[LZ4-decompress-ByteBufferPair] Start decompressing " + lenToDecompress);
        int uncompressedBytesLen = 0;
        byte[] bytesToDecompress;

        if (_decompressedBytes == null)
            _decompressedBytes = new byte[_maxCompressionInLen];

        if (!bufferToDecompress.buffer().hasArray())
        {
            // Bytes to decompress could be larger than the ripc message size in the scenario where CompFrag is used
            if (_bytesToDecompress == null)
                _bytesToDecompress = new byte[getMaxCompressedLength(_maxCompressionInLen)];

            bytesToDecompress = _bytesToDecompress;

            for (int i = 0; i < lenToDecompress; i++)
            {
                bytesToDecompress[i] = bufferToDecompress.buffer().get(dataStartPos + i);
            }
        }
        else
        {
            bytesToDecompress = bufferToDecompress.buffer().array();
        }

        try
        {
        	uncompressedBytesLen = _lz4Decompressor.decompress(bytesToDecompress, 0, // src offset
                                                           lenToDecompress, _decompressedBytes, 0); // dst offset
        } catch(LZ4Exception e)
        {
        	throw new CompressorException("LZ4 decompress(ByteBufferPair, ByteBufferPair, int, int) exception: " + e.getMessage());
        }
        
        decompressedBuffer.buffer().clear();
        decompressedBuffer.buffer().put(_decompressedBytes, 0, uncompressedBytesLen);
        decompressedBuffer.buffer().limit(decompressedBuffer.buffer().position());
        decompressedBuffer.buffer().position(0);

        //System.out.println("[LZ4] in=" + lenToDecompress + " decompressed=" + uncompressedBytesLen);
        return uncompressedBytesLen;
    }

    /* The compression level for LZ4 is ignored, since there is only one level currently supported. */
    @Override
    void compressionLevel(int level)
    {
        // This is not directly applicable for the LZ4 algorithm, as it is for zlib.
        // The LZ4 library does offer two options (FAST and a HIGH_COMPRESSION).
        // Since the primary motivation for introducing lz4 is its speed (compared to zlib), the fast implementation is used here.
        // Potentially, levels could be defined to allow selection of the different lz4 options.
    }

    @Override
    void maxCompressionLength(int len)
    {
        _maxCompressionInLen = len;
    }

    @Override
    int getMaxCompressedLength(int numBytesToCompress)
    {
        return _lz4Compressor.maxCompressedLength(numBytesToCompress);
    }

    @Override
    void close()
    {
    }

	@Override
	int preDecompress(ByteBufferPair bufferToDecompress, int dataStartPos, int lenToDecompress) {
		throw new UnsupportedOperationException();
	}

	@Override
	void writeDecompressBuffer(ByteBufferPair decompressedBuffer) {
		throw new UnsupportedOperationException();
	}
}
