/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import org.junit.Before;
import org.junit.Test;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;

import static org.junit.Assert.*;

public class HttpParserJUnitTest {

    private static final String JSON_MSG_BODY = "{ var1: \"value1\", var2: \"value2\"}";

    private static final String GET_REQUEST_CORRECT_TEST1 =
            "GET /testing HTTP/1.1\r\n" +
            "Content-type: application/json\r\n" +
            "Accept:application/json\r\n" +
            "Cookie: onecookie=1;twocookie=2;\r\n" +
            "Authorization: got423ot1testsomethin&gcode==\r\n" +
            "Custom-header-1: ==123==\r\n" +
            "Integer-header: 12321\r\n" +
            "Strange-header: \"Particular phrase &head& \\es\\caping\\\"\"; without-symbols\r\n" +
            "\r\n";

    private static final String GET_REQUEST_WITHOUT_URI_TEST2 = "GET HTTP/1.1";

    private static final byte[][] GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3 = new byte[4][];

    //this is correct header for parser, but here Header2 and net is values for Header1.
    private static final String GET_REQUEST_HEADER_WITH_DIFF_DELIMS_TEST4 =
            "GET /testing HTTP/1.1\r\nHeader1: head, Header2: net\n";

    private static final String POST_REQUEST_CORRECT_TEST5 =
            "POST /testing HTTP/1.1\r\n" +
            "Content-type: application/json\r\n" +
            "\r\n" +
            JSON_MSG_BODY;

    private static final String RESPONSE_CORRECT_TEST6 =
            "HTTP/1.1 200 OK\n" +
            "Content-type: application/json\r\n" +
            "Accept:application/json\r\n" +
            "Cookie: onecookie=1;twocookie=2;\r\n" +
            "Authorization: got423ot1testsomethin&gcode==\r\n" +
            "Custom-header-1: ==123==\r\n" +
            "Integer-header: 12321\r\n" +
            "Strange-header: \"Particular phrase &head& \\es\\caping\\\"\"; without-symbols\r\n" +
            "\r\n" +
            JSON_MSG_BODY;

    private static final String RESPONSE_WITHOUT_STATUS_TEST7 = "HTTP/1.1  OK\n";
    
    private static final byte[][] RESPONSE_WITH_INCORRECT_HEADERS_TEST8 = new byte[4][];
    
