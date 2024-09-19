/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Map.Entry;

import static com.refinitiv.eta.codec.CodecReturnCodes.BLANK_DATA;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class EnumeratedValuesTests 
{
	JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();
    DataDictionary dictionary = CodecFactory.createDataDictionary();
    
    private static final byte[] UPTICK = new byte[] {-30, -121, -89};
    private static final byte[] DOWNTICK = new byte[] {-30, -121, -87};
    private static final byte[] KG53ANDHALF = new byte[] {75, 103, 53, 51, -62, -67};
	
	@Before
    public void init() {
        final String fieldDictionaryFile = "../../etc/RDMFieldDictionary";
        final String enumTypeFile = "../../etc/enumtype.def";
        
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        assertTrue(error.text(), dictionary.loadFieldDictionary(fieldDictionaryFile, error) == CodecReturnCodes.SUCCESS);
        assertTrue(error.text(), dictionary.loadEnumTypeDictionary(enumTypeFile, error) == CodecReturnCodes.SUCCESS);

        convError = ConverterFactory.createJsonConverterError();
    }
	
	@Test
    public void testExpanedEnumValues() throws IOException {
		
		converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, true)
                .setDictionary(dictionary)
                .build(convError);
		
        DecodeIterator decIter = generateFieldListDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[1000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        
        JsonNode root = mapper.readTree(outBuffer.data);
        
        checkJsonData(root);
 
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer, false);
    }
	
	@Test
    public void testExpanedEnumValues_Disallow_EnumDisplay() throws IOException {
		
		converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, false)
                .setDictionary(dictionary)
                .build(convError);
		
        DecodeIterator decIter = generateFieldListDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[1000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        
        JsonNode root = mapper.readTree(outBuffer.data);
        
        checkJsonData(root);
 
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer, true);
    }
	
	 private DecodeIterator generateFieldListDecodeIterator() {
	        DecodeIterator decIter = CodecFactory.createDecodeIterator();
	        EncodeIterator encIter = CodecFactory.createEncodeIterator();
	        Buffer buffer = CodecFactory.createBuffer();
	        buffer.data(ByteBuffer.allocate(20000));
	        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

	        FieldList fieldList = CodecFactory.createFieldList();
	        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
	        Enum enumValue = CodecFactory.createEnum();

	        fieldList.applyHasStandardData();
	        fieldList.encodeInit(encIter, null, 0);
	        
	        // Start encoding Field 
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(4);
	        enumValue.value(255);
	        fieldEntry.encode(encIter, enumValue);
	        
	        fieldEntry.clear();
	        enumValue.clear();
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(14);
	        enumValue.value(1);
	        fieldEntry.encode(encIter, enumValue);
	        
	        fieldEntry.clear();
	        enumValue.clear();
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(12762);
	        enumValue.value(2);
	        fieldEntry.encode(encIter, enumValue);
	        
	        fieldEntry.clear();
	        enumValue.clear();
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(54);
	        enumValue.value(70);
	        fieldEntry.encode(encIter, enumValue);
	        
	        // Test with the same enum value
	        fieldEntry.clear();
	        enumValue.clear();
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(3111);
	        enumValue.value(1);
	        fieldEntry.encode(encIter, enumValue);
	        
	        fieldEntry.clear();
	        enumValue.clear();
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(3112);
	        enumValue.value(2);
	        fieldEntry.encode(encIter, enumValue);
	        
	        // Test with non existing enum value
	        fieldEntry.clear();
	        enumValue.clear();
	        fieldEntry.dataType(DataTypes.ENUM);
	        fieldEntry.fieldId(3113);
	        enumValue.value(999);
	        fieldEntry.encode(encIter, enumValue);
	        
	        fieldList.encodeComplete(encIter, true);

	        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
	        return decIter;
	    }
	 
	 private void checkJsonData(JsonNode jsonNode) throws UnsupportedEncodingException
	 {
		int index = 0;
		
		Iterator<Entry<String,JsonNode>> it = jsonNode.fields();
		
		while(it.hasNext())
		{
			Entry<String,JsonNode> entry = it.next();
			++index;
			
			switch(index)
			{
			case 1:
				assertEquals("RDN_EXCHID", entry.getKey());
				assertTrue(entry.getValue().isTextual());
				assertEquals("CFS", entry.getValue().asText());
				break;
			case 2:
				assertEquals("PRCTCK_1", entry.getKey());
				assertTrue(entry.getValue().isTextual());
				assertTrue(Arrays.equals(UPTICK, entry.getValue().asText().getBytes("UTF-8")));
				break;
			case 3:
				assertEquals("BAS_PRCTCK", entry.getKey());
				assertTrue(entry.getValue().isTextual());
				assertTrue(Arrays.equals(DOWNTICK, entry.getValue().asText().getBytes("UTF-8")));
				break;
			case 4:
				assertEquals("LOTSZUNITS", entry.getKey());
				assertTrue(entry.getValue().isTextual());
				assertTrue(Arrays.equals(KG53ANDHALF, entry.getValue().asText().getBytes("UTF-8")));
				break;
			case 5:
				assertEquals("TICK_1", entry.getKey());
				assertTrue(entry.getValue().isTextual());
				assertTrue(Arrays.equals(UPTICK, entry.getValue().asText().getBytes("UTF-8")));
				break;
			case 6:
				assertEquals("TICK_2", entry.getKey());
				assertTrue(entry.getValue().isTextual());
				assertTrue(Arrays.equals(DOWNTICK, entry.getValue().asText().getBytes("UTF-8")));
				break;
			case 7:
				assertEquals("TICK_3", entry.getKey());
				assertTrue(entry.getValue().isInt());
				assertEquals(999, entry.getValue().asInt());
				break;
			}
		}
	 }
	 
	 private void checkRWFData(Buffer encodedBuffer, boolean disallowDisplayValue)
	 {
		 DecodeIterator decIter = CodecFactory.createDecodeIterator();
		 decIter.setBufferAndRWFVersion(encodedBuffer, Codec.majorVersion(), Codec.minorVersion());
		 
		 FieldList fieldList = CodecFactory.createFieldList();
		 FieldEntry fieldEntry = CodecFactory.createFieldEntry();
		 Enum enumValue = CodecFactory.createEnum();
		 
		 int ret = fieldList.decode(decIter, null);
		 assertEquals(SUCCESS, ret);
		 
	     ret = fieldEntry.decode(decIter);
	     assertEquals(SUCCESS, ret);
	     
	     /* Check whether the convert allow display value for the enum type. */
	     if(disallowDisplayValue == false)
	     {
		     assertEquals(4,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(255, enumValue.toInt());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(14,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(1, enumValue.toInt());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(12762,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(2, enumValue.toInt());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(54,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(70, enumValue.toInt());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(3111,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(1, enumValue.toInt());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(3112,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(2, enumValue.toInt());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(3113,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(999, enumValue.toInt());
	     }
	     else
	     {
	    	 assertEquals(4,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(BLANK_DATA, ret);
		     assertTrue(enumValue.isBlank());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(14,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(BLANK_DATA, ret);
		     assertTrue(enumValue.isBlank());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(12762,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(BLANK_DATA, ret);
		     assertTrue(enumValue.isBlank());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(54,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(BLANK_DATA, ret);
		     assertTrue(enumValue.isBlank());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(3111,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(BLANK_DATA, ret);
		     assertTrue(enumValue.isBlank());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(3112,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(BLANK_DATA, ret);
		     assertTrue(enumValue.isBlank());
		     
		     fieldEntry.clear();
		     enumValue.clear();
		     ret = fieldEntry.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(3113,fieldEntry.fieldId());    
		     ret = enumValue.decode(decIter);
		     assertEquals(SUCCESS, ret);
		     assertEquals(999, enumValue.toInt());
	     }
	 }
}
