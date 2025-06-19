/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;

class ZlibCompressor extends Compressor
{
    public static final int DEFAULT_ZLIB_COMPRESSION_LEVEL = 6;

    private Deflater _deflater;
    private Inflater _inflater;
    private DeflaterOutputStream _deflaterOutputStream;
    private ByteArrayOutputStream _compressedBytesOutputStream;
    private byte[] _compressedBytes;
    private byte[] _decompressedBytes;
    private byte[] _compressByteArray;
    private int _numBytesAfterDecompress;
    private int _maxCompressionLen;
    private boolean _appendTrailing;
    private boolean _compressnocontexttakeover;
    private boolean _nowrap;
    private int _compressionLevel;
    final static byte[] EndingTrailing = new byte[4];

    {
        EndingTrailing[0] = 0;
        EndingTrailing[1] = 0;
        EndingTrailing[2] = -1;
        EndingTrailing[3] = -1;
        _numBytesAfterDecompress = 0;
        _maxCompressionLen = 6144;
        _appendTrailing = false;
        _compressnocontexttakeover = false;
        _nowrap = false;
        _compressionLevel = DEFAULT_ZLIB_COMPRESSION_LEVEL;
    }

    ZlibCompressor()
    {
        _deflater = new Deflater();
        _inflater = new Inflater();
        _deflater.setLevel(DEFAULT_ZLIB_COMPRESSION_LEVEL);
    }

    /*
     * Specifies the nowrap option for the Inflater class.
     * If the parameter 'nowrap' is true then the ZLIB header and checksum fields will not be used.
     */
    ZlibCompressor(int compressionLevel, boolean nowrap)
    {
        _deflater = new Deflater(compressionLevel, nowrap);
        _inflater = new Inflater(nowrap);
        
        _compressionLevel = compressionLevel;
        _nowrap = nowrap;
    }

    @Override
    int compress(TransportBufferImpl bufferToCompress, int dataStartPos, int lenToCompress)
    {
        if (_appendTrailing) {
            _deflater.reset();
        }
        
        if(lenToCompress > _maxCompressionLen)
        {
        	_maxCompressionLen = lenToCompress;
        	_compressedBytesOutputStream = null;;
        	 _compressByteArray = null;
        	 _deflaterOutputStream = null;
        }
        
        if(_compressnocontexttakeover) {
        	
        	_deflater.end();
        	_deflater = new Deflater(_compressionLevel, _nowrap);
        	
        	if (_compressedBytesOutputStream == null)
        		_compressedBytesOutputStream = new ByteArrayOutputStream(getMaxCompressedLength(_maxCompressionLen));
        	else
        		_compressedBytesOutputStream.reset();
        	
        	_deflaterOutputStream = new DeflaterOutputStream(_compressedBytesOutputStream, _deflater,
                    getMaxCompressedLength(_maxCompressionLen), true);
        }
        
        // lazily initialize _compressedBytesOutputStream buffer since we don't know size up front
        if (_compressedBytesOutputStream == null)
        {
            _compressedBytesOutputStream = new ByteArrayOutputStream(getMaxCompressedLength(_maxCompressionLen));
        }
        // lazily initialize _deflaterOutputStream buffer since we don't know size up front
        if (_deflaterOutputStream == null)
        {
            _deflaterOutputStream = new DeflaterOutputStream(_compressedBytesOutputStream, _deflater,
                    getMaxCompressedLength(_maxCompressionLen), true);
        }
        // lazily initialize _compressByteArray buffer since we don't know size up front
        if (_compressByteArray == null)
        {
            _compressByteArray = new byte[_maxCompressionLen];
        }
        // copy bufferToCompress contents to _compressByteArray
        for (int i = 0; i < lenToCompress; i++)
        {
            _compressByteArray[i] = bufferToCompress.data().get(dataStartPos + i);
        }
        _compressedBytesOutputStream.reset();
        try
        {
            // write bytes to compress
            _deflaterOutputStream.write(_compressByteArray, 0, lenToCompress);
            // flush bytes to compress
            _deflaterOutputStream.flush();
        }
        catch (Exception e)
        {
            throw new CompressorException(e.getLocalizedMessage());
        }
        // get compressed bytes
        _compressedBytes = _compressedBytesOutputStream.toByteArray();

        // System.out.println("[ZLIB-C2] in=" + lenToCompress + " compressed=" + _compressedBytes.length);
        return _compressedBytes.length;
    }

