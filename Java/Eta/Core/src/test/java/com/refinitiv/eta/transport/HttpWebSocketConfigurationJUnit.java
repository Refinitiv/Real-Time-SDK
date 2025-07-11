/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.JUnitConfigVariables;
import com.refinitiv.eta.RetryRule;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

import static org.junit.Assert.*;

public class HttpWebSocketConfigurationJUnit {

    private final WebSocketHandler httpWebSocketConfiguration = new WebSocketHandlerImpl();

    private static final int REQUEST_RESPONSE_BUFFER_SIZE = 512;

    private static final String ALL_SUPPORTED_PROTOCOLS = "rssl.rwf, tr_json2, rssl.json.v2";

    private static final String WS_KEY_EXAMPLE = "dGhlIHNhbXBsZSBub25jZQ==";

    private static final String WS_ACCEPT_KEY_EXAMPLE = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";

    private static final String REQUEST_HANDSHAKE_VALID_TEST1 =
            "GET /websocket HTTP/1.1\n" +
            "Host: server.example.com\n" +
            "Upgrade: websocket\n" +
            "Connection: Upgrade\n" +
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n" +
            "Origin: http://example.com\n" +
            "Sec-WebSocket-Protocol: tr_json2, rssl.json.v2\n" +
            "Sec-WebSocket-Version: 13\n" +
            "Cookie: testCook=2;testcook1=3\n";

    private static final String REQUEST_HANDSHAKE_INVALID_TEST2_1 =
            "GET /websocket HTTP/1.1\n" +
            "Host: server.example.com\n" +
            "Upgrade: none\n" +
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n" +
            "Origin: http://example.com\n" +
            "Sec-WebSocket-Protocol: not_real1, rssl.json.v2\n" +
            "Sec-WebSocket-Version: 13\n";

    private static final String REQUEST_HANDSHAKE_INVALID_TEST2_2 =
            "GET /websocket HTTP/1.1\n" +
            "Host: server.example.com\n" +
            "Upgrade: websocket\n" +
            "Origin: http://example.com\n" +
            "Sec-WebSocket-Protocol: tr_json2, rssl.json.v2\n" +
            "Sec-WebSocket-Version: 13\n";

    private static final String REQUEST_HANDSHAKE_INVALID_TEST2_3 =
            "GET /websocket HTTP/1.1\n" +
            "Host: server.example.com\n" +
            "Upgrade: websocket\n" +
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n" +
            "Origin: http://example.com\n" +
            "Sec-WebSocket-Protocol: tr_json2, rssl.json.v2\n" +
            "Sec-WebSocket-Version: 14\n";

    private static final String REQUEST_HANDSHAKE_NOT_SUPPORTED_PROTOCOLS_TEST3 =
            "GET /websocket HTTP/1.1\n" +
            "Host: server.example.com\n" +
            "Upgrade: websocket\n" +
            "Connection: Upgrade\n" +
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n" +
            "Origin: http://example.com\n" +
            "Sec-WebSocket-Protocol: not_real_1, not_real_2\n" +
            "Sec-WebSocket-Version: 13\n" +
            "Cookie: testCook=2;testcook1=3\n";

    private static final String REQUEST_HANDSHAKE_PROTOCOLS_NOT_SENT_TEST4 =
            "GET /websocket HTTP/1.1\n" +
            "Host: server.example.com\n" +
            "Upgrade: websocket\n" +
            "Connection: Upgrade\n" +
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n" +
            "Origin: http://example.com\n" +
            "Sec-WebSocket-Version: 13\n";

    private static final String RESPONSE_HANDSHAKE_VALID_TEST5 =
            "HTTP/1.1 101 Switching Protocols\n" +
            "Upgrade: websocket\n" +
            "Connection: Upgrade\n" +
            "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\n" +
            "Sec-WebSocket-Protocol: tr_json2\n";

    private static final String RESPONSE_HANDSHAKE_INVALID_TEST6 =
            "HTTP/1.1 101 Switching Protocols\n" +
            "Upgrade: false\n" +
            "Connection: Upgrade\n" +
            "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzjhZRbK+xOo=\n" +
            "Sec-WebSocket-Protocol: tr_json2\n";


    private final Buffer rwsKey = CodecFactory.createBuffer();
    private final Error error = TransportFactory.createError();

