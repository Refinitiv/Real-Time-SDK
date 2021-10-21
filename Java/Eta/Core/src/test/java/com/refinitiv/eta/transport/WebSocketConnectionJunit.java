package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Codec;
import org.junit.Test;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Objects;
import java.util.concurrent.*;

import static com.refinitiv.eta.transport.SocketChannelJunitTest.*;
import static org.junit.Assert.*;
import static org.junit.Assume.assumeTrue;

public class WebSocketConnectionJunit {

    private static final String DEFAULT_LISTEN_PORT_AS_STRING = "14002";
    private static final String JSON_WS_PROTOCOL = "tr_json2";
    private static final String RWF_WS_PROTOCOL = "rssl.rwf";

    public void executeWebSocketHandshake(String clientWsProtocol, String serverWsProtocol, boolean proxy, boolean encrypted, int expectedChannelState) {
        ExecutorService executor = Executors.newFixedThreadPool(4);
        final int compressionType = Ripc.CompressionTypes.LZ4;
        RsslSocketChannel clientChannel = null;
        RsslSocketChannel serverChannel = null;
        Server server = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        final ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        final BindOptions bindOptions = TransportFactory.createBindOptions();

        prepareWebSocketConnectOptions(clientWsProtocol, serverWsProtocol, compressionType, connectOptions, bindOptions, encrypted);
        if (proxy) {
            assumeTrue(useProxy(connectOptions, bindOptions));
        }

        final AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        try (
                Selector serverSelector = Selector.open();
                Selector clientSelector = Selector.open();
        ) {
            //Initialize transport
            initTransport(false);

            //Initialize server
            server = serverBind(serverSelector, bindOptions, error);

            // ETAJ client to connect to ETAJ server.
            clientChannel = (RsslSocketChannel) Transport.connect(connectOptions, error);
            if (Objects.isNull(clientChannel)) {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }
            assertEquals(encrypted, clientChannel._encrypted);
            clientChannel.selectableChannel().register(clientSelector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT | SelectionKey.OP_WRITE, clientChannel);

            //Execute proxy initialization if available.
            if (proxy) {
                connectClientSocketChannelToProxy(clientChannel, inProg, error);
            }

            // have the ETAJ server accept the connection from the ETAJ client.
            serverChannel = (RsslSocketChannel) serverAccept(serverSelector, acceptOptions, error);
            assertNotNull(serverChannel);
            assertNotNull(serverChannel.selectableChannel());

            // initChannel should send connectReq to our server.
            final RsslSocketChannel rsslClientChannel = clientChannel;
            final RsslSocketChannel rsslServerChannel = serverChannel;
            Future<Integer> clientLogic = executor.submit(() -> Common.executeHandshake(clientSelector, rsslClientChannel, error, inProg));
            Future<Integer> serverLogic = executor.submit(() -> Common.executeHandshake(serverSelector, rsslServerChannel, error, inProg));

            // accept that client verify response handshake from server.
            int channelState = serverLogic.get(60, TimeUnit.SECONDS);
            assertEquals(error.text(), expectedChannelState, channelState);
            channelState = clientLogic.get(60, TimeUnit.SECONDS);
            assertEquals(error.text(), expectedChannelState, channelState);
        } catch (IOException | InterruptedException | ExecutionException | TimeoutException e) {
            fail("Exception caught, exception=" + e.toString());
        } finally {
            executor.shutdownNow();
            if (Objects.nonNull(server)) {
                server.close(error);
            }
            if (Objects.nonNull(clientChannel)) {
                clientChannel.closeSocketChannel(error);
            }
            if (Objects.nonNull(serverChannel)) {
                serverChannel.closeSocketChannel(error);
            }
            Transport.uninitialize();
        }
    }

