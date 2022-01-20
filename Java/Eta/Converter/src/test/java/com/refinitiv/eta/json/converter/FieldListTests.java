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
import java.util.HashSet;
import java.util.Set;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_DATA;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_TYPE;
import static org.junit.Assert.*;

@RunWith(Parameterized.class)
public class FieldListTests {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    public class MessageParameters
    {
        public int id;
        public int[] dataTypes;
    }
    FieldListTests.MessageParameters msgParameters = new FieldListTests.MessageParameters();

    public FieldListTests(
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
        DecodeIterator decIter = generateFieldListDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);
        checkJsonNode(root);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer);
    }

    private void checkJsonNode(JsonNode root) {

        for (int i = 0; i < msgParameters.dataTypes.length; i++) {
            JsonNode node = root.path(CodecUtil.dataTypeNameMap.get(msgParameters.dataTypes[i]));
            assertFalse(node.isMissingNode());
            switch (msgParameters.dataTypes[i]) {
                case DataTypes.ARRAY:
                    assertTrue(node.isObject());
                    assertFalse(node.path(JSON_TYPE).isMissingNode());
                    assertFalse(node.path(JSON_DATA).isMissingNode());
                    assertTrue(node.path(JSON_DATA).isArray());
                    break;
                case DataTypes.INT:
                    assertTrue(node.isInt());
                    break;
                case DataTypes.DATE:
                case DataTypes.TIME:
                case DataTypes.ASCII_STRING:
                    assertTrue(node.isTextual());
                    break;
                case DataTypes.ELEMENT_LIST:
                    assertTrue(node.isObject());
                    for (int j = 0; j < CodecUtil.defaultElementListTypes.length; j++)
                        assertFalse(node.path(CodecUtil.elDataTypeNameMap.get(CodecUtil.defaultElementListTypes[j])).isMissingNode());
                    break;
                case DataTypes.VECTOR:
                    assertTrue(node.isObject());
                    break;
                default:
                    break;
            }
        }
    }

    private void checkRWFData(Buffer buffer) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Set<Integer> dataPresent = new HashSet<>();

        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        int ret = fieldList.decode(decIter, null);
        assertEquals(SUCCESS, ret);
        ret = fieldEntry.decode(decIter);
        while (ret != CodecReturnCodes.END_OF_CONTAINER) {
            dataPresent.add(fieldEntry.fieldId());
            switch (fieldEntry.fieldId()) {
                case CodecUtil.arrayFid:
                    Array arr = CodecFactory.createArray();
                    ArrayEntry arrEntry = CodecFactory.createArrayEntry();
                    assertEquals(SUCCESS, arr.decode(decIter));
                    assertEquals(arr.primitiveType(), CodecUtil.defaultArrayDataType);
                    Qos qos = CodecFactory.createQos();
                    while (arrEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
                        qos.clear();
                        qos.decode(decIter);
                        assertTrue(qos.equals(CodecUtil.qos));
                    }
                    break;
                case CodecUtil.intFid:
                    Int intv = CodecFactory.createInt();
                    assertEquals(SUCCESS, intv.decode(decIter));
                    assertTrue(intv.equals(CodecUtil.iv));
                    break;
                case CodecUtil.realFid:
                    Real real = CodecFactory.createReal();
                    assertEquals(SUCCESS, real.decode(decIter));
                    break;
                case CodecUtil.timeFid:
                    Time time = CodecFactory.createTime();
                    assertEquals(SUCCESS, time.decode(decIter));
                    assertTrue(time.equals(CodecUtil.time));
                    break;
                case CodecUtil.dateFid:
                    Date date = CodecFactory.createDate();
                    assertEquals(SUCCESS, date.decode(decIter));
                    assertTrue(date.equals(CodecUtil.date));
                    break;
                case CodecUtil.asciiFid:
                    Buffer ascii = CodecFactory.createBuffer();
                    assertEquals(SUCCESS, ascii.decode(decIter));
                    assertTrue(ascii.equals(CodecUtil.ascii));
                    break;
                case CodecUtil.elemListFid:
                    CodecUtil.decodeAndCheckDefaultElementList(decIter);
                    break;
                case CodecUtil.vectorFid:
                    CodecUtil.decodeAndCheckDefaultVector(decIter);
                    break;
                case CodecUtil.mapFid:
                    CodecUtil.decodeAndCheckDefaultMap(decIter);
                    break;
                default:
                    break;
            }

            fieldEntry.clear();
            ret = fieldEntry.decode(decIter);
        }

        for (int i = 0; i < msgParameters.dataTypes.length; i++)
            assertTrue(dataPresent.contains(CodecUtil.elDataTypeFidMap.get(msgParameters.dataTypes[i])));
        assertEquals(dataPresent.size(), msgParameters.dataTypes.length);
    }

    private DecodeIterator generateFieldListDecodeIterator() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.encodeSimpleFieldList(encIter, msgParameters.dataTypes);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        return decIter;
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][] {
                {0, new int[]{DataTypes.INT, DataTypes.REAL, DataTypes.ARRAY, DataTypes.DATE, DataTypes.TIME, DataTypes.ASCII_STRING} },
                {1, new int[]{DataTypes.ASCII_STRING, DataTypes.DATE, DataTypes.TIME} },
                {2, new int[]{DataTypes.INT, DataTypes.REAL, DataTypes.ARRAY } },
                {3, new int[]{ DataTypes.DATE, DataTypes.MAP, DataTypes.ELEMENT_LIST } },
                {4, new int[]{ DataTypes.DATE, DataTypes.VECTOR, DataTypes.ELEMENT_LIST } }
        });
    }
}