    static {
    	
    	/* "GET /testing HTTP/1.1\r\nCont@ent-type: application/json\r\n" */
    	GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[0] = new byte[] {71, 69, 84, 32, 47, 116, 101, 115, 116, 105, 110, 103, 32, 72, 84, 84, 80, 47, 49, 46, 49, 13, 10, 67, 111, 110, 116, 64, 101, 110, 116, 45, 116, 121, 112, 101, 58, 32, 97, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 47, 106, 115, 111, 110, 13, 10};
    	
    	/* "GET /testing HTTP/1.1\r\nAnother one: wow\r\n" */
    	GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[1] = new byte[] {71, 69, 84, 32, 47, 116, 101, 115, 116, 105, 110, 103, 32, 72, 84, 84, 80, 47, 49, 46, 49, 13, 10, 65, 110, 111, 116, 104, 101, 114, 32, 111, 110, 101, 58, 32, 119, 111, 119, 13, 10};
    	
    	GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[2] = new byte[] {71, 69, 84, 32, 47, 116, 101, 115, 116, 105, 110, 103, 32, 72, 84, 84, 80, 47, 49, 46, 49, 13, 10, 67, 117, 115, 116, 111, 109, 45, 104, 101, 97, 100, 101, 114, 45, 49, 58, 32, 61, 61, 49, 50, -62, -87, 51, 61, 61, 13, 10};
    	
    	/* "GET /testing HTTP/1.1\r\nHeader1: head\rHeader2: net\n" */
    	GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[3] = new byte[] {71, 69, 84, 32, 47, 116, 101, 115, 116, 105, 110, 103, 32, 72, 84, 84, 80, 47, 49, 46, 49, 13, 10, 72, 101, 97, 100, 101, 114, 49, 58, 32, 104, 101, 97, 100, 13, 72, 101, 97, 100, 101, 114, 50, 58, 32, 110, 101, 116, 10};
    	
    	
    	/*  "HTTP/1.1 200 OK\r\nCont@ent-type: application/json\r\n" */
    	RESPONSE_WITH_INCORRECT_HEADERS_TEST8[0] = new byte[] {72, 84, 84, 80, 47, 49, 46, 49, 32, 50, 48, 48, 32, 79, 75, 13, 10, 67, 111, 110, 116, 64, 101, 110, 116, 45, 116, 121, 112, 101, 58, 32, 97, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 47, 106, 115, 111, 110, 13, 10};
    
    	/* "HTTP/1.1 200 OK\r\nAnother one: wow\r\n" */
    	RESPONSE_WITH_INCORRECT_HEADERS_TEST8[1] = new byte[] {72, 84, 84, 80, 47, 49, 46, 49, 32, 50, 48, 48, 32, 79, 75, 13, 10, 65, 110, 111, 116, 104, 101, 114, 32, 111, 110, 101, 58, 32, 119, 111, 119, 13, 10};
    	
    	RESPONSE_WITH_INCORRECT_HEADERS_TEST8[2] = new byte[] {72, 84, 84, 80, 47, 49, 46, 49, 32, 50, 48, 48, 32, 79, 75, 13, 10, 67, 117, 115, 116, 111, 109, 45, 104, 101, 97, 100, 101, 114, 45, 49, 58, 32, 61, 61, 49, 50, -62, -87, 51, 61, 61, 13, 10};
    	
    	/* "HTTP/1.1 200 OK\r\nHeader1: head\rHeader2: net\n" */
    	RESPONSE_WITH_INCORRECT_HEADERS_TEST8[3] = new byte[] {72, 84, 84, 80, 47, 49, 46, 49, 32, 50, 48, 48, 32, 79, 75, 13, 10, 72, 101, 97, 100, 101, 114, 49, 58, 32, 104, 101, 97, 100, 13, 72, 101, 97, 100, 101, 114, 50, 58, 32, 110, 101, 116, 10};
    }

    private final HttpRequest httpRequest = new HttpRequest();
    private final HttpResponse httpResponse = new HttpResponse();
    private final HttpMessageHandler httpMessageHandler = new HttpMessageHandlerImpl();
    private final ByteBuffer byteData = ByteBuffer.allocate(1024);
    private final Error error = TransportFactory.createError();

    public HttpParserJUnitTest() {
        httpMessageHandler.initialize();
    }

    @Before
    public void clearAllGlobals() {
        httpRequest.clear();
        httpResponse.clear();
        httpMessageHandler.clear();
        byteData.clear();
        error.clear();
    }

    //Test 1
    @Test
    public void givenCorrectGetRequest_whenRequestShouldBeParsed_thenAllDataCouldBeObtainedCorrectly() {
        byteData.put(GET_REQUEST_CORRECT_TEST1.getBytes());
        int returnCode = httpMessageHandler.parseHttpRequest(httpRequest, byteData, GET_REQUEST_CORRECT_TEST1.length(), 0, error);
        assertEquals(ParserReturnCodes.SUCCESS, returnCode);
        assertFalse(httpRequest.getRequestHeaders().isEmpty());

        /*accept connection info*/
        HttpRequestConnectionInfo httpRequestConnectionInfo = httpRequest.getHttpRequestConnectionInfo();
        assertEquals("GET", httpRequestConnectionInfo.getRequestMethod());
        assertEquals("/testing", httpRequestConnectionInfo.getConnectionUri());
        assertEquals("GET /testing HTTP/1.1", httpRequestConnectionInfo.getConnectionLine());
        int headersCount = 0;
        for (HttpHeader httpHeader : httpRequest.getRequestHeaders().getHeaders()) {
            switch (httpHeader.getHeaderName().toLowerCase()) {
                case "content-type":
                case "accept":
                    ++headersCount;
                    assertEquals("application/json", httpHeader.getFirstHeaderValue());
                    break;
                case "authorization":
                    ++headersCount;
                    assertEquals("got423ot1testsomethin&gcode==", httpHeader.getFirstHeaderValue());
                    break;
                case "custom-header-1":
                    ++headersCount;
                    assertEquals("==123==", httpHeader.getFirstHeaderValue());
                    break;
                case "integer-header":
                    ++headersCount;
                    int integerHeaderValue = Integer.parseUnsignedInt(httpHeader.getSimpleHeaderValue());
                    assertEquals(12321, integerHeaderValue);
                    break;
                case "strange-header":
                    ++headersCount;
                    final List<String> strangeHeaderValues = httpMessageHandler.getStringValues(httpHeader);
                    assertEquals(2, strangeHeaderValues.size());
                    assertTrue(strangeHeaderValues.contains("Particular phrase &head& \\es\\caping\\\""));
                    assertTrue(strangeHeaderValues.contains("without-symbols"));
                    break;
            }
        }
        assertEquals(6, headersCount);

        /*assert that cookies was parsed*/
        final Map<String, String> cookies = httpRequest.getRequestHeaders().getCookies();
        assertFalse(cookies.isEmpty());
        assertEquals("1", cookies.get("onecookie"));
        assertEquals("2", cookies.get("twocookie"));
    }

