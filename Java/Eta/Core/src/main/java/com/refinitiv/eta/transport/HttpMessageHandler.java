package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;

import java.nio.ByteBuffer;
import java.util.List;

interface HttpMessageHandler {

    /**
     * Initialize the parser.
     */
    void initialize();

    /**
     * Clear this instance of the parser.
     */
    void clear();

    /**
     * Parse and set data into particular {@link HttpRequest} instance.
     *
     * @param httpRequest - http request entity to which data will be written.
     * @param data        - data buffer with in request content.
     * @param dataLength  - data length of the request content.
     * @param start       - start index of the request content in data buffer.
     * @param error       - error buffer with encapsulating of errors and warning during parsing.
     * @return {@link ParserReturnCodes#SUCCESS} if request was parsed successfully and entity has been filled.
     * {@link ParserReturnCodes#FAILURE} or less if request was parsed with errors.
     */
    int parseHttpRequest(HttpRequest httpRequest, ByteBuffer data, int dataLength, int start, Error error);

    /**
     * Parse and set data into particular {@link HttpResponse} instance.
     *
     * @param httpResponse - http response entity to which data will be written.
     * @param data         - data buffer with in response content.
     * @param dataLength   - data length of the response content.
     * @param start        - start index of the response content in data buffer.
     * @param error        - error buffer with encapsulating of errors and warning during parsing.
     * @return {@link ParserReturnCodes#SUCCESS} if response was parsed successfully and entity has been filled.
     * {@link ParserReturnCodes#FAILURE} or less if response was parsed with errors.
     */
    int parseHttpResponse(HttpResponse httpResponse, ByteBuffer data, int dataLength, int start, Error error);

    /**
     * Read the first line of request content.
     *
     * @param httpRequest entity to which information will be written
     * @param data        buffer of the request content
     * @param dataLength  of the request content
     * @param start       position of the request content
     * @param error       buffer for encapsulating errors and warnings.
     * @return {@link ParserReturnCodes#SUCCESS} if parsing was finished successfully.
     * Will return {@link ParserReturnCodes#FAILURE} or less if parsing was finished with errors.
     */
    int readRequestFirstLine(HttpRequest httpRequest, ByteBuffer data, int dataLength, int start, Error error);

    /**
     * Read the first line of response content.
     *
     * @param httpResponse - entity to which information will be written
     * @param data         - buffer of the response content
     * @param dataLength   - of the response content
     * @param start        - position of the response content
     * @param error        - buffer for encapsulating errors and warnings.
     * @return {@link ParserReturnCodes#SUCCESS} if parsing was finished with errors.
     * Will return {@link ParserReturnCodes#FAILURE} or less if parsing was finished with errors.
     */
    int readResponseFirstLine(HttpResponse httpResponse, ByteBuffer data, int dataLength, int start, Error error);

    /**
     * Reads HttpHeaders from content data to specified entity.
     *
     * @param httpHeaders - of particular {@link HttpRequest} or {@link HttpResponse} instance.
     * @param data        - buffer of the request content.
     * @param dataLength  - of the request content.
     * @param start       - position of the request content.
     * @param error       - buffer for encapsulating errors and warnings.
     * @return {@link ParserReturnCodes#SUCCESS} if headers successfully filled. Will return status less or equals of
     * {@link ParserReturnCodes#FAILURE} if parsing of headers finish with errors.
     */
    int readHttpHeaders(HttpHeaders httpHeaders, ByteBuffer data, int dataLength, int start, Error error);

    /**
     * Emphasizes request/response body to the specified buffer.
     *
     * @param bodyBuffer - buffer to which request/response body should be written.
     * @param data       - byte buffer which should be analyzed for existence of request/response body.
     * @param dataLength - length of request/response body in the data buffer.
     * @param start      - start position of request/response body in the data buffer.
     * @param error      - buffer for encapsulating errors and warnings.
     * @return {@link ParserReturnCodes#SUCCESS} if body had been found and successfully was written.
     */
    int parseHttpBody(Buffer bodyBuffer, ByteBuffer data, int dataLength, int start, Error error);

    /**
     * Get string values with splitting across delimiters according to standard RFC specification.
     *
     * @param httpHeader - header which should be parsed.
     * @return list of string values.
     */
    List<String> getStringValues(HttpHeader httpHeader);

    /**
     *
     * @param httpRequest
     * @param messageRequestBuffer
     * @param error
     * @return
     */
    int encodeRequestMessage(HttpRequest httpRequest, ByteBuffer messageRequestBuffer, Error error);

    /**
     *
     * @param httpResponse
     * @param messageResponseBuffer
     * @param error
     * @return
     */
    int encodeResponseMessage(HttpResponse httpResponse, ByteBuffer messageResponseBuffer, Error error);
}
