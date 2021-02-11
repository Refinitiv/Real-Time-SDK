package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.transport.TransportBuffer;

public interface JsonConverter {

    /**
     * parses the JSON contained in the Buffer provided and stores the parsed JSON internally until {@see decodeJsonMsg()} method is called;
     * in case two subsequent calls to this method are performed without calling {@see decodeJsonMsg()} between them,
     * messages obtained after the last call to parseJsonMsg will be stored internally and retrieved when {@see decodeJsonMsg()} is called
     *
     * @param inBuffer buffer that holds the JSON that contains one message or an array of messages to be transformed to RWF format
     * @param options options to be applied during parsing
     * @param error in case of parsing FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful parsing, CodecReturnCodes.FAILURE otherwise
     */
    int parseJsonBuffer(Buffer inBuffer, ParseJsonOptions options, JsonConverterError error);

    /**
     * parses the JSON contained in the TransportBuffer provided and stores the parsed JSON internally until {@see decodeJsonMsg()} method is called;
     * in case two subsequent calls to this method are performed without calling {@see decodeJsonMsg()} between them,
     * messages obtained after the last call to parseJsonMsg will be stored internally and retrieved when {@see decodeJsonMsg()} is called
     *
     * @param inBuffer transport buffer that holds the JSON that contains one message or an array of messages to be transformed to RWF format
     * @param options options to be applied during parsing
     * @param error in case of parsing FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful parsing, CodecReturnCodes.FAILURE otherwise
     */
    int parseJsonBuffer(TransportBuffer inBuffer, ParseJsonOptions options, JsonConverterError error);

    /**
     * transforms the parsed JSON message to RWF format and encodes it to the provided outRwfBuffer;
     * in case the input JSON parsed by {@see parseJsonBuffer()} method contained an array of messages,
     * transforms the next available message and encodes it into the supplied outRwfBuffer
     *
     * @param jsonMsg JSON message object to which the incoming buffer data is decoded and output RWF message is encoded
     * @param options options applied during decoding
     * @param error in case of decoding FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful JSON -> RWF transformation when there are more messages to be transformed
     * (if an array of messages was supplied to {@see parseJsonBuffer()} method),
     * CodecReturnCodes.END_OF_CONTAINER in case all messages were transformed successfully and there are no more messages waiting for the conversion,
     * CodecReturnCodes.FAILURE in case transformation of the current message failed
     */
    int decodeJsonMsg(JsonMsg jsonMsg, DecodeJsonMsgOptions options, JsonConverterError error);


    /**
     * converts the inMsg supplied to JSON format and stores the converted message internally until {@see getJsonBuffer()} method is called
     *
     * @param inMsg message in RWF format to be converted to JSON format
     * @param options options that are going to be used during transformation
     * @param outResults the output parameter that, in case supplied not null, will hold the length of the output JSON message
     * @param error in case of conversion FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful conversion, CodecReturnCodes.FAILURE otherwise
     */
    int convertRWFToJson(Msg inMsg, RWFToJsonOptions options, ConversionResults outResults, JsonConverterError error);

    /**
     * converts the inMsg supplied to JSON format and stores the converted message internally until {@see getJsonBuffer()} method is called
     *
     * @param inMsg message in RWF format to be converted to JSON format
     * @param options options that are going to be used during transformation
     * @param error in case of conversion FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful conversion, CodecReturnCodes.FAILURE otherwise
     */
    int convertRWFToJson(Msg inMsg, RWFToJsonOptions options, JsonConverterError error);

    /**
     * fetches the converted JSON message that is currently stored internally by the converter after the call to {@see convertRWFToJson()} method
     *
     * @param buffer Buffer that holds the output Json message
     * @param options options used to fetch the JSON message; in case the streamID provided in the original message should be replaced by another
     *                value, {@see GetJsonMsgOptions.streamId} should be set to this other value, in case this field is left null, the streamId
     *                in the output JSON message will not be altered
     * @param error in case of conversion FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful conversion, CodecReturnCodes.FAILURE otherwise
     */
    int getJsonBuffer(Buffer buffer, GetJsonMsgOptions options, JsonConverterError error);

    /**
     * fetches the converted JSON message that is currently stored internally by the converter after the call to {@see convertRWFToJson()} method
     *
     * @param buffer TransportBuffer that holds the output Json message
     * @param options options used to fetch the JSON message
     * @param error in case of conversion FAILURE carries information about error that has occurred
     * @return CodecReturnCodes.SUCCESS in case of successful conversion, CodecReturnCodes.FAILURE otherwise
     */
    int getJsonBuffer(TransportBuffer buffer, GetJsonMsgOptions options, JsonConverterError error);

    /**
     * in case of failed JSON -> RWF conversion, prepares an error message based on the original JSON message received
     * and error information obtained from the call to {@see decodeJsonMsg}
     *
     * @param outBuffer the Buffer that contains the resulting error message
     * @param params parameters used to prepare JSON error message
     * @param error carries error information in case of failure during JSON error message creation}
     * @return CodecReturnCodes.SUCCESS in case of successful error message creation and when the conversion on the previous step failed,
     * or CodecReturnCodes.FAILURE on case the conversion on the previous step was successful of the creation of the error message failed
     */
    int getErrorMessage(Buffer outBuffer, GetJsonErrorParams params, JsonConverterError error);
}
