package com.refinitiv.eta.json.converter;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;

import org.junit.Before;
import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.transport.TransportFactory;

public class PostUserInfoTest {
	Buffer jsonBuffer;
    Msg resultMsg;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    Buffer origMsgBuffer;
    EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
    DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    int streamId = MsgClasses.POST;
    Buffer errorBuffer = CodecFactory.createBuffer();
    GetJsonErrorParams errorParams = ConverterFactory.createJsonErrorParams();
    
    @Before
    public void init() {
        jsonMsg.clear();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        resultMsg = CodecFactory.createMsg();
        origMsgBuffer = CodecFactory.createBuffer();
        origMsgBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        decodeIter.clear();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[]{JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(streamId);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
    }
	
    @Test
    public void decodePostUserInfoAddressFromIntValue() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": 168310684,\"UserID\": 1},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
    
    @Test
    public void decodePostUserInfoAddressFromLongValue() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": 4294967295,\"UserID\": 1},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
    
    @Test
    public void decodePostUserInfoAddressFromStringValue() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": \"127.0.0.1\",\"UserID\": 1},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
    
    @Test
    public void decodePostUserInfoAddressFromNullValueValue_thenDecodingFails() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": null,\"UserID\": 1},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        assertTrue(convError.getText().indexOf("JSON Converter Token Type error: token: Address was not expected type:NULL") != -1);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        JsonConverterTestUtils.checkJsonErrorMsg(errorBuffer);
    }
    
    @Test
    public void decodePostUserInfoAddressFromBooleanValueValue_thenDecodingFails() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": true,\"UserID\": 1},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        assertTrue(convError.getText().indexOf("JSON Converter Token Type error: token: Address was not expected type:BOOLEAN") != -1);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        JsonConverterTestUtils.checkJsonErrorMsg(errorBuffer);
    }
    
    @Test
    public void decodePostUserInfoUserIdFromTextValue() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": 168310684,\"UserID\": \"1\"},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
    
    @Test
    public void decodePostUserInfoUserIdFromIntValue() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": 168310684,\"UserID\": 168310684},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
    
    @Test
    public void decodePostUserInfoUserIdFromLongValue() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"Ack\":true,\"PostID\":3,\"PermData\":\"AwO9ZWLA\",\"PostUserInfo\": {\"Address\": 168310684,\"UserID\": 4294967295},\"Message\":{\"ID\":0,\"Type\":\"Update\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":60000},\"Domain\":\"MarketPrice\",\"Fields\":{\"RDN_EXCHID\":155}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
}
