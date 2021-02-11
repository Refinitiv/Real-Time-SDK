package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE;
import static org.junit.Assert.assertEquals;

public class JsonNonConvertedMsgTest {

    private static final String PING_JSON = "{\"Type\":\"Ping\"}";

    private static final String PONG_JSON = "{\"Type\":\"Ping\"}";

    private static final String ERROR_JSON = "{\"Id\":2,\"Type\":\"Error\",\"Text\":\"Expected STRING, received PRIMITIVE.\"}";

    Buffer jsonBuffer;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    JsonConverter converter;
    DataDictionary dictionary = CodecFactory.createDataDictionary();

    @Before
    public void init() {
        jsonMsg.clear();
        parseJsonOptions.clear();
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions = new DecodeJsonMsgOptionsImpl();
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        final String dictionaryFileName1 = "src/test/resources/RDMFieldDictionary";
        final String enumTypeFile = "src/test/resources/enumtype.def";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();

        dictionary.loadFieldDictionary(dictionaryFileName1, error);
        dictionary.loadEnumTypeDictionary(enumTypeFile, error);
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[] {JsonConverter.class},
                proxy);
    }

    @Test
    public void testPingMessage() {
        jsonBuffer.data().put(PING_JSON.getBytes());
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }

    @Test
    public void testPongMessage() {
        jsonBuffer.data().put(PONG_JSON.getBytes());
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }

    @Test
    public void testErrorMessage() {
        jsonBuffer.data().put(ERROR_JSON.getBytes());
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }

    @Test
    public void testUnitTestMessage() {
        jsonBuffer.data().put(("{\"ID\":4,\"Type\":\"Refresh\",\"Key\":{\"Service\":1,\"Name\":\"IBM.N\"},\"State\":" +
                "{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"\"},\"Complete\":false,\"ClearCache\":false,\"Solicited\":false,\"Fields\":{\"TRDPRC_1\":60,\"HIGH_1\":120,\"LOW_1\":130}}")
                .getBytes());
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion());
        Msg msg = CodecFactory.createMsg();
        msg.decode(decIter);
        converter.convertRWFToJson(msg, rwfToJsonOptions, convError);
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.wrap(new byte[4096]));
        converter.getJsonBuffer(buf, getJsonMsgOptions, convError);
        for (int i = 0; i < buf.length(); i++) {
            System.out.print((char)buf.data().get(buf.position() + i));
        }
    }


}
