///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.OmmException;

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
			TestUtilities.checkResult( true, "empty Map - exception expected: " +  excp.getMessage()  );
			return;
		}

		TestUtilities.checkResult( false, "empty Map - did not get expected exception" );
	}
	
	public void testMapContainsPartialEmptyFieldList_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsPartialEmptyFieldList_EncodeDecodeAll","Encode Map of FieldList which has info, but no entry with EMA, decode all");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);
		
		try {
			Map mapEnc = EmaFactory.createMap();
			mapEnc.totalCountHint(1);
			mapEnc.keyFieldId(3426);

			FieldList flEnc = EmaFactory.createFieldList();
			TestUtilities.EmaEncodeFieldListAll(flEnc);

			mapEnc.summaryData(flEnc);

			ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());

			//first entry  //Delete Buffer
			ByteBuffer orderBuf =(ByteBuffer)ByteBuffer.wrap("ABCD".getBytes());
			
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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);
		
		try {
			com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1024));

			TestUtilities.upa_EncodeMapKeyIntWithEmptyFLs(buf);
			
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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

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
	
	public void testMapKeyUtf8String_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testMapKeyUtf8String_EncodeDecodeAll","Encode Map (key is UTF8) that contains FieldLists with EMA and Decode Map with EMA");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

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

	
	public void testMapWithKeyUInt_ContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll()
	{
		TestUtilities.printTestHead("testMapContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll","Encode Map (key is UInt) that contains FieldLists with UPA and Decode Map with EMA");

		// Create a UPA Buffer to encode into
		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		
		// load dictionary
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		// Encode Map with UPA.
		int retVal;
		System.out.println("Begin UPA Map Encoding");
		if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(buf, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeMapKeyUIntWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
			com.thomsonreuters.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		// check first map entry
		Iterator<MapEntry> mapIter = map.iterator();
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me1.key().uintValue(), 1);
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
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
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me2.key().uintValue(), 2);
		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.UINT);
		TestUtilities.checkResult(me3.key().uintValue(), 3);
		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll passed");
	}

	public void testMapWithKeyInt_ContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll()
	{
		System.out.println("testMapKeyIntContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll");
		System.out.println("Encode Map (key is Int) that contains FieldLists with UPA and Decode Map with EMA\n");

		// Create a UPA Buffer to encode into
		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		// Encode Map with UPA.
		int retVal;
		System.out.println("Begin UPA Map Encoding");
		if ((retVal = TestUtilities.upa_EncodeMapKeyIntWithFLs(buf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeMapKeyIntWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
			com.thomsonreuters.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		Iterator<MapEntry> mapIter = map.iterator();
		
		// check first map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me1.key().intValue(), 1);
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
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
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me2.key().intValue(), 2);
		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.INT);
		TestUtilities.checkResult(me3.key().intValue(), 3);
		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapKeyIntContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll passed");
	}

	public void testMapWithKeyAsciiString_ContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll()
	{
		System.out.println("testMapKeyAsciiStringContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll");
		System.out
				.println("Encode Map (key is AsciiString) that contains FieldLists with UPA and Decode Map with EMA\n");

		// Create a UPA Buffer to encode into
		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		// Encode Map with UPA.
		int retVal;
		System.out.println("Begin UPA Map Encoding");
		if ((retVal = TestUtilities.upa_EncodeMapKeyAsciiStringWithFLs(buf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeMapKeyAsciiStringWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
			com.thomsonreuters.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		Iterator<MapEntry> mapIter = map.iterator();
		
		// check first map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData1");
		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
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
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me2.key().ascii().toString(), "keyData2");
		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.ASCII);
		TestUtilities.checkResult(me3.key().ascii().toString(), "keyData3");
		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapKeyAsciiStringContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll passed");
	}

	public void testMapWithKeyBuffer_ContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll()
	{
		System.out.println("testMapKeyBufferContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll");
		System.out.println("Encode Map (key is Buffer) that contains FieldLists with UPA and Decode Map with EMA\n");

		// Create a UPA Buffer to encode into
		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		// Encode Map with UPA.
		int retVal;
		System.out.println("Begin UPA Map Encoding");
		if ((retVal = TestUtilities.upa_EncodeMapKeyBufferWithFLs(buf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding map.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeMapKeyBufferWithFLs.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA Map Encoding");
		System.out.println();

		// Decode Map with EMA.
		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
			com.thomsonreuters.ema.access.FieldList fl = map.summaryData().fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
			break;

		default:
			TestUtilities.checkResult(false);
		}

		// check first map entry
		Iterator<MapEntry> mapIter = map.iterator();
		
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.BUFFER);

		com.thomsonreuters.upa.codec.Buffer keyData = CodecFactory.createBuffer();
		keyData.data("keyData1");
		ByteBuffer expectedByteBuffer1 = keyData.data();
		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer1.array()));

		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
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
			com.thomsonreuters.ema.access.FieldList fl = me1.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check second map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.BUFFER);

		keyData.data("keyData2");
		ByteBuffer expectedByteBuffer2 = keyData.data();
		TestUtilities.checkResult(Arrays.equals(me2.key().buffer().buffer().array(), expectedByteBuffer2.array()));

		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.FIELD_LIST);
		{
			com.thomsonreuters.ema.access.FieldList fl = me2.fieldList();
			TestUtilities.EmaDecode_UPAFieldListAll(fl, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
		}

		// check third map entry
		TestUtilities.checkResult(mapIter.hasNext());
		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.BUFFER);

		keyData.data("keyData3");
		ByteBuffer expectedByteBuffer3 = keyData.data();
		TestUtilities.checkResult(Arrays.equals(me3.key().buffer().buffer().array(), expectedByteBuffer3.array()));

		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);

		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries

		System.out.println("\ntestMapKeyBufferContainsFieldLists_EncodeUPA_decodeEMA_DecodeAll passed");
	}

