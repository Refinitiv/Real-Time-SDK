package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class MapTests_Negative {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        convError = ConverterFactory.createJsonConverterError();

        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setDictionary(dictionary)
                .build(convError);
    }

    @Test
    public void testMap_notObject() throws JsonProcessingException {

        String wrongJson = "[]";
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }


    @Test
    public void testMap_unexpectedKey() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Int\",\"UnexpectedKey\":1,\"Entries\":[{\"Action\":\"Delete\",\"Key\":13,\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":13," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}\n";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testMap_unexpectedSummaryValue() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\"," +
                "\"Summary\":[{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]," +
                "\"CountHint\":3,\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testMap_unexpectedCountHintValue() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\"," +
                "\"Summary\":[{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]," +
                "\"CountHint\":\"3\",\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testMap_unexpectedSummaryType() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\"," +
                "\"Summary\":{\"UnexpectedKey\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testMap_unsupportedKeyType() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Unsupported\"," +
                "\"Summary\":{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_DATA_TYPE, convError.getCode());
    }

    @Test
    public void testMap_unexpectedAction() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\"," +
                "\"Summary\":{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"UnexpectedAction\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testMap_unexpectedTokenInEntry() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Int\",\"KeyFieldID\":1,\"Entries\":[{\"Action\":\"Delete\",\"Key\":13,\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Unexpected Key\":\"Unexpected Value\"},{\"Action\":\"Add\",\"Key\":13,\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testMap_summaryAndEntryContainerTypesDiffer() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\",\"Summary\":{\"Series\":" +
                "{\"Summary\":{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"Entries\":[{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}}," +
                "\"CountHint\":3,\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, convError.getCode());
    }

    @Test
    public void testMap_differentEntryContainerTypes() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\",\"CountHint\":3,\"KeyFieldID\":1,\"Entries\":" +
                "[{\"Action\":\"Delete\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\",\"Series\":{\"Summary\":{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"Entries\":[{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, convError.getCode());
    }

    @Test
    public void testMap_noEntryKeyInMapEntry() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Float\",\"Summary\":" +
                "{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"KeyFieldID\":1,\"Entries\":[{\"Action\":\"Add\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "{\"Action\":\"Add\",\"Key\":1.234,\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
    }

    @Test
    public void testMap_noActionInMapEntry() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Float\",\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"KeyFieldID\":1,\"Entries\":[{\"Action\":\"Add\",\"Key\":1.234,\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "{\"Key\":1.234,\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
    }

    @Test
    public void testMap_keyTypeNotPrimitive() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Map\"," +
                "\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345," +
                "\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}},\"KeyFieldID\":1," +
                "\"Entries\":[{\"Action\":\"Add\",\"Key\":1.234,\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345," +
                "\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}},{\"Action\":\"Add\",\"Key\":1.234," +
                "\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_INVALID_PRIMITIVE_TYPE, convError.getCode());
    }

    @Test
    public void testMap_decodeMapFails() {

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        Map map = CodecFactory.createMap();
        map.containerType(DataTypes.ELEMENT_LIST);
        map.keyPrimitiveType(DataTypes.INT);
        map.applyHasSummaryData();
        map.encodeInit(iter, 0,0);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer out = new JsonBuffer();
        out.data = new byte[200];
        converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, out, true, null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
    }

    @Test
    public void testMap_noEntryAction() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"Time\"," +
                "\"Summary\":{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"KeyFieldID\":1," +
                "\"Entries\":[{\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Action\":\"Add\",\"Key\":\"00:00:02.0000025\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Action\":\"Update\",\"Key\":\"00:00:02.0000025\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
    }

    @Test
    public void testMap_encodeRWFEmptyEntryContainer_shouldNotCrash() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"UInt\",\"CountHint\":3,\"Entries\":" +
                "[{\"Action\":\"Add\",\"Key\":13,\"Fields\":{}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isSuccessful());
    }

    @Test
    public void testMap_encodeRWFBlankContainer_shouldNotCrash() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"UInt\",\"CountHint\":3,\"Entries\":[{\"Action\":\"Add\",\"Key\":13}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isSuccessful());
    }

    @Test
    public void testMap_encodeRWFEmptyEntries_shouldNotCrash() throws JsonProcessingException {

        String wrongJson = "{\"KeyType\":\"UInt\",\"CountHint\":3,\"Entries\":[]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.MAP).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isSuccessful());
    }
}
