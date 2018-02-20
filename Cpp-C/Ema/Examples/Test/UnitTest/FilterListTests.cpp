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

TEST(FilterListTests, testFilterListContainsFieldListsDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  Clear, FieldList-Set, FieldList-Set, FieldList-Update

		RsslBuffer filterListBuffer;
		filterListBuffer.length = 2048;
		filterListBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslFilterList rsslFL;
		RsslEncodeIterator filterEncodeIter;

		rsslClearFilterList( &rsslFL );
		rsslClearEncodeIterator( &filterEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &filterEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &filterEncodeIter, &filterListBuffer );
		rsslFL.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT ;

		rsslFL.containerType = RSSL_DT_FIELD_LIST;
		rsslFL.totalCountHint = 4;

		rsslEncodeFilterListInit( &filterEncodeIter, &rsslFL );
		RsslFilterEntry filterEntry;

		//first entry  //Clear
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_CLEAR_ENTRY;
		filterEntry.id = 1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//second entry  //Add FieldList
		rsslClearFilterEntry( &filterEntry );
		// allocate buffer for the field list for FilterEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.id = 2;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//third entry  //Add FieldList
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		filterEntry.id = 3;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//fourth entry  //Update FieldList
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
		filterEntry.encData = rsslBuf1;
		filterEntry.id = 4;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		rsslEncodeFilterListComplete( &filterEncodeIter, RSSL_TRUE );

		//Now do EMA decoding of FilterList
		FilterList fl;
		StaticDecoder::setRsslData( &fl, &filterListBuffer, RSSL_DT_FILTER_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasTotalCountHint() ) << "FilterList contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( fl.getTotalCountHint(), 4 ) << "FilterList contains FieldList - getTotalCountHint()" ;

		EXPECT_TRUE( fl.forth() ) << "FilterList contains FieldList - first forth()" ;

		const FilterEntry& fe1 = fl.getEntry();

		EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
		EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		fl.reset();
		{
			EXPECT_TRUE( fl.forth() ) << "FilterList contains FieldList - filterList forth() after reset()" ;

			const FilterEntry& fe = fl.getEntry();

			EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
			EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains FieldList - second forth()" ;

		const FilterEntry& fe2 = fl.getEntry();

		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::FieldListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = fe2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains FieldList - third forth()" ;

		const FilterEntry& fe3 = fl.getEntry();

		EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::FieldListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = fe3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains FieldList - fourth forth()" ;

		const FilterEntry& fe4 = fl.getEntry();

		EXPECT_EQ( fe4.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::FieldListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = fe4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_FALSE( fl.forth() ) << "FilterList contains FieldList - fifth forth()" ;

		EXPECT_TRUE( true ) << "FilterList contains FieldList - exception not expected" ;

		free( filterListBuffer.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList contains FieldList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FilterListTests, testFilterListContainsElementListsDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  Clear, ElementList-Set, ElementList-Set, ElementList-Update

		RsslBuffer filterListBuffer;
		filterListBuffer.length = 2048;
		filterListBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslFilterList rsslFL;
		RsslEncodeIterator filterEncodeIter;

		rsslClearFilterList( &rsslFL );
		rsslClearEncodeIterator( &filterEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &filterEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &filterEncodeIter, &filterListBuffer );
		rsslFL.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT ;

		rsslFL.containerType = RSSL_DT_ELEMENT_LIST;
		rsslFL.totalCountHint = 5;

		rsslEncodeFilterListInit( &filterEncodeIter, &rsslFL );
		RsslFilterEntry filterEntry;

		//first entry  //Clear
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_CLEAR_ENTRY;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//second entry  //Set ElementList
		rsslClearFilterEntry( &filterEntry );
		// allocate buffer for the element list for FilterEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeElementListAll( rsslBuf1 );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//third entry  //Set ElementList
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//fourth entry  //Update ElementList
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		rsslEncodeFilterListComplete( &filterEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of FilterList
		FilterList fl;
		StaticDecoder::setRsslData( &fl, &filterListBuffer, RSSL_DT_FILTER_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasTotalCountHint() ) << "FilterList contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( fl.getTotalCountHint(), 5 ) << "FilterList contains ElementList - getTotalCountHint()" ;


		EXPECT_TRUE( fl.forth() ) << "FilterList contains ElementList - first forth()" ;

		const FilterEntry& fe1 = fl.getEntry();

		EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
		EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;

		fl.reset();
		{
			EXPECT_TRUE( fl.forth() ) << "FilterList contains ElementList - filterList forth() after reset()" ;

			const FilterEntry& fe = fl.getEntry();

			EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
			EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains ElementList - second forth()" ;

		const FilterEntry& fe2 = fl.getEntry();

		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::ElementListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = fe2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains ElementList - third forth()" ;

		const FilterEntry& fe3 = fl.getEntry();

		EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::ElementListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = fe3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_FALSE( fl.forth() ) << "FilterList contains ElementList - fifth forth()" ;

		EXPECT_TRUE( true ) << "FilterList contains ElementList - exception not expected" ;

		free( filterListBuffer.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList contains ElementList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FilterListTests, testFilterListContainsMapsDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  Clear, Map-Set, Map-Set, Map-Update

		RsslBuffer filterListBuffer;
		filterListBuffer.length = 2048;
		filterListBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslFilterList rsslFL;
		RsslEncodeIterator filterEncodeIter;

		rsslClearFilterList( &rsslFL );
		rsslClearEncodeIterator( &filterEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &filterEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &filterEncodeIter, &filterListBuffer );
		rsslFL.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT ;

		rsslFL.containerType = RSSL_DT_MAP;
		rsslFL.totalCountHint = 5;

		rsslEncodeFilterListInit( &filterEncodeIter, &rsslFL );
		RsslFilterEntry filterEntry;

		//second entry  //Add Map
		rsslClearFilterEntry( &filterEntry );
		// allocate buffer for the field list for FilterEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeMapAll( rsslBuf1 );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//third entry  //Add Map
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//fourth entry  //Update Map
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		rsslEncodeFilterListComplete( &filterEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of FilterList
		FilterList fl;
		StaticDecoder::setRsslData( &fl, &filterListBuffer, RSSL_DT_FILTER_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasTotalCountHint() ) << "FilterList contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( fl.getTotalCountHint(), 5 ) << "FilterList contains Map - getTotalCountHint()" ;

		EXPECT_TRUE( fl.forth() ) << "FilterList contains Map - second forth()" ;

		const FilterEntry& fe2 = fl.getEntry();

		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::MapEnum ) << "FilterEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = fe2.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FilterEntry Map within filterList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FilterEntry Map within filterList - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "EFGHI", 5 );
				EXPECT_STREQ( me3a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me3a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "JKLMNOP", 7 );
				EXPECT_STREQ( me4a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me4a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_FALSE( map.forth() ) << "FilterEntry Map within filterList - fifth map forth()" ;
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains Map - third forth()" ;

		const FilterEntry& fe3 = fl.getEntry();

		EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::MapEnum ) << "FilterEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = fe3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FilterEntry Map within filterList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FilterEntry Map within filterList - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "FilterEntry Map within filterList - fifth map forth()" ;
		}


		EXPECT_TRUE( fl.forth() ) << "FilterList contains FieldList - fourth forth()" ;

		const FilterEntry& fe4 = fl.getEntry();

		EXPECT_EQ( fe4.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::MapEnum ) << "FilterEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = fe3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FilterEntry Map within filterList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FilterEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "FilterEntry Map within filterList - fifth map forth()" ;
		}


		EXPECT_FALSE( fl.forth() ) << "FilterList contains Map - fifth forth()" ;

		EXPECT_TRUE( true ) << "FilterList contains Map - exception not expected" ;

		free( filterListBuffer.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList contains Map - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FilterListTests, testFilterListContainsOpaqueXmlAnsiPageDecodeAll)
{

	RsslDataDictionary dictionary;

	RsslBuffer filterListBuffer;
	filterListBuffer.length = 2048;
	filterListBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

	RsslFilterList rsslFL;
	RsslEncodeIterator filterEncodeIter;

	rsslClearFilterList( &rsslFL );
	rsslClearEncodeIterator( &filterEncodeIter );
	rsslSetEncodeIteratorRWFVersion( &filterEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &filterEncodeIter, &filterListBuffer );
	rsslFL.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT;
	rsslFL.containerType = RSSL_DT_OPAQUE;

	rsslFL.totalCountHint = 3;

	RsslRet ret = rsslEncodeFilterListInit( &filterEncodeIter, &rsslFL );
	RsslFilterEntry filterEntry;

	rsslClearFilterEntry( &filterEntry );

	char buffer[200];
	RsslBuffer rsslBuf;
	rsslBuf.length = 200;
	rsslBuf.data = ( char* )buffer;

	filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.id = 1;
	filterEntry.containerType = RSSL_DT_OPAQUE;

	RsslBuffer opaqueValue;
	opaqueValue.data = ( char* )"482wfshfsrf2";
	opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

	encodeNonRWFData( &rsslBuf, &opaqueValue );

	filterEntry.encData.data = rsslBuf.data;
	filterEntry.encData.length = rsslBuf.length;

	ret =  rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

	rsslClearFilterEntry( &filterEntry );

	filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.id = 2;
	filterEntry.containerType = RSSL_DT_XML;

	RsslBuffer xmlValue;
	xmlValue.data = ( char* )"<value> KLMNOPQR </value>";
	xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

	encodeNonRWFData( &rsslBuf, &xmlValue );

	filterEntry.encData = rsslBuf;

	ret =  rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

	rsslClearFilterEntry( &filterEntry );

	filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.encData = rsslBuf;
	filterEntry.id = 3;
	filterEntry.containerType = RSSL_DT_ANSI_PAGE;

	RsslBuffer ansiPageValue;
	ansiPageValue.data = ( char* )"328-srfsjkj43rouw-01-20ru2l24903$%";
	ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

	encodeNonRWFData( &rsslBuf, &ansiPageValue );

	filterEntry.encData = rsslBuf;

	ret =  rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

	ret = rsslEncodeFilterListComplete( &filterEncodeIter, RSSL_TRUE );

	FilterList fl;
	StaticDecoder::setRsslData( &fl, &filterListBuffer, RSSL_DT_FILTER_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

	EXPECT_TRUE( fl.hasTotalCountHint() ) << "FilterList contains Opaque - hasTotalCountHint()" ;
	EXPECT_EQ( fl.getTotalCountHint(), 3 ) << "FilterList contains Opaque - getTotalCountHint()" ;

	EXPECT_TRUE( fl.forth() ) << "FilterList contains Opaque - first forth()" ;

	const FilterEntry& fe1 = fl.getEntry();

	EXPECT_EQ( fe1.getFilterId(), 1 ) << "fe1.getFilterId() == 1" ;
	EXPECT_EQ( fe1.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
	EXPECT_EQ( fe1.getLoad().getDataType(), DataType::OpaqueEnum ) << "FilterEntry::getLoad().getDataType() == DataType::OpaqueEnum" ;
	{
		EmaBuffer Buf( "482wfshfsrf2", 12 );
		EXPECT_STREQ( fe1.getOpaque().getBuffer(), Buf ) << "ElementEntry::getOpaque()" ;
	}

	EXPECT_TRUE( fl.forth() ) << "FilterList contains Xml - second forth()" ;

	const FilterEntry& fe2 = fl.getEntry();

	EXPECT_EQ( fe2.getFilterId(), 2 ) << "fe1.getFilterId() == 2" ;
	EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
	EXPECT_EQ( fe2.getLoad().getDataType(), DataType::XmlEnum ) << "FilterEntry::getLoad().getDataType() == DataType::XmlEnum" ;
	{
		EmaBuffer Buf( "<value> KLMNOPQR </value>", 25 );
		EXPECT_STREQ( fe2.getXml().getBuffer(), Buf ) << "ElementEntry::getXml()" ;
	}

	EXPECT_TRUE( fl.forth() ) << "FilterList contains Xml - third forth()" ;

	const FilterEntry& fe3 = fl.getEntry();

	EXPECT_EQ( fe3.getFilterId(), 3 ) << "fe1.getFilterId() == 3" ;
	EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
	EXPECT_EQ( fe3.getLoad().getDataType(), DataType::AnsiPageEnum ) << "FilterEntry::getLoad().getDataType() == DataType::AnsiPageEnum" ;
	{
		EmaBuffer Buf( "328-srfsjkj43rouw-01-20ru2l24903$%", 34 );
		EXPECT_STREQ( fe3.getAnsiPage().getBuffer(), Buf ) << "ElementEntry::getAnsiPage().getBuffer()" ;
	}
}

TEST(FilterListTests, testFilterListEncodeDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FilterList flEnc;
	char* s1 = const_cast<char*>("PERMISSION DATA");
	EmaBuffer permission( s1, 15 );

	try
	{
		// Encoding

		ElementList ELEnc1;

		ELEnc1.addAscii( EmaString( "Element Ascii" ), "ABCDEF" );
		ELEnc1.addReal( EmaString( "Element Real" ), 10, OmmReal::ExponentNeg11Enum );
		ELEnc1.addDateTime( EmaString( "Element Date Time" ), 1975, 9, 6, 11, 22, 33, 44 );
		ELEnc1.complete();

		ElementList ELEnc2;

		ELEnc2.addAscii( EmaString( "Element Ascii" ), "GHIJKL" );
		ELEnc2.addReal( EmaString( "Element Real" ), 10, OmmReal::ExponentNeg10Enum );
		ELEnc2.addDateTime( EmaString( "Element Date Time" ), 1985, 9, 6, 11, 22, 33, 44 );
		ELEnc2.complete();

		ElementList ELEnc3;

		ELEnc3.addAscii( EmaString( "Element Ascii" ), "MNOPQR" );
		ELEnc3.addReal( EmaString( "Element Real" ), 10, OmmReal::ExponentNeg12Enum );
		ELEnc3.addDateTime( EmaString( "Element Date Time" ), 1995, 9, 6, 11, 22, 33, 44 );
		ELEnc3.complete();

		//Encoding


		flEnc.add( 1, FilterEntry::ClearEnum, ELEnc1, permission )
		.add( 2, FilterEntry::SetEnum, ELEnc2 )
		.add( 3, FilterEntry::UpdateEnum, ELEnc3, permission )
		.complete();

		//Decoding
		StaticDecoder::setData( &flEnc, &dictionary );



		try
		{
			const FilterEntry& fe = flEnc.getEntry();
			EXPECT_FALSE( true ) << "FilterList with ElementList - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FilterList with ElementList - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - first forth()" ;
		const FilterEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::NoDataEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 1 ) << "FilterEntry::getFilterId() == 1" ;
		EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
		EXPECT_TRUE( fe1.hasPermissionData() ) << "FilterEntry::hasPermissionData() == true" ;
		EXPECT_STREQ( fe1.getPermissionData(), EmaBuffer( "PERMISSION DATA", 15 ) ) << "FilterEntry::getPermissionData() == \"PERMISSION DATA\"" ;

		try
		{
			const ElementList& el1 = fe1.getElementList();
			EXPECT_FALSE( true ) << "FilterList with ElementList - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FilterList with ElementList - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - second forth()" ;
		const FilterEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe2.getFilterId(), 2 ) << "FilterEntry::getFilterId() == 2" ;
		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_FALSE( fe2.hasPermissionData() ) << "FilterEntry::hasPermissionData() == false" ;
		try
		{
			fe2.getPermissionData();
			EXPECT_FALSE( true ) << "FilterEntry::getPermissionData, Expected exception, entry does not have permission data" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FilterEntry::getPermissionData - exception expected: " ).append( excp.getText() ) ;
		}

		const ElementList& el2 = fe2.getElementList();

		// first forth on ElementList.
		EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - first forth()" ;
		const ElementEntry& ee1 = el2.getEntry();
		EXPECT_EQ( ee1.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( ee1.getName(), "Element Ascii" ) << "ee1.getName() == Element Ascii" ;
		EXPECT_STREQ( ee1.getAscii(), "GHIJKL" ) << "ee1.getAscii() == GHIJKL" ;

		// second forth on ElementList.
		EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - second forth()" ;
		const ElementEntry& ee2 = el2.getEntry();
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_STREQ( ee2.getName(), "Element Real" ) << "ee1.getName() == Element Real" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 10 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), OmmReal::ExponentNeg10Enum ) << "ElementEntry::getReal().getMagnitudeType()" ;

		// second forth on ElementList.
		EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - third forth()" ;
		const ElementEntry& ee3 = el2.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element Date Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getDateTime().getDay(), 6 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee3.getDateTime().getMonth(), 9 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee3.getDateTime().getYear(), 1985 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee3.getDateTime().getHour(), 11 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee3.getDateTime().getMinute(), 22 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee3.getDateTime().getSecond(), 33 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee3.getDateTime().getMillisecond(), 44 ) << "ElementEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - third forth()" ;
		const FilterEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe3.getFilterId(), 3 ) << "FilterEntry::getFilterId() == 3" ;
		EXPECT_EQ( fe3.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_TRUE( fe3.hasPermissionData() ) << "FilterEntry::hasPermissionData() == true" ;
		EXPECT_STREQ( fe3.getPermissionData(), EmaBuffer( "PERMISSION DATA", 15 ) ) << "FilterEntry::getPermissionData() == \"PERMISSION DATA\"" ;
		const ElementList& el3 = fe3.getElementList();

		EXPECT_FALSE( flEnc.forth() ) << "FilterList with ElementList - final forth()" ;

		EXPECT_TRUE( true ) << "FilterList with all data types - exception not expected" ;


		flEnc.reset();
		{
			try
			{
				const FilterEntry& fe = flEnc.getEntry();
				EXPECT_FALSE( true ) << "FilterList with ElementList - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "FilterList with ElementList - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - first forth()" ;
			const FilterEntry& fe1 = flEnc.getEntry();
			EXPECT_EQ( fe1.getLoadType(), DataType::NoDataEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
			EXPECT_EQ( fe1.getFilterId(), 1 ) << "FilterEntry::getFilterId() == 1" ;
			EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
			EXPECT_TRUE( fe1.hasPermissionData() ) << "FilterEntry::hasPermissionData() == true" ;
			EXPECT_STREQ( fe1.getPermissionData(), EmaBuffer( "PERMISSION DATA", 15 ) ) << "FilterEntry::getPermissionData() == \"PERMISSION DATA\"" ;

			try
			{
				const ElementList& el1 = fe1.getElementList();
				EXPECT_FALSE( true ) << "FilterList with ElementList - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "FilterList with ElementList - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - second forth()" ;
			const FilterEntry& fe2 = flEnc.getEntry();
			EXPECT_EQ( fe2.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
			EXPECT_EQ( fe2.getFilterId(), 2 ) << "FilterEntry::getFilterId() == 2" ;
			EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
			EXPECT_FALSE( fe2.hasPermissionData() ) << "FilterEntry::hasPermissionData() == false" ;
			try
			{
				fe2.getPermissionData();
				EXPECT_FALSE( true ) << "FilterEntry::getPermissionData, Expected exception, entry does not have permission data" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "FilterEntry::getPermissionData - exception expected: " ).append( excp.getText() ) ;
			}

			const ElementList& el2 = fe2.getElementList();

			// first forth on ElementList.
			EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - first forth()" ;
			const ElementEntry& ee1 = el2.getEntry();
			EXPECT_EQ( ee1.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ee1.getName(), "Element Ascii" ) << "ee1.getName() == Element Ascii" ;
			EXPECT_STREQ( ee1.getAscii(), "GHIJKL" ) << "ee1.getAscii() == GHIJKL" ;

			// second forth on ElementList.
			EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - second forth()" ;
			const ElementEntry& ee2 = el2.getEntry();
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_STREQ( ee2.getName(), "Element Real" ) << "ee1.getName() == Element Real" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 10 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), OmmReal::ExponentNeg10Enum ) << "ElementEntry::getReal().getMagnitudeType()" ;

			// second forth on ElementList.
			EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - third forth()" ;
			const ElementEntry& ee3 = el2.getEntry();
			EXPECT_STREQ( ee3.getName(), "Element Date Time" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee3.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
			EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee3.getDateTime().getDay(), 6 ) << "ElementEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ee3.getDateTime().getMonth(), 9 ) << "ElementEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ee3.getDateTime().getYear(), 1985 ) << "ElementEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ee3.getDateTime().getHour(), 11 ) << "ElementEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ee3.getDateTime().getMinute(), 22 ) << "ElementEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ee3.getDateTime().getSecond(), 33 ) << "ElementEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ee3.getDateTime().getMillisecond(), 44 ) << "ElementEntry::getDateTime().getMillisecond()" ;


			EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - third forth()" ;
			const FilterEntry& fe3 = flEnc.getEntry();
			EXPECT_EQ( fe3.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
			EXPECT_EQ( fe3.getFilterId(), 3 ) << "FilterEntry::getFilterId() == 3" ;
			EXPECT_EQ( fe3.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
			EXPECT_TRUE( fe3.hasPermissionData() ) << "FilterEntry::hasPermissionData() == true" ;
			EXPECT_STREQ( fe3.getPermissionData(), EmaBuffer( "PERMISSION DATA", 15 ) ) << "FilterEntry::getPermissionData() == \"PERMISSION DATA\"" ;
			const ElementList& el3 = fe3.getElementList();

			EXPECT_FALSE( flEnc.forth() ) << "FilterList with ElementList - final forth()" ;

			EXPECT_TRUE( true ) << "FilterList with ElementList - exception not expected" ;
		}
	}

	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode FilterList after reset() - exception not expected" ;
	}

	flEnc.clear();

	try
	{
		ElementList ELEnc1;
		ElementList ELEnc2;
		ElementList ELEnc3;

		flEnc.add( 3, FilterEntry::UpdateEnum, ELEnc1, permission );

		ELEnc1.addAscii( EmaString( "Element Ascii" ), "ABCDEF" );
		ELEnc1.addReal( EmaString( "Element Real" ), 10, OmmReal::ExponentNeg11Enum );
		ELEnc1.addDateTime( EmaString( "Element Date Time" ), 1975, 9, 6, 11, 22, 33, 44 );
		ELEnc1.complete();

		flEnc.add( 4, FilterEntry::SetEnum, ELEnc2 );

		ELEnc2.addAscii( EmaString( "Element Ascii" ), "GHIJKL" );
		ELEnc2.addReal( EmaString( "Element Real" ), 10, OmmReal::ExponentNeg10Enum );
		ELEnc2.addDateTime( EmaString( "Element Date Time" ), 1985, 9, 6, 11, 22, 33, 44 );
		ELEnc2.complete();

		flEnc.add( 5, FilterEntry::UpdateEnum, ELEnc3, permission );

		ELEnc3.addAscii( EmaString( "Element Ascii" ), "MNOPQR" );
		ELEnc3.addReal( EmaString( "Element Real" ), 10, OmmReal::ExponentNeg12Enum );
		ELEnc3.addDateTime( EmaString( "Element Date Time" ), 1995, 9, 6, 11, 22, 33, 44 );
		ELEnc3.complete();

		flEnc.complete();

		//Decoding
		StaticDecoder::setData( &flEnc, &dictionary );


		try
		{
			const FilterEntry& fe = flEnc.getEntry();
			EXPECT_FALSE( true ) << "FilterList with all data types - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FilterList with all data types - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FilterList with all data types- first forth()" ;
		const FilterEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 3 ) << "FilterEntry::getFilterId() == 3" ;
		EXPECT_EQ( fe1.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_TRUE( fe1.hasPermissionData() ) << "FilterEntry::hasPermissionData() == true" ;
		EXPECT_STREQ( fe1.getPermissionData(), EmaBuffer( "PERMISSION DATA", 15 ) ) << "FilterEntry::getPermissionData() == \"PERMISSION DATA\"" ;
		const ElementList& el1 = fe1.getElementList();

		try
		{
			const FieldList& el = fe1.getFieldList();
			EXPECT_FALSE( true ) << "FilterList with all data types - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FilterList with all data types - exception expected: " ).append( excp.getText() ) ;
		}


		EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - second forth()" ;
		const FilterEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe2.getFilterId(), 4 ) << "FilterEntry::getFilterId() == 4" ;
		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_FALSE( fe2.hasPermissionData() ) << "FilterEntry::hasPermissionData() == false" ;
		try
		{
			fe2.getPermissionData();
			EXPECT_FALSE( true ) << "FilterEntry::getPermissionData, Expected exception, entry does not have permission data" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FilterEntry::getPermissionData - exception expected: " ).append( excp.getText() ) ;
		}
		const ElementList& el2 = fe2.getElementList();

		// first forth on ElementList.
		EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - first forth()" ;
		const ElementEntry& ee1 = el2.getEntry();
		EXPECT_EQ( ee1.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( ee1.getName(), "Element Ascii" ) << "ee1.getName() == Element Ascii" ;
		EXPECT_STREQ( ee1.getAscii(), "GHIJKL" ) << "ee1.getAscii() == GHIJKL" ;

		// second forth on ElementList.
		EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - second forth()" ;
		const ElementEntry& ee2 = el2.getEntry();
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_STREQ( ee2.getName(), "Element Real" ) << "ee1.getName() == Element Real" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 10 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), OmmReal::ExponentNeg10Enum ) << "ElementEntry::getReal().getMagnitudeType()" ;

		// second forth on ElementList.
		EXPECT_TRUE( el2.forth() ) << "ElementList inside FilterList with Set action - third forth()" ;
		const ElementEntry& ee3 = el2.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element Date Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getDateTime().getDay(), 6 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee3.getDateTime().getMonth(), 9 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee3.getDateTime().getYear(), 1985 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee3.getDateTime().getHour(), 11 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee3.getDateTime().getMinute(), 22 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee3.getDateTime().getSecond(), 33 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee3.getDateTime().getMillisecond(), 44 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FilterList with ElementList - third forth()" ;
		const FilterEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getLoadType(), DataType::ElementListEnum ) << "FilterEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe3.getFilterId(), 5 ) << "FilterEntry::getFilterId() == 5" ;
		EXPECT_EQ( fe3.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_TRUE( fe3.hasPermissionData() ) << "FilterEntry::hasPermissionData() == true" ;
		EXPECT_STREQ( fe3.getPermissionData(), EmaBuffer( "PERMISSION DATA", 15 ) ) << "FilterEntry::getPermissionData() == \"PERMISSION DATA\"" ;
		const ElementList& el3 = fe3.getElementList();

		EXPECT_FALSE( flEnc.forth() ) << "FilterList with ElementList - final forth()" ;

		EXPECT_TRUE( true ) << "FilterList with ElementList - exception not expected" ;

	}

	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode FilterList after clear() - exception not expected" ;
	}
}


TEST(FilterListTests, testFilterListContainsFieldListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FilterList filterListEnc;
	filterListEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  Clear, FieldList-Set, FieldList-Set, FieldList-Update

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Clear
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		filterListEnc.add( 1, FilterEntry::ClearEnum, flEnc1, permission );

		//second entry  //Set FieldList
		filterListEnc.add( 2, FilterEntry::SetEnum, flEnc1, permission );

		//third entry  //Set FieldList
		filterListEnc.add( 3, FilterEntry::SetEnum, flEnc1, permission );

		//fourth entry  //Update FieldList
		filterListEnc.add( 4, FilterEntry::UpdateEnum, flEnc1, permission );

		filterListEnc.complete();


		//Now do EMA decoding of FilterList
		StaticDecoder::setData( &filterListEnc, &dictionary );


		EXPECT_TRUE( filterListEnc.hasTotalCountHint() ) << "FilterList contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( filterListEnc.getTotalCountHint(), 5 ) << "FilterList contains FieldList - getTotalCountHint()" ;


		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains FieldList - first forth()" ;

		const FilterEntry& fe1 = filterListEnc.getEntry();

		EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 1 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;

		filterListEnc.reset();

		{
			EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains FieldList - filterList forth() after reset()" ;

			const FilterEntry& fe = filterListEnc.getEntry();

			EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
			EXPECT_EQ( fe1.getFilterId(), 1 ) << "FilterEntry::getFilterId()" ;
			EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		}

		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains FieldList - second forth()" ;

		const FilterEntry& fe2 = filterListEnc.getEntry();

		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 2 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::FieldListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = fe2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains FieldList - third forth()" ;

		const FilterEntry& fe3 = filterListEnc.getEntry();

		EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 3 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::FieldListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = fe3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains FieldList - fourth forth()" ;

		const FilterEntry& fe4 = filterListEnc.getEntry();

		EXPECT_EQ( fe4.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 4 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::FieldListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = fe4.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_FALSE( filterListEnc.forth() ) << "FilterList contains FieldList - fifth forth()" ;

		EXPECT_TRUE( true ) << "FilterList contains FieldList - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList contains FieldList - exception not expected" ;
	}
}

TEST(FilterListTests, testFilterListContainsElementListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FilterList filterListEnc;
	filterListEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  Clear, ElementList-Set, ElementList-Set, ElementList-Update

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Clear
		ElementList elEnc1;
		EmaEncodeElementListAll( elEnc1 );
		filterListEnc.add( 1, FilterEntry::ClearEnum, elEnc1, permission );

		//second entry  //Set ElementList
		filterListEnc.add( 2, FilterEntry::SetEnum, elEnc1, permission );

		//third entry  //Set ElementList
		filterListEnc.add( 3, FilterEntry::SetEnum, elEnc1, permission );

		//fourth entry  //Update ElementList
		filterListEnc.add( 4, FilterEntry::UpdateEnum, elEnc1, permission );

		filterListEnc.complete();


		//Now do EMA decoding of FilterList
		StaticDecoder::setData( &filterListEnc, &dictionary );


		EXPECT_TRUE( filterListEnc.hasTotalCountHint() ) << "FilterList contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( filterListEnc.getTotalCountHint(), 5 ) << "FilterList contains ElementList - getTotalCountHint()" ;


		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains ElementList - first forth()" ;

		const FilterEntry& fe1 = filterListEnc.getEntry();

		EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 1 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;

		filterListEnc.reset();

		{
			EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains ElementList - filterList forth() after reset()" ;

			const FilterEntry& fe = filterListEnc.getEntry();

			EXPECT_EQ( fe1.getAction(), FilterEntry::ClearEnum ) << "FilterEntry::getAction() == FilterEntry::ClearEnum" ;
			EXPECT_EQ( fe1.getFilterId(), 1 ) << "FilterEntry::getFilterId()" ;
			EXPECT_EQ( fe1.getLoad().getDataType(), DataType::NoDataEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		}

		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains ElementList - second forth()" ;

		const FilterEntry& fe2 = filterListEnc.getEntry();

		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 2 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::ElementListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = fe2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains ElementList - third forth()" ;

		const FilterEntry& fe3 = filterListEnc.getEntry();

		EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 3 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::ElementListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = fe3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains ElementList - fourth forth()" ;

		const FilterEntry& fe4 = filterListEnc.getEntry();

		EXPECT_EQ( fe4.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_EQ( fe1.getFilterId(), 4 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::ElementListEnum ) << "FilterEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = fe4.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_FALSE( filterListEnc.forth() ) << "FilterList contains ElementList - fifth forth()" ;

		EXPECT_TRUE( true ) << "FilterList contains ElementList - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList contains FieldList - exception not expected" ;
	}
}

TEST(FilterListTests, testFilterListContainsMapsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FilterList filterListEnc;
	filterListEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  Clear, Map-Set, Map-Set, Map-Update

		char* s1 = const_cast<char*>("PERMISSION DATA");
		EmaBuffer permission( s1, 15 );

		//first entry  //Clear
		Map mapEnc1;
		EmaEncodeMapAll( mapEnc1 );
//		filterListEnc.add( 1, FilterEntry::ClearEnum, mapEnc1, permission );

		//second entry  //Set Map
		filterListEnc.add( 2, FilterEntry::SetEnum, mapEnc1, permission );

		//third entry  //Set Map
		filterListEnc.add( 3, FilterEntry::SetEnum, mapEnc1, permission );

		//fourth entry  //Update Map
		filterListEnc.add( 4, FilterEntry::UpdateEnum, mapEnc1, permission );

		filterListEnc.complete();


		//Now do EMA decoding of FilterList
		StaticDecoder::setData( &filterListEnc, &dictionary );


		EXPECT_TRUE( filterListEnc.hasTotalCountHint() ) << "FilterList contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( filterListEnc.getTotalCountHint(), 5 ) << "FilterList contains Map - getTotalCountHint()" ;

		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains Map - second forth()" ;

		const FilterEntry& fe2 = filterListEnc.getEntry();

		EXPECT_EQ( fe2.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe2.getFilterId(), 2 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::MapEnum ) << "FilterEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = fe2.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FilterEntry Map within filterList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FilterEntry Map within filterList - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "FilterEntry Map within filterList - fifth map forth()" ;
		}


		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains Map - third forth()" ;

		const FilterEntry& fe3 = filterListEnc.getEntry();

		EXPECT_EQ( fe3.getAction(), FilterEntry::SetEnum ) << "FilterEntry::getAction() == FilterEntry::SetEnum" ;
		EXPECT_EQ( fe3.getFilterId(), 3 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::MapEnum ) << "FilterEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = fe3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FilterEntry Map within filterList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FilterEntry Map within filterList - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "FilterEntry Map within filterList - fifth map forth()" ;
		}


		EXPECT_TRUE( filterListEnc.forth() ) << "FilterList contains FieldList - fourth forth()" ;

		const FilterEntry& fe4 = filterListEnc.getEntry();

		EXPECT_EQ( fe4.getAction(), FilterEntry::UpdateEnum ) << "FilterEntry::getAction() == FilterEntry::UpdateEnum" ;
		EXPECT_EQ( fe4.getFilterId(), 4 ) << "FilterEntry::getFilterId()" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::MapEnum ) << "FilterEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = fe3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FilterEntry Map within filterList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FilterEntry Map within filterList - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FilterEntry Map within filterList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "FilterEntry Map within filterList - fifth map forth()" ;
		}


		EXPECT_FALSE( filterListEnc.forth() ) << "FilterList contains Map - fifth forth()" ;

		EXPECT_TRUE( true ) << "FilterList contains Map - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList contains Map - exception not expected" ;
	}

}


