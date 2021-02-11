package com.refinitiv.eta.shared;

import com.refinitiv.eta.json.converter.*;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.*;
import com.refinitiv.eta.transport.Error;

public class JsonSession {

    //   {\"Type\":\"Pong\"}
    private static final byte[] WEB_SOCKET_JSON_PONG_MESSAGE = new byte[]{
            0X7B, 0X22, 0X54, 0X79, 0X70, 0X65, 0X22, 0X3A, 0X22, 0X50, 0X6F, 0X6E, 0X67, 0X22, 0X7D
    };

    private Channel channel;
    private JsonConverterBuilder converterBuilder = ConverterFactory.createJsonConverterBuilder();
    private JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    private JsonConverterError converterError = ConverterFactory.createJsonConverterError();

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();

    private WriteArgs pingWriteArgs = TransportFactory.createWriteArgs();

    private XmlTraceDump xmlTraceDump = CodecFactory.createXmlTraceDump();
    private StringBuilder xmlString = new StringBuilder();
    private boolean enableXmlTrace;

    private JsonConverter converter;
    private DecodeJsonMsgOptions decodeJsonMsgOptions;
    private ParseJsonOptions parseJsonOptions;
    private GetJsonMsgOptions getJsonMsgOptions;
    private RWFToJsonOptions rwfToJsonOptions;
    private ConversionResults conversionResults;

    private boolean initialized;

    public int sendJsonMsg(TransportBuffer msgBuffer, WriteArgs writeArgs, Error error) {
        int ret = channel.write(msgBuffer, writeArgs, error);

        while (ret == TransportReturnCodes.WRITE_CALL_AGAIN) {
            if ((ret = channel.flush(error)) < TransportReturnCodes.SUCCESS) {
                return setError(error, TransportReturnCodes.FAILURE, "channel.flush() failed with return code %d - <%s>", ret, error.text());
            }
            ret = channel.write(msgBuffer, writeArgs, error);
        }

        if (ret != TransportReturnCodes.SUCCESS) {
            return setError(error, TransportReturnCodes.FAILURE, "channel.write() failed with return code %d - <%s>", ret, error.text());
        }
        return ret;
    }

    public int initialize(Channel channel, JsonConverterInitOptions converterInitOptions, Error error) {
    	
    	/* Always update with the latest channel */
        this.channel = channel;
    	
        if (initialized) {
            return setError(error, TransportReturnCodes.FAILURE, "This Json Session had been initialized");
        }

        decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
        parseJsonOptions = ConverterFactory.createParseJsonOptions();
        getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
        rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
        conversionResults = ConverterFactory.createConversionResults();

        this.converter = converterBuilder.setDictionary(converterInitOptions.getDataDictionary())
                .setServiceConverter(converterInitOptions.getServiceNameIdConverter())
                .setProperty(JsonConverterProperties.JSON_CPC_DEFAULT_SERVICE_ID, converterInitOptions.getDefaultServiceId())
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, converterInitOptions.isJsonExpandedEnumFields())
                .setProperty(JsonConverterProperties.JSON_CPC_USE_DEFAULT_DYNAMIC_QOS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, true)
                .build(converterError);

