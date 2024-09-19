/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Internal
{
    internal abstract class Compressor
    {
        /// <summary>
        /// Compresses data in the TransportBuffer object.
        /// The compressed data is written to an internal byte[] which can be accessed with <see cref="CompressedData"/> and <see cref="CompressedDataLength"/>.
        /// </summary>
        /// <param name="bufferToCompress">Contains the data to compress</param>
        /// <param name="dataStartPos">The starting position in bufferToCompress</param>
        /// <param name="lenToCompress">The number of bytes to compress from bufferToCompress</param>
        /// <returns>The number of compressed bytes written to the internal byte array</returns>
        public abstract int Compress(TransportBuffer bufferToCompress, int dataStartPos, int lenToCompress);

        /// <summary>
        /// Provides a compression method where bytes to compress are contained in a <see cref="ByteBuffer"/>.
        /// The compressed data is written to an internal byte[] which can be accessed with <see cref="CompressedData"/> and <see cref="CompressedDataLength"/>
        /// </summary>
        /// <param name="bufferToCompress">Contains the data to compress</param>
        /// <param name="dataStartPos">The starting position in bufferToCompress</param>
        /// <param name="lenToCompress">The number of bytes to compress from bufferToCompress</param>
        /// <returns>The number of compressed bytes written to the internal byte array</returns>
        public abstract int Compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress);

        /// <summary>
        /// Gets the internal compressed data byte array owned by this instance.
        /// </summary>
        /// <value>The byte array containing compressed data from the last compression</value>
        public abstract byte[] CompressedData { get; }

        /// <summary>
        /// Gets the number of bytes written into the array returned by <see cref="CompressedData"/>.
        /// </summary>
        /// <value>The number of bytes written into the internal byte[] for compressed data</value>
        public abstract int CompressedDataLength { get; }

        /// <summary>
        /// Decompression of data using <c>TransportBuffer</c> object.
        /// </summary>
        /// <param name="bufferToDecompress">Contains the compressed data</param>
        /// <param name="decompressedBuffer">The decompressed data will be written into this buffer</param>
        /// <param name="lenToDecompress">The number of bytes to decompress</param>
        /// <returns>The number of uncompressed bytes written to the decompressedBuffer</returns>
        public abstract int Decompress(TransportBuffer bufferToDecompress, TransportBuffer decompressedBuffer, int lenToDecompress);

        /// <summary>
        /// Decompression of data using <c>ByteBuffer</c> object.
        /// </summary>
        /// <param name="bufferToDecompress">Contains the compressed data</param>
        /// <param name="decompressedBuffer">The decompressed data will be written into this buffer</param>
        /// <param name="dataStartPos">The start position of compressed data for bufferToDecompress</param>
        /// <param name="lenToDecompress">The number of bytes to decompress from bufferToDecompress</param>
        /// <returns>The number of uncompressed bytes written to the decompressedBuffer</returns>
        public abstract int Decompress(ByteBuffer bufferToDecompress, ByteBuffer decompressedBuffer, int dataStartPos, int lenToDecompress);

        /// <summary>
        /// Pre-decompression of data using ByteBuffer object. This is used to decompress data into the internal buffer.
        /// </summary>
        /// <param name="bufferToDecompress">Contains the compressed data</param>
        /// <param name="dataStartPos">The start position of compressed data for bufferToDecompress</param>
        /// <param name="lenToDecompress">The number of bytes to decompress from bufferToDecompress</param>
        /// <returns>The number of uncompressed bytes</returns>
        public abstract int PreDecompress(ByteBuffer bufferToDecompress, int dataStartPos, int lenToDecompress);

        /// <summary>
        /// Writes the internal buffer into the decompressed buffer. This function must be called after the <see cref="PreDecompress(ByteBuffer, int, int)"/>.
        /// </summary>
        /// <param name="decompressedBuffer">The decompressed data will be written into this buffer</param>
        public abstract void WriteDecompressBuffer(ByteBuffer decompressedBuffer);

        /// <summary>
        /// Sets or gets the compression level for this compression implementation.
        /// </summary>
        /// <value>The compression level supported by each compression implementation</value>
        public abstract byte CompressionLevel { get; set; }

        /// <summary>
        /// Sets or gets the maximum number of bytes allocated internally to support compression requests.
        /// </summary>
        /// <value>The number of bytes required for compression</value>
        public abstract int MaxCompressionLength { get; set; }

        /// <summary>
        /// This method returns the theoretical maximum size of the compressed result for this compression implementation.
        /// </summary>
        /// <remarks>Calculates the maximum number of compressed bytes for numBytesToCompress</remarks>
        /// <param name="numBytesToCompress">The number of bytes to compress</param>
        public abstract int GetMaxCompressedLength(int numBytesToCompress);

        /// <summary>
        /// Close by cleaning up anything necessary.
        /// </summary>
        public abstract void Close();
    }
}
