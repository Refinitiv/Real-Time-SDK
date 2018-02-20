/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Access/Impl/StaticDecoder.h"
#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace std;

TEST(VectorTests, testVectorContainsFieldListsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Delete, FieldList-Set, FieldList-Set, FieldList-Update

		RsslBuffer vectorBuffer;
		vectorBuffer.length = 4096;
		vectorBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslVector rsslVector = RSSL_INIT_VECTOR;
		RsslEncodeIterator vectorEncodeIter;

		rsslClearVector( &rsslVector );
		rsslClearEncodeIterator( &vectorEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &vectorEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &vectorEncodeIter, &vectorBuffer );
		rsslVector.flags = RSSL_VTF_HAS_SUMMARY_DATA | RSSL_VTF_HAS_TOTAL_COUNT_HINT;

		rsslVector.containerType = RSSL_DT_FIELD_LIST;
		rsslVector.totalCountHint = 5;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslVector.encSummaryData = rsslBuf;

		rsslEncodeVectorInit( &vectorEncodeIter, &rsslVector, 0, 0 );
		RsslVectorEntry vectorEntry;

		//first entry  //Delete
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_DELETE_ENTRY;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//second entry  //Set FieldList
		rsslClearVectorEntry( &vectorEntry );
		// allocate buffer for the field list for VectorEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//third entry  //Set FieldList
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//fourth entry  //Update FieldList
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		vectorBuffer.length = rsslGetEncodedBufferLength( &vectorEncodeIter );
		rsslEncodeVectorComplete( &vectorEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Vector
		Vector vector;
		StaticDecoder::setRsslData( &vector, &vectorBuffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( vector.hasTotalCountHint() ) << "Vector contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( vector.getTotalCountHint(), 5 ) << "Vector contains FieldList - getTotalCountHint()" ;

		switch ( vector.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
		{
			const FieldList& fl = vector.getSummaryData().getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Vector Decode Summary FieldList - vector.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( vector.forth() ) << "Vector contains FieldList - first vector forth()" ;

		const VectorEntry& ve1 = vector.getEntry();
		EXPECT_EQ( ve1.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
		EXPECT_EQ( ve1.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		vector.reset();
		{
			EXPECT_TRUE( vector.forth() ) << "Vector contains FieldList - vector forth() after reset()" ;

			const VectorEntry& ve = vector.getEntry();

			EXPECT_EQ( ve.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
			EXPECT_EQ( ve.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}

		EXPECT_TRUE( vector.forth() ) << "Vector contains FieldList - second vector forth()" ;

		const VectorEntry& ve2 = vector.getEntry();
		EXPECT_EQ( ve2.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve2.getLoad().getDataType(), DataType::FieldListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = ve2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains FieldList - third vector forth()" ;

		const VectorEntry& ve3 = vector.getEntry();
		EXPECT_EQ( ve3.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve3.getLoad().getDataType(), DataType::FieldListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		{
			const FieldList& fl = ve3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains FieldList - fourth vector forth()" ;

		const VectorEntry& ve4 = vector.getEntry();
		EXPECT_EQ( ve4.getAction(), VectorEntry::UpdateEnum ) << "VectorEntry::getAction() == VectorEntry::UpdateEnum" ;
		EXPECT_EQ( ve4.getLoad().getDataType(), DataType::FieldListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::FieldListEnum" ;


		EXPECT_FALSE( vector.forth() ) << "Vector contains FieldList - final vector forth()" ;

		free( rsslBuf.data );
		free( vectorBuffer.data );

		EXPECT_TRUE( true ) << "Vector contains FieldList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector contains FieldList - exception not expectedd" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(VectorTests, testVectorContainsElementListsDecodeAll)
{

	try
	{
		// encoding order:  SummaryData(with ElementList), Delete, ElementList-Set, ElementList-Set, ElementList-Update

		RsslBuffer vectorBuffer;
		vectorBuffer.length = 4096;
		vectorBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslVector rsslVector = RSSL_INIT_VECTOR;
		RsslEncodeIterator vectorEncodeIter;

		rsslClearVector( &rsslVector );
		rsslClearEncodeIterator( &vectorEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &vectorEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &vectorEncodeIter, &vectorBuffer );
		rsslVector.flags = RSSL_VTF_HAS_SUMMARY_DATA | RSSL_VTF_HAS_TOTAL_COUNT_HINT;

		rsslVector.containerType = RSSL_DT_ELEMENT_LIST;
		rsslVector.totalCountHint = 5;

		// allocate buffer for the element list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeElementListAll( rsslBuf );
		rsslVector.encSummaryData = rsslBuf;

		rsslEncodeVectorInit( &vectorEncodeIter, &rsslVector, 0, 0 );
		RsslVectorEntry vectorEntry;

		//first entry  //Delete
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_DELETE_ENTRY;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//second entry  //Set ElementList
		rsslClearVectorEntry( &vectorEntry );
		// allocate buffer for the element list for VectorEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeElementListAll( rsslBuf1 );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//third entry  //Set ElementList
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//fourth entry  //Update ElementList
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		vectorBuffer.length = rsslGetEncodedBufferLength( &vectorEncodeIter );
		rsslEncodeVectorComplete( &vectorEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Vector
		Vector vector;
		StaticDecoder::setRsslData( &vector, &vectorBuffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_TRUE( vector.hasTotalCountHint() ) << "Vector contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( vector.getTotalCountHint(), 5 ) << "Vector contains ElementList - getTotalCountHint()" ;

		switch ( vector.getSummaryData().getDataType() )
		{
		case DataType::ElementListEnum :
		{
			const ElementList& el = vector.getSummaryData().getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Vector Decode Summary FieldList - vector.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( vector.forth() ) << "Vector contains ElementList - first vector forth()" ;

		const VectorEntry& ve1 = vector.getEntry();
		EXPECT_EQ( ve1.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
		EXPECT_EQ( ve1.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		vector.reset();
		{
			EXPECT_TRUE( vector.forth() ) << "Vector contains ElementList - vector forth() after reset()" ;

			const VectorEntry& ve = vector.getEntry();

			EXPECT_EQ( ve.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
			EXPECT_EQ( ve.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains ElementList - second vector forth()" ;

		const VectorEntry& ve2 = vector.getEntry();
		EXPECT_EQ( ve2.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve2.getLoad().getDataType(), DataType::ElementListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = ve2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains ElementList - third vector forth()" ;

		const VectorEntry& ve3 = vector.getEntry();

		EXPECT_EQ( ve3.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve3.getLoad().getDataType(), DataType::ElementListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = ve3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains ElementList - fourth vector forth()" ;

		const VectorEntry& ve4 = vector.getEntry();

		EXPECT_EQ( ve4.getAction(), VectorEntry::UpdateEnum ) << "VectorEntry::getAction() == VectorEntry::UpdateEnum" ;
		EXPECT_EQ( ve4.getLoad().getDataType(), DataType::ElementListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = ve4.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_FALSE( vector.forth() ) << "Vector contains ElementList - final vector forth()" ;

		free( rsslBuf.data );
		free( vectorBuffer.data );

		EXPECT_TRUE( true ) << "Vector contains ElementList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector contains ElementList - exception not expectedd" ;
	}
}

TEST(VectorTests, testVectorContainsMapsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with Map), Delete, Map-Set, Map-Set, Map-Update

		RsslBuffer vectorBuffer;
		vectorBuffer.length = 4096;
		vectorBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslVector rsslVector = RSSL_INIT_VECTOR;
		RsslEncodeIterator vectorEncodeIter;

		rsslClearVector( &rsslVector );
		rsslClearEncodeIterator( &vectorEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &vectorEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &vectorEncodeIter, &vectorBuffer );
		rsslVector.flags = RSSL_VTF_HAS_SUMMARY_DATA | RSSL_VTF_HAS_TOTAL_COUNT_HINT;

		rsslVector.containerType = RSSL_DT_MAP;
		rsslVector.totalCountHint = 5;

		// allocate buffer for the map for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeMapAll( rsslBuf );
		rsslVector.encSummaryData = rsslBuf;

		rsslEncodeVectorInit( &vectorEncodeIter, &rsslVector, 0, 0 );
		RsslVectorEntry vectorEntry;

		//first entry  //Delete
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_DELETE_ENTRY;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//second entry  //Set Map
		rsslClearVectorEntry( &vectorEntry );
		// allocate buffer for the map for VectorEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeMapAll( rsslBuf1 );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//third entry  //Set Map
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//fourth entry  //Update Map
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		vectorBuffer.length = rsslGetEncodedBufferLength( &vectorEncodeIter );
		rsslEncodeVectorComplete( &vectorEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Vector
		Vector vector;
		StaticDecoder::setRsslData( &vector, &vectorBuffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( vector.hasTotalCountHint() ) << "Vector contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( vector.getTotalCountHint(), 5 ) << "Vector contains Map - getTotalCountHint()" ;

		switch ( vector.getSummaryData().getDataType() )
		{
		case DataType::MapEnum :
		{
			const Map& mapS = vector.getSummaryData().getMap();
			EXPECT_TRUE( mapS.hasKeyFieldId() ) << "Vector Decode Summary Map - hasKeyFieldId()" ;
			EXPECT_EQ( mapS.getKeyFieldId(), 3426 ) << "Vector Decode Summary Map - getKeyFieldId()" ;

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Map - first map forth()" ;
			const MapEntry& me1a = mapS.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Map - second map forth()" ;
			const MapEntry& me2a = mapS.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Map - third map forth()" ;
			const MapEntry& me3a = mapS.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Mapp - fourth map forth()" ;
			const MapEntry& me4a = mapS.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( mapS.forth() ) << "Vector Decode Summary Map - fifth map forth()" ;
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Vector Decode Summary Map - vector.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( vector.forth() ) << "Vector contains Map - first vector forth()" ;

		const VectorEntry& ve1 = vector.getEntry();
		EXPECT_EQ( ve1.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
		EXPECT_EQ( ve1.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		vector.reset();
		{
			EXPECT_TRUE( vector.forth() ) << "Vector contains Map - vector forth() after reset()" ;

			const VectorEntry& ve = vector.getEntry();

			EXPECT_EQ( ve.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
			EXPECT_EQ( ve.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains Map - second vector forth()" ;

		const VectorEntry& ve2 = vector.getEntry();
		EXPECT_EQ( ve2.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve2.getLoad().getDataType(), DataType::MapEnum ) << "VectorEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = ve2.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "VectorEntry Map within vector - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "VectorEntry Map within vector - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - second map forth()" ;
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

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within map - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within map - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "VectorEntry Map within map - fifth map forth()" ;
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains Map - third vector forth()" ;

		const VectorEntry& ve3 = vector.getEntry();

		EXPECT_EQ( ve3.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve3.getLoad().getDataType(), DataType::MapEnum ) << "VectorEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = ve3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "VectorEntry Map within vector - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "VectorEntry Map within vector - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - second map forth()" ;
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

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "VectorEntry Map within vector - fifth map forth()" ;
		}


		EXPECT_TRUE( vector.forth() ) << "Vector contains Map - fourth vector forth()" ;

		const VectorEntry& ve4 = vector.getEntry();

		EXPECT_EQ( ve4.getAction(), VectorEntry::UpdateEnum ) << "VectorEntry::getAction() == VectorEntry::UpdateEnum" ;
		EXPECT_EQ( ve4.getLoad().getDataType(), DataType::MapEnum ) << "VectorEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = ve4.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "VectorEntry Map within vector - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "MapEntry Map within vector - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - second map forth()" ;
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

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "VectorEntry Map within vector - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "VectorEntry Map within vector - fifth map forth()" ;
		}


		EXPECT_FALSE( vector.forth() ) << "Vector contains Map - final vector forth()" ;

		free( rsslBuf.data );
		free( vectorBuffer.data );

		EXPECT_TRUE( true ) << "Vector contains Map - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector contains Map - exception not expectedd" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(VectorTests, testVectorContainsOpaqueDecodeAll)
{

	try
	{
		RsslBuffer vectorBuffer;
		vectorBuffer.length = 4096;
		vectorBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslVector rsslVector = RSSL_INIT_VECTOR;
		RsslEncodeIterator vectorEncodeIter;

		rsslClearVector( &rsslVector );
		rsslClearEncodeIterator( &vectorEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &vectorEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &vectorEncodeIter, &vectorBuffer );
		rsslVector.flags = RSSL_VTF_HAS_TOTAL_COUNT_HINT;

		rsslVector.containerType = RSSL_DT_OPAQUE;
		rsslVector.totalCountHint = 1;

		rsslEncodeVectorInit( &vectorEncodeIter, &rsslVector, 0, 0 );
		RsslVectorEntry vectorEntry;

		rsslClearVectorEntry( &vectorEntry );

		char buffer[100];
		RsslBuffer rsslBuf1;
		rsslBuf1.data = buffer;
		rsslBuf1.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf1, &opaqueValue );

		vectorEntry.index = 0;
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		//Now do EMA decoding of Vector
		Vector vector;
		StaticDecoder::setRsslData( &vector, &vectorBuffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_TRUE( vector.forth() ) << "Vector contains Opaque - first map forth()" ;

		const VectorEntry& ve = vector.getEntry();

		EXPECT_EQ( ve.getPosition(), 0 ) << "ve.getPosition()" ;
		EXPECT_EQ( ve.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve.getLoad().getDataType(), DataType::OpaqueEnum ) << "VectorEntry::getLoad().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( ve.getOpaque().getBuffer(), compareTo ) << "VectorEntry::getOpaque().getBuffer()" ;

		free( vectorBuffer.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector Decode with Opaque payload - exception not expected" ;
	}
}

TEST(VectorTests, testVectorContainsXmlDecodeAll)
{

	try
	{
		RsslBuffer vectorBuffer;
		vectorBuffer.length = 4096;
		vectorBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslVector rsslVector = RSSL_INIT_VECTOR;
		RsslEncodeIterator vectorEncodeIter;

		rsslClearVector( &rsslVector );
		rsslClearEncodeIterator( &vectorEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &vectorEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &vectorEncodeIter, &vectorBuffer );
		rsslVector.flags = RSSL_VTF_HAS_TOTAL_COUNT_HINT;

		rsslVector.containerType = RSSL_DT_XML;
		rsslVector.totalCountHint = 1;

		rsslEncodeVectorInit( &vectorEncodeIter, &rsslVector, 0, 0 );

		RsslVectorEntry vectorEntry;

		rsslClearVectorEntry( &vectorEntry );

		char buffer[200];
		RsslBuffer rsslBuf1;
		rsslBuf1.data = buffer;
		rsslBuf1.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf1, &xmlValue );

		vectorEntry.index = 0;
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		rsslEncodeVectorComplete( &vectorEncodeIter, RSSL_TRUE );

		vectorBuffer.length = rsslGetEncodedBufferLength( &vectorEncodeIter );

		Vector vector;
		StaticDecoder::setRsslData( &vector, &vectorBuffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		EXPECT_TRUE( vector.forth() ) << "Vector contains Xml - first vector forth()" ;

		const VectorEntry& ve = vector.getEntry();

		EXPECT_EQ( ve.getPosition(), 0 ) << "ve.getPosition()" ;
		EXPECT_EQ( ve.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve.getLoad().getDataType(), DataType::XmlEnum ) << "VectorEntry::getLoad().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( ve.getXml().getBuffer(), compareTo ) << "VectorEntry::getXml().getBuffer()" ;

		free( vectorBuffer.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector Decode with Xml payload - exception not expected" ;
	}
}

TEST(VectorTests, testVectorContainsAnsiPageDecodeAll)
{

	try
	{
		RsslBuffer vectorBuffer;
		vectorBuffer.length = 4096;
		vectorBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslVector rsslVector = RSSL_INIT_VECTOR;
		RsslEncodeIterator vectorEncodeIter;

		rsslClearVector( &rsslVector );
		rsslClearEncodeIterator( &vectorEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &vectorEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &vectorEncodeIter, &vectorBuffer );
		rsslVector.flags = RSSL_VTF_HAS_TOTAL_COUNT_HINT;

		rsslVector.containerType = RSSL_DT_ANSI_PAGE;
		rsslVector.totalCountHint = 1;

		rsslEncodeVectorInit( &vectorEncodeIter, &rsslVector, 0, 0 );
		RsslVectorEntry vectorEntry;

		rsslClearVectorEntry( &vectorEntry );

		char buffer[100];
		RsslBuffer rsslBuf1;
		rsslBuf1.data = buffer;
		rsslBuf1.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = (char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf1, &ansiPageValue );

		vectorEntry.index = 0;
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.encData = rsslBuf1;
		rsslEncodeVectorEntry( &vectorEncodeIter, &vectorEntry );

		rsslEncodeVectorComplete( &vectorEncodeIter, RSSL_TRUE );

		vectorBuffer.length = rsslGetEncodedBufferLength( &vectorEncodeIter );

		Vector vector;
		StaticDecoder::setRsslData( &vector, &vectorBuffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		EXPECT_TRUE( vector.forth() ) << "Vector contains AnsiPage - first vector forth()" ;

		const VectorEntry& ve = vector.getEntry();

		EXPECT_EQ( ve.getPosition(), 0 ) << "ve.getPosition()" ;
		EXPECT_EQ( ve.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve.getLoad().getDataType(), DataType::AnsiPageEnum ) << "VectorEntry::getLoad().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( ve.getAnsiPage().getBuffer(), compareTo ) << "VectorEntry::getAnsiPage().getBuffer()" ;

		free( vectorBuffer.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector Decode with AnsiPage payload - exception not expected" ;
	}
}



void vectorOfFieldList_RsslEncodeEmaDecode( bool useSetDefinitions, std::string& decodedMsg )

{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	try
	{
	  ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator( &encodeIter );
		rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		RsslBuffer buffer;
		buffer.length = 2048;
		buffer.data = ( char* )malloc( sizeof( char ) * 2048 );
		rsslSetEncodeIteratorBuffer( &encodeIter, &buffer );

		RsslVector rsslVector;
		rsslClearVector( &rsslVector );
		if ( useSetDefinitions )
		{
			rsslVector.flags = RSSL_MPF_HAS_SET_DEFS;
		}
		rsslVector.containerType = RSSL_DT_FIELD_LIST;
		rsslEncodeVectorInit( &encodeIter, &rsslVector, 0, 0 );

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
			rsslEncodeVectorSetDefsComplete( &encodeIter, RSSL_TRUE );
		}

		RsslVectorEntry entry;
		rsslClearVectorEntry( &entry );
		entry.action = RSSL_MPEA_ADD_ENTRY;
		entry.flags = RSSL_MPEF_NONE;
		const RsslUInt rsslUInt = 100212;
		rsslEncodeVectorEntryInit( &encodeIter, &entry, 0 );

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
		fieldEntry.fieldId = 22;
		fieldEntry.dataType = RSSL_DT_REAL;
		RsslReal rsslReal;
		rsslClearReal( &rsslReal );
		rsslReal.hint = RSSL_RH_EXPONENT_2;
		rsslReal.value = 227;
		rsslEncodeFieldEntry( &encodeIter, &fieldEntry, &rsslReal );

		rsslClearFieldEntry( &fieldEntry );
		fieldEntry.fieldId = 25;
		fieldEntry.dataType = RSSL_DT_REAL;
		rsslClearReal( &rsslReal );
		rsslReal.hint = RSSL_RH_EXPONENT_4;
		rsslReal.value = 22801;
		rsslEncodeFieldEntry( &encodeIter, &fieldEntry, &rsslReal );

		rsslClearFieldEntry( &fieldEntry );
		fieldEntry.fieldId = 18;
		fieldEntry.dataType = RSSL_DT_TIME;
		RsslTime rsslTime;
		rsslClearTime( &rsslTime );
		rsslTime.hour = 8;
		rsslTime.minute = 39;
		rsslTime.second = 24;
		rsslEncodeFieldEntry( &encodeIter, &fieldEntry, &rsslTime );

		rsslEncodeFieldListComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeVectorEntryComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeVectorComplete( &encodeIter, RSSL_TRUE );

		Vector vector;
		StaticDecoder::setRsslData( &vector, &buffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );
		decodedMsg = vector;
	}
	catch ( const OmmException& )
	{
	  EXPECT_FALSE(true) << "Decoding of Vector of FieldList, encoded from rssl set defs - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(VectorTests, testVectorContainsFieldListSetDefinitionsDecode)
{

	std::string msgFromStandardData;
	vectorOfFieldList_RsslEncodeEmaDecode( false, msgFromStandardData );
	std::string msgFromSetDef;
	vectorOfFieldList_RsslEncodeEmaDecode( true, msgFromSetDef );

	EXPECT_EQ( msgFromSetDef, msgFromStandardData ) << "Encoding from set definitions results in same decoded message." ;
}

void vectorOfElementList_RsslEncodeEmaDecode( bool useSetDefinitions, std::string& decodedMsg )

{
	try
	{
		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator( &encodeIter );
		rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		RsslBuffer buffer;
		buffer.length = 2048;
		buffer.data = ( char* )malloc( sizeof( char ) * 2048 );
		rsslSetEncodeIteratorBuffer( &encodeIter, &buffer );

		RsslVector rsslVector;
		rsslClearVector( &rsslVector );
		if ( useSetDefinitions )
		{
			rsslVector.flags = RSSL_MPF_HAS_SET_DEFS;
		}
		rsslVector.containerType = RSSL_DT_ELEMENT_LIST;
		rsslEncodeVectorInit( &encodeIter, &rsslVector, 0, 0 );

		RsslLocalElementSetDefDb elementSetDefDb;
		RsslElementSetDefEntry elementSetDefEntries[3] =
		{
		  { {3, const_cast<char*>( "BID" )}, RSSL_DT_REAL },
		  { {3, const_cast<char*>( "ASK")}, RSSL_DT_REAL_8RB },
		  { {10, const_cast<char*>( "TRADE_TIME")}, RSSL_DT_TIME_3 }
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
			rsslEncodeVectorSetDefsComplete( &encodeIter, RSSL_TRUE );
		}

		RsslVectorEntry vectorEntry;
		rsslClearVectorEntry( &vectorEntry );
		vectorEntry.action = RSSL_MPEA_ADD_ENTRY;
		vectorEntry.flags = RSSL_MPEF_NONE;
		const RsslUInt rsslUInt = 100212;
		rsslEncodeVectorEntryInit( &encodeIter, &vectorEntry, 0 );

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
		rsslEncodeVectorEntryComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeVectorComplete( &encodeIter, RSSL_TRUE );

		Vector vector;
		StaticDecoder::setRsslData( &vector, &buffer, RSSL_DT_VECTOR, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );
		decodedMsg = vector;
	}
	catch ( const OmmException& )
	{
	  EXPECT_FALSE(true) << "Decoding of Vector of ElementList, encoded from rssl set defs - exception not expected" ;
	}
}

TEST(VectorTests, testVectorContainsElementListSetDefinitionsDecode)
{

	std::string msgFromStandardData;
	vectorOfElementList_RsslEncodeEmaDecode( false, msgFromStandardData );
	std::string msgFromSetDef;
	vectorOfElementList_RsslEncodeEmaDecode( true, msgFromSetDef );

	EXPECT_EQ( msgFromSetDef, msgFromStandardData ) << "Encoding from set definitions results in same decoded message." ;
}

TEST(VectorTests, testVectorContainsFieldListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Vector vectorEnc;
	vectorEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Delete, FieldList-Set, FieldList-Set, FieldList-Update

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		vectorEnc.summaryData( flEnc );

		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );

		char* permS = const_cast<char*>( "PERMISSION DATA" );
		EmaBuffer permissionData1( permS, 15 );

		//first entry  //Delete
		vectorEnc.add( 1, VectorEntry::DeleteEnum, flEnc1, permissionData1 );

		//second entry  //Set FieldList
		vectorEnc.add( 1, VectorEntry::SetEnum, flEnc1, permissionData1 );

		//third entry  //Set FieldList
		vectorEnc.add( 2, VectorEntry::SetEnum, flEnc1, permissionData1 );

		//fpurth entry  //Update FieldList
		vectorEnc.add( 3, VectorEntry::UpdateEnum, flEnc1, permissionData1 );

		vectorEnc.complete();

		//Now do EMA decoding of Vector
		StaticDecoder::setData( &vectorEnc, &dictionary );


		EXPECT_TRUE( vectorEnc.hasTotalCountHint() ) << "Vector contains FieldLists - hasTotalCountHint()" ;
		EXPECT_EQ( vectorEnc.getTotalCountHint(), 5 ) << "Vector contains FieldLists - getTotalCountHint()" ;

		switch ( vectorEnc.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
		{
			const FieldList& fl = vectorEnc.getSummaryData().getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Vector Decode Summary FieldList - vector.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains FieldLists - first vector forth()" ;

		const VectorEntry& ve1 = vectorEnc.getEntry();
		EXPECT_EQ( ve1.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
		EXPECT_EQ( ve1.getPosition(), 1 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve1.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		vectorEnc.reset();
		{
			EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains FieldLists - vector forth() after reset()" ;

			const VectorEntry& ve = vectorEnc.getEntry();

			EXPECT_EQ( ve.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
			EXPECT_EQ( ve.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}

		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains FieldLists - second vector forth()" ;

		const VectorEntry& ve2 = vectorEnc.getEntry();
		EXPECT_EQ( ve2.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve2.getPosition(), 1 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve2.getLoad().getDataType(), DataType::FieldListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = ve2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains FieldLists - third vector forth()" ;

		const VectorEntry& ve3 = vectorEnc.getEntry();
		EXPECT_EQ( ve3.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve3.getPosition(), 2 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve3.getLoad().getDataType(), DataType::FieldListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		{
			const FieldList& fl = ve3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains FieldLists - fourth vector forth()" ;

		const VectorEntry& ve4 = vectorEnc.getEntry();
		EXPECT_EQ( ve4.getAction(), VectorEntry::UpdateEnum ) << "VectorEntry::getAction() == VectorEntry::UpdateEnum" ;
		EXPECT_EQ( ve4.getPosition(), 3 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve4.getLoad().getDataType(), DataType::FieldListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::FieldListEnum" ;


		EXPECT_FALSE( vectorEnc.forth() ) << "Vector contains FieldLists - final vector forth()" ;

		EXPECT_TRUE( true ) << "Vector contains FieldLists - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector contains FieldLists - exception not expectedd" ;
	}
}

TEST(VectorTests, testVectorContainsElementListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Vector vectorEnc;
	vectorEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with ElementList), Delete, ElementList-Set, ElementList-Set, ElementList-Update

		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		vectorEnc.summaryData( elEnc );

		ElementList elEnc1;
		EmaEncodeElementListAll( elEnc1 );

		char* permS = const_cast<char*>( "PERMISSION DATA" );
		EmaBuffer permissionData1( permS, 15 );

		//first entry  //Delete
		vectorEnc.add( 1, VectorEntry::DeleteEnum, elEnc1, permissionData1 );

		//second entry  //Set ElementList
		vectorEnc.add( 1, VectorEntry::SetEnum, elEnc1, permissionData1 );

		//third entry  //Set ElementList
		vectorEnc.add( 2, VectorEntry::SetEnum, elEnc1, permissionData1 );

		//fourth entry  //Update ElementList
		vectorEnc.add( 3, VectorEntry::UpdateEnum, elEnc1, permissionData1 );

		vectorEnc.complete();


		//Now do EMA decoding of Vector
		StaticDecoder::setData( &vectorEnc, &dictionary );


		EXPECT_TRUE( vectorEnc.hasTotalCountHint() ) << "Vector contains ElementLists - hasTotalCountHint()" ;
		EXPECT_EQ( vectorEnc.getTotalCountHint(), 5 ) << "Vector contains ElementLists - getTotalCountHint()" ;

		switch ( vectorEnc.getSummaryData().getDataType() )
		{
		case DataType::ElementListEnum :
		{
			const ElementList& el = vectorEnc.getSummaryData().getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Vector Decode Summary FieldList - vector.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains ElementLists - first vector forth()" ;

		const VectorEntry& ve1 = vectorEnc.getEntry();
		EXPECT_EQ( ve1.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
		EXPECT_EQ( ve1.getPosition(), 1 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve1.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		vectorEnc.reset();
		{
			EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains ElementLists - vector forth() after reset()" ;

			const VectorEntry& ve = vectorEnc.getEntry();

			EXPECT_EQ( ve.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
			EXPECT_EQ( ve.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains ElementLists - second vector forth()" ;

		const VectorEntry& ve2 = vectorEnc.getEntry();
		EXPECT_EQ( ve2.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve2.getPosition(), 1 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve2.getLoad().getDataType(), DataType::ElementListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = ve2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains ElementLists - third vector forth()" ;

		const VectorEntry& ve3 = vectorEnc.getEntry();

		EXPECT_EQ( ve3.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve3.getPosition(), 2 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve3.getLoad().getDataType(), DataType::ElementListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = ve3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains ElementLists - fourth vector forth()" ;

		const VectorEntry& ve4 = vectorEnc.getEntry();

		EXPECT_EQ( ve4.getAction(), VectorEntry::UpdateEnum ) << "VectorEntry::getAction() == VectorEntry::UpdateEnum" ;
		EXPECT_EQ( ve4.getPosition(), 3 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve4.getLoad().getDataType(), DataType::ElementListEnum ) << "VectorEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = ve4.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}

		EXPECT_FALSE( vectorEnc.forth() ) << "Vector contains ElementLists - final vector forth()" ;

		EXPECT_TRUE( true ) << "Vector contains ElementLists - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector contains ElementLists - exception not expectedd" ;
	}
}


TEST(VectorTests, testVectorContainsMapsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Vector vectorEnc;
	vectorEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with Map), Delete, Map-Set, Map-Set, Map-Update

		Map mapEncS;
		EmaEncodeMapAll( mapEncS );

		vectorEnc.summaryData( mapEncS );

		Map mapEnc1;
		EmaEncodeMapAll( mapEnc1 );

		char* permS = const_cast<char*>( "PERMISSION DATA" );
		EmaBuffer permissionData1( permS, 15 );

		//first entry  //Delete
		vectorEnc.add( 1, VectorEntry::DeleteEnum, mapEnc1, permissionData1 );

		//second entry  //Set Map
		vectorEnc.add( 1, VectorEntry::SetEnum, mapEnc1, permissionData1 );

		//third entry  //Set Map
		vectorEnc.add( 2, VectorEntry::SetEnum, mapEnc1, permissionData1 );

		//fourth entry  //Update Map
		vectorEnc.add( 3, VectorEntry::UpdateEnum, mapEnc1, permissionData1 );

		vectorEnc.complete();


		//Now do EMA decoding of Vector
		StaticDecoder::setData( &vectorEnc, &dictionary );


		EXPECT_TRUE( vectorEnc.hasTotalCountHint() ) << "Vector contains Maps - hasTotalCountHint()" ;
		EXPECT_EQ( vectorEnc.getTotalCountHint(), 5 ) << "Vector contains Maps - getTotalCountHint()" ;

		switch ( vectorEnc.getSummaryData().getDataType() )
		{
		case DataType::MapEnum :
		{
			const Map& mapS = vectorEnc.getSummaryData().getMap();
			EXPECT_TRUE( mapS.hasKeyFieldId() ) << "Vector Decode Summary Map - hasKeyFieldId()" ;
			EXPECT_EQ( mapS.getKeyFieldId(), 3426 ) << "Vector Decode Summary Map - getKeyFieldId()" ;

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Map - first map forth()" ;
			const MapEntry& me1a = mapS.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Map - second map forth()" ;
			const MapEntry& me2a = mapS.getEntry();
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

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Map - third map forth()" ;
			const MapEntry& me3a = mapS.getEntry();
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

			EXPECT_TRUE( mapS.forth() ) << "Vector Decode Summary Mapp - fourth map forth()" ;
			const MapEntry& me4a = mapS.getEntry();
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

			EXPECT_FALSE( mapS.forth() ) << "Vector Decode Summary Map - fifth map forth()" ;
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Vector Decode Summary Map - vector.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains Map - first vector forth()" ;

		const VectorEntry& ve1 = vectorEnc.getEntry();
		EXPECT_EQ( ve1.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
		EXPECT_EQ( ve1.getPosition(), 1 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve1.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

		vectorEnc.reset();
		{
			EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains Maps - vector forth() after reset()" ;

			const VectorEntry& ve = vectorEnc.getEntry();

			EXPECT_EQ( ve.getAction(), VectorEntry::DeleteEnum ) << "VectorEntry::getAction() == VectorEntry::DeleteEnum" ;
			EXPECT_EQ( ve.getLoad().getDataType(), DataType::NoDataEnum ) << "VectorEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains Map - second vector forth()" ;

		const VectorEntry& ve2 = vectorEnc.getEntry();
		EXPECT_EQ( ve2.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve2.getPosition(), 1 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve2.getLoad().getDataType(), DataType::MapEnum ) << "VectorEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& mapNested = ve2.getMap();
			EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "VectorEntry Map within series - hasKeyFieldId()" ;
			EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - first map forth()" ;
			const MapEntry& me1a = mapNested.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within map - second map forth()" ;
			const MapEntry& me2a = mapNested.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me2a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - third map forth()" ;
			const MapEntry& me3a = mapNested.getEntry();
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

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - fourth map forth()" ;
			const MapEntry& me4a = mapNested.getEntry();
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

			EXPECT_FALSE( mapNested.forth() ) << "VectorEntry Map within series - fifth map forth()" ;
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains Map - third vector forth()" ;

		const VectorEntry& ve3 = vectorEnc.getEntry();
		EXPECT_EQ( ve3.getAction(), VectorEntry::SetEnum ) << "VectorEntry::getAction() == VectorEntry::SetEnum" ;
		EXPECT_EQ( ve3.getPosition(), 2 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve3.getLoad().getDataType(), DataType::MapEnum ) << "VectorEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& mapNested = ve3.getMap();
			EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "VectorEntry Map within series - hasKeyFieldId()" ;
			EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - first map forth()" ;
			const MapEntry& me1a = mapNested.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within map - second map forth()" ;
			const MapEntry& me2a = mapNested.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me2a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - third map forth()" ;
			const MapEntry& me3a = mapNested.getEntry();
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

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - fourth map forth()" ;
			const MapEntry& me4a = mapNested.getEntry();
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

			EXPECT_FALSE( mapNested.forth() ) << "VectorEntry Map within series - fifth map forth()" ;
		}


		EXPECT_TRUE( vectorEnc.forth() ) << "Vector contains Map - fourth vector forth()" ;

		const VectorEntry& ve4 = vectorEnc.getEntry();
		EXPECT_EQ( ve4.getAction(), VectorEntry::UpdateEnum ) << "VectorEntry::getAction() == VectorEntry::UpdateEnum" ;
		EXPECT_EQ( ve4.getPosition(), 3 ) << "VectorEntry::getPostion()" ;
		EXPECT_EQ( ve4.getLoad().getDataType(), DataType::MapEnum ) << "VectorEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& mapNested = ve4.getMap();
			EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "VectorEntry Map within series - hasKeyFieldId()" ;
			EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - first map forth()" ;
			const MapEntry& me1a = mapNested.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within map - second map forth()" ;
			const MapEntry& me2a = mapNested.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me2a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - third map forth()" ;
			const MapEntry& me3a = mapNested.getEntry();
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

			EXPECT_TRUE( mapNested.forth() ) << "VectorEntry Map within series - fourth map forth()" ;
			const MapEntry& me4a = mapNested.getEntry();
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

			EXPECT_FALSE( mapNested.forth() ) << "VectorEntry Map within series - fifth map forth()" ;
		}


		EXPECT_FALSE( vectorEnc.forth() ) << "Vector contains Map - fourth vector forth()" ;

		EXPECT_TRUE( true ) << "Vector contains Map - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector contains Map - exception not expectedd" ;
	}
}

TEST(VectorTests, testVectorError)
{

	{
		try
		{
			Vector vec;
			vec.complete();
			EXPECT_FALSE( true ) << "Vector::complete() on empty vector - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Vector::complete() on empty vector - exception expected" ;
		}
	}

	{
		try
		{
			Vector vec;
			ElementList el;
			el.addAscii( "entry", "value" ).complete();
			vec.summaryData( el );
			vec.totalCountHint( 2 );
			vec.complete();

			EXPECT_TRUE( true ) << "Vector::complete() on empty vector with summary - exception not expected" ;

			StaticDecoder::setData( &vec, 0 );


			EXPECT_TRUE( vec.hasTotalCountHint() ) << "Vector::hasTotalCountHint()" ;
			EXPECT_EQ( vec.getTotalCountHint(), 2 ) << "Vector::getTotalCountHint()" ;

			EXPECT_EQ( vec.getSummaryData().getDataType(), DataType::ElementListEnum ) << "Vector::getSummaryData()::getDataType()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Vector::complete() on empty vector with summary - exception not expected" ;
		}
	}

	{
		try
		{
			Vector vec;
			ElementList el;
			el.addAscii( "entry", "value" ).complete();
			vec.summaryData( el );
			vec.totalCountHint( 2 );

			FieldList fl;
			fl.addUInt( 1, 1 ).complete();

			vec.add( 1, VectorEntry::SetEnum, fl );

			vec.complete();

			EXPECT_FALSE( true ) << "Vector::summaryData( ElementList ).add( FieldList ) - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Vector::summaryData( ElementList ).add( FieldList ) - exception expected" ;
		}
	}

	{
		try
		{
			Vector vec;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			vec.add( 1, VectorEntry::SetEnum, el );

			FieldList fl;
			fl.addUInt( 1, 1 ).complete();

			vec.add( 2, VectorEntry::SetEnum, fl );

			vec.complete();

			EXPECT_FALSE( true ) << "Vector::add( ElementList ).add( FieldList ) - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Vector::add( ElementList ).add( FieldList ) - exception expected" ;
		}
	}

	{
		try
		{
			Vector vec;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			vec.add( 1, VectorEntry::SetEnum, el );

			vec.complete();

			vec.add( 2, VectorEntry::SetEnum, el );

			EXPECT_FALSE( true ) << "Vector add after complete - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Vector add after complete - exception expected" ;
		}
	}

	{
		try
		{
			Vector vec;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			vec.add( 1, VectorEntry::SetEnum, el );

			vec.complete();

			vec.clear();

			vec.add( 2, VectorEntry::SetEnum, el );

			vec.complete();

			StaticDecoder::setData( &vec, 0 );


			EXPECT_TRUE( vec.forth() ) << "Vector::forth()" ;

			EXPECT_FALSE( vec.forth() ) << "Vector::forth()" ;

			EXPECT_TRUE( true ) << "Vector add after complete - exception not expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Vector add after complete - exception not expected" ;
		}
	}

	{
		try
		{
			Vector vec;

			ElementList el;
			el.addAscii( "entry", "value" );

			vec.add( 1, VectorEntry::SetEnum, el );

			vec.complete();

			EXPECT_FALSE( true ) << "Vector add element list while element list is not complete - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Vector add element list while element list is not complete - exception expected" ;
		}
	}

	try
	{
		Vector container;

		RefreshMsg msg;

		container.add( 1, VectorEntry::SetEnum, msg );

		EXPECT_FALSE( true ) << "Vector::add( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Vector::add( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		Vector container;

		RefreshMsg msg;

		msg.serviceId( 1 );

		container.add( 1, VectorEntry::SetEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Vector::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "VectorEntry::getLoadType()" ;

		EXPECT_TRUE( true ) << "Vector::add( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector::add( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Vector container;

		RefreshMsg msg;

		container.summaryData( msg );

		container.complete();

		EXPECT_FALSE( true ) << "Vector::summaryData( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Vector::summaryData( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		Vector container;

		RefreshMsg msg;

		msg.streamId( 10 );

		container.summaryData( msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( true ) << "Vector::summaryData( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector::summaryData( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Vector container;

		GenericMsg msg;

		container.add( 1, VectorEntry::SetEnum, msg );

		EXPECT_FALSE( true ) << "Vector::add( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Vector::add( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		Vector container;

		GenericMsg msg;

		msg.serviceId( 1 );

		container.add( 1, VectorEntry::SetEnum, msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Vector::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "VectorEntry::getLoadType()" ;

		EXPECT_TRUE( true ) << "Vector::add( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector::add( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}

	try
	{
		Vector container;

		GenericMsg msg;

		container.summaryData( msg );

		container.complete();

		EXPECT_FALSE( true ) << "Vector::summaryData( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Vector::summaryData( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		Vector container;

		GenericMsg msg;

		msg.streamId( 10 );

		container.summaryData( msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( true ) << "Vector::summaryData( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Vector::summaryData( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
}