        this.enableXmlTrace = converterInitOptions.isEnableXmlTrace();
        initialized = true;
        return TransportReturnCodes.SUCCESS;
    }

    public int convertToJson(TransportBuffer transportBuffer, Error error) {
        jsonMsg.clear();
        dIter.clear();
        dIter.setBufferAndRWFVersion(transportBuffer, channel.majorVersion(), channel.minorVersion());

        final Msg rwfMsg = jsonMsg.rwfMsg();
        int ret = rwfMsg.decode(dIter);
        if (ret == CodecReturnCodes.SUCCESS) {
            rwfToJsonOptions.clear();
            rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
            converter.convertRWFToJson(rwfMsg, rwfToJsonOptions, conversionResults, converterError);
            if (converterError.isFailed()) {
                return setError(error, TransportReturnCodes.FAILURE,
                        "Failed to convert RWF to JSON protocol. Error code: %d. Error text: %s.",
                        converterError.getCode(), converterError.getText());
            }
            
            /* Releases the original buffer if success */
            channel.releaseBuffer(transportBuffer, error);
        }
        return ret;
    }

    public int convertFromJson(TransportBuffer jsonMsgBuffer, Error error) {
        parseJsonOptions.clear();
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        converterError.clear();
        int ret = converter.parseJsonBuffer(jsonMsgBuffer, parseJsonOptions, converterError);

        if (converterError.isFailed()) {
            return setError(error, TransportReturnCodes.FAILURE,
                    "Failed to parse JSON to RWF object. Error code: %d. Error text: %s.",
                    ret, converterError.getText());
        }

        jsonMsg.clear();
        decodeJsonMsgOptions.clear();
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, converterError);

        if (converterError.isSuccessful()) {
            switch (jsonMsg.jsonMsgClass()) {
                case JsonMsgClasses.RSSL_MESSAGE: {
                    if (enableXmlTrace) {
                        xmlString.setLength(0);
                        xmlString.append("\nMessage after JSON -> RWF conversion:");
                        final TransportBuffer messageBuffer =
                                channel.getBuffer(jsonMsg.rwfMsg().encodedMsgBuffer().length(), false, error);
                        messageBuffer.data().put(jsonMsg.rwfMsg().encodedMsgBuffer().data().array(), jsonMsg.rwfMsg().encodedMsgBuffer().position(), jsonMsg.rwfMsg().encodedMsgBuffer().length());
                        xmlTraceDump.dumpBuffer(channel, Codec.RWF_PROTOCOL_TYPE, messageBuffer, null, xmlString, error);
                        System.out.println(xmlString);
                    }
                    break;
                }
                case JsonMsgClasses.PING: {
                    final TransportBuffer pingBuffer =
                            channel.getBuffer(WEB_SOCKET_JSON_PONG_MESSAGE.length, false, error);
                    pingBuffer.data().put(WEB_SOCKET_JSON_PONG_MESSAGE);
                    pingWriteArgs.clear();
                    pingWriteArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
                    ret = sendJsonMsg(pingBuffer, pingWriteArgs, error);
                    if (ret >= TransportReturnCodes.SUCCESS) {
                        ret = TransportReturnCodes.READ_PING;
                    }
                    break;
                }
                case JsonMsgClasses.PONG: {
                    ret = TransportReturnCodes.READ_PING;
                    break;
                }
                default: {
                    ret = setError(error, TransportReturnCodes.FAILURE, "Received JSON error message:\n%s", jsonMsg.jsonMsgData().toString());
                    break;
                }
            }
        } else {
            ret = setError(error, TransportReturnCodes.FAILURE, "Code: %d, text: %s", converterError.getCode(), converterError.getText());
        }
        return ret;
    }

    public JsonMsg getJsonMsg() {
        return jsonMsg;
    }

    public TransportBuffer getTransportJsonBuffer(Error error) {
        Msg rwfMsg = jsonMsg.rwfMsg();
        getJsonMsgOptions.clear();
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(rwfMsg.streamId());
        getJsonMsgOptions.isCloseMsg(rwfMsg.msgClass() == MsgClasses.CLOSE);

        TransportBuffer tempBuffer = channel.getBuffer(conversionResults.getLength(), false, error);
        if (tempBuffer != null) {
            converter.getJsonBuffer(tempBuffer, getJsonMsgOptions, converterError);
        }

        if (converterError.isFailed() || tempBuffer == null) {
            setError(error, TransportReturnCodes.FAILURE,
                    "Failed to get converted JSON message. Error code: %d. Error text: %s",
                    converterError.getCode(), converterError.getText());
        }
        return tempBuffer;
    }

    public void clear() {
        converterBuilder.clear();
        jsonMsg.clear();
        converterError.clear();

        dIter.clear();

        pingWriteArgs.clear();
        xmlString.setLength(0);
        channel = null;

        if (initialized) {
            decodeJsonMsgOptions.clear();
            parseJsonOptions.clear();
            getJsonMsgOptions.clear();
            rwfToJsonOptions.clear();

            converter = null;
            initialized = false;
        }
    }

    private int setError(Error error, int errorCode, String errorText, Object... formatArgs) {
        error.errorId(errorCode);
        error.text(String.format(errorText, formatArgs));
        return errorCode;
    }
}
