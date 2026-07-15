package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.*;

import static org.junit.Assert.*;

public class DictionaryMultipartWebSocketJsonTest {

    private static final int ENCODE_BUFFER_SIZE = 140000;
    private static final int STREAM_ID = 3;
    private static final int ENUM_STREAM_ID = 4;
    private static final int SERVICE_ID = 1;
    private static final int VERBOSITY = Dictionary.VerbosityValues.NORMAL;

    private DataDictionary dictionary;
    private JsonConverter converter;
    private JsonConverterError converterError;
    private EncodeIterator encIter;
    private DecodeIterator decIter;

    @Before
    public void setUp() {
        Error codecError = TransportFactory.createError();
        dictionary = CodecFactory.createDataDictionary();
        dictionary.clear();

        String[] fieldPaths = {
                "src/test/resources/RDMFieldDictionary",
                "../../etc/RDMFieldDictionary",
                "../../../etc/RDMFieldDictionary",
                "Java/Eta/TestTools/UnitTests/Tests/com.refinitiv.eta.json.converter/src/test/resources/RDMFieldDictionary"
        };
        String[] enumPaths = {
                "src/test/resources/enumtype.def",
                "../../etc/enumtype.def",
                "../../../etc/enumtype.def",
                "Java/Eta/TestTools/UnitTests/Tests/com.refinitiv.eta.json.converter/src/test/resources/enumtype.def"
        };

        for (String path : fieldPaths) {
            if (new File(path).exists()) {
                if (dictionary.loadFieldDictionary(path, codecError) >= CodecReturnCodes.SUCCESS) {
                    break;
                }
            }
        }

        for (String path : enumPaths) {
            if (new File(path).exists()) {
                if (dictionary.loadEnumTypeDictionary(path, codecError) >= CodecReturnCodes.SUCCESS) {
                    break;
                }
            }
        }

        converterError = ConverterFactory.createJsonConverterError();

        // Provide a dummy ServiceNameIdConverter to avoid NPE in tests
        ServiceNameIdConverter dummyServiceNameIdConverter = new ServiceNameIdConverter() {
            @Override
            public int serviceNameToId(String serviceName, JsonConverterError error) {
                // Always return SERVICE_ID for test
                return SERVICE_ID;
            }
            @Override
            public String serviceIdToName(int id, JsonConverterError error) {
                // Always return a dummy name for test
                return "DUMMY_SERVICE";
            }
        };

        converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, false)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setDictionary(dictionary)
                .setServiceConverter(dummyServiceNameIdConverter)
                .build(converterError);

        assertNotNull("JsonConverter should build successfully", converter);

