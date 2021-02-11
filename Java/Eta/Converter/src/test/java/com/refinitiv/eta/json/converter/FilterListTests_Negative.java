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

public class FilterListTests_Negative {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "src/test/resources/RDMFieldDictionary";
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
    public void testFilterList_notObject() throws JsonProcessingException {

        String wrongJson = "[]";
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(wrongNode, "", null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testFilterList_unexpectedKey() throws JsonProcessingException {

        String wrongJson = "{\"CountHint\":3,\"Entries\":[{\"ID\":0,\"Action\":\"Clear\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"ID\":1,\"Action\":\"Update\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"ID\":2,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}], \"UnexpectedKey\":\"SomeValue\"}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testFilterList_unexpectedEntriesValue() throws JsonProcessingException {

        String wrongJson = "{\"CountHint\":3,\"Entries\":{\"array\":[{\"ID\":0,\"Action\":\"Clear\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"ID\":1,\"Action\":\"Update\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"ID\":2,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testFilterList_unexpectedEntryKey() throws JsonProcessingException {

        String wrongJson = "{\"CountHint\":3,\"Entries\":[{\"ID\":0,\"Action\":\"Clear\",\"UnexpectedKey\":0,\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"ID\":1,\"Action\":\"Update\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"ID\":2,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testFilterList_unexpectedEntryAction() throws JsonProcessingException {

        String wrongJson = "{\"CountHint\":3,\"Entries\":[{\"ID\":0,\"Action\":\"UnexpectedAction\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"ID\":1,\"Action\":\"Update\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"ID\":2,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testFilterList_unexpectedPermDataType() throws JsonProcessingException {

        String wrongJson = "{\"CountHint\":3,\"Entries\":[{\"ID\":0,\"Action\":\"Clear\",\"PermData\":123}," +
                "{\"ID\":1,\"Action\":\"Update\",\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"ID\":2,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }
}
