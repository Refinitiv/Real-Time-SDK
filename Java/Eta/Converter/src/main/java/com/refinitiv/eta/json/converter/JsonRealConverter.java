/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import java.lang.Math;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonRealConverter extends AbstractPrimitiveTypeConverter {
	
	static int[] _posExponentTable = {14,15,16,17,18,19,20,21};
	static int[] _negExponentTable = {0,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
	
	final static int POS_EXP_MIN = 0;
	final static int POS_EXP_MAX = 7;
	final static int NEG_EXP_MIN = 1;
	final static int NEG_EXP_MAX = 14;

    JsonRealConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] {DataTypes.REAL, DataTypes.REAL_4RB, DataTypes.REAL_8RB };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createReal();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseReal((Real) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Real real = (Real) type;
        real.clear();
        return real.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeReal((Real) type, outBuffer, false, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {

        Real real = (Real) msg;
        real.clear();
        if (node.isNull()) {
            real.blank();
            return;
        }
        
        int result = SUCCESS;
        
        if (node.isDouble()) {
        	result = processReal(node.asText(), real);
        } else if (node.isInt()) {
        	result = real.value(node.asInt(), RealHints.EXPONENT0);
        } else if (node.isLong()) {
        	result = real.value(node.asLong(), RealHints.EXPONENT0);
        } else if (node.isFloat()) {
        	result = processReal(node.asText(), real);
        }
        else if (node.isTextual()) {
            switch (node.textValue()) {
                case jsonDoublePositiveInfinityStr:
                    result = real.value(java.lang.Double.POSITIVE_INFINITY, 1);
                    break;
                case jsonDoubleNegativeInfinityStr:
                    result = real.value(java.lang.Double.NEGATIVE_INFINITY, 1);
                    break;
                case jsonDoubleNanStr:
                    result = real.value(java.lang.Double.NaN, 1);
                    break;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "Invalid text '" + node.textValue() + "' expected Real");
                    return;
            }
        }
        
        if (result != SUCCESS) {
        	error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Failed to converter the JSON numeric '" + node.asText() + "' value to Real");
            return;
        }
    }
    
    static int processReal(String strValue, Real realValue)
    {
    	if(strValue.isEmpty())
    	{
    		realValue.isBlank();
    		return SUCCESS;
    	}
    	
    	int decimalIndex = -1;
    	int expoIndex = -1;
    	int result = SUCCESS;
    	long value;
    	int hint;
    	int expVal = 0;
    	int posExp = 0;
    	boolean foundNegative = false;
    	
    	for(int index = 0; index < strValue.length(); index++)
    	{
    		char ch = strValue.charAt(index);
    		
    		if(ch == '-')
    		{
    			if(expoIndex == -1)
    				foundNegative = true;
    		}
    		/* the decimal always be prior to the exponent */
    		else if(ch == '.'){
    			decimalIndex = index;
    		}
    		else if(ch == 'E' || ch == 'e') {
    			expoIndex = index;
    			break;
    		}
    	}
    	
    	try
    	{
	    	/* Found exponential */
	    	if(expoIndex != -1)
	    	{
	    		value = Long.parseLong(strValue.substring(0, decimalIndex));
	    	
	    		/* Found decimal */
	    		if(decimalIndex != -1)
	    		{
	    			long orgValue;
	    			
	    			orgValue = value;
	    			
	    			/* find out how many digits are between the . and the E. */
	    			posExp = expoIndex - decimalIndex - 1;
	    			value = value * (long)Math.pow(10, posExp);
	    			
	    			if(!foundNegative)
	    			{
	    				value += Long.parseLong(strValue.substring(decimalIndex + 1, expoIndex));
	    				
	    				/* Checks for overflow from the maximum positive value */
	    				if(value < orgValue)
	    				{
	    					return FAILURE;
	    				}
	    			}
	    			else
	    			{
	    				value -= Long.parseLong(strValue.substring(decimalIndex + 1, expoIndex));
	    				
	    				/* Checks for overflow from the maximum negative value */
	    				if(value > orgValue)
	    				{
	    					return FAILURE;
	    				}
	    			}
	    		}
	    		
	    		/* Checks negative exponential */
	    		if(strValue.charAt(expoIndex + 1) == '-')
	    		{
	    			expVal = Integer.parseInt(strValue.substring(expoIndex + 2, strValue.length()));
	    			
	    			expVal += posExp; 
	    			
	    			if(expVal >= NEG_EXP_MIN && expVal <= NEG_EXP_MAX)
	    			{
	    				hint = _negExponentTable[expVal];
	    			}
	    			else
	    			{
	    				return FAILURE;
	    			}
	    		}
	    		else
	    		{
	    			expVal = Integer.parseInt(strValue.substring(expoIndex + 1, strValue.length()));
	    		
	    			/* Found decimal */
	    			if(decimalIndex != -1)
	    			{
	    				if(posExp > expVal)
	    				{
	    					expVal = posExp - expVal;
	    					
	    					if(expVal >= NEG_EXP_MIN && expVal <= NEG_EXP_MAX)
	    					{
	    						hint =  _negExponentTable[expVal];
	    						return realValue.value(value, hint);
	    					}
	    					else
	    					{
	    						return FAILURE;
	    					}
	    				}
	    			}
	    			
	    			expVal -= posExp;
	    			
	    			if(expVal >= POS_EXP_MIN && expVal <= POS_EXP_MAX)
	    			{
	    				hint = _posExponentTable[expVal];
	    			}
	    			else
	    			{
	    				return FAILURE;
	    			}
	    		}
	    		
	    		result = realValue.value(value, hint);
	    	}
	    	else
	    	{
	    		result = realValue.value(strValue);
	    	}
    	}
    	catch(Exception exp)
    	{
    		return FAILURE;
    	}
    	
    	return result;
    }

    @Override
    void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {

        Real real = JsonFactory.createReal();
        real.clear();
        int result = SUCCESS;
      
        try {
            if (dataNode.isNull())
                real.blank();
            else if (dataNode.isDouble()) {
            	result = processReal(dataNode.asText(), real);
            } else if (dataNode.isInt()) {
            	result = real.value(dataNode.asInt(), RealHints.EXPONENT0);
            } else if (dataNode.isLong()) {
            	result = real.value(dataNode.asLong(), RealHints.EXPONENT0);
            } else if (dataNode.isBigInteger()) {
            	result = real.value(dataNode.bigIntegerValue().longValueExact(), RealHints.EXPONENT0);
            } else if (dataNode.isFloat()) {
            	result = processReal(dataNode.asText(), real);
            } else if (dataNode.isTextual()) {
                switch (dataNode.textValue()) {
                    case jsonDoublePositiveInfinityStr:
                    	result = real.value(java.lang.Double.POSITIVE_INFINITY, 1);
                        break;
                    case jsonDoubleNegativeInfinityStr:
                    	result = real.value(java.lang.Double.NEGATIVE_INFINITY, 1);
                        break;
                    case jsonDoubleNanStr:
                    	result = real.value(java.lang.Double.NaN, 1);
                        break;
                    default:
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "Invalid text '" + dataNode.textValue() + "' expected Real");
                        return;
                }
            }
            
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }

            result = real.encode(iter);

            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        }
        catch(Exception exep) {
        	error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Failed to converter the JSON numeric '" + dataNode.asText() + "' value to Real");
            return;
        }
        finally {
            JsonFactory.releaseReal(real);
        }
    }
}
