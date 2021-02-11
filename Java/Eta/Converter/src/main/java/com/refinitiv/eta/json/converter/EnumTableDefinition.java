package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.EnumType;
import com.refinitiv.eta.codec.EnumTypeTable;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.locks.ReentrantLock;

class EnumTableDefinition {
	
	private Map<String, Integer> enumByDispalyValueHash;
	private JsonBuffer outputBuffer;
	private ByteBuffer byteBuffer;
	private ReentrantLock accessLock = new java.util.concurrent.locks.ReentrantLock();
	
	EnumTableDefinition(int maxEnumValue)
	{
		enumByDispalyValueHash = new HashMap<>(maxEnumValue);
		byteBuffer = ByteBuffer.allocate(256);

		outputBuffer = new JsonBuffer();
		outputBuffer.position = 0;
		outputBuffer.data = byteBuffer.array();
	}
	
	int findEnumDefinition(String displayValue)
	{
		accessLock.lock();
		
		try
		{
			Integer enumValue = enumByDispalyValueHash.get(displayValue);
		
			if(Objects.nonNull(enumValue))
			{
				return enumValue.intValue();
			}
			else
			{
				return -1;
			}
		}
		finally
		{
			accessLock.unlock();
		}
	}
	
	static boolean compareDisplayValue(byte[] displayByteArray, byte[] outputByteArray)
	{
		for(int index = 0; index < displayByteArray.length; index++)
		{
			if(displayByteArray[index] != outputByteArray[index+1])
			{
				return false;
			}
		}
		
		return true;
	}
	
	int addEnumDefinition(EnumTypeTable enumTypeTable, String displayValue, JsonConverterError error)
	{
		int enumValue = -1; /* Indicates not found */
		EnumType enumType;
		boolean success = true;
		byte[] displayByteArray;
		
		accessLock.lock();
		
		try
		{
			try {
				displayByteArray = displayValue.getBytes("UTF-8");
			} catch (UnsupportedEncodingException e) {
				return -1; /* Failed to encode the display value. */
			}
			
			for(int index = 0; index <= enumTypeTable.maxValue(); index++)
			{
				enumType = enumTypeTable.enumTypes()[index];

				/* Continue to next index when the EnumType is not available */
				if(Objects.isNull(enumType))
				{
					continue;
				}
				
				/* Clears the position of the JsonBuffer*/
				outputBuffer.position = 0;
				
				success = BasicPrimitiveConverter.writeRMTESString(enumType.display(), outputBuffer, error);
				
				if (!success)
				{
	                error.setError(JsonConverterErrorCodes.JSON_ERROR_OUT_OF_MEMORY, null);
	                return -1;
				}
				
				if( (displayByteArray.length + 2) == outputBuffer.position)
				{
					if (compareDisplayValue(displayByteArray, outputBuffer.data))
					{
						enumValue = enumType.value();
						enumByDispalyValueHash.put(displayValue, Integer.valueOf(enumValue));
						return enumValue;
					}
				}
				
			}
			
			return enumValue;
		}
		finally
		{
			accessLock.unlock();
		}
	}
}
