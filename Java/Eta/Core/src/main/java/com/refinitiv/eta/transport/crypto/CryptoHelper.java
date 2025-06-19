/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLEngineResult;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

// CryptoHelper is a partial Java SocketChannel implementation that uses
// the Java SSLEngine to do encryption and decryption of application data
//
//
// Flow of encrypted/decrypted application data and handshaking data
// through the Java SSLEngine and the 4 ByteBuffers it requires:
//
//         -------------------application data---------------------
//                    ^                               |
//   (decrypted data) |                               | (data not yet encrypted)
//                    |                               v
//           +------------------+            +------------------+
//           |  getAppRecvBuffer()  |            |  getAppSendBuffer()  |
//           +------------------+            +------------------+
//                    ^                               |
//   (decrypted data) |                               | (data not yet encrypted)
//                    |                               v
//         +----------+-------------------------------+------------+
//         |+---------+-------------------------------+-----------+|
//         ||         ^                               |           ||
//         ||         |           SSLEngine           |           ||
//         ||         |                               v           ||
//         ||      unwrap()---->   ..........   ---->wrap()       ||
//         ||         ^          handshake info.      |           ||
//         ||         |                               |           ||
//         ||         |                               v           ||
//         |+---------+-------------------------------+-----------+|
//         +----------+-------------------------------+------------+
//                    ^                               |
//   (encrypted data) |                               | (encrypted data)
//                    |                               v
//           +------------------+            +------------------+
//           |  getNetRecvBuffer()  |            |  getNetSendBuffer()  |
//           +------------------+            +------------------+
//                    ^                               |
//   (encrypted data) |                               | (encrypted data)
//                    |                               |
//                    |                               v
//         -----------------------network data---------------------
//
// 'application data' is also known as plaintext or cleartext
// 'network data' can be handshaking data and/or encrypted data (also known as ciphertext)
//
//
// the 'application data' is the destination buffers used in RsslEncryptedSocketChannel
// and
// the 'network data' is the data from each SocketChannel read after each RsslEncryptedSocketChannel::read(ByteBuffer dst)
// or from each SocketChannel write after each RsslSocketChannel::write()

public class CryptoHelper {
    public final int BUFFER_OVERFLOW_ON_READ = Integer.MIN_VALUE;
    private final SSLEngineFactory _sslEngineFactory;
    private final CryptoHandshakeFactory _handshakeManager;
    public SSLEngine _engine;
    CryptoHandshake _handshake;
    private BufferData _bufferData;
    private SocketChannel _socketChannel;
    private int readCount; // number of bytes read after each read(ByteBuffer[], offset, length) call

    CryptoHelper(SSLEngineFactory sslEngineFactory, CryptoHandshakeFactory handshakeManager)
    {
        _sslEngineFactory = sslEngineFactory;
        _handshakeManager = handshakeManager;
    }

    private ByteBuffer netSend()
    {
        return _bufferData.netSend.buffer();
    }
    private ByteBuffer appSend()
    {
        return _bufferData.appSend.buffer();
    }
    private ByteBuffer netRecv()
    {
        return _bufferData.netRecv.buffer();
    }
    private ByteBuffer appRecv()
    {
        return _bufferData.appRecv.buffer();
    }

    private ByteBuffer _netSendBuf;
    private ByteBuffer _appSendBuf;
    private ByteBuffer _netRecvBuf;
    private ByteBuffer _appRecvBuf;

    public BufferData getBufferData()
    {
        return _bufferData;
    }

    public void initializeEngine(SocketChannel socketChannel) throws IOException
    {
        _socketChannel = socketChannel;
        _engine = _sslEngineFactory.create();
        _bufferData = BufferData.create(_engine.getSession());
        _netSendBuf = netSend();
        _appSendBuf = appSend();
        _netRecvBuf = netRecv();
        _appRecvBuf = appRecv();
        _handshake = _handshakeManager.create(_engine, _bufferData);
    }

    // Implementation of SocketChannel::read(ByteBuffer dst)
    public final int read(final ByteBuffer dst) throws IOException
    {
        checkEngine();
        readCount = 0;

        if (_appRecvBuf.position() > 0) //any data received during handshake renegotiation? should be a rare case
        {
            _appRecvBuf.flip();
            readCount += copyBytes(_appRecvBuf, dst);
        }

        int decryptCount = 0;
        if (dst.hasRemaining()) {
            if (dst.capacity() >= _engine.getSession().getApplicationBufferSize()) {
                decryptCount = decryptNetworkData(dst, true);
                if (decryptCount != BUFFER_OVERFLOW_ON_READ)
                    readCount += decryptCount;
            } else { //if the destination buffer is small for the engine to unwrap data, use the intermediate buffer
                decryptNetworkData(_appRecvBuf, false);
                _appRecvBuf.flip();
                readCount += copyBytes(_appRecvBuf, dst);
            }
        }

        if ((_engine.getHandshakeStatus() != SSLEngineResult.HandshakeStatus.FINISHED)
                && (_engine.getHandshakeStatus() != SSLEngineResult.HandshakeStatus.NOT_HANDSHAKING))
        {
            performHandshake();
        }

        return decryptCount != BUFFER_OVERFLOW_ON_READ ? readCount : decryptCount;
    }

