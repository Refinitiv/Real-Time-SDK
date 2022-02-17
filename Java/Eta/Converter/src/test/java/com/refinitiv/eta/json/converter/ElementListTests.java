package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_DATA;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_TYPE;
import static org.junit.Assert.*;

@RunWith(Parameterized.class)
public class ElementListTests {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    public class MessageParameters
    {
        public int id;
        public int[] dataTypes;
    }
    ElementListTests.MessageParameters msgParameters = new ElementListTests.MessageParameters();

    public ElementListTests(
            int id,
            int[] dataTypes
    ) {
        msgParameters.id = id;
        msgParameters.dataTypes = dataTypes;
    }

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
    public void testRwfToJson_Sunny() throws IOException {
        DecodeIterator decIter = generateElementListDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[4000];
        assertEquals(true, converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);
        checkJsonNode(root);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(3000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer);
    }

    private void checkJsonNode(JsonNode root) {

        for (int i = 0; i < msgParameters.dataTypes.length; i++) {
            switch (msgParameters.dataTypes[i]) {
                case DataTypes.STATE:
                    JsonNode stateNode = root.path(CodecUtil.stateName.toString());
                    assertFalse(stateNode.isMissingNode());
                    assertTrue(stateNode.isObject());
                    assertFalse(stateNode.path(JSON_TYPE).isMissingNode());
                    assertFalse(stateNode.path(JSON_DATA).isMissingNode());
                    break;
                case DataTypes.QOS:
                    JsonNode qosNode = root.path(CodecUtil.qosName.toString());
                    assertFalse(qosNode.isMissingNode());
                    assertTrue(qosNode.isObject());
                    assertFalse(qosNode.path(JSON_TYPE).isMissingNode());
                    assertFalse(qosNode.path(JSON_DATA).isMissingNode());
                    break;
                case DataTypes.DOUBLE:
                    JsonNode doubleNode = root.path(CodecUtil.doubleName.toString());
                    assertFalse(doubleNode.isMissingNode());
                    assertTrue(doubleNode.isObject());
                    assertFalse(doubleNode.path(JSON_TYPE).isMissingNode());
                    assertFalse(doubleNode.path(JSON_DATA).isMissingNode());
                    break;
                case DataTypes.FLOAT:
                    JsonNode floatNode = root.path(CodecUtil.floatName.toString());
                    assertFalse(floatNode.isMissingNode());
                    assertFalse(floatNode.path(JSON_TYPE).isMissingNode());
                    assertFalse(floatNode.path(JSON_DATA).isMissingNode());
                    break;
                case DataTypes.INT:
                    JsonNode intNode = root.path(CodecUtil.intName.toString());
                    assertFalse(intNode.isMissingNode());
                    assertTrue(intNode.isObject());
                    assertFalse(intNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.UINT:
                    JsonNode uintNode = root.path(CodecUtil.uintName.toString());
                    assertFalse(uintNode.isMissingNode());
                    assertTrue(uintNode.isValueNode());
                    break;
                case DataTypes.REAL:
                    JsonNode realNode = root.path(CodecUtil.realName.toString());
                    assertFalse(realNode.isMissingNode());
                    assertTrue(realNode.isObject());
                    assertFalse(realNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.ASCII_STRING:
                    JsonNode asciiNode = root.path(CodecUtil.asciiName.toString());
                    assertFalse(asciiNode.isMissingNode());
                    assertTrue(asciiNode.isValueNode());
                    break;
                case DataTypes.DATE:
                    JsonNode dateNode = root.path(CodecUtil.dateName.toString());
                    assertFalse(dateNode.isMissingNode());
                    assertTrue(dateNode.isObject());
                    assertFalse(dateNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.TIME:
                    JsonNode timeNode = root.path(CodecUtil.timeName.toString());
                    assertFalse(timeNode.isMissingNode());
                    assertTrue(timeNode.isObject());
                    assertFalse(timeNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.DATETIME:
                    JsonNode dateTimeNode = root.path(CodecUtil.dateTimeName.toString());
                    assertFalse(dateTimeNode.isMissingNode());
                    assertTrue(dateTimeNode.isObject());
                    assertFalse(dateTimeNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.ELEMENT_LIST:
                    JsonNode elListNode = root.path(CodecUtil.elListName.toString());
                    assertFalse(elListNode.isMissingNode());
                    assertTrue(elListNode.isObject());
                    assertFalse(elListNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.VECTOR:
                    JsonNode vectorNode = root.path(CodecUtil.vectorName.toString());
                    assertFalse(vectorNode.isMissingNode());
                    assertTrue(vectorNode.isObject());
                    assertFalse(vectorNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.SERIES:
                    JsonNode seriesNode = root.path(CodecUtil.seriesName.toString());
                    assertFalse(seriesNode.isMissingNode());
                    assertTrue(seriesNode.isObject());
                    assertFalse(seriesNode.path(JSON_TYPE).isMissingNode());
                    break;
                case DataTypes.MAP:
                    JsonNode mapNode = root.path(CodecUtil.mapName.toString());
                    assertFalse(mapNode.isMissingNode());
                    assertTrue(mapNode.isObject());
                    assertFalse(mapNode.path(JSON_TYPE).isMissingNode());
                    break;
                default:
                    break;
            }
        }

    }

    private void checkRWFData(Buffer buffer) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.decodeAndCheckElementList(decIter, msgParameters.dataTypes);
    }

    private DecodeIterator generateElementListDecodeIterator() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(1024));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.encodeSimpleElementList(encIter, msgParameters.dataTypes);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        return decIter;
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][] {
                {0, new int[] {DataTypes.ARRAY, DataTypes.INT, DataTypes.UINT} },
                {1, new int[] {DataTypes.MAP, DataTypes.INT, DataTypes.UINT} },
                {2, new int[] {DataTypes.DATE, DataTypes.DATETIME, DataTypes.FIELD_LIST, DataTypes.INT, DataTypes.UINT} },
                {3, new int[] {DataTypes.MAP, DataTypes.INT, DataTypes.ASCII_STRING} },
                {4, new int[] {DataTypes.UINT, DataTypes.INT, DataTypes.ASCII_STRING,
                        DataTypes.DATE, DataTypes.DATETIME, DataTypes.TIME, DataTypes.QOS, DataTypes.STATE,
                        DataTypes.REAL, DataTypes.DOUBLE, DataTypes.FLOAT} },
                {5, new int[] {DataTypes.FILTER_LIST} },
                {6, new int[] {DataTypes.VECTOR } },
                {7, new int[] {DataTypes.SERIES} },
                {8, new int[] {DataTypes.ELEMENT_LIST} },
                {9, new int[] {DataTypes.OPAQUE} },
                {10, new int[] {DataTypes.XML} },
                {11, new int[] {DataTypes.JSON} },
                {12, new int[] {DataTypes.MSG} },
                {13, new int[] {DataTypes.ANSI_PAGE} }
        });
    }
}
