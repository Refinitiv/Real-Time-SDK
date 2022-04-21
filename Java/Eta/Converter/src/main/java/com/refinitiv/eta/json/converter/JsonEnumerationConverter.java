/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE;

class JsonEnumerationConverter extends AbstractPrimitiveTypeConverter {

    JsonEnumerationConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes  = new int[] { DataTypes.ENUM };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createEnum();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseEnum((Enum) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Enum enumeration = (Enum) type;
        enumeration.clear();
        return enumeration.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeLong(((Enum) type).toInt(), outBuffer, error);
    }

    boolean encodeJson(DecodeIterator decIter, DictionaryEntry entry, JsonBuffer outBuffer, JsonConverterError error) {

        Enum enumer = JsonFactory.createEnum();
        try {

            enumer.clear();
            int ret = enumer.decode(decIter);

            if (ret < CodecReturnCodes.SUCCESS){
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, null);
                return false;
            }
            boolean res;
            if (ret == CodecReturnCodes.BLANK_DATA)
                res = BufferHelper.writeArray(ConstCharArrays.nullBytes, outBuffer, false, error);
            else {
                EnumType enumType = (entry.enumTypeTable() != null
                                     && enumer.toInt() <= entry.enumTypeTable().maxValue()) ? entry.enumTypeTable().enumTypes()[enumer.toInt()] : null;
                if (enumType != null && enumType.display().data() != null) {
                    res = BasicPrimitiveConverter.writeRMTESString(enumType.display(), outBuffer, error);
                } else {
                    res = BasicPrimitiveConverter.writeLong(enumer.toInt(), outBuffer, error);
                }
            }
            if (!res)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_OUT_OF_MEMORY, null);

            return res;

        } catch (Exception e) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR, e.getMessage());
            return false;
        } finally {
            JsonFactory.releaseEnum(enumer);
        }
    }
    
    private int convertDisplayToEnum(JsonNode dataNode, JsonConverterError error)
    {
		DictionaryEntry dictEntry = converter.dictionaryEntry();
		EnumTableDefinition enumTableDefinition = converter.getEnumTableDefinition(dictEntry.enumTypeTable());
		int enumValue = -1;
		
		enumValue = enumTableDefinition.findEnumDefinition(dataNode.textValue());
		
		if(enumValue == -1)
		{
			enumValue = enumTableDefinition.addEnumDefinition(dictEntry.enumTypeTable(), dataNode.textValue(), error);
		}
	
		return enumValue; 
    }

    @Override
    protected void decodeJson(JsonNode dataNode, Object msg, JsonConverterError error) {
        Enum enumer = (Enum) msg;
        
        if(dataNode.isTextual() && converter.allowEnumDisplayStrings())
    	{	
        	/* Converts the display value to enum value */
        	int enumValue = convertDisplayToEnum(dataNode, error);
        	
        	if(enumValue != -1)
        	{
        		enumer.value(enumValue);
        	}
        	else
        	{
        		/* Indicates blank value to the caller with the enumer.isBlank() call */
            	enumer.value("");
        	}
    	}
        else if (dataNode.isInt())
        {
        	enumer.value(dataNode.intValue());
        }
        else
        {
        	/* Indicates blank value to the caller with the enumer.isBlank() call */
        	enumer.value("");
        }
    }
    
    void encodeRWF(int enumValue, String key, EncodeIterator iter, JsonConverterError error) {
 	   Enum encIntValue = JsonFactory.createEnum();
        try {
            encIntValue.clear();
            int result = encIntValue.value(enumValue);
            if (result != SUCCESS) {
                error.setError(JSON_ERROR_UNEXPECTED_VALUE, "Received '" + enumValue + "'");
                return;
            }
            result = encIntValue.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseEnum(encIntValue);
        }
     }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {

    	if(dataNode.isTextual() && converter.allowEnumDisplayStrings())
    	{	/* Converts the display value to enum value */
    		
    		int enumValue = convertDisplayToEnum(dataNode, error);
    		
    		if(enumValue != -1)
    		{
    			encodeRWF(enumValue, key, iter, error);
    		}
    		else
    		{
    			/* Do nothing to encode a blank value */
    		}
    	}
    	else if (dataNode.isNumber())
    	{
    		encodeRWF(dataNode.intValue(), key, iter, error);
    	}
    	else
    	{
    		/* Do nothing to encode a blank value */
    	}
    }
}
