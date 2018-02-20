package com.thomsonreuters.upa.transport;

import java.nio.ByteBuffer;

abstract class Compressor
{
    /* Compresses data in the TransportBufferImpl object bufferToCompress.
     * The compressed data is written to an internal byte[] which can be
     * accessed with compressedData() and compressedDataLength().
     * 
     * bufferToCompress contains the data to compress
     * dataStartPos is the starting position in bufferToCompress
     * lenToCompress is the number of bytes to compress from bufferToCompress
     * 
     * Returns the number of compressed bytes written to the internal byte array.
     */
    abstract int compress(TransportBufferImpl bufferToCompress, int dataStartPos, int lenToCompress);

    /* Provides a compression method where bytes to compress are contained in a ByteBuffer.
     * 
     * bufferToCompress contains the data to compress
     * dataStartPos is the starting position in bufferToCompress
     * lenToCompress is the number of bytes to compress from bufferToCompress
     * 
     * Returns the number of compressed bytes written to the internal byte array.
     */
    abstract int compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress);

    /* Provides access to the internal compressed data byte array owned by this instance.
     * 
     * Returns the byte[] containing compressed data from the last compression
     */
    abstract byte[] compressedData();

    /* Provides the number of bytes written into the array returned by compressedData().
     * 
     * Returns the number of bytes written into the internal byte[] for compressed data.
     */
    abstract int compressedDataLength();

    /* Decompression of data using TransportBufferImpl object.
     * 
     * bufferToDecompress contains the compressed data
     * decompressedBuffer is the decompressed data will be written into this buffer
     * lenToDecompress is the number of bytes to decompress
     * 
     * Returns the number of uncompressed bytes written to the decompressedBuffer.
     */
    abstract int decompress(TransportBufferImpl bufferToDecompress, TransportBufferImpl decompressedBuffer, int lenToDecompress);

    /* Decompression of data using ByteBufferPair object.
     * 
     * bufferToDecompress contains the compressed data
     * decompressedBuffer is the decompressed data will be written into this buffer
     * dataStartPos is the start position of compressed data for bufferToDecompress
     * lenToDecompress is the number of bytes to decompress from bufferToDecompress
     * 
     * Returns the number of uncompressed bytes written to the decompressedBuffer.
     */
    abstract int decompress(ByteBufferPair bufferToDecompress, ByteBufferPair decompressedBuffer, int dataStartPos, int lenToDecompress);

    /* Sets the compression level for this compression implementation.
     * 
     * level is the value of the compression level. The range and definition of the level is specific to each compression implementation.
     */
    abstract void compressionLevel(int level);

    /* Sets the maximum number of bytes allocated internally to support compression requests.
     * 
     * len is the number of bytes required for compression
     */
    abstract void maxCompressionLength(int len);

    /* For a given maximum number of bytes to compress,
     * this method returns the theoretical maximum size of the compressed result for this compression implementation.
     * 
     * Calculates the maximum number of compressed bytes for numBytesToCompress.
     */
    abstract int getMaxCompressedLength(int numBytesToCompress);

    /* Close by cleaning up anything necessary. */
    abstract void close();
}

