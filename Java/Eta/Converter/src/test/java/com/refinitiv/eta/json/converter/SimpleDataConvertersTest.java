package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.codec.Enum;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class SimpleDataConvertersTest {

    private JsonConverterError convError;
    private JsonAbstractConverter converter;

    @Before
    public void init() {
        convError = ConverterFactory.createJsonConverterError();

        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .build(convError);
    }

    @Test
    public void testArrayConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();

        UInt uInt  = CodecFactory.createUInt();
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));

        Array array = CodecFactory.createArray();
        ArrayEntry arrEntry = CodecFactory.createArrayEntry();

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        array.primitiveType(DataTypes.UINT);
        array.itemLength(0);

        array.encodeInit(enc);

        uInt.value(1);
        arrEntry.encode(enc, uInt);
        uInt.value(2);
        arrEntry.encode(enc, uInt);
        uInt.value(3);
        arrEntry.encode(enc, uInt);
        uInt.value(-1);
        arrEntry.encode(enc, uInt);
        uInt.value(-2);
        arrEntry.encode(enc, uInt);

        array.encodeComplete(enc, true);

        JsonBuffer buffer = new JsonBuffer();
        buffer.position = 5;
        buffer.data = new byte[200];

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        assertTrue(converter.getPrimitiveHandler(DataTypes.ARRAY).encodeJson(dec, buffer, convError));
        String correct = "{\"Type\":\"UInt\",\"Data\":[1,2,3,-1,-2]}";
        for (int i = 0; i < correct.length();  i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 5]);
        }

        Time time = CodecFactory.createTime();
        array.clear();
        arrEntry.clear();

        dec.clear();
        enc.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(200));

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        array.primitiveType(DataTypes.TIME);
        array.itemLength(0);

        array.encodeInit(enc);
        time.hour(11);
        time.minute(59);
        time.second(1);
        time.millisecond(14);
        arrEntry.encode(enc, time);
        time.clear();
        time.hour(4);
        time.minute(10);
        time.second(12);
        time.millisecond(155);
        arrEntry.encode(enc, time);
        array.encodeComplete(enc, true);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        buffer.position = 5;
        buffer.data = new byte[200];

        assertTrue(converter.getPrimitiveHandler(DataTypes.ARRAY).encodeJson(dec, buffer, convError));
        correct = "{\"Type\":\"Time\",\"Data\":[\"11:59:01.014\",\"04:10:12.155\"]}";
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 5]);
        }
    }

    @Test
    public void testQosConverter() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));

        JsonBuffer buffer = new JsonBuffer();
        buffer.position = 3;
        buffer.data = new byte[200];

        Qos qos = CodecFactory.createQos();
        qos.timeliness(QosTimeliness.REALTIME);
        qos.rate(QosRates.TICK_BY_TICK);
        qos.timeInfo(65532);

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        qos.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        String correct = "{\"Timeliness\":\"Realtime\",\"Rate\":\"TickByTick\"}";
        assertTrue(converter.getPrimitiveHandler(DataTypes.QOS).encodeJson(dec, buffer, convError));
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 3]);
        }

        dec.clear();
        enc.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(200));

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        qos.clear();
        qos.timeliness(QosTimeliness.DELAYED);
        qos.rate(QosRates.TIME_CONFLATED);
        qos.rateInfo(65533);
        qos.timeInfo(65532);

        buffer.position = 10;
        buffer.data = new byte[15];

        qos.encode(enc);
        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getPrimitiveHandler(DataTypes.QOS).encodeJson(dec, buffer, convError));
        correct = "{\"Timeliness\":\"Delayed\",\"Rate\":\"TimeConflated\",\"TimeInfo\":65532,\"RateInfo\":65533}";
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 10]);
        }
    }

    @Test
    public void testStateConverter() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));

        JsonBuffer buffer = new JsonBuffer();
        buffer.position = 17;
        buffer.data = new byte[25];

        State state = CodecFactory.createState();
        Buffer text = CodecFactory.createBuffer();
        text.data("this is a test string \"");
        state.text(text);

        state.code(StateCodes.FAILOVER_COMPLETED);
        state.streamState(StreamStates.REDIRECTED);

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        state.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getPrimitiveHandler(DataTypes.STATE).encodeJson(dec, buffer, convError));
        String correct = "{\"Stream\":\"Redirected\",\"Data\":\"NoChange\",\"Code\":\"FailoverCompleted\",\"Text\":\"this is a test string \\\"\"}";
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 17]);
        }
    }

    @Test
    public void testBufferConverter() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        JsonBuffer buffer = new JsonBuffer();
        buffer.position = 17;
        buffer.data = new byte[25];

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));

        Buffer text = CodecFactory.createBuffer();
        text.data("this is a test string \"");

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        text.encode(enc);
        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(dec, buffer, convError));
        String correct = "\"dGhpcyBpcyBhIHRlc3Qgc3RyaW5nICI=\"";
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 17]);
        }

        enc.clear();
        dec.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(200));

        text = CodecFactory.createBuffer();
        text.data("this is a test string ");

        buffer.position = 17;
        buffer.data = new byte[25];

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        text.encode(enc);
        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(dec, buffer, convError));
        correct = "\"dGhpcyBpcyBhIHRlc3Qgc3RyaW5nIA==\"";
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 17]);
        }

    }

    @Test
    public void testEnumerationConverter() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));

        JsonBuffer buffer = new JsonBuffer();
        buffer.position = 4;
        buffer.data = new byte[25];

        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        Enum enumeration = CodecFactory.createEnum();
        enumeration.value(12);

        enumeration.encode(enc);
        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getPrimitiveHandler(DataTypes.ENUM).encodeJson(dec, buffer, convError));
        String correct = "12";
        for (int i = 0; i < correct.length(); i++) {
            assertEquals(correct.charAt(i), (char)buffer.data[i + 4]);
        }
    }
}
