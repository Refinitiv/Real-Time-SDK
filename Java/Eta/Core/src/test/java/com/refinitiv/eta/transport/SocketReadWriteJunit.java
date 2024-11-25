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

public class SocketReadWriteJunit {
    private static final int MAX_FRAGMENT_SIZE = 50;
    private static final int CHANNEL_INFO_MAX_FRAGMENT_SIZE = 31;

    String RWF_MSG_1 = "123456RWFEncodedBufferxxxfweirfsdfkjl";
    String RWF_MSG_2 = "999999999999988888888888888888998eqwewqerqwer9qwtqfafjakefjaskerjfra;9345353453433243324-fasdfj0000000NULL";

    ByteBuffer RWF_MSG_1_ByteBuffer = ByteBuffer.allocate(RWF_MSG_1.length());
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
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        /* Set RIPC header */
        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */

        /* Set message body */
        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());

        int maxRead = RWF_MSG_1.length() - 10;

        socketHelperMock.maxWrite(maxRead); /* Can read data only partial message */

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNull(readBuffer);
        assertEquals(maxRead, readArgs.readRetVal());  // More data to read
        assertEquals(maxRead, readArgs.bytesRead());

        socketHelperMock.maxWrite(-1); /* Read the remaining data from network */

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(ripcMessageLength - maxRead, readArgs.bytesRead());
        assertEquals(ripcMessageLength, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readOneRWFMessage() {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        /* Set RIPC header */
        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */

        /* Set message body */
        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(ripcMessageLength, readArgs.bytesRead());
        assertEquals(ripcMessageLength, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readTwoRWFMessagesSameBuffer() {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        /* Set RIPC header */
        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */

        /* Set message body */
        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());

        short ripcMessageLength2 = (short) (ripcHdrLength + RWF_MSG_2.length());

        /* Set RIPC header */
        socketHelperMock.networkBuffer().position(ripcMessageLength);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength2);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */

        /* Set message body */
        socketHelperMock.networkBuffer().put(RWF_MSG_2.getBytes());

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(ripcMessageLength2, readArgs.readRetVal());  // More data to read
        assertEquals(ripcMessageLength + ripcMessageLength2, readArgs.bytesRead());
        assertEquals(ripcMessageLength, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(0, readArgs.bytesRead());
        assertEquals(ripcHdrLength + RWF_MSG_2.length(), readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(ripcMessageLength + ripcHdrLength, readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readTwoRWFMessagesDifferentBuffer() {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        short ripcHdrLength = 3;
        short ripcMessageLength = (short) (ripcHdrLength + RWF_MSG_1.length());

        /* Set RIPC header */
        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */

        /* Set message body */
        socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());

        short ripcMessageLength2 = (short) (ripcHdrLength + RWF_MSG_2.length());

        /* Set RIPC header */
        socketHelperMock.networkBuffer().position(ripcMessageLength);
        socketHelperMock.networkBuffer().putShort(ripcMessageLength2);
        socketHelperMock.networkBuffer().put((byte) RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */

        /* Set message body */
        socketHelperMock.networkBuffer().put(RWF_MSG_2.getBytes());

        socketHelperMock.maxWrite(ripcMessageLength);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(ripcMessageLength, readArgs.bytesRead());
        assertEquals(ripcMessageLength, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(ripcHdrLength, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        socketHelperMock.maxWrite(-1);

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(ripcMessageLength2, readArgs.bytesRead());
        assertEquals(ripcMessageLength2, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(ripcHdrLength, readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    public void writeOneRWFMessage(boolean directWrite) {
        Transport._globalLock = new DummyLock();
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        TransportBuffer writeBuffer = channel.getBuffer(RWF_MSG_1.length(), false, error);

        assertNotNull(writeBuffer);

        if (directWrite) {
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
        }

        writeBuffer.data().put(RWF_MSG_1.getBytes());

        int retValue = channel.write(writeBuffer, writeArgs, error);

        if (!directWrite) {
            retValue = channel.flush(error);
        }

        assertEquals(0, retValue);  // No bytes pending to flush
        assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE, writeArgs.bytesWritten());
        assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE, writeArgs.uncompressedBytesWritten());

        ByteBuffer msgBuffer = ByteBuffer.allocate(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE);
        msgBuffer.put(socketHelperMock.networkBuffer().array(), 0, RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE);
        msgBuffer.rewind();

        /* Checks the RIPC header */
        assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE, msgBuffer.getShort());
        assertEquals(RsslSocketChannel.IPC_DATA, msgBuffer.get());

        /* Checks RWF message */
        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    public void writeRWFPackagingMessages(boolean directWrite, boolean checkEndOfBuffer) {
        Transport._globalLock = new DummyLock();
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        TransportBuffer writeBuffer = channel.getBuffer(RWF_MSG_1.length() + RWF_MSG_2.length() + 2, true, error);

        assertNotNull(writeBuffer);

        if (directWrite) {
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
        }

        writeBuffer.data().put(RWF_MSG_1.getBytes());

        int retValue = channel.packBuffer(writeBuffer, error);

        /* Checks to ensure there is enough buffer to pack another message */
        assertEquals(RWF_MSG_2.length(), retValue);

        writeBuffer.data().put(RWF_MSG_2.getBytes());

        if (checkEndOfBuffer) {
            /* Checks the error handling when there is no buffer left */
            retValue = channel.packBuffer(writeBuffer, error);
            assertEquals(0, retValue);
        }

        retValue = channel.write(writeBuffer, writeArgs, error);

        if (!directWrite) {
            retValue = channel.flush(error);
        }

        int messageLength = RWF_MSG_1.length() + RWF_MSG_2.length() + 4;
        int payloadLength = messageLength + RsslSocketChannel.RIPC_HDR_SIZE;

        assertEquals(0, retValue);  // No bytes pending to flush
        ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength);
        msgBuffer.put(socketHelperMock.networkBuffer().array(), 0, payloadLength);
        msgBuffer.rewind();

        /* Checks the RIPC header */
        assertEquals(payloadLength, msgBuffer.getShort());
        assertEquals(RsslSocketChannel.IPC_DATA | RsslSocketChannel.IPC_PACKING, msgBuffer.get());

        assertEquals(0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    public void writeCompressedRWFMessages(boolean directWrite, boolean packing) {
        Transport._globalLock = new DummyLock();
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        channel._compressor = new ZlibCompressor();
        channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
        channel._sessionCompLowThreshold = 30;

        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);

        ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);
        ByteBuffer decompressedBuffer = ByteBuffer.allocate(6144);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        TransportBuffer writeBuffer = null;

        if (packing) {
            writeBuffer = channel.getBuffer(RWF_MSG_1.length() + RWF_MSG_2.length() + 2, true, error);
        } else {
            writeBuffer = channel.getBuffer(RWF_MSG_1.length(), false, error);
        }

        assertNotNull(writeBuffer);

        if (directWrite) {
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
        }

        writeBuffer.data().put(RWF_MSG_1.getBytes());

        int retValue = 0;

        if (packing) {
            retValue = channel.packBuffer(writeBuffer, error);

            /* Checks to ensure there is enough buffer to pack another message */
            assertEquals(RWF_MSG_2.length(), retValue);

            writeBuffer.data().put(RWF_MSG_2.getBytes());
        }

        retValue = channel.write(writeBuffer, writeArgs, error);

        if (!directWrite) {
            retValue = channel.flush(error);
        }

        int messageLength = 0;

        if (packing) {
            messageLength = zlibCompressor.compress(PACKED_RWF_MSG_1_MSG_2_ByteBuffer, 0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.limit());
        } else {
            messageLength = zlibCompressor.compress(RWF_MSG_1_ByteBuffer, 0, RWF_MSG_1.length());
        }

        int payloadLength = messageLength + RsslSocketChannel.RIPC_HDR_SIZE;

        assertEquals(0, retValue);  // No bytes pending to flush

        if (packing) {
            assertEquals(RWF_MSG_1.length() + RWF_MSG_2.length() + 4 + RsslSocketChannel.RIPC_HDR_SIZE, writeArgs.uncompressedBytesWritten());
        } else {
            assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE, writeArgs.uncompressedBytesWritten());
        }

        ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength);
        msgBuffer.put(socketHelperMock.networkBuffer().array(), 0, payloadLength);
        msgBuffer.rewind();

        int numberOfBytpes = 0;

        numberOfBytpes = zlibCompressor.decompress(msgBuffer, RsslSocketChannel.RIPC_HDR_SIZE, decompressedBuffer, messageLength);

        /* Checks the RIPC header */
        assertEquals(payloadLength, msgBuffer.getShort());

        /* Checks the actual message length */
        if (packing) {
            assertEquals(RWF_MSG_1.length() + RWF_MSG_2.length() + 4, numberOfBytpes); // 4 is additional length for packing with the RWF protocol
            assertEquals(RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION, msgBuffer.get());
        } else {
            assertEquals(RWF_MSG_1.length(), numberOfBytpes);
            assertEquals(Ripc.Flags.COMPRESSION, msgBuffer.get());
        }
    }

    @Test
    public void readPackingRWFMessages() {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().put(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER);

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(1, readArgs.readRetVal());  // Has more data to read
        assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(), readArgs.bytesRead());
        assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(), readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_HDR_SIZE + RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(0, readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_HDR_SIZE + RsslSocketChannel.RIPC_PACKED_HDR_SIZE
                + RWF_MSG_1.length() + RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readCompressedPackingRWFMessages() {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();
        channel._compressor = new ZlibCompressor();
        channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
        channel._sessionCompLowThreshold = 30;

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);

        zlibCompressor.compress(PACKED_RWF_MSG_1_MSG_2_ByteBuffer, 0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.limit());

        int messageLength = zlibCompressor.compressedDataLength() + RsslSocketChannel.RIPC_HDR_SIZE;

        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().putShort((short) messageLength);
        socketHelperMock.networkBuffer().put((byte) (RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION));
        socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(1, readArgs.readRetVal());  // Has more data to read
        assertEquals(messageLength, readArgs.bytesRead());
        assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(), readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));

        readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // No more data to read
        assertEquals(0, readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(RsslSocketChannel.RIPC_PACKED_HDR_SIZE + RWF_MSG_1.length() + RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());

        msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void readCompressedRWFMessage() {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();
        channel._compressor = new ZlibCompressor();
        channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
        channel._sessionCompLowThreshold = 30;
        channel._decompressBuffer = new TransportBufferImpl(channel._internalMaxFragmentSize);
        channel._protocolFunctions = new RipcProtocolFunctions(channel);

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);

        zlibCompressor.compress(RWF_MSG_1_ByteBuffer, 0, RWF_MSG_1_ByteBuffer.limit());

        int messageLength = zlibCompressor.compressedDataLength() + RsslSocketChannel.RIPC_HDR_SIZE;

        socketHelperMock.networkBuffer().position(0);
        socketHelperMock.networkBuffer().putShort((short) messageLength);
        socketHelperMock.networkBuffer().put((byte) (Ripc.Flags.COMPRESSION));
        socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNotNull(readBuffer);
        assertEquals(0, readArgs.readRetVal());  // Has more data to read
        assertEquals(messageLength, readArgs.bytesRead());
        assertEquals(RWF_MSG_1_ByteBuffer.limit() + RsslSocketChannel.RIPC_HDR_SIZE, readArgs.uncompressedBytesRead());
        assertEquals(RWF_MSG_1.length(), readBuffer.length());
        assertEquals(0, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }

    @Test
    public void executePingPongInteraction() {
        Transport._globalLock = new DummyLock();
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        final SocketHelperMock socket = new SocketHelperMock();
        RsslSocketChannel channel = createSocketChannelAndMakeActive(socket);
        RsslSocketChannel serverChannel = createSocketChannelAndMakeActive(socket);
        assertFalse(error.text(), channel.ping(error) < TransportReturnCodes.SUCCESS);

        //read ping msg.
        serverChannel.read(readArgs, error);
        assertEquals(TransportReturnCodes.READ_PING, readArgs.readRetVal());

        socket.clear();

        assertFalse(testPong(serverChannel, error) < TransportReturnCodes.FAILURE);

        //read pong msg.
        channel.read(readArgs, error);
        //assert that client received expected content PONG
        assertEquals(TransportReturnCodes.READ_PING, readArgs.readRetVal());

    }

    @Test
    public void givenActiveConnection_whenCloseConnection_thenClose() {
        Transport._globalLock = new DummyLock();
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        final SocketHelperMock socket = new SocketHelperMock();
        RsslSocketChannel channel = createSocketChannelAndMakeActive(socket);
        RsslSocketChannel serverChannel = createSocketChannelAndMakeActive(socket);
        assertFalse(error.text(), channel.close(error) < TransportReturnCodes.SUCCESS);

        TransportBuffer transportBuffer = serverChannel.read(readArgs, error);
        assertNull(transportBuffer);
        assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

        transportBuffer = channel.read(readArgs, error);
        assertNull(transportBuffer);
        assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
    }

    private RsslSocketChannel createSocketChannelAndMakeActive(SocketHelperMock socket) {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE) {
            @Override
            protected int closeSocketChannel(Error error) {
                return TransportReturnCodes.SUCCESS;
            }
        };

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.buffer());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
        channel._scktChannel = socket;
        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);
        return channel;
    }

    private int testPong(RsslSocketChannel channel, Error error) {
        try {
            return channel._protocolFunctions.pong(error);
        } catch (IOException e) {
        }
        return TransportReturnCodes.FAILURE;
    }

    public void writeFragmentedRWFMessage(boolean directWrite) {
        Transport._globalLock = new DummyLock();
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);

        /* Overrides the default max fragmentation size */
        channel._channelInfo._maxFragmentSize = CHANNEL_INFO_MAX_FRAGMENT_SIZE;
        channel._internalMaxFragmentSize = MAX_FRAGMENT_SIZE;

        channel._ipcProtocol = new Ripc14Protocol();
        channel._ipcProtocol.channel(channel);

        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
        channel._readBufStateMachine.ripcVersion(Ripc.RipcVersions.VERSION14);

        channel.createBigBufferPool(channel._internalMaxFragmentSize);

        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        TransportBuffer writeBuffer = null;

        writeBuffer = channel.getBuffer(RWF_MSG_2.length(), false, error);

        assertNotNull(writeBuffer);

        assertEquals(RWF_MSG_2.length(), writeBuffer.length());

        if (directWrite) {
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
        }

        writeBuffer.data().put(RWF_MSG_2.getBytes());

        int retValue = 0;

        retValue = channel.write(writeBuffer, writeArgs, error);

        if (!directWrite) {
            retValue = channel.flush(error);
        }

        int readSize = 50;
        int fragId = BigBuffer._ID;

        int payloadLength = readSize - TransportBufferImpl._firstFragmentHeaderLength;
        int nextReadIndex = TransportBufferImpl._firstFragmentHeaderLength + payloadLength;

        assertEquals(0, retValue);  // No bytes pending to flush

        int bytesWritten = 128;

        assertEquals(bytesWritten, writeArgs.bytesWritten());
        assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());

        int ripcMsgPayload = payloadLength;

        ByteBuffer msgBuffer = ByteBuffer.allocate(readSize);
        msgBuffer.put(socketHelperMock.networkBuffer().array(), 0, readSize);
        msgBuffer.rewind();

        /* Checks the first RIPC fragmented header */
        assertEquals(readSize, msgBuffer.getShort());
        assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer.get());
        assertEquals(TransportBufferImpl.FRAGMENT_HEADER_RIPC_FLAGS, msgBuffer.get());
        assertEquals(RWF_MSG_2.length(), msgBuffer.getInt());
        assertEquals(fragId, msgBuffer.getShort());

        RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload);

        // Checks the data of the first fragmented message
        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));

        payloadLength = readSize - TransportBufferImpl._nextFragmentHeaderLength;
        nextReadIndex += TransportBufferImpl._nextFragmentHeaderLength + payloadLength;

        assertEquals(bytesWritten, writeArgs.bytesWritten());

        assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());

        int ripcMsgPayload2 = payloadLength;

        ByteBuffer msgBuffer2 = ByteBuffer.allocate(readSize);
        msgBuffer2.put(socketHelperMock.networkBuffer().array(), nextReadIndex - readSize, readSize);
        msgBuffer2.rewind();

        /* Checks the next RIPC fragmented header */
        assertEquals(readSize, msgBuffer2.getShort());
        assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer2.get());
        assertEquals(TransportBufferImpl.FRAGMENT_RIPC_FLAGS, msgBuffer2.get());
        assertEquals(fragId, msgBuffer2.getShort());

        RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload + ripcMsgPayload2);
        RWF_MSG_2_ByteBuffer.position(ripcMsgPayload);

        // Checks the data of the next fragmented message
        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer2));

        readSize = 28;

        payloadLength = readSize - TransportBufferImpl._nextFragmentHeaderLength;
        nextReadIndex += TransportBufferImpl._nextFragmentHeaderLength + payloadLength;

        assertEquals(bytesWritten, writeArgs.bytesWritten());

        assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());

        int ripcMsgPayload3 = payloadLength;

        ByteBuffer msgBuffer3 = ByteBuffer.allocate(readSize);
        msgBuffer3.put(socketHelperMock.networkBuffer().array(), nextReadIndex - readSize, readSize);
        msgBuffer3.rewind();

        /* Checks the next RIPC fragmented header */
        assertEquals(readSize, msgBuffer3.getShort());
        assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer3.get());
        assertEquals(TransportBufferImpl.FRAGMENT_RIPC_FLAGS, msgBuffer3.get());
        assertEquals(fragId, msgBuffer3.getShort());

        RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload + ripcMsgPayload2 + ripcMsgPayload3);
        RWF_MSG_2_ByteBuffer.position(ripcMsgPayload + ripcMsgPayload2);

        // Checks the data of the next fragmented message
        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer3));
    }

    public void readFragmentedRWFMessages(boolean compressed) {
        Transport._globalLock = new DummyLock();
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        if (compressed) {
            channel._compressor = new ZlibCompressor();
            channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
            channel._sessionCompLowThreshold = 30;
        }

        /* Overrides the default max fragmentation size */
        channel._channelInfo._maxFragmentSize = CHANNEL_INFO_MAX_FRAGMENT_SIZE;
        channel._internalMaxFragmentSize = MAX_FRAGMENT_SIZE;

        channel._ipcProtocol = new Ripc14Protocol();
        channel._ipcProtocol.channel(channel);

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
        channel._readBufStateMachine.ripcVersion(Ripc.RipcVersions.VERSION14);

        channel.createBigBufferPool(channel._internalMaxFragmentSize);

        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(100);

        channel._scktChannel = new SocketHelperMock();

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

        assertEquals(compressed ? 82 : 78, readArgs.readRetVal());  // Has more data to read
        assertEquals(compressed ? 115 : 128, readArgs.bytesRead());
        assertEquals(50, readArgs.uncompressedBytesRead());

        if (compressed) {
            readBuffer = channel.read(readArgs, error);

            assertNull(readBuffer);
            assertEquals(32, readArgs.readRetVal());  // Has more data to read
            assertEquals(0, readArgs.bytesRead());
            assertEquals(0, readArgs.uncompressedBytesRead());

            readBuffer = channel.read(readArgs, error);

            assertNull(readBuffer);
            assertEquals(28, readArgs.readRetVal());  // Has more data to read
            assertEquals(0, readArgs.bytesRead());
            assertEquals(53, readArgs.uncompressedBytesRead());

            readBuffer = channel.read(readArgs, error);

            assertNotNull(readBuffer);
            assertEquals(0, readArgs.readRetVal());  // No more data to read
            assertEquals(0, readArgs.bytesRead());
            assertEquals(28, readArgs.uncompressedBytesRead());

        } else {

            readBuffer = channel.read(readArgs, error);
            assertNull(readBuffer);

            assertEquals(28, readArgs.readRetVal());  // Has more data to read
            assertEquals(0, readArgs.bytesRead());
            assertEquals(50, readArgs.uncompressedBytesRead());

            readBuffer = channel.read(readArgs, error);

            assertNotNull(readBuffer);
            assertEquals(0, readArgs.readRetVal());  // No more data to read

            assertEquals(0, readArgs.bytesRead());
            assertEquals(28, readArgs.uncompressedBytesRead());
        }

        assertEquals(RWF_MSG_2.length(), readBuffer.length());
        assertEquals(0, readBuffer.dataStartPosition());

        ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
        msgBuffer.put(readBuffer.data());
        msgBuffer.rewind();

        assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    public void readPingOrPong(boolean withPayload) {
        RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Error error = TransportFactory.createError();

        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        channel._protocolFunctions = new RipcProtocolFunctions(channel);
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);

        SocketHelperMock socketHelperMock = new SocketHelperMock();

        channel._scktChannel = socketHelperMock;

        String payload = withPayload ? "This is test payload of ping/pong." : "";

        socketHelperMock.networkBuffer().position(0);

        if (withPayload)
            socketHelperMock.networkBuffer().put(payload.getBytes());

        TransportBuffer readBuffer = channel.read(readArgs, error);

        assertNull(readBuffer);

        assertEquals(payload.length(), readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());

        socketHelperMock.networkBuffer().clear();

        readBuffer = channel.read(readArgs, error);

        assertEquals(0, readArgs.bytesRead());
        assertEquals(0, readArgs.uncompressedBytesRead());

    }

    @Test
    public void writeOneRWFMessage_Direct() {
        writeOneRWFMessage(true);
    }

    @Test
    public void writeOneRWFMessage() {
        writeOneRWFMessage(false);
    }

    @Test
    public void writeRWFPackagingMessages() {
        writeRWFPackagingMessages(false, false);
    }

    @Test
    public void writeRWFPackagingMessages_DirectWriteWhenCheckEndOfBuffer() {
        writeRWFPackagingMessages(true, true);
    }

    @Test
    public void writeRWFPackagingMessagesWhenCheckEndOfBuffer() {
        writeRWFPackagingMessages(false, true);
    }

    @Test
    public void writeRWFPackagingMessages_DirectWrite() {
        writeRWFPackagingMessages(true, false);
    }

    @Test
    public void writeCompressedOneRWFMessage() {
        writeCompressedRWFMessages(false, false);
    }

    @Test
    public void writeCompressedOneRWFMessage_DirectWrite() {
        writeCompressedRWFMessages(true, false);
    }

    @Test
    public void writeCompressedPackingRWFMessages_DirectWrite() {
        writeCompressedRWFMessages(true, true);
    }

    @Test
    public void writeCompressedPackingRWFMessage() {
        writeCompressedRWFMessages(false, true);
    }

    @Test
    public void writeFragmentedRWFMessage_DirectWrite() {
        writeFragmentedRWFMessage(true);
    }

    @Test
    public void writeFragmentedRWFMessage() {
        writeFragmentedRWFMessage(false);
    }

    @Test
    public void readFragmentedRWFNMessage_Compressed() {
        readFragmentedRWFMessages(true);
    }

    @Test
    public void readFragmentedRWFMessage() {
        readFragmentedRWFMessages(false);
    }

    @Test
    public void readPingAndPong_RWFProtocol() {
        readPingOrPong(false);
        readPingOrPong(true);
    }
}