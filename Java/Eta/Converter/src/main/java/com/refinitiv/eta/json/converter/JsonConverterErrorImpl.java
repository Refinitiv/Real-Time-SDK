/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import java.util.stream.Collectors;
import java.util.stream.IntStream;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterErrorCodes.*;

class JsonConverterErrorImpl implements JsonConverterError {

    private int errorId;
    private String text;
    private String caller;
    private String key;
    String declaringClass;
    int line = -1;

    private String callerToString() {
        StackTraceElement[] stackTraceElements = Thread.currentThread().getStackTrace();

        declaringClass = stackTraceElements[4].getClassName();
        line = stackTraceElements[4].getLineNumber();

        return stackTraceElements[3] + "\n\n"+
                IntStream.range(0, stackTraceElements.length)
                        .mapToObj(i -> Integer.toString(i)  + " " + stackTraceElements[i])
                        .collect(Collectors.joining("\n"));
    }

    @Override
    public int setEncodeError(int encodeErrorId, String key) {
        return setError(JSON_ERROR_RSSL_ENCODE_ERROR,"code=" + encodeErrorId + ", '" + key + "' encoding failure");
    }

    @Override
    public int setError(int errorId, String text) {
        this.errorId = errorId;

        switch (errorId) {
            case JSON_ERROR_PARSE_ERROR:
                this.text = "JSON Converter parse error: " + text;
                break;
            case JSON_ERROR_UNSUPPORTED_PROTOCOL:
                this.text = "JSON Converter init error. Unsupported protocol: [" + text + "]. At the moment supported version is JSON Simple (2)";
                break;
            case JSON_ERROR_UNKNOWN_PROPERTY:
                this.text = "JSON Converter init error. Invalid property: " + text;
                break;
            case JSON_ERROR_INVALID_TOKEN_TYPE:
                this.text = "JSON Converter Token Type error: " + text;
                break;
            case JSON_ERROR_UNEXPECTED_KEY:
                this.text = "JSON Converter unexpected key. Received " + text;
                break;
            case JSON_ERROR_UNSUPPORTED_MESSAGE:
                this.text = "JSON Converter parsing is not supported: " + text;
                break;
            case JSON_ERROR_UNEXPECTED_VALUE:
                this.text = "JSON Converter unexpected value: " + text;
                break;
            case JSON_ERROR_RSSL_ENCODE_ERROR:
                this.text = "JSON Converter encode error: " + text;
                break;
            case JSON_ERROR_MISSING_KEY:
                this.text = "JSON Converter missing key: " + text;
                break;
            case JSON_ERROR_OPERATION_NOT_SUPPORTED:
                this.text = "Operation not supported for class: " + text;
                break;
            default:
                this.text = "JSON Converter error: " + text;
        }

        caller = callerToString();

        if(errorId != SUCCESS)
            return FAILURE;

        return  errorId;
    }

    @Override
    public int getCode() {
        return errorId;
    }

    @Override
    public boolean isSuccessful() {
        return errorId == SUCCESS;
    }

    @Override
    public boolean isFailed() {
        return errorId != SUCCESS;
    }

    @Override
    public String getText() {
        return text;
    }

    @Override
    public String getFile() {
        return declaringClass;
    }

    @Override
    public int getLine() {
        return line;
    }

    @Override
    public String toString() {
        return "JsonConverterError{" +
                "errorId=" + errorId +
                ", text='" + text + '\'' +
                ", caller=" + caller +
                '}';
    }

    @Override
    public int setError(int errorId, String text, String key) {
        this.key = key;
        this.errorId = errorId;

        switch (errorId) {
            case JSON_ERROR_PARSE_ERROR:
                this.text = "Key: " + key + ", " + "JSON Converter parse error: " + text;
                break;
            case JSON_ERROR_UNSUPPORTED_PROTOCOL:
                this.text = "Key: " + key + ", " + "JSON Converter init error. Unsupported protocol: [" + text + "]. At the moment supported version is JSON Simple (2)";
                break;
            case JSON_ERROR_UNKNOWN_PROPERTY:
                this.text = "Key: " + key + ", " + "JSON Converter init error. Invalid property: " + text;
                break;
            case JSON_ERROR_INVALID_TOKEN_TYPE:
                this.text = "Key: " + key + ", " + "JSON Converter Token Type error: " + text;
                break;
            case JSON_ERROR_UNEXPECTED_KEY:
                this.text = "Key: " + key + ", " + "JSON Converter unexpected key. Received " + text;
                break;
            case JSON_ERROR_UNSUPPORTED_MESSAGE:
                this.text = "Key: " + key + ", " + "JSON Converter parsing is not supported: " + text;
                break;
            case JSON_ERROR_UNEXPECTED_VALUE:
                this.text = "Key: " + key + ", " + "JSON Converter unexpected value: " + text;
                break;
            case JSON_ERROR_RSSL_ENCODE_ERROR:
                this.text = "Key: " + key + ", " + "JSON Converter encode error: " + text;
                break;
            case JSON_ERROR_MISSING_KEY:
                this.text = "Key: " + key + ", " + "JSON Converter missing key: " + text;
                break;
            case JSON_ERROR_OPERATION_NOT_SUPPORTED:
                this.text = "Key: " + key + ", " + "Operation not supported for class: " + text;
                break;
            default:
                this.text = "Key: " + key + ", " + "JSON Converter error: " + text;
        }

        caller = callerToString();

        if(errorId != SUCCESS)
            return FAILURE;

        return  errorId;
    }

    @Override
    public void clear() {
        errorId = SUCCESS;
        text = null;
        caller = null;
        key = null;
        line = -1;
    }
}