    @Test
    public void testIncorrectProtocolDuringWebSocketInitializing() {
        final String wrongRequest = "GET /websocket HTTP/1.1\r\n" +
                                    "Host: server.example.com\r\n" +
                                    "Upgrade: none\r\n" + //This header is invalid. Expected value: 'websocket'
                                    "Connection: none\r\n" + //This header is invalid. Expected value 'upgrade'
                                    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n" +
                                    "Origin: http://example.com\r\n" +
                                    "Sec-WebSocket-Protocol: tr_json2, rssl.json.v2\r\n" +
                                    "Sec-WebSocket-Version: 13\r\n";
        final int compressionType = Ripc.CompressionTypes.LZ4;
        RsslSocketChannel clientChannel = null;
        Server server = null;
        Channel serverChannel = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        final ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        final BindOptions bindOptions = TransportFactory.createBindOptions();

        prepareWebSocketConnectOptions(JSON_WS_PROTOCOL, JSON_WS_PROTOCOL, compressionType, connectOptions, bindOptions, false);

        final AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        try (Selector serverSelector = Selector.open()) {
            initTransport(false); // initialize RS
            server = serverBind(serverSelector, bindOptions, error);
            clientChannel = (RsslSocketChannel) Transport.connect(connectOptions, error);
            // ETAJ client to connect to ETAJ server.
            if (Objects.isNull(clientChannel)) {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            // have the ETAJ server accept the connection from the ETAJ client.
            serverChannel = serverAccept(serverSelector, acceptOptions, error);
            assertNotNull(serverChannel);
            assertNotNull(serverChannel.selectableChannel());

            // initChannel should send connectReq to our server.
            clientChannel._scktChannel.finishConnect();
            clientChannel._scktChannel.write(ByteBuffer.wrap(wrongRequest.getBytes()));
            clientChannel.initializeWebSocketConnection();
            clientChannel._initChnlState = RsslSocketChannel.InitChnlState.WAIT_WS_RESPONSE_HANDSHAKE;

            //Server only send response handshake and set INACTIVE status because protocols are not matched for connection.
            assertEquals(TransportReturnCodes.FAILURE, serverChannel.init(inProg, error));
            assertEquals(TransportReturnCodes.FAILURE, clientChannel.init(inProg, error));
        } catch (IOException e) {
            fail("Exception caught, exception=" + e.toString());
        } finally {
            if (Objects.nonNull(server)) {
                server.close(error);
            }
            Transport.uninitialize();
        }
    }

    @Test
    public void testEncryptedWebSocketJsonHandshake() {
        executeWebSocketHandshake(JSON_WS_PROTOCOL, JSON_WS_PROTOCOL, false, true, ChannelState.ACTIVE);
    }

    @Test
    public void testEncryptedWebSocketRwfHandshake() {
        executeWebSocketHandshake(RWF_WS_PROTOCOL, RWF_WS_PROTOCOL, false, true, ChannelState.ACTIVE);
    }

    @Test
    public void testDefaultWebSocketJsonHandshake() {
        executeWebSocketHandshake(JSON_WS_PROTOCOL, JSON_WS_PROTOCOL, false, false, ChannelState.ACTIVE);
    }

    @Test
    public void testDefaultWebSocketRwfHandshake() {
        executeWebSocketHandshake(RWF_WS_PROTOCOL, RWF_WS_PROTOCOL, false, false, ChannelState.ACTIVE);
    }

    @Test
    public void testRwfWebSocketProxy() {
        executeWebSocketHandshake(RWF_WS_PROTOCOL, RWF_WS_PROTOCOL, true, false, ChannelState.ACTIVE);
    }

    @Test
    public void testJsonWebSocketProxy() {
        executeWebSocketHandshake(JSON_WS_PROTOCOL, JSON_WS_PROTOCOL, true, false, ChannelState.ACTIVE);
    }

    @Test
    public void testWebSocketConnectionWithDifferentProtocols() {
        executeWebSocketHandshake(JSON_WS_PROTOCOL, RWF_WS_PROTOCOL, false, false, ChannelState.INACTIVE);
    }

    private void prepareWebSocketConnectOptions(String clientProtocols, String serverProtocols, int compressionType,
                                                ConnectOptions connectOptions, BindOptions bindOptions, boolean encrypted) {
        if (encrypted) {
            /*Prepare client and server options for encrypted connection*/
            connectOptions.connectionType(ConnectionTypes.ENCRYPTED);
            connectOptions.encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
            connectOptions.encryptionOptions().KeystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
            connectOptions.encryptionOptions().KeystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
            connectOptions.encryptionOptions().KeystoreType("JKS");
            connectOptions.encryptionOptions().TrustManagerAlgorithm("");
            connectOptions.encryptionOptions().KeyManagerAlgorithm("SunX509");
            connectOptions.encryptionOptions().SecurityProtocol("TLS");
            connectOptions.encryptionOptions().SecurityProvider("SunJSSE");
            connectOptions.tunnelingInfo().tunnelingType("None");

            bindOptions.connectionType(ConnectionTypes.ENCRYPTED);
            bindOptions.encryptionOptions().keystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
            bindOptions.encryptionOptions().keystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
            bindOptions.encryptionOptions().keystoreType("JKS");
            bindOptions.encryptionOptions().trustManagerAlgorithm("");
            bindOptions.encryptionOptions().keyManagerAlgorithm("SunX509");
            bindOptions.encryptionOptions().securityProtocol("TLS");
            bindOptions.encryptionOptions().securityProvider("SunJSSE");
        } else {
            /*Prepare client and server options for standard WebSocket connection*/
            connectOptions.connectionType(ConnectionTypes.WEBSOCKET);
            bindOptions.connectionType(ConnectionTypes.SOCKET);
        }

        /*Prepare connect common options for client.*/
        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        connectOptions.wSocketOpts().protocols(clientProtocols);
        connectOptions.guaranteedOutputBuffers(2);
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());
        connectOptions.compressionType(compressionType);

        /*Prepare bind common options for server.*/
        bindOptions.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bindOptions.compressionType(compressionType);
        bindOptions.compressionLevel(5);
        bindOptions.wSocketOpts().protocols(serverProtocols);
    }