    private WebSocketSession webSocketSession;
    private final WSocketOpts webSocketOptions;


    {
        httpWebSocketConfiguration.initialize();
        webSocketSession = httpWebSocketConfiguration.getWsSession();
        webSocketOptions = new WSocketOptsImpl();
        webSocketSession.setWebSocketOpts(webSocketOptions);
    }

    @Rule
    public TestName testName = new TestName();

    @Rule
    public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

    @Before
    public void beforeTest() {
        httpWebSocketConfiguration.clear();
        webSocketSession = httpWebSocketConfiguration.getWsSession();
        webSocketSession.setWebSocketOpts(webSocketOptions);

        System.out.println(">>>>>>>>>>>>>>>>>>>>  " + testName.getMethodName() + " Test <<<<<<<<<<<<<<<<<<<<<<<");
    }

    //Test 1
    @Test
    public void givenCorrectData_whenParseOpeningHandshake_thenWSSessionDataIsCreated() {
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        ByteBuffer byteBuffer = ByteBuffer.allocate(REQUEST_HANDSHAKE_VALID_TEST1.length());
        byteBuffer.put(REQUEST_HANDSHAKE_VALID_TEST1.getBytes());

        int returnCode = httpWebSocketConfiguration.readOpeningHandshake(byteBuffer, REQUEST_HANDSHAKE_VALID_TEST1.length(), 0, error);
        if (returnCode < TransportReturnCodes.SUCCESS) {
            System.err.println(error.text());
        }
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);

        final HttpRequest httpRequest = webSocketSession.getHandshakeOpeningRequest();
        assertEquals("GET /websocket HTTP/1.1", httpRequest.getHttpRequestConnectionInfo().getConnectionLine());
        assertEquals("GET", httpRequest.getHttpRequestConnectionInfo().getRequestMethod());
        assertEquals("/websocket", httpRequest.getHttpRequestConnectionInfo().getConnectionUri());
        assertEquals(webSocketSession.getHost(), "server.example.com");
        assertEquals(webSocketSession.getAcceptedProtocol(), Codec.JSON_PROTOCOL_TYPE);
        assertEquals(webSocketSession.getOrigin(), "http://example.com");
        assertTrue(webSocketSession.isConnectionUpgrade());
        assertTrue(webSocketSession.isUpgrade());

        rwsKey.clear();
        rwsKey.data(WS_KEY_EXAMPLE);
        assertTrue(webSocketSession.getWsSessionKey().equals(rwsKey));

