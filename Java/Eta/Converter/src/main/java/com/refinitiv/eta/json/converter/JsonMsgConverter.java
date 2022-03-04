/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_TYPE;

class JsonMsgConverter extends AbstractContainerTypeConverter {

    JsonMsgConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.MSG };
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        return CodecReturnCodes.FAILURE;
    }
    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {
        return false;
    }
    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        return FAILURE;
    }
    @Override
    protected boolean writeEntry(DecodeIterator decIterator, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {
        return false;
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, JsonBuffer outBuffer, boolean writeTag, Object setDb, JsonConverterError error) {

        Msg message = JsonFactory.createMsg();
        try {
            message.clear();
            if (writeTag) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_MESSAGE, outBuffer, false, error);
            }
            int ret = message.decode(decIter);
            if (ret < SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding Message, code = " + ret);
                return false;
            }
            return converter.processMsg(decIter, message, outBuffer, error, false);
        } finally {
            JsonFactory.releaseMsg(message);
        }
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        int msgType = MsgClasses.REQUEST;
        JsonNode msgTypeNode = dataNode.path(JSON_TYPE);
        if (msgTypeNode!=null && !msgTypeNode.isMissingNode()){
            msgType = converter.getRwfMsgTypeFromJson(msgTypeNode, error);
            if (error.isFailed())
                return;
        }

        Msg msg = null;
        try {
            msg = JsonFactory.createMsg();
            converter.decodeRsslMessage(msgType, dataNode, msg, error, iter);
        } finally {
            if (msg != null)
                JsonFactory.releaseMsg(msg);
        }
    }


}
