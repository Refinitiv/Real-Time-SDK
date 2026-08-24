package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.lang.Double;
import java.nio.ByteBuffer;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class FieldListMiscTests {

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

    @After
    public void tearDown() {
        convError = null;
        converter = null;
    }

    @Test
    public void testRealDecoding() throws IOException {

        java.lang.Double[] doubleValues = {
                76900000000000.00,
                769000000000.00,
                7690000000000000.00,
                10000000000.0,
                0.1234567,
                1.5e9,
                1.76e12,
                1.76e13,
                9223372036854775807.00,
                -9223372036854775808.0,
                922337.2036854775807,
                92233720368547.75807,
                -9223372036854775.808,
                1.5e-9
        };

        int[] hints = {
                RealHints.EXPONENT_2,
                RealHints.EXPONENT_2,
                RealHints.EXPONENT_2,
                RealHints.EXPONENT_1,
                RealHints.EXPONENT_7,
                RealHints.EXPONENT0,
                RealHints.EXPONENT6,
                RealHints.EXPONENT_2,
                RealHints.EXPONENT0,
                RealHints.EXPONENT0,
                RealHints.EXPONENT_13,
                RealHints.EXPONENT_5,
                RealHints.EXPONENT_3,
                RealHints.EXPONENT_10
        };

        int[] fids = { 32742, 32741, 32693, 32681, 32682, 32683, 32684, 32685, 32686, 32652, 32653, 32654, 32655, 32656 }; // 32657, 32658, 32659, 32660, 32661, 32662, 32663

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(70000));
        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Real real = CodecFactory.createReal();

        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        fieldList.applyHasStandardData();
        fieldList.encodeInit(encodeIter, null, 0);

        for (int i = 0; i < doubleValues.length; i++)
        {
            fieldEntry.clear();
            fieldEntry.dataType(DataTypes.REAL);
            fieldEntry.fieldId(fids[i]);

            real.clear();
            real.value(doubleValues[i], hints[i]);
            fieldEntry.encode(encodeIter, real);
        }

        fieldList.encodeComplete(encodeIter, true);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(70000));
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());

        DecodeIterator decDecIter = CodecFactory.createDecodeIterator();
        decDecIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        FieldList decFieldList = CodecFactory.createFieldList();
        FieldEntry decFieldEntry = CodecFactory.createFieldEntry();

        int ret = decFieldList.decode(decDecIter, null);
        assertEquals(SUCCESS, ret);

        for (int i = 0; i < doubleValues.length; i++)
        {
            ret = decFieldEntry.decode(decDecIter);
            real.clear();
            real.decode(decDecIter);
            assertEquals(SUCCESS, ret);
            assertTrue(doubleValues[i] == real.toDouble());
        }
    }

    @Test
    public void fieldsInsideBounds() throws JsonProcessingException {
        String json = "{\"RDN_EXCHID\":100,\"PRCTCK_1\":65535,\"TRD_UNITS\":0}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode node = mapper.readTree(json);
        AbstractContainerTypeConverter containerHandler = converter.getContainerHandler(DataTypes.FIELD_LIST);
        containerHandler.encodeRWF(node, "", iter, convError);
        assertTrue(convError.isSuccessful());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_NO_ERROR_CODE, convError.getCode());
    }
}
