/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import org.junit.Test;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.Objects;

import static com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class ArrayDecodingTest {

    @Test
    public void testDecodingArrayOfMessages() throws UnsupportedEncodingException {

        JsonConverterError convError = ConverterFactory.createJsonConverterError();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .build(convError);

        String input = "[{\"ID\":5,\"Type\":\"Request\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"}}, " +
                "[{\"ID\":5,\"Type\":\"Request\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"}}, {\"ID\":5,\"Type\":\"Request\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"}}], " +
                "[{\"ID\":5,\"Type\":\"Request\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"}}]]";

        JsonMsg jsonMsg = ConverterFactory.createJsonMsg();

        Buffer name = CodecFactory.createBuffer();
        name.data("TINY");

        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.wrap(input.getBytes("UTF-8")));

        ParseJsonOptions parseOptions = ConverterFactory.createParseJsonOptions();
        parseOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        DecodeJsonMsgOptions decodeOptions = ConverterFactory.createDecodeJsonMsgOptions();
        decodeOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

        Buffer outBuffer = CodecFactory.createBuffer();
        outBuffer.data(ByteBuffer.allocate(500));

        int ret = converter.parseJsonBuffer(buffer, parseOptions, convError);
        assertEquals(SUCCESS, ret);

        int i = 0;
        while ((ret = converter.decodeJsonMsg(jsonMsg, decodeOptions, convError)) != END_OF_CONTAINER) {
            i++;
            assertEquals(ret, SUCCESS);

            assertTrue(jsonMsg.rwfMsg().msgClass() == MsgClasses.REQUEST);
            assertEquals(5, jsonMsg.rwfMsg().streamId());

            MsgKey key = jsonMsg.rwfMsg().msgKey();
            assertTrue(Objects.nonNull(key));
            assertEquals(key.serviceId(), 555);
            assertTrue(key.name().equals(name));

            outBuffer.clear();
            outBuffer.data(ByteBuffer.allocate(500));
        }

        assertEquals(4, i);  /* the original input string contained 4 messages in total */
    }
}