    private int copyBytes(ByteBuffer source, ByteBuffer dest) {
        int count = 0;
        while (source.hasRemaining() && dest.hasRemaining()) {
            dest.put(source.get());
            count++;
        }
        source.compact();
        return count;
    }

    // Implementation of SocketChannel::write(ByteBuffer src)
    // (src should be immediately readable)
    public final int write(final ByteBuffer src) throws IOException
    {
        boolean canWrite = true;
        int writeCount = 0;

        checkEngine();

        while (src.hasRemaining() && canWrite)
        {
            SSLEngineResult result = _engine.wrap(src, _netSendBuf);
            if (result.getStatus() == SSLEngineResult.Status.OK)
            {
                writeCount += result.bytesConsumed();
            } else if (result.getStatus() == SSLEngineResult.Status.CLOSED) {
                break;
            }

            _netSendBuf.flip();
            try
            {
                while (_netSendBuf.hasRemaining())
                {
                    if (_socketChannel.write(_netSendBuf) <= 0)
                    {
                        canWrite = false;
                        break;
                    }
                }
            }
            catch (IOException e)
            {
                _netSendBuf.clear();
                canWrite = false;
            }
            _netSendBuf.compact();

            if ((_engine.getHandshakeStatus() != SSLEngineResult.HandshakeStatus.FINISHED)
                    && (_engine.getHandshakeStatus() != SSLEngineResult.HandshakeStatus.NOT_HANDSHAKING))
            {
                performHandshake();
            }
        }

        // return the number of bytes that we used from the src buffer.
        return writeCount;
    }

    // Implementation of AbstractSelectableChannel::write(final ByteBuffer[] srcs, final int offset, final int length)
    public final long write(final ByteBuffer[] srcs, final int offset, final int length) throws IOException
    {
        long writeCount = 0;
        for (int i = offset; i < offset + length; i++)
        {
            writeCount += write(srcs[i]);
            if (srcs[i].hasRemaining())
                break;
        }

        return writeCount;
    }

    // We must send the appropriate alerts to indicate to the peer that we intend to close the TLS/SSL connection.
    public void cleanup() throws IOException
    {
        if (_bufferData != null && _bufferData.appSend != null && _bufferData.appSend.buffer() != null){
            _bufferData.appSend.buffer().clear();
        }

        if (_engine != null)
            _engine.closeOutbound();
        _engine = null;
    }

    // Process the encrypted or decrypted data or process the handshake information
    void performHandshake() throws IOException
    {
        checkEngine();
        _handshake.perform(_socketChannel);
    }

    private int decryptNetworkData(ByteBuffer dest, boolean checkBufferOverflow) throws IOException
    {
        int count = 0;
        SSLEngineResult result;
        checkEngine();

        long bytesReadFromChannel = readFromChannel();

        if (bytesReadFromChannel == -1)
            throw new IOException("Tunnel Channel disconnected");

        // make NetRecvBuffer readable
        _netRecvBuf.flip();

        do {
            result = _engine.unwrap(_netRecvBuf, dest);
            checkUnwrapEngineResultStatus(result);
            count += result.bytesProduced();
        }  while (_netRecvBuf.hasRemaining() && (result.getStatus() != SSLEngineResult.Status.BUFFER_UNDERFLOW)
                && (result.getStatus() != SSLEngineResult.Status.BUFFER_OVERFLOW));

        // setup _bufferData.getNetRecvBuffer() back to writable
        _netRecvBuf.compact();

        if (checkBufferOverflow && result.getStatus() == SSLEngineResult.Status.BUFFER_OVERFLOW)
        {
            return BUFFER_OVERFLOW_ON_READ;
            //return count > 0 ? count : BUFFER_OVERFLOW_ON_READ;
        }

        return count;
    }

    private void checkUnwrapEngineResultStatus(SSLEngineResult result) throws IOException
    {
        switch (result.getStatus())
        {
            case CLOSED:
                throw new IOException("Tunnel Channel Closed");
                // case BUFFER_OVERFLOW:
                // either _bufferData.getAppRecvBuffer() is too small for a handshake message,
                // or the client is not properly emptying the application buffer.
                // throw new IOException("Tunnel Channel BufferOverflow");
                // case BUFFER_UNDERFLOW:
                // we need more encrypted data, but we didn't manage to read any from the channel
                // break;
                // case OK:
                // we got all encrypted data?
                // break;
            default:
                break;
        }
    }

    // Attempt to fill _bufferData.getNetRecvBuffer() from the channel and return the number of bytes filled
    private int readFromChannel() throws IOException
    {
        return _socketChannel.read(_netRecvBuf);
    }

    // Start TLS/SSL handshaking with the server peer.
    public void doHandshake() throws IOException
    {
        checkEngine();
        _engine.beginHandshake();
        performHandshake();
    }

    private void checkEngine()
    {
        if(_engine == null){
            throw new IllegalStateException("SSL engine is not initialized");
        }
    }

    public String getActiveTLSVersion()
    {
        if (_engine == null)
            throw new IllegalStateException("SSL engine is not initialized");

        return _engine.getSession().getProtocol();
    }
}
