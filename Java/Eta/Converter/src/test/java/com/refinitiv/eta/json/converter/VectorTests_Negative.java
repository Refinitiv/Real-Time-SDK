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

public class VectorTests_Negative {

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
    public void testVetcor_notObject() throws JsonProcessingException {

        String wrongJson = "[]";
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testVector_unexpectedKeyInVector() throws JsonProcessingException {

        String wrongJson = "{\"CountHint\":3," +
                "\"SupportSorting\":true, \"UnexpectedKey\":\"UnexpectedValue\"," +
                "\"Entries\":[{\"Index\":0,\"Action\":\"Set\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"," +
                "\"Series\":{\"Summary\":{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\"," +
                "\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\"," +
                "\"TimeInfo\":0,\"RateInfo\":0},{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "\"CountHint\":3,\"Entries\":[{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345},\"datetime\":{\"Type\":\"DateTime\"," +
                "\"Data\":\"2020-11-02T13:55:18.015001003\"},\"array\":{\"Type\":\"Array\"," +
                "\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\"," +
                "\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}," +
                "{\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13},\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}," +
                "\"datetime\":{\"Type\":\"DateTime\",\"Data\":\"2020-11-02T13:55:18.015001003\"}," +
                "\"array\":{\"Type\":\"Array\",\"Data\":{\"Type\":\"Qos\",\"Data\":[{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}," +
                "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":0,\"RateInfo\":0}]}}}}]}}," +
                "{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Clear\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testVector_unexpectedSummaryValue() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":[{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]," +
                "\"CountHint\":3,\"SupportSorting\":true," +
                "\"Entries\":[{\"Index\":0,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testVector_unexpectedSummaryContainerType() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"UnexpectedType\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":3,\"SupportSorting\":true,\"Entries\":[{\"Index\":0,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testVector_unexpectedSupportSortingValue() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":3,\"SupportSorting\":2,\"Entries\":[{\"Index\":0,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testVector_unexpectedCountHintValue() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":\"3\",\"SupportSorting\":true,\"Entries\":[{\"Index\":0,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}," +
                "{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testVector_unexpectedEntriesValue() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":3,\"SupportSorting\":true,\"Entries\":{\"Index\":0,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testVector_unexpectedEntryType() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":3,\"SupportSorting\":true,\"Entries\":[{\"Index\":0,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}, \"Entry\"]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testVector_unexpectedEntryContainerType() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":3,\"SupportSorting\":true,\"Entries\":[{\"Index\":0,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\", " +
                "\"UnexpectedContainer\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testVector_summaryEntryContainerTypeMismatch() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"CountHint\":3,\"SupportSorting\":true,\"Entries\":[{\"Index\":0,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\", " +
                "\"Elements\":{\"int\":{\"Type\":\"Int\",\"Data\":13}," +
                "\"real\":{\"Type\":\"Real\",\"Data\":0.000000012345}}},{\"Index\":1,\"Action\":\"Delete\"},{\"Index\":2,\"Action\":\"Delete\",\"PermData\":\"cGVybWlzc2lvbiBkYXRhIHN0cmluZw==\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, convError.getCode());
    }

    @Test
    public void testVector_decodeVectorFails() {

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        Vector vector = CodecFactory.createVector();
        vector.containerType(DataTypes.ELEMENT_LIST);
        vector.applySupportsSorting();
        vector.applyHasSummaryData();
        vector.encodeInit(iter, 0,0);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer out = new JsonBuffer();
        out.data = new byte[200];
        converter.getContainerHandler(DataTypes.VECTOR).encodeJson(decIter, out, true, null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
    }

}
