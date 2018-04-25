/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace std;

TEST(MapTests, testMapContainsFieldListsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, FieldList-Add, FieldList-Add, FieldList-Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_TOTAL_COUNT_HINT;

		rsslMap.containerType = RSSL_DT_FIELD_LIST;
		rsslMap.totalCountHint = 5;

		rsslMap.keyPrimitiveType = RSSL_DT_BUFFER;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABCD");
		orderBuf.length = 4;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("EFGHI");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//fourth entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("JKLMNOP");
		orderBuf.length = 7;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map contains FieldList - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map contains FieldList - getKeyFieldId()" ;
		EXPECT_TRUE( map.hasTotalCountHint() ) << "Map contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( map.getTotalCountHint(), 5 ) << "Map contains FieldList - getTotalCountHint()" ;

		switch ( map.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
		{
			const FieldList& fl = map.getSummaryData().getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Map Decode Summary FieldList - map.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( map.forth() ) << "MapEntry contains FieldList - first map forth()" ;

		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		map.reset();
		{
			EXPECT_TRUE( map.forth() ) << "Map contains FieldList - map forth() after reset()" ;

			const MapEntry& me = map.getEntry();

			EXPECT_EQ( me.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}

		EXPECT_TRUE( map.forth() ) << "Map contains FieldList - second map forth()" ;

		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map contains FieldList - third map forth()" ;

		const MapEntry& me3 = map.getEntry();

		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map contains FieldList - fourth map forth()" ;

		const MapEntry& me4 = map.getEntry();

		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me4.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map contains FieldList - final map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map contains FieldList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map contains FieldList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapContainsElementListsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with ElementList), Delete, ElementList-Add, ElementList-Add, ElementList-Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 4096;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_TOTAL_COUNT_HINT;

		rsslMap.containerType = RSSL_DT_ELEMENT_LIST;
		rsslMap.totalCountHint = 5;

		rsslMap.keyPrimitiveType = RSSL_DT_BUFFER;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the element list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeElementListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABCD");
		orderBuf.length = 4;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add ElementList
		rsslClearMapEntry( &mapEntry );
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeElementListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Add ElementList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("EFGHI");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//fourth entry  //Update ElementList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("JKLMNOP");
		orderBuf.length = 7;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map contains ElementList - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map contains ElementList - getKeyFieldId()" ;
		EXPECT_TRUE( map.hasTotalCountHint() ) << "Map contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( map.getTotalCountHint(), 5 ) << "Map contains ElelementList - getTotalCountHint()" ;

		switch ( map.getSummaryData().getDataType() )
		{
		case DataType::ElementListEnum :
		{
			const ElementList& el = map.getSummaryData().getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Map Decode Summary ElementList - map.getSummaryType() not expected" ;
			break;
		}


		EXPECT_TRUE( map.forth() ) << "Map contains ElementList - first map forth()" ;

		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		map.reset();
		{
			EXPECT_TRUE( map.forth() ) << "Map contains ElementList - map forth() after reset()" ;

			const MapEntry& me = map.getEntry();

			EXPECT_EQ( me.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}

		EXPECT_TRUE( map.forth() ) << "Map contains ElementList - second map forth()" ;

		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = me2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_TRUE( map.forth() ) << "Map contains ElementList - third map forth()" ;

		const MapEntry& me3 = map.getEntry();

		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = me3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_TRUE( map.forth() ) << "Map contains ElementList - fourth map forth()" ;

		const MapEntry& me4 = map.getEntry();

		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me4.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = me3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_FALSE( map.forth() ) << "Map contains ElementList - final map forth()" ;

		free( rsslBuf.data );

		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map contains ElementList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map contains ElementList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapContainsMapsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with Map), Delete, Map-Add, Map-Add, Map-Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 4096;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_TOTAL_COUNT_HINT;

		rsslMap.containerType = RSSL_DT_MAP;
		rsslMap.totalCountHint = 5;

		rsslMap.keyPrimitiveType = RSSL_DT_BUFFER;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the map for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 2048;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslEncodeMapAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;


		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete Buffer
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABCD");
		orderBuf.length = 4;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add Map
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the map for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 2048;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 2048 );
		RsslEncodeMapAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Add Map
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("EFGHI");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//fourth entry  //Update Map
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("JKLMNOP");
		orderBuf.length = 7;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map contains Map - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map contains Map - getKeyFieldId()" ;
		EXPECT_TRUE( map.hasTotalCountHint() ) << "Map contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( map.getTotalCountHint(), 5 ) << "Map contains Map - getTotalCountHint()" ;

		switch ( map.getSummaryData().getDataType() )
		{
		case DataType::MapEnum :
		{
			const Map& mapS = map.getSummaryData().getMap();
			EXPECT_TRUE( mapS.hasKeyFieldId() ) << "Map Decode Summary Map - hasKeyFieldId()" ;
			EXPECT_EQ( mapS.getKeyFieldId(), 3426 ) << "Map Decode Summary Map - getKeyFieldId()" ;

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Map - first map forth()" ;
			const MapEntry& me1a = mapS.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Map - second map forth()" ;
			const MapEntry& me2a = mapS.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Map - third map forth()" ;
			const MapEntry& me3a = mapS.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me3a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Mapp - fourth map forth()" ;
			const MapEntry& me4a = mapS.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me4a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_FALSE( mapS.forth() ) << "Map Decode Summary Map - fifth map forth()" ;
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Map Decode Summary Map - map.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( map.forth() ) << "Map contains Map - first map forth()" ;

		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		map.reset();
		{
			EXPECT_TRUE( map.forth() ) << "Map contains Map - map forth() after reset()" ;

			const MapEntry& me = map.getEntry();

			EXPECT_EQ( me.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}

		EXPECT_TRUE( map.forth() ) << "Map contains Map - second map forth()" ;

		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::MapEnum ) << "MapEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = me2.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "MapEntry Map within map - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "MapEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me3a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me4a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_FALSE( map.forth() ) << "MapEntry Map within map - fifth map forth()" ;
		}

		EXPECT_TRUE( map.forth() ) << "Map contains Map - third map forth()" ;

		const MapEntry& me3 = map.getEntry();

		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::MapEnum ) << "MapEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = me3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "MapEntry Map within map - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "MapEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me3a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me4a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_FALSE( map.forth() ) << "MapEntry Map within map - fifth map forth()" ;
		}

		EXPECT_TRUE( map.forth() ) << "Map contains Map - fourth map forth()" ;

		const MapEntry& me4 = map.getEntry();

		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me4.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::MapEnum ) << "MapEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = me4.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "MapEntry Map within map - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "MapEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me3a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me4a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_FALSE( map.forth() ) << "MapEntry Map within map - fifth map forth()" ;
		}

		EXPECT_FALSE( map.forth() ) << "Map contains Map - fifth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map contains Map - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map contains Map - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapContainsOpaqueDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslBuffer mapBuffer;
		mapBuffer.length = 4096;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_TOTAL_COUNT_HINT;

		rsslMap.containerType = RSSL_DT_OPAQUE;
		rsslMap.totalCountHint = 2;

		rsslMap.keyPrimitiveType = RSSL_DT_ASCII_STRING;
		rsslMap.keyFieldId = 235;

		RsslRet ret = rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		rsslClearMapEntry( &mapEntry );

		char buffer[100];
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 100;
		rsslBuf1.data = buffer;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf1, &opaqueValue );

		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;

		RsslBuffer key1;
		key1.data = const_cast<char*>("Key1");
		key1.length = 4;

		ret = rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &key1 );

		rsslClearMapEntry( &mapEntry );

		char buffer2[100];
		RsslBuffer rsslBuf2;
		rsslBuf2.length = 14;
		rsslBuf2.data = buffer2;

		RsslBuffer opaqueValue2;
		opaqueValue2.data = ( char * )"fsjkf9372uhh20";
		opaqueValue2.length = static_cast<rtrUInt32>( strlen( opaqueValue2.data ) );

		encodeNonRWFData( &rsslBuf2, &opaqueValue2 );

		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf2;

		RsslBuffer key2;
		key2.data = const_cast<char*>("Key2");
		key2.length = 4;

		ret = rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &key2 );

		ret = rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );

		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.forth() ) << "Map contains Opaque - first map forth()" ;

		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me1.getKey().getAscii(), EmaString( "Key1" ) ) << "MapEntry::getKey().getAscii()";
		EXPECT_EQ( me1.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::OpaqueEnum ) << "MapEntry::getLoad().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo1( opaqueValue.data, opaqueValue.length );
		EXPECT_EQ( me1.getOpaque().getBuffer(),  compareTo1 ) << "MapEntry::getOpaque().getBuffer()";

		EXPECT_TRUE( map.forth() ) << "Map contains Opaque - second map forth()" ;

		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me2.getKey().getAscii(), EmaString( "Key2" ) ) << "MapEntry::getKey().getAscii()";
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::OpaqueEnum ) << "MapEntry::getLoad().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo2( opaqueValue2.data, opaqueValue2.length );
		EXPECT_EQ( me2.getOpaque().getBuffer(), compareTo2 ) << "MapEntry::getOpaque().getBuffer()";
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map Decode with Opaque payload - exception not expected" ;
	}
}

TEST(MapTests, testMapContainsXmlDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslBuffer mapBuffer;
		mapBuffer.length = 4096;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_TOTAL_COUNT_HINT;

		rsslMap.containerType = RSSL_DT_XML;
		rsslMap.totalCountHint = 2;

		rsslMap.keyPrimitiveType = RSSL_DT_ASCII_STRING;
		rsslMap.keyFieldId = 235;

		RsslRet ret = rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		rsslClearMapEntry( &mapEntry );

		char buffer[200];
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 200;
		rsslBuf1.data = buffer;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf1, &xmlValue );

		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;

		RsslBuffer key1;
		key1.data = const_cast<char*>("Key1");
		key1.length = 4;

		ret = rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &key1 );

		rsslClearMapEntry( &mapEntry );

		char buffer2[100];
		RsslBuffer rsslBuf2;
		rsslBuf2.length = 100;
		rsslBuf2.data = buffer2;

		RsslBuffer xmlValue2;
		xmlValue2.data = ( char* )"<value> KLMNOPQR </value>";
		xmlValue2.length = static_cast<rtrUInt32>( strlen( xmlValue2.data ) );

		encodeNonRWFData( &rsslBuf2, &xmlValue2 );

		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf2;

		RsslBuffer key2;
		key2.data = const_cast<char*>("Key2");
		key2.length = 4;

		ret = rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &key2 );

		ret = rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );

		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.forth() ) << "Map contains Xml - first map forth()" ;

		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me1.getKey().getAscii(), EmaString( "Key1" ) ) << "MapEntry::getKey().getAscii()";
		EXPECT_EQ( me1.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::XmlEnum ) << "MapEntry::getLoad().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( me1.getXml().getBuffer(), compareTo ) << "MapEntry::getXml().getBuffer()" ;

		EXPECT_TRUE( map.forth() ) << "Map contains Xml - second map forth()" ;

		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me2.getKey().getAscii(), EmaString( "Key2" ) ) << "MapEntry::getKey().getAscii()";
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::XmlEnum ) << "MapEntry::getLoad().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo2( xmlValue2.data, xmlValue2.length );
		EXPECT_EQ( me2.getXml().getBuffer(), compareTo2 ) << "MapEntry::getXml().getBuffer()";
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map Decode with Xml payload - exception not expected" ;
	}
}

