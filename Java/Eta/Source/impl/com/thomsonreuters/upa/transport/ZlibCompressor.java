package com.thomsonreuters.upa.transport;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;

class ZlibCompressor extends Compressor
{
    private Deflater _deflater;
    private Inflater _inflater;
    private DeflaterOutputStream _deflaterOutputStream;
    private ByteArrayOutputStream _compressedBytesOutputStream;
    private byte[] _compressedBytes;
    private byte[] _decompressedBytes;
    private byte[] _compressByteArray;
    private int _numBytesAfterDecompress;
    private int _maxCompressionLen;

    {
        _deflater = new Deflater();
        _inflater = new Inflater();
        _numBytesAfterDecompress = 0;
        _maxCompressionLen = 6144;
        _deflater.setLevel(6);
    }

    @Override
    int compress(TransportBufferImpl bufferToCompress, int dataStartPos, int lenToCompress)
    {
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
        byte[] byteArray = new byte[lenToDecompress];
        int contentStartPos = bufferToDecompress.dataStartPosition();
        for (int i = 0; i < lenToDecompress; i++)
        {
            byteArray[i] = bufferToDecompress.data().get(contentStartPos + i);
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
        byte[] byteArray = new byte[lenToDecompress];
        for (int i = 0; i < lenToDecompress; i++)
        {
            byteArray[i] = bufferToDecompress.buffer().get(dataStartPos + i);
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

}
