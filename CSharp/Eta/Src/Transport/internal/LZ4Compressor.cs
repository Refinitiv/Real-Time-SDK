/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Transports;
using K4os.Compression.LZ4;

namespace Refinitiv.Eta.Internal
{
    internal class Lz4Compressor : Compressor
    {
        private byte[] m_CompressedBytes;
        private int m_CompressionLength;

        public override byte[] CompressedData => m_CompressedBytes;

        public override int CompressedDataLength => m_CompressionLength;

        /* This is not directly applicable for the LZ4 algorithm, as it is for zlib. The L00_FAST level is used by default. */
        public override byte CompressionLevel { get; set; }
        
        public override int MaxCompressionLength { get; set; }

        private const LZ4Level DEFAULT_LZ4_LEVEL = LZ4Level.L00_FAST;

        public Lz4Compressor()
        {
            MaxCompressionLength = 6144;
        }

        public override void Close()
        {
            m_CompressedBytes = null;
        }

        public override int Compress(TransportBuffer bufferToCompress, int dataStartPos, int lenToCompress)
        {
            if (m_CompressedBytes is null)
            {
                m_CompressedBytes = new byte[MaxCompressionLength];
            }

            m_CompressionLength = LZ4Codec.Encode(bufferToCompress.Data.Contents, dataStartPos, lenToCompress, m_CompressedBytes, 
                0, m_CompressedBytes.Length, DEFAULT_LZ4_LEVEL);

            if(m_CompressionLength < 0)
            {
                throw new TransportException("Failed to perform LZ4 compression.");
            }

            return m_CompressionLength;
        }

        public override int Compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress)
        {
            if (m_CompressedBytes is null)
            {
                m_CompressedBytes = new byte[MaxCompressionLength];
            }

            m_CompressionLength = LZ4Codec.Encode(bufferToCompress.Contents, dataStartPos, lenToCompress, m_CompressedBytes,
                0, m_CompressedBytes.Length, DEFAULT_LZ4_LEVEL);

            if (m_CompressionLength < 0)
            {
                throw new TransportException("Failed to perform LZ4 compression.");
            }

            return m_CompressionLength;
        }

        public override int Decompress(TransportBuffer bufferToDecompress, TransportBuffer decompressedBuffer, int lenToDecompress)
        {
            int bytesWrittern = LZ4Codec.Decode(bufferToDecompress.Data.Contents, bufferToDecompress.DataStartPosition, lenToDecompress,
                decompressedBuffer.Data.Contents, 0, decompressedBuffer.Data.Contents.Length);

            if(bytesWrittern < 0)
            {
                throw new TransportException("Failed to perform LZ4 decompression.");
            }

            decompressedBuffer.Data.ReadPosition = 0;
            decompressedBuffer.Data.WritePosition = bytesWrittern;
            decompressedBuffer.Data.Limit = bytesWrittern;

            return bytesWrittern;
        }

        public override int Decompress(ByteBuffer bufferToDecompress, ByteBuffer decompressedBuffer, int dataStartPos, int lenToDecompress)
        {
            int bytesWrittern = LZ4Codec.Decode(bufferToDecompress.Contents, dataStartPos, lenToDecompress,
                decompressedBuffer.Contents, 0, decompressedBuffer.Contents.Length);

            if (bytesWrittern < 0)
            {
                throw new TransportException("Failed to perform LZ4 decompression.");
            }

            decompressedBuffer.ReadPosition = 0;
            decompressedBuffer.WritePosition = bytesWrittern;
            decompressedBuffer.Limit = bytesWrittern;

            return bytesWrittern;
        }

        public override int GetMaxCompressedLength(int numBytesToCompress)
        {
            return LZ4Codec.MaximumOutputSize(numBytesToCompress);
        }

        public override int PreDecompress(ByteBuffer bufferToDecompress, int dataStartPos, int lenToDecompress)
        {
            throw new System.NotImplementedException(); // This is used for JSON protocol over the websocket connection.
        }

        public override void WriteDecompressBuffer(ByteBuffer decompressedBuffer)
        {
            throw new System.NotImplementedException(); // This is used for JSON protocol over the websocket connection.
        }
    }
}
