/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.Login;

import java.util.Iterator;

import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonMsgKeyConverter extends AbstractRsslMessageChunkTypeConverter {

    JsonMsgKeyConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msgObj, JsonConverterError error) {
        Msg msg = (RequestMsg) msgObj;
        MsgKey msgKey = msg.msgKey();
        msgKey.nameType(InstrumentNameTypes.RIC);
        msgKey.applyHasNameType();


        for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
            String key = it.next();
            JsonNode currentNode = node.path(key);
            switch(key) {
                case JSON_NAME:
                    if (currentNode.isTextual()) {
                        msgKey.applyHasName();
                        msgKey.name().data(getText(currentNode, key, error));
                    } else if (currentNode.isArray() && msg instanceof RequestMsg) {
                        // The key contains an array of item names
                        // would be encoded with the message payload (body), set data type for now
                        ((RequestMsg) msg).applyHasBatch();
                        msg.containerType(DataTypes.ELEMENT_LIST);
                    } else {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, key + " TEXT or ARRAY type was expected", JSON_NAME);
                    }

                    break;
                case JSON_KEY_NAME_TYPE:
                    checkStringOrInt(currentNode, key, error);
                    if (error.isFailed())
                        break;

                    msgKey.applyHasNameType();
                    if (currentNode.isTextual())
                        msgKey.nameType(ConstCharArrays.JsonNameType.ofValue(getText(currentNode, key, error), error));
                    else
                        msgKey.nameType(getInt(currentNode, key, error));

                    break;

                case JSON_ELEMENTS:
                    msgKey.applyHasAttrib();
                    msgKey.attribContainerType(DataTypes.ELEMENT_LIST);
                    break;

                case JSON_KEY_FILTER:
                    msgKey.applyHasFilter();
                    if (currentNode.isInt())
                        msgKey.filter(currentNode.asInt());
                    else if (currentNode.isLong())
                        msgKey.filter(currentNode.asLong());
                    else
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "expected Long type for Filter, found " + currentNode.getNodeType().toString(), JSON_KEY_FILTER);
                    break;

                case JSON_KEY_IDENTIFIER:
                    msgKey.applyHasIdentifier();
                    msgKey.identifier(getInt(currentNode, key, error));
                    break;

                case JSON_KEY_SERVICE:
                    checkStringOrInt(currentNode, key, error);
                    if (error.isFailed())
                        break;

                    int serviceId = -1; // indicate unknown service ID
                    if (currentNode.isTextual())
                    {
                    	serviceId = converter.serviceNameToId(getText(currentNode, key, error), error);
                    	
                    	 if(serviceId == -1)
                    	 {
                    		if(!error.isFailed()) // Checks whether a failure is set.
                    		{
                    			error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "token: " + key + " , failed to find appropriate service ID");
                    		}
                    		
                    		return;
                    	 }
                    }
                    else
                    {
                    	serviceId = getInt(currentNode, key, error);
                    }
                   
                    // Valid service ID range is between 0 and 65535
                    if(serviceId < 0)
                    {
                	error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "token: " + key + " , invalid service ID " + serviceId);
                	return;
                    }
                    
                    msgKey.serviceId(serviceId);
               	    msgKey.applyHasServiceId();
                    
                    break;
                default:
                    processUnexpectedKey(key, error);
            }

            if (error.isFailed())
                return;

        }
        if ((msgKey.nameType() == Login.UserIdTypes.COOKIE || msgKey.nameType() == 5) && !msgKey.checkHasName()) {
            msgKey.applyHasName();
            msgKey.name().data(blankStringConst);
        }
        if (!msgKey.checkHasServiceId() && converter.hasDefaultServiceId() && (msg.domainType() != DomainTypes.SOURCE) && (msg.domainType() != DomainTypes.LOGIN)) {
            msgKey.applyHasServiceId();
            msgKey.serviceId(converter.getDefaultServiceId());
        }
    }


    boolean encodeJson(DecodeIterator decIter, Object msgKey, JsonBuffer outBuffer, int domain, boolean wantServiceName, JsonConverterError error) {

        boolean comma = false;
        MsgKey key = (MsgKey) msgKey;

        if (key.flags() == MsgKeyFlags.NONE)
            return BufferHelper.beginObject(outBuffer, error) && BufferHelper.endObject(outBuffer, error);

        BufferHelper.beginObject(outBuffer, error);
        if (key.checkHasServiceId()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY_SERVICE, outBuffer, false, error);
            if (wantServiceName) {
                String serviceName = converter.getServiceNameIdConverter().serviceIdToName(key.serviceId(), error);
                if (serviceName != null)
                    BufferHelper.writeArray(serviceName, outBuffer, true, error);
                else
                    BasicPrimitiveConverter.writeLong(key.serviceId(), outBuffer, error);
            } else
                BasicPrimitiveConverter.writeLong(key.serviceId(), outBuffer, error);

            comma = true;
        }

        if (key.checkHasNameType() && key.nameType() != 1) { //1 is the default value, we don't write it
            BufferHelper.writeArrayAndColon(JSON_KEY_NAME_TYPE, outBuffer, comma, error);
            String nameType;
            if (domain == DomainTypes.LOGIN)
                nameType = getLoginUserIdType(key.nameType());
            else
                nameType = getInstrumentNameType(key.nameType());
            if (nameType != null)
                BufferHelper.writeArray(nameType, outBuffer, true, error);
            else
                BasicPrimitiveConverter.writeLong(key.nameType(), outBuffer, error);
            comma = true;
        }

        if (key.checkHasName()) {
            if (!(key.nameType() == Login.UserIdTypes.COOKIE || key.nameType() == 5)) { //5 is the default value, we don't write it
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_NAME, outBuffer, comma, error);
                if (key.name().length() == 0 || (key.name().length() == 1 && (key.name().data() == null || key.name().equals(emptyBuffer))))
                    BufferHelper.writeArray(ConstCharArrays.nullString, outBuffer, false, error);
                else
                    BasicPrimitiveConverter.writeAsciiString(key.name(), outBuffer, error);
                comma = true;
            }
        }

        if (key.checkHasFilter()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY_FILTER, outBuffer, comma, error);
            BasicPrimitiveConverter.writeLong(key.filter(), outBuffer, error);
            comma = true;
        }

        if (key.checkHasIdentifier()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY_IDENTIFIER, outBuffer, comma, error);
            BasicPrimitiveConverter.writeLong(key.identifier(), outBuffer, error);
            comma = true;
        }

        if (key.checkHasAttrib()) {
            DecodeIterator iter = JsonFactory.createDecodeIterator();
            try {
                iter.clear();
                iter.setBufferAndRWFVersion(key.encodedAttrib(), Codec.majorVersion(), Codec.minorVersion());
                if (comma)
                    BufferHelper.comma(outBuffer, error);
                if (!converter.getContainerHandler(key.attribContainerType()).encodeJson(iter, outBuffer, true, null, error))
                    return false;
            } finally {
                JsonFactory.releaseDecodeIterator(iter);
            }
        }
        BufferHelper.endObject(outBuffer, error);
        return error.isSuccessful();
    }

    private static String getLoginUserIdType(int nameType) {
        switch (nameType) {
            case Login.UserIdTypes.NAME:
                return ConstCharArrays.JSON_NAME;
            case Login.UserIdTypes.EMAIL_ADDRESS:
                return ConstCharArrays.LOGIN_USER_EMAIL_ADDRESS;
            case Login.UserIdTypes.TOKEN:
                return ConstCharArrays.LOGIN_USER_TOKEN;
            case Login.UserIdTypes.COOKIE:
                return ConstCharArrays.LOGIN_USER_COOKIE;
            case Login.UserIdTypes.AUTHN_TOKEN:
                return ConstCharArrays.LOGIN_USER_AUTHENTICATION_TOKEN;
            default:
                return null;
        }
    }

    private static String getInstrumentNameType(int nameType) {
        switch (nameType) {
            case InstrumentNameTypes.RIC:
                return ConstCharArrays.NAME_TYPE_STR_RIC;
            case InstrumentNameTypes.CONTRIBUTOR:
                return ConstCharArrays.NAME_TYPE_STR_CONTRIBUTOR;
            case InstrumentNameTypes.UNSPECIFIED:
                return ConstCharArrays.NAME_TYPE_STR_UNSPECIFIED;
            default:
                return null;
        }
    }
}
