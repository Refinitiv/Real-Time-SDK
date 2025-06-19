/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.PostUserInfo;

import java.util.Iterator;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonPostUserInfoConverter extends AbstractRsslMessageChunkTypeConverter {
    JsonPostUserInfoConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        checkObject(node, JSON_POSTUSERINFO, error);
        if (error.isFailed())
            return;

        boolean hasAddress = false;
        boolean hasUserId = false;

        PostUserInfo postUserInfo = (PostUserInfo) msg;
        for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
            String key = it.next();
            JsonNode currentNode = node.path(key);
            int result = SUCCESS;
            switch (key) {
                case JSON_ADDRESS:
                	if(currentNode.isTextual())
                	{
                		String userAddr = getText(currentNode, key, error);
                		if (error.isFailed())
                			break;

                		checkIpV4Address(userAddr, error);
                		if (error.isFailed())
                			break;

                		postUserInfo.userAddr(userAddr);
                		hasAddress = true;
                	}
                	else if (currentNode.isInt())
                	{
                		postUserInfo.userAddr(currentNode.intValue());
                		hasAddress = true;
                	}
                	else if (currentNode.isLong())
                	{
                		postUserInfo.userAddr(currentNode.longValue());
                		hasAddress = true;
                	}
                	else
                	{
                		error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + key + " was not expected type:" + currentNode.getNodeType());
                	}
                    break;

                case JSON_USERID:
                	if(currentNode.isTextual())
                	{
                		postUserInfo.userId(currentNode.asLong());
                		hasUserId = true;
                	}
                	else if (currentNode.isInt())
                	{
                		postUserInfo.userId(currentNode.intValue());
                		hasUserId = true;
                	}
                	else if (currentNode.isLong())
                	{
                		postUserInfo.userId(currentNode.longValue());
                		hasUserId = true;
                	}
                	else
                	{
                		error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + key + " was not expected type:" + currentNode.getNodeType());
                	}
                    break;

                default:
                    processUnexpectedKey(key, error);
            }

            if (error.isFailed())
                return;

            if (result != SUCCESS)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, key + "=" + currentNode.asText());
        }

        if (!hasAddress)
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_ADDRESS);

        if (!hasUserId)
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_USERID);

    }

    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {

        PostUserInfo userInfo = (PostUserInfo) type;
        BufferHelper.beginObject(outBuffer, error);
        BufferHelper.writeArrayAndColon(JSON_ADDRESS, outBuffer, false, error);
        writeUserAddress(userInfo.userAddr(), outBuffer, error);
        BufferHelper.writeArrayAndColon(JSON_USERID, outBuffer, true, error);
        BasicPrimitiveConverter.writeLong(userInfo.userId(), outBuffer, error);
        BufferHelper.endObject(outBuffer, error);
        return error.isSuccessful();
    }

    private static boolean writeUserAddress(long addr, JsonBuffer outBuffer, JsonConverterError error) {

        BufferHelper.doubleQuote(outBuffer, error);
        BasicPrimitiveConverter.writeLong((addr >> 24) & 0xFF, outBuffer, error);
        BufferHelper.writeAsciiChar(outBuffer, '.', error);
        BasicPrimitiveConverter.writeLong((addr >> 16) & 0xFF, outBuffer, error);
        BufferHelper.writeAsciiChar(outBuffer, '.', error);
        BasicPrimitiveConverter.writeLong((addr >> 8) & 0xFF, outBuffer, error);
        BufferHelper.writeAsciiChar(outBuffer, '.', error);
        BasicPrimitiveConverter.writeLong(addr & 0xFF, outBuffer, error);
        BufferHelper.doubleQuote(outBuffer, error);
        return error.isSuccessful();
    }
}