//	public void testMapWithKeyUInt_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapKeyUIntContainsElementLists_EncodeUPA_decodeEMA_DecodeAll");
//		System.out.println("Encode Map (key is UInt) that contains ElementLists with UPA and Decode Map with EMA\n");
//
//		// Create a UPA Buffer to encode into
//		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.upa_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with UPA.
//		int retVal;
//		System.out.println("Begin UPA Map Encoding");
//		if ((retVal = TestUtilities.upa_EncodeMapKeyUIntWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.upa_EncodeMapAll.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End UPA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
//			com.thomsonreuters.ema.access.ElementList el = map.summaryData().elementList();
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
//		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me1.key().uintValue(), 1);
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
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
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me2.key().uintValue(), 2);
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.UINT);
//		TestUtilities.checkResult(me3.key().uintValue(), 3);
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapKeyUIntContainsElementLists_EncodeUPA_decodeEMA_DecodeAll passed");
//	}
//
//	public void testMapWithKeyInt_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapKeyntContainsElementLists_EncodeUPA_decodeEMA_DecodeAll");
//		System.out.println("Encode Map (key is Int) that contains ElementLists with UPA and Decode Map with EMA\n");
//
//		// Create a UPA Buffer to encode into
//		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.upa_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with UPA.
//		int retVal;
//		System.out.println("Begin UPA Map Encoding");
//		if ((retVal = TestUtilities.upa_EncodeMapKeyIntWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.upa_EncodeMapKeyIntWithELs.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End UPA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
//			com.thomsonreuters.ema.access.ElementList el = map.summaryData().elementList();
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
//		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me1.key().intValue(), 1);
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
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
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me2.key().intValue(), 2);
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.INT);
//		TestUtilities.checkResult(me3.key().intValue(), 3);
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapKeyntContainsElementLists_EncodeUPA_decodeEMA_DecodeAll passed");
//	}
//
//	public void testMapWithKeyBuffer_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapWithKeyBuffer_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll");
//		System.out.println("Encode Map (key is Buffer) that contains ElementLists with UPA and Decode Map with EMA\n");
//
//		// Create a UPA Buffer to encode into
//		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.upa_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with UPA.
//		int retVal;
//		System.out.println("Begin UPA Map Encoding");
//		if ((retVal = TestUtilities.upa_EncodeMapKeyBufferWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.upa_EncodeMapKeyBufferWithELs.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End UPA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
//			com.thomsonreuters.ema.access.ElementList el = map.summaryData().elementList();
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
//		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.BUFFER);
//
//		com.thomsonreuters.upa.codec.Buffer keyData = CodecFactory.createBuffer();
//		keyData.data("keyData1");
//		ByteBuffer expectedByteBuffer1 = keyData.data();
//		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer1.array()));
//
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
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
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.BUFFER);
//
//		keyData.data("keyData2");
//		ByteBuffer expectedByteBuffer2 = keyData.data();
//		TestUtilities.checkResult(Arrays.equals(me1.key().buffer().asHex().array(), expectedByteBuffer2.array()));
//
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
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
//		System.out.println("\ntestMapWithKeyBuffer_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll passed");
//	}
//
//	public void testMapWithKeyAsciiString_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll()
//	{
//		System.out.println("testMapWithKeyAsciiString_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll");
//		System.out.println(
//				"Encode Map (key is AsciiString) that contains ElementLists with UPA and Decode Map with EMA\n");
//
//		// Create a UPA Buffer to encode into
//		com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
//		buf.data(ByteBuffer.allocate(1024));
//
//		// load dictionary
//		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
//				.createDataDictionary();
//		TestUtilities.upa_encodeDictionaryMsg(dictionary);
//
//		// Encode Map with UPA.
//		int retVal;
//		System.out.println("Begin UPA Map Encoding");
//		if ((retVal = TestUtilities.upa_EncodeMapKeyAsciiStringWithELs(buf)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("Error encoding map.");
//			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
//					+ ") encountered with TestUtilities.upa_EncodeMapKeyBufferWithELs.  " + "Error Text: "
//					+ CodecReturnCodes.info(retVal));
//			return;
//		}
//		System.out.println("End UPA Map Encoding");
//		System.out.println();
//
//		// Decode Map with EMA.
//		com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
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
//			com.thomsonreuters.ema.access.ElementList el = map.summaryData().elementList();
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
//		com.thomsonreuters.ema.access.MapEntry me1 = mapIter.next();
//		TestUtilities.checkResult(me1.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData1");
//		TestUtilities.checkResult(me1.action(), MapEntry.MapAction.UPDATE);
//		TestUtilities.checkResult(me1.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
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
//			com.thomsonreuters.ema.access.ElementList el = me1.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check second map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me2 = mapIter.next();
//		TestUtilities.checkResult(me2.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData2");
//		TestUtilities.checkResult(me2.action(), MapEntry.MapAction.ADD);
//		TestUtilities.checkResult(me2.load().dataType(), DataType.DataTypes.ELEMENT_LIST);
//		{
//			com.thomsonreuters.ema.access.ElementList el = me2.elementList();
//			TestUtilities.EmaDecodeElementListAll(el);
//		}
//
//		// check third map entry
//		TestUtilities.checkResult(mapIter.hasNext());
//		com.thomsonreuters.ema.access.MapEntry me3 = mapIter.next();
//		TestUtilities.checkResult(me3.key().dataType(), DataType.DataTypes.ASCII);
//		TestUtilities.checkResult(me1.key().ascii().toString(), "keyData3");
//		TestUtilities.checkResult(me3.action(), MapEntry.MapAction.DELETE);
//		TestUtilities.checkResult(me3.load().dataType(), DataType.DataTypes.NO_DATA);
//
//		TestUtilities.checkResult(!mapIter.hasNext()); // finalhasNext() - no more map entries
//
//		System.out.println("\ntestMapWithKeyAsciiString_ContainsElementLists_EncodeUPA_decodeEMA_DecodeAll passed");
//	}
	
	public void testMap_EncodeUPAMapWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for FieldList type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
       
   		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

       // Encode Map with UPA.
       if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
       {
           System.out.println("Error encoding field list.");
           System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                   + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                   + CodecReturnCodes.info(retVal));
           return;
       }

       // Decode Map with EMA.
       com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
       JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
       
       // Copy decoded entries into a different Map with EMA     
       com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
       
       mapCopy.keyFieldId(map.keyFieldId());
       mapCopy.totalCountHint(map.totalCountHint());
       mapCopy.summaryData(map.summaryData().data());
    
       Iterator<MapEntry> iterator = map.iterator();
       while (iterator.hasNext())
       {
    	   mapCopy.add(iterator.next());
       }
       
       assertEquals(mapCopy.size(), map.size());
       
       com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
       
       JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
       
       TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);

       System.out.println("\testMap_EncodeUPAMapWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeUPAMapWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for ElementList type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with UPA.
      if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);

      System.out.println("\testMap_EncodeUPAMapWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeUPAMapWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for FilterList type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(32000));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with UPA.
      if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);

      System.out.println("\testMap_EncodeUPAMapWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeUPAMapWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for Series type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(14000));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with UPA.
      if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.SERIES);

      System.out.println("\testMap_EncodeUPAMapWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeUPAMapWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for Vector type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(14000));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with UPA.
      if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.VECTOR);

      System.out.println("\testMap_EncodeUPAMapWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeUPAMapWithMapType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithMapType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for Map type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with UPA.
      if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.MAP);

      System.out.println("\testMap_EncodeUPAMapWithMapType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	
	public void testMap_EncodeUPAMapWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode() 
	{
		 int retVal;
		 
		 TestUtilities.printTestHead("testMap_EncodeUPAMapWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for RefreshMsg type, Encode it to another Map.");

		 // Create a UPA Buffer to encode into
		 com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		 buf.data(ByteBuffer.allocate(8192));
      
  		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

      // Encode Map with UPA.
      if ((retVal = TestUtilities.upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
      {
          System.out.println("Error encoding field list.");
          System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                  + ") encountered with TestUtilities.upa_EncodeMapKeyUIntAll for container types.  " + "Error Text: "
                  + CodecReturnCodes.info(retVal));
          return;
      }

      // Decode Map with EMA.
      com.thomsonreuters.ema.access.Map map = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(map, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      // Copy decoded entries into a different Map with EMA     
      com.thomsonreuters.ema.access.Map mapCopy = EmaFactory.createMap();
      
      mapCopy.keyFieldId(map.keyFieldId());
      mapCopy.totalCountHint(map.totalCountHint());
      mapCopy.summaryData(map.summaryData().data());
   
      Iterator<MapEntry> iterator = map.iterator();
      while (iterator.hasNext())
      {
   	   mapCopy.add(iterator.next());
      }
      
      assertEquals(mapCopy.size(), map.size());
      
      com.thomsonreuters.ema.access.Map decMapCopy = JUnitTestConnect.createMap();
      JUnitTestConnect.setRsslData(decMapCopy, mapCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
      
      TestUtilities.EmaDecode_UPAMapKeyUIntAll(decMapCopy, com.thomsonreuters.upa.codec.DataTypes.MSG);

      System.out.println("\testMap_EncodeUPAMapWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
}