        //check cookies
        Map<String, String> cookies = webSocketSession.getHandshakeOpeningRequest().getRequestHeaders().getCookies();
        assertEquals(2, cookies.size());
        assertTrue(cookies.containsKey("testCook"));
        assertEquals(cookies.get("testCook"), "2");
        assertTrue(cookies.containsKey("testcook1"));
        assertEquals(cookies.get("testcook1"), "3");
    }

    //Test 2_1
    @Test
    public void givenIncorrectData_whenParseOpeningHandshake_thenOpeningHandshakeReturnFailure() {
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        ByteBuffer byteBuffer = ByteBuffer.allocate(REQUEST_HANDSHAKE_INVALID_TEST2_1.length());
        byteBuffer.put(REQUEST_HANDSHAKE_INVALID_TEST2_1.getBytes());

        int returnCode = httpWebSocketConfiguration.readOpeningHandshake(byteBuffer, REQUEST_HANDSHAKE_INVALID_TEST2_1.length(), 0, error);
        assertEquals(TransportReturnCodes.FAILURE, returnCode);
    }

    //Test 2_2
    @Test
    public void givenIncorrectDataWhereSecWebSocketKeyIsAbsent_whenParseOpeningHandshake_thenOpeningHandshakeReturnFailure() {
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        ByteBuffer byteBuffer = ByteBuffer.allocate(REQUEST_HANDSHAKE_INVALID_TEST2_2.length());
        byteBuffer.put(REQUEST_HANDSHAKE_INVALID_TEST2_2.getBytes());

        int returnCode = httpWebSocketConfiguration.readOpeningHandshake(byteBuffer, REQUEST_HANDSHAKE_INVALID_TEST2_2.length(), 0, error);
        assertEquals(TransportReturnCodes.FAILURE, returnCode);
    }

    //Test 2_3
    @Test
    public void givenIncorrectDataWhereSecWebSocketVersionIsIncorrect_whenParseOpeningHandshake_thenOpeningHandshakeReturnFailure() {
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        ByteBuffer byteBuffer = ByteBuffer.allocate(REQUEST_HANDSHAKE_INVALID_TEST2_3.length());
        byteBuffer.put(REQUEST_HANDSHAKE_INVALID_TEST2_3.getBytes());

        int returnCode = httpWebSocketConfiguration.readOpeningHandshake(byteBuffer, REQUEST_HANDSHAKE_INVALID_TEST2_3.length(), 0, error);
        assertEquals(TransportReturnCodes.FAILURE, returnCode);
    }

    //Test 3
    @Test
    public void givenCorrectDataWithNonSupportedProtocols_whenParseOpeningHandshake_thenOpeningHandshakeReturnFailure() {
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        ByteBuffer byteBuffer = ByteBuffer.allocate(REQUEST_HANDSHAKE_NOT_SUPPORTED_PROTOCOLS_TEST3.length());
        byteBuffer.put(REQUEST_HANDSHAKE_NOT_SUPPORTED_PROTOCOLS_TEST3.getBytes());

        int returnCode = httpWebSocketConfiguration.readOpeningHandshake(byteBuffer, REQUEST_HANDSHAKE_NOT_SUPPORTED_PROTOCOLS_TEST3.length(), 0, error);
        assertEquals(TransportReturnCodes.FAILURE, returnCode);
    }

    //Test 4
    @Test
    public void givenRequestHandshakeWithoutProtocols_whenParseOpeningHandshake_thenOpeningHandshakeReturnFailure() {
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        ByteBuffer byteBuffer = ByteBuffer.allocate(REQUEST_HANDSHAKE_PROTOCOLS_NOT_SENT_TEST4.length());
        byteBuffer.put(REQUEST_HANDSHAKE_PROTOCOLS_NOT_SENT_TEST4.getBytes());

        int returnCode = httpWebSocketConfiguration.readOpeningHandshake(byteBuffer, REQUEST_HANDSHAKE_PROTOCOLS_NOT_SENT_TEST4.length(), 0, error);
        assertEquals(error.text(), TransportReturnCodes.FAILURE, returnCode);
    }

    //Test 5
    @Test
    public void givenCorrectData_whenParseResponseHandshake_thenWSSessionDataIsCreated() {
        ByteBuffer byteBuffer = ByteBuffer.allocate(RESPONSE_HANDSHAKE_VALID_TEST5.length());
        byteBuffer.put(RESPONSE_HANDSHAKE_VALID_TEST5.getBytes());

        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);
        webSocketSession.getWsSessionAcceptKey().data(WS_ACCEPT_KEY_EXAMPLE);
        int returnCode = httpWebSocketConfiguration.readResponseHandshake(byteBuffer, RESPONSE_HANDSHAKE_VALID_TEST5.length(), 0, error);
        if (returnCode < TransportReturnCodes.SUCCESS) {
            System.err.println(error.text());
        }
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);

        assertEquals("HTTP/1.1 101 Switching Protocols",
                webSocketSession.getHandshakeOpeningResponse().getHttpResponseConnectionInfo().getConnectionLine());
        assertEquals(101, webSocketSession.getHandshakeOpeningResponse().getHttpResponseConnectionInfo().getResponseStatus());
        assertEquals("tr_json2", webSocketSession.getProtocolName());
        assertEquals(Codec.JSON_PROTOCOL_TYPE, webSocketSession.getAcceptedProtocol());
        assertTrue(webSocketSession.isUpgrade());
        assertTrue(webSocketSession.isConnectionUpgrade());

        rwsKey.clear();
        rwsKey.data(WS_ACCEPT_KEY_EXAMPLE);
        assertTrue(rwsKey.equals(webSocketSession.getWsSessionAcceptKey()));
    }

    //Test 6
    @Test
    public void givenIncorrectData_whenParseResponseHandshake_thenOpeningHandshakeReturnFailure() {
        ByteBuffer byteBuffer = ByteBuffer.allocate(RESPONSE_HANDSHAKE_INVALID_TEST6.length());
        byteBuffer.put(RESPONSE_HANDSHAKE_INVALID_TEST6.getBytes());

        webSocketSession.getWsSessionKey().data(WS_ACCEPT_KEY_EXAMPLE);
        webSocketOptions.protocols(ALL_SUPPORTED_PROTOCOLS);
        int returnCode = httpWebSocketConfiguration.readResponseHandshake(byteBuffer, RESPONSE_HANDSHAKE_INVALID_TEST6.length(), 0, error);
        assertEquals(TransportReturnCodes.FAILURE, returnCode);
    }

    //Test 7
    @Test
    public void givenWsSession_whenExecuteHandshake_thenRequestAndResponseContainsProperlyData() {
        final WebSocketHandler clientHandler = new WebSocketHandlerImpl();
        clientHandler.initialize();

        final WebSocketSession clientSession = clientHandler.getWsSession();
        final WSocketOpts clientOptions = new WSocketOptsImpl();

        /*fill client session and prepare request*/
        clientSession.setWebSocketOpts(clientOptions);
        clientSession.applyOriginSending();
        clientSession.setWebSocketUri(URI.create("/WebSocket"));
        clientSession.setOrigin("http://example.com");
        clientSession.setRecoveryVersion(1);
        clientSession.setHost("server.example.com");
        clientSession.getWsSessionKey().data(WS_KEY_EXAMPLE);
        clientOptions.protocols(ALL_SUPPORTED_PROTOCOLS);

        /*encode handshake request*/
        final ByteBuffer dataBuffer = ByteBuffer.allocate(REQUEST_RESPONSE_BUFFER_SIZE);
        int returnCode = clientHandler.createRequestHandshake(dataBuffer, error);

        /*Check that request was properly generated.*/
        assertNotNull(clientSession.getWsSessionKey());
        assertNotNull(clientSession.getWsSessionAcceptKey());
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);
        assertCaseSensitiveRequest(webSocketSession.getHandshakeOpeningRequest());

        /*read request by a server*/
        final WebSocketHandler serverHandler = new WebSocketHandlerImpl();
        serverHandler.initialize();
        final WebSocketSession serverSession = serverHandler.getWsSession();
        final WSocketOpts serverOptions = new WSocketOptsImpl();
        serverSession.setWebSocketOpts(serverOptions);
        serverSession.getWebSocketOpts().protocols(ALL_SUPPORTED_PROTOCOLS);
        returnCode = serverHandler.readOpeningHandshake(dataBuffer, dataBuffer.limit(), 0, error);

        /*Check that request was properly read.*/
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);
        assertEquals("GET /WebSocket HTTP/1.1", serverSession.getHandshakeOpeningRequest().getHttpRequestConnectionInfo().getConnectionLine());
        assertEquals("GET", serverSession.getHandshakeOpeningRequest().getHttpRequestConnectionInfo().getRequestMethod());
        assertEquals("/WebSocket", serverSession.getHandshakeOpeningRequest().getHttpRequestConnectionInfo().getConnectionUri());
        assertEquals(serverSession.getHost(), "server.example.com");
        assertEquals(serverSession.getAcceptedProtocol(), Codec.RWF_PROTOCOL_TYPE);
        assertEquals(serverSession.getOrigin(), "http://example.com");
        assertTrue(serverSession.isConnectionUpgrade());
        assertTrue(serverSession.isUpgrade());
        //validate WS key
        assertTrue(serverSession.getWsSessionKey().equals(clientSession.getWsSessionKey()));

        dataBuffer.clear();
        returnCode = serverHandler.createResponseHandshake(dataBuffer, error);
        /*Check that response was properly generated.*/
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);
        assertCaseSensitiveResponse(serverSession.getHandshakeOpeningResponse());

        /*read response by a client*/
        returnCode = clientHandler.readResponseHandshake(dataBuffer, dataBuffer.limit(), 0, error);
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);
        final HttpResponseConnectionInfo responseConnectionInfo =
                clientSession.getHandshakeOpeningResponse().getHttpResponseConnectionInfo();
        assertEquals(101, responseConnectionInfo.getResponseStatus());
        assertEquals("Switching Protocols", responseConnectionInfo.getResponseReasonText());
        assertEquals("rssl.rwf", clientSession.getProtocolName());
        assertEquals(Codec.RWF_PROTOCOL_TYPE, clientSession.getAcceptedProtocol());
        assertTrue(clientSession.isUpgrade());
        assertTrue(clientSession.isConnectionUpgrade());
    }

    private void assertCaseSensitiveRequest(HttpRequest request) {
        final List<String> expectedValues = Arrays.stream(WebSocketRequestHeader.values())
                .map(WebSocketRequestHeader::getHeaderTitle)
                .collect(Collectors.toList());
        assertCaseSensitive(request.getRequestHeaders(), expectedValues);
    }

    private void assertCaseSensitiveResponse(HttpResponse response) {
        final List<String> expectedValues = Arrays.stream(WebSocketResponseHeader.values())
                .map(WebSocketResponseHeader::getHeaderTitle)
                .collect(Collectors.toList());
        assertCaseSensitive(response.getResponseHeaders(), expectedValues);
    }

    private void assertCaseSensitive(HttpHeaders httpHeaders, List<String> availableValues) {
        int expectedAmount = httpHeaders.getHeaders().size();
        long actualAmount = availableValues.stream()
                .filter(httpHeaders::containsHeader)
                .count();
        assertEquals(expectedAmount, actualAmount);
    }

    @Test
    public void givenWsSessionWithCallbackFunc_whenExecutesWSHandshake_thenCallbackFunctionWorksProperly() {
        final WebSocketHandler clientHandler = new WebSocketHandlerImpl();
        final WebSocketHandler serverHandler = new WebSocketHandlerImpl();
        clientHandler.initialize();
        serverHandler.initialize();

        final WebSocketSession clientSession = clientHandler.getWsSession();
        clientSession.setWebSocketOpts(new WSocketOptsImpl());
        final WebSocketSession serverSession = serverHandler.getWsSession();
        serverSession.setWebSocketOpts(new WSocketOptsImpl());
        final WSocketOpts clientOptions = clientSession.getWebSocketOpts();
        final WSocketOpts serverOptions = serverSession.getWebSocketOpts();
        final WebSocketCallbackFuncTest callback = new WebSocketCallbackFuncTest(clientSession, serverSession);

        /*fill client session and prepare request*/
        clientSession.applyOriginSending();
        clientSession.setWebSocketUri(URI.create("/WebSocket"));
        clientSession.setOrigin("http://example.com");
        clientSession.setRecoveryVersion(1);
        clientSession.setHost("server.example.com");
        clientSession.getWsSessionKey().data(WS_KEY_EXAMPLE);
        clientOptions.protocols(ALL_SUPPORTED_PROTOCOLS);
        clientOptions.httpCallback(callback);

        serverOptions.protocols(ALL_SUPPORTED_PROTOCOLS);
        serverOptions.httpCallback(callback);

        /*encode handshake request*/
        final ByteBuffer dataBuffer = ByteBuffer.allocate(REQUEST_RESPONSE_BUFFER_SIZE);
        int returnCode = clientHandler.createRequestHandshake(dataBuffer, error);

        if (returnCode < TransportReturnCodes.SUCCESS) {
            fail(error.text());
        }

        /*read request by a server*/
        returnCode = serverHandler.readOpeningHandshake(dataBuffer, dataBuffer.limit(), 0, error);

        if (returnCode < TransportReturnCodes.SUCCESS) {
            fail(error.text());
        }

        dataBuffer.clear();
        returnCode = serverHandler.createResponseHandshake(dataBuffer, error);
        /*Check that response was properly generated.*/
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);

        /*read response by a client, here should be pushed response callback*/
        returnCode = clientHandler.readResponseHandshake(dataBuffer, dataBuffer.limit(), 0, error);
        assertEquals(error.text(), TransportReturnCodes.SUCCESS, returnCode);
    }

    private static class WebSocketCallbackFuncTest implements HttpCallback {

        private final WebSocketSession clientSession;

        private final WebSocketSession serverSession;

        public WebSocketCallbackFuncTest(WebSocketSession clientSession, WebSocketSession serverSession) {
            this.clientSession = clientSession;
            this.serverSession = serverSession;
        }

        @Override
        public void httpCallback(HttpMessage httpMessage, Error error) {
            if (Objects.isNull(httpMessage.getHttpRequestConnectionInfo().getConnectionLine())) {
                assertHttpResponse(httpMessage, error);
            } else {
                assertHttpRequest(httpMessage, error);
            }
        }

        //During request reading names should be case insensitive.
        private void assertHttpRequest(HttpMessage httpRequest, Error error) {
            Map<String, WebSocketRequestHeader> expectedHeaderNames = Arrays.stream(WebSocketRequestHeader.values())
                    .filter(reqHdr -> reqHdr != WebSocketRequestHeader.SEC_WEB_SOCKET_EXTENSIONS)
                    .collect(Collectors.toMap(WebSocketRequestHeader::getHeaderTitleInsensitiveCase, key -> key));
            assertEquals("GET /WebSocket HTTP/1.1", httpRequest.getHttpRequestConnectionInfo().getConnectionLine());
            assertEquals("GET", httpRequest.getHttpRequestConnectionInfo().getRequestMethod());
            assertEquals("/WebSocket", httpRequest.getHttpRequestConnectionInfo().getConnectionUri());
            int headersCount = 0;
            for (HttpHeader httpHeader : httpRequest.getHttpHeaders()) {
                final String headerName = httpHeader.getHeaderName();
                if (expectedHeaderNames.containsKey(headerName)) {
                    ++headersCount;
                    switch (expectedHeaderNames.get(headerName)) {
                        case HOST:
                            assertEquals(clientSession.getCompleteHostName(), httpHeader.getFirstHeaderValue());
                            break;
                        case UPGRADE:
                            assertEquals("websocket", httpHeader.getFirstHeaderValue());
                            break;
                        case CONNECTION:
                            assertEquals("Upgrade", httpHeader.getFirstHeaderValue());
                            break;
                        case SEC_WEB_SOCKET_KEY:
                            assertEquals(clientSession.getWsSessionKey().toString(), httpHeader.getFirstHeaderValue());
                            break;
                        case ORIGIN:
                            assertEquals(clientSession.getOrigin(), httpHeader.getFirstHeaderValue());
                            break;
                        case SEC_WEB_SOCKET_PROTOCOL:
                            assertEquals(clientSession.getWebSocketOpts().protocols(), httpHeader.getFirstHeaderValue());
                            break;
                        case SEC_WEB_SOCKET_VERSION:
                            assertEquals(String.valueOf(clientSession.getRecoveryVersion()), httpHeader.getFirstHeaderValue());
                            break;
                    }
                }
            }
            assertEquals(7, headersCount);
        }

        //We should read case insensitive response.
        private void assertHttpResponse(HttpMessage httpResponse, Error error) {
            Map<String, WebSocketResponseHeader> expectedHeaderNames = Arrays.stream(WebSocketResponseHeader.values())
                    .filter(respHdr -> respHdr != WebSocketResponseHeader.SEC_WEB_SOCKET_EXTENSIONS)
                    .collect(Collectors.toMap(WebSocketResponseHeader::getHeaderTitleCaseInsensitive, key -> key));
            assertEquals(101, httpResponse.getHttpResponseConnectionInfo().getResponseStatus());
            assertEquals("Switching Protocols", httpResponse.getHttpResponseConnectionInfo().getResponseReasonText());
            int headersCount = 0;
            for (HttpHeader httpHeader : httpResponse.getHttpHeaders()) {
                if (expectedHeaderNames.containsKey(httpHeader.getHeaderName())) {
                    ++headersCount;
                    switch (expectedHeaderNames.get(httpHeader.getHeaderName())) {
                        case UPGRADE:
                            assertEquals("websocket", httpHeader.getFirstHeaderValue());
                            break;
                        case CONNECTION:
                            assertEquals("Upgrade", httpHeader.getFirstHeaderValue());
                            break;
                        case SEC_WEB_SOCKET_ACCEPT:
                            assertEquals(serverSession.getWsSessionAcceptKey().toString(), httpHeader.getFirstHeaderValue());
                            break;
                        case SEC_WEB_SOCKET_PROTOCOL:
                            assertEquals(serverSession.getProtocolName(), httpHeader.getFirstHeaderValue());
                            break;
                    }
                }
            }
            assertEquals(4, headersCount);
        }
    }
}
