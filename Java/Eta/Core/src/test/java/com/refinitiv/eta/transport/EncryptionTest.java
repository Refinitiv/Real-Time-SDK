package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;
import org.junit.Test;

import javax.net.ssl.SSLEngine;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.concurrent.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class EncryptionTest {

    void runWithNoBlocks(RsslSocketChannel channel, Selector selector, CountDownLatch latch) {

        try {
            ByteBuffer src = ByteBuffer.allocate(1000000);
            for (int i = 0; i < 1000000; i++) {
                src.put((byte)i);
            }
            src.flip();

            //CryptoHelper.write should not block
            channel._scktChannel.write(src);

            ByteBuffer readBuffer = ByteBuffer.allocate(10000000);

            while (src.hasRemaining() && readBuffer.hasRemaining() && latch.getCount() == 2) {
                selector.select();
                Iterator selectedKeys = selector.selectedKeys().iterator();
                while (selectedKeys.hasNext()) {
                    SelectionKey key = (SelectionKey) selectedKeys.next();
                    selectedKeys.remove();

                    if (!key.isValid()) {
                        continue;
                    }
                    if (key.isReadable()) {
                        ((EncryptedSocketHelper) channel._scktChannel)._crypto.read(readBuffer);
                    } else if (key.isWritable()) {
                        ((EncryptedSocketHelper) channel._scktChannel)._crypto.write(src);
                    }
                }
            }

            latch.countDown();

        } catch (Exception e) {

        }
    }

    void reachNoWrite(RsslSocketChannel channel) throws Exception {

        SSLEngine engine = ((EncryptedSocketHelper)channel._scktChannel)._crypto._engine;
        CryptoHelper crypto =  ((EncryptedSocketHelper)channel._scktChannel)._crypto;

        SocketChannel chnl = channel._scktChannel._socket;

        ByteBuffer appSendBuffer = ByteBuffer.allocate(10000);
        ByteBuffer netSendBuffer = crypto.getNetSendBuffer();

        boolean done = false;
        while (!done) {

            for (int i = 0; i < 10000; i++) {
                appSendBuffer.put((byte)i);
            }

            appSendBuffer.flip();
            engine.wrap(appSendBuffer, netSendBuffer);
            netSendBuffer.flip();
            while (netSendBuffer.hasRemaining() && !done) {
                done = chnl.write(netSendBuffer) <= 0;
            }
            netSendBuffer.compact();
            appSendBuffer.clear();
        }
    }

    boolean establishConnection(ClientConnection cc, ServerConnection sc, int connType) {

        boolean success = false;

        try {
            ExecutorService executor = Executors.newFixedThreadPool(2);

            sc.bind();
            cc.connect(connType);
            sc.accept();

            Future<Integer> serverLogic = executor.submit(() -> Common.executeHandshake(sc.serverSelector, sc.serverChannel, sc.error, sc.inProg));
            Future<Integer> clientLogic = executor.submit(() -> Common.executeHandshake(cc.clientSelector, cc.clientChannel, cc.error, cc.inProg));

            success = clientLogic.get(5, TimeUnit.SECONDS) == ChannelState.ACTIVE && serverLogic.get(5, TimeUnit.SECONDS) == ChannelState.ACTIVE;

        } catch (Exception ex) {
            success = false;
        }

        return success;
    }

    void readMsg(Channel channel, Selector selector, CountDownLatch latch) {
        try {

            ReadArgs readArgs = TransportFactory.createReadArgs();
            Error error = TransportFactory.createError();
            TransportBuffer buf = null;

            while (buf == null) {
                selector.select();
                Iterator selectedKeys = selector.selectedKeys().iterator();
                while (selectedKeys.hasNext()) {
                    SelectionKey key = (SelectionKey) selectedKeys.next();
                    selectedKeys.remove();

                    if (!key.isValid()) {
                        continue;
                    }
                    if (key.isReadable()) {
                        buf = channel.read(readArgs, error);
                    }
                }
            }

            latch.countDown();

        } catch (Exception e) {
            for (int i = 0; i < e.getStackTrace().length; i++)
                System.out.println("   " + e.getStackTrace()[i]);
        }
    }

    @Test
    public void testEncryption_NoDeadlock() {

        ClientConnection cc = new ClientConnection("14002");
        ServerConnection sc = new ServerConnection("14002");

        try {
            Error error = TransportFactory.createError();
            ExecutorService executor = Executors.newFixedThreadPool(2);

            InitArgs initArgs = TransportFactory.createInitArgs();
            if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
            {
                throw new IOException("RsslTransport.initialize() failed: " + error.text());
            }

            assertTrue(establishConnection(cc, sc, ConnectionTypes.SOCKET));

            reachNoWrite(cc.clientChannel);
            reachNoWrite(sc.serverChannel);

            CountDownLatch latch = new CountDownLatch(2);

            executor.submit(() -> runWithNoBlocks(sc.serverChannel, sc.serverSelector, latch));
            executor.submit(() -> runWithNoBlocks(cc.clientChannel, cc.clientSelector, latch));

            latch.await(5, TimeUnit.SECONDS);
            assertTrue(latch.getCount() == 0);

        } catch (Exception e) {
            assert(false);
        } finally {
            cc.terminate();
            sc.terminate();
        }
    }

    @Test
    public void testHandshakeRenegotiation() {

        ClientConnection cc = new ClientConnection("14003");
        ServerConnection sc = new ServerConnection("14003");

        try {
            Error error = TransportFactory.createError();
            ExecutorService executor = Executors.newFixedThreadPool(2);

            InitArgs initArgs = TransportFactory.createInitArgs();
            if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
            {
                throw new IOException("RsslTransport.initialize() failed: " + error.text());
            }

            assertTrue(establishConnection(cc, sc, ConnectionTypes.SOCKET));

            reachNoWrite(cc.clientChannel);

            CountDownLatch latch = new CountDownLatch(2);

            executor.submit(() -> {
                try {
                    ((EncryptedSocketHelper)cc.clientChannel._scktChannel)._crypto.doHandshake();
                    runWithNoBlocks(cc.clientChannel, cc.clientSelector, latch);
                } catch (Exception e) {
                }
            });
            executor.submit(() -> runWithNoBlocks(sc.serverChannel, sc.serverSelector, latch));

            latch.await(10, TimeUnit.SECONDS);
            assertTrue(latch.getCount() == 0);

        } catch (Exception e) {
            assert(false);
        } finally {
            cc.terminate();
            sc.terminate();
        }
    }

    @Test
    public void testHandleBufferOverflow() {

        ClientConnection cc = new ClientConnection("14004");
        ServerConnection sc = new ServerConnection("14004");

        try {
            Error error = TransportFactory.createError();
            ExecutorService executor = Executors.newFixedThreadPool(2);

            InitArgs initArgs = TransportFactory.createInitArgs();
            if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
            {
                throw new IOException("RsslTransport.initialize() failed: " + error.text());
            }

            assertTrue(establishConnection(cc, sc, ConnectionTypes.SOCKET));

            cc.clientChannel._readIoBuffer.buffer().position(cc.clientChannel._readIoBuffer.buffer().capacity() - ( sc.getMsgLen() / 2 ));
            cc.clientChannel._readBufStateMachine._lastReadPosition = cc.clientChannel._readIoBuffer.buffer().position();
            cc.clientChannel._readBufStateMachine._currentMsgStartPos = cc.clientChannel._readIoBuffer.buffer().position();

            CountDownLatch latch = new CountDownLatch(1);
            sc.writeMsg();
            executor.submit(() -> readMsg(cc.clientChannel, cc.clientSelector, latch));

            latch.await(10, TimeUnit.SECONDS);

            assertEquals(latch.getCount(), 0);

        } catch (Exception e) {
            assert(false);
        } finally {
            cc.terminate();
            sc.terminate();
        }
    }

    @Test
    public void testHandleBufferOverflowHttpConnection() {

        ClientConnection cc = new ClientConnection("14005");
        ServerConnection sc = new ServerConnection("14005");

        try {
            Error error = TransportFactory.createError();
            ExecutorService executor = Executors.newFixedThreadPool(2);

            InitArgs initArgs = TransportFactory.createInitArgs();
            if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
            {
                throw new IOException("RsslTransport.initialize() failed: " + error.text());
            }

            assertTrue(establishConnection(cc, sc, ConnectionTypes.HTTP));

            cc.clientChannel._readIoBuffer.buffer().position(cc.clientChannel._readIoBuffer.buffer().capacity() - ( sc.getMsgLen() / 2 ));
            cc.clientChannel._readBufStateMachine._lastReadPosition = cc.clientChannel._readIoBuffer.buffer().position();
            cc.clientChannel._readBufStateMachine._currentMsgStartPos = cc.clientChannel._readIoBuffer.buffer().position();

            CountDownLatch latch = new CountDownLatch(1);
            sc.writeMsg();
            executor.submit(() -> readMsg(cc.clientChannel, cc.clientSelector, latch));

            latch.await(10, TimeUnit.SECONDS);

            assertEquals(latch.getCount(), 0);

        } catch (Exception e) {
            assert(false);
        } finally {
            cc.terminate();
            sc.terminate();
        }
    }

    class ClientConnection {

        RsslSocketChannel clientChannel = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        Selector clientSelector = null;
        String port = "14002";

        ClientConnection(String port) {
            if (port != null && !port.equals("")) {
                this.port = port;
            }
        }

        void connect(int connType) throws Exception {

            connectOptions.connectionType(ConnectionTypes.ENCRYPTED);
            connectOptions.encryptionOptions().connectionType(connType);
            connectOptions.encryptionOptions().KeystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
            connectOptions.encryptionOptions().KeystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
            connectOptions.encryptionOptions().KeystoreType("JKS");
            connectOptions.encryptionOptions().TrustManagerAlgorithm("");
            connectOptions.encryptionOptions().KeyManagerAlgorithm("SunX509");
            connectOptions.encryptionOptions().SecurityProtocol("TLS");
            connectOptions.encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
            connectOptions.encryptionOptions().SecurityProvider("SunJSSE");
            connectOptions.tunnelingInfo().tunnelingType("None");
            connectOptions.unifiedNetworkInfo().address("localhost");
            connectOptions.unifiedNetworkInfo().serviceName(port);
            connectOptions.guaranteedOutputBuffers(2);
            connectOptions.majorVersion(Codec.majorVersion());
            connectOptions.minorVersion(Codec.minorVersion());
            connectOptions.sysRecvBufSize(64 * 1024);

            clientSelector = Selector.open();

            clientChannel = (RsslSocketChannel)Transport.connect(connectOptions, error);
            clientChannel.selectableChannel().register(clientSelector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT | SelectionKey.OP_WRITE, clientChannel);
        }

        void terminate() {
            if (clientSelector != null && clientSelector.isOpen()) {
                for (SelectionKey key : clientSelector.keys())
                    if (key.isValid()) {
                        key.cancel();
                    }

            }
            if (clientChannel != null) {
                assertEquals(clientChannel.close(error), TransportReturnCodes.SUCCESS);
            }
        }
    }

    class ServerConnection {

        RsslSocketChannel serverChannel = null;
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Selector serverSelector = null;
        BindOptions bindOptions = TransportFactory.createBindOptions();
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        String port = "14002";
        Server server;

        ServerConnection(String port) {
            if (port != null && !port.equals("")) {
                this.port = port;
            }
        }

        void bind() throws Exception {
            serverSelector = Selector.open();

            bindOptions.connectionType(ConnectionTypes.ENCRYPTED);
            bindOptions.encryptionOptions().keystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
            bindOptions.encryptionOptions().keystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
            bindOptions.encryptionOptions().keystoreType("JKS");
            bindOptions.encryptionOptions().trustManagerAlgorithm("");
            bindOptions.encryptionOptions().keyManagerAlgorithm("SunX509");
            bindOptions.encryptionOptions().securityProtocol("TLS");
            bindOptions.encryptionOptions().securityProvider("SunJSSE");
            bindOptions.serviceName(port);

            server = Common.serverBind(serverSelector, bindOptions, 60, 30, Ripc.CompressionTypes.NONE, 0,
                    Codec.protocolType(), Codec.majorVersion(), Codec.minorVersion(), false, error);
        }

        void accept() {
            serverChannel = (RsslSocketChannel) Common.serverAccept(serverSelector, acceptOptions, error);
        }

        private TransportBuffer getMessage() {

            UInt intNum = CodecFactory.createUInt();
            TransportBuffer msgBuf = null;
            Error error = TransportFactory.createError();
            RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
            EncodeIterator encIter = CodecFactory.createEncodeIterator();

            ElementList elementList = CodecFactory.createElementList();
            ElementEntry elementEntry = CodecFactory.createElementEntry();

            Buffer applicationId = CodecFactory.createBuffer();
            Buffer applicationName = CodecFactory.createBuffer();
            Buffer applicationPosition = CodecFactory.createBuffer();

            elementList.clear();
            elementEntry.clear();

            msgBuf = serverChannel.getBuffer(1000, false, error);

            encIter.clear();
            encIter.setBufferAndRWFVersion(msgBuf, serverChannel.majorVersion(), serverChannel.minorVersion());

            refreshMsg.clear();

            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.LOGIN);
            refreshMsg.containerType(DataTypes.ELEMENT_LIST);

            refreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.CLEAR_CACHE);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.state().code(StateCodes.NONE);

            String stateText = "Login accepted by host ";
            String hostName = null;
            hostName = "localhost";

            stateText += hostName;

            refreshMsg.state().text().data(stateText);
            refreshMsg.streamId(1);
            refreshMsg.msgKey().flags(MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME);

            refreshMsg.msgKey().name().data("user");
            refreshMsg.msgKey().nameType(Login.UserIdTypes.NAME);
            refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

            refreshMsg.encodeInit(encIter, 0);

            elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
            elementList.encodeInit(encIter, null, 0);

            applicationId.data("256");
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.APPID);
            elementEntry.encode(encIter, applicationId);

            applicationName.data("ETA Provider Training");
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.APPNAME);

            elementEntry.encode(encIter, applicationName);

            applicationPosition.data("position");
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.POSITION);
            elementEntry.encode(encIter, applicationPosition);

            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.SUPPORT_BATCH);
            intNum.value(1);
            elementEntry.encode(encIter, intNum);

            elementList.encodeComplete(encIter, true);
            refreshMsg.encodeKeyAttribComplete(encIter, true);

            elementList.clear();
            elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
            elementList.encodeInit(encIter, null, 0);

            for (int i = 0; i < 20; i++) {

                elementEntry.dataType(DataTypes.UINT);
                elementEntry.name(ElementNames.SUPPORT_BATCH);
                intNum.value(1);
                elementEntry.encode(encIter, intNum);
            }

            elementList.encodeComplete(encIter, true);
            refreshMsg.encodeComplete(encIter, true);

            return msgBuf;
        }

        int getMsgLen() {
            return getMessage().length();
        }

        void writeMsg() {

            TransportBuffer buf = getMessage();

            WriteArgs writeArgs = TransportFactory.createWriteArgs();
            for (int i = 0; i < 10; i++) {
                assertTrue(serverChannel.write(buf, writeArgs, error) >= CodecReturnCodes.SUCCESS);
                buf = getMessage();
            }
        }

        void terminate() {
            if (serverSelector != null && serverSelector.isOpen()) {
                for (SelectionKey key : serverSelector.keys())
                    if (key.isValid()) {
                        key.cancel();
                    }

            }
            if (serverChannel != null) {
                assertEquals(serverChannel.close(error), TransportReturnCodes.SUCCESS);
            }
            if (server != null) {
                assertEquals(server.close(error), TransportReturnCodes.SUCCESS);
            }
        }
    }
}
