///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Codec;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;

import static org.junit.Assert.*;

public class HttpSocketReadWriteJunit {

    private static final int MAX_FRAGMENT_SIZE = 63;
    private static final int CHANNEL_INFO_MAX_FRAGMENT_SIZE = 44;
    private static final int HEADER_SIZE = 6;
    private static final int FOOTER_SIZE = 2;

    String RWF_MSG_1 = "123456RWFEncodedBufferxxxfweirfsdfkjl";
    ByteBuffer RWF_MSG_1_ByteBuffer = ByteBuffer.allocate(RWF_MSG_1.length());
    String RWF_MSG_2 = "999999999999988888888888888888998eqwewqerqwer9qwtqfafjakefjaskerjfra;9345353453433243324-fasdfj0000000NULL";

    ByteBuffer RWF_MSG_2_ByteBuffer = ByteBuffer.allocate(RWF_MSG_2.length());
    ByteBuffer PACKED_RWF_MSG_1_MSG_2_ByteBuffer = ByteBuffer.allocate(2 + RWF_MSG_1.length() + 2 + RWF_MSG_2.length());
    ByteBuffer PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER = ByteBuffer.allocate(3 + 2 + RWF_MSG_1.length() + 2 + RWF_MSG_2.length());
    ByteBuffer COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER = ByteBuffer.allocate(3 + 2 + RWF_MSG_1.length() + 2 + RWF_MSG_2.length());

