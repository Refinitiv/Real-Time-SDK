package com.refinitiv.eta.json.converter;

public class JsonConverterErrorCodes {

    public final static int JSON_ERROR_NO_ERROR_CODE = 0; // No error code was set - default
    public final static int JSON_ERROR_MEM_ALLOC_FAILURE = 1; // Unable to allocate memory
    public final static int JSON_ERROR_PARSE_ERROR = 2; // Error was detected in parser
    public final static int JSON_ERROR_INVALID_TOKEN_TYPE = 3; // An unexpected token type was encountered in json message
    public final static int JSON_ERROR_UNEXPECTED_VALUE = 4; // An unexpected value was encountered in a token
    public final static int JSON_ERROR_INVALID_PRIMITIVE_TYPE = 5;
    public final static int JSON_ERROR_INVALID_CONTAINER_TYPE = 6;
    public final static int JSON_ERROR_SET_DEFINITION_ERROR = 7;
    public final static int JSON_ERROR_RSSL_ENCODE_ERROR = 8; // see RSSL return code
    public final static int JSON_ERROR_NO_MSG_BASE = 9;
    public final static int JSON_ERROR_UNSUPPORTED_MSG_TYPE = 10;
    public final static int JSON_ERROR_UNSUPPORTED_DATA_TYPE = 11;
    public final static int JSON_ERROR_MISSING_KEY = 12; // Missing Required Key
    public final static int JSON_ERROR_TYPE_MISMATCH = 13; // Array Type Mismatch
    public final static int JSON_ERROR_UNEXPECTED_KEY = 14; // Unexpected Key
    public final static int JSON_ERROR_UNEXPECTED_FID = 15; // Unexpected FID
    public final static int JSON_ERROR_RSSL_DICT_NOT_INIT = 16; // RsslDataDictionary is not initialized.

    public static final int JSON_ERROR_UNSUPPORTED_PROTOCOL = 20;
    public static final int JSON_ERROR_UNKNOWN_PROPERTY = 21;
    public final static int JSON_ERROR_UNSUPPORTED_MESSAGE = 50;

    public final static int JSON_ERROR_DECODING_FAILED = 15;
    public final static int JSON_ERROR_OUT_OF_MEMORY = 16;
    public final static int JSON_ERROR = 17;

    public final static int JSON_ERROR_OPERATION_NOT_SUPPORTED = 100;
}
