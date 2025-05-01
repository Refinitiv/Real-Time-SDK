/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecReturnCodes;
import org.apache.commons.codec.Charsets;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class HttpMessageHandlerImpl implements HttpMessageHandler {

    private static final byte[] HTTP_PROTOCOL_BYTES = new byte[]{0x48, 0x54, 0x54, 0x50, 0X2F, 0x31, 0X2E, 0x31};

    private static final byte HEADER_TAG_DELIMITER = 0x3A;
    private static final byte SPACE = 0x20;

    private static final byte LF = 0x0A;
    private static final byte CR = 0x0D;

    private static final String COOKIE_HEADER_NAME = "Cookie";

    private static final String COOKIE_ENTRY_DELIMITER = ";";
    private static final char COOKIE_KEY_VALUE_DELIMITER = '=';

    /**
     * {@link Matcher#group()} - complete matching.
     * group 1 - matching for value in quotes (in case if matches on first condition)
     * group 2 - usually, last symbol of group 1.
     * group 3 - matching for value without quotes
     */
    private static final String HEADER_VALUE_REGEXP_STR =
            "\\x22(([\\x09\\x20\\x21\\x23-\\x5B\\x5D-\\x7E\\x80-\\xFF]|\\x5c[\\x09\\x20\\x21-\\x7E\\x80-\\xFF])+)\\x22|([a-zA-Z0-9!#$%&'*+\\-.^_`|~]+)"; //simple string

    /**
     * Group 1 - full matching.
     * Group 2 - request type (GET, POST).
     * Group 3 - request URI.
     */
    private static final Pattern URI_REQUEST_LINE_REGEXP = Pattern.compile(
            "((GET|POST|HEAD|PUT|DELETE|CONNECT|OPTIONS|TRACE) " + //Request method
            "(.+) " + //request URI
            "HTTP\\/1\\.1\\x0D?\\x0A)" //HTTP protocol and end of line.
    );

    /**
     * Group 1 - header tag;
     * Group 2 - header value.
     */
    private static final Pattern HEADER_REGEXP = Pattern.compile(
            "([a-zA-Z0-9!#$%&'*+\\-.^_`|~&&[^(),/:;<=>?@}\\x7B\\x5B-\\x5D\\x0A\\x0D\\x22]]+):\\s?((" + //header name  RFC7230 p.3.2.6 => token
            HEADER_VALUE_REGEXP_STR + //header values
            "|([(),\\/:;<=>?@}\\x7B\\x5B-\\x5D\\x22\\x20]))*)\\x0D?\\x0A" //possible delimiters
    );

    /**
     * Group 1 - full matching.
     * Group 2 - response status code.
     * Group 3 - response status reason text.
     */
    private static final Pattern RESPONSE_FIRST_LINE_REGEXP = Pattern.compile(
            "(HTTP\\/1\\.1 (\\d{3})\\s(.*)\\x0D?\\x0A)"
    );

    /**
     * @see #HEADER_VALUE_REGEXP_STR
     */
    private static final Pattern HEADER_VALUE_REGEXP = Pattern.compile(
            HEADER_VALUE_REGEXP_STR
    );

    private StringBuilder lineBuilder;

    @Override
    public void initialize() {
        lineBuilder = new StringBuilder();
    }

    @Override
    public void clear() {
        clearLineBuilder();
    }

    public int parseHttpRequest(
            HttpRequest httpRequest, ByteBuffer data, int dataLength, int start, Error error) {
        int groupStart = start;
        int groupLength = dataLength;

        //Parse request connection line
        int returnCode = readRequestFirstLine(httpRequest, data, groupLength, groupStart, error);
        if (returnCode < ParserReturnCodes.SUCCESS) {
            return returnCode;
        }
        //Parse request headers
        groupStart = Math.addExact(groupStart, httpRequest.getHttpRequestConnectionInfo().getContentLength());
        groupLength = Math.subtractExact(groupLength, httpRequest.getHttpRequestConnectionInfo().getContentLength());
        returnCode = readHttpHeaders(httpRequest.getRequestHeaders(), data, groupLength, groupStart, error);
        if (returnCode < ParserReturnCodes.SUCCESS) {
            return returnCode;
        }
        //Parse request body
        groupStart = Math.addExact(groupStart, httpRequest.getRequestHeaders().getHeaderGroupLength());
        groupLength = Math.subtractExact(groupLength, httpRequest.getRequestHeaders().getHeaderGroupLength());
        returnCode = parseHttpBody(httpRequest.getRequestBody(), data, groupLength, groupStart, error);
        return returnCode;
    }

    public int parseHttpResponse(
            HttpResponse httpResponse, ByteBuffer data, int dataLength, int start, Error error) {
        int groupStart = start;
        int groupLength = dataLength;
        //Parse response status line
        int returnCode = readResponseFirstLine(httpResponse, data, groupLength, groupStart, error);
        if (returnCode < ParserReturnCodes.SUCCESS) {
            return returnCode;
        }
        //Parse response headers
        groupStart = Math.addExact(groupStart, httpResponse.getHttpResponseConnectionInfo().getContentLength());
        groupLength = Math.subtractExact(groupLength, httpResponse.getHttpResponseConnectionInfo().getContentLength());
        returnCode = readHttpHeaders(httpResponse.getResponseHeaders(), data, groupLength, groupStart, error);
        if (returnCode < ParserReturnCodes.SUCCESS) {
            return returnCode;
        }
        //Parse response body
        groupStart = Math.addExact(groupStart, httpResponse.getResponseHeaders().getHeaderGroupLength());
        groupLength = Math.subtractExact(groupLength, httpResponse.getResponseHeaders().getHeaderGroupLength());
        returnCode = parseHttpBody(httpResponse.getResponseBody(), data, groupLength, groupStart, error);
        return returnCode;
    }


    public int readRequestFirstLine(HttpRequest httpRequest, ByteBuffer data, int dataLength, int start, Error error) {
        if (dataLength <= 0) {
        	return ParserReturnCodes.INVALID_CONNECTION_LINE;
        }
        clearLineBuilder();
        HeaderLineParseState lineParseState;
        int bufferLength = start + dataLength;
        for (int i = start; i < bufferLength; i++) {
            lineParseState = checkEndOfLine(data, i);
            if (lineParseState.isLineEnd() || i == bufferLength - 1) {
                Matcher firstLineMatcher = URI_REQUEST_LINE_REGEXP.matcher(lineBuilder);
                if (!firstLineMatcher.matches()) {
                	return ParserReturnCodes.SUCCESS;
                }
                final HttpRequestConnectionInfo httpRequestConnectionInfo = httpRequest.getHttpRequestConnectionInfo();
                final String connectionLine = firstLineMatcher.group(1);
                httpRequestConnectionInfo.setConnectionLine(connectionLine.trim());
                httpRequestConnectionInfo.setRequestMethod(firstLineMatcher.group(2));
                httpRequestConnectionInfo.setConnectionUri(firstLineMatcher.group(3));
                httpRequestConnectionInfo.setContentLength(connectionLine.length());
                return ParserReturnCodes.SUCCESS;
            }
        }
        return populateError(error, ParserReturnCodes.INVALID_CONNECTION_LINE, "Error during parsing the first line.");
    }

    public int readResponseFirstLine(HttpResponse httpResponse, ByteBuffer data, int dataLength, int start, Error error) {
        if (dataLength <= 0) {
        	return ParserReturnCodes.INVALID_CONNECTION_LINE;
        }
        clearLineBuilder();
        HeaderLineParseState headerLineParseState;
        int bufferLength = start + dataLength;
        for (int i = start; i < bufferLength; i++) {
            headerLineParseState = checkEndOfLine(data, i);
            if (headerLineParseState.isLineEnd() || i == bufferLength - 1) {
                Matcher firstLineMatcher = RESPONSE_FIRST_LINE_REGEXP.matcher(lineBuilder);
                if (!firstLineMatcher.matches()) {
                    return populateError(error, ParserReturnCodes.INVALID_CONNECTION_LINE,
                            "Invalid connect string for client: " + lineBuilder);
                }
                final HttpResponseConnectionInfo responseConnectionInfo = httpResponse.getHttpResponseConnectionInfo();
                final String connectionLine = firstLineMatcher.group(1);
                responseConnectionInfo.setConnectionLine(connectionLine.trim());
                //parse without checking because already had checked on matching to regex '\d{3}'
                responseConnectionInfo.setResponseStatus(Integer.parseInt(firstLineMatcher.group(2)));
                responseConnectionInfo.setResponseReasonText(firstLineMatcher.group(3));
                responseConnectionInfo.setContentLength(connectionLine.length());
                return ParserReturnCodes.SUCCESS;
            }
        }
        return populateError(error, ParserReturnCodes.INVALID_CONNECTION_LINE, "Error during parsing the first line.");
    }

    /**
     * @param buffer     - buffer to which http request/response body should be written.
     * @param data       - byte buffer which should be analyzed for existence of request/response body.
     * @param dataLength - length of request/response body in the data buffer.
     * @param start      - start position of request/response body in the data buffer.
     * @param error      - buffer for encapsulating errors and warnings.
     * @return
     */
    public int parseHttpBody(Buffer buffer, ByteBuffer data, int dataLength, int start, Error error) {
        if (dataLength <= 0) {
            return ParserReturnCodes.SUCCESS;
        }
        final byte[] bodyBytes = new byte[dataLength];
        final ByteBuffer bodyByteBuffer = ByteBuffer.wrap(bodyBytes);
        data.position(start);
        data.get(bodyBytes);
        buffer.clear();
        buffer.data(bodyByteBuffer);
        return ParserReturnCodes.SUCCESS;
    }

    /**
     * @param httpHeaders of particular {@link HttpRequest} or {@link HttpResponse} instance.
     * @param data        buffer of the request content.
     * @param dataLength  of the request content.
     * @param start       position of the request content.
     * @param error       buffer for encapsulating errors and warnings.
     * @return {@link ParserReturnCodes#SUCCESS} if headers successfully filled. Will return status less or equals of
     * {@link ParserReturnCodes#FAILURE} if parsing finishes with errors.
     */
    public int readHttpHeaders(HttpHeaders httpHeaders, ByteBuffer data, int dataLength, int start, Error error) {
        if (dataLength <= 0) {
            return populateError(error, ParserReturnCodes.HEADERS_NOT_PRESENTED, "Header group is empty.");
        }
        clearLineBuilder();
        int returnCode;
        int sbCursor = 0;
        int lineAmount = 0;
        int bufferLength = start + dataLength;
        HeaderLineParseState headerLineParseState;
        for (int i = start; i < bufferLength; i++) {
            headerLineParseState = checkEndOfLine(data, i);
            i += headerLineParseState.getIncrementalValue();
            if (headerLineParseState.isLineEnd() || i >= bufferLength - 1) {
                //end of header line
                String headerLine = lineBuilder.substring(sbCursor);
                ++lineAmount;
                //add header into pull
                returnCode = validateAndPushHeader(
                        httpHeaders, headerLine, lineAmount, headerLine.length(), start + sbCursor, error);
                if (returnCode < ParserReturnCodes.SUCCESS) {
                    return returnCode;
                }

                //move cursor of string builder and byte buffer data
                sbCursor = lineBuilder.length();

                //check for end of header's section
                if (i < bufferLength) {
                    headerLineParseState = checkEndOfLine(data, i);
                    i += headerLineParseState.getIncrementalValue();
                    if (headerLineParseState.isLineEnd()) {
                        break;
                    }
                }
            }
        }
        httpHeaders.setHeaderGroupLength(lineBuilder.length());
        returnCode = parseCookies(httpHeaders, error);
        return returnCode;
    }

    public List<String> getStringValues(HttpHeader httpHeader) {
        final List<String> headerValues = new ArrayList<>();
        final String httpHeaderSetValues = httpHeader.getSimpleHeaderValue();
        final Matcher matcher = HEADER_VALUE_REGEXP.matcher(httpHeaderSetValues);
        while (matcher.find()) {
            final String completeMatch = matcher.group();
            if (completeMatch.startsWith("\"")) {
                headerValues.add(matcher.group(1));
            } else {
                headerValues.add(completeMatch);
            }
        }
        return headerValues;
    }

    @Override
    public int encodeRequestMessage(HttpRequest httpRequest, ByteBuffer messageRequestBuffer, Error error) {
        final HttpRequestConnectionInfo connectionInfo = httpRequest.getHttpRequestConnectionInfo();
        try {
            //encode first line of request
            messageRequestBuffer
                    .put(connectionInfo.getRequestMethod().getBytes(Charsets.US_ASCII))
                    .put(SPACE)
                    .put(connectionInfo.getConnectionUri().getBytes(Charsets.US_ASCII))
                    .put(SPACE)
                    .put(HTTP_PROTOCOL_BYTES)
                    .put(CR).put(LF);

            //encode headers
            httpRequest.getRequestHeaders().getHeaders().forEach(
                    header -> messageRequestBuffer
                            .put(header.getHeaderName().getBytes(Charsets.US_ASCII))
                            .put(HEADER_TAG_DELIMITER)
                            .put(SPACE)
                            .put(header.getSimpleHeaderValue().getBytes(Charsets.US_ASCII))
                            .put(CR).put(LF)
            );

            messageRequestBuffer.put(CR).put(LF);

            final Buffer requestBody = httpRequest.getRequestBody();
            if (Objects.nonNull(requestBody.data())) {
                messageRequestBuffer
                        .put(requestBody.data())
                        .put(CR).put(LF);
            }
        } catch (BufferOverflowException e) {
            populateError(error, CodecReturnCodes.FAILURE, "Passed buffer cannot keep completely the specified http request.");
        }
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int encodeResponseMessage(HttpResponse httpResponse, ByteBuffer messageResponseBuffer, Error error) {
        final HttpResponseConnectionInfo connectionInfo = httpResponse.getHttpResponseConnectionInfo();
        try {
            //encode first line of response
            messageResponseBuffer
                    .put(HTTP_PROTOCOL_BYTES)
                    .put(SPACE)
                    .put(String.valueOf(httpResponse.getHttpResponseConnectionInfo().getResponseStatus()).getBytes(Charsets.US_ASCII))
                    .put(SPACE)
                    .put(connectionInfo.getResponseReasonText().getBytes(Charsets.US_ASCII))
                    .put(CR).put(LF);

            //encode response headers
            httpResponse.getResponseHeaders().getHeaders().forEach(
                    header -> messageResponseBuffer
                            .put(header.getHeaderName().getBytes(Charsets.US_ASCII))
                            .put(HEADER_TAG_DELIMITER)
                            .put(SPACE)
                            .put(header.getSimpleHeaderValue().getBytes(Charsets.US_ASCII))
                            .put(CR).put(LF)
            );

            messageResponseBuffer.put(CR).put(LF);

            final Buffer responseBody = httpResponse.getResponseBody();
            if (Objects.nonNull(responseBody.data())) {
                messageResponseBuffer
                        .put(responseBody.data())
                        .put(CR).put(LF);
            }
        } catch (BufferOverflowException e) {
            populateError(error, CodecReturnCodes.FAILURE, "Passed buffer cannot keep completely the specified http response.");
        }
        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Try to parse cookie header from the list of headers.
     *
     * @param httpHeaders - list of all incoming headers.
     * @param error       - error buffer for storing error and warning messages.
     * @return {@link ParserReturnCodes#SUCCESS} if parsing finishes successfully;
     * Will be returned {@link ParserReturnCodes#INVALID_COOKIES} or less if parsing finishes with errors.
     */
    public int parseCookies(HttpHeaders httpHeaders, Error error) {
        final HttpHeader cookieHeader = httpHeaders.getHeader(COOKIE_HEADER_NAME, false);
        if (Objects.isNull(cookieHeader)) {
            return ParserReturnCodes.SUCCESS;
        }
        try {
            cookieHeader.getHeaderInfo().stream()
                    .map(HttpHeaderLineInfo::getHeaderValue)
                    .flatMap(cookieValue -> Arrays.stream(cookieValue.split(COOKIE_ENTRY_DELIMITER)))
                    .filter(s -> !s.trim().isEmpty())
                    .forEach(cookieValue -> {
                        int equalIndex = cookieValue.indexOf(COOKIE_KEY_VALUE_DELIMITER);
                        httpHeaders.getCookies()
                                .put(cookieValue.substring(0, equalIndex), cookieValue.substring(equalIndex + 1));
                    });
        } catch (Exception e) {
            return populateError(error, ParserReturnCodes.INVALID_COOKIES, "Failed during parse the cookie header.");
        }
        return ParserReturnCodes.SUCCESS;
    }

    /**
     * @param data   of request/response.
     * @param offset in data where byte should be read.
     * @return last readed symbol.
     */
    private HeaderLineParseState checkEndOfLine(ByteBuffer data, int offset) {
        HeaderLineParseState returnedState = HeaderLineParseState.CONTINUE;
        int localOffset = offset;
        byte readByte = data.get(localOffset);
        lineBuilder.append((char) (readByte & 0xFF));

        if (readByte == CR) {
            ++localOffset;
            readByte = data.get(localOffset);
            lineBuilder.append((char) (readByte & 0xFF));
            returnedState = HeaderLineParseState.CONTINUE_WITH_TRANSITION;
        }

        //Although the line terminator for the start-line and header fields is
        //the sequence CRLF, a recipient MAY recognize a single LF as a line
        //terminator and ignore any preceding CR.
        if (readByte == LF) {
            returnedState = HeaderLineParseState.getEndState(returnedState);
        }
        return returnedState;
    }

    //Used only for reading.
    private int validateAndPushHeader(HttpHeaders httpHeaders, String headerLine, int lineNumber, int lineLength, int bufferOffsetStart, Error error) {
        final Matcher headerMatcher = HEADER_REGEXP.matcher(headerLine);
        if (headerMatcher.matches()) {
            String headerName = headerMatcher.group(1);
            String headerValue = headerMatcher.group(2);
            final HttpHeaderLineInfo line = HttpHeaderLineInfo.builder()
                    .headerValue(headerValue)
                    .lineNumber(lineNumber)
                    .dataLength(lineLength)
                    .startOffset(bufferOffsetStart)
                    .build();
            httpHeaders.addHeader(headerName, line, false);
            return ParserReturnCodes.SUCCESS;
        }
        return populateError(error, ParserReturnCodes.INVALID_HEADER_GROUP,
                "Error during validating header line %d with value: %s", lineNumber, headerLine);
    }

    private int populateError(Error error, int errorId, String text, Object... args) {
        error.errorId(errorId);
        error.text(String.format(text, args));
        return errorId;
    }

    private void clearLineBuilder() {
        if (Objects.nonNull(lineBuilder)) {
            lineBuilder.delete(0, lineBuilder.length());
        }
    }
}