TEST(MapTests, testMapContainsAnsiPageDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslBuffer mapBuffer;
		mapBuffer.length = 4096;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_TOTAL_COUNT_HINT;

		rsslMap.containerType = RSSL_DT_ANSI_PAGE;
		rsslMap.totalCountHint = 2;

		rsslMap.keyPrimitiveType = RSSL_DT_ASCII_STRING;
		rsslMap.keyFieldId = 235;

		RsslRet ret = rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		rsslClearMapEntry( &mapEntry );

		char buffer[100];
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 100;
		rsslBuf1.data = buffer;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"ur20#-43w5wjfa924irsjf%#&#@jrw398";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf1, &ansiPageValue );

		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;

		RsslBuffer key1;
		key1.data = const_cast<char*>("Key1");
		key1.length = 4;

		ret = rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &key1 );

		rsslClearMapEntry( &mapEntry );

		char buffer2[100];
		RsslBuffer rsslBuf2;
		rsslBuf2.length = 100;
		rsslBuf2.data = buffer2;

		RsslBuffer ansiPageValue2;
		ansiPageValue2.data = ( char* )"iwrwo9348$*@JIH92hf";
		ansiPageValue2.length = static_cast<rtrUInt32>( strlen( ansiPageValue2.data ) );

		encodeNonRWFData( &rsslBuf2, &ansiPageValue2 );

		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf2;

		RsslBuffer key2;
		key2.data = const_cast<char*>("Key2");
		key2.length = 4;

		ret = rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &key2 );

		ret = rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );

		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.forth() ) << "Map contains AnsiPage - first map forth()" ;

		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me1.getKey().getAscii(), EmaString( "Key1" ) ) << "MapEntry::getKey().getAscii()";
		EXPECT_EQ( me1.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::AnsiPageEnum ) << "MapEntry::getLoad().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer comnpareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( me1.getAnsiPage().getBuffer(), comnpareTo ) << "MapEntry::getAnsiPage().getBuffer()" ;

		EXPECT_TRUE( map.forth() ) << "Map contains AnsiPage - second map forth()" ;

		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me2.getKey().getAscii(), EmaString( "Key2" ) ) << "MapEntry::getKey().getAscii()";
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::AnsiPageEnum ) << "MapEntry::getLoad().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo2( ansiPageValue2.data, ansiPageValue2.length );
		EXPECT_EQ( me2.getAnsiPage().getBuffer(), compareTo2 ) << "MapEntry::getAnsiPage().getBuffer()";
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map Decode with AnsiPage payload - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyBufferDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_BUFFER;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABCD");
		orderBuf.length = 4;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("EFGHI");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("JKLMNOP");
		orderBuf.length = 7;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Buffer - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Buffer - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Buffer - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Buffer - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Buffer - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Buffer - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Buffer - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Buffer - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyIntDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_INT;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslInt orderInt = 1;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderInt );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderInt = 2;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderInt );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderInt = 3;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderInt );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Int - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Int - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Int - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 1 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Int - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 2 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Int - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 3 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Int - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Int - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Int - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyUIntDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_UINT;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslUInt orderUInt = 1;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderUInt );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderUInt = 2;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderUInt );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderUInt = 3;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderUInt );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key UInt - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key UInt - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key UInt - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me1.getKey().getUInt(), 1 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key UInt - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me2.getKey().getUInt(), 2 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key UInt - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me3.getKey().getUInt(), 3 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key UInt - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key UInt - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key UInt - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyRealDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_REAL;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslReal orderReal;
		orderReal.isBlank = RSSL_FALSE;
		orderReal.hint = RSSL_RH_EXPONENT_2;
		orderReal.value = 11;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderReal );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderReal.isBlank = RSSL_FALSE;
		orderReal.hint = RSSL_RH_FRACTION_2;
		orderReal.value = 22;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderReal );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderReal.isBlank = RSSL_FALSE;
		orderReal.hint = RSSL_RH_FRACTION_2;
		orderReal.value = 33;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderReal );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Real - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Real - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Real - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me1.getKey().getReal().getMantissa(), 11 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me1.getKey().getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Real - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me2.getKey().getReal().getMantissa(), 22 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me2.getKey().getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Real - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me3.getKey().getReal().getMantissa(), 33 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me3.getKey().getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Real - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Real - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Real - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyFloatDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{


		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_FLOAT;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslFloat orderFloat = 11.11f;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderFloat );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderFloat = 22.22f;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderFloat );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderFloat = 33.33f;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderFloat );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Float - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Float - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Float - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me1.getKey().getFloat(), 11.11f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Float - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me2.getKey().getFloat(), 22.22f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Float - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me1.getKey().getFloat(), 33.33f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Float - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Float - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Float - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyDoubleDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_DOUBLE;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslDouble orderDouble = 11.11f;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDouble );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderDouble = 22.22f;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDouble );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderDouble = 33.33f;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDouble );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Double - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Double - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Double - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me1.getKey().getDouble(), 11.11f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Double - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me2.getKey().getDouble(), 22.22f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Double - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me1.getKey().getDouble(), 33.33f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Double - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Double - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Double - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyDateDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_DATE;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslDate orderDate;
		orderDate.year = 1111;
		orderDate.month = 11;
		orderDate.day = 1;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDate );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderDate.year = 2222;
		orderDate.month = 2;
		orderDate.day = 2;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDate );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderDate.year = 3333;
		orderDate.month = 3;
		orderDate.day = 3;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDate );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Date - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Date - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Date - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me1.getKey().getDate().getYear(), 1111 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me1.getKey().getDate().getMonth(), 11 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me1.getKey().getDate().getDay(), 1 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Date - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me2.getKey().getDate().getYear(), 2222 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me2.getKey().getDate().getMonth(), 2 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me2.getKey().getDate().getDay(), 2 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Date - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me3.getKey().getDate().getYear(), 3333 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me3.getKey().getDate().getMonth(), 3 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me3.getKey().getDate().getDay(), 3 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Date - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Date - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Date - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyTimeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_TIME;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslTime orderTime;
		orderTime.hour = 02;
		orderTime.minute = 03;
		orderTime.second = 04;
		orderTime.millisecond = 05;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderTime );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderTime.hour = 04;
		orderTime.minute = 05;
		orderTime.second = 06;
		orderTime.millisecond = 07;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderTime );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderTime.hour = 14;
		orderTime.minute = 15;
		orderTime.second = 16;
		orderTime.millisecond = 17;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderTime );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Time - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Time - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Time - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me1.getKey().getTime().getHour(), 02 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me1.getKey().getTime().getMinute(), 03 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me1.getKey().getTime().getSecond(), 04 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me1.getKey().getTime().getMillisecond(), 05 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Time - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me2.getKey().getTime().getHour(), 04 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me2.getKey().getTime().getMinute(), 05 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me2.getKey().getTime().getSecond(), 06 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me2.getKey().getTime().getMillisecond(), 07 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Time - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me3.getKey().getTime().getHour(), 14 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me3.getKey().getTime().getMinute(), 15 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me3.getKey().getTime().getSecond(), 16 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me3.getKey().getTime().getMillisecond(), 17 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Time - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Time - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Time - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyDateTimeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_DATETIME;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslDateTime orderDateTime;
		orderDateTime.date.year = 1111;
		orderDateTime.date.month = 11;
		orderDateTime.date.day = 1;
		orderDateTime.time.hour = 14;
		orderDateTime.time.minute = 15;
		orderDateTime.time.second = 16;
		orderDateTime.time.millisecond = 17;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDateTime );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderDateTime.date.year = 2222;
		orderDateTime.date.month = 2;
		orderDateTime.date.day = 2;
		orderDateTime.time.hour = 14;
		orderDateTime.time.minute = 15;
		orderDateTime.time.second = 16;
		orderDateTime.time.millisecond = 17;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDateTime );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderDateTime.date.year = 3333;
		orderDateTime.date.month = 3;
		orderDateTime.date.day = 3;
		orderDateTime.time.hour = 14;
		orderDateTime.time.minute = 15;
		orderDateTime.time.second = 16;
		orderDateTime.time.millisecond = 17;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderDateTime );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key DateTime - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key DateTime - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key DateTime - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me1.getKey().getDateTime().getYear(), 1111 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMonth(), 11 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getDay(), 1 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key DateTime - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me2.getKey().getDateTime().getYear(), 2222 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMonth(), 2 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getDay(), 2 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key DateTime - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me3.getKey().getDateTime().getYear(), 3333 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMonth(), 3 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getDay(), 3 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key DateTime - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key DateTime - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key DateTime - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyQosDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_QOS;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslQos orderQos;
		orderQos.timeliness = RSSL_QOS_TIME_REALTIME;
		orderQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		orderQos.dynamic = 1;
		orderQos.rateInfo = 0;
		orderQos.timeInfo = 0;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderQos );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderQos.timeliness = RSSL_QOS_TIME_REALTIME;
		orderQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		orderQos.dynamic = 1;
		orderQos.rateInfo = 9999;
		orderQos.timeInfo = 0;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderQos );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderQos.timeliness = RSSL_QOS_TIME_DELAYED;
		orderQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		orderQos.dynamic = 1;
		orderQos.rateInfo = 0;
		orderQos.timeInfo = 5698;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderQos );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Qos - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Qos - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Qos - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me1.getKey().getQos().getTimeliness(), OmmQos:: RealTimeEnum ) << "MapEntry::getKey().getQos().getTimeliness()";
		EXPECT_EQ( me1.getKey().getQos().getRate(), OmmQos::TickByTickEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me1.getKey().getQos().getTimelinessAsString(), "RealTime" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me1.getKey().getQos().getRateAsString(), "TickByTick" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me1.getKey().getQos().toString(), "RealTime/TickByTick" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Qos - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me2.getKey().getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me2.getKey().getQos().getRate(), 9999 ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me2.getKey().getQos().getTimelinessAsString(), "RealTime" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me2.getKey().getQos().getRateAsString(), "Rate: 9999" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me2.getKey().getQos().toString(), "RealTime/Rate: 9999" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Qos - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me3.getKey().getQos().getTimeliness(), 5698 ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me3.getKey().getQos().getRate(), OmmQos::TickByTickEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me3.getKey().getQos().getTimelinessAsString(), "Timeliness: 5698" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me3.getKey().getQos().getRateAsString(), "TickByTick" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me3.getKey().getQos().toString(), "Timeliness: 5698/TickByTick" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Qos - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Qos - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Qos - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyStateDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_STATE;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslState orderState;
		orderState.streamState = RSSL_STREAM_OPEN;
		orderState.dataState = RSSL_DATA_OK;
		orderState.code = RSSL_SC_NONE;
		orderState.text.data = ( char* )"Succeeded";
		orderState.text.length = 9;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderState );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderState.streamState = RSSL_STREAM_CLOSED_RECOVER;
		orderState.dataState = RSSL_DATA_SUSPECT;
		orderState.code = RSSL_SC_TIMEOUT;
		orderState.text.data = ( char* )"Suspect Data";
		orderState.text.length = 12;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderState );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderState.streamState = RSSL_STREAM_CLOSED;
		orderState.dataState = RSSL_DATA_SUSPECT;
		orderState.code = RSSL_SC_USAGE_ERROR;
		orderState.text.data = ( char* )"Usage Error";
		orderState.text.length = 11;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderState );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key State - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key State - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key State - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me1.getKey().getState().getStreamState(), OmmState::OpenEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me1.getKey().getState().getDataState(), OmmState::OkEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me1.getKey().getState().getStatusCode(), OmmState::NoneEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me1.getKey().getState().getStatusText(), "Succeeded" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me1.getKey().getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "MapEntry::getKey().getState().toString()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key State - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me2.getKey().getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me2.getKey().getState().getDataState(), OmmState::SuspectEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me2.getKey().getState().getStatusCode(), OmmState::TimeoutEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me2.getKey().getState().getStatusText(), "Suspect Data" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me2.getKey().getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'") << "MapEntry::getKey().getState().toString()";
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key State - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me3.getKey().getState().getStreamState(), OmmState::ClosedEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me3.getKey().getState().getDataState(), OmmState::SuspectEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me3.getKey().getState().getStatusCode(), OmmState::UsageErrorEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me3.getKey().getState().getStatusText(), "Usage Error" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me3.getKey().getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "MapEntry::getKey().getState().toString()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key State - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key State - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key State - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyEnumDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_ENUM;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslEnum orderEnum = 29;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderEnum );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderEnum = 5300;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderEnum );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderEnum = 8100;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderEnum );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Enum - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Enum - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Enum - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me1.getKey().getEnum(), 29 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Enum - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me2.getKey().getEnum(), 5300 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Enum - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me3.getKey().getEnum(), 8100 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Enum - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Enum - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Enum - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyAsciiStringDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_ASCII_STRING;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABC");
		orderBuf.length = 3;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("DEFGH");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("KLMNOPQRS");
		orderBuf.length = 9;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key AsciiString - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key AsciiString - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key AsciiString - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me1.getKey().getAscii(), "ABC" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key AsciiString - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me2.getKey().getAscii(), "DEFGH" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key AsciiString - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me3.getKey().getAscii(), "KLMNOPQRS" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key AsciiString - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key AsciiString - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key AsciiString - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyUtf8StringDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_UTF8_STRING;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABC");
		orderBuf.length = 3;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("DEFGH");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("KLMNOPQRS");
		orderBuf.length = 9;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key Utf8String - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key Utf8String - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key Utf8String - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_STREQ( me1.getKey().getUtf8(), EmaBuffer( "ABC", 3 ) ) << "MapEntry::getKey().getUtf8()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key Utf8String - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_STREQ( me2.getKey().getUtf8(), EmaBuffer( "DEFGH", 5 ) ) << "MapEntry::getKey().getUtf8()" ;

		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( map.forth() ) << "Map key Utf8String - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_STREQ( me3.getKey().getUtf8(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "MapEntry::getKey().getUtf8()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key Utf8String - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key Utf8String - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Utf8String - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapKeyRmtesStringDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, Add, Update

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );
		rsslClearEncodeIterator( &mapEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );
		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_RMTES_STRING;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );
		RsslMapEntry mapEntry;

		//first entry  //Delete
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABC");
		orderBuf.length = 3;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//second entry  //Add FieldList
		rsslClearMapEntry( &mapEntry );
		// allocate buffer for the field list for MapEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("DEFGH");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		//third entry  //Update FieldList
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("KLMNOPQRS");
		orderBuf.length = 9;
		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Map
		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( map.hasKeyFieldId() ) << "Map key RmtesString - hasKeyFieldId()" ;
		EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Map key RmtesString - getKeyFieldId()" ;

		EXPECT_TRUE( map.forth() ) << "Map key RmtesString - first map forth()" ;
		const MapEntry& me1 = map.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_STREQ( me1.getKey().getRmtes().getAsUTF8(), EmaBuffer( "ABC", 3 ) ) << "MapEntry::getKey().getRmtes()" ;

		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( map.forth() ) << "Map key RmtesString - second map forth()" ;
		const MapEntry& me2 = map.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_STREQ( me2.getKey().getRmtes().getAsUTF8(), EmaBuffer( "DEFGH", 5 ) ) << "MapEntry::getKey().getRmtes()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( map.forth() ) << "Map key RmtesString - third map forth()" ;
		const MapEntry& me3 = map.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_STREQ( me3.getKey().getRmtes().getAsUTF8(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "MapEntry::getKey().getRmtes()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( map.forth() ) << "Map key RmtesString - fourth map forth()" ;

		free( rsslBuf.data );
		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "Map key RmtesString - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key RmtesString - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(MapTests, testMapEmptyEncode)
{

	try
	{
		Map map;
		map.complete();
	}
	catch ( OmmException& excp )
	{
		EmaString text;
		EXPECT_FALSE( true ) << text.append( "empty Map - exception not expected: " ).append( excp.getText() ) ;
		return;
	}
}

TEST(MapTests, testMapContainsFieldListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, FieldList-Add, FieldList-Add, FieldList-Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		char* orderBufData1 = const_cast<char*>("ABCD");
		EmaBuffer orderBuf1( orderBufData1, 4 );
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		char* orderBufData2 = const_cast<char*>("EFGHI");
		EmaBuffer orderBuf2( orderBufData2, 5 );
		mapEnc.addKeyBuffer( orderBuf2, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		char* orderBufData3 = const_cast<char*>("JKLMNOP");
		EmaBuffer orderBuf3( orderBufData3, 7 );
		mapEnc.addKeyBuffer( orderBuf3, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map contains FieldList - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map contains FieldList - getKeyFieldId()" ;
		EXPECT_TRUE( mapEnc.hasTotalCountHint() ) << "Map contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( mapEnc.getTotalCountHint(), 5 ) << "Map contains FieldList - getTotalCountHint()" ;

		switch ( mapEnc.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
		{
			const FieldList& fl = mapEnc.getSummaryData().getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Map Decode Summary FieldList - map.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains FieldList - first map forth()" ;

		const MapEntry& me1 = mapEnc.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;


		mapEnc.reset();
		{
			EXPECT_TRUE( mapEnc.forth() ) << "Map contains FieldList - map forth() after reset()" ;

			const MapEntry& me = mapEnc.getEntry();

			EXPECT_EQ( me.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( mapEnc.forth() ) << "Map contains FieldList - second map forth()" ;

		const MapEntry& me2 = mapEnc.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains FieldList - third map forth()" ;

		const MapEntry& me3 = mapEnc.getEntry();

		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains FieldList - fourth map forth()" ;

		const MapEntry& me4 = mapEnc.getEntry();

		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me4.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		EXPECT_FALSE( mapEnc.forth() ) << "Map contains FieldList - final map forth()" ;

		EXPECT_TRUE( true ) << "Map contains FieldList - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map contains FieldList - exception not expected" ;
	}
}

TEST(MapTests, testMapContainsElementListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with ElementList), Delete, ElementList-Add, ElementList-Add, ElementList-Update

		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		mapEnc.summaryData( elEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		char* orderBufData1 = const_cast<char*>("ABCD");
		EmaBuffer orderBuf1( orderBufData1, 4 );
		ElementList elEnc1;
		EmaEncodeElementListAll( elEnc1 );
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::DeleteEnum, elEnc1, permission );

		//second entry  //Add ElementList
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::AddEnum, elEnc1, permission );

		//third entry  //Add ElementList
		char* orderBufData2 = const_cast<char*>("EFGHI");
		EmaBuffer orderBuf2( orderBufData2, 5 );
		mapEnc.addKeyBuffer( orderBuf2, MapEntry::AddEnum, elEnc1, permission );

		//fourth entry  //Update ElementList
		char* orderBufData3 = const_cast<char*>("JKLMNOP");
		EmaBuffer orderBuf3( orderBufData3, 7 );
		mapEnc.addKeyBuffer( orderBuf3, MapEntry::UpdateEnum, elEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map contains ElementList - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map contains ElementList - getKeyFieldId()" ;
		EXPECT_TRUE( mapEnc.hasTotalCountHint() ) << "Map contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( mapEnc.getTotalCountHint(), 5 ) << "Map contains ElelementList - getTotalCountHint()" ;

		switch ( mapEnc.getSummaryData().getDataType() )
		{
		case DataType::ElementListEnum :
		{
			const ElementList& el = mapEnc.getSummaryData().getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Map Decode Summary ElementList - map.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains ElementList - first map forth()" ;

		const MapEntry& me1 = mapEnc.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;


		mapEnc.reset();
		{
			EXPECT_TRUE( mapEnc.forth() ) << "Map contains ElementList - map forth() after reset()" ;

			const MapEntry& me = mapEnc.getEntry();

			EXPECT_EQ( me.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( mapEnc.forth() ) << "Map contains ElementList - second map forth()" ;

		const MapEntry& me2 = mapEnc.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;

		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( me2.getElementList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains ElementList - third map forth()" ;

		const MapEntry& me3 = mapEnc.getEntry();

		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( me3.getElementList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains ElementList - fourth map forth()" ;

		const MapEntry& me4 = mapEnc.getEntry();

		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me4.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::ElementListEnum ) << "MapEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( me3.getElementList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map contains ElementList - final map forth()" ;

		EXPECT_TRUE( true ) << "Map contains ElementList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map contains ElementList - exception not expected" ;
	}
}

TEST(MapTests, testMapContainsMapsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with Map), Delete, Map-Add, Map-Add, Map-Update

		Map mapEncS;
		EmaEncodeMapAll( mapEncS );

		mapEnc.summaryData( mapEncS );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete Buffer
		char* orderBufData1 = const_cast<char*>("ABCD");
		EmaBuffer orderBuf1( orderBufData1, 4 );
		Map mapEnc1;
		EmaEncodeMapAll( mapEnc1 );
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::DeleteEnum, mapEnc1, permission );

		//second entry  //Add Map
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::AddEnum, mapEnc1, permission );

		//third entry  //Add Map
		char* orderBufData2 = const_cast<char*>("EFGHI");
		EmaBuffer orderBuf2( orderBufData2, 5 );
		mapEnc.addKeyBuffer( orderBuf2, MapEntry::AddEnum, mapEnc1, permission );

		//fourth entry  //Update Map
		char* orderBufData3 = const_cast<char*>("JKLMNOP");
		EmaBuffer orderBuf3( orderBufData3, 7 );
		mapEnc.addKeyBuffer( orderBuf3, MapEntry::UpdateEnum, mapEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map contains Map - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map contains Map - getKeyFieldId()" ;
		EXPECT_TRUE( mapEnc.hasTotalCountHint() ) << "Map contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( mapEnc.getTotalCountHint(), 5 ) << "Map contains Map - getTotalCountHint()" ;

		switch ( mapEnc.getSummaryData().getDataType() )
		{
		case DataType::MapEnum :
		{
			const Map& mapS = mapEnc.getSummaryData().getMap();
			EXPECT_TRUE( mapS.hasKeyFieldId() ) << "Map Decode Summary Map - hasKeyFieldId()" ;
			EXPECT_EQ( mapS.getKeyFieldId(), 3426 ) << "Map Decode Summary Map - getKeyFieldId()" ;

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Map - first map forth()" ;
			const MapEntry& me1a = mapS.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Map - second map forth()" ;
			const MapEntry& me2a = mapS.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me2a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Map - third map forth()" ;
			const MapEntry& me3a = mapS.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( mapS.forth() ) << "Map Decode Summary Mapp - fourth map forth()" ;
			const MapEntry& me4a = mapS.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( mapS.forth() ) << "Map Decode Summary Map - fifth map forth()" ;
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Map Decode Summary Map - map.getSummaryType() not expected" ;
			break;
		}


		EXPECT_TRUE( mapEnc.forth() ) << "Map contains Map - first map forth()" ;

		const MapEntry& me1 = mapEnc.getEntry();

		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me1.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		mapEnc.reset();
		{
			EXPECT_TRUE( mapEnc.forth() ) << "Map contains Map - map forth() after reset()" ;

			const MapEntry& me = mapEnc.getEntry();

			EXPECT_EQ( me.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( mapEnc.forth() ) << "Map contains Map - second map forth()" ;

		const MapEntry& me2 = mapEnc.getEntry();

		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me2.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::MapEnum ) << "MapEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& mapNested = me2.getMap();
			EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "MapEntry Map within map - hasKeyFieldId()" ;
			EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "MapEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - first map forth()" ;
			const MapEntry& me1a = mapNested.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - second map forth()" ;
			const MapEntry& me2a = mapNested.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me2a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - third map forth()" ;
			const MapEntry& me3a = mapNested.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = mapNested.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( mapNested.forth() ) << "MapEntry Map within map - fifth map forth()" ;
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains Map - third map forth()" ;

		const MapEntry& me3 = mapEnc.getEntry();

		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me3.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::MapEnum ) << "MapEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& mapNested = me3.getMap();
			EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "MapEntry Map within map - hasKeyFieldId()" ;
			EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "MapEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - first map forth()" ;
			const MapEntry& me1a = mapNested.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - second map forth()" ;
			const MapEntry& me2a = mapNested.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me2a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - third map forth()" ;
			const MapEntry& me3a = mapNested.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = mapNested.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( mapNested.forth() ) << "MapEntry Map within map - fifth map forth()" ;
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map contains Map - fourth map forth()" ;

		const MapEntry& me4 = mapEnc.getEntry();

		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_STREQ( me4.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::MapEnum ) << "MapEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& mapNested = me4.getMap();
			EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "MapEntry Map within map - hasKeyFieldId()" ;
			EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "MapEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - first map forth()" ;
			const MapEntry& me1a = mapNested.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - second map forth()" ;
			const MapEntry& me2a = mapNested.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me2a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - third map forth()" ;
			const MapEntry& me3a = mapNested.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( mapNested.forth() ) << "MapEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = mapNested.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( mapNested.forth() ) << "MapEntry Map within map - fifth map forth()" ;
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map contains Map - fifth map forth()" ;

		EXPECT_TRUE( true ) << "Map contains Map - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map contains Map - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyBufferEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		char* orderBufData1 = const_cast<char*>("ABCD");
		EmaBuffer orderBuf1( orderBufData1, 4 );
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyBuffer( orderBuf1, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		const char* orderBufData2 = const_cast<char*>("EFGHI");
		EmaBuffer orderBuf2( orderBufData2, 5 );
		mapEnc.addKeyBuffer( orderBuf2, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		char* orderBufData3 = const_cast<char*>("JKLMNOP");
		EmaBuffer orderBuf3( orderBufData3, 7 );
//		orderBuf2.clear();
//		orderBuf2.append(orderBuf3);
		mapEnc.addKeyBuffer( orderBuf3, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();

		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Buffer - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Buffer - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Buffer - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Buffer - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Buffer - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Buffer - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Buffer - fifth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Buffer - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Buffer - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyIntEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		Int64 orderInt = 1;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyInt( orderInt, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyInt( orderInt, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderInt = 2;
		mapEnc.addKeyInt( orderInt, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderInt = 3;
		mapEnc.addKeyInt( orderInt, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Int - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Int - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Int - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 1 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Int - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 1 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Int - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 2 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Int - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::IntEnum ) << "MapEntry::getKey().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( me1.getKey().getInt(), 3 ) << "MapEntry::getKey().getInt()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Int - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Int - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Int - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyUIntEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt64 orderUInt = 1;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyUInt( orderUInt, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyUInt( orderUInt, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderUInt = 2;
		mapEnc.addKeyUInt( orderUInt, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderUInt = 3;
		mapEnc.addKeyUInt( orderUInt, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key UInt - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key UInt - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key UInt - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me1.getKey().getUInt(), 1 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key UInt - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me1.getKey().getUInt(), 1 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key UInt - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me1.getKey().getUInt(), 2 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key UInt - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::UIntEnum ) << "MapEntry::getKey().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( me1.getKey().getUInt(), 3 ) << "MapEntry::getKey().getUInt()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key UInt - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key UInt - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key UInt - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyRealEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		Int64 mantissa = 11;
		OmmReal::MagnitudeType magnitudeType = OmmReal::ExponentNeg2Enum;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyReal( mantissa, magnitudeType, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyReal( mantissa, magnitudeType, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		mantissa = 22;
		magnitudeType = OmmReal::Divisor2Enum;
		mapEnc.addKeyReal( mantissa, magnitudeType, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		mantissa = 33;
		magnitudeType = OmmReal::Divisor2Enum;
		mapEnc.addKeyReal( mantissa, magnitudeType, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Real - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Real - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Real - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me1.getKey().getReal().getMantissa(), 11 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me1.getKey().getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Real - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me2.getKey().getReal().getMantissa(), 11 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me2.getKey().getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Real - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me3.getKey().getReal().getMantissa(), 22 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me3.getKey().getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Real - third map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::RealEnum ) << "MapEntry::getKey().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( me4.getKey().getReal().getMantissa(), 33 ) << "MapEntry::getKey().getReal().getMantissa()" ;
		EXPECT_EQ( me4.getKey().getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "MapEntry::getKey().getReal().getMagnitudeType()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Real - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Real - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Real - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyFloatEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		float orderFloat = 11.11f;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyFloat( orderFloat, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyFloat( orderFloat, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderFloat = 22.22f;
		mapEnc.addKeyFloat( orderFloat, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderFloat = 33.33f;
		mapEnc.addKeyFloat( orderFloat, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Float - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Float - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Float - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me1.getKey().getFloat(), 11.11f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Float - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me2.getKey().getFloat(), 11.11f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Float - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me3.getKey().getFloat(), 22.22f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Float - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::FloatEnum ) << "MapEntry::getKey().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( me4.getKey().getFloat(), 33.33f ) << "MapEntry::getKey().getFloat()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Float - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Float - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Float - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyDoubleEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		double orderDouble = 11.11f;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyDouble( orderDouble, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyDouble( orderDouble, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderDouble = 22.22f;
		mapEnc.addKeyDouble( orderDouble, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderDouble = 33.33f;
		mapEnc.addKeyDouble( orderDouble, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Double - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Double - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Double - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me1.getKey().getDouble(), 11.11f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Double - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me2.getKey().getDouble(), 11.11f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Double - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me3.getKey().getDouble(), 22.22f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Double - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::DoubleEnum ) << "MapEntry::getKey().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( me4.getKey().getDouble(), 33.33f ) << "MapEntry::getKey().getDouble()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Double - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Double - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Double - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyDateEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt16 orderYear = 1111;
		UInt8 orderMonth = 11;
		UInt8 orderDay = 1;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyDate( orderYear, orderMonth, orderDay, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyDate( orderYear, orderMonth, orderDay, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderYear = 2222;
		orderMonth = 2;
		orderDay = 2;
		mapEnc.addKeyDate( orderYear, orderMonth, orderDay, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderYear = 3333;
		orderMonth = 3;
		orderDay = 3;
		mapEnc.addKeyDate( orderYear, orderMonth, orderDay, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Date - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Date - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Date - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me1.getKey().getDate().getYear(), 1111 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me1.getKey().getDate().getMonth(), 11 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me1.getKey().getDate().getDay(), 1 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Date - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me2.getKey().getDate().getYear(), 1111 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me2.getKey().getDate().getMonth(), 11 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me2.getKey().getDate().getDay(), 1 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me2.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Date - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me3.getKey().getDate().getYear(), 2222 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me3.getKey().getDate().getMonth(), 2 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me3.getKey().getDate().getDay(), 2 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me3.getFieldList() );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Date - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::DateEnum ) << "MapEntry::getKey().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( me4.getKey().getDate().getYear(), 3333 ) << "MapEntry::getKey().getDate().getYear()" ;
		EXPECT_EQ( me4.getKey().getDate().getMonth(), 3 ) << "MapEntry::getKey().getDate().getMonth()" ;
		EXPECT_EQ( me4.getKey().getDate().getDay(), 3 ) << "MapEntry::getKey().getDate().getDay()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( me4.getFieldList() );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Date - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Date - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Date - exception not expected" ;
	}

}

TEST(MapTests, testMapKeyTimeEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt8 orderHour = 02;
		UInt8 orderMinute = 03;
		UInt8 orderSecond = 04;
		UInt16 orderMillisecond = 05;
		UInt16 orderMicrosecond = 06;
		UInt16 orderNanosecond = 07;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyTime( orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                   MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyTime( orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                   MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderHour = 04;
		orderMinute = 05;
		orderSecond = 06;
		orderMillisecond = 07;
		orderMicrosecond = 8;
		orderNanosecond = 9;
		mapEnc.addKeyTime( orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                   MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderHour = 14;
		orderMinute = 15;
		orderSecond = 16;
		orderMillisecond = 17;
		orderMicrosecond = 18;
		orderNanosecond = 19;
		mapEnc.addKeyTime( orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                   MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Time - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Time - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Time - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me1.getKey().getTime().getHour(), 02 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me1.getKey().getTime().getMinute(), 03 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me1.getKey().getTime().getSecond(), 04 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me1.getKey().getTime().getMillisecond(), 05 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me1.getKey().getTime().getMicrosecond(), 06 ) << "MapEntry::getKey().getTime().getMicrosecond()" ;
		EXPECT_EQ( me1.getKey().getTime().getNanosecond(), 07 ) << "MapEntry::getKey().getTime().getNanosecond()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Time - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me2.getKey().getTime().getHour(), 02 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me2.getKey().getTime().getMinute(), 03 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me2.getKey().getTime().getSecond(), 04 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me2.getKey().getTime().getMillisecond(), 05 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me2.getKey().getTime().getMicrosecond(), 06 ) << "MapEntry::getKey().getTime().getMicrosecond()" ;
		EXPECT_EQ( me2.getKey().getTime().getNanosecond(), 07 ) << "MapEntry::getKey().getTime().getNanosecond()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Time - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me3.getKey().getTime().getHour(), 04 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me3.getKey().getTime().getMinute(), 05 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me3.getKey().getTime().getSecond(), 06 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me3.getKey().getTime().getMillisecond(), 07 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me3.getKey().getTime().getMicrosecond(), 8 ) << "MapEntry::getKey().getTime().getMicrosecond()" ;
		EXPECT_EQ( me3.getKey().getTime().getNanosecond(), 9 ) << "MapEntry::getKey().getTime().getNanosecond()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Time - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::TimeEnum ) << "MapEntry::getKey().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( me4.getKey().getTime().getHour(), 14 ) << "MapEntry::getKey().getTime().getHour()" ;
		EXPECT_EQ( me4.getKey().getTime().getMinute(), 15 ) << "MapEntry::getKey().getTime().getMinute()" ;
		EXPECT_EQ( me4.getKey().getTime().getSecond(), 16 ) << "MapEntry::getKey().getTime().getSecond()" ;
		EXPECT_EQ( me4.getKey().getTime().getMillisecond(), 17 ) << "MapEntry::getKey().getTime().getMillisecond()" ;
		EXPECT_EQ( me4.getKey().getTime().getMicrosecond(), 18 ) << "MapEntry::getKey().getTime().getMicrosecond()" ;
		EXPECT_EQ( me4.getKey().getTime().getNanosecond(), 19 ) << "MapEntry::getKey().getTime().getNanosecond()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Time - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Time - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Time - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyDateTimeEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt16 orderYear = 1111;
		UInt8 orderMonth = 11;
		UInt8 orderDay = 1;
		UInt8 orderHour = 14;
		UInt8 orderMinute = 15;
		UInt8 orderSecond = 16;
		UInt16 orderMillisecond = 17;
		UInt16 orderMicrosecond = 18;
		UInt16 orderNanosecond = 19;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyDateTime( orderYear, orderMonth, orderDay, orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                       MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyDateTime( orderYear, orderMonth, orderDay, orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                       MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderYear = 2222;
		orderMonth = 2;
		orderDay = 2;
		orderHour = 14;
		orderMinute = 15;
		orderSecond = 16;
		orderMillisecond = 17;
		orderMicrosecond = 18;
		orderNanosecond = 19;
		mapEnc.addKeyDateTime( orderYear, orderMonth, orderDay, orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                       MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderYear = 3333;
		orderMonth = 3;
		orderDay = 3;
		orderHour = 14;
		orderMinute = 15;
		orderSecond = 16;
		orderMillisecond = 17;
		orderMicrosecond = 18;
		orderNanosecond = 19;
		mapEnc.addKeyDateTime( orderYear, orderMonth, orderDay, orderHour, orderMinute, orderSecond, orderMillisecond, orderMicrosecond, orderNanosecond,
		                       MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key DateTime - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key DateTime - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key DateTime - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me1.getKey().getDateTime().getYear(), 1111 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMonth(), 11 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getDay(), 1 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMicrosecond(), 18 ) << "MapEntry::getKey().getDateTime().getMicrosecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getNanosecond(), 19 ) << "MapEntry::getKey().getDateTime().getNanosecond()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key DateTime - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me2.getKey().getDateTime().getYear(), 1111 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMonth(), 11 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getDay(), 1 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getMicrosecond(), 18 ) << "MapEntry::getKey().getDateTime().getMicrosecond()" ;
		EXPECT_EQ( me2.getKey().getDateTime().getNanosecond(), 19 ) << "MapEntry::getKey().getDateTime().getNanosecond()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key DateTime - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me3.getKey().getDateTime().getYear(), 2222 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMonth(), 2 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getDay(), 2 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getMicrosecond(), 18 ) << "MapEntry::getKey().getDateTime().getMicrosecond()" ;
		EXPECT_EQ( me3.getKey().getDateTime().getNanosecond(), 19 ) << "MapEntry::getKey().getDateTime().getNanosecond()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key DateTime - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::DateTimeEnum ) << "MapEntry::getKey().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( me1.getKey().getDateTime().getYear(), 3333 ) << "MapEntry::getKey().getDateTime().getYear()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMonth(), 3 ) << "MapEntry::getKey().getDateTime().getMonth()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getDay(), 3 ) << "MapEntry::getKey().getDateTime().getDay()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getHour(), 14 ) << "MapEntry::getKey().getDateTime().getHour()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMinute(), 15 ) << "MapEntry::getKey().getDateTime().getMinute()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getSecond(), 16 ) << "MapEntry::getKey().getDateTime().getSecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMillisecond(), 17 ) << "MapEntry::getKey().getDateTime().getMillisecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getMicrosecond(), 18 ) << "MapEntry::getKey().getDateTime().getMicrosecond()" ;
		EXPECT_EQ( me1.getKey().getDateTime().getNanosecond(), 19 ) << "MapEntry::getKey().getDateTime().getNanosecond()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key DateTime - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key DateTime - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key DateTime - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyQosEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt32 orderTimeliness = OmmQos::RealTimeEnum;
		UInt32 orderRate = OmmQos::TickByTickEnum;

		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderTimeliness = OmmQos::RealTimeEnum;
		orderRate = OmmQos::JustInTimeConflatedEnum;
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderTimeliness = OmmQos::RealTimeEnum;
		orderRate = OmmQos::TickByTickEnum;
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::UpdateEnum, flEnc1, permission );

		//fifth entry  //Add FieldList
		orderTimeliness = OmmQos::InexactDelayedEnum;
		orderRate = OmmQos::JustInTimeConflatedEnum;
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::AddEnum, flEnc1, permission );

		//sixth entry  //Add FieldList
		orderTimeliness = 2569;
		orderRate = 1236;
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::AddEnum, flEnc1, permission );

		//seventh entry  //Add FieldList
		orderTimeliness = 123565;
		orderRate = 123698;
		mapEnc.addKeyQos( orderTimeliness, orderRate, MapEntry::AddEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Qos - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Qos - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me1.getKey().getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me1.getKey().getQos().getRate(), OmmQos::TickByTickEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me1.getKey().getQos().getTimelinessAsString(), "RealTime" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me1.getKey().getQos().getRateAsString(), "TickByTick" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me1.getKey().getQos().toString(), "RealTime/TickByTick" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me2.getKey().getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me2.getKey().getQos().getRate(), OmmQos::TickByTickEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me3.getKey().getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me3.getKey().getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me3.getKey().getQos().getTimelinessAsString(), "RealTime" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me3.getKey().getQos().getRateAsString(), "JustInTimeConflated" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me3.getKey().getQos().toString(), "RealTime/JustInTimeConflated" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me4.getKey().getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me4.getKey().getQos().getRate(), OmmQos::TickByTickEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - fifth map forth()" ;
		const MapEntry& me5 = mapEnc.getEntry();
		EXPECT_EQ( me5.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me5.getKey().getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me5.getKey().getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me5.getKey().getQos().getTimelinessAsString(), "InexactDelayed" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me5.getKey().getQos().getRateAsString(), "JustInTimeConflated" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me5.getKey().getQos().toString(), "InexactDelayed/JustInTimeConflated" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me5.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me5.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me5.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - sixth map forth()" ;
		const MapEntry& me6 = mapEnc.getEntry();
		EXPECT_EQ( me6.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me6.getKey().getQos().getTimeliness(), 2569 ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me6.getKey().getQos().getRate(), 1236 ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me6.getKey().getQos().getTimelinessAsString(), "Timeliness: 2569" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me6.getKey().getQos().getRateAsString(), "Rate: 1236" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me6.getKey().getQos().toString(), "Timeliness: 2569/Rate: 1236" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me6.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me6.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me6.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Qos - seventh map forth()" ;
		const MapEntry& me7 = mapEnc.getEntry();
		EXPECT_EQ( me7.getKey().getDataType(), DataType::QosEnum ) << "MapEntry::getKey().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( me7.getKey().getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "MapEntry::getKey().getQos().getTimeliness()" ;
		EXPECT_EQ( me7.getKey().getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "MapEntry::getKey().getQos().getRate()" ;
		EXPECT_STREQ( me7.getKey().getQos().getTimelinessAsString(), "InexactDelayed" ) << "MapEntry::getKey().getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( me7.getKey().getQos().getRateAsString(), "JustInTimeConflated" ) << "MapEntry::getKey().getQos().getRateAsString()" ;
		EXPECT_STREQ( me7.getKey().getQos().toString(), "InexactDelayed/JustInTimeConflated" ) << "MapEntry::getKey().getQos().toString()" ;
		EXPECT_EQ( me7.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me7.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me7.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Qos - final map forth()" ;

		EXPECT_TRUE( true ) << "Map key Qos - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Qos - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyStateEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt8 orderStatusCode = OmmState::NoneEnum;
		const EmaString orderStatusText1( "Succeeded" );
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyState( OmmState::OpenEnum, OmmState::OkEnum, orderStatusCode, orderStatusText1, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyState( OmmState::OpenEnum, OmmState::OkEnum, orderStatusCode, orderStatusText1, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderStatusCode = OmmState::TimeoutEnum;
		const EmaString orderStatusText2( "Suspect Data" );
		mapEnc.addKeyState( OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, orderStatusCode, orderStatusText2, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderStatusCode = OmmState::ErrorEnum;
		const EmaString orderStatusText3( "Usage Error" );
		mapEnc.addKeyState( OmmState::ClosedEnum, OmmState::SuspectEnum, orderStatusCode, orderStatusText3, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key State - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key State - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key State - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me1.getKey().getState().getStreamState(), OmmState::OpenEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me1.getKey().getState().getDataState(), OmmState::OkEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me1.getKey().getState().getStatusCode(), OmmState::NoneEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me1.getKey().getState().getStatusText(), "Succeeded" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me1.getKey().getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "MapEntry::getKey().getState().toString()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key State - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me2.getKey().getState().getStreamState(), OmmState::OpenEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me2.getKey().getState().getDataState(), OmmState::OkEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me2.getKey().getState().getStatusCode(), OmmState::NoneEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me2.getKey().getState().getStatusText(), "Succeeded" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me2.getKey().getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "MapEntry::getKey().getState().toString()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key State - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me3.getKey().getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me3.getKey().getState().getDataState(), OmmState::SuspectEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me3.getKey().getState().getStatusCode(), OmmState::TimeoutEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me3.getKey().getState().getStatusText(), "Suspect Data" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me3.getKey().getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'" ) <<  "MapEntry::getKey().getState().toString()";
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key State - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::StateEnum ) << "MapEntry::getKey().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( me4.getKey().getState().getStreamState(), OmmState::ClosedEnum ) << "MapEntry::getKey().getState().getStreamState()" ;
		EXPECT_EQ( me4.getKey().getState().getDataState(), OmmState::SuspectEnum ) << "MapEntry::getKey().getState().getDataState()" ;
		EXPECT_EQ( me4.getKey().getState().getStatusCode(), OmmState::ErrorEnum ) << "MapEntry::getKey().getState().getStatusCode()" ;
		EXPECT_STREQ( me4.getKey().getState().getStatusText(), "Usage Error" ) << "MapEntry::getKey().getState().getStatusText()" ;
		EXPECT_STREQ( me4.getKey().getState().toString(), "Closed / Suspect / Internal error from sender / 'Usage Error'" ) << "MapEntry::getKey().getState().toString()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key State - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key State - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key State - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyEnumEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		UInt16 orderKey = 29;
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyEnum( orderKey, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyEnum( orderKey, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		orderKey = 5300;
		const EmaString orderStatusText2( "Suspect Data" );
		mapEnc.addKeyEnum( orderKey, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		orderKey = 8100;
		mapEnc.addKeyEnum( orderKey, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Enum - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Enum - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Enum - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me1.getKey().getEnum(), 29 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Enum - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me2.getKey().getEnum(), 29 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Enum - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me3.getKey().getEnum(), 5300 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Enum - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::EnumEnum ) << "MapEntry::getKey().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( me4.getKey().getEnum(), 8100 ) << "MapEntry::getKey().getEnum()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Enum - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Enum - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Enum - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyAsciiStringEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		const EmaString orderAsciiString( "ABC" );
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyAscii( orderAsciiString, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyAscii( orderAsciiString, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		const EmaString orderAsciiString2( "DEFGH" );
		mapEnc.addKeyAscii( orderAsciiString2, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		const EmaString orderAsciiString3( "KLMNOPQRS" );
		mapEnc.addKeyAscii( orderAsciiString3, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key AsciiString - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key AsciiString - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key AsciiString - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me1.getKey().getAscii(), "ABC" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key AsciiString - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me2.getKey().getAscii(), "ABC" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key AsciiString - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me3.getKey().getAscii(), "DEFGH" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key AsciiString - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::AsciiEnum ) << "MapEntry::getKey().getDataType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( me4.getKey().getAscii(), "KLMNOPQRS" ) << "MapEntry::getKey().getAscii()" ;
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key AsciiString - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key AsciiString - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key AsciiString - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyUtf8StringEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		char* orderBufData1 = const_cast<char*>("ABC");
		EmaBuffer orderBuf1( orderBufData1, 3 );
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyUtf8( orderBuf1, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyUtf8( orderBuf1, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		const char* orderBufData2 = const_cast<char*>("DEFGH");
		EmaBuffer orderBuf2( orderBufData2, 5 );
		mapEnc.addKeyUtf8( orderBuf2, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		char* orderBufData3 = const_cast<char*>("KLMNOPQRS");
		EmaBuffer orderBuf3( orderBufData3, 9 );
		mapEnc.addKeyUtf8( orderBuf3, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key Utf8String - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key Utf8String - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Utf8String - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( me1.getKey().getUtf8(), orderBuf1 ) <<  "MapEntry::getKey().getUtf8()";
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Utf8String - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( me2.getKey().getUtf8(), orderBuf1) << "MapEntry::getKey().getUtf8()";
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Utf8String - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( me3.getKey().getUtf8(), orderBuf2 ) << "MapEntry::getKey().getUtf8()";
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key Utf8String - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::Utf8Enum ) << "MapEntry::getKey().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( me4.getKey().getUtf8(), orderBuf3 ) << "MapEntry::getKey().getUtf8()";
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key Utf8String - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key Utf8String - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key Utf8String - exception not expected" ;
	}
}

TEST(MapTests, testMapKeyRmtesStringEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Map mapEnc;
	mapEnc.totalCountHint( 5 );
	mapEnc.keyFieldId( 3426 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, Add, Add, Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		mapEnc.summaryData( flEnc );

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Delete
		char* orderBufData1 = const_cast<char*>("ABC");
		EmaBuffer orderBuf1( orderBufData1, 3 );
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		mapEnc.addKeyRmtes( orderBuf1, MapEntry::DeleteEnum, flEnc1, permission );

		//second entry  //Add FieldList
		mapEnc.addKeyRmtes( orderBuf1, MapEntry::AddEnum, flEnc1, permission );

		//third entry  //Add FieldList
		const char* orderBufData2 = const_cast<char*>("DEFGH");
		EmaBuffer orderBuf2( orderBufData2, 5 );
		mapEnc.addKeyRmtes( orderBuf2, MapEntry::AddEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		char* orderBufData3 = const_cast<char*>("KLMNOPQRS");
		EmaBuffer orderBuf3( orderBufData3, 9 );
		mapEnc.addKeyRmtes( orderBuf3, MapEntry::UpdateEnum, flEnc1, permission );

		mapEnc.complete();


		//Now do EMA decoding of Map
		StaticDecoder::setData( &mapEnc, &dictionary );


		EXPECT_TRUE( mapEnc.hasKeyFieldId() ) << "Map key RmtesString - hasKeyFieldId()" ;
		EXPECT_EQ( mapEnc.getKeyFieldId(), 3426 ) << "Map key RmtesString - getKeyFieldId()" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key RmtesString - first map forth()" ;
		const MapEntry& me1 = mapEnc.getEntry();
		EXPECT_EQ( me1.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( me1.getKey().getRmtes().getAsUTF8(), orderBuf1) << "MapEntry::getKey().getRmtes()";
		EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
		EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		EXPECT_TRUE( mapEnc.forth() ) << "Map key RmtesString - second map forth()" ;
		const MapEntry& me2 = mapEnc.getEntry();
		EXPECT_EQ( me2.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( me2.getKey().getRmtes().getAsUTF8(), orderBuf1 ) << "MapEntry::getKey().getRmtes()";
		EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key RmtesString - third map forth()" ;
		const MapEntry& me3 = mapEnc.getEntry();
		EXPECT_EQ( me3.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( me3.getKey().getRmtes().getAsUTF8(), orderBuf2) << "MapEntry::getKey().getRmtes()";
		EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
		EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_TRUE( mapEnc.forth() ) << "Map key RmtesString - fourth map forth()" ;
		const MapEntry& me4 = mapEnc.getEntry();
		EXPECT_EQ( me4.getKey().getDataType(), DataType::RmtesEnum ) << "MapEntry::getKey().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( me4.getKey().getRmtes().getAsUTF8(), orderBuf3 ) << "MapEntry::getKey().getRmtes()";
		EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
		EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = me4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( mapEnc.forth() ) << "Map key RmtesString - fourth map forth()" ;

		EXPECT_TRUE( true ) << "Map key RmtesString - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map key RmtesString - exception not expected" ;
	}
}

TEST(MapTests, testMapDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslMap rsslMap;
		RsslEncodeIterator mapEncodeIter;

		rsslClearMap( &rsslMap );

		rsslClearEncodeIterator( &mapEncodeIter );

		rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );

		rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;

		rsslMap.containerType = RSSL_DT_FIELD_LIST;

		rsslMap.keyPrimitiveType = RSSL_DT_BUFFER;
		rsslMap.keyFieldId = 3426;

		// allocate buffer for the field list
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		EmaString inText;
		encodeFieldList( rsslBuf, inText );

		rsslMap.encSummaryData = rsslBuf;

		rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );

		RsslMapEntry mapEntry;

		rsslClearMapEntry( &mapEntry );

		mapEntry.flags = RSSL_MPEF_NONE;

		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;

		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABCD");
		orderBuf.length = 4;

		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		rsslClearMapEntry( &mapEntry );

		mapEntry.flags = RSSL_MPEF_NONE;

		mapEntry.action = RSSL_MPEA_ADD_ENTRY;

		mapEntry.encData = rsslBuf;

		orderBuf.data = const_cast<char*>("EFGHI");
		orderBuf.length = 5;

		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		rsslClearMapEntry( &mapEntry );

		mapEntry.flags = RSSL_MPEF_NONE;

		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;

		mapEntry.encData = rsslBuf;

		orderBuf.data = const_cast<char*>("JKLMNOP");
		orderBuf.length = 7;

		rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

		mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );

		rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );

		Map map;

		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		free( rsslBuf.data );

		free( mapBuffer.data );

		EXPECT_TRUE( true ) << "toString Decoding of Map of FieldList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding of Map of FieldList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

void mapOfFieldList_RsslEncodeEmaDecode( bool useSetDefinitions, std::string& decodedMsg )

{
	RsslDataDictionary dictionary;

	try
	{
	        ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator( &encodeIter );
		rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );
		rsslSetEncodeIteratorBuffer( &encodeIter, &mapBuffer );

		RsslMap rsslMap;
		rsslClearMap( &rsslMap );
		if ( useSetDefinitions )
		{
			rsslMap.flags = RSSL_MPF_HAS_SET_DEFS;
		}
		rsslMap.containerType = RSSL_DT_FIELD_LIST;
		rsslMap.keyPrimitiveType = RSSL_DT_UINT;
		rsslEncodeMapInit( &encodeIter, &rsslMap, 0, 0 );

		RsslLocalFieldSetDefDb fieldSetDefDb;
		RsslFieldSetDefEntry fieldSetDefEntries[3] =
		{
			{ 22, RSSL_DT_REAL },
			{ 25, RSSL_DT_REAL_8RB },
			{ 18, RSSL_DT_TIME_3 }
		};
		if ( useSetDefinitions )
		{
			RsslFieldSetDef fieldSetDef;

			fieldSetDef.setId = 5;
			fieldSetDef.count = 3;
			fieldSetDef.pEntries = fieldSetDefEntries;
			rsslClearLocalFieldSetDefDb( &fieldSetDefDb );
			fieldSetDefDb.definitions[5] = fieldSetDef;

			rsslEncodeLocalFieldSetDefDb( &encodeIter, &fieldSetDefDb );
			rsslEncodeMapSetDefsComplete( &encodeIter, RSSL_TRUE );
		}

		RsslMapEntry mapEntry;
		rsslClearMapEntry( &mapEntry );
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.flags = RSSL_MPEF_NONE;
		const RsslUInt rsslUInt = 100212;
		rsslEncodeMapEntryInit( &encodeIter, &mapEntry, &rsslUInt, 0 );

		RsslFieldList fieldList;
		rsslClearFieldList( &fieldList );
		if ( useSetDefinitions )
		{
			fieldList.setId = 5;
			fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_ELF_HAS_SET_DATA;
			rsslEncodeFieldListInit( &encodeIter, &fieldList, &fieldSetDefDb, 0 );
		}
		else
		{
			fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
			rsslEncodeFieldListInit( &encodeIter, &fieldList, 0, 0 );
		}

		RsslFieldEntry fieldEntry;
		rsslClearFieldEntry( &fieldEntry );
		fieldEntry.fieldId = fieldSetDefEntries[0].fieldId;
		fieldEntry.dataType = RSSL_DT_REAL;
		RsslReal rsslReal;
		rsslClearReal( &rsslReal );
		rsslReal.hint = RSSL_RH_EXPONENT_2;
		rsslReal.value = 227;
		rsslEncodeFieldEntry( &encodeIter, &fieldEntry, &rsslReal );

		rsslClearFieldEntry( &fieldEntry );
		fieldEntry.fieldId = fieldSetDefEntries[1].fieldId;
		fieldEntry.dataType = RSSL_DT_REAL;
		rsslClearReal( &rsslReal );
		rsslReal.hint = RSSL_RH_EXPONENT_4;
		rsslReal.value = 22801;
		rsslEncodeFieldEntry( &encodeIter, &fieldEntry, &rsslReal );

		rsslClearFieldEntry( &fieldEntry );
		fieldEntry.fieldId = fieldSetDefEntries[2].fieldId;
		fieldEntry.dataType = RSSL_DT_TIME;
		RsslTime rsslTime;
		rsslClearTime( &rsslTime );
		rsslTime.hour = 8;
		rsslTime.minute = 39;
		rsslTime.second = 24;
		rsslEncodeFieldEntry( &encodeIter, &fieldEntry, &rsslTime );

		rsslEncodeFieldListComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeMapEntryComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeMapComplete( &encodeIter, RSSL_TRUE );

		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );
		decodedMsg = map;
	}
	catch ( const OmmException& )
	{
	  EXPECT_TRUE( false ) << "Decoding of Map of FieldList, encoded from rssl set defs - exception not expected";
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapContainsFieldListSetDefinitionsDecode)
{

	std::string msgFromStandardData;
	mapOfFieldList_RsslEncodeEmaDecode( false, msgFromStandardData );
	std::string msgFromSetDef;
	mapOfFieldList_RsslEncodeEmaDecode( true, msgFromSetDef );

	EXPECT_EQ( msgFromSetDef, msgFromStandardData ) << "Encoding from set definitions results in same decoded message." ;
}

void mapOfElementList_RsslEncodeEmaDecode( bool useSetDefinitions, std::string& decodedMsg )

{
	RsslDataDictionary dictionary;

	try
	{
	        ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";
	
		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator( &encodeIter );
		rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		RsslBuffer mapBuffer;
		mapBuffer.length = 2048;
		mapBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );
		rsslSetEncodeIteratorBuffer( &encodeIter, &mapBuffer );

		RsslMap rsslMap;
		rsslClearMap( &rsslMap );
		if ( useSetDefinitions )
		{
			rsslMap.flags = RSSL_MPF_HAS_SET_DEFS;
		}
		rsslMap.containerType = RSSL_DT_ELEMENT_LIST;
		rsslMap.keyPrimitiveType = RSSL_DT_UINT;
		rsslEncodeMapInit( &encodeIter, &rsslMap, 0, 0 );

		RsslLocalElementSetDefDb elementSetDefDb;
		RsslElementSetDefEntry elementSetDefEntries[3] =
		{
		  { {3, const_cast<char*>( "BID") }, RSSL_DT_REAL },
		  { {3, const_cast<char*>( "ASK") }, RSSL_DT_REAL_8RB },
		  { {10, const_cast<char*>( "TRADE_TIME") }, RSSL_DT_TIME_3 }
		};
		if ( useSetDefinitions )
		{
			RsslElementSetDef elementSetDef;

			elementSetDef.setId = 5;
			elementSetDef.count = 3;
			elementSetDef.pEntries = elementSetDefEntries;
			rsslClearLocalElementSetDefDb( &elementSetDefDb );
			elementSetDefDb.definitions[5] = elementSetDef;

			rsslEncodeLocalElementSetDefDb( &encodeIter, &elementSetDefDb );
			rsslEncodeMapSetDefsComplete( &encodeIter, RSSL_TRUE );
		}

		RsslMapEntry mapEntry;
		rsslClearMapEntry( &mapEntry );
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.flags = RSSL_MPEF_NONE;
		const RsslUInt rsslUInt = 100212;
		rsslEncodeMapEntryInit( &encodeIter, &mapEntry, &rsslUInt, 0 );

		RsslElementList elementList;
		rsslClearElementList( &elementList );
		if ( useSetDefinitions )
		{
			elementList.setId = 5;
			elementList.flags = RSSL_FLF_HAS_SET_ID | RSSL_ELF_HAS_SET_DATA;
			rsslEncodeElementListInit( &encodeIter, &elementList, &elementSetDefDb, 0 );
		}
		else
		{
			elementList.flags = RSSL_FLF_HAS_STANDARD_DATA;
			rsslEncodeElementListInit( &encodeIter, &elementList, 0, 0 );
		}

		RsslElementEntry elementEntry;
		rsslClearElementEntry( &elementEntry );
		elementEntry.name = elementSetDefEntries[0].name;
		elementEntry.dataType = RSSL_DT_REAL;
		RsslReal rsslReal;
		rsslClearReal( &rsslReal );
		rsslReal.hint = RSSL_RH_EXPONENT_2;
		rsslReal.value = 227;
		rsslEncodeElementEntry( &encodeIter, &elementEntry, &rsslReal );

		rsslClearElementEntry( &elementEntry );
		elementEntry.name = elementSetDefEntries[1].name;
		elementEntry.dataType = RSSL_DT_REAL;
		rsslClearReal( &rsslReal );
		rsslReal.hint = RSSL_RH_EXPONENT_4;
		rsslReal.value = 22801;
		rsslEncodeElementEntry( &encodeIter, &elementEntry, &rsslReal );

		rsslClearElementEntry( &elementEntry );
		elementEntry.name = elementSetDefEntries[2].name;
		elementEntry.dataType = RSSL_DT_TIME;
		RsslTime rsslTime;
		rsslClearTime( &rsslTime );
		rsslTime.hour = 8;
		rsslTime.minute = 39;
		rsslTime.second = 24;
		rsslEncodeElementEntry( &encodeIter, &elementEntry, &rsslTime );

		rsslEncodeElementListComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeMapEntryComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeMapComplete( &encodeIter, RSSL_TRUE );

		Map map;
		StaticDecoder::setRsslData( &map, &mapBuffer, RSSL_DT_MAP, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );
		decodedMsg = map;
	}
	catch ( const OmmException& )
	{
	  EXPECT_TRUE( false ) << "Decoding of Map of ElementList, encoded from rssl set defs - exception not expected";
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(MapTests, testMapContainsElementListSetDefinitionsDecode)
{

	std::string msgFromStandardData;
	mapOfElementList_RsslEncodeEmaDecode( false, msgFromStandardData );
	std::string msgFromSetDef;
	mapOfElementList_RsslEncodeEmaDecode( true, msgFromSetDef );

	EXPECT_EQ( msgFromSetDef, msgFromStandardData ) << "Encoding from set definitions results in same decoded message." ;
}

TEST(MapTests, testMapPrePostBindElementList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode ElementList via prebind
		Map map1;
		{
			ElementList elementList;
			const EmaBuffer permission( "PERMISSION DATA", 15 );
			map1.addKeyUInt( 0, MapEntry::AddEnum, elementList, permission );
			EmaEncodeElementListAll( elementList );
			map1.complete();
			StaticDecoder::setData( &map1, &dictionary );
		}

		// Encode ElementList via postbind
		Map map2;
		{
			ElementList elementList;
			EmaEncodeElementListAll( elementList );
			const EmaBuffer permission( "PERMISSION DATA", 15 );
			map2.addKeyUInt( 0, MapEntry::AddEnum, elementList, permission );
			map2.complete();
			StaticDecoder::setData( &map2, &dictionary );
		}

		EXPECT_STREQ( map1.toString(), map2.toString() ) << "Pre/Post-bound ElementLists are equal - exception not expected";

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound ElementLists are equal - exception not expected" ;
	}
}

TEST(MapTests, testMapPrePostBindFieldList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode FieldList via prebind
		Map map1;
		{
			FieldList fieldList;
			const EmaBuffer permission( "PERMISSION DATA", 15 );
			map1.addKeyUInt( 0, MapEntry::AddEnum, fieldList, permission );
			EmaEncodeFieldListAll( fieldList );
			map1.complete();
			StaticDecoder::setData( &map1, &dictionary );
		}

		// Encode FieldList via postbind
		Map map2;
		{
			FieldList fieldList;
			EmaEncodeFieldListAll( fieldList );
			const EmaBuffer permission( "PERMISSION DATA", 15 );
			map2.addKeyUInt( 0, MapEntry::AddEnum, fieldList, permission );
			map2.complete();
			StaticDecoder::setData( &map2, &dictionary );
		}

		EXPECT_STREQ( map1.toString(), map2.toString() ) << "Pre/Post-bound FieldLists are equal - exception not expected";

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound FieldLists are equal - exception not expected" ;
	}
}

TEST(MapTests, testMapError)
{

	{
		try
		{
			Map m;
			m.complete();
			EXPECT_TRUE( true ) << "Map::complete() on empty Map - Can encode default Map with Buffer type for Map entry key" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Map::complete() on empty Map - exception not expected" ;
		}
	}

	{
		try
		{
			Map m;
			m.totalCountHint( 2 );
			m.complete();
		}
		catch ( const OmmException& excp)
		{
			EXPECT_FALSE( true ) << "Map::complete() on empty Map with total count hint - exception not expected : " << excp.getText().c_str();
		}
	}

	{
		try
		{
			Map m;
			m.keyFieldId( 12 );
			m.totalCountHint( 2 );
			m.complete();
			EXPECT_TRUE( true ) << "Map::complete() on empty Map with total count hint and key field id - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Map::complete() on empty Map with total count hint and key field id - exception expected" ;
		}
	}

	{
		try
		{
			Map m;

			ElementList el;
			el.addBuffer( "entry", EmaBuffer( "123", 3 ) );
			m.summaryData( el );
			m.complete();
			EXPECT_FALSE( true ) << "Map::complete() on Map with not complete summary - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Map::complete() on Map  with not complete summary - exception expected" ;
		}
	}

	{
		try
		{
			Map m;

			ElementList el;
			el.addBuffer( "entry", EmaBuffer( "123", 3 ) ).complete();
			m.summaryData( el );
			m.complete();
			EXPECT_TRUE( true ) << "Map::complete() on Map with complete summary - exception not expected()" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Map::complete() on Map  with complete summary - exception not expected" ;
		}
	}

	{
		try
		{
			Map m;

			ElementList el;
			el.addBuffer( "entry", EmaBuffer( "123", 3 ) ).complete();
			m.summaryData( el );
			m.addKeyDate( 1900, 12, 12, MapEntry::AddEnum, el );
			m.complete();

			StaticDecoder::setData( &m, 0 );

			EXPECT_EQ( m.getSummaryData().getDataType(), DataType::ElementListEnum ) << "Map::getSummaryData()::getDataType()" ;

			EXPECT_TRUE( m.forth() ) << "Map::forth()" ;

			EXPECT_EQ( m.getEntry().getLoadType(), DataType::ElementListEnum ) << "Map::getEntry()::getLoadType()" ;

			EXPECT_FALSE( m.forth() ) << "Map::forth()" ;

			EXPECT_TRUE( true ) << "Map with complete summary and entry - exception not expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Map with complete summary and entry - exception not expected" ;
		}
	}

	{
		try
		{
			Map m;

			ElementList el;
			el.addBuffer( "entry", EmaBuffer( "123", 3 ) ).complete();
			m.summaryData( el );
			m.addKeyDate( 1900, 12, 12, MapEntry::AddEnum, el );
			m.addKeyAscii( "entry", MapEntry::AddEnum, el );
			m.complete();

			EXPECT_FALSE( true ) << "Map with complete summary and two entries with mixed key data type - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Map with complete summary and two entries with mixed key data type - exception expected" ;
		}
	}

	{
		try
		{
			Map m;

			ElementList el;
			el.addBuffer( "entry", EmaBuffer( "123", 3 ) ).complete();
			m.summaryData( el );
			m.addKeyDate( 1900, 12, 12, MapEntry::AddEnum, el );

			FieldList fl;
			fl.addUInt( 1, 1 ).complete();

			m.addKeyDate( 1900, 12, 12, MapEntry::AddEnum, fl );

			m.complete();

			EXPECT_FALSE( true ) << "Map with complete summary and two entries with mixed loaf data type - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Map with complete summary and two entries with mixed load data type - exception expected" ;
		}
	}

	try
	{
		Map container;

		RefreshMsg msg;

		container.summaryData( msg );

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		EXPECT_FALSE( true ) << "Map::summaryData( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Map::summaryData( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		Map container;

		RefreshMsg msg;

		msg.streamId( 10 );

		container.summaryData( msg );

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "Mapentry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::summaryData( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::summaryData( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Map container;

		GenericMsg msg;

		container.summaryData( msg );

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		EXPECT_FALSE( true ) << "Map::summaryData( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Map::summaryData( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		Map container;

		GenericMsg msg;

		msg.streamId( 10 );

		container.summaryData( msg );

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "Mapentry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::summaryData( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::summaryData( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}

	try
	{
		Map container;

		RefreshMsg msg;

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		EXPECT_FALSE( true ) << "Map::addKeyAscii( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Map::addKeyAscii( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		Map container;

		RefreshMsg msg;

		msg.streamId( 10 );

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "Mapentry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Map container;

		GenericMsg msg;

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		EXPECT_FALSE( true ) << "Map::addKeyAscii( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Map::addKeyAscii( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		Map container;

		GenericMsg msg;

		msg.streamId( 10 );

		container.addKeyAscii( "entry", MapEntry::AddEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "Mapentry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}




	try
	{
		Map container;

		ElementList el;

		container.addKeyAscii( "entry", MapEntry::DeleteEnum, el );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "MapEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( DeleteEnum, ElementList ) while ElementList is empty - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( DeleteEnum, ElementList ) while ElementList is empty - exception not expected" ;
	}

	try
	{
		Map container;

		ElementList el;

		el.addInt( "entry int", 1 ).complete();

		container.addKeyAscii( "entry", MapEntry::DeleteEnum, el );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "MapEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( DeleteEnum, ElementList ) while ElementList is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( DeleteEnum, ElementList ) while ElementList is populated - exception not expected" ;
	}





	try
	{
		Map container;

		RefreshMsg msg;

		container.addKeyAscii( "entry", MapEntry::DeleteEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "MapEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( DeleteEnum, RefreshMsg ) while RefreshMsg is empty - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( DeleteEnum, RefreshMsg ) while RefreshMsg is empty - exception not expected" ;
	}

	try
	{
		Map container;

		RefreshMsg msg;

		msg.streamId( 10 );

		container.addKeyAscii( "entry", MapEntry::DeleteEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "MapEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( DeleteEnum, RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( DeleteEnum, RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Map container;

		GenericMsg msg;

		container.addKeyAscii( "entry", MapEntry::DeleteEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "MapEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( DeleteEnum, GenericMsg ) while GenericMsg is empty - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( DeleteEnum, GenericMsg ) while GenericMsg is empty - exception not expected" ;
	}

	try
	{
		Map container;

		GenericMsg msg;

		msg.streamId( 10 );

		container.addKeyAscii( "entry", MapEntry::DeleteEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Map::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "MapEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "Map::forth()" ;

		EXPECT_TRUE( true ) << "Map::addKeyAscii( DeleteEnum, GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Map::addKeyAscii( DeleteEnum, GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
}

TEST(MapTests, testMapEmpty_Encode)
{
	try
	{
		FieldList fieldList;

		fieldList.info(1, 1);

		Map map;
		map.complete();

		fieldList.addMap(1, map).complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode empty map - exception not expected with text" << exp.getText().c_str();
		return;
	}

	EXPECT_TRUE(true) << "Encode empty map successfully - exception not expected";
}

TEST(MapTests, testMapWithSummaryDataButNoEntry_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{

		FieldList summaryData;
		summaryData.addUInt(1, 3056).addEnum(15, 840).addDate(3386, 2018, 2, 28).complete();

		Map map;
		map.keyFieldId(11).totalCountHint(0).summaryData(summaryData).complete();

		ElementList elementList;

		elementList.info(1);

		elementList.addMap("1", map).complete();

		StaticDecoder::setData(&elementList, &dictionary);

		EXPECT_TRUE(elementList.forth());

		EXPECT_TRUE(elementList.getEntry().getMap().getKeyFieldId() == 11) << "Check key field ID from Map";
		EXPECT_TRUE(elementList.getEntry().getMap().getTotalCountHint() == 0) << "Check key totoal count hint from Map";

		const FieldList& decodeFieldList = elementList.getEntry().getMap().getSummaryData().getFieldList();
		EXPECT_TRUE(decodeFieldList.forth());

		EXPECT_TRUE(decodeFieldList.getEntry().getFieldId() == 1) << "Check the field ID of the first field entry";
		EXPECT_TRUE(decodeFieldList.getEntry().getUInt() == 3056) << "Check the value of the first field entry";
		EXPECT_TRUE(decodeFieldList.forth());

		EXPECT_TRUE(decodeFieldList.getEntry().getFieldId() == 15) << "Check the field ID of the second field entry";
		EXPECT_TRUE(decodeFieldList.getEntry().getEnum() == 840) << "Check the value of the second field entry";
		EXPECT_TRUE(decodeFieldList.forth());
		EXPECT_TRUE(decodeFieldList.getEntry().getFieldId() == 3386) << "Check the field ID of the third field entry";
		EXPECT_TRUE(decodeFieldList.getEntry().getDate().getYear() == 2018) << "Check the year value of the third field entry";
		EXPECT_TRUE(decodeFieldList.getEntry().getDate().getMonth() == 2) << "Check the month value of the third field entry";
		EXPECT_TRUE(decodeFieldList.getEntry().getDate().getDay() == 28) << "Check the day value of the third field entry";

		EXPECT_FALSE(decodeFieldList.forth()) << "Check whether this is an entry from FieldList";


		EXPECT_FALSE(elementList.getEntry().getMap().forth()) << "Check whether this is an entry from Map";

		EXPECT_FALSE(elementList.forth()) << "Check whether this is an entry from ElementList";
	}
	catch (const OmmException& exp)
	{
		EXPECT_TRUE(false) << "Fails to encode no entry map - exception not expected with text : " << exp.getText().c_str();
		return;
	}
}

TEST(MapTests, testMapSpecifyInvalidKeyType_Encode)
{
	try
	{
		Map map;
		map.keyType(DataType::FieldListEnum);
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Fails to encode a Map - exception expected with text : " << exp.getText().c_str();
		EXPECT_STREQ("The specified key type 'FieldList' is not a primitive type", exp.getText().c_str());
		return;
	}
}

TEST(MapTests, testMapKeyTypeAndAddEntryMismatchKeyType_Encode)
{
	try
	{
		Map map;
		map.keyFieldId(11).totalCountHint(3).keyType(DataType::IntEnum)
			.addKeyAscii("Key1", MapEntry::AddEnum)
			.addKeyAscii("Key2", MapEntry::AddEnum)
			.addKeyAscii("Key3", MapEntry::AddEnum).complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Fails to encode a map entry - exception expected with text : " << exp.getText().c_str();
		EXPECT_STREQ("Attempt to add an entry key type of Ascii while Map entry key is set to Int with the keyType() method", exp.getText().c_str());
		return;
	}

	EXPECT_TRUE(false) << "Fails to encode a map entry - exception expected";
}

TEST(MapTests, testMapKeyTypeAndAddEntry_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		FieldList fieldList;
		fieldList.addUInt(1, 3056).complete();

		FieldList fieldList2;
		fieldList2.addUInt(2, 4563).complete();

		FieldList fieldList3;
		fieldList3.addEnum(4, 9656).complete();

		Map map;
		map.keyFieldId(11).totalCountHint(3).keyType(DataType::AsciiEnum)
			.addKeyAscii("Key1", MapEntry::AddEnum, fieldList)
			.addKeyAscii("Key2", MapEntry::UpdateEnum, fieldList2)
			.addKeyAscii("Key3", MapEntry::UpdateEnum, fieldList3).complete();

		StaticDecoder::setData(&map, &dictionary);

		EXPECT_TRUE(map.getKeyFieldId() == 11) << "Check key field ID from Map";
		EXPECT_TRUE(map.getTotalCountHint() == 3) << "Check key totoal count hint from Map";

		EXPECT_TRUE(map.forth());

		EXPECT_TRUE(map.getEntry().getKey().getAscii() == "Key1") << "Check the key value of the first Map entry";
		EXPECT_TRUE(map.getEntry().getAction() == MapEntry::AddEnum ) << "Check the action value of the first Map entry";
		EXPECT_TRUE(map.getEntry().getLoadType() == DataType::FieldListEnum) << "Check the load type of the first Map entry";
		const FieldList& decodeFieldList1 = map.getEntry().getFieldList();
		
		EXPECT_TRUE(decodeFieldList1.forth());
		EXPECT_TRUE(decodeFieldList1.getEntry().getFieldId() == 1) << "Check the field ID of the field entry";
		EXPECT_TRUE(decodeFieldList1.getEntry().getUInt() == 3056) << "Check the value of the field entry";
		EXPECT_FALSE(decodeFieldList1.forth());

		EXPECT_TRUE(map.forth());

		EXPECT_TRUE(map.getEntry().getKey().getAscii() == "Key2") << "Check the key value of the first Map entry";
		EXPECT_TRUE(map.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action value of the first Map entry";
		EXPECT_TRUE(map.getEntry().getLoadType() == DataType::FieldListEnum) << "Check the load type of the first Map entry";
		const FieldList& decodeFieldList2 = map.getEntry().getFieldList();

		EXPECT_TRUE(decodeFieldList2.forth());
		EXPECT_TRUE(decodeFieldList2.getEntry().getFieldId() == 2) << "Check the field ID of the field entry";
		EXPECT_TRUE(decodeFieldList2.getEntry().getUInt() == 4563) << "Check the value of the field entry";
		EXPECT_FALSE(decodeFieldList2.forth());

		EXPECT_TRUE(map.forth());

		EXPECT_TRUE(map.getEntry().getKey().getAscii() == "Key3") << "Check the key value of the first Map entry";
		EXPECT_TRUE(map.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action value of the first Map entry";
		EXPECT_TRUE(map.getEntry().getLoadType() == DataType::FieldListEnum) << "Check the load type of the first Map entry";
		const FieldList& decodeFieldList3 = map.getEntry().getFieldList();

		EXPECT_TRUE(decodeFieldList3.forth());
		EXPECT_TRUE(decodeFieldList3.getEntry().getFieldId() == 4) << "Check the field ID of the field entry";
		EXPECT_TRUE(decodeFieldList3.getEntry().getEnum() == 9656) << "Check the value of the field entry";
		EXPECT_FALSE(decodeFieldList3.forth());

		EXPECT_FALSE(map.forth());
	}
	catch (const OmmException& exp)
	{
		EXPECT_TRUE(false) << "Fails to encode a map entry - exception expected with text : " << exp.getText().c_str();
		return;
	}
}

TEST(MapTests, testMapClear_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		FieldList summaryData;
		summaryData.addUInt(1, 4563).complete();

		Map map;

		map.keyType(DataType::DateTimeEnum).summaryData(summaryData).complete();

		StaticDecoder::setData(&map, &dictionary);

		EXPECT_TRUE(map.getSummaryData().getDataType() == DataType::FieldListEnum) << "Check the data type of summary data";

		map.clear();

		map.keyFieldId(11);
		map.addKeyAscii("Key1", MapEntry::AddEnum).complete();

		ElementList elementList;

		elementList.addMap("1", map).complete();

		StaticDecoder::setData(&elementList, NULL);

		EXPECT_TRUE(elementList.forth());

		EXPECT_TRUE(elementList.getEntry().getName() == "1") << "Check element list key value";

		const Map& mapDecode = elementList.getEntry().getMap();

		EXPECT_TRUE(mapDecode.forth());

		EXPECT_TRUE(mapDecode.getEntry().getKey().getAscii() == "Key1") << "Check the key value of the first Map entry";
		EXPECT_TRUE(mapDecode.getEntry().getAction() == MapEntry::AddEnum) << "Check the action value of the first Map entry";

		EXPECT_FALSE(mapDecode.forth());
		EXPECT_FALSE(elementList.forth());
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode - exception not expected with text : " << exp.getText().c_str();
		return;
	}
}

TEST(MapTests, testMapEntryKeyAsciiWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyAscii("ITEM1", MapEntry::AddEnum);
		encodedMap.addKeyAscii("ITEM2", MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyAscii("ITEM3", MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getAscii() == "ITEM1") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getAscii() == "ITEM2") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData ) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getAscii() == "ITEM3") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyBufferWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		EmaBuffer keyBuffer1;
		keyBuffer1.setFrom("KeyBuffer1", 10);

		EmaBuffer keyBuffer2;
		keyBuffer2.setFrom("KeyBuffer2", 10);

		EmaBuffer keyBuffer3;
		keyBuffer3.setFrom("KeyBuffer2", 10);

		encodedMap.addKeyBuffer(keyBuffer1, MapEntry::AddEnum);
		encodedMap.addKeyBuffer(keyBuffer2, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyBuffer(keyBuffer3, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getBuffer() == keyBuffer1) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getBuffer() == keyBuffer2) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getBuffer() == keyBuffer3) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyDateWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyDate(2018, 01, 02, MapEntry::AddEnum);
		encodedMap.addKeyDate(2019, 02, 03, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyDate(2020, 03, 04, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDate().toString() == "02 JAN 2018") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDate().toString() == "03 FEB 2019") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDate().toString() == "04 MAR 2020") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyDateTimeWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyDateTime(2021, 04, 05, 1, 2, 3, 4, 5, 6, MapEntry::AddEnum);
		encodedMap.addKeyDateTime(2022, 05, 06, 2, 3, 4, 5, 6, 7, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyDateTime(2023, 06, 07, 3, 4, 5, 6, 7, 8, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDateTime().toString() == "05 APR 2021 01:02:03:004:005:006") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDateTime().toString() == "06 MAY 2022 02:03:04:005:006:007") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDateTime().toString() == "07 JUN 2023 03:04:05:006:007:008") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyDoubleWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyDouble(56.55, MapEntry::AddEnum);
		encodedMap.addKeyDouble(65.66, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyDouble(75.77, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDouble() ==  56.55) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDouble() == 65.66) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getDouble() == 75.77) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyEnumWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyEnum(55, MapEntry::AddEnum);
		encodedMap.addKeyEnum(66, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyEnum(77, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getEnum() == 55) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getEnum() == 66) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getEnum() == 77) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyFloatWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyFloat(123.4f, MapEntry::AddEnum);
		encodedMap.addKeyFloat(234.5f, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyFloat(345.6f, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getFloat() == 123.4f) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getFloat() == 234.5f) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getFloat() == 345.6f) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyQosWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyQos(OmmQos::RealTimeEnum, OmmQos::JustInTimeConflatedEnum, MapEntry::AddEnum);
		encodedMap.addKeyQos(OmmQos::InexactDelayedEnum, 5, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyQos(15, OmmQos::TickByTickEnum, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getQos().toString() == "RealTime/JustInTimeConflated") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getQos().toString() == "InexactDelayed/Rate: 5") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getQos().toString() == "Timeliness: 15/TickByTick") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyIntWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyInt(2147483647, MapEntry::AddEnum);
		encodedMap.addKeyInt(-2147483648ll, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyInt(555, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getInt() == 2147483647) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getInt() == -2147483648ll) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getInt() == 555) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyRealFromDoubleWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyRealFromDouble( 485.55, MapEntry::AddEnum);
		encodedMap.addKeyRealFromDouble(9956.694, MapEntry::UpdateEnum, OmmReal::ExponentPos1Enum, permissionData);
		encodedMap.addKeyRealFromDouble(1095.894, MapEntry::DeleteEnum, OmmReal::ExponentPos1Enum );
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getReal().toString() == "486") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getReal().toString() == "9960") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getReal().toString() == "1100") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyRealFromMantissaWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyReal(5678, OmmReal::ExponentNeg1Enum, MapEntry::AddEnum);
		encodedMap.addKeyReal(9956, OmmReal::ExponentNeg2Enum, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyReal(31095, OmmReal::ExponentNeg3Enum, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getReal().toString() == "567.8") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getReal().toString() == "99.56") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getReal().toString() == "31.095") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyRmtesWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		EmaBuffer rmtesKey1;
		rmtesKey1.setFrom("12345", 5);

		EmaBuffer rmtesKey2;
		rmtesKey2.setFrom("67891", 5);

		EmaBuffer rmtesKey3;
		rmtesKey3.setFrom("23456", 5);

		encodedMap.addKeyRmtes(rmtesKey1, MapEntry::AddEnum);
		encodedMap.addKeyRmtes(rmtesKey2, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyRmtes(rmtesKey3, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getRmtes().toString() == "12345") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getRmtes().toString() == "67891") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getRmtes().toString() == "23456") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyStateWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyState(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Text for key1", MapEntry::AddEnum);
		encodedMap.addKeyState(OmmState::NonStreamingEnum, OmmState::SuspectEnum, OmmState::AlreadyOpenEnum, "Text for key2", MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyState(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::InvalidArgumentEnum, "Text for key3", MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getState().toString() == "Open / Ok / None / 'Text for key1'") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getState().toString() == "Non-streaming / Suspect / Already open / 'Text for key2'") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getState().toString() == "Closed / Suspect / Invalid argument / 'Text for key3'") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyTimeWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyTime(8, 20, 30, 400, 500, 600, MapEntry::AddEnum);
		encodedMap.addKeyTime(9, 30, 40, 500, 600, 700, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyTime(10, 40, 50, 600, 700, 800, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getTime().toString() == "08:20:30:400:500:600") << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getTime().toString() == "09:30:40:500:600:700") << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getTime().toString() == "10:40:50:600:700:800") << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyUIntWithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		encodedMap.addKeyUInt(1234, MapEntry::AddEnum);
		encodedMap.addKeyUInt(2345, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyUInt(3456, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getUInt() == 1234) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getUInt() == 2345) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getUInt() == 3456) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

TEST(MapTests, testMapEntryKeyUtf8WithNoPayload_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		Map encodedMap;
		EmaBuffer permissionData;

		permissionData.setFrom("permissiondata", 14);

		EmaBuffer utf8Key1;
		utf8Key1.setFrom("34561", 5);

		EmaBuffer utf8Key2;
		utf8Key2.setFrom("78912", 5);

		EmaBuffer utf8Key3;
		utf8Key3.setFrom("12135", 5);

		encodedMap.addKeyUtf8(utf8Key1, MapEntry::AddEnum);
		encodedMap.addKeyUtf8(utf8Key2, MapEntry::UpdateEnum, permissionData);
		encodedMap.addKeyUtf8(utf8Key3, MapEntry::DeleteEnum);
		encodedMap.complete();

		StaticDecoder::setData(&encodedMap, &dictionary);

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the first map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getUtf8() == utf8Key1) << "Check the key value of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::AddEnum) << "Check the action of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the first map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the first map entry has permission data";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the second map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getUtf8() == utf8Key2) << "Check the key value of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::UpdateEnum) << "Check the action of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the second map entry";
		EXPECT_TRUE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the second map entry has permission data";
		EXPECT_TRUE(encodedMap.getEntry().getPermissionData() == permissionData) << "Compare the permission data of the second map entry";

		EXPECT_TRUE(encodedMap.forth()) << "Iterate the third map enty";

		EXPECT_TRUE(encodedMap.getEntry().getKey().getUtf8() == utf8Key3) << "Check the key value of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getAction() == MapEntry::DeleteEnum) << "Check the action of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third map entry";
		EXPECT_TRUE(encodedMap.getEntry().getLoad().getDataType() == DataType::NoDataEnum) << "Get load and check data type of the third map entry";
		EXPECT_FALSE(encodedMap.getEntry().hasPermissionData()) << "Check wheter the third map entry has permission data";

		EXPECT_FALSE(encodedMap.forth()) << "Check the end of Map";
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception not expected with text : " << excp.getText();
		return;
	}
}

