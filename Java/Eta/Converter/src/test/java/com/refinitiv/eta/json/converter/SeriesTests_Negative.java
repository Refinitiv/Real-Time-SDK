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

public class SeriesTests_Negative {

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
    public void testSeries_notObject() throws JsonProcessingException {

        String wrongJson = "[]";
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.SERIES).encodeRWF(wrongNode, "", null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testSeries_unexpectedSummaryValue() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":[{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]," +
                "\"Entries\":[{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.SERIES).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testSeries_unexpectedSummaryContainerType() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"UnexpectedContainer\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"Entries\":[{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "{\"Fields\":{\"CASH_BASIS\":\"ascii string\",\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.SERIES).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, convError.getCode());
    }

    @Test
    public void testSeries_unexpectedEntriesValue() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Xml\":\"Xml data\"},\"Entries\":{\"array\":[{\"Xml\":\"Xml data\"},{\"Xml\":\"Xml data\"},{\"Xml\":\"Xml data\"}]}}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.SERIES).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testSeries_summaryEntryContainerTypeMismatch() throws JsonProcessingException {

        String wrongJson = "{\"Summary\":{\"Fields\":{\"CASH_BASIS\":\"ascii string\"," +
                "\"TRDPRC_2\":0.000000012345,\"TRADE_DATE\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}}," +
                "\"Entries\":[{\"Xml\":\"Xml data\"},{\"Xml\":\"Xml data\"},{\"Xml\":\"Xml data\"}]}";

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.SERIES).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, convError.getCode());
    }

    @Test
    public void testSeries_decodeSeriesFails() {

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        Series series = CodecFactory.createSeries();
        series.containerType(DataTypes.ELEMENT_LIST);
        series.applyHasSummaryData();
        series.encodeInit(iter, 0,0);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer out = new JsonBuffer();
        out.data = new byte[200];
        converter.getContainerHandler(DataTypes.SERIES).encodeJson(decIter, out, true, null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
    }

}