    //Test 2
    @Test
    public void givenGetRequestWithoutURI_whenRequestShouldBeParsed_thenReturnInvalidConnectionLineFailure() {
        byteData.put(GET_REQUEST_WITHOUT_URI_TEST2.getBytes());
        int returnCode = httpMessageHandler.parseHttpRequest(httpRequest, byteData, GET_REQUEST_CORRECT_TEST1.length(), 0, error);
        assertEquals(ParserReturnCodes.INVALID_HEADER_GROUP, returnCode);
    }

    //Test 3
    @Test
    public void givenFewGetRequestsWithIncorrectHeaders_whenRequestShouldBeParsed_thenReturnHeaderErrors() throws UnsupportedEncodingException {
        int start = 0;
        for (int i = 0; i < GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3.length; i++) {
            byteData.put(GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[i]);
            int returnCode = httpMessageHandler.parseHttpRequest(httpRequest, byteData, GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[i].length, start, error);
            assertEquals("Request assertion was failed for: \n" + new String(GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[i], "UTF-8"), ParserReturnCodes.INVALID_HEADER_GROUP, returnCode);
            start += GET_REQUEST_ARRAY_WITH_INCORRECT_HEADERS_TEST3[i].length;
            httpRequest.clear();
        }
    }

    //Test 4
    @Test
    public void givenRequestWithStrangeValidHeader_whenItShouldBeParsed_thenReturnSuccess() {
        final String headerNameForCheck = "header1";
        byteData.put(GET_REQUEST_HEADER_WITH_DIFF_DELIMS_TEST4.getBytes());
        int returnCode = httpMessageHandler.parseHttpRequest(httpRequest, byteData,
                GET_REQUEST_HEADER_WITH_DIFF_DELIMS_TEST4.length(), 0, error);

        assertEquals(ParserReturnCodes.SUCCESS, returnCode);
        assertTrue(httpRequest.getRequestHeaders().containsHeader(headerNameForCheck));
        List<String> headerValues = httpMessageHandler.getStringValues(httpRequest.getRequestHeaders().getHeader(headerNameForCheck));
        assertEquals(3, headerValues.size());
        assertTrue(headerValues.contains("head"));
        assertTrue(headerValues.contains("Header2"));
        assertTrue(headerValues.contains("net"));
    }

    //Test 5
    @Test
    public void givenValidPostRequest_whenRequestShouldBeParsed_thenReturnSuccess() {
        byteData.put(POST_REQUEST_CORRECT_TEST5.getBytes());
        int returnCode = httpMessageHandler.parseHttpRequest(httpRequest, byteData, POST_REQUEST_CORRECT_TEST5.length(), 0, error);
        assertEquals(ParserReturnCodes.SUCCESS, returnCode);
        assertTrue(httpRequest.getRequestHeaders().containsHeader("content-type"));
        assertEquals("POST", httpRequest.getHttpRequestConnectionInfo().getRequestMethod());
        assertEquals(JSON_MSG_BODY, httpRequest.getRequestBody().toString());
    }

