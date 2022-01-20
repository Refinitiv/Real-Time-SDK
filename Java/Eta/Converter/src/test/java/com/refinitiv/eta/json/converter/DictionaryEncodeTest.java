package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;

public class DictionaryEncodeTest {

    private Int tmpInt = CodecFactory.createInt();
    private DataDictionary dictionary = CodecFactory.createDataDictionary();
    com.refinitiv.eta.transport.Error error;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();

    JsonConverterError convError;
    JsonAbstractConverter converter;
    final String fieldDictionaryFile = "../../etc/RDMFieldDictionary";
    final String enumTypeFile = "../../etc/enumtype.def";

    @Before
    public void init() {

        error = TransportFactory.createError();
        dictionary.clear();

        convError = ConverterFactory.createJsonConverterError();
        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, true)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
                .build(convError);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
    }

    @Test
    public void testEnumDictionaryEncoding() {

        Buffer buffer = prepareDictionaryRefreshMsg(true);
        Buffer result = CodecFactory.createBuffer();
        result.data(ByteBuffer.wrap(new byte[4096]));

        Msg decodeMsg = CodecFactory.createMsg();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        decodeMsg.decode(decIter);

        assertEquals(CodecReturnCodes.SUCCESS, converter.convertRWFToJson(decodeMsg, rwfToJsonOptions, convError));
        assertEquals(CodecReturnCodes.SUCCESS, converter.getJsonBuffer(result, getJsonMsgOptions, convError));

        assertEquals(CodecReturnCodes.SUCCESS, converter.parseJsonBuffer(result, parseJsonOptions, convError));
        assertEquals(CodecReturnCodes.SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }

    @Test
    public void testRDMDictionaryEncoding() {

        Buffer buffer = prepareDictionaryRefreshMsg(false);
        Buffer result = CodecFactory.createBuffer();
        result.data(ByteBuffer.wrap(new byte[4096]));

        Msg decodeMsg = CodecFactory.createMsg();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        decodeMsg.decode(decIter);

        assertEquals(CodecReturnCodes.SUCCESS, converter.convertRWFToJson(decodeMsg, rwfToJsonOptions, convError));
        assertEquals(CodecReturnCodes.SUCCESS, converter.getJsonBuffer(result, getJsonMsgOptions, convError));

        assertEquals(CodecReturnCodes.SUCCESS, converter.parseJsonBuffer(result, parseJsonOptions, convError));
        assertEquals(CodecReturnCodes.SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }

    private Buffer prepareDictionaryRefreshMsg(boolean loadEnumType) {

        RefreshMsg msg = (RefreshMsg) CodecFactory.createMsg();
        msg.clear();
        msg.msgClass(MsgClasses.REFRESH);
        msg.domainType(DomainTypes.DICTIONARY);
        msg.streamId(1);
        msg.containerType(DataTypes.SERIES);
        msg.applyHasMsgKey();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasFilter();
        msg.msgKey().applyHasServiceId();
        msg.msgKey().name(ElementNames.ADDRESS);
        msg.msgKey().filter(10);
        msg.msgKey().applyHasServiceId();
        msg.msgKey().serviceId(5);
        msg.state().streamState(StreamStates.OPEN);
        msg.state().dataState(DataStates.OK);
        msg.state().code(StateCodes.NONE);
        msg.state().text().data("Dictionary Refresh Completed");


        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(100000));
        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        msg.encodeInit(encodeIter, 0);

        tmpInt.value(19);
        if (loadEnumType) {
            dictionary.loadEnumTypeDictionary(enumTypeFile, error);
            dictionary.encodeEnumTypeDictionaryAsMultiPart(encodeIter, tmpInt, 10, error);
        } else {
            dictionary.loadFieldDictionary(fieldDictionaryFile, error);
            dictionary.encodeFieldDictionary(encodeIter, tmpInt, 10, error);
        }

        msg.encodeComplete(encodeIter, true);

        return buffer;
    }
}
