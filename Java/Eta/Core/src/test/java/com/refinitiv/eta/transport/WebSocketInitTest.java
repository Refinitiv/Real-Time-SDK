/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Codec;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.io.IOException;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketOption;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.nio.channels.spi.SelectorProvider;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.*;

import static com.refinitiv.eta.transport.SocketChannelJunitTest.*;
import static org.junit.Assert.*;

@RunWith(Parameterized.class)
public class WebSocketInitTest {

    SocketHelperMock _helper = new SocketHelperMock();

    @Parameterized.Parameters
    public static Object[][] data()
    {
        return new Object[][]{
                {1, 150},
                {3, 200},
                {7, 200},
                {2, 300},
                {10, 100},
                {10, 200},
                {5, 150},
                {8, 200},
                {3, 300},
                {12, 50},
                {6, 100}
        };
    }

    public WebSocketInitTest(int numOfParts, int timeout)
    {
        _helper.numOfParts = numOfParts;
        _helper.timeout = timeout;
    }

    @Test
    public void testWebsocketInitMsgsInFewParts()
    {
        ExecutorService executor = Executors.newFixedThreadPool(4);
        RsslSocketChannel clientChannel = null;
        RsslSocketChannel serverChannel = null;
        Server server = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        final ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        final BindOptions bindOptions = TransportFactory.createBindOptions();

        connectOptions.connectionType(ConnectionTypes.WEBSOCKET);
        bindOptions.connectionType(ConnectionTypes.SOCKET);

        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        connectOptions.wSocketOpts().protocols("rssl.rwf");
        connectOptions.guaranteedOutputBuffers(2);
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());

        /*Prepare bind common options for server.*/
        bindOptions.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bindOptions.wSocketOpts().protocols("rssl.rwf");