    private boolean useProxy(ConnectOptions connectOptions, BindOptions bindOptions) {
        final String proxyHost = System.getProperty("proxyHost");
        final String proxyPort = System.getProperty("proxyPort");
        final String serverHost = System.getProperty("serverHost");
        final String serverPort = System.getProperty("serverPort");
        if (Objects.nonNull(proxyHost) && Objects.nonNull(proxyPort) && Objects.nonNull(serverHost) && Objects.nonNull(serverPort)) {
            connectOptions.tunnelingInfo().HTTPproxy(true);
            connectOptions.tunnelingInfo().HTTPproxyHostName(proxyHost);
            connectOptions.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort));
            connectOptions.unifiedNetworkInfo().address(serverHost);
            connectOptions.unifiedNetworkInfo().serviceName(serverPort);

            bindOptions.serviceName(serverPort);
            return true;
        }
        System.out.println("For testing proxy WebSocket connection, please, use next command for launching:\n" +
                           "./gradlew eta:valueadd:test --tests \"com.refinitiv.eta.transport.SocketChannelJunitTest.testProxyWebSocketHandshake\"\n" +
                           "-PvmArgs=\"-DproxyHost=[proxy host] -DproxyPort=[proxy port] -DserverHost=[server host IP] -DserverPort=[server port]\""
        );
        return false;
    }

    private void connectClientSocketChannelToProxy(RsslSocketChannel clientChannel, InProgInfo inProgInfo, Error error) throws InterruptedException {
        while (clientChannel._initChnlState != RsslSocketChannel.InitChnlState.CLIENT_WAIT_PROXY_ACK) {
            int retCode = clientChannel.init(inProgInfo, error);
            assertEquals(TransportReturnCodes.CHAN_INIT_IN_PROGRESS, retCode);
            Thread.sleep(500);
        }
    }
}
