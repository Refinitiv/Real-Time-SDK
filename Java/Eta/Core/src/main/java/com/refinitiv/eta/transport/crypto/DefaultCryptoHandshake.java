/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLEngineResult;
import java.io.IOException;
import java.nio.channels.SocketChannel;

class DefaultCryptoHandshake implements CryptoHandshake {
    private SSLEngine engine;
    private BufferData bufferData;

    public DefaultCryptoHandshake(SSLEngine engine, BufferData bufferData) {
        this.engine = engine;
        this.bufferData = bufferData;
    }

    public void perform(SocketChannel socketChannel) throws IOException
    {
        boolean handshakeComplete = false;
        while (!handshakeComplete)  {
            switch (engine.getHandshakeStatus()) {
                case FINISHED:
                case NOT_HANDSHAKING:
                    handshakeComplete = true;
                    break;
                case NEED_TASK:
                    Runnable task;
                    while ((task = engine.getDelegatedTask()) != null) {
                        task.run();
                    }
                    break;
                case NEED_UNWRAP:
                    handleUnwrap(socketChannel, engine, bufferData.netRecv, bufferData.appRecv);
                    break;
                case NEED_WRAP:
                    handleWrap(socketChannel, engine, bufferData.netSend, bufferData.appSend);
                    break;
                default:
                    throw new IOException("Invalid handshake status for DefaultCryptoHandshake.perform()");
            }
        }
    }

    private void handleUnwrap(SocketChannel socketChannel,
                                     SSLEngine engine,
                                     SafeBuffer net,
                                     SafeBuffer app) throws IOException
    {
        try(SafeBuffer.Writable netWritable = net.writable())
        {
            if(netWritable.isWritable())
            {
                if(socketChannel.read(net.buffer()) == -1)
                    throw new IOException("Tunnel Channel disconnected");
            }
        }
        try(SafeBuffer.Readable netReadable = net.readable())
        {
            SSLEngineResult.HandshakeStatus handshakeStatus = SSLEngineResult.HandshakeStatus.NEED_UNWRAP;
            app.buffer().clear();
            try(SafeBuffer.Writable appWritable = app.writable())
            {
                while (net.buffer().hasRemaining() &&
                    handshakeStatus == SSLEngineResult.HandshakeStatus.NEED_UNWRAP) {
                    SSLEngineResult unwrap_result = engine.unwrap(net.buffer(), app.buffer());
                    if (unwrap_result.getStatus() == SSLEngineResult.Status.BUFFER_OVERFLOW) {
                        appWritable.doubleSize();
                    }
                    if (unwrap_result.getStatus() == SSLEngineResult.Status.BUFFER_UNDERFLOW) {
                        net.setUnderflow();
                        return;
                    }
                    handshakeStatus = unwrap_result.getHandshakeStatus();
                }
            }
        }
    }

    private void handleWrap(SocketChannel socketChannel,
                                     SSLEngine engine,
                                     SafeBuffer net,
                                     SafeBuffer app) throws IOException
    {
        try(SafeBuffer.Readable appReadable = app.readable())
        {
            try(SafeBuffer.Writable netWritable = net.writable())
            {
                engine.wrap(app.buffer(), net.buffer());
            }
        }
        try (SafeBuffer.Readable netReadable = net.readable())
        {
            try {
                while (net.buffer().hasRemaining()) {
                    socketChannel.write(net.buffer());
                }
            } catch (IOException e) {
                net.buffer().clear();
            }
        }
    }
}