/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using System.IO;
using System.IO.Compression;
using System;

namespace LSEG.Eta.Internal
{
    internal class ZlibCompressor : Compressor
    {
        public const int DEFAULT_ZLIB_COMPRESSION_LEVEL = 6;
        private ZLibStream m_ZLibCompStream = null;
        private MemoryStream m_MemoryCompStream = null;
        private ZLibStream m_ZLibDeCompStream = null;
        private MemoryStream m_MemoryDeCompStream = null;
        private byte[] m_CompressedBytes;

        public ZlibCompressor()
        {
            MaxCompressionLength = 6144;
            CompressionLevel = DEFAULT_ZLIB_COMPRESSION_LEVEL;
        }

        public override byte[] CompressedData
        {
            get
            {
                return m_CompressedBytes;
            }
        }

        public override int CompressedDataLength
        {
            get
            {
                return m_CompressedBytes.Length;
            }
        }

        public override byte CompressionLevel { get; set; }
          
        public override int MaxCompressionLength { get; set; }

        public override void Close()
        {
            if (m_MemoryCompStream != null)
            {
                m_MemoryCompStream.Close();
                m_ZLibCompStream.Close();
                m_MemoryCompStream = null;
                m_ZLibCompStream = null;
            }

            if (m_MemoryDeCompStream != null)
            {
                m_MemoryDeCompStream.Close();
                m_ZLibDeCompStream.Close();
                m_MemoryDeCompStream = null;
                m_ZLibDeCompStream = null;
            }

            m_CompressedBytes = null;
        }

        public override int Compress(TransportBuffer bufferToCompress, int dataStartPos, int lenToCompress)
        {
            CreateZLibStream4Compression();

            /* Resets the position memory stream to resuse it.*/
            m_MemoryCompStream.Position = 0;
            m_MemoryCompStream.SetLength(0);

            try
            {
                m_ZLibCompStream.Write(bufferToCompress.Data.Contents, dataStartPos, lenToCompress);
                m_ZLibCompStream.Flush();
            }
            catch (Exception e)
            {
                throw new TransportException(e.Message);
            }

            m_CompressedBytes = m_MemoryCompStream.ToArray();
            
            return m_CompressedBytes.Length;
        }

        public override int Compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress)
        {
            CreateZLibStream4Compression();

            /* Resets the position memory stream to resuse it.*/
            m_MemoryCompStream.Position = 0;
            m_MemoryCompStream.SetLength(0);

            try
            {
                m_ZLibCompStream.Write(bufferToCompress.Contents, dataStartPos, lenToCompress);
                m_ZLibCompStream.Flush();
            }
            catch (Exception e)
            {
                throw new TransportException(e.Message);
            }

            m_CompressedBytes = m_MemoryCompStream.ToArray();

            return m_CompressedBytes.Length;
        }

        public override int Decompress(TransportBuffer bufferToDecompress, TransportBuffer decompressedBuffer, int lenToDecompress)
        {
            if (m_MemoryDeCompStream == null)
            {
                m_MemoryDeCompStream = new MemoryStream(MaxCompressionLength);
                m_MemoryDeCompStream.Write(bufferToDecompress.Data.Contents, bufferToDecompress.GetDataStartPosition(), lenToDecompress);
                m_ZLibDeCompStream = new ZLibStream(m_MemoryDeCompStream, CompressionMode.Decompress, false);
            }
            else
            {
                m_MemoryDeCompStream.SetLength(0);
                m_MemoryDeCompStream.Write(bufferToDecompress.Data.Contents, bufferToDecompress.GetDataStartPosition(), lenToDecompress);
            }

            m_MemoryDeCompStream.Position = 0;

            int bytesWrittern = -1;

            try
            {
                bytesWrittern = m_ZLibDeCompStream.Read(decompressedBuffer.Data.Contents, 0, decompressedBuffer.Data.Contents.Length);

                decompressedBuffer.Data.ReadPosition = 0;
                decompressedBuffer.Data.WritePosition = bytesWrittern;
                decompressedBuffer.Data.Limit = bytesWrittern;
            }
            catch(Exception e)
            {
                throw new TransportException(e.Message);
            }

            return bytesWrittern;
        }

        public override int Decompress(ByteBuffer bufferToDecompress, ByteBuffer decompressedBuffer, int dataStartPos, int lenToDecompress)
        {
            if (m_MemoryDeCompStream == null)
            {
                m_MemoryDeCompStream = new MemoryStream(MaxCompressionLength);
                m_MemoryDeCompStream.Write(bufferToDecompress.Contents, dataStartPos, lenToDecompress);
                m_ZLibDeCompStream = new ZLibStream(m_MemoryDeCompStream, CompressionMode.Decompress, false);
            }
            else
            {
                m_MemoryDeCompStream.SetLength(0);
                m_MemoryDeCompStream.Write(bufferToDecompress.Contents, dataStartPos, lenToDecompress);
            }

            m_MemoryDeCompStream.Position = 0;

            int bytesWrittern = -1;

            try
            {
                bytesWrittern = m_ZLibDeCompStream.Read(decompressedBuffer.Contents, 0, decompressedBuffer.Contents.Length);

                decompressedBuffer.ReadPosition = 0;
                decompressedBuffer.WritePosition = bytesWrittern;
                decompressedBuffer.Limit = bytesWrittern;
            }
            catch (Exception e)
            {
                throw new TransportException(e.Message);
            }

            return bytesWrittern;
        }

        public override int GetMaxCompressedLength(int numBytesToCompress)
        {
            return (numBytesToCompress + 13);
        }

        public override int PreDecompress(ByteBuffer bufferToDecompress, int dataStartPos, int lenToDecompress)
        {
            throw new System.NotImplementedException(); // This is used for JSON protocol over the websocket connection.
        }

        public override void WriteDecompressBuffer(ByteBuffer decompressedBuffer)
        {
            throw new System.NotImplementedException(); // This is used for JSON protocol over the websocket connection.
        }

        private static CompressionLevel ConvertCompressionLevel(byte sessionCompLevel)
        {
            return sessionCompLevel switch
            {
                1 => System.IO.Compression.CompressionLevel.Fastest,
                6 => System.IO.Compression.CompressionLevel.Optimal,
                9 => System.IO.Compression.CompressionLevel.SmallestSize,
                0 => System.IO.Compression.CompressionLevel.NoCompression,
                _ => System.IO.Compression.CompressionLevel.Optimal,
            };
        }

        private void CreateZLibStream4Compression()
        {
            if (m_MemoryCompStream is null)
            {
                m_MemoryCompStream = new MemoryStream(MaxCompressionLength);
                m_ZLibCompStream = new ZLibStream(m_MemoryCompStream, ConvertCompressionLevel(CompressionLevel));
            }
        }
    }
}
