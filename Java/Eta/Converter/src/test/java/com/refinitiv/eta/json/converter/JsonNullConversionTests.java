/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.nio.ByteBuffer;

import static org.junit.Assert.*;

public class JsonNullConversionTests {

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
    public void nullConversionElementList() throws IOException {

        String nullTypes = "{\"uint\":7,\"int\":{\"Type\":\"Int\",\"Data\":null},\"ascii\":\"hh\"," +
                "\"date\":{\"Type\":\"Date\",\"Data\":null},\"datetime\":{\"Type\":\"DateTime\",\"Data\":null}," +
                "\"time\":{\"Type\":\"Time\",\"Data\":null},\"qos\":{\"Type\":\"Qos\",\"Data\":null}," +
                "\"state\":{\"Type\":\"State\",\"Data\":null},\"real\":{\"Type\":\"Real\",\"Data\":null}," +
                "\"double\":{\"Type\":\"Double\",\"Data\":null},\"float\":{\"Type\":\"Float\",\"Data\":null}}\n";
        JsonNode node = mapper.readTree(nullTypes);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(5000));
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeRWF(node, "", encIter, convError);
        assertTrue(convError.isSuccessful());
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        JsonNode result = mapper.readTree(outBuffer.data, 0, outBuffer.position);
        JsonNode child = result.path("int");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("date");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("datetime");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("time");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("qos");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("state");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("real");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("double");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("float");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
    }

    @Test
    public void nullConversionFieldList() throws IOException {

        String nullTypes = "{\"PROD_PERM\":null,\"TRDPRC_2\":null,\"TENORS\":{\"Type\":\"Qos\",\"Data\":null},\"TRADE_DATE\":null,\"TRDTIM_1\":null}\n";
        JsonNode node = mapper.readTree(nullTypes);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(5000));
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(node, "", encIter, convError);
        assertTrue(convError.isSuccessful());
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        JsonNode result = mapper.readTree(outBuffer.data, 0, outBuffer.position);
        JsonNode child = result.path("PROD_PERM");
        assertFalse(child.isMissingNode());
        assertTrue(child.isNull());
        child = result.path("TRDPRC_2");
        assertFalse(child.isMissingNode());
        assertTrue(child.isNull());
        child = result.path("TRADE_DATE");
        assertFalse(child.isMissingNode());
        assertTrue(child.isNull());
        child = result.path("TRDTIM_1");
        assertFalse(child.isMissingNode());
        assertTrue(child.isNull());
    }

    @Test
    public void nullConversionFieldList_nullFields() throws IOException {

        String[] elements = new String[] {"BOND_RATING", "TRDPRC_2", "TRADE_DATE", "TRDTIM_1", "AFTMKT_DT", "AFTMKT_NS", "AFTMKT_PRC", "AFTMKT_VOL",
                "52W_LIND", "52W_HIND",  "AC_TRN_CRS", "ASK_MMID1", "AUC_EX_NO", "BCASTREF32", "BCKGRNDPAG", "BC_10_50K", "BC_50_100K", "BLKVOLUM", "BLK_DATE",
                "BLK_FLAG", "BLK_PRC1", "BLK_SEQNO", "BLK_TRDID", "BOND_TYPE", "BOOK_STATE", "CANCEL_IND", "CAN_COND", "CAN_COND_N", "CAN_DATE", "CAN_PRC", "CAN_SUBIND",
                "CAN_TDTH_X", "CAN_TERMS", "CAN_TRD_ID", "CAN_VOL", "CLOSE_TIME", "CNTX_VER_N", "CRSSALCOND", "CRSTIM_MS", "CRSTRD_PRC", "CRS_DATE", "CRS_NUMOV", "CRS_SEQNO", "CRS_TRDID", "CRS_TRDVOL",
                "CTRDTIM", "CTRDTIM", "CTRDTIM_MS", "DJTIME", "DM_TYPE", "IMB_ACT_TP", "IMB_SH", "IMB_SIDE", "IMB_TIM_MS", "IMP_VOLT",
                "INSCOND", "INSPRC", "INSSALCOND", "INSTIM_MS", "INSTRD_DT", "INSTRD_TIM",
                "INSVOL", "INS_SEQNO", "INS_SUBIND", "INS_TDTH_X", "INS_TRDID", "IPO_PRC", "LMT_REFPR2", "LMT_TYPE", "LMT_TYPE2", "LOLIMIT_2", "LULD_T2_MS", "MK_STATUS",
                "OFF_CD_IN2", "OFF_CLOSE", "OFF_CLS_DT", "OFF_CLS_MS", "OFF_CL_TIM", "ORDREC2_MS", "PDACVOL", "PDTRDDATE", "PDTRDPRC",
                "PDTRDTM_MS",  "PD_SALCOND", "PD_SEQNO", "PD_SUBIND", "PD_TDTH_X", "PD_TRDID", "PERIOD_CD2",
                "PREDAYVOL", "PREMKT_DT", "PREMKT_NS", "PREMKT_PRC", "PREMKT_VOL", "RCS_AS_CL2", "REG_PILOT", "SEE_RIC",
                "TIMCOR", "TIMCOR_MS", "TRDREC2_MS", "TURNOVER", "TURN_BLOCK", "UPLIMIT_2", "VWAP_FLAG", "YCHIGH_IND", "YCLOW_IND"
        };
        StringBuilder json = new StringBuilder();
        json.append("{");
        for (int i = 0; i < elements.length - 1; i++) {
            json.append("\"");
            json.append(elements[i]);
            json.append("\":");
            json.append("null,");
        }
        json.append("\"");
        json.append(elements[elements.length - 1]);
        json.append("\":");
        json.append("null}");

        JsonNode node = mapper.readTree(json.toString());
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(5000));
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(node, "", encIter, convError);
        assertTrue(convError.isSuccessful());
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        JsonNode result = mapper.readTree(outBuffer.data, 0, outBuffer.position);
        for (int i = 0; i < elements.length; i++) {
            JsonNode child = result.path(elements[i]);
            assertTrue(child.isNull() || child.isMissingNode());
        }
    }

    @Test
    public void nullConversionElementList_withEnum() throws IOException {

        String nullTypes = "{\"uint\":7,\"int\":{\"Type\":\"Int\",\"Data\":null},\"ascii\":\"hh\"," +
                "\"date\":{\"Type\":\"Date\",\"Data\":null},\"datetime\":{\"Type\":\"DateTime\",\"Data\":null}," +
                "\"time\":{\"Type\":\"Time\",\"Data\":null},\"qos\":{\"Type\":\"Qos\",\"Data\":null}," +
                "\"state\":{\"Type\":\"State\",\"Data\":null},\"real\":{\"Type\":\"Real\",\"Data\":null}," +
                "\"double\":{\"Type\":\"Double\",\"Data\":null},\"BOND RATING\":{\"Type\":\"Enum\",\"Data\":null}," +
                "\"rmtes\":{\"Type\" : \"RmtesString\",\"Data\" : null}," +
                "\"Utf8String\" : {\"Type\" : \"Utf8String\", \"Data\" : null}," +
                "\"buffer\" : {\"Type\" : \"Buffer\", \"Data\" : null}," +
                "\"real\" : {\"Type\" : \"Real\", \"Data\" : null}}\n";
        JsonNode node = mapper.readTree(nullTypes);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(5000));
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeRWF(node, "", encIter, convError);
        assertTrue(convError.isSuccessful());
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        JsonNode result = mapper.readTree(outBuffer.data, 0, outBuffer.position);
        JsonNode child = result.path("int");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("date");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("datetime");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("time");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("qos");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("state");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("real");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("double");
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("BOND RATING");       //Enum type
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("rmtes");            //Rmtes string
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("Utf8String");       //Utf8String
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("buffer");       //Buffer
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
        child = result.path("real");       //real
        assertFalse(child.isMissingNode());
        assertTrue(child.path("Data").isNull());
    }
}
