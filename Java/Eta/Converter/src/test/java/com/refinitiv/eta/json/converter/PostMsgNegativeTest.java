/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import org.junit.Before;
import org.junit.Test;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;

public class PostMsgNegativeTest {
    PostMsg postMsg = (PostMsg) CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    int streamId = MsgClasses.POST;

    @Before
    public void init() {
        initMsg();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[]{JsonConverter.class},
                proxy);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        conversionResults.setLength(0);
    }

    private void initMsg() {
        postMsg.clear();
        postMsg.streamId(streamId);
        postMsg.domainType(DomainTypes.MARKET_PRICE);
        postMsg.msgClass(streamId);
        postMsg.encodedDataBody(CodecFactory.createBuffer());
        postMsg.encodedDataBody().data(ByteBuffer.allocate(0));
    }

    @Test
    //Message returns INCOMPLETE DATA when we write data without MSG, therefore it will return FAILURE when msg has not been encoded.
    public void givenEmptyMsgContainer_whenEncodeToJsonPost_thenConvertingFails() {
        postMsg.containerType(DataTypes.MSG);
        assertEquals(FAILURE, converter.convertRWFToJson(postMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    //We have success here because we write empty object when list has not been encoded.
    public void givenEmptyFieldListContainer_whenEncodeToJsonPost_thenConvertingSucceeds() {
        postMsg.containerType(DataTypes.FIELD_LIST);
        assertEquals(SUCCESS, converter.convertRWFToJson(postMsg, rwfToJsonOptions, conversionResults, convError));
    }
}