        final AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        try (
                Selector serverSelector = Selector.open();
                Selector clientSelector = Selector.open();
        )
        {
            //Initialize transport
            initTransport(false);

            //Initialize server
            server = serverBind(serverSelector, bindOptions, error);

            // ETAJ client to connect to ETAJ server.
            clientChannel = (RsslSocketChannel) Transport.connect(connectOptions, error);
            if (Objects.isNull(clientChannel)) {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }
            clientChannel.selectableChannel().register(clientSelector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT | SelectionKey.OP_WRITE, clientChannel);

            // have the ETAJ server accept the connection from the ETAJ client.
            serverChannel = (RsslSocketChannel) serverAccept(serverSelector, acceptOptions, error);
            assertNotNull(serverChannel);
            assertNotNull(serverChannel.selectableChannel());

            _helper.realSocketHelper = serverChannel._scktChannel;
            serverChannel._scktChannel = _helper;

            // initChannel should send connectReq to our server.
            final RsslSocketChannel rsslClientChannel = clientChannel;
            final RsslSocketChannel rsslServerChannel = serverChannel;

            Future<Integer> clientLogic = executor.submit(() -> Common.executeHandshake(clientSelector, rsslClientChannel, error, inProg));
            Future<Integer> serverLogic = executor.submit(() -> Common.executeHandshake(serverSelector, rsslServerChannel, error, inProg));

            // accept that client verify response handshake from server.
            int channelState = serverLogic.get(100, TimeUnit.SECONDS);
            assertEquals(error.text(), ChannelState.ACTIVE, channelState);
            channelState = clientLogic.get(100, TimeUnit.SECONDS);
            assertEquals(error.text(), ChannelState.ACTIVE, channelState);
        }
        catch (IOException | InterruptedException | ExecutionException | TimeoutException e)
        {
            fail("Exception caught, exception=" + e.toString());
        }
        finally
        {
            executor.shutdownNow();
            if (Objects.nonNull(server))
            {
                server.close(error);
            }
            if (Objects.nonNull(clientChannel))
            {
                clientChannel.closeSocketChannel(error);
            }
            if (Objects.nonNull(serverChannel)) {
                serverChannel.closeSocketChannel(error);
            }
            Transport.uninitialize();
        }
    }

    @Test
    public void testHttpInitMsgsInFewParts()
    {
        ExecutorService executor = Executors.newFixedThreadPool(4);
        RsslHttpSocketChannel clientChannel = null;
        RsslHttpSocketChannel serverChannel = null;
        Server server = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        final ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        final BindOptions bindOptions = TransportFactory.createBindOptions();

        connectOptions.connectionType(ConnectionTypes.HTTP);
        bindOptions.connectionType(ConnectionTypes.HTTP);

        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        connectOptions.guaranteedOutputBuffers(2);
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());

        /*Prepare bind common options for server.*/
        bindOptions.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);

        final AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        try (
                Selector serverSelector = Selector.open();
                Selector clientSelector = Selector.open();
        )
        {
            //Initialize transport
            initTransport(false);

            //Initialize server
            server = serverBind(serverSelector, bindOptions, error);

            // ETAJ client to connect to ETAJ server.
            clientChannel = (RsslHttpSocketChannel) Transport.connect(connectOptions, error);
            if (Objects.isNull(clientChannel)) {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }
            clientChannel.selectableChannel().register(clientSelector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT | SelectionKey.OP_WRITE, clientChannel);

            // have the ETAJ server accept the connection from the ETAJ client.
            serverChannel = (RsslHttpSocketChannel) serverAccept(serverSelector, acceptOptions, error);
            assertNotNull(serverChannel);
            assertNotNull(serverChannel.selectableChannel());

            _helper.realSocketHelper = serverChannel._scktChannel;
            serverChannel._scktChannel = _helper;

            // initChannel should send connectReq to our server.
            final RsslHttpSocketChannel rsslClientChannel = clientChannel;
            final RsslHttpSocketChannel rsslServerChannel = serverChannel;

            Future<Integer> clientLogic = executor.submit(() -> Common.executeHandshake(clientSelector, rsslClientChannel, error, inProg));
            Future<Integer> serverLogic = executor.submit(() -> Common.executeHandshake(serverSelector, rsslServerChannel, error, inProg));

            // accept that client verify response handshake from server.
            int channelState = serverLogic.get(100, TimeUnit.SECONDS);
            assertEquals(error.text(), ChannelState.ACTIVE, channelState);
            channelState = clientLogic.get(100, TimeUnit.SECONDS);
            assertEquals(error.text(), ChannelState.ACTIVE, channelState);
        }
        catch (IOException | InterruptedException | ExecutionException | TimeoutException e)
        {
            fail("Exception caught, exception=" + e.toString());
        }
        finally
        {
            executor.shutdownNow();
            if (Objects.nonNull(server))
            {
                server.close(error);
            }
            if (Objects.nonNull(clientChannel))
            {
                clientChannel.closeSocketChannel(error);
            }
            if (Objects.nonNull(serverChannel)) {
                serverChannel.closeSocketChannel(error);
            }
            Transport.uninitialize();
        }
    }

    @Test
    public void testSocketInitMsgsInFewParts()
    {
        ExecutorService executor = Executors.newFixedThreadPool(4);
        RsslSocketChannel clientChannel = null;
        RsslSocketChannel serverChannel = null;
        Server server = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        final ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        final BindOptions bindOptions = TransportFactory.createBindOptions();

        connectOptions.connectionType(ConnectionTypes.SOCKET);
        bindOptions.connectionType(ConnectionTypes.SOCKET);

        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        connectOptions.guaranteedOutputBuffers(2);
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());

        /*Prepare bind common options for server.*/
        bindOptions.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);

        final AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        try (
                Selector serverSelector = Selector.open();
                Selector clientSelector = Selector.open();
        )
        {
            //Initialize transport
            initTransport(false);

            //Initialize server
            server = serverBind(serverSelector, bindOptions, error);

            // ETAJ client to connect to ETAJ server.
            clientChannel = (RsslSocketChannel) Transport.connect(connectOptions, error);
            if (Objects.isNull(clientChannel)) {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }
            clientChannel.selectableChannel().register(clientSelector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT | SelectionKey.OP_WRITE, clientChannel);

            // have the ETAJ server accept the connection from the ETAJ client.
            serverChannel = (RsslSocketChannel) serverAccept(serverSelector, acceptOptions, error);
            assertNotNull(serverChannel);
            assertNotNull(serverChannel.selectableChannel());

            _helper.realSocketHelper = serverChannel._scktChannel;
            serverChannel._scktChannel = _helper;

            // initChannel should send connectReq to our server.
            final RsslSocketChannel rsslClientChannel = clientChannel;
            final RsslSocketChannel rsslServerChannel = serverChannel;

            Future<Integer> clientLogic = executor.submit(() -> Common.executeHandshake(clientSelector, rsslClientChannel, error, inProg));
            Future<Integer> serverLogic = executor.submit(() -> Common.executeHandshake(serverSelector, rsslServerChannel, error, inProg));

            // accept that client verify response handshake from server.
            int channelState = serverLogic.get(100, TimeUnit.SECONDS);
            assertEquals(error.text(), ChannelState.ACTIVE, channelState);
            channelState = clientLogic.get(100, TimeUnit.SECONDS);
            assertEquals(error.text(), ChannelState.ACTIVE, channelState);
        }
        catch (IOException | InterruptedException | ExecutionException | TimeoutException e)
        {
            fail("Exception caught, exception=" + e.toString());
        }
        finally
        {
            executor.shutdownNow();
            if (Objects.nonNull(server))
            {
                server.close(error);
            }
            if (Objects.nonNull(clientChannel))
            {
                clientChannel.closeSocketChannel(error);
            }
            if (Objects.nonNull(serverChannel)) {
                serverChannel.closeSocketChannel(error);
            }
            Transport.uninitialize();
        }
    }

    class SocketHelperMock extends SocketHelper {

        int numOfParts = 7;
        long timeout = 100;

        SocketHelper realSocketHelper = null;

        @Override
        public int write(ByteBuffer src) throws IOException
        {
            int bytesToBeWritten = src.limit() - src.position();

            int start = 0;
            int pieceLength = bytesToBeWritten / numOfParts;

            try
            {
                if (pieceLength > 0)
                {
                    for (int i = 0; i < numOfParts; i++)
                    {
                        start = i * pieceLength;
                        ByteBuffer bb = ByteBuffer.allocate(pieceLength);
                        for (int j = start; j < start + pieceLength; j++) bb.put(src.get(j));
                        bb.flip();
                        realSocketHelper.write(bb);
                        Thread.sleep(timeout);
                    }
                }

                int remaining = bytesToBeWritten - numOfParts * pieceLength;
                if (remaining > 0)
                {
                    ByteBuffer bb = ByteBuffer.allocate(remaining);
                    for (int j = numOfParts * pieceLength; j < numOfParts * pieceLength + remaining; j++) bb.put(src.get(j));
                    bb.flip();
                    realSocketHelper.write(bb);
                }

                src.position(src.limit());
            }
            catch (Exception e)
            {
                assertFalse(true);
            }


            return bytesToBeWritten;
        }

        @Override
        public SocketChannel bind(SocketAddress local) throws IOException
        {
            return realSocketHelper.bind(local);
        }

        @Override
        public <T> SocketChannel setOption(SocketOption<T> name, T value) throws IOException
        {
            return realSocketHelper.setOption(name, value);
        }

        @Override
        public SocketChannel shutdownInput() throws IOException
        {
            return realSocketHelper.shutdownInput();
        }

        @Override
        public SocketChannel shutdownOutput() throws IOException
        {
            return realSocketHelper.shutdownOutput();
        }

        @Override
        public Socket socket()
        {
            return realSocketHelper.socket();
        }

        @Override
        public boolean isConnectionPending()
        {
            return realSocketHelper.isConnectionPending();
        }

        @Override
        public boolean connect(SocketAddress remote, boolean proxy) throws IOException
        {
            return realSocketHelper.connect(remote, proxy);
        }

        @Override
        public boolean finishConnect() throws IOException
        {
            return realSocketHelper.finishConnect();
        }

        @Override
        public SocketAddress getRemoteAddress() throws IOException
        {
            return realSocketHelper.getRemoteAddress();
        }

        @Override
        public long read(ByteBuffer[] dsts, int offset, int length) throws IOException
        {
            return realSocketHelper.read(dsts, offset, length);
        }

        @Override
        public long write(ByteBuffer[] srcs, int offset, int length) throws IOException
        {
            return realSocketHelper.write(srcs, offset, length);
        }

        @Override
        public SocketAddress getLocalAddress() throws IOException
        {
            return realSocketHelper.getLocalAddress();
        }

        @Override
        public SelectorProvider provider()
        {
            return realSocketHelper.provider();
        }

        @Override
        public int validOps()
        {
            return realSocketHelper.validOps();
        }

        @Override
        public boolean isRegistered()
        {
            return realSocketHelper.isRegistered();
        }

        @Override
        public SelectionKey keyFor(Selector sel)
        {
            return realSocketHelper.keyFor(sel);
        }

        @Override
        public SelectionKey register(Selector sel, int ops, Object att) throws ClosedChannelException
        {
            return realSocketHelper.register(sel, ops, att);
        }

        @Override
        public SelectableChannel configureBlocking(boolean block) throws IOException
        {
            return realSocketHelper.configureBlocking(block);
        }

        @Override
        public boolean isConnected()
        {
            return realSocketHelper.isConnected();
        }

        @Override
        public boolean isBlocking()
        {
            return realSocketHelper.isBlocking();
        }

        @Override
        public int read(ByteBuffer dst) throws IOException
        {
            return realSocketHelper.read(dst);
        }

        @Override
        public Object blockingLock()
        {
            return realSocketHelper.blockingLock();
        }

        @Override
        public boolean isOpen()
        {
            return realSocketHelper.isOpen();
        }

        @Override
        public void close() throws IOException
        {
            realSocketHelper.close();
        }

        @Override
        public void completedProxyConnection()
        {
            realSocketHelper._completedProxy = true;
        }

        @Override
        public boolean postProxyInit() throws IOException
        {
            /* No-op here, used in encrypted case. */
            return true;
        }

        @Override
        public String getActiveTLSVersion() throws IOException
        {
            /* Used in encrypted case. */
            return "None";
        }

        @Override
        public long read(ByteBuffer[] dsts) throws IOException
        {
            return realSocketHelper.read(dsts);
        }

        @Override
        public long write(ByteBuffer[] srcs) throws IOException
        {
            return realSocketHelper.write(srcs);
        }

        @Override
        public <T> T getOption(SocketOption<T> name) throws IOException
        {
            return realSocketHelper.getOption(name);
        }

        @Override
        public Set<SocketOption<?>> supportedOptions()
        {
            return realSocketHelper.supportedOptions();
        }

        @Override
        public void setSocketChannel(SocketChannel socket)
        {
            realSocketHelper._socket = socket;
        }

        @Override
        public SocketChannel getSocketChannel()
        {
            return realSocketHelper._socket;
        }

        public void copy(SocketHelper dstSocket)
        {
            dstSocket._socket = realSocketHelper._socket;
            dstSocket._completedProxy = realSocketHelper._completedProxy;
        }

        @Override
        public void initialize(ConnectOptions options) throws IOException
        {
            realSocketHelper._completedProxy = false;
        }

        @Override
        public void initialize(BindOptions options) throws IOException
        {
            // No proxy connections for servers
            realSocketHelper._completedProxy = true;
        }
    }
}
