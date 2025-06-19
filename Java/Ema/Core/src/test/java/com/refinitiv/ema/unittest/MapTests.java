/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.unittest;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Iterator;

import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.EmaUtility;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.JUnitTestConnect;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmQos;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;

import junit.framework.TestCase;

public class MapTests extends TestCase
{

	public MapTests(String name)
	{
		super(name);
	}

	public void testMapEmpty_Encode()
	{
		TestUtilities.printTestHead("testMapEmpty_Encode","Encode Map of no entry with EMA");
		
		try {
			Map map = EmaFactory.createMap();
			FieldEntry fieldEntry = EmaFactory.createFieldEntry();
			
			fieldEntry.map(3, map);
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "empty Map - exception not expected: " +  excp.getMessage()  );
			return;
		}
		
		TestUtilities.checkResult( true, "empty Map - Can encode default Map with Buffer type for Map entry key" );
	}
	
	public void testMapWithSummaryDataButNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapWithSummaryDataButNoPayload_Encode_Decode","Encode Map with summary data but no entry with EMA");
		
		com.refinitiv.eta.codec.DataDictionary dataDictionary = TestUtilities.getDataDictionary();
		
		try
		{
			FieldList summaryData = EmaFactory.createFieldList();
			summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
			summaryData.add(EmaFactory.createFieldEntry().enumValue(15, 840));
			summaryData.add(EmaFactory.createFieldEntry().date(3386, 2018, 2, 28));
			
			Map map = EmaFactory.createMap();
			map.keyFieldId(11).totalCountHint(0).summaryData(summaryData);
			
			ElementList elementList = EmaFactory.createElementList();
			elementList.info(1).add(EmaFactory.createElementEntry().map("1", map));
			
			ElementList elementListDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), dataDictionary, null);

			
			Iterator<ElementEntry> elementListIt = elementListDec.iterator();
			
			ElementEntry elementEntry = elementListIt.next();
			
			Map mapDec = elementEntry.map();
			
			TestUtilities.checkResult( mapDec.keyFieldId() == 11, "Check key field ID of map" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 0, "Check totoal cound hint map" );
			
			FieldList summaryDataDec = mapDec.summaryData().fieldList();
			
			Iterator<FieldEntry> fieldIt = summaryDataDec.iterator();
			
			FieldEntry fieldEntry = fieldIt.next();
			
			TestUtilities.checkResult( fieldEntry.fieldId() == 1, "Check the field ID of the first field entry" );
			TestUtilities.checkResult( fieldEntry.uintValue() == 3056, "Check the value of the first field entry" );
			
			fieldEntry = fieldIt.next();
			
			TestUtilities.checkResult( fieldEntry.fieldId() == 15, "Check the field ID of the second field entry" );
			TestUtilities.checkResult( fieldEntry.enumValue() == 840, "Check the value of the second field entry" );
			
			fieldEntry = fieldIt.next();
			TestUtilities.checkResult( fieldEntry.fieldId() == 3386, "Check the field ID of the third field entry" );
			TestUtilities.checkResult( fieldEntry.date().year() == 2018, "Check the year value of the third field entry" );
			TestUtilities.checkResult( fieldEntry.date().month() == 2, "Check the month value of the third field entry" );
			TestUtilities.checkResult( fieldEntry.date().day() == 28, "Check the day value of the third field entry" );
			
			TestUtilities.checkResult( fieldIt.hasNext() == false, "Check whether this is an entry from FieldList");
			
			TestUtilities.checkResult( mapDec.iterator().hasNext() == false, "Check whether this is an entry from Map");
			
			TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether this is an entry from ElementList");
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode no entry map - exception not expected with text : " +  excp.getMessage()  );
		}
	}
	
	public void testMapSpecifyInvalidKeyType_Encode()
	{
		TestUtilities.printTestHead("testMapSpecifyInvalidKeyType_Encode","Encode Map with invalid key type with EMA");
		
		try
		{
			Map map = EmaFactory.createMap();
			map.keyType(DataTypes.FIELD_LIST);
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( true, "Fails to encode with invalid key type - exception expected with text : " +  excp.getMessage()  );
			TestUtilities.checkResult(excp.getMessage().equals("The specified key type 'FieldList' is not a primitive type"), "Check exception message text");
			return;
		}
		
		TestUtilities.checkResult( false, "Fails to encode with invalid key type - expect exception" );
	}
	
	public void testMapKeyTypeAndAddEntryKeyTypeMismatch_Encode()
	{
		TestUtilities.printTestHead("testMapKeyTypeAndAddEntryMismatch_Encode","Encode Map with mismatch key type");
		
		try
		{
			Map map = EmaFactory.createMap();
			map.keyFieldId(11).totalCountHint(3).keyType(DataType.DataTypes.INT);
			map.add(EmaFactory.createMapEntry().keyAscii("Key1", MapEntry.MapAction.ADD));
			map.add(EmaFactory.createMapEntry().keyAscii("Key2", MapEntry.MapAction.ADD));
			map.add(EmaFactory.createMapEntry().keyAscii("Key3", MapEntry.MapAction.ADD));
			
			ElementList element = EmaFactory.createElementList();
			element.add(EmaFactory.createElementEntry().map("1", map));
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( true, "Fails to encode with invalid key type - exception expected with text : " +  excp.getMessage()  );
			TestUtilities.checkResult(excp.getMessage().equals("Attempt to add entry of ASCII_STRING while Map entry key is set to INT with keyType() method"), "Check exception message text");
			return;
		}
		
		TestUtilities.checkResult( false, "Fails to encode with invalid key type - expect exception" );
	}
	
	public void testMapKeyTypeAndAddEntryKeyTypeMatch_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapKeyTypeAndAddEntryKeyTypeMatch_Encode_Decode","Encode Map with mismatch key type");
		
		try
		{
			Map map = EmaFactory.createMap();
			map.keyFieldId(11).totalCountHint(3).keyType(DataType.DataTypes.ASCII);
			map.add(EmaFactory.createMapEntry().keyAscii("Key1", MapEntry.MapAction.ADD));
			TestUtilities.checkResult("Map.toString() == toString()", map.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
			
			ElementList elementList = EmaFactory.createElementList();
			elementList.add(EmaFactory.createElementEntry().map("1", map));
			
			ElementList elementListDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), null, null);
			
			Iterator<ElementEntry> elementListIt = elementListDec.iterator();
			
			ElementEntry elementEntry = elementListIt.next();
			
			TestUtilities.checkResult( elementEntry.name().equals("1"), "Check element list key value" );
			
			Iterator<MapEntry> mapIt = elementEntry.map().iterator();
			
			MapEntry mapEntry = mapIt.next();
			
			TestUtilities.checkResult( mapEntry.key().ascii().ascii().equals("Key1"), "Check the key value of the first Map entry" );
			TestUtilities.checkResult( mapEntry.action() == MapEntry.MapAction.ADD, "Check the key action of the first Map entry" );
			
			TestUtilities.checkResult( mapIt.hasNext() == false, "Check whether there is another Map entry" );
			
			TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether there is another Element entry" );
			
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode with invalid key type - exception not expected with text : " +  excp.getMessage()  );
			return;
		}
		
		TestUtilities.checkResult( true, "Encode with match key type - exception not expected" );
	}
	
	public void testMapClear_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapClear_Encode_Decode","Test Clear map before encoding");
		
		try
		{
		FieldList summaryData = EmaFactory.createFieldList();
		summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 4563));
		
		Map map = EmaFactory.createMap();
		map.keyType(DataType.DataTypes.DATETIME).summaryData(summaryData);
		
		Map mapDec = JUnitTestConnect.createMap();
		JUnitTestConnect.setRsslData(mapDec, map, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		TestUtilities.checkResult( mapDec.summaryData().dataType() == DataType.DataTypes.FIELD_LIST, "Check data type of summary data before calling the clear method" );
		
		map.clear();
		
		TestUtilities.checkResult( map.summaryData().dataType() == DataType.DataTypes.NO_DATA, "Check data type of summary data after calling the clear method" );
		
		map.keyFieldId(11);
		map.add(EmaFactory.createMapEntry().keyAscii("Key1", MapEntry.MapAction.ADD));
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().map("1", map));
		
		ElementList elementListDec = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<ElementEntry> elementListIt = elementListDec.iterator();
		
		ElementEntry elementEntry = elementListIt.next();
		
		TestUtilities.checkResult( elementEntry.name().equals("1"), "Check element list key value" );
		
		Iterator<MapEntry> mapIt = elementEntry.map().iterator();
		
		MapEntry mapEntry = mapIt.next();
		
		TestUtilities.checkResult( mapEntry.key().ascii().ascii().equals("Key1"), "Check the key value of the first Map entry" );
		TestUtilities.checkResult( mapEntry.action() == MapEntry.MapAction.ADD, "Check the key action of the first Map entry" );
		
		TestUtilities.checkResult( mapIt.hasNext() == false, "Check whether there is another Map entry" );
		
		TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether there is another Element entry" );
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails encode after calling the clear method - exception not expected with text : " +  excp.getMessage()  );
			return;
		}		
	}
	
	public void testMapEntryKeyAsciiWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyAsciiWithNoPayload_Encode_Decode","Encode and decode Map with no payload for ascii key entry");

		String mapString = "Map\n" +
				"    MapEntry action=\"Add\" key dataType=\"Ascii\" value=\"ITEM1\" dataType=\"NoData\"\n" +
				"        NoData\n" +
				"        NoDataEnd\n" +
				"    MapEntryEnd\n" +
				"    MapEntry action=\"Update\" key dataType=\"Ascii\" value=\"ITEM2\" permissionData=\"00 00 04 d2\" dataType=\"NoData\"\n" +
				"        NoData\n" +
				"        NoDataEnd\n" +
				"    MapEntryEnd\n" +
				"    MapEntry action=\"Delete\" key dataType=\"Ascii\" value=\"ITEM3\" dataType=\"NoData\"\n" +
				"        NoData\n" +
				"        NoDataEnd\n" +
				"    MapEntryEnd\n" +
				"MapEnd\n";

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			Map mapEmpty = EmaFactory.createMap();

			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1234).flip();
			
			MapEntry me = EmaFactory.createMapEntry().keyAscii("ITEM1", MapEntry.MapAction.ADD);
			TestUtilities.checkResult("MapEntry.toString() == toString()", me.toString().equals("\nEntity is not encoded yet. Complete encoding to use this method.\n"));
			
			mapEnc.add(me);
			mapEnc.add(EmaFactory.createMapEntry().keyAscii("ITEM2", MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyAscii("ITEM3", MapEntry.MapAction.DELETE));

			DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

			TestUtilities.checkResult("Map.toString(dictionary) == toString(dictionary)", mapEnc.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

			emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
			emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

			TestUtilities.checkResult("Map.toString(dictionary) == toString(dictionary)", mapEnc.toString(emaDataDictionary).equals(mapString));

			TestUtilities.checkResult("Map.toString(dictionary) == toString(dictionary)", mapEmpty.toString(emaDataDictionary).equals("Map\nMapEnd\n"));

			mapEmpty.add(me);
			mapEmpty.clear();
			TestUtilities.checkResult("Map.toString(dictionary) == toString(dictionary)", mapEmpty.toString(emaDataDictionary).equals("Map\nMapEnd\n"));

			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			// check that we can still get the toString on encoded/decoded container.
			TestUtilities.checkResult("Map.toString() != toString()", !(mapDec.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));

			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			// check that we can still get the toString on encoded/decoded entry.
			TestUtilities.checkResult("MapEntry.toString() != toString()", !(mapEntryA.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));

			TestUtilities.checkResult( mapEntryA.key().ascii().ascii().equals("ITEM1"), "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ascii().ascii().equals("ITEM2"), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData), "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ascii().ascii().equals("ITEM3"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode and decode Map with no payload for ascii key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyBufferWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyBufferWithNoPayload_Encode_Decode","Encode Map with no payload for buffer key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(5678).flip();
			
			ByteBuffer keyBuffer1 = ByteBuffer.allocate(10);
			keyBuffer1.put("KeyBuffer1".getBytes()).flip();
			ByteBuffer keyBuffer2 = ByteBuffer.allocate(10);
			keyBuffer2.put("KeyBuffer2".getBytes()).flip();
			ByteBuffer keyBuffer3 = ByteBuffer.allocate(10);
			keyBuffer3.put("KeyBuffer3".getBytes()).flip();
			ByteBuffer keyBuffer4 = ByteBuffer.allocate(20);
			keyBuffer4.put("\u00cb\u00c3\u00c4\u00c5\u00c7\u00c8\u00c9\u00ca\u00d2\u00d3".getBytes(Charset.forName("ISO-8859-1"))).flip();			

			mapEnc.add(EmaFactory.createMapEntry().keyBuffer(keyBuffer1, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyBuffer(keyBuffer2, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyBuffer(keyBuffer3, MapEntry.MapAction.DELETE));
			mapEnc.add(EmaFactory.createMapEntry().keyBuffer(keyBuffer4, MapEntry.MapAction.DELETE));			
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().buffer().buffer().equals(keyBuffer1), "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			TestUtilities.checkResult( EmaUtility.asAsciiString(mapEntryA.key().buffer()).equals("KeyBuffer1") == true, "Check if EmaUtility.asAsciiString converts the string correctly" );
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().buffer().buffer().equals(keyBuffer2), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			TestUtilities.checkResult( EmaUtility.asAsciiString(mapEntryB.key().buffer()).equals("KeyBuffer2") == true, "Check if EmaUtility.asAsciiString converts the string correctly" );
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().buffer().buffer().equals(keyBuffer3), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryC.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");		
			TestUtilities.checkResult( EmaUtility.asAsciiString(mapEntryC.key().buffer()).equals("KeyBuffer3") == true, "Check if EmaUtility.asAsciiString converts the string correctly" );
			MapEntry mapEntryD = mapIt.next();
			TestUtilities.checkResult( mapEntryD.key().buffer().buffer().equals(keyBuffer4), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryD.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryD.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryD.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryD.hasPermissionData() == false, "Check wheter the third map entry has permission data");			
			
			TestUtilities.checkResult( EmaUtility.asAsciiString(mapEntryD.key().buffer()).equals("\u00cb\u00c3\u00c4\u00c5\u00c7\u00c8\u00c9\u00ca\u00d2\u00d3") == false, "Check if EmaUtility.asAsciiString converts the string correctly with ISO-8859-1 charset" );			
			TestUtilities.checkResult( EmaUtility.asString(mapEntryD.key().buffer(), Charset.forName("ISO-8859-1")).equals(new String("\u00cb\u00c3\u00c4\u00c5\u00c7\u00c8\u00c9\u00ca\u00d2\u00d3".getBytes(Charset.forName("ISO-8859-1")), Charset.forName("ISO-8859-1"))) == true, "Check if EmaUtility.asAsciiString converts the string correctly with ISO-8859-1 charset" );
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for buffer key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyDateWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyDateWithNoPayload_Encode_Decode","Encode Map with no payload for date key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(9101).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyDate(2018, 01, 02, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyDate(2019, 02, 03, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyDate(2020, 03, 04, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().date().toString().equals("02 JAN 2018") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().date().toString().equals("03 FEB 2019"), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().date().toString().equals("04 MAR 2020"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for date key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyDateTimeWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyDateTimeWithNoPayload_Encode_Decode","Encode Map with no payload for date time key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1213).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyDateTime(2021, 04, 05, 1, 2, 3, 4, 5, 6, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyDateTime(2022, 05, 06, 2, 3, 4, 5, 6, 7, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyDateTime(2023, 06, 07, 3, 4, 5, 6, 7, 8, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().dateTime().toString().equals("05 APR 2021 01:02:03:004:005:006") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().dateTime().toString().equals("06 MAY 2022 02:03:04:005:006:007"), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().dateTime().toString().equals("07 JUN 2023 03:04:05:006:007:008"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for date time key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyDoubleWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyDoubleWithNoPayload_Encode_Decode","Encode Map with no payload for double key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1314).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyDouble(56.55, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyDouble(65.66, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyDouble(75.77, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().ommDoubleValue().doubleValue() == 56.55 , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ommDoubleValue().doubleValue() == 65.66, "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ommDoubleValue().doubleValue() == 75.77, "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for double key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyEnumWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyEnumWithNoPayload_Encode_Decode","Encode Map with no payload for enum key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1415).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyEnum(55, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyEnum(66, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyEnum(77, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().ommEnumValue().enumValue() == 55 , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ommEnumValue().enumValue() == 66, "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ommEnumValue().enumValue() == 77, "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for enum key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyFloatWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyFloatWithNoPayload_Encode_Decode","Encode Map with no payload for float key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1516).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyFloat(123.4f, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyFloat(234.5f, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyFloat(345.6f, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().ommFloatValue().floatValue() == 123.4f , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ommFloatValue().floatValue() == 234.5f, "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ommFloatValue().floatValue() == 345.6f, "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for float key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyQosWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyQosWithNoPayload_Encode_Decode","Encode Map with no payload for qos key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1617).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyQos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.JUST_IN_TIME_CONFLATED,MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyQos(OmmQos.Timeliness.INEXACT_DELAYED, 5, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyQos(15, OmmQos.Rate.TICK_BY_TICK, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().qos().toString().equals("RealTime/JustInTimeConflated") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().qos().toString().equals("InexactDelayed/Rate: 5"), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().qos().toString().equals("Timeliness: 15/TickByTick") , "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for qos key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyIntWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyIntWithNoPayload_Encode_Decode","Encode Map with no payload for int key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1718).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyInt(Integer.MAX_VALUE, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyInt(Integer.MIN_VALUE, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyInt(555, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().ommIntValue().intValue() == Integer.MAX_VALUE , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ommIntValue().intValue() == Integer.MIN_VALUE, "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ommIntValue().intValue() == 555, "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for int key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyRealFromDoubleWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyRealFromDoubleWithNoPayload_Encode_Decode","Encode Map with no payload for real from double key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1819).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyReal(485.55, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyReal(9956.694, OmmReal.MagnitudeType.EXPONENT_POS_1, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyReal(1095.894, OmmReal.MagnitudeType.EXPONENT_POS_1, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().real().toString().equals("486.0") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().real().toString().equals("9960.0"), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().real().toString().equals("1100.0"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for real from double key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyRealFromMantissaWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyRealFromMantissaWithNoPayload_Encode_Decode","Encode Map with no payload for real from mantissa key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(1920).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyReal(5678, OmmReal.MagnitudeType.EXPONENT_NEG_1, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyReal(9956, OmmReal.MagnitudeType.EXPONENT_NEG_2, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyReal(31095, OmmReal.MagnitudeType.EXPONENT_NEG_3, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().real().toString().equals("567.8") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().real().toString().equals("99.56"), "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().real().toString().equals("31.095"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for real from mantissa key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyRmtesWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyRmtesWithNoPayload_Encode_decode","Encode Map with no payload for Rmtes key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(2021).flip();
			
			ByteBuffer rmtesKey1 = ByteBuffer.allocate(5);
			rmtesKey1.putInt(12345).flip();
			
			ByteBuffer rmtesKey2 = ByteBuffer.allocate(5);
			rmtesKey2.putInt(67891).flip();
			
			ByteBuffer rmtesKey3 = ByteBuffer.allocate(5);
			rmtesKey3.putInt(23456).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyRmtes(rmtesKey1, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyRmtes(rmtesKey2, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyRmtes(rmtesKey3, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().rmtes().asHex().equals(rmtesKey1) , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().rmtes().asHex().equals(rmtesKey2) , "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().rmtes().asHex().equals(rmtesKey3) , "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for Rmtes key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyStateWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyStateWithNoPayload_Encode_Decode","Encode Map with no payload for state key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(2122).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyState(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Text for key1", MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyState(OmmState.StreamState.NON_STREAMING, OmmState.DataState.SUSPECT, OmmState.StatusCode.ALREADY_OPEN, "Text for key2", MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyState(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.INVALID_ARGUMENT, "Text for key3", MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().state().toString().equals("Open / Ok / None / 'Text for key1'") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().state().toString().equals("Non-streaming / Suspect / Already open / 'Text for key2'") , "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().state().toString().equals("Closed / Suspect / Invalid argument / 'Text for key3'"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for state key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyTimeWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyTimeWithNoPayload_Encode_Decode","Encode Map with no payload for time key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(2223).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyTime(8, 20, 30 , 400, 500, 600, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyTime(9, 30, 40 , 500, 600, 700, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyTime(10, 40, 50 , 600, 700, 800, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().time().toString().equals("08:20:30:400:500:600") , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().time().toString().equals("09:30:40:500:600:700") , "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().time().toString().equals("10:40:50:600:700:800"), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for time key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyUIntFromBigIntegerWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyUIntFromBigIntegerWithNoPayload_Encode_Decode","Encode Map with no payload for BigInteger key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(2324).flip();
			
			BigInteger key1 = new BigInteger("123");
			BigInteger key2 = new BigInteger("234");
			BigInteger key3 = new BigInteger("345");
			
			mapEnc.add(EmaFactory.createMapEntry().keyUInt(key1, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyUInt(key2, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyUInt(key3, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().ommUIntValue().longValue() == key1.longValue() , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ommUIntValue().longValue() == key2.longValue() , "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ommUIntValue().longValue() == key3.longValue(), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for BigInteger key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyUIntFromLongWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyUIntFromBigIntegerWithNoPayload_Encode_Decode","Encode Map with no payload for long key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(3435).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyUInt(1234, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyUInt(2345, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyUInt(3456, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().ommUIntValue().longValue() == 1234 , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().ommUIntValue().longValue() == 2345 , "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().ommUIntValue().longValue() == 3456, "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for BigInteger key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapEntryKeyUtf8WithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testMapEntryKeyUtf8WithNoPayload_Encode_Decode","Encode Map with no payload for Utf8 key entry");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			
			ByteBuffer permissionData = ByteBuffer.allocate(4);
			permissionData.putInt(3435).flip();
			
			ByteBuffer utf8key1 = ByteBuffer.allocate(5);
			utf8key1.putInt(34561).flip();
			
			ByteBuffer utf8key2 = ByteBuffer.allocate(5);
			utf8key1.putInt(78912).flip();
			
			ByteBuffer utf8key3 = ByteBuffer.allocate(5);
			utf8key1.putInt(12135).flip();
			
			mapEnc.add(EmaFactory.createMapEntry().keyUtf8(utf8key1, MapEntry.MapAction.ADD));
			mapEnc.add(EmaFactory.createMapEntry().keyUtf8(utf8key2, MapEntry.MapAction.UPDATE, permissionData));
			mapEnc.add(EmaFactory.createMapEntry().keyUtf8(utf8key3, MapEntry.MapAction.DELETE));
			
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			
			Iterator<MapEntry> mapIt = mapDec.iterator();
			
			MapEntry mapEntryA = mapIt.next();
			TestUtilities.checkResult( mapEntryA.key().utf8().buffer().equals(utf8key1) , "Check the key value of the first map entry");
			TestUtilities.checkResult( mapEntryA.action() == MapEntry.MapAction.ADD, "Check the action of the first map entry");
			TestUtilities.checkResult( mapEntryA.loadType() == DataTypes.NO_DATA, "Check the load type of the first map entry");	
			TestUtilities.checkResult( mapEntryA.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the first map entry");
			TestUtilities.checkResult( mapEntryA.hasPermissionData() == false, "Check wheter the first map entry has permission data");
			MapEntry mapEntryB = mapIt.next();
			TestUtilities.checkResult( mapEntryB.key().utf8().buffer().equals(utf8key2) , "Check the key value of the second map entry");
			TestUtilities.checkResult( mapEntryB.action() == MapEntry.MapAction.UPDATE, "Check the action of the second map entry");
			TestUtilities.checkResult( mapEntryB.loadType() == DataTypes.NO_DATA, "Check the load type of the second map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryB.hasPermissionData(), "Check wheter the second map entry has permission data");
			TestUtilities.checkResult( mapEntryB.permissionData().equals(permissionData) == true, "Compare the permission data of the second map entry");
			MapEntry mapEntryC = mapIt.next();
			TestUtilities.checkResult( mapEntryC.key().utf8().buffer().equals(utf8key3), "Check the key value of the third map entry");
			TestUtilities.checkResult( mapEntryC.action() == MapEntry.MapAction.DELETE, "Check the action of the third map entry");
			TestUtilities.checkResult( mapEntryC.loadType() == DataTypes.NO_DATA, "Check the load type of the third map entry");
			TestUtilities.checkResult( mapEntryB.load().dataType() == DataTypes.NO_DATA, "Get load and check data type of the second map entry");
			TestUtilities.checkResult( mapEntryC.hasPermissionData() == false, "Check wheter the third map entry has permission data");
			
			TestUtilities.checkResult( mapIt.hasNext() == false , "Check the end of Map");
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode Map with no payload for Utf8 key entry - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testMapContainsPartialEmptyFieldList_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsPartialEmptyFieldList_EncodeDecodeAll","Encode Map of FieldList which has info, but no entry with EMA, decode all");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			mapEnc.totalCountHint(1);
			mapEnc.keyFieldId(3426);

			FieldList flEnc = EmaFactory.createFieldList();
			TestUtilities.EmaEncodeFieldListAll(flEnc);

			mapEnc.summaryData(flEnc);

			ByteBuffer permission = ByteBuffer.wrap("PERMISSION DATA".getBytes());

			//first entry  //Delete Buffer
			ByteBuffer orderBuf =ByteBuffer.wrap("ABCD".getBytes());
			
			flEnc.clear();
			flEnc.info(1, 1);
			
			mapEnc.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));
			
			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iterator();
			
			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map contains FieldList - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map contains FieldList - getKeyFieldId()" );
			TestUtilities.checkResult( mapDec.hasTotalCountHint(), "Map contains FieldList - hasTotalCountHint()" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 1, "Map contains FieldList - getTotalCountHint()" );

			FieldList fl = mapIter.next().fieldList();
			Iterator<FieldEntry> iter = fl.iterator();
			
			TestUtilities.checkResult(iter != null);
			TestUtilities.checkResult(! iter.hasNext(), "Decode FieldList - first fieldlist iter.hasNext() is not true" );
			TestUtilities.checkResult( fl.size() == 0, "Decode FieldList - fieldlist size is 0" );
			TestUtilities.checkResult( iter.next() == null, "Decode FieldList - first fieldlist iter.next() is null" );
		}
		catch ( Exception excp )
		{
			TestUtilities.checkResult( true, "empty fieldlist - exception expected: " +  excp.getMessage()  );
			return;
		}

		TestUtilities.checkResult( false, "empty Map - did not get expected exception" );
	}
	
	public void testMapContainsCompletelEmptyFieldList_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsCompletelEmptyFieldList_EncodeDecodeAll","Encode Map of FieldList which has no any info with EMA, decode all");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		try {
			com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1024));

			TestUtilities.eta_EncodeMapKeyIntWithEmptyFLs(buf);
			
			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iterator();
			
			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map contains FieldList - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map contains FieldList - getKeyFieldId()" );
			TestUtilities.checkResult( mapDec.hasTotalCountHint(), "Map contains FieldList - hasTotalCountHint()" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 1, "Map contains FieldList - getTotalCountHint()" );

			FieldList fl = mapIter.next().fieldList();
			Iterator<FieldEntry> iter = fl.iterator();
			
			TestUtilities.checkResult(iter != null);
			TestUtilities.checkResult(! iter.hasNext(), "Decode FieldList - first fieldlist iter.hasNext() is not true" );
			TestUtilities.checkResult( fl.size() == 0, "Decode FieldList - fieldlist size is 0" );
			TestUtilities.checkResult( iter.next() == null, "Decode FieldList - first fieldlist iter.next() is null" );
		}
		catch ( Exception excp )
		{
			TestUtilities.checkResult( true, "empty fieldlist - exception expected: " +  excp.getMessage()  );
			return;
		}

		TestUtilities.checkResult( false, "empty Map - did not get expected exception" );
	}
	
	public void testMapContainsFieldLists_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsFieldLists_EncodeDecodeAll","Encode Map that contains FieldLists with EMA and Decode Map with EMA");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with FieldList), Delete, FieldList-Add, FieldList-Add, FieldList-Update

			Map mapEnc = EmaFactory.createMap();
			TestUtilities.EmaEncodeMapAllWithFieldList( mapEnc);

			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iterator();
			
			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map contains FieldList - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map contains FieldList - getKeyFieldId()" );
			TestUtilities.checkResult( mapDec.hasTotalCountHint(), "Map contains FieldList - hasTotalCountHint()" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 5, "Map contains FieldList - getTotalCountHint()" );
			
			switch ( mapDec.summaryData().dataType() )
			{
				case DataType.DataTypes.FIELD_LIST :
				{
					FieldList fl = mapDec.summaryData().fieldList();
					TestUtilities.EmaDecodeFieldListAll(fl);
				}
				break;
				default :
					TestUtilities.checkResult( false, "Map Decode Summary FieldList - map.summaryType() not expected" );
				break;
			}

			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - first maphasNext()" );

			MapEntry me1 = mapIter.next();

			TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );


			mapIter = mapDec.iterator();
			{
				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - maphasNext() after regenerating iterator" );

				me1 = mapIter.next();

				TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
				TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
			}


			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - second maphasNext()" );

			MapEntry me2 = mapIter.next();

			TestUtilities.checkResult( me2.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me2.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
			TestUtilities.checkResult( me2.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeFieldListAll( me2.fieldList() );

			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - third maphasNext()" );

			MapEntry me3 = mapIter.next();

			TestUtilities.checkResult( me3.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me3.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me3.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
			TestUtilities.checkResult( me3.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
			TestUtilities.EmaDecodeFieldListAll( me3.fieldList() );
			
			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - fourth maphasNext()" );

			MapEntry me4 = mapIter.next();

			TestUtilities.checkResult( me4.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me4.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me4.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
			TestUtilities.checkResult( me4.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

			TestUtilities.checkResult( !mapIter.hasNext(), "Map contains FieldList - final maphasNext()" );

			TestUtilities.checkResult( true, "Map contains FieldList - exception not expected" );
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "Map contains FieldList - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}
	
	public void testMapContainsFieldLists_EfficientDecode_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsFieldLists_EfficientDecode_EncodeDecodeAll","Encode Map that contains FieldLists with EMA and Decode Map with EMA using Efficient methods for iteration");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with FieldList), Delete, FieldList-Add, FieldList-Add, FieldList-Update

			Map mapEnc = EmaFactory.createMap();
			TestUtilities.EmaEncodeMapAllWithFieldList( mapEnc);

			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iteratorByRef();
			
			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map contains FieldList - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map contains FieldList - getKeyFieldId()" );
			TestUtilities.checkResult( mapDec.hasTotalCountHint(), "Map contains FieldList - hasTotalCountHint()" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 5, "Map contains FieldList - getTotalCountHint()" );
			
			switch ( mapDec.summaryData().dataType() )
			{
				case DataType.DataTypes.FIELD_LIST :
				{
					FieldList fl = mapDec.summaryData().fieldList();
					TestUtilities.EmaDecodeFieldListAll(fl);
				}
				break;
				default :
					TestUtilities.checkResult( false, "Map Decode Summary FieldList - map.summaryType() not expected" );
				break;
			}

			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - first maphasNext()" );

			MapEntry me1 = mapIter.next();

			TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );


			mapIter = mapDec.iteratorByRef();
			{
				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - maphasNext() after regenerating iterator" );

				me1 = mapIter.next();

				TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
				TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
			}


			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - second maphasNext()" );

			MapEntry me2 = mapIter.next();

			TestUtilities.checkResult( me2.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me2.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
			TestUtilities.checkResult( me2.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeFieldListAll( me2.fieldList() );

			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - third maphasNext()" );

			MapEntry me3 = mapIter.next();

			TestUtilities.checkResult( me3.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me3.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me3.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
			TestUtilities.checkResult( me3.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
			TestUtilities.EmaDecodeFieldListAll( me3.fieldList() );
			
			TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - fourth maphasNext()" );

			MapEntry me4 = mapIter.next();

			TestUtilities.checkResult( me4.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me4.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me4.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
			TestUtilities.checkResult( me4.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

			TestUtilities.checkResult( !mapIter.hasNext(), "Map contains FieldList - final maphasNext()" );

			TestUtilities.checkResult( true, "Map contains FieldList - exception not expected" );
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "Map contains FieldList - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}
	
	public void testMapKeyUtf8String_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapKeyUtf8String_EncodeDecodeAll","Encode Map (key is UTF8) that contains FieldLists with EMA and Decode Map with EMA");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update
			Map mapEnc = EmaFactory.createMap();
			TestUtilities.EmaEncodeMapAllWithFieldListWithUtf8Key( mapEnc);

			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iterator();
			
			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map key Utf8String - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map key Utf8String - getKeyFieldId()" );
			
			TestUtilities.checkResult( mapIter.hasNext(), "Map key Utf8String - first maphasNext()" );
			MapEntry me1 = mapIter.next();
			TestUtilities.checkResult( me1.key().dataType() == DataTypes.UTF8, "MapEntry.key().dataType() == DataTypes.UTF8" );
			TestUtilities.checkResult( Arrays.equals( me1.key().utf8().buffer().array() , new String("ABC").getBytes()), "MapEntry.key().utf8().buffer()" );
			TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

			TestUtilities.checkResult( mapIter.hasNext(), "Map key Utf8String - second maphasNext()" );
			MapEntry me2 = mapIter.next();
			TestUtilities.checkResult( me2.key().dataType() == DataTypes.UTF8, "MapEntry.key().dataType() == DataTypes.UTF8" );
			TestUtilities.checkResult( Arrays.equals( me2.key().utf8().buffer().array() , new String("ABC").getBytes()), "MapEntry.key().utf8().buffer()" );
			TestUtilities.checkResult( me2.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
			TestUtilities.checkResult( me2.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			{
				FieldList fl = me2.fieldList();
				TestUtilities.EmaDecodeFieldListAll(fl);
			}

			TestUtilities.checkResult( mapIter.hasNext(), "Map key Utf8String - third maphasNext()" );
			MapEntry me3 = mapIter.next();
				TestUtilities.checkResult( me3.key().dataType() == DataTypes.UTF8, "MapEntry.key().dataType() == DataTypes.UTF8" );
				TestUtilities.checkResult( Arrays.equals( me3.key().utf8().buffer().array() , new String("DEFGH").getBytes()), "MapEntry.key().utf8().buffer()" );
			TestUtilities.checkResult( me3.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
			TestUtilities.checkResult( me3.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			{
				FieldList fl = me3.fieldList();
				TestUtilities.EmaDecodeFieldListAll(fl);
			}
			
			TestUtilities.checkResult( mapIter.hasNext(), "Map key Utf8String - fourth maphasNext()" );
			MapEntry me4 = mapIter.next();
			TestUtilities.checkResult( me4.key().dataType() == DataTypes.UTF8, "MapEntry.key().dataType() == DataTypes.UTF8" );
			TestUtilities.checkResult( Arrays.equals( me4.key().utf8().buffer().array() , new String("KLMNOPQRS").getBytes()), "MapEntry.key().utf8().buffer()" );
			TestUtilities.checkResult( me4.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
			TestUtilities.checkResult( me4.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			{
				FieldList fl = me4.fieldList();
				TestUtilities.EmaDecodeFieldListAll(fl);
			}
			
			TestUtilities.checkResult( !mapIter.hasNext(), "Map key Utf8String - fourth maphasNext()" );

			TestUtilities.checkResult( true, "Map key Utf8String - exception not expected" );

		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "Map key Utf8String - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}
	
	public void testMapContainsElementLists_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsElementLists_EncodeDecodeAll","Encode Map that contains ElementLists with EMA and Decode Map with EMA");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with ElementList), Delete, ElementList-Add, ElementList-Add, ElementList-Update
			Map mapEnc = EmaFactory.createMap();
			TestUtilities.EmaEncodeMapAllWithElementList( mapEnc);

			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iterator();

			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map contains ElementList - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map contains ElementList - getKeyFieldId()" );
			TestUtilities.checkResult( mapDec.hasTotalCountHint(), "Map contains ElementList - hasTotalCountHint()" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 5, "Map contains ElelementList - getTotalCountHint()" );

			switch ( mapDec.summaryData().dataType() )
			{
				case DataType.DataTypes.ELEMENT_LIST :
				{
					ElementList el = mapDec.summaryData().elementList();
					TestUtilities.EmaDecodeElementListAll( el );
				}
				break;
				default :
					TestUtilities.checkResult( false, "Map Decode Summary ElementList - map.summaryType() not expected" );
				break;
			}
			
			TestUtilities.checkResult( mapIter.hasNext(), "Map contains ElementList - first map hasNext()" );

			MapEntry me1 = mapIter.next();

			TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

		TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - second maphasNext()" );

		MapEntry me2 = mapIter.next();

		TestUtilities.checkResult( me2.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
		TestUtilities.checkResult( Arrays.equals( me2.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
		TestUtilities.checkResult( me2.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
		TestUtilities.checkResult( me2.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
		TestUtilities.EmaDecodeElementListAll( me2.elementList() );

		TestUtilities.checkResult( mapIter.hasNext(), "Map contains ElementList - third maphasNext()" );

		MapEntry me3 = mapIter.next();

		TestUtilities.checkResult( me3.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
		TestUtilities.checkResult( Arrays.equals( me3.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
		TestUtilities.checkResult( me3.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
		TestUtilities.checkResult( me3.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
		TestUtilities.EmaDecodeElementListAll( me3.elementList() );
		
		TestUtilities.checkResult( mapIter.hasNext(), "Map contains ElementList - fourth maphasNext()" );

		MapEntry me4 = mapIter.next();

		TestUtilities.checkResult( me4.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
		TestUtilities.checkResult( Arrays.equals( me4.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
		TestUtilities.checkResult( me4.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
		TestUtilities.checkResult( me4.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

		TestUtilities.checkResult( !mapIter.hasNext(), "Map contains ElementList - final maphasNext()" );

		TestUtilities.checkResult( true, "Map contains ElementList - exception not expected" );
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "Map contains ElementList - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}
	
	public void testMapContainsMaps_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsMaps_EncodeDecodeAll","Encode Map  that contains Map with EMA and Decode Map with EMA");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with Map), Delete, Map-Add, Map-Add, Map-Update
			Map mapEnc = EmaFactory.createMap();
			TestUtilities.EmaEncodeMapAllWithMap(mapEnc);

			//Now do EMA decoding of Map
			Map mapDec = JUnitTestConnect.createMap();
			JUnitTestConnect.setRsslData(mapDec, mapEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(mapDec);

			Iterator<MapEntry> mapIter = mapDec.iterator();

			TestUtilities.checkResult( mapDec.hasKeyFieldId(), "Map contains ElementList - hasKeyFieldId()" );
			TestUtilities.checkResult( mapDec.keyFieldId() == 3426, "Map contains ElementList - getKeyFieldId()" );
			TestUtilities.checkResult( mapDec.hasTotalCountHint(), "Map contains ElementList - hasTotalCountHint()" );
			TestUtilities.checkResult( mapDec.totalCountHint() == 5, "Map contains ElelementList - getTotalCountHint()" );

			switch ( mapDec.summaryData().dataType() )
			{
				case DataTypes.MAP :
				{
					Map mapS = mapDec.summaryData().map();
					TestUtilities.checkResult( mapS.hasKeyFieldId(), "Map Decode Summary Map - hasKeyFieldId()" );
					TestUtilities.checkResult( mapS.keyFieldId() == 3426, "Map Decode Summary Map - getKeyFieldId()" );
				
					Iterator<MapEntry> summaryIter = mapS.iterator();
					
					TestUtilities.checkResult( summaryIter.hasNext(), "Map Decode Summary Map - first map hasNext()" );
					MapEntry me1a = summaryIter.next();
					TestUtilities.checkResult( me1a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
					TestUtilities.checkResult( Arrays.equals( me1a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
					TestUtilities.checkResult( me1a.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
					TestUtilities.checkResult( me1a.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

					TestUtilities.checkResult( summaryIter.hasNext(), "Map contains FieldList - second maphasNext()" );
	
					MapEntry me2a = summaryIter.next();
	
					TestUtilities.checkResult( me2a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
					TestUtilities.checkResult( Arrays.equals( me2a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
					TestUtilities.checkResult( me2a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
					TestUtilities.checkResult( me2a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
					TestUtilities.EmaDecodeFieldListAll( me2a.fieldList() );
	
					TestUtilities.checkResult( summaryIter.hasNext(), "Map contains FieldList - third maphasNext()" );
	
					MapEntry me3a = summaryIter.next();
	
					TestUtilities.checkResult( me3a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
					TestUtilities.checkResult( Arrays.equals( me3a.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
					TestUtilities.checkResult( me3a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
					TestUtilities.checkResult( me3a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
					TestUtilities.EmaDecodeFieldListAll( me3a.fieldList() );
					
					TestUtilities.checkResult( summaryIter.hasNext(), "Map contains FieldList - fourth maphasNext()" );
	
					MapEntry me4a = summaryIter.next();
	
					TestUtilities.checkResult( me4a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
					TestUtilities.checkResult( Arrays.equals( me4a.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
					TestUtilities.checkResult( me4a.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
					TestUtilities.checkResult( me4a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

					TestUtilities.checkResult( !summaryIter.hasNext(), "Map Decode Summary Map - fifth map hasNext()" );
				}
				break;
				default :
					TestUtilities.checkResult( false, "Map Decode Summary Map - map.summaryType() not expected" );
				break;
			}

			
			TestUtilities.checkResult( mapIter.hasNext(), "Map contains Map - first map hasNext()" );

			MapEntry me1 = mapIter.next();

			TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

			mapIter = mapDec.iterator();
			{
				TestUtilities.checkResult( mapIter.hasNext(), "Map contains Map - map hasNext() after regenerating iterator" );

				 me1 = mapIter.next();

				TestUtilities.checkResult( me1.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me1.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
				TestUtilities.checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
			}

			TestUtilities.checkResult( mapIter.hasNext(), "Map contains Map - second map hasNext()" );

			MapEntry me2 = mapIter.next();

			TestUtilities.checkResult( me2.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me2.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me2.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me2.load().dataType() == DataTypes.MAP, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

			{
				Map mapNested = me2.map();
				TestUtilities.checkResult( mapNested.hasKeyFieldId(), "MapEntry Map within map - hasKeyFieldId()" );
				TestUtilities.checkResult( mapNested.keyFieldId() == 3426, "MapEntry Map within map - getKeyFieldId()" );
			
				Iterator<MapEntry> mapNestedIter = mapNested.iterator();
				TestUtilities.checkResult( mapNestedIter.hasNext(), "MapEntry Map within map - first map hasNext()" );
				MapEntry me1a = mapNestedIter.next();
				
				TestUtilities.checkResult( me1a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me1a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me1a.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
				TestUtilities.checkResult( me1a.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - second maphasNext()" );

				MapEntry me2a = mapNestedIter.next();

				TestUtilities.checkResult( me2a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me2a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me2a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
				TestUtilities.checkResult( me2a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
				TestUtilities.EmaDecodeFieldListAll( me2a.fieldList() );

				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - third maphasNext()" );

				MapEntry me3a = mapNestedIter.next();

				TestUtilities.checkResult( me3a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me3a.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me3a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
				TestUtilities.checkResult( me3a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
				TestUtilities.EmaDecodeFieldListAll( me3a.fieldList() );
				
				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - fourth maphasNext()" );

				MapEntry me4a = mapNestedIter.next();

				TestUtilities.checkResult( me4a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me4a.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me4a.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
				TestUtilities.checkResult( me4a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

				TestUtilities.checkResult( !mapNestedIter.hasNext(), "Map Decode Summary Map - fifth map hasNext()" );
			}

			TestUtilities.checkResult( mapIter.hasNext(), "Map contains Map - third map hasNext()" );

			MapEntry me3 = mapIter.next();

			TestUtilities.checkResult( me3.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me3.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me3.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me3.load().dataType() == DataTypes.MAP, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

			{
				Map mapNested = me3.map();
				TestUtilities.checkResult( mapNested.hasKeyFieldId(), "MapEntry Map within map - hasKeyFieldId()" );
				TestUtilities.checkResult( mapNested.keyFieldId() == 3426, "MapEntry Map within map - getKeyFieldId()" );
			
				Iterator<MapEntry> mapNestedIter = mapNested.iterator();
				TestUtilities.checkResult( mapNestedIter.hasNext(), "MapEntry Map within map - first map hasNext()" );
				MapEntry me1a = mapNestedIter.next();
				
				TestUtilities.checkResult( me1a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me1a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me1a.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
				TestUtilities.checkResult( me1a.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - second maphasNext()" );

				MapEntry me2a = mapNestedIter.next();

				TestUtilities.checkResult( me2a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me2a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me2a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
				TestUtilities.checkResult( me2a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
				TestUtilities.EmaDecodeFieldListAll( me2a.fieldList() );

				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - third maphasNext()" );

				MapEntry me3a = mapNestedIter.next();

				TestUtilities.checkResult( me3a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me3a.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me3a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
				TestUtilities.checkResult( me3a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
				TestUtilities.EmaDecodeFieldListAll( me3a.fieldList() );
				
				TestUtilities.checkResult( mapIter.hasNext(), "Map contains FieldList - fourth maphasNext()" );

				MapEntry me4a = mapNestedIter.next();

				TestUtilities.checkResult( me4a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me4a.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me4a.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
				TestUtilities.checkResult( me4a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

				TestUtilities.checkResult( !mapNestedIter.hasNext(), "Map Decode Summary Map - fifth map hasNext()" );
			}
			
			TestUtilities.checkResult( mapIter.hasNext(), "Map contains Map - fourth map hasNext()" );

			MapEntry me4 = mapIter.next();

			TestUtilities.checkResult( me4.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult( Arrays.equals( me4.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
			TestUtilities.checkResult( me4.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
			TestUtilities.checkResult( me4.load().dataType() == DataTypes.MAP, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

			{
				Map mapNested = me4.map();
				TestUtilities.checkResult( mapNested.hasKeyFieldId(), "MapEntry Map within map - hasKeyFieldId()" );
				TestUtilities.checkResult( mapNested.keyFieldId() == 3426, "MapEntry Map within map - getKeyFieldId()" );
			
				Iterator<MapEntry> mapNestedIter = mapNested.iterator();
				TestUtilities.checkResult( mapNestedIter.hasNext(), "MapEntry Map within map - first map hasNext()" );
				MapEntry me1a = mapNestedIter.next();
				
				TestUtilities.checkResult( me1a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me1a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me1a.action() == MapEntry.MapAction.DELETE, "MapEntry.action() == MapEntry.MapAction.DELETE" );
				TestUtilities.checkResult( me1a.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NO_DATA" );

				TestUtilities.checkResult( mapNestedIter.hasNext(), "Map contains FieldList - second maphasNext()" );

				MapEntry me2a = mapNestedIter.next();

				TestUtilities.checkResult( me2a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me2a.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me2a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
				TestUtilities.checkResult( me2a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
				TestUtilities.EmaDecodeFieldListAll( me2a.fieldList() );

				TestUtilities.checkResult( mapNestedIter.hasNext(), "Map contains FieldList - third maphasNext()" );

				MapEntry me3a = mapNestedIter.next();

				TestUtilities.checkResult( me3a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me3a.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me3a.action() == MapEntry.MapAction.ADD, "MapEntry.action() == MapEntry.MapAction.ADD" );
				TestUtilities.checkResult( me3a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NO_DATA" );
				TestUtilities.EmaDecodeFieldListAll( me3a.fieldList() );
				
				TestUtilities.checkResult( mapNestedIter.hasNext(), "Map contains FieldList - fourth maphasNext()" );

				MapEntry me4a = mapNestedIter.next();

				TestUtilities.checkResult( me4a.key().dataType() == DataType.DataTypes.BUFFER, "MapEntry.key().dataType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult( Arrays.equals( me4a.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry.key().buffer()" );
				TestUtilities.checkResult( me4a.action() == MapEntry.MapAction.UPDATE, "MapEntry.action() == MapEntry.MapAction.UPDATE" );
				TestUtilities.checkResult( me4a.load().dataType() == DataType.DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

				TestUtilities.checkResult( !mapNestedIter.hasNext(), "Map Decode Summary Map - fifth map hasNext()" );
			}

			TestUtilities.checkResult( !mapIter.hasNext(), "Map contains Map - fifth map hasNext()" );

			TestUtilities.checkResult( true, "Map contains Map - exception not expected" );
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "Map contains Map - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}

	
	public void testMapWithKeyUInt_ContainsFieldLists_EncodeETA_decodeEMA_DecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsFieldLists_EncodeETA_decodeEMA_DecodeAll","Encode Map (key is UInt) that contains FieldLists with ETA and Decode Map with EMA");

		// Create a ETA Buffer to encode into
		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		
		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		// Encode Map with ETA.
		int retVal;
		System.out.println("Begin ETA Map Encoding");
		if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(buf, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeMapKeyUIntWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		//
		// Check decoded Map data
		TestUtilities.checkResult(map.hasKeyFieldId());
		TestUtilities.checkResult(map.keyFieldId(), 3426);
		TestUtilities.checkResult(map.hasTotalCountHint());
		TestUtilities.checkResult(map.totalCountHint(), 3);

		switch (map.summaryData().dataType())
		{
		case DataType.DataTypes.FIELD_LIST:
			com.refinitiv.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		// check first map entry
		Iterator<MapEntry> mapIter = map.iterator();
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me1.key().uintValue(), 1);
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check reseting iteration to start of map
		mapIter = map.iterator();
		// check first map entry again
		TestUtilities.checkResult(mapIter.hasNext());
		me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me1.key().uintValue(), 1);
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me2.key().uintValue(), 2);
		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me3.key().uintValue(), 3);
		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapContainsFieldLists_EncodeETA_decodeEMA_DecodeAll passed");
	}

	public void testMapWithKeyInt_ContainsFieldLists_EncodeETA_decodeEMA_DecodeAll()
	{
		System.out.println("testMapKeyIntContainsFieldLists_EncodeETA_decodeEMA_DecodeAll");
		System.out.println("Encode Map (key is Int) that contains FieldLists with ETA and Decode Map with EMA\n");

		// Create a ETA Buffer to encode into
		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		// Encode Map with ETA.
		int retVal;
		System.out.println("Begin ETA Map Encoding");
		if ((retVal = TestUtilities.eta_EncodeMapKeyIntWithFLs(buf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeMapKeyIntWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		//
		// Check decoded Map data
		TestUtilities.checkResult(map.hasKeyFieldId());
		TestUtilities.checkResult(map.keyFieldId(), 3426);
		TestUtilities.checkResult(map.hasTotalCountHint());
		TestUtilities.checkResult(map.totalCountHint(), 3);

		switch (map.summaryData().dataType())
		{
		case DataType.DataTypes.FIELD_LIST:
			com.refinitiv.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		Iterator<MapEntry> mapIter = map.iterator();
		
		// check first map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me1.key().intValue(), 1);
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check reseting iteration to start of map
		mapIter = map.iterator();
		// check first map entry again
		TestUtilities.checkResult(mapIter.hasNext());
		me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me1.key().intValue(), 1);
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me2.key().intValue(), 2);
		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me3.key().intValue(), 3);
		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapKeyIntContainsFieldLists_EncodeETA_decodeEMA_DecodeAll passed");
	}

	public void testMapWithKeyAsciiString_ContainsFieldLists_EncodeETA_decodeEMA_DecodeAll()
	{
		System.out.println("testMapKeyAsciiStringContainsFieldLists_EncodeETA_decodeEMA_DecodeAll");
		System.out
				.println("Encode Map (key is AsciiString) that contains FieldLists with ETA and Decode Map with EMA\n");

		// Create a ETA Buffer to encode into
		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		// Encode Map with ETA.
		int retVal;
		System.out.println("Begin ETA Map Encoding");
		if ((retVal = TestUtilities.eta_EncodeMapKeyAsciiStringWithFLs(buf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeMapKeyAsciiStringWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		//
		// Check decoded Map data
		TestUtilities.checkResult(map.hasKeyFieldId());
		TestUtilities.checkResult(map.keyFieldId(), 3426);
		TestUtilities.checkResult(map.hasTotalCountHint());
		TestUtilities.checkResult(map.totalCountHint(), 3);

		switch (map.summaryData().dataType())
		{
		case DataType.DataTypes.FIELD_LIST:
			com.refinitiv.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		Iterator<MapEntry> mapIter = map.iterator();
		
		// check first map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData1");
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check reseting iteration to start of map
		mapIter = map.iterator();
		// check first map entry again
		TestUtilities.checkResult(mapIter.hasNext());
		me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData1");
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me2.key().ascii().toString(), "keyData2");
		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me3.key().ascii().toString(), "keyData3");
		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapKeyAsciiStringContainsFieldLists_EncodeETA_decodeEMA_DecodeAll passed");
	}

	public void testMapWithKeyBuffer_ContainsFieldLists_EncodeETA_decodeEMA_DecodeAll()
	{
		System.out.println("testMapKeyBufferContainsFieldLists_EncodeETA_decodeEMA_DecodeAll");
		System.out.println("Encode Map (key is Buffer) that contains FieldLists with ETA and Decode Map with EMA\n");

		// Create a ETA Buffer to encode into
		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		// Encode Map with ETA.
		int retVal;
		System.out.println("Begin ETA Map Encoding");
		if ((retVal = TestUtilities.eta_EncodeMapKeyBufferWithFLs(buf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeMapKeyBufferWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		//
		// Check decoded Map data
		TestUtilities.checkResult(map.hasKeyFieldId());
		TestUtilities.checkResult(map.keyFieldId(), 3426);
		TestUtilities.checkResult(map.hasTotalCountHint());
		TestUtilities.checkResult(map.totalCountHint(), 3);

		switch (map.summaryData().dataType())
		{
		case DataType.DataTypes.FIELD_LIST:
			com.refinitiv.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		// check first map entry
		Iterator<MapEntry> mapIter = map.iterator();
		
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.BUFFER);

		com.refinitiv.eta.codec.Buffer keyData = CodecFactory.createBuffer();
		keyData.data("keyData1");
		ByteBuffer expectedByteBuffer1 = keyData.data();
		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer1.array()));

		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check reseting iteration to start of map
		mapIter = map.iterator();

		// check first map entry again
		TestUtilities.checkResult(mapIter.hasNext());
		me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.BUFFER);
		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer1.array()));
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.BUFFER);

		keyData.data("keyData2");
		ByteBuffer expectedByteBuffer2 = keyData.data();
		TestUtilities.checkResult(Arrays.equals(me2.key().buffer().buffer().array(), expectedByteBuffer2.array()));

		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.refinitiv.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_ETAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.BUFFER);

		keyData.data("keyData3");
		ByteBuffer expectedByteBuffer3 = keyData.data();
		TestUtilities.checkResult(Arrays.equals(me3.key().buffer().buffer().array(), expectedByteBuffer3.array()));

		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapKeyBufferContainsFieldLists_EncodeETA_decodeEMA_DecodeAll passed");
	}

//	public void testMapWithKeyUInt_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapKeyUIntContainsElementLists_EncodeETA_decodeEMA_DecodeAll");
//		System.out.println("Encode Map (key is UInt) that contains ElementLists with ETA and Decode Map with EMA\n");
//
//		// Create a ETA Buffer to encode into
//		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.eta_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with ETA.
//		int retVal;
//		System.out.println("Begin ETA Map Encoding");
//		if ((retVal = TestUtilities.eta_EncodeMapKeyUIntWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.eta_EncodeMapAll.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End ETA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
//		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
//
//		//
//		// Check decoded Map data
//		TestUtilities.checkResult(map.hasKeyFieldId());
//		TestUtilities.checkResult(map.keyFieldId(), 3426);
//		TestUtilities.checkResult(map.hasTotalCountHint());
//		TestUtilities.checkResult(map.totalCountHint(), 3);
//
//		switch (map.summaryData().dataType())
//		{
//		case DataType.DataTypes.ELEMENT_LIST:
//			com.refinitiv.ema.access.ElementList el = map.summaryData().elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//			break;
//
//		default:
//			TestUtilities.checkResult(false);
//		}
//
//		// check first map entry
//		Iterator<MapEntry> mapIter = map.iterator();
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me1.key().uintValue(), 1);
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check reseting iteration to start of map
//		mapIter = map.iterator();
//
//		// check first map entry again
//		TestUtilities.checkResult(mapIter.hasNext());
//		me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me1.key().uintValue(), 1);
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me2.key().uintValue(), 2);
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me3.key().uintValue(), 3);
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapKeyUIntContainsElementLists_EncodeETA_decodeEMA_DecodeAll passed");
//	}
//
//	public void testMapWithKeyInt_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapKeyntContainsElementLists_EncodeETA_decodeEMA_DecodeAll");
//		System.out.println("Encode Map (key is Int) that contains ElementLists with ETA and Decode Map with EMA\n");
//
//		// Create a ETA Buffer to encode into
//		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.eta_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with ETA.
//		int retVal;
//		System.out.println("Begin ETA Map Encoding");
//		if ((retVal = TestUtilities.eta_EncodeMapKeyIntWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.eta_EncodeMapKeyIntWithELs.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End ETA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
//		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
//
//		//
//		// Check decoded Map data
//		TestUtilities.checkResult(map.hasKeyFieldId());
//		TestUtilities.checkResult(map.keyFieldId(), 3426);
//		TestUtilities.checkResult(map.hasTotalCountHint());
//		TestUtilities.checkResult(map.totalCountHint(), 3);
//
//		switch (map.summaryData().dataType())
//		{
//		case DataType.DataTypes.ELEMENT_LIST:
//			com.refinitiv.ema.access.ElementList el = map.summaryData().elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//			break;
//
//		default:
//			TestUtilities.checkResult(false);
//		}
//
//		// check first map entry
//		Iterator<MapEntry> mapIter = map.iterator();
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me1.key().intValue(), 1);
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check reseting iteration to start of map
//		mapIter = map.iterator();
//
//		// check first map entry again
//		TestUtilities.checkResult(mapIter.hasNext());
//		me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me1.key().intValue(), 1);
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me2.key().intValue(), 2);
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me3.key().intValue(), 3);
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapKeyntContainsElementLists_EncodeETA_decodeEMA_DecodeAll passed");
//	}
//
//	public void testMapWithKeyBuffer_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapWithKeyBuffer_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll");
//		System.out.println("Encode Map (key is Buffer) that contains ElementLists with ETA and Decode Map with EMA\n");
//
//		// Create a ETA Buffer to encode into
//		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.eta_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with ETA.
//		int retVal;
//		System.out.println("Begin ETA Map Encoding");
//		if ((retVal = TestUtilities.eta_EncodeMapKeyBufferWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.eta_EncodeMapKeyBufferWithELs.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End ETA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
//		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
//
//		//
//		// Check decoded Map data
//		TestUtilities.checkResult(map.hasKeyFieldId());
//		TestUtilities.checkResult(map.keyFieldId(), 3426);
//		TestUtilities.checkResult(map.hasTotalCountHint());
//		TestUtilities.checkResult(map.totalCountHint(), 3);
//
//		switch (map.summaryData().dataType())
//		{
//		case DataType.DataTypes.ELEMENT_LIST:
//			com.refinitiv.ema.access.ElementList el = map.summaryData().elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//			break;
//
//		default:
//			TestUtilities.checkResult(false);
//		}
//
//		// check first map entry
//		Iterator<MapEntry> mapIter = map.iterator();
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.BUFFER);
//
//		com.refinitiv.eta.codec.Buffer keyData = CodecFactory.createBuffer();
//		keyData.data("keyData1");
//		ByteBuffer expectedByteBuffer1 = keyData.data();
//		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer1.array()));
//
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check reseting iteration to start of map
//		mapIter = map.iterator();
//		// check first map entry again
//		TestUtilities.checkResult(mapIter.hasNext());
//		me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.BUFFER);
//		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer1.array()));
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.BUFFER);
//
//		keyData.data("keyData2");
//		ByteBuffer expectedByteBuffer2 = keyData.data();
//		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer2.array()));
//
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.BUFFER);
//
//		keyData.data("keyData3");
//		ByteBuffer expectedByteBuffer3 = keyData.data();
//		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer3.array()));
//
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapWithKeyBuffer_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll passed");
//	}
//
//	public void testMapWithKeyAsciiString_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapWithKeyAsciiString_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll");
//		System.out.println(
//				"Encode Map (key is AsciiString) that contains ElementLists with ETA and Decode Map with EMA\n");
//
//		// Create a ETA Buffer to encode into
//		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.eta_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with ETA.
//		int retVal;
//		System.out.println("Begin ETA Map Encoding");
//		if ((retVal = TestUtilities.eta_EncodeMapKeyAsciiStringWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.eta_EncodeMapKeyBufferWithELs.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End ETA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
//		JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
//
//		//
//		// Check decoded Map data
//		TestUtilities.checkResult(map.hasKeyFieldId());
//		TestUtilities.checkResult(map.keyFieldId(), 3426);
//		TestUtilities.checkResult(map.hasTotalCountHint());
//		TestUtilities.checkResult(map.totalCountHint(), 3);
//
//		switch (map.summaryData().dataType())
//		{
//		case DataType.DataTypes.ELEMENT_LIST:
//			com.refinitiv.ema.access.ElementList el = map.summaryData().elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//			break;
//
//		default:
//			TestUtilities.checkResult(false);
//		}
//
//		// check first map entry
//		Iterator<MapEntry> mapIter = map.iterator();
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData1");
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check reseting iteration to start of map
//		mapIter = map.iterator();
//		// check first map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData1");
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData2");
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.refinitiv.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.refinitiv.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData3");
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapWithKeyAsciiString_ContainsElementLists_EncodeETA_decodeEMA_DecodeAll passed");
//	}
	
	public void testMap_EncodeETAMapWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for FieldList type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
       
   		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

       // Encode Map with ETA.
       if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
       {
           System.out.println("Error encoding field list.");
           System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                   + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                   + CodecReturnCodes.info(retVal));
           return;
       }

       // Decode Map with EMA.
       com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
       JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
       
       // Copy decoded entries into a different Map with EMA     
       com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
       
       mapCopy.keyFieldId(map.keyFieldId());
       mapCopy.totalCountHint(map.totalCountHint());
       mapCopy.summaryData(map.summaryData().data());
    
       Iterator<MapEntry> iterator = map.iterator();
       while (iterator.hasNext())
       {
    	   mapCopy.add(iterator.next());
       }
       
       assertEquals(mapCopy.size(), map.size());
       
       com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
       
       JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
       
       TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);

       System.out.println("\testMap_EncodeETAMapWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeETAMapWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for ElementList type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with ETA.
      if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);

      System.out.println("\testMap_EncodeETAMapWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeETAMapWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for FilterList type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(32000));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with ETA.
      if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);

      System.out.println("\testMap_EncodeETAMapWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeETAMapWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for Series type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(14000));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with ETA.
      if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.SERIES);

      System.out.println("\testMap_EncodeETAMapWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeETAMapWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for Vector type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(14000));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with ETA.
      if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.VECTOR);

      System.out.println("\testMap_EncodeETAMapWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeETAMapWithMapType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithMapType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for Map type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with ETA.
      if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.MAP);

      System.out.println("\testMap_EncodeETAMapWithMapType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeETAMapWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeETAMapWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for RefreshMsg type, Encode it to another Map.");

		 // Create a ETA Buffer to encode into
		 com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with ETA.
      if ((retVal = TestUtilities.eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.eta_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.refinitiv.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.refinitiv.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.refinitiv.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_ETAMapKeyUIntAll(decMapCopy, com.refinitiv.eta.codec.DataTypes.MSG);

      System.out.println("\testMap_EncodeETAMapWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
}