void encodeErrorFilterList( RsslBuffer& rsslBuf )
{
}

void testErrorFilterListDecode()
{
}

TEST(FilterListTests, testFilterListDecodetoString)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  Clear, FieldList-Set, FieldList-Set, FieldList-Update

		RsslBuffer filterListBuffer;
		filterListBuffer.length = 2048;
		filterListBuffer.data = ( char* )malloc( sizeof( char ) * 2048 );

		RsslFilterList rsslFL;
		RsslEncodeIterator filterEncodeIter;

		rsslClearFilterList( &rsslFL );
		rsslClearEncodeIterator( &filterEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &filterEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &filterEncodeIter, &filterListBuffer );
		rsslFL.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT ;

		rsslFL.containerType = RSSL_DT_FIELD_LIST;
		rsslFL.totalCountHint = 5;

		rsslEncodeFilterListInit( &filterEncodeIter, &rsslFL );
		RsslFilterEntry filterEntry;

		//first entry  //Clear
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_CLEAR_ENTRY;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//second entry  //Add FieldList
		rsslClearFilterEntry( &filterEntry );
		// allocate buffer for the field list for FilterEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//third entry  //Add FieldList
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		//fourth entry  //Update FieldList
		rsslClearFilterEntry( &filterEntry );
		filterEntry.flags = RSSL_FTEF_NONE;
		filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
		filterEntry.encData = rsslBuf1;
		rsslEncodeFilterEntry( &filterEncodeIter, &filterEntry );

		rsslEncodeFilterListComplete( &filterEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of FilterList
		FilterList fl;
		StaticDecoder::setRsslData( &fl, &filterListBuffer, RSSL_DT_FILTER_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		free( filterListBuffer.data );

		EXPECT_TRUE( true ) << "FilterList toString Decode - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList toString Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FilterListTests, testFilterListError)
{

	{
		try
		{
			FilterList fl;
			fl.complete();
			EXPECT_TRUE( true ) << "FilterList::complete() on empty filter list - exception not expected" ;

			StaticDecoder::setData( &fl, 0 );


			EXPECT_FALSE( fl.forth() ) << "FilterList::forth()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "FilterList::complete() on empty filter list - exception not expected" ;
		}
	}

	{
		try
		{
			FilterList fl;
			fl.totalCountHint( 2 );
			fl.complete();
			EXPECT_TRUE( true ) << "FilterList::complete() on empty filter list with total count hint - exception not expected" ;

			StaticDecoder::setData( &fl, 0 );


			EXPECT_TRUE( fl.hasTotalCountHint() ) << "FilterList::hasTotalCountHint()" ;

			EXPECT_EQ( fl.getTotalCountHint(), 2 ) << "FilterList::getTotalCountHint()" ;

			EXPECT_FALSE( fl.forth() ) << "FilterList::forth()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "FilterList::complete() on empty filter list with total count hint - exception not expected" ;
		}
	}

	{
		try
		{
			FilterList fl;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			fl.add( 1, FilterEntry::SetEnum, el );

			fl.complete();

			fl.add( 2, FilterEntry::SetEnum, el );

			EXPECT_FALSE( true ) << "FilterList add after complete - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FilterList add after complete - exception expected" ;
		}
	}

	{
		try
		{
			FilterList fl;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			fl.add( 1, FilterEntry::SetEnum, el );

			fl.complete();

			fl.clear();

			fl.add( 2, FilterEntry::SetEnum, el );

			StaticDecoder::setData( &fl, 0 );


			StaticDecoder::setData( &fl, 0 );


			EXPECT_TRUE( fl.forth() ) << "FilterList::forth()" ;
			EXPECT_FALSE( fl.forth() ) << "FilterList::forth()" ;

			EXPECT_TRUE( true ) << "FilterList add after complete & clear - exception not expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "FilterList add after complete & clear - exception not expected" ;
		}
	}

	try
	{
		FilterList container;

		RefreshMsg msg;

		container.add( 1, FilterEntry::SetEnum, msg );

		EXPECT_FALSE( true ) << "FilterList::add( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "FilterList::add( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		FilterList container;

		RefreshMsg msg;

		msg.serviceId( 1 );

		container.add( 1, FilterEntry::SetEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "FilterList::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "FilterEntry::getLoadType()" ;

		EXPECT_TRUE( true ) << "FilterList::add( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList::add( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}


	try
	{
		FilterList container;

		GenericMsg msg;

		container.add( 1, FilterEntry::SetEnum, msg );

		EXPECT_FALSE( true ) << "FilterList::add( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "FilterList::add( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		FilterList container;

		GenericMsg msg;

		msg.serviceId( 1 );

		container.add( 1, FilterEntry::SetEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "FilterList::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "FilterEntry::getLoadType()" ;

		EXPECT_TRUE( true ) << "FilterList::add( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList::add( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}


	try
	{
		FilterList container;

		GenericMsg msg;

		container.add( 1, FilterEntry::ClearEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "FilterEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_TRUE( true ) << "FilterList::add( ClearEnum, GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList::add( ClearEnum, GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		FilterList container;

		GenericMsg msg;

		msg.serviceId( 1 );

		container.add( 1, FilterEntry::ClearEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "FilterList::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "FilterEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_TRUE( true ) << "FilterList::add( ClearEnum, GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList::add( ClearEnum, GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}

	try
	{
		FilterList container;

		ElementList el;

		container.add( 1, FilterEntry::ClearEnum, el );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "FilterEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_TRUE( true ) << "FilterList::add( ClearEnum, ElementList ) while ElementList is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList::add( ClearEnum, ElementList ) while ElementList is empty - exception expected" ;
	}

	try
	{
		FilterList container;

		ElementList el;

		el.addInt( "entry", 1 ).complete();

		container.add( 1, FilterEntry::ClearEnum, el );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::NoDataEnum ) << "FilterEntry::getLoadType()" ;

		EXPECT_FALSE( container.forth() ) << "FilterList::forth()" ;

		EXPECT_TRUE( true ) << "FilterList::add( ClearEnum, ElementList ) while ElementList is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "FilterList::add( ClearEnum, ElementList ) while ElementList is empty - exception expected" ;
	}

}

