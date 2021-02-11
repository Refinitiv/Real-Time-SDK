package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Msg;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.*;

public class JsonConverterTest {

    private static final int MSG_BUFFER_SIZE = 10000;

    Buffer jsonBuffer;
    Msg resultMsg;

    @Before
    public void init()
    {
        jsonBuffer = CodecFactory.createBuffer();
        resultMsg = CodecFactory.createMsg();
        Buffer msgBuffer = CodecFactory.createBuffer();
        msgBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
    }

    @Test
    public void testConverterSupportedVersions()
    {
        JsonConverterError convError = ConverterFactory.createJsonConverterError();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_UNKNOWN)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .build(convError);

        assertNull(converter);
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_PROTOCOL, convError.getCode());

        converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .build(convError);
        assertNotNull(converter);

        converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .build(convError);
        assertNotNull(converter);
    }

}