    //Test 6
    @Test
    public void givenCorrectResponse_whenResponseShouldBeParsed_thenAllDataCouldBeObtainedCorrectly() {
        byteData.put(RESPONSE_CORRECT_TEST6.getBytes());
        int returnCode = httpMessageHandler.parseHttpResponse(httpResponse, byteData, RESPONSE_CORRECT_TEST6.length(), 0, error);
        assertEquals(ParserReturnCodes.SUCCESS, returnCode);
        assertFalse(httpResponse.getResponseHeaders().isEmpty());

        /*accept connection info*/
        HttpResponseConnectionInfo httpResponseConnectionInfo = httpResponse.getHttpResponseConnectionInfo();
        assertEquals(200, httpResponseConnectionInfo.getResponseStatus());
        assertEquals("HTTP/1.1 200 OK", httpResponseConnectionInfo.getConnectionLine());
        int headersCount = 0;
        for (HttpHeader httpHeader : httpResponse.getResponseHeaders().getHeaders()) {
            switch (httpHeader.getHeaderName().toLowerCase()) {
                case "content-type":
                case "accept":
                    ++headersCount;
                    assertEquals("application/json", httpHeader.getFirstHeaderValue());
                    break;
                case "authorization":
                    ++headersCount;
                    assertEquals("got423ot1testsomethin&gcode==", httpHeader.getFirstHeaderValue());
                    break;
                case "custom-header-1":
                    ++headersCount;
                    assertEquals("==123==", httpHeader.getFirstHeaderValue());
                    break;
                case "integer-header":
                    ++headersCount;
                    int integerHeaderValue = Integer.parseUnsignedInt(httpHeader.getSimpleHeaderValue());
                    assertEquals(12321, integerHeaderValue);
                    break;
                case "strange-header":
                    ++headersCount;
                    final List<String> strangeHeaderValues = httpMessageHandler.getStringValues(httpHeader);
                    assertEquals(2, strangeHeaderValues.size());
                    assertTrue(strangeHeaderValues.contains("Particular phrase &head& \\es\\caping\\\""));
                    assertTrue(strangeHeaderValues.contains("without-symbols"));
                    break;
            }
        }
        assertEquals(6, headersCount);

        /*assert that cookies was parsed*/
        final Map<String, String> cookies = httpResponse.getResponseHeaders().getCookies();
        assertFalse(cookies.isEmpty());
        assertEquals("1", cookies.get("onecookie"));
        assertEquals("2", cookies.get("twocookie"));

        /*assert response body*/
        assertEquals(JSON_MSG_BODY, httpResponse.getResponseBody().toString());
    }

    //Test 7
    @Test
    public void givenGetResponseWithoutURI_whenResponseShouldBeParsed_thenReturnInvalidConnectionLineFailure() {
        byteData.put(RESPONSE_WITHOUT_STATUS_TEST7.getBytes());
        int returnCode = httpMessageHandler.parseHttpResponse(httpResponse, byteData, RESPONSE_WITHOUT_STATUS_TEST7.length(), 0, error);
        assertEquals(ParserReturnCodes.INVALID_CONNECTION_LINE, returnCode);
    }

    //Test 8
    @Test
    public void givenFewGetResponsesWithIncorrectHeaders_whenResponseShouldBeParsed_thenReturnHeaderErrors() throws UnsupportedEncodingException {
        int start = 0;
        for (int i = 0; i < RESPONSE_WITH_INCORRECT_HEADERS_TEST8.length; i++) {
            byteData.put(RESPONSE_WITH_INCORRECT_HEADERS_TEST8[i]);
            int returnCode = httpMessageHandler.parseHttpResponse(httpResponse, byteData, RESPONSE_WITH_INCORRECT_HEADERS_TEST8[i].length, start, error);
            assertEquals("Response assertion was failed for: \n" + new String(RESPONSE_WITH_INCORRECT_HEADERS_TEST8[i], "UTF-8"), ParserReturnCodes.INVALID_HEADER_GROUP, returnCode);
            start += RESPONSE_WITH_INCORRECT_HEADERS_TEST8[i].length;
            httpResponse.clear();
        }
    }
}