    @Override
    int compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress)
    {
        if (_appendTrailing) {
        	
            _deflater.reset();
        }
        
        if(lenToCompress > _maxCompressionLen)
        {
        	_maxCompressionLen = lenToCompress;
        	_compressedBytesOutputStream = null;;
        	 _compressByteArray = null;
        	 _deflaterOutputStream = null;
        }
        
        if(_compressnocontexttakeover) {
        	
        	_deflater.end();
        	_deflater = new Deflater(_compressionLevel, _nowrap);
        	
        	if (_compressedBytesOutputStream == null)
        		_compressedBytesOutputStream = new ByteArrayOutputStream(getMaxCompressedLength(_maxCompressionLen));
        	else
        		_compressedBytesOutputStream.reset();
        	
        	 _deflaterOutputStream = new DeflaterOutputStream(_compressedBytesOutputStream, _deflater,
                     getMaxCompressedLength(_maxCompressionLen), true);
        }
        
        // lazily initialize _compressedBytesOutputStream buffer since we don't know size up front
        if (_compressedBytesOutputStream == null)
        {
            _compressedBytesOutputStream = new ByteArrayOutputStream(getMaxCompressedLength(_maxCompressionLen));
        }
        // lazily initialize _deflaterOutputStream buffer since we don't know size up front
        if (_deflaterOutputStream == null)
        {
            _deflaterOutputStream = new DeflaterOutputStream(_compressedBytesOutputStream, _deflater,
                    getMaxCompressedLength(_maxCompressionLen), true);
        }
        // lazily initialize _compressByteArray buffer since we don't know size up front
        if (_compressByteArray == null)
        {
            _compressByteArray = new byte[_maxCompressionLen];
        }
        // copy bufferToCompress contents to _compressByteArray
        for (int i = 0; i < lenToCompress; i++)
        {
            _compressByteArray[i] = bufferToCompress.get(dataStartPos + i);
        }
        _compressedBytesOutputStream.reset();
        try
        {
            // write bytes to compress
            _deflaterOutputStream.write(_compressByteArray, 0, lenToCompress);
            // flush bytes to compress
            _deflaterOutputStream.flush();
        }
        catch (Exception e)
        {
            throw new CompressorException(e.getLocalizedMessage());
        }
        // get compressed bytes
        _compressedBytes = _compressedBytesOutputStream.toByteArray();

        // System.out.println("[ZLIB-C3] in=" + lenToCompress + " compressed=" + _compressedBytes.length);
        return _compressedBytes.length;
    }

    @Override
    int decompress(TransportBufferImpl bufferToDecompress, TransportBufferImpl decompressedBuffer, int lenToDecompress)
    {
        // lazily initialize _decompressedBytes buffer since we don't know size up front
        if (_decompressedBytes == null)
        {
            _decompressedBytes = new byte[_maxCompressionLen];
        }

        // copy bufferToDecompress contents to byte array
        byte[] byteArray = new byte[lenToDecompress + 4];
        int contentStartPos = bufferToDecompress.dataStartPosition();
        int i = 0;
        for (; i < lenToDecompress; i++)
        {
            byteArray[i] = bufferToDecompress.data().get(contentStartPos + i);
        }

        if(_appendTrailing)
        {
            lenToDecompress += 4;
            for(int j = 0; j < 4; j++)
            {
                byteArray[i + j] = EndingTrailing[j];
            }
        }

        _inflater.setInput(byteArray, 0, lenToDecompress);
        try
        {
            _numBytesAfterDecompress = _inflater.inflate(_decompressedBytes);
            decompressedBuffer.data().clear();
            decompressedBuffer.data().put(_decompressedBytes, 0, _numBytesAfterDecompress);
            decompressedBuffer.data().limit(decompressedBuffer.data().position());
            decompressedBuffer.data().position(0);
        }
        catch (DataFormatException e)
        {
            throw new CompressorException(e.getLocalizedMessage());
        }

        // System.out.println("[ZLIB-decompress-TransportBufferImpl] in=" + lenToDecompress + " decompressed=" + _numBytesAfterDecompress);
        return _numBytesAfterDecompress;
    }

    @Override
    int decompress(ByteBufferPair bufferToDecompress, ByteBufferPair decompressedBuffer, int dataStartPos, int lenToDecompress)
    {
        // lazily initialize _decompressedBytes buffer since we don't know size up front
        if (_decompressedBytes == null)
        {
            _decompressedBytes = new byte[_maxCompressionLen];
        }
        // copy bufferToDecompress contents to byte array
        byte[] byteArray = new byte[lenToDecompress + 4];
        int i = 0;
        for (; i < lenToDecompress; i++)
        {
            byteArray[i] = bufferToDecompress.buffer().get(dataStartPos + i);
        }

        if(_appendTrailing)
        {
            lenToDecompress += 4;
            for(int j = 0; j < 4; j++)
            {
                byteArray[i + j] = EndingTrailing[j];
            }
        }

        _inflater.setInput(byteArray, 0, lenToDecompress);
        try
        {
            _numBytesAfterDecompress = _inflater.inflate(_decompressedBytes);
            decompressedBuffer.buffer().clear();
            decompressedBuffer.buffer().put(_decompressedBytes, 0, _numBytesAfterDecompress);
            decompressedBuffer.buffer().limit(decompressedBuffer.buffer().position());
            decompressedBuffer.buffer().position(0);
        }
        catch (DataFormatException e)
        {
            // System.out.println("[ZLIB-decompress-ByteBufferPair] error: " + e.getMessage());
            throw new CompressorException(e.getLocalizedMessage());
        }

        // System.out.println("[ZLIB-decompress-ByteBufferPair] in=" + lenToDecompress + " decompressed=" + _numBytesAfterDecompress);
        return _numBytesAfterDecompress;
    }

    @Override
    void compressionLevel(int level)
    {
        _deflater.setLevel(level);
    }

    @Override
    void maxCompressionLength(int len)
    {
        _maxCompressionLen = len;
    }

    @Override
    int getMaxCompressedLength(int numBytesToCompress)
    {
        return (numBytesToCompress + 13);
    }

    @Override
    byte[] compressedData()
    {
        return _compressedBytes;
    }

    @Override
    int compressedDataLength()
    {
        return _compressedBytes.length;
    }

    @Override
    void close()
    {
        _inflater.reset();
        _deflater.reset();
    }

    @Override
    void appendCompressTrailing()
    {
        _appendTrailing = true;
    }

    @Override
    void compressnocontexttakeover()
    {
    	_compressnocontexttakeover = true;
    }

    @Override
	int preDecompress(ByteBufferPair bufferToDecompress, int dataStartPos, int lenToDecompress) {
		
		// lazily initialize _decompressedBytes buffer since we don't know size up front
    	int estimatedBytes = lenToDecompress * 4;
    	
    	if(_decompressedBytes != null)
    	{
    		if(estimatedBytes >_decompressedBytes.length)
    		{
    			_decompressedBytes = new byte[estimatedBytes];
    		}
    	}
    	else
    	{
    		if(estimatedBytes > _maxCompressionLen)
        	{
        		_decompressedBytes = new byte[estimatedBytes];
        	}
        	else
        	{
        		_decompressedBytes = new byte[_maxCompressionLen];
        	}
    	}
    	
        // copy bufferToDecompress contents to byte array
        byte[] byteArray = new byte[lenToDecompress + 4];
        int i = 0;
        for (; i < lenToDecompress; i++)
        {
            byteArray[i] = bufferToDecompress.buffer().get(dataStartPos + i);
        }

        if(_appendTrailing)
        {
            lenToDecompress += 4;
            for(int j = 0; j < 4; j++)
            {
                byteArray[i + j] = EndingTrailing[j];
            }
        }

        try
        {
            _inflater.setInput(byteArray, 0, lenToDecompress);
        	
            _numBytesAfterDecompress = _inflater.inflate(_decompressedBytes);
            
            while(!_inflater.finished() && _inflater.getRemaining() > 0)
            {
            	byte[] decompressedBytesTemp = Arrays.copyOf(_decompressedBytes, _decompressedBytes.length * 2);
            	
            	_numBytesAfterDecompress += _inflater.inflate(decompressedBytesTemp, _numBytesAfterDecompress, decompressedBytesTemp.length - _decompressedBytes.length);
            	
            	_decompressedBytes = decompressedBytesTemp;
            	
            }
            
        	if(_inflater.finished())
        	{
        		_inflater.reset();
        	}
            	
        }
        catch (DataFormatException e)
        {
            throw new CompressorException(e.getLocalizedMessage());
        }

        return _numBytesAfterDecompress;
	}

	@Override
	void writeDecompressBuffer(ByteBufferPair decompressedBuffer) {
		
	    decompressedBuffer.buffer().clear();
        decompressedBuffer.buffer().put(_decompressedBytes, 0, _numBytesAfterDecompress);
        decompressedBuffer.buffer().limit(decompressedBuffer.buffer().position());
        decompressedBuffer.buffer().position(0);
	}
}