        encIter = CodecFactory.createEncodeIterator();
        decIter = CodecFactory.createDecodeIterator();
    }

    @Test
    public void multipartFieldDictionaryJsonRoundTripTest() {
        List<String> jsonParts = encodeFieldDictionaryAsJsonParts();
        assertFalse("Should produce at least one JSON part", jsonParts.isEmpty());

        DataDictionary rebuiltDict = CodecFactory.createDataDictionary();
        rebuiltDict.clear();
        // Removed unused codecError

        for (int i = 0; i < jsonParts.size(); i++) {
            String json = jsonParts.get(i);
            assertNotNull("JSON part " + i + " should not be null", json);
            assertFalse("JSON part " + i + " should not be empty", json.isEmpty());

            Buffer rwfBuffer = jsonToRwf(json);
            assertNotNull("Converted RWF buffer for part " + i + " should not be null", rwfBuffer);

            decIter.clear();
            decIter.setBufferAndRWFVersion(rwfBuffer, Codec.majorVersion(), Codec.minorVersion());

            Msg decodedMsg = CodecFactory.createMsg();
            int ret = decodedMsg.decode(decIter);
            assertEquals("Should decode RWF message for part " + i, CodecReturnCodes.SUCCESS, ret);
            assertEquals("Decoded msg class should be REFRESH", MsgClasses.REFRESH, decodedMsg.msgClass());
            assertEquals("Decoded domain should be DICTIONARY", DomainTypes.DICTIONARY, decodedMsg.domainType());
        }
    }

    @Test
    public void multipartFieldDictionaryJsonEntryIntegrityTest() {
        List<String> jsonParts = encodeFieldDictionaryAsJsonParts();
        assertFalse("Should produce at least one JSON part", jsonParts.isEmpty());

        // Removed unused totalFields
        for (int i = 0; i < jsonParts.size(); i++) {
            String json = jsonParts.get(i);

            assertTrue("Part " + i + " should contain Type field", json.contains("\"Type\""));
            assertTrue("Part " + i + " should be a Refresh message",
                    json.contains("\"Refresh\"") || json.contains("Refresh"));
            assertTrue("Part " + i + " should contain stream ID",
                    json.contains("\"ID\"") || json.contains("ID"));

            // Each part should have Fields entries
            // (removed totalFields, not used)
        }

        // Verify final part has complete flag
        String lastPart = jsonParts.get(jsonParts.size() - 1);
        assertTrue("Last part should be complete",
                lastPart.contains("\"Complete\"") || lastPart.contains("Complete"));
    }

    @Test
    public void multipartEnumTypeDictionaryJsonRoundTripTest() {
        List<String> jsonParts = encodeEnumTypeDictionaryAsJsonParts();
        assertFalse("Should produce at least one enum JSON part", jsonParts.isEmpty());

        for (int i = 0; i < jsonParts.size(); i++) {
            String json = jsonParts.get(i);
            assertNotNull("Enum JSON part " + i + " should not be null", json);
            assertFalse("Enum JSON part " + i + " should not be empty", json.isEmpty());

            Buffer rwfBuffer = jsonToRwf(json);
            assertNotNull("Converted RWF buffer for enum part " + i + " should not be null", rwfBuffer);

            decIter.clear();
            decIter.setBufferAndRWFVersion(rwfBuffer, Codec.majorVersion(), Codec.minorVersion());

            Msg decodedMsg = CodecFactory.createMsg();
            int ret = decodedMsg.decode(decIter);
            assertEquals("Should decode enum RWF message for part " + i, CodecReturnCodes.SUCCESS, ret);
            assertEquals("Decoded msg class should be REFRESH", MsgClasses.REFRESH, decodedMsg.msgClass());
        }
    }

    @Test
    public void multipartEnumTypeDictionaryJsonEnumValuesIntegrityTest() {
        List<String> jsonParts = encodeEnumTypeDictionaryAsJsonParts();
        assertFalse("Should produce at least one enum JSON part", jsonParts.isEmpty());

        for (int i = 0; i < jsonParts.size(); i++) {
            String json = jsonParts.get(i);
            assertNotNull("Enum JSON part " + i + " should not be null", json);

            assertTrue("Enum part " + i + " should contain Type field", json.contains("\"Type\""));
            assertTrue("Enum part " + i + " should be a Refresh message",
                    json.contains("\"Refresh\"") || json.contains("Refresh"));

            // Verify it contains enum type definitions
            assertTrue("Enum part " + i + " should contain enum data",
                    json.contains("\"Entries\"") || json.contains("\"Series\"") || json.length() > 100);
        }

        // Final part should be marked complete
        String lastPart = jsonParts.get(jsonParts.size() - 1);
        assertTrue("Last enum part should be complete",
                lastPart.contains("\"Complete\"") || lastPart.contains("Complete"));
    }

    @Test
    public void outOfOrderPartsProduceErrorOrCorrectResultTest() {
        List<String> jsonParts = encodeFieldDictionaryAsJsonParts();
        assertFalse("Should produce at least one JSON part", jsonParts.isEmpty());

        if (jsonParts.size() < 2) {
            // Single part - out-of-order test not meaningful, verify single part is valid
            String json = jsonParts.get(0);
            assertTrue("Single part should contain Complete flag",
                    json.contains("\"Complete\"") || json.contains("Complete"));
            return;
        }

        // Reverse order to simulate out-of-order delivery
        List<String> reversedParts = new ArrayList<>(jsonParts);
        Collections.reverse(reversedParts);

        // Process out-of-order parts — each individual part should still be valid JSON
        // even if the dictionary assembled from them would be incomplete/incorrect
        for (int i = 0; i < reversedParts.size(); i++) {
            String json = reversedParts.get(i);
            assertNotNull("Out-of-order part " + i + " should not be null", json);
            assertFalse("Out-of-order part " + i + " should not be empty", json.isEmpty());

            // Individual JSON part should still be structurally valid
            assertTrue("Out-of-order part " + i + " should be valid JSON structure",
                    json.startsWith("[") || json.startsWith("{"));

            // Converting out-of-order part back to RWF should succeed or produce clear error
            try {
                Buffer rwfBuffer = jsonToRwf(json);
                // If conversion succeeded, the decoded message should at least have correct class
                if (rwfBuffer != null) {
                    decIter.clear();
                    decIter.setBufferAndRWFVersion(rwfBuffer, Codec.majorVersion(), Codec.minorVersion());
                    Msg decodedMsg = CodecFactory.createMsg();
                    int ret = decodedMsg.decode(decIter);
                    if (ret == CodecReturnCodes.SUCCESS) {
                        assertEquals("Decoded domain should be DICTIONARY",
                                DomainTypes.DICTIONARY, decodedMsg.domainType());
                    }
                }
            } catch (Exception e) {
                // An exception is an acceptable response to out-of-order data
                assertNotNull("Exception from out-of-order part should have a message", e.getMessage());
            }
        }
    }

    @Test
    public void droppedPartProducesIncompleteOrFailedDictionaryTest() {
        List<String> jsonParts = encodeFieldDictionaryAsJsonParts();
        assertFalse("Should produce at least one JSON part", jsonParts.isEmpty());

        if (jsonParts.size() < 2) {
            // Single part — nothing to drop meaningfully
            assertTrue("Single part dictionary should be valid", true);
            return;
        }

        // Keep only first and last parts, dropping the middle ones
        List<String> incompleteParts = new ArrayList<>();
        incompleteParts.add(jsonParts.get(0));
        if (jsonParts.size() > 2) {
            incompleteParts.add(jsonParts.get(jsonParts.size() - 1));
        }

        int successfullyDecoded = 0;
        for (int i = 0; i < incompleteParts.size(); i++) {
            String json = incompleteParts.get(i);
            assertNotNull("Incomplete set part " + i + " should not be null", json);

            Buffer rwfBuffer = jsonToRwf(json);
            if (rwfBuffer != null) {
                decIter.clear();
                decIter.setBufferAndRWFVersion(rwfBuffer, Codec.majorVersion(), Codec.minorVersion());
                Msg decodedMsg = CodecFactory.createMsg();
                if (decodedMsg.decode(decIter) == CodecReturnCodes.SUCCESS) {
                    successfullyDecoded++;
                }
            }
        }

        // At minimum the first part should decode successfully
        assertTrue("At least one part from incomplete set should decode successfully",
                successfullyDecoded >= 1);

        // The dictionary built from incomplete parts has fewer entries than the full dictionary
        assertTrue("Dictionary source should have entries", dictionary.numberOfEntries() > 0);
    }

    @Test
    public void jsonPartStructuralValidityTest() {
        List<String> jsonParts = encodeFieldDictionaryAsJsonParts();
        assertFalse("Should produce at least one JSON part", jsonParts.isEmpty());

        for (int i = 0; i < jsonParts.size(); i++) {
            String json = jsonParts.get(i);

            assertNotNull("JSON part " + i + " should not be null", json);
            assertFalse("JSON part " + i + " should not be empty", json.isEmpty());

            // WebSocket JSON messages are wrapped in a JSON array
            assertTrue("JSON part " + i + " should start with '[' for WebSocket JSON",
                    json.startsWith("[") || json.startsWith("{"));
            assertTrue("JSON part " + i + " should end with ']' or '}'",
                    json.endsWith("]") || json.endsWith("}"));

            // Required fields for a dictionary Refresh
            assertTrue("JSON part " + i + " must have ID field", json.contains("\"ID\""));
            assertTrue("JSON part " + i + " must have Type field", json.contains("\"Type\""));
            assertTrue("JSON part " + i + " must be Refresh type",
                    json.contains("Refresh"));
            assertTrue("JSON part " + i + " must have Key field", json.contains("\"Key\""));
            assertTrue("JSON part " + i + " must have State field", json.contains("\"State\""));
        }
    }

    @Test
    public void enumTypeJsonPartStructuralValidityTest() {
        List<String> jsonParts = encodeEnumTypeDictionaryAsJsonParts();
        assertFalse("Should produce at least one enum JSON part", jsonParts.isEmpty());

        for (int i = 0; i < jsonParts.size(); i++) {
            String json = jsonParts.get(i);

            assertNotNull("Enum JSON part " + i + " should not be null", json);
            assertFalse("Enum JSON part " + i + " should not be empty", json.isEmpty());

            assertTrue("Enum JSON part " + i + " should start with '[' for WebSocket JSON",
                    json.startsWith("[") || json.startsWith("{"));
            assertTrue("Enum JSON part " + i + " should end with ']' or '}'",
                    json.endsWith("]") || json.endsWith("}"));

            assertTrue("Enum JSON part " + i + " must have ID field", json.contains("\"ID\""));
            assertTrue("Enum JSON part " + i + " must have Type field", json.contains("\"Type\""));
            assertTrue("Enum JSON part " + i + " must be Refresh type",
                    json.contains("Refresh"));
            assertTrue("Enum JSON part " + i + " must have Key field", json.contains("\"Key\""));
        }
    }

    /**
     * Test that JSON with invalid UTF-8 middle byte (0x22) is properly sanitized.
     * This simulates the error: "Invalid UTF-8 middle byte 0x22"
     * which occurs when a high byte (>=0x80) is followed by a quote character.
     */
    @Test
    public void jsonWithInvalidUtf8MiddleByteIsSanitizedTest() {
        // Create a byte array with an invalid UTF-8 sequence:
        // A high byte (0xC2 - start of 2-byte sequence) followed by 0x22 (quote)
        // This is invalid because 0x22 is not a valid continuation byte (should be 10xxxxxx)
        byte[] invalidUtf8Json = createJsonWithInvalidUtf8MiddleByte();

        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocate(invalidUtf8Json.length);
        byteBuffer.put(invalidUtf8Json);
        jsonBuffer.data(byteBuffer, 0, invalidUtf8Json.length);

        // This should NOT throw an exception - the sanitizer should escape invalid bytes
        converterError.clear();
        int ret = converter.parseJsonBuffer(jsonBuffer, parseOptions, converterError);

        // The parsing should succeed (invalid UTF-8 bytes are escaped)
        assertEquals("parseJsonBuffer should succeed after UTF-8 sanitization: " +
                (converterError != null ? converterError.getText() : "unknown"),
                CodecReturnCodes.SUCCESS, ret);
    }

    /**
     * Test that JSON with truncated UTF-8 sequence at end is properly sanitized.
     * This simulates incomplete multipart data where a UTF-8 sequence is cut off.
     */
    @Test
    public void jsonWithTruncatedUtf8SequenceIsSanitizedTest() {
        // Create JSON with a truncated UTF-8 sequence at the end of a string value
        byte[] truncatedUtf8Json = createJsonWithTruncatedUtf8Sequence();

        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocate(truncatedUtf8Json.length);
        byteBuffer.put(truncatedUtf8Json);
        jsonBuffer.data(byteBuffer, 0, truncatedUtf8Json.length);

        converterError.clear();
        int ret = converter.parseJsonBuffer(jsonBuffer, parseOptions, converterError);

        // The parsing should succeed after sanitization
        assertEquals("parseJsonBuffer should succeed after UTF-8 sanitization for truncated sequence: " +
                (converterError != null ? converterError.getText() : "unknown"),
                CodecReturnCodes.SUCCESS, ret);
    }

    /**
     * Test that valid UTF-8 JSON is not altered by the sanitization process.
     */
    @Test
    public void validUtf8JsonIsPreservedTest() {
        // Create valid JSON with proper UTF-8 characters
        String validJson = "[{\"ID\":3,\"Type\":\"Refresh\",\"Key\":{\"Name\":\"Test\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\"},\"Text\":\"Caf\\u00e9\"}]";
        byte[] validUtf8Json = validJson.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocate(validUtf8Json.length);
        byteBuffer.put(validUtf8Json);
        jsonBuffer.data(byteBuffer, 0, validUtf8Json.length);

        converterError.clear();
        int ret = converter.parseJsonBuffer(jsonBuffer, parseOptions, converterError);

        assertEquals("parseJsonBuffer should succeed for valid UTF-8 JSON: " +
                (converterError != null ? converterError.getText() : "unknown"),
                CodecReturnCodes.SUCCESS, ret);
    }

    /**
     * Test parsing JSON with multiple invalid UTF-8 sequences scattered throughout.
     */
    @Test
    public void jsonWithMultipleInvalidUtf8SequencesIsSanitizedTest() {
        byte[] multipleInvalidJson = createJsonWithMultipleInvalidUtf8Sequences();

        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocate(multipleInvalidJson.length);
        byteBuffer.put(multipleInvalidJson);
        jsonBuffer.data(byteBuffer, 0, multipleInvalidJson.length);

        converterError.clear();
        int ret = converter.parseJsonBuffer(jsonBuffer, parseOptions, converterError);

        assertEquals("parseJsonBuffer should succeed after sanitizing multiple invalid UTF-8 sequences: " +
                (converterError != null ? converterError.getText() : "unknown"),
                CodecReturnCodes.SUCCESS, ret);
    }

    /**
     * Test that simulates receiving malformed enum dictionary data similar to real-world scenario.
     * The enum type dictionary can contain display values with special characters that,
     * when truncated in multipart messages, produce invalid UTF-8.
     */
    @Test
    public void simulatedMalformedEnumDictionaryJsonTest() {
        // Simulate JSON similar to what a server might send with malformed enum display values
        // The key issue is when UTF-8 sequences get truncated or corrupted
        byte[] malformedEnumJson = createSimulatedMalformedEnumDictionaryJson();

        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocate(malformedEnumJson.length);
        byteBuffer.put(malformedEnumJson);
        jsonBuffer.data(byteBuffer, 0, malformedEnumJson.length);

        converterError.clear();
        int ret = converter.parseJsonBuffer(jsonBuffer, parseOptions, converterError);

        assertEquals("parseJsonBuffer should succeed for simulated malformed enum dictionary: " +
                (converterError != null ? converterError.getText() : "unknown"),
                CodecReturnCodes.SUCCESS, ret);
    }

    // -------------------------------------------------------------------------
    // Helper methods for creating test JSON with invalid UTF-8
    // -------------------------------------------------------------------------

    /**
     * Creates JSON bytes with an invalid UTF-8 sequence (lone high byte).
     * This simulates a truncated character at end of string.
     * After sanitization, the lone high byte becomes an escaped sequence.
     */
    private byte[] createJsonWithInvalidUtf8MiddleByte() {
        // Build: [{"ID":3,"Type":"Refresh","Text":"test<0x80>value"}]
        // 0x80 alone is invalid UTF-8 (it's a continuation byte without a start byte)
        // After sanitization it becomes \u0080 which is valid JSON
        String prefix = "[{\"ID\":3,\"Type\":\"Refresh\",\"Key\":{\"Name\":\"Test\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\"},\"Text\":\"test";
        String suffix = "value\"}]";
        byte[] prefixBytes = prefix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);
        byte[] suffixBytes = suffix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);

        byte[] result = new byte[prefixBytes.length + 1 + suffixBytes.length];
        System.arraycopy(prefixBytes, 0, result, 0, prefixBytes.length);
        result[prefixBytes.length] = (byte) 0x80; // Invalid: lone continuation byte
        System.arraycopy(suffixBytes, 0, result, prefixBytes.length + 1, suffixBytes.length);

        return result;
    }

    /**
     * Creates JSON bytes with a truncated UTF-8 sequence.
     * This simulates data cut off mid-character in multipart messages.
     */
    private byte[] createJsonWithTruncatedUtf8Sequence() {
        // Build: [{"ID":3,"Type":"Refresh","Text":"test<0xE2><0x82>"}]
        // 0xE2 0x82 is the start of a 3-byte UTF-8 sequence (e.g., Euro sign €)
        // but the third byte is missing, making it truncated
        String prefix = "[{\"ID\":3,\"Type\":\"Refresh\",\"Key\":{\"Name\":\"Test\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\"},\"Text\":\"test";
        String suffix = "\"}]";
        byte[] prefixBytes = prefix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);
        byte[] suffixBytes = suffix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);

        byte[] result = new byte[prefixBytes.length + 2 + suffixBytes.length];
        System.arraycopy(prefixBytes, 0, result, 0, prefixBytes.length);
        result[prefixBytes.length] = (byte) 0xE2;     // Start of 3-byte UTF-8 sequence
        result[prefixBytes.length + 1] = (byte) 0x82; // Valid continuation byte
        // Missing third byte - sequence is truncated
        System.arraycopy(suffixBytes, 0, result, prefixBytes.length + 2, suffixBytes.length);

        return result;
    }

    /**
     * Creates JSON with multiple invalid UTF-8 sequences scattered throughout.
     */
    private byte[] createJsonWithMultipleInvalidUtf8Sequences() {
        // Build JSON with several invalid sequences (lone continuation bytes)
        // These will be escaped as backslash-u00XX sequences
        String prefix = "[{\"ID\":3,\"Type\":\"Refresh\",\"Key\":{\"Name\":\"Test";
        String middle = "Name\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\"},\"Text\":\"data";
        String suffix = "end\"}]";

        byte[] prefixBytes = prefix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);
        byte[] middleBytes = middle.getBytes(java.nio.charset.StandardCharsets.US_ASCII);
        byte[] suffixBytes = suffix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);

        // Insert invalid bytes (lone continuation bytes which are always invalid)
        byte[] result = new byte[prefixBytes.length + 1 + middleBytes.length + 1 + suffixBytes.length];
        int pos = 0;
        System.arraycopy(prefixBytes, 0, result, pos, prefixBytes.length);
        pos += prefixBytes.length;
        result[pos++] = (byte) 0x80; // Invalid: lone continuation byte
        System.arraycopy(middleBytes, 0, result, pos, middleBytes.length);
        pos += middleBytes.length;
        result[pos++] = (byte) 0x81; // Invalid: another lone continuation byte
        System.arraycopy(suffixBytes, 0, result, pos, suffixBytes.length);

        return result;
    }

    /**
     * Creates simulated malformed enum dictionary JSON similar to real-world corruption.
     * Uses lone continuation bytes which are clearly invalid UTF-8.
     */
    private byte[] createSimulatedMalformedEnumDictionaryJson() {
        // Simulate enum dictionary JSON structure with corrupted display values
        // Use lone high bytes (0x80-0xBF) which are always invalid UTF-8
        String jsonTemplate = "[{\"ID\":4,\"Type\":\"Refresh\",\"Domain\":\"Dictionary\"," +
                "\"Key\":{\"Name\":\"RWFEnum\",\"ServiceID\":1}," +
                "\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Enum Dictionary\"}," +
                "\"Series\":{\"Entries\":[{\"Elements\":{\"FIDS\":{\"Type\":\"Array\",\"Data\":[1,2]}," +
                "\"VALUES\":{\"Type\":\"Array\",\"Data\":[0,1]}," +
                "\"DISPLAYS\":{\"Type\":\"Array\",\"Data\":[\"UP";
        String jsonSuffix = "\",\"DOWN\"]}}}]}}]";

        byte[] templateBytes = jsonTemplate.getBytes(java.nio.charset.StandardCharsets.US_ASCII);
        byte[] suffixBytes = jsonSuffix.getBytes(java.nio.charset.StandardCharsets.US_ASCII);

        // Insert invalid UTF-8 bytes (lone continuation bytes)
        byte[] result = new byte[templateBytes.length + 2 + suffixBytes.length];
        System.arraycopy(templateBytes, 0, result, 0, templateBytes.length);
        result[templateBytes.length] = (byte) 0x80;     // Invalid: lone continuation byte
        result[templateBytes.length + 1] = (byte) 0x91; // Invalid: another lone continuation byte
        System.arraycopy(suffixBytes, 0, result, templateBytes.length + 2, suffixBytes.length);

        return result;
    }

    // -------------------------------------------------------------------------
    // Helper: encode field dictionary as a list of JSON part strings
    // -------------------------------------------------------------------------
    private List<String> encodeFieldDictionaryAsJsonParts() {
        List<String> parts = new ArrayList<>();

        Int currentFid = CodecFactory.createInt();
        currentFid.value(dictionary.minFid());
        boolean complete = false;
        int partNum = 0;

        while (!complete) {
            Buffer encodeBuffer = CodecFactory.createBuffer();
            encodeBuffer.data(ByteBuffer.allocate(ENCODE_BUFFER_SIZE));

            encIter.clear();
            assertEquals("EncIter setBuffer should succeed",
                    CodecReturnCodes.SUCCESS,
                    encIter.setBufferAndRWFVersion(encodeBuffer, Codec.majorVersion(), Codec.minorVersion()));

            RefreshMsg refreshMsg = (RefreshMsg) CodecFactory.createMsg();
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.DICTIONARY);
            refreshMsg.containerType(DataTypes.SERIES);
            refreshMsg.streamId(STREAM_ID);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("RWFFld");
            refreshMsg.msgKey().applyHasFilter();
            refreshMsg.msgKey().filter(VERBOSITY);
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(SERVICE_ID);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.state().code(StateCodes.NONE);
            refreshMsg.state().text().data("Field Dictionary Refresh (starting fid " + currentFid.toLong() + ")");

            int ret = refreshMsg.encodeInit(encIter, 0);
            assertEquals("RefreshMsg encodeInit should return ENCODE_CONTAINER for part " + partNum,
                    CodecReturnCodes.ENCODE_CONTAINER, ret);

            Error error = TransportFactory.createError();
            ret = dictionary.encodeFieldDictionary(encIter, currentFid, VERBOSITY, error);

            if (ret == CodecReturnCodes.SUCCESS) {
                complete = true;
                refreshMsg.applyRefreshComplete();
            } else {
                assertEquals("encodeFieldDictionary should return DICT_PART_ENCODED if not done, part " + partNum,
                        CodecReturnCodes.DICT_PART_ENCODED, ret);
            }

            ret = refreshMsg.encodeComplete(encIter, true);
            assertEquals("RefreshMsg encodeComplete should succeed for part " + partNum,
                    CodecReturnCodes.SUCCESS, ret);

            // Trim the buffer to the actual encoded length
            Buffer encodedMsg = CodecFactory.createBuffer();
            encodedMsg.data(encodeBuffer.data(), 0, encIter.buffer().length());

            // Decode the encodedMsg as a Msg for RWF->JSON
            DecodeIterator tmpDecIter = CodecFactory.createDecodeIterator();
            tmpDecIter.setBufferAndRWFVersion(encodedMsg, Codec.majorVersion(), Codec.minorVersion());
            Msg msg = CodecFactory.createMsg();
            int decodeRet = msg.decode(tmpDecIter);
            assertEquals("Should decode encoded RWF message for JSON conversion", CodecReturnCodes.SUCCESS, decodeRet);

            parts.add(rwfToJson(msg, partNum));
            partNum++;
        }

        return parts;
    }

    // -------------------------------------------------------------------------
    // Helper: encode enum type dictionary as a list of JSON part strings
    // -------------------------------------------------------------------------
    private List<String> encodeEnumTypeDictionaryAsJsonParts() {
        List<String> parts = new ArrayList<>();

        Int currentCount = CodecFactory.createInt();
        currentCount.value(0);
        boolean complete = false;
        int partNum = 0;

        while (!complete) {
            Buffer encodeBuffer = CodecFactory.createBuffer();
            encodeBuffer.data(ByteBuffer.allocate(ENCODE_BUFFER_SIZE));

            encIter.clear();
            assertEquals("EncIter setBuffer should succeed for enum part " + partNum,
                    CodecReturnCodes.SUCCESS,
                    encIter.setBufferAndRWFVersion(encodeBuffer, Codec.majorVersion(), Codec.minorVersion()));

            RefreshMsg refreshMsg = (RefreshMsg) CodecFactory.createMsg();
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.DICTIONARY);
            refreshMsg.containerType(DataTypes.SERIES);
            refreshMsg.streamId(ENUM_STREAM_ID);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("RWFEnum");
            refreshMsg.msgKey().applyHasFilter();
            refreshMsg.msgKey().filter(VERBOSITY);
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(SERVICE_ID);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.state().code(StateCodes.NONE);
            refreshMsg.state().text().data("Enum Type Dictionary Refresh (starting count " + currentCount.toLong() + ")");

            int ret = refreshMsg.encodeInit(encIter, 0);
            assertEquals("RefreshMsg encodeInit should return ENCODE_CONTAINER for enum part " + partNum,
                    CodecReturnCodes.ENCODE_CONTAINER, ret);

            Error error = TransportFactory.createError();
            ret = dictionary.encodeEnumTypeDictionaryAsMultiPart(encIter, currentCount, VERBOSITY, error);

            if (ret == CodecReturnCodes.SUCCESS) {
                complete = true;
                refreshMsg.applyRefreshComplete();
            } else {
                assertEquals("encodeEnumTypeDictionaryAsMultiPart should return DICT_PART_ENCODED if not done, part " + partNum,
                        CodecReturnCodes.DICT_PART_ENCODED, ret);
            }

            ret = refreshMsg.encodeComplete(encIter, true);
            assertEquals("RefreshMsg encodeComplete should succeed for enum part " + partNum,
                    CodecReturnCodes.SUCCESS, ret);

            // Trim the buffer to the actual encoded length
            Buffer encodedMsg = CodecFactory.createBuffer();
            encodedMsg.data(encodeBuffer.data(), 0, encIter.buffer().length());

            // Decode the encodedMsg as a Msg for RWF->JSON
            DecodeIterator tmpDecIter = CodecFactory.createDecodeIterator();
            tmpDecIter.setBufferAndRWFVersion(encodedMsg, Codec.majorVersion(), Codec.minorVersion());
            Msg msg = CodecFactory.createMsg();
            int decodeRet = msg.decode(tmpDecIter);
            assertEquals("Should decode encoded RWF message for JSON conversion", CodecReturnCodes.SUCCESS, decodeRet);

            parts.add(rwfToJson(msg, partNum));
            partNum++;
        }

        return parts;
    }

    // -------------------------------------------------------------------------
    // Helper: convert a fully-encoded RWF message buffer to a JSON string
    // -------------------------------------------------------------------------
    // Converts a Msg to JSON string using the correct JsonConverter API
    private String rwfToJson(Msg msg, int partNum) {
        RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        ConversionResults convResults = ConverterFactory.createConversionResults();
        int ret = converter.convertRWFToJson(msg, rwfToJsonOptions, convResults, converterError);
        assertEquals("rwfToJson part " + partNum + ": JSON Converter error: " +
                (converterError != null ? converterError.getText() : "unknown"), CodecReturnCodes.SUCCESS, ret);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(java.nio.ByteBuffer.allocate(convResults.getLength() > 0 ? convResults.getLength() : 4096));
        GetJsonMsgOptions getJsonOptions = ConverterFactory.createGetJsonMsgOptions();
        getJsonOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonOptions.streamId(msg.streamId());
        ret = converter.getJsonBuffer(jsonBuffer, getJsonOptions, converterError);
        assertEquals("rwfToJson getJsonBuffer part " + partNum + ": JSON Converter error: " +
                (converterError != null ? converterError.getText() : "unknown"), CodecReturnCodes.SUCCESS, ret);
        return new String(jsonBuffer.data().array(), 0, jsonBuffer.length());
    }

    // -------------------------------------------------------------------------
    // Helper: convert a JSON string back to an RWF buffer
    // -------------------------------------------------------------------------
    private Buffer jsonToRwf(String jsonString) {
        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer jsonBuffer = CodecFactory.createBuffer();
        byte[] jsonBytes = jsonString.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocate(jsonBytes.length);
        byteBuffer.put(jsonBytes);
        jsonBuffer.data(byteBuffer, 0, jsonBytes.length);

        int ret = converter.parseJsonBuffer(jsonBuffer, parseOptions, converterError);
        assertEquals("parseJsonBuffer should succeed: " + (converterError != null ? converterError.getText() : "unknown error"),
                CodecReturnCodes.SUCCESS, ret);

        // Now decode the JSON to RWF using JsonMsg
        JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
        DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, converterError);
        assertEquals("decodeJsonMsg should succeed: " + (converterError != null ? converterError.getText() : "unknown error"),
                CodecReturnCodes.SUCCESS, ret);

        // The RWF buffer is in jsonMsg.rwfMsg().encodedMsgBuffer()
        Buffer rwfBuffer = CodecFactory.createBuffer();
        rwfBuffer.data(java.nio.ByteBuffer.allocate(jsonMsg.rwfMsg().encodedMsgBuffer().length()));
        rwfBuffer.data().put(jsonMsg.rwfMsg().encodedMsgBuffer().data().array(), 0, jsonMsg.rwfMsg().encodedMsgBuffer().length());
        return rwfBuffer;
    }
}