    {
        RWF_MSG_1_ByteBuffer.put(RWF_MSG_1.getBytes());
        RWF_MSG_1_ByteBuffer.rewind();

        RWF_MSG_2_ByteBuffer.put(RWF_MSG_2.getBytes());
        RWF_MSG_2_ByteBuffer.rewind();

        PACKED_RWF_MSG_1_MSG_2_ByteBuffer.putShort((short) RWF_MSG_1.length());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer.put(RWF_MSG_1.getBytes());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer.putShort((short) RWF_MSG_2.length());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer.put(RWF_MSG_2.getBytes());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer.rewind();

        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short) PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put((byte) (RsslSocketChannel.IPC_DATA | RsslSocketChannel.IPC_PACKING));
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short) RWF_MSG_1.length());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_1.getBytes());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short) RWF_MSG_2.length());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_2.getBytes());
        PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.rewind();

        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short) COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit());
        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put((byte) (RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION));
        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short) RWF_MSG_1.length());
        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_1.getBytes());
        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short) RWF_MSG_2.length());
        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_2.getBytes());
        COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.rewind();
    }

    /* Provides ZLIB compression functionality to verify the result of testing */
    class ZlibCompressorTest {
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
        private byte[] _endingTrailing;

        ZlibCompressorTest(boolean appendTrailing) {
            _deflater = new Deflater();
            _inflater = new Inflater();
            _numBytesAfterDecompress = 0;
            _maxCompressionLen = 6144;
            _deflater.setLevel(6);
            _appendTrailing = appendTrailing;
            _endingTrailing = new byte[4];
            _endingTrailing[0] = 0;
            _endingTrailing[1] = 0;
            _endingTrailing[2] = -1;
            _endingTrailing[3] = -1;
        }

        int decompress(ByteBuffer bufferToDecompress, int offSet, ByteBuffer decompressedBuffer, int lenToDecompress) {
            // lazily initialize _decompressedBytes buffer since we don't know size up front
            if (_decompressedBytes == null) {
                _decompressedBytes = new byte[_maxCompressionLen];
            }

            // copy bufferToDecompress contents to byte array
            byte[] byteArray = new byte[lenToDecompress + 4];
            int contentStartPos = offSet;
            int i = 0;
            for (; i < lenToDecompress; i++) {
                byteArray[i] = bufferToDecompress.get(contentStartPos + i);
            }

            if (_appendTrailing) {
                lenToDecompress += 4;
                for (int j = 0; j < 4; j++) {
                    byteArray[i + j] = _endingTrailing[j];
                }
            }

            _inflater.setInput(byteArray, 0, lenToDecompress);
            try {
                _numBytesAfterDecompress = _inflater.inflate(_decompressedBytes);
                decompressedBuffer.clear();
                decompressedBuffer.put(_decompressedBytes, 0, _numBytesAfterDecompress);
                decompressedBuffer.limit(decompressedBuffer.position());
                decompressedBuffer.position(0);
            } catch (DataFormatException e) {
                throw new CompressorException(e.getLocalizedMessage());
            }

            return _numBytesAfterDecompress;
        }

        int compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress) {
            // lazily initialize _compressedBytesOutputStream buffer since we don't know size up front
            if (_compressedBytesOutputStream == null) {
                _compressedBytesOutputStream = new ByteArrayOutputStream(getMaxCompressedLength(_maxCompressionLen));
            }
            // lazily initialize _deflaterOutputStream buffer since we don't know size up front
            if (_deflaterOutputStream == null) {
                _deflaterOutputStream = new DeflaterOutputStream(_compressedBytesOutputStream, _deflater,
                        getMaxCompressedLength(_maxCompressionLen), true);
            }
            // lazily initialize _compressByteArray buffer since we don't know size up front
            if (_compressByteArray == null) {
                _compressByteArray = new byte[_maxCompressionLen];
            }
            // copy bufferToCompress contents to _compressByteArray
            for (int i = 0; i < lenToCompress; i++) {
                _compressByteArray[i] = bufferToCompress.get(dataStartPos + i);
            }
            _compressedBytesOutputStream.reset();
            try {
                // write bytes to compress
                _deflaterOutputStream.write(_compressByteArray, 0, lenToCompress);
                // flush bytes to compress
                _deflaterOutputStream.flush();
            } catch (Exception e) {
                throw new CompressorException(e.getLocalizedMessage());
            }
            // get compressed bytes
            _compressedBytes = _compressedBytesOutputStream.toByteArray();

            return _compressedBytes.length;
        }

        int getMaxCompressedLength(int numBytesToCompress) {
            return (numBytesToCompress + 13);
        }

        byte[] compressedData() {
            return _compressedBytes;
        }

        int compressedDataLength() {
            return _compressedBytes.length;
        }
    }

    class SocketHelperMock extends SocketHelper {
        private ByteBuffer _networkBuffer = ByteBuffer.allocate(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        private int _lastRead = 0;
        private int _maxWrite = -1;

        SocketHelperMock() {
        }

        @Override
        public int read(ByteBuffer dst) {
            int byteRead = 0;

            for (int index = _lastRead; index < maxWrite(); index++) {
                dst.put(_networkBuffer.get(index));

                ++byteRead;
            }

            _lastRead = byteRead;

            return byteRead;
        }

        public ByteBuffer networkBuffer() {
            return _networkBuffer;
        }

        public void maxWrite(int maxWrite) {
            _maxWrite = maxWrite;
        }

        @Override
        public long write(ByteBuffer[] srcs, int offset, int length) {
            long byteWritten = 0;

            for (int index = offset; index < length; index++) {
                byteWritten += (srcs[index].limit() - srcs[index].position());

                _networkBuffer.put(srcs[index]);
            }

            return byteWritten;
        }

        @Override
        public int write(ByteBuffer src) throws IOException {
            int bytesWritten = src.limit() - src.position();
            _networkBuffer.put(src);
            return bytesWritten;
        }

        public void clear() {
            _lastRead = 0;
            _maxWrite = -1;
            networkBuffer().clear();
        }

        private int maxWrite() {
            if (_maxWrite == -1) {
                return _networkBuffer.position();
            } else {
                return _maxWrite <= _networkBuffer.position() ? _maxWrite : _networkBuffer.position();
            }
        }

        @Override
        public void close() throws IOException {
        }
    }

    @Test
    public void readPartialRWFMessage() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), ripcMessageLength, 0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA);

        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());
        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        int maxRead = RWF_MSG_1.length() - 10;

        socketHelperMock.maxWrite(maxRead);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNull(readBuffer);
        assertEquals(maxRead, readArgs.readRetVal());
        assertEquals(maxRead, readArgs.bytesRead());

        socketHelperMock.maxWrite(-1);

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(ripcMessageLength - maxRead + HEADER_SIZE + FOOTER_SIZE, readArgs.bytesRead());
        assertEquals(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength + HEADER_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readOneRWFMessage() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), ripcMessageLength, 0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA);

        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());
        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.bytesRead());
        assertEquals(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength + HEADER_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readTwoRWFMessagesSameBuffer() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), ripcMessageLength, 0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA);

        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());

        int newPosition = socketHelperMock.networkBuffer().position() + FOOTER_SIZE;
        socketHelperMock.networkBuffer().position(newPosition);

        short ripcMessageLength2 = (short) (ripcHdrLength + RWF_MSG_2.length());

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), ripcMessageLength2, newPosition, false);

        socketHelperMock.networkBuffer().position(newPosition + HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength2);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA);

        socketHelperMock.networkBuffer().put(RWF_MSG_2.getBytes());
        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals((HEADER_SIZE + FOOTER_SIZE) * 2 + ripcMessageLength2, readArgs.readRetVal());
        assertEquals((HEADER_SIZE + FOOTER_SIZE) * 2 + ripcMessageLength + ripcMessageLength2, readArgs.bytesRead());
        assertEquals(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength + HEADER_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(0, readArgs.bytesRead());
        assertEquals(ripcHdrLength + RWF_MSG_2.length() + HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(HEADER_SIZE + ripcMessageLength + ripcHdrLength + HEADER_SIZE + FOOTER_SIZE,
                readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readTwoRWFMessagesDifferentBuffer() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), ripcMessageLength, 0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA);

        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());

        int newPosition = socketHelperMock.networkBuffer().position() + FOOTER_SIZE;
        socketHelperMock.networkBuffer().position(newPosition);

        short ripcMessageLength2 = (short) (ripcHdrLength + RWF_MSG_2.length());

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), ripcMessageLength2, newPosition, false);

        socketHelperMock.networkBuffer().position(newPosition + HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength2);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA);

        socketHelperMock.networkBuffer().put(RWF_MSG_2.getBytes());
        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        socketHelperMock.maxWrite(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.bytesRead());
        assertEquals(ripcMessageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength + HEADER_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        socketHelperMock.maxWrite(-1);

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(ripcMessageLength2 + HEADER_SIZE + FOOTER_SIZE, readArgs.bytesRead());
        assertEquals(ripcMessageLength2 + HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(ripcHdrLength + HEADER_SIZE, readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readPackingRWFMessages() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(),
                0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().put(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER);

        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(1 + HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit() + HEADER_SIZE + FOOTER_SIZE,
                readArgs.bytesRead());
        assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit() + HEADER_SIZE + FOOTER_SIZE,
                readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_HDR_SIZE + RsslSocketChannel.RIPC_PACKED_HDR_SIZE + HEADER_SIZE,
                readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(0, readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_HDR_SIZE + RsslSocketChannel.RIPC_PACKED_HDR_SIZE
                + RWF_MSG_1.length() + RsslSocketChannel.RIPC_PACKED_HDR_SIZE + HEADER_SIZE, readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readCompressedPackingRWFMessages() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();
        channel._compressor = new ZlibCompressor();
        channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
        channel._sessionCompLowThreshold = 30;

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);

        zlibCompressor.compress(PACKED_RWF_MSG_1_MSG_2_ByteBuffer, 0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.limit());

        int messageLength = zlibCompressor.compressedDataLength() + RsslSocketChannel.RIPC_HDR_SIZE;

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), messageLength, 0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort((short) messageLength);
        socketHelperMock.networkBuffer().put((byte) (RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION));
        socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());

        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(1 + HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(messageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.bytesRead());
        assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit() + HEADER_SIZE + FOOTER_SIZE,
                readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(0, readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_PACKED_HDR_SIZE + RWF_MSG_1.length() + RsslSocketChannel.RIPC_PACKED_HDR_SIZE,
                readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readCompressedRWFMessage() {
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();
        channel._compressor = new ZlibCompressor();
        channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
        channel._sessionCompLowThreshold = 30;
        channel._decompressBuffer = new TransportBufferImpl(channel._internalMaxFragmentSize);
        channel._protocolFunctions = new RipcProtocolFunctions(channel);

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);

        zlibCompressor.compress(RWF_MSG_1_ByteBuffer, 0, RWF_MSG_1_ByteBuffer.limit());

        int messageLength = zlibCompressor.compressedDataLength() + RsslSocketChannel.RIPC_HDR_SIZE;

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), messageLength, 0, false);

        socketHelperMock.networkBuffer().position(HEADER_SIZE);
        socketHelperMock.networkBuffer().putShort((short) messageLength);
        socketHelperMock.networkBuffer().put((byte) (Ripc.Flags.COMPRESSION));
        socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());

        socketHelperMock.networkBuffer().position(socketHelperMock.networkBuffer().position() + FOOTER_SIZE);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.readRetVal());
        assertEquals(messageLength + HEADER_SIZE + FOOTER_SIZE, readArgs.bytesRead());
        assertEquals(RWF_MSG_1_ByteBuffer.limit() + RsslSocketChannel.RIPC_HDR_SIZE + HEADER_SIZE + FOOTER_SIZE,
                readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(0, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    public void readFragmentedRWFMessages(boolean compressed) {
        Transport._globalLock = new DummyLock();
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();
        channel._isProviderHTTP = true;
        channel._http = true;

        if (compressed) {
            channel._compressor = new ZlibCompressor();
            channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
            channel._sessionCompLowThreshold = 30;
        }

        channel._channelInfo._maxFragmentSize = CHANNEL_INFO_MAX_FRAGMENT_SIZE;
        channel._internalMaxFragmentSize = MAX_FRAGMENT_SIZE;

        channel._ipcProtocol = new Ripc14Protocol();
        channel._ipcProtocol.channel(channel);

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
        channel._readBufStateMachine.ripcVersion(Ripc.RipcVersions.VERSION14);

        channel.createBigBufferPool(channel._internalMaxFragmentSize);

        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);

        channel._scktChannel = new SocketHelperMock();
        channel._providerHelper = new RsslHttpSocketChannelProvider(channel);

        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        TransportBuffer writeBuffer = null;

        writeBuffer = channel.getBuffer(RWF_MSG_2.length(), false, error);

        assertNotNull(writeBuffer);

        assertEquals(RWF_MSG_2.length(), writeBuffer.length());

        writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);

        writeBuffer.data().put(RWF_MSG_2.getBytes());

        int retValue = 0;

        retValue = channel.write(writeBuffer, writeArgs, error);

        assertEquals(0, retValue);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNull(readBuffer);

        assertEquals(compressed ? 69 : 97, readArgs.readRetVal());
        assertEquals(compressed ? 114 : 152, readArgs.bytesRead());
        assertEquals(compressed ? 71 : 63, readArgs.uncompressedBytesRead());

        if (compressed) {
            readBuffer = channel.read(readArgs, error);

            assertNotNull(readBuffer);
            assertEquals(8, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead());
            assertEquals(67, readArgs.uncompressedBytesRead());

        } else {

            readBuffer = channel.read(readArgs, error);
            assertNull(readBuffer);

            assertEquals(34, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead());
            assertEquals(63, readArgs.uncompressedBytesRead());

            readBuffer = channel.read(readArgs, error);

            assertNotNull(readBuffer);
            assertEquals(8, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead());
            assertEquals(26, readArgs.uncompressedBytesRead());
        }

        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(0, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readPingOrPongFrames(){
        RsslSocketChannel channel = new RsslHttpSocketChannel();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._protocolType = Codec.RWF_PROTOCOL_TYPE;
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        encodeHttpHeaderAndFooter(socketHelperMock.networkBuffer(), 0, 0, true);

        int bytesRead = 2;

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNull(readBuffer);

        assertEquals(TransportReturnCodes.READ_PING, readArgs.readRetVal());
        assertEquals(bytesRead + HEADER_SIZE, readArgs.bytesRead());
        assertEquals(HEADER_SIZE + FOOTER_SIZE, readArgs.uncompressedBytesRead());

        socketHelperMock.networkBuffer().clear();

        readBuffer = channel.read(readArgs, error);

        assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
        assertEquals(0, readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());
    }

    @Test
    public void readCompressedRWFMessage_Server() {
        readCompressedRWFMessage();
    }

    @Test
    public void readFragmentedRWFMessage_() {
        readFragmentedRWFMessages(false);
    }

    @Test
    public void readFragmentedRWFMessage_Compressed() {
        readFragmentedRWFMessages(true);
    }

    private void encodeHttpHeaderAndFooter(ByteBuffer msgBuffer, int messageLength, int initialPosition, boolean isPingPong) {
        if(isPingPong) {
            byte[] headerData = {(byte) 0x33, (byte) 0x0D, (byte) 0x0A, (byte) 0,
                    (byte) 3, (byte) 2, (byte) 0x0D, (byte) 0x0D};
            msgBuffer.put(headerData);
        } else {
            byte[] commonData = {(byte) 0x0D, (byte) 0x0A};
            msgBuffer.position(initialPosition);
            msgBuffer.putInt(HEADER_SIZE);
            msgBuffer.put(commonData);
            msgBuffer.position(messageLength + HEADER_SIZE);
            msgBuffer.put(commonData);
        }
    }
}