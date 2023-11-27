/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace std;

TEST(SeriesTests, testSeriesContainsFieldListsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with FieldList), Empty, FieldList, FieldList, FieldList

		RsslBuffer seriesBuffer;
		seriesBuffer.length = 4096;
		seriesBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslSeries rsslSeries;
		RsslEncodeIterator seriesEncodeIter;

		rsslClearSeries( &rsslSeries );
		rsslClearEncodeIterator( &seriesEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &seriesEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &seriesEncodeIter, &seriesBuffer );
		rsslSeries.flags = RSSL_SRF_HAS_SUMMARY_DATA | RSSL_SRF_HAS_TOTAL_COUNT_HINT;

		rsslSeries.containerType = RSSL_DT_FIELD_LIST;
		rsslSeries.totalCountHint = 5;

		// allocate buffer for the field list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeFieldListAll( rsslBuf );
		rsslSeries.encSummaryData = rsslBuf;

		rsslEncodeSeriesInit( &seriesEncodeIter, &rsslSeries, 0, 0 );
		RsslSeriesEntry seriesEntry;

		//first entry  //FieldList
		rsslClearSeriesEntry( &seriesEntry );
		// allocate buffer for the field list for SeriesEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf1 );
		seriesEntry.encData = rsslBuf1;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//second entry  //FieldList
		rsslClearSeriesEntry( &seriesEntry );
		RsslBuffer rsslBuf2;
		rsslBuf2.length = 1000;
		rsslBuf2.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeFieldListAll( rsslBuf2 );
		seriesEntry.encData = rsslBuf2;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//third entry  //FieldList
		rsslClearSeriesEntry( &seriesEntry );
		seriesEntry.encData = rsslBuf1;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		seriesBuffer.length = rsslGetEncodedBufferLength( &seriesEncodeIter );
		rsslEncodeSeriesComplete( &seriesEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Series
		Series series;
		StaticDecoder::setRsslData( &series, &seriesBuffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( series.hasTotalCountHint() ) << "Series contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( series.getTotalCountHint(), 5 ) << "Series contains FieldList - getTotalCountHint()" ;

		switch ( series.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
		{
			const FieldList& fl = series.getSummaryData().getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Series Decode Summary FieldList - series.getSummaryType() not expected" ;
			break;
		}

//		EXPECT_TRUE( series.forth() ) << "Series contains FieldList - first series forth()" ;
//
//		const SeriesEntry& se1 = series.getEntry();
//		EXPECT_EQ( se1.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
//		{
//			const FieldList& fl = se1.getFieldList();
//			EXPECT_TRUE( fl.hasInfo() ) << "SeriesEntry FieldList within series - hasInfo()" ;
//			EXPECT_FALSE( fl.forth() ) << "Series Decode Summary FieldList - first and final fieldlist forth()" ;
//		}
		EXPECT_TRUE( series.forth() ) << "Series contains FieldList - second series forth()" ;

		const SeriesEntry& se1 = series.getEntry();
		EXPECT_EQ( se1.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = se1.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		series.reset();
		{
//			EXPECT_TRUE( series.forth() ) << "Series contains FieldList - series forth() after reset()" ;
//
//			const SeriesEntry& se = series.getEntry();
//			EXPECT_EQ( se.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
//			{
//				const FieldList& fl = se1.getFieldList();
//				EXPECT_FALSE( fl.hasInfo() ) << "SeriesEntry FieldList within series - hasInfo()" ;
//				EXPECT_FALSE( fl.forth() ) << "Series Decode Summary FieldList - first and final fieldlist forth()" ;
//			}
			EXPECT_TRUE( series.forth() ) << "Series contains FieldList - series forth() after reset" ;

			const SeriesEntry& se = series.getEntry();
			EXPECT_EQ( se.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = se.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}
		}


		EXPECT_TRUE( series.forth() ) << "Series contains FieldList - second series forth()" ;

		const SeriesEntry& se2 = series.getEntry();
		EXPECT_EQ( se2.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = se2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( series.forth() ) << "Series contains FieldList - third series forth()" ;

		const SeriesEntry& se3 = series.getEntry();
		EXPECT_EQ( se3.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		{
			const FieldList& fl = se3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( series.forth() ) << "Series contains FieldList - final series forth()" ;

		free( rsslBuf.data );
		free( seriesBuffer.data );

		EXPECT_TRUE( true ) << "Series contains FieldList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains FieldList - exception not expectedd" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(SeriesTests, testSeriesContainsElementListsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with ElementList), Empty, ElementList, ElementList, ElementList

		RsslBuffer seriesBuffer;
		seriesBuffer.length = 4096;
		seriesBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslSeries rsslSeries = RSSL_INIT_SERIES;
		RsslEncodeIterator seriesEncodeIter;

		rsslClearSeries( &rsslSeries );
		rsslClearEncodeIterator( &seriesEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &seriesEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &seriesEncodeIter, &seriesBuffer );
		rsslSeries.flags = RSSL_SRF_HAS_SUMMARY_DATA | RSSL_SRF_HAS_TOTAL_COUNT_HINT;

		rsslSeries.containerType = RSSL_DT_ELEMENT_LIST;
		rsslSeries.totalCountHint = 5;

		// allocate buffer for the element list for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeElementListAll( rsslBuf );
		rsslSeries.encSummaryData = rsslBuf;

		rsslEncodeSeriesInit( &seriesEncodeIter, &rsslSeries, 0, 0 );
		RsslSeriesEntry seriesEntry;

		//first entry  //ElementList
		rsslClearSeriesEntry( &seriesEntry );
		// allocate buffer for the element list for SeriesEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeElementListAll( rsslBuf1 );
		seriesEntry.encData = rsslBuf1;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//second entry  //ElementList
		rsslClearSeriesEntry( &seriesEntry );
		RsslBuffer rsslBuf2;
		rsslBuf2.length = 1000;
		rsslBuf2.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeElementListAll( rsslBuf2 );
		seriesEntry.encData = rsslBuf2;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//third entry  //ElementList
		rsslClearSeriesEntry( &seriesEntry );
		seriesEntry.encData = rsslBuf1;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		seriesBuffer.length = rsslGetEncodedBufferLength( &seriesEncodeIter );
		rsslEncodeSeriesComplete( &seriesEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Series
		Series series;
		StaticDecoder::setRsslData( &series, &seriesBuffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( series.hasTotalCountHint() ) << "Series contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( series.getTotalCountHint(), 5 ) << "Series contains ElementList - getTotalCountHint()" ;

		switch ( series.getSummaryData().getDataType() )
		{
		case DataType::ElementListEnum :
		{
			const ElementList& el = series.getSummaryData().getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Series Decode Summary FieldList - series.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( series.forth() ) << "Series contains ElementList - first series forth()" ;

		const SeriesEntry& se1 = series.getEntry();
		EXPECT_EQ( se1.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = se1.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		series.reset();
		{
			EXPECT_TRUE( series.forth() ) << "Series contains ElementList - series forth() after reset()" ;

			const SeriesEntry& se = series.getEntry();
			EXPECT_EQ( se.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
			{
				const ElementList& el = se.getElementList();
				SCOPED_TRACE("calling EmaDecodeElementListAll");
				EmaDecodeElementListAll( el );
			}
		}

		EXPECT_TRUE( series.forth() ) << "Series contains ElementList - second series forth()" ;

		const SeriesEntry& se2 = series.getEntry();
		EXPECT_EQ( se2.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = se2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( series.forth() ) << "Series contains ElementList - third series forth()" ;

		const SeriesEntry& se3 = series.getEntry();
		EXPECT_EQ( se3.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = se3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_FALSE( series.forth() ) << "Series contains ElementList - final series forth()" ;

		free( rsslBuf.data );
		free( seriesBuffer.data );

		EXPECT_TRUE( true ) << "Series contains ElementList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains ElementList - exception not expectedd" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(SeriesTests, testSeriesContainsMapsDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  SummaryData(with Map), Empty, Map, Map, Map

		RsslBuffer seriesBuffer;
		seriesBuffer.length = 4096;
		seriesBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslSeries rsslSeries = RSSL_INIT_SERIES;
		RsslEncodeIterator seriesEncodeIter;

		rsslClearSeries( &rsslSeries );
		rsslClearEncodeIterator( &seriesEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &seriesEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &seriesEncodeIter, &seriesBuffer );
		rsslSeries.flags = RSSL_SRF_HAS_SUMMARY_DATA | RSSL_SRF_HAS_TOTAL_COUNT_HINT;

		rsslSeries.containerType = RSSL_DT_MAP;
		rsslSeries.totalCountHint = 5;

		// allocate buffer for the map for SummaryData
		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		RsslEncodeMapAll( rsslBuf );
		rsslSeries.encSummaryData = rsslBuf;

		rsslEncodeSeriesInit( &seriesEncodeIter, &rsslSeries, 0, 0 );
		RsslSeriesEntry seriesEntry;

		//first entry  //Empty Map
		rsslClearSeriesEntry( &seriesEntry );
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//second entry  //Map
		rsslClearSeriesEntry( &seriesEntry );
		// allocate buffer for the element list for SeriesEntries
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeMapAll( rsslBuf1 );
		seriesEntry.encData = rsslBuf1;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//third entry  //Map
		rsslClearSeriesEntry( &seriesEntry );
		RsslBuffer rsslBuf2;
		rsslBuf2.length = 1000;
		rsslBuf2.data = ( char* )malloc( sizeof( char ) * 1000 );
		RsslEncodeMapAll( rsslBuf2 );
		seriesEntry.encData = rsslBuf2;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		//fourth entry  //Map
		rsslClearSeriesEntry( &seriesEntry );
		seriesEntry.encData = rsslBuf1;
		rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		seriesBuffer.length = rsslGetEncodedBufferLength( &seriesEncodeIter );
		rsslEncodeSeriesComplete( &seriesEncodeIter, RSSL_TRUE );


		//Now do EMA decoding of Series
		Series series;
		StaticDecoder::setRsslData( &series, &seriesBuffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( series.hasTotalCountHint() ) << "Series contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( series.getTotalCountHint(), 5 ) << "Series contains Map - getTotalCountHint()" ;

		switch ( series.getSummaryData().getDataType() )
		{
		case DataType::MapEnum :
		{
			const Map& mapS = series.getSummaryData().getMap();

			EXPECT_TRUE( mapS.hasKeyFieldId() ) << "Series Decode Summary Map - hasKeyFieldId()" ;
			EXPECT_EQ( mapS.getKeyFieldId(), 3426 ) << "Series Decode Summary Map - getKeyFieldId()" ;

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Map - first map forth()" ;
			const MapEntry& me1a = mapS.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Map - second map forth()" ;
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

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Map - third map forth()" ;
			const MapEntry& me3a = mapS.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Mapp - fourth map forth()" ;
			const MapEntry& me4a = mapS.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( mapS.forth() ) << "Series Decode Summary Map - fifth map forth()" ;
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Series Decode Summary Map - series.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( series.forth() ) << "Series contains Map - first series forth()" ;

		const SeriesEntry& se1 = series.getEntry();
		EXPECT_EQ( se1.getLoad().getDataType(), DataType::MapEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = se1.getMap();
			EXPECT_FALSE( map.forth() ) << "Series Decode Summary Map - first and final map forth()" ;
		}

		series.reset();
		{
			EXPECT_TRUE( series.forth() ) << "Series contains Map - series forth() after reset()" ;

			const SeriesEntry& se = series.getEntry();

			EXPECT_EQ( se.getLoad().getDataType(), DataType::MapEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::MapEnum" ;
			{
				const Map& map = se.getMap();
				EXPECT_FALSE( map.forth() ) << "Series Decode Summary Map - first and final map forth()" ;
			}
		}


		EXPECT_TRUE( series.forth() ) << "Series contains Map - second series forth()" ;

		const SeriesEntry& se2 = series.getEntry();
		EXPECT_EQ( se2.getLoad().getDataType(), DataType::MapEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = se2.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "SeriesEntry Map within series - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - second map forth()" ;
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

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "SeriesEntry Map within series - fifth map forth()" ;
		}


		EXPECT_TRUE( series.forth() ) << "Series contains ElementList - third series forth()" ;

		const SeriesEntry& se3 = series.getEntry();

		EXPECT_EQ( se3.getLoad().getDataType(), DataType::MapEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = se3.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "SeriesEntry Map within series - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - second map forth()" ;
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

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "SeriesEntry Map within series - fifth map forth()" ;
		}


		EXPECT_TRUE( series.forth() ) << "Series contains Map - fourth series forth()" ;

		const SeriesEntry& se4 = series.getEntry();

		EXPECT_EQ( se4.getLoad().getDataType(), DataType::MapEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = se4.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "SeriesEntry Map within series - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - second map forth()" ;
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

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "SeriesEntry Map within series - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "SeriesEntry Map within series - fifth map forth()" ;
		}


		EXPECT_FALSE( series.forth() ) << "Series contains Map - final series forth()" ;

		free( rsslBuf.data );
		free( seriesBuffer.data );

		EXPECT_TRUE( true ) << "Series contains Map - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains Map - exception not expectedd" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(SeriesTests, testSeriesContainsOpaqueDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{

		RsslBuffer seriesBuffer;
		seriesBuffer.length = 4096;
		seriesBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslSeries rsslSeries = RSSL_INIT_SERIES;
		RsslEncodeIterator seriesEncodeIter;

		rsslClearSeries( &rsslSeries );
		rsslClearEncodeIterator( &seriesEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &seriesEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &seriesEncodeIter, &seriesBuffer );
		rsslSeries.flags = RSSL_SRF_HAS_TOTAL_COUNT_HINT;

		rsslSeries.containerType = RSSL_DT_OPAQUE;
		rsslSeries.totalCountHint = 1;

		RsslRet ret = rsslEncodeSeriesInit( &seriesEncodeIter, &rsslSeries, 0, 0 );
		RsslSeriesEntry seriesEntry;

		rsslClearSeriesEntry( &seriesEntry );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		seriesEntry.encData = rsslBuf;

		ret = rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		ret = rsslEncodeSeriesComplete( &seriesEncodeIter, RSSL_TRUE );

		seriesBuffer.length = rsslGetEncodedBufferLength( &seriesEncodeIter );

		Series series;
		StaticDecoder::setRsslData( &series, &seriesBuffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( series.forth() ) << "Series contains Opaque - first forth()" ;

		const SeriesEntry& se = series.getEntry();
		EXPECT_EQ( se.getLoadType(), DataType::OpaqueEnum ) << "ElementEntry::getLoadType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( se.getOpaque().getBuffer(), compareTo ) << "ElementEntry::getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains Opaque payload - exception not expected" ;
	}
}

TEST(SeriesTests, testSeriesContainsXmlDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslBuffer seriesBuffer;
		seriesBuffer.length = 4096;
		seriesBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslSeries rsslSeries = RSSL_INIT_SERIES;
		RsslEncodeIterator seriesEncodeIter;

		rsslClearSeries( &rsslSeries );
		rsslClearEncodeIterator( &seriesEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &seriesEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &seriesEncodeIter, &seriesBuffer );
		rsslSeries.flags = RSSL_SRF_HAS_TOTAL_COUNT_HINT;

		rsslSeries.containerType = RSSL_DT_XML;
		rsslSeries.totalCountHint = 1;

		RsslRet ret = rsslEncodeSeriesInit( &seriesEncodeIter, &rsslSeries, 0, 0 );
		RsslSeriesEntry seriesEntry;

		rsslClearSeriesEntry( &seriesEntry );

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		seriesEntry.encData = rsslBuf;

		ret = rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		ret = rsslEncodeSeriesComplete( &seriesEncodeIter, RSSL_TRUE );

		seriesBuffer.length = rsslGetEncodedBufferLength( &seriesEncodeIter );

		Series series;
		StaticDecoder::setRsslData( &series, &seriesBuffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( series.forth() ) << "Series contains Opaque - first forth()" ;

		const SeriesEntry& se = series.getEntry();
		EXPECT_EQ( se.getLoadType(), DataType::XmlEnum ) << "ElementEntry::getLoadType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( se.getXml().getBuffer(), compareTo ) << "ElementEntry::getXml().getBuffer()" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains Xml payload - exception not expected" ;
	}
}

TEST(SeriesTests, testSeriesContainsAnsiPageDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{

		RsslBuffer seriesBuffer;
		seriesBuffer.length = 4096;
		seriesBuffer.data = ( char* )malloc( sizeof( char ) * 4096 );

		RsslSeries rsslSeries = RSSL_INIT_SERIES;
		RsslEncodeIterator seriesEncodeIter;

		rsslClearSeries( &rsslSeries );
		rsslClearEncodeIterator( &seriesEncodeIter );
		rsslSetEncodeIteratorRWFVersion( &seriesEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &seriesEncodeIter, &seriesBuffer );
		rsslSeries.flags = RSSL_SRF_HAS_TOTAL_COUNT_HINT;

		rsslSeries.containerType = RSSL_DT_ANSI_PAGE;
		rsslSeries.totalCountHint = 1;

		// allocate buffer for the map for SummaryData

		RsslRet ret = rsslEncodeSeriesInit( &seriesEncodeIter, &rsslSeries, 0, 0 );
		RsslSeriesEntry seriesEntry;

		//first entry  //Empty Map
		rsslClearSeriesEntry( &seriesEntry );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		seriesEntry.encData = rsslBuf;

		ret = rsslEncodeSeriesEntry( &seriesEncodeIter, &seriesEntry );

		ret = rsslEncodeSeriesComplete( &seriesEncodeIter, RSSL_TRUE );

		seriesBuffer.length = rsslGetEncodedBufferLength( &seriesEncodeIter );

		Series series;
		StaticDecoder::setRsslData( &series, &seriesBuffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( series.forth() ) << "Series contains AnsiPage - first forth()" ;

		const SeriesEntry& se = series.getEntry();
		EXPECT_EQ( se.getLoadType(), DataType::AnsiPageEnum ) << "ElementEntry::getLoadType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( se.getAnsiPage().getBuffer(), compareTo ) << "ElementEntry::getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains AnsiPage payload - exception not expected" ;
	}
}

void seriesOfFieldList_RsslEncodeEmaDecode( bool useSetDefinitions, std::string& decodedMsg )
{
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

		RsslSeries rsslSeries;
		rsslClearSeries( &rsslSeries );
		if ( useSetDefinitions )
		{
			rsslSeries.flags = RSSL_MPF_HAS_SET_DEFS;
		}
		rsslSeries.containerType = RSSL_DT_FIELD_LIST;
		rsslEncodeSeriesInit( &encodeIter, &rsslSeries, 0, 0 );

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
			rsslEncodeSeriesSetDefsComplete( &encodeIter, RSSL_TRUE );
		}

		RsslSeriesEntry seriesEntry;
		rsslClearSeriesEntry( &seriesEntry );
		rsslEncodeSeriesEntryInit( &encodeIter, &seriesEntry, 0 );

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
		rsslEncodeSeriesEntryComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeSeriesComplete( &encodeIter, RSSL_TRUE );

		Series series;
		StaticDecoder::setRsslData( &series, &buffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );
		decodedMsg = series;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Decoding of Series of FieldList, encoded from rssl set defs - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(SeriesTests, testSeriesContainsFieldListSetDefinitionsDecode)
{

	std::string msgFromStandardData;
	seriesOfFieldList_RsslEncodeEmaDecode( false, msgFromStandardData );
	std::string msgFromSetDef;
	seriesOfFieldList_RsslEncodeEmaDecode( true, msgFromSetDef );

	EXPECT_EQ( msgFromSetDef, msgFromStandardData ) << "Encoding from set definitions results in same decoded message." ;
}

void seriesOfElementList_RsslEncodeEmaDecode( bool useSetDefinitions, std::string& decodedMsg )

{
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

		RsslSeries rsslSeries;
		rsslClearSeries( &rsslSeries );
		if ( useSetDefinitions )
		{
			rsslSeries.flags = RSSL_MPF_HAS_SET_DEFS;
		}
		rsslSeries.containerType = RSSL_DT_ELEMENT_LIST;
		rsslEncodeSeriesInit( &encodeIter, &rsslSeries, 0, 0 );

		RsslLocalElementSetDefDb elementSetDefDb;
		RsslElementSetDefEntry elementSetDefEntries[3] =
		{
		  { {3, const_cast<char*>("BID")}, RSSL_DT_REAL },
		  { {3, const_cast<char*>("ASK")}, RSSL_DT_REAL_8RB },
		  { {10, const_cast<char*>("TRADE_TIME")}, RSSL_DT_TIME_3 }
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
			rsslEncodeSeriesSetDefsComplete( &encodeIter, RSSL_TRUE );
		}

		RsslSeriesEntry seriesEntry;
		rsslClearSeriesEntry( &seriesEntry );
		rsslEncodeSeriesEntryInit( &encodeIter, &seriesEntry, 0 );

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
		rsslEncodeSeriesEntryComplete( &encodeIter, RSSL_TRUE );
		rsslEncodeSeriesComplete( &encodeIter, RSSL_TRUE );

		Series series;
		StaticDecoder::setRsslData( &series, &buffer, RSSL_DT_SERIES, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );
		decodedMsg = series;
	}
	catch ( const OmmException& )
	{
	  EXPECT_FALSE( true ) << "Decoding of Series of ElementList, encoded from rssl set defs - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(SeriesTests, testSeriesEmptyEncode)
{
	try
	{
		Series series;
		series.totalCountHint(0).complete();

		StaticDecoder::setData(&series, NULL);

		EXPECT_TRUE(series.hasTotalCountHint()) << "Check has total count hint attribute";
		EXPECT_TRUE(series.getTotalCountHint() == 0) << "Check the total count hint attribute";
		EXPECT_FALSE(series.forth()) << "Check to make sure that there is no enty in Series";
	}
	catch ( OmmException& excp )
	{
		EmaString text;
		EXPECT_FALSE( true ) << text.append( "empty Series - exception not expected: " ).append( excp.getText() ) ;
		return;
	}
}

TEST(SeriesTests, testSeriesContainsFieldListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Series seriesEnc;
	EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";

	seriesEnc.totalCountHint( 5 );
	EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";


	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with FieldList), Empty, FieldList, FieldList, FieldList

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		seriesEnc.summaryData( flEnc );
		EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";


		//first entry  //FieldList
		FieldList flEnc1;
		EmaEncodeFieldListAll( flEnc1 );
		seriesEnc.add( flEnc1 );
		EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";

		//second entry  //FieldList
		FieldList flEnc2;
		EmaEncodeFieldListAll( flEnc2 );
		seriesEnc.add( flEnc2 );
		EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";

		//third entry  //FieldList
		seriesEnc.add( flEnc1 );
		EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";

		seriesEnc.complete();
		EXPECT_EQ( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() == Decoding of just encoded object in the same application is not supported";


		//Now do EMA decoding of Series
		StaticDecoder::setData( &seriesEnc, &dictionary );
		EXPECT_NE( seriesEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "Series.toString() != Decoding of just encoded object in the same application is not supported";


		EXPECT_TRUE( seriesEnc.hasTotalCountHint() ) << "Series contains FieldList - hasTotalCountHint()" ;
		EXPECT_EQ( seriesEnc.getTotalCountHint(), 5 ) << "Series contains FieldList - getTotalCountHint()" ;

		switch ( seriesEnc.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
		{
			const FieldList& fl = seriesEnc.getSummaryData().getFieldList();
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( fl );
			}
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Series Decode Summary FieldList - series.getSummaryType() not expected" ;
			break;
		}

//		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains FieldList - first series forth()" ;
//
//		const SeriesEntry& se1 = seriesEnc.getEntry();
//		EXPECT_EQ( se1.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
//		{
//			const FieldList& fl = se1.getFieldList();
//			EXPECT_TRUE( fl.hasInfo() ) << "SeriesEntry FieldList within series - hasInfo()" ;
//			EXPECT_FALSE( fl.forth() ) << "Series Decode Summary FieldList - first and final fieldlist forth()" ;
//		}
		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains FieldList - second series forth()" ;

		const SeriesEntry& se1 = seriesEnc.getEntry();
		EXPECT_EQ( se1.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = se1.getFieldList();
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( fl );
			}
		}


		seriesEnc.reset();
		{
//			EXPECT_TRUE( seriesEnc.forth() ) << "Series contains FieldList - series forth() after reset()" ;
//
//			const SeriesEntry& se = seriesEnc.getEntry();
//			EXPECT_EQ( se.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
//			{
//				const FieldList& fl = se1.getFieldList();
//				EXPECT_FALSE( fl.hasInfo() ) << "SeriesEntry FieldList within series - hasInfo()" ;
//				EXPECT_FALSE( fl.forth() ) << "Series Decode Summary FieldList - first and final fieldlist forth()" ;
//			}
			EXPECT_TRUE( seriesEnc.forth() ) << "Series contains FieldList - series forth() after reset" ;

			const SeriesEntry& se = seriesEnc.getEntry();
			EXPECT_EQ( se.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = se.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}
		}


		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains FieldList - second series forth()" ;

		const SeriesEntry& se2 = seriesEnc.getEntry();
		EXPECT_EQ( se2.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = se2.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}


		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains FieldList - third series forth()" ;

		const SeriesEntry& se3 = seriesEnc.getEntry();
		EXPECT_EQ( se3.getLoad().getDataType(), DataType::FieldListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::NoDataEnum" ;
		{
			const FieldList& fl = se3.getFieldList();
			SCOPED_TRACE("calling EmaDecodeFieldListAll");
			EmaDecodeFieldListAll( fl );
		}

		EXPECT_FALSE( seriesEnc.forth() ) << "Series contains FieldList - final series forth()" ;

		EXPECT_TRUE( true ) << "Series contains FieldList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains FieldList - exception not expectedd" ;
	}
}

TEST(SeriesTests, testSeriesContainsElementListsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Series seriesEnc;
	seriesEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with ElementList), ElementList, ElementList, ElementList

		ElementList elEncS;
		EmaEncodeElementListAll( elEncS );

		seriesEnc.summaryData( elEncS );

		//first entry  //ElementList
		ElementList elEnc1;
		EmaEncodeElementListAll( elEnc1 );
		seriesEnc.add( elEnc1 );

		//second entry  //ElementList
		ElementList elEnc2;
		EmaEncodeElementListAll( elEnc2 );
		seriesEnc.add( elEnc2 );

		//third entry  //ElementList
		seriesEnc.add( elEnc1 );

		seriesEnc.complete();


		//Now do EMA decoding of Series
		StaticDecoder::setData( &seriesEnc, &dictionary );


		EXPECT_TRUE( seriesEnc.hasTotalCountHint() ) << "Series contains ElementList - hasTotalCountHint()" ;
		EXPECT_EQ( seriesEnc.getTotalCountHint(), 5 ) << "Series contains ElementList - getTotalCountHint()" ;

		switch ( seriesEnc.getSummaryData().getDataType() )
		{
		case DataType::ElementListEnum :
		{
			const ElementList& el = seriesEnc.getSummaryData().getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Series Decode Summary FieldList - series.getSummaryType() not expected" ;
			break;
		}

		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains ElementList - first series forth()" ;

		const SeriesEntry& se1 = seriesEnc.getEntry();
		EXPECT_EQ( se1.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = se1.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		seriesEnc.reset();
		{
			EXPECT_TRUE( seriesEnc.forth() ) << "Series contains ElementList - series forth() after reset()" ;

			const SeriesEntry& se = seriesEnc.getEntry();
			EXPECT_EQ( se.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
			{
				const ElementList& el = se.getElementList();
				SCOPED_TRACE("calling EmaDecodeElementListAll");
				EmaDecodeElementListAll( el );
			}
		}

		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains ElementList - second series forth()" ;

		const SeriesEntry& se2 = seriesEnc.getEntry();
		EXPECT_EQ( se2.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = se2.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_TRUE( seriesEnc.forth() ) << "Series contains ElementList - third series forth()" ;

		const SeriesEntry& se3 = seriesEnc.getEntry();
		EXPECT_EQ( se3.getLoad().getDataType(), DataType::ElementListEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& el = se3.getElementList();
			SCOPED_TRACE("calling EmaDecodeElementListAll");
			EmaDecodeElementListAll( el );
		}


		EXPECT_FALSE( seriesEnc.forth() ) << "Series contains ElementList - final series forth()" ;

		EXPECT_TRUE( true ) << "Series contains ElementList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains ElementList - exception not expectedd" ;
	}
}

TEST(SeriesTests, testSeriesContainsMapsEncodeDecodeAll)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	Series seriesEnc;
	seriesEnc.totalCountHint( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  SummaryData(with Map), Map, Map, Map

		Map mapEncS;
		EmaEncodeMapAll( mapEncS );

		seriesEnc.summaryData( mapEncS );

		//first entry  //Map
		Map mapEnc1;
		EmaEncodeMapAll( mapEnc1 );
		seriesEnc.add( mapEnc1 );

		//second entry  //Map
		Map mapEnc2;
		EmaEncodeMapAll( mapEnc2 );
		seriesEnc.add( mapEnc2 );

		//third entry  //Map
		seriesEnc.add( mapEnc1 );

		seriesEnc.complete();


		//Now do EMA decoding of Series
		StaticDecoder::setData( &seriesEnc, &dictionary );


		EXPECT_TRUE( seriesEnc.hasTotalCountHint() ) << "Series contains Map - hasTotalCountHint()" ;
		EXPECT_EQ( seriesEnc.getTotalCountHint(), 5 ) << "Series contains Map - getTotalCountHint()" ;

		switch ( seriesEnc.getSummaryData().getDataType() )
		{
		case DataType::MapEnum :
		{
			const Map& mapS = seriesEnc.getSummaryData().getMap();
			EXPECT_TRUE( mapS.hasKeyFieldId() ) << "Series Decode Summary Map - hasKeyFieldId()" ;
			EXPECT_EQ( mapS.getKeyFieldId(), 3426 ) << "Series Decode Summary Map - getKeyFieldId()" ;

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Map - first map forth()" ;
			const MapEntry& me1a = mapS.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Map - second map forth()" ;
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

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Map - third map forth()" ;
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

			EXPECT_TRUE( mapS.forth() ) << "Series Decode Summary Mapp - fourth map forth()" ;
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

			EXPECT_FALSE( mapS.forth() ) << "Series Decode Summary Map - fifth map forth()" ;
		}
		break;
		default :
			EXPECT_FALSE( true ) << "Series Decode Summary Map - series.getSummaryType() not expected" ;
			break;
		}

		//3 SeriesEntries (with the same Map)
		for ( int i = 0; i < 3; i++ )
		{
			EXPECT_TRUE( seriesEnc.forth() ) << "Series contains Map - next forth()" ;

			const SeriesEntry& nextSeriesEntry = seriesEnc.getEntry();
			EXPECT_EQ( nextSeriesEntry.getLoad().getDataType(), DataType::MapEnum ) << "SeriesEntry::getLoad().getDataType() == DataType::MapEnum" ;
			{
				const Map& mapNested = nextSeriesEntry.getMap();
				EXPECT_TRUE( mapNested.hasKeyFieldId() ) << "SeriesEntry Map within series - hasKeyFieldId()" ;
				EXPECT_EQ( mapNested.getKeyFieldId(), 3426 ) << "SeriesEntry Map within series - getKeyFieldId()" ;

				EXPECT_TRUE( mapNested.forth() ) << "SeriesEntry Map within series - first map forth()" ;
				const MapEntry& me1a = mapNested.getEntry();
				EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
				{
					EmaBuffer Buf( "ABCD", 4 );
					EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
				}
				EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
				EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

				EXPECT_TRUE( mapNested.forth() ) << "SeriesEntry Map within map - second map forth()" ;
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

				EXPECT_TRUE( mapNested.forth() ) << "SeriesEntry Map within series - third map forth()" ;
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

				EXPECT_TRUE( mapNested.forth() ) << "SeriesEntry Map within series - fourth map forth()" ;
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

				EXPECT_FALSE( mapNested.forth() ) << "SeriesEntry Map within series - fifth map forth()" ;
			}
		}

		EXPECT_FALSE( seriesEnc.forth() ) << "Series contains Map - fourth forth()" ;

		EXPECT_TRUE( true ) << "Series contains Map - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series contains Map - exception not expectedd" ;
	}
}


TEST(SeriesTests, testSeriesContainsElementListSetDefinitionsDecode)
{

	std::string msgFromStandardData;
	seriesOfElementList_RsslEncodeEmaDecode( false, msgFromStandardData );
	std::string msgFromSetDef;
	seriesOfElementList_RsslEncodeEmaDecode( true, msgFromSetDef );

	EXPECT_EQ( msgFromSetDef, msgFromStandardData ) << "Encoding from set definitions results in same decoded message." ;
}

TEST(SeriesTests, testSeriesPrePostBindElementList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode ElementList via prebind
		Series series1;
		{
			ElementList elementList;
			series1.add( elementList );
			EmaEncodeElementListAll( elementList );
			series1.complete();
			StaticDecoder::setData( &series1, &dictionary );
		}

		// Encode ElementList via postbind
		Series series2;
		{
			ElementList elementList;
			EmaEncodeElementListAll( elementList );
			series2.add( elementList );
			series2.complete();
			StaticDecoder::setData( &series2, &dictionary );
		}

		EXPECT_STREQ( series1.toString(), series2.toString() ) << "Pre/Post-bound ElementLists are equal - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound ElementLists are equal - exception not expected" ;
	}
}

TEST(SeriesTests, testSeriesPrePostBindFieldList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode FieldList via prebind
		Series series1;
		{
			FieldList fieldList;
			series1.add( fieldList );
			EmaEncodeFieldListAll( fieldList );
			series1.complete();
			StaticDecoder::setData( &series1, &dictionary );
		}

		// Encode FieldList via postbind
		Series series2;
		{
			FieldList fieldList;
			EmaEncodeFieldListAll( fieldList );
			series2.add( fieldList );
			series2.complete();
			StaticDecoder::setData( &series2, &dictionary );
		}

		EXPECT_STREQ( series1.toString(), series2.toString() ) << "Pre/Post-bound FieldLists are equal - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound FieldLists are equal - exception not expected" ;
	}
}

TEST(SeriesTests, testSeriesError)
{
	{
		try
		{
			Series series;
			ElementList el;
			el.addAscii( "entry", "value" ).complete();
			series.summaryData( el );
			series.totalCountHint( 2 );
			series.complete();

			EXPECT_TRUE( true ) << "Series::complete() on empty series with summary - exception not expected" ;

			StaticDecoder::setData( &series, 0 );


			EXPECT_TRUE( series.hasTotalCountHint() ) << "Series::hasTotalCountHint()" ;
			EXPECT_EQ( series.getTotalCountHint(), 2 ) << "Series::getTotalCountHint()" ;

			EXPECT_EQ( series.getSummaryData().getDataType(), DataType::ElementListEnum ) << "Series::getSummaryData()::getDataType()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Series::complete() on empty series with summary - exception not expected" ;
		}
	}

	{
		try
		{
			Series series;
			ElementList el;
			el.addAscii( "entry", "value" ).complete();
			series.summaryData( el );
			series.totalCountHint( 2 );

			FieldList fl;
			fl.addUInt( 1, 1 ).complete();

			series.add( fl );

			series.complete();

			EXPECT_FALSE( true ) << "Series::summaryData( ElementList ).add( FieldList ) - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Series::summaryData( ElementList ).add( FieldList ) - exception expected" ;
		}
	}

	{
		try
		{
			Series series;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			series.add( el );

			FieldList fl;
			fl.addUInt( 1, 1 ).complete();

			series.add( fl );

			series.complete();

			EXPECT_FALSE( true ) << "Series::add( ElementList ).add( FieldList ) - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Series::add( ElementList ).add( FieldList ) - exception expected" ;
		}
	}

	{
		try
		{
			Series series;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			series.add( el );

			series.complete();

			series.add( el );

			EXPECT_FALSE( true ) << "Series add after complete - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Series add after complete - exception expected" ;
		}
	}

	{
		try
		{
			Series series;

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			series.add( el );

			series.complete();

			series.clear();

			series.add( el );

			series.complete();

			StaticDecoder::setData( &series, 0 );


			EXPECT_TRUE( series.forth() ) << "Series::forth()" ;

			EXPECT_FALSE( series.forth() ) << "Series::forth()" ;

			EXPECT_TRUE( true ) << "Series add after complete - exception not expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "Series add after complete - exception not expected" ;
		}
	}

	{
		try
		{
			Series series;

			ElementList el;
			el.addAscii( "entry", "value" );

			series.add( el );

			series.complete();

			EXPECT_FALSE( true ) << "Series add element list while element list is not complete - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "Series add element list while element list is not complete - exception expected" ;
		}
	}

	try
	{
		Series container;

		RefreshMsg msg;

		container.add( msg );

		EXPECT_FALSE( true ) << "Series::add( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Series::add( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		Series container;

		RefreshMsg msg;

		msg.serviceId( 1 );

		container.add( msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Series::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "SeriesEntry::getLoadType()" ;

		EXPECT_TRUE( true ) << "Series::add( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series::add( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Series container;

		RefreshMsg msg;

		container.summaryData( msg );

		container.complete();

		EXPECT_FALSE( true ) << "Series::summaryData( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Series::summaryData( RefreshMsg ) while RefreshMsg is empty - exception expected" ;
	}

	try
	{
		Series container;

		RefreshMsg msg;

		msg.streamId( 10 );

		container.summaryData( msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( true ) << "Series::summaryData( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series::summaryData( RefreshMsg ) while RefreshMsg is populated - exception not expected" ;
	}

	try
	{
		Series container;

		GenericMsg msg;

		container.add( msg );

		EXPECT_FALSE( true ) << "Series::add( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Series::add( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		Series container;

		GenericMsg msg;

		msg.serviceId( 1 );

		container.add( msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( container.forth() ) << "Series::forht()" ;

		EXPECT_EQ( container.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "SeriesEntry::getLoadType()" ;

		EXPECT_TRUE( true ) << "Series::add( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series::add( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}

	try
	{
		Series container;

		GenericMsg msg;

		container.summaryData( msg );

		container.complete();

		EXPECT_FALSE( true ) << "Series::summaryData( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Series::summaryData( GenericMsg ) while GenericMsg is empty - exception expected" ;
	}

	try
	{
		Series container;

		GenericMsg msg;

		msg.streamId( 10 );

		container.summaryData( msg );

		container.complete();

		StaticDecoder::setData( &container, 0 );


		EXPECT_TRUE( true ) << "Series::summaryData( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Series::summaryData( GenericMsg ) while GenericMsg is populated - exception not expected" ;
	}
}

TEST(SeriesTests, testSeriesEntryWithNoPayload_Encode_Decode)
{
	try
	{
		Series series;
		series.totalCountHint(3)
			.add()
			.add()
			.add()
			.complete();

		StaticDecoder::setData(&series, NULL);

		EXPECT_TRUE(series.hasTotalCountHint()) << "Check wheter has total count hint in Series";
		EXPECT_TRUE(series.getTotalCountHint() == 3) << "Check total count hint of Series";

		EXPECT_TRUE(series.forth()) << "Get the first Series entry";
		EXPECT_TRUE(series.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first entry";

		EXPECT_TRUE(series.forth()) << "Get the second Series entry";
		EXPECT_TRUE(series.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second entry";

		EXPECT_TRUE(series.forth()) << "Get the third Series entry";
		EXPECT_TRUE(series.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the third entry";

		EXPECT_FALSE(series.forth()) << "Check to make sure that there is no more enty in Series";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode and decode Series - exception not expected with text" << exp.getText().c_str();
	}
}

TEST(SeriesTests, testSeriesAddTotalCountAfterInitialized)
{
	try
	{
		Series series;
		series.add().totalCountHint(5).complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Encode total count hint after Series is initialized - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Invalid attempt to call totalCountHint() when container is initialized.", exp.getText().c_str());
		return;
	}

	EXPECT_TRUE(false) << "Encode total count hint after Series is initialized - exception expected";
}

TEST(SeriesTests, testSeriesAddSummaryDataAfterInitialized)
{
	try
	{
		FieldList summaryData;
		summaryData.addUInt(1, 3056).complete();

		Series series;
		series.add().summaryData(summaryData).complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Encode summary data after Series is initialized - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Invalid attempt to call summaryData() when container is initialized.", exp.getText().c_str());
		return;
	}

	EXPECT_TRUE(false) << "Encode summary data after Series is initialized - exception expected";
}

TEST(SeriesTests, testSeriesAddMismatchEntryDataType_Encode)
{
	try
	{
		FieldList fieldList;
		fieldList.addUInt(1, 3056).complete();

		Series series;
		series.totalCountHint(2)
			.add(fieldList)
			.add()
			.complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Fails to encode Series with mistmatch entry type - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Attempt to add an entry with a different DataType. Encode DataType as NoData while the expected DataType is FieldList", exp.getText().c_str());
	}
}
TEST(SeriesTests, testSeriesAddEntryAfterCallingComplete_Encode)
{
	try
	{
		FieldList fieldList;
		fieldList.addUInt(1, 3056).complete();

		Series series;
		series.totalCountHint(1).complete();
		series.add(fieldList);
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Fails to encode Series after the complete() is called - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Attempt to add an entry after complete() was called.", exp.getText().c_str());
	}
}

TEST(SeriesTests, testSeriesClear_Encode_Decode)
{
	try
	{
		FieldList fieldList;
		fieldList.addUInt(1, 3056).complete();

		Series series;
		series.totalCountHint(1)
			.add(fieldList)
			.clear()
			.add()
			.complete();

		StaticDecoder::setData(&series, NULL);

		EXPECT_FALSE(series.hasTotalCountHint()) << "Check has total count hint attribute";

		EXPECT_TRUE(series.forth()) << "Get the first Series entry";
		EXPECT_TRUE(series.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first entry";

		EXPECT_FALSE(series.forth()) << "Check to make sure that there is no more enty in Series";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode and decode Series - exception not expected with text" << exp.getText().c_str();
	}
}

TEST(SeriesTests, testSeriesWithSummaryDataButNoEntry_Encode_Decode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{

		FieldList summaryData;
		summaryData.addUInt(1, 3056).addEnum(15, 840).addDate(3386, 2018, 2, 28).complete();

		Series series;
		series.totalCountHint(0).summaryData(summaryData).complete();

		ElementList elementList;

		elementList.info(1);

		elementList.addSeries("1", series).complete();

		StaticDecoder::setData(&elementList, &dictionary);

		EXPECT_TRUE(elementList.forth());

		EXPECT_TRUE(elementList.getEntry().getSeries().getTotalCountHint() == 0) << "Check key total count hint from Series";

		const FieldList& decodeFieldList = elementList.getEntry().getSeries().getSummaryData().getFieldList();
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


		EXPECT_FALSE(elementList.getEntry().getSeries().forth()) << "Check whether this is an entry from Series";

		EXPECT_FALSE(elementList.forth()) << "Check whether this is an entry from ElementList";
	}
	catch (const OmmException& exp)
	{
		EXPECT_TRUE(false) << "Fails to encode summary data but no entry - exception not expected with text : " << exp.getText().c_str();
		return;
	}
}

TEST(SeriesTests, testSeriesAddNotCompletedContainer)
{
	try
	{
		Series series;
		ElementList elementList;

		series.add(elementList);
		series.complete();

		EXPECT_FALSE(true) << "Series complete while ElementList is not completed  - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Series complete while ElementList is not completed  - exception expected";
	}

	try
	{
		Series series;
		ElementList elementList;
		series.add(elementList);
		elementList.addUInt("test", 64);
		series.complete();

		EXPECT_FALSE(true) << "Series complete while ElementList with data is not completed  - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Series complete while ElementList with data is not completed  - exception expected";
	}

	try
	{
		Series series;
		ElementList elementList, elementList1;
		series.add(elementList);
		series.add(elementList1);

		EXPECT_FALSE(true) << "Series add two not completed ElementLists - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Series add two not completed ElementLists - exception expected";
	}

	try
	{
		Series series;
		ElementList elementList, elementList1;
		series.add(elementList);
		elementList.complete();
		series.add(elementList1);
		series.complete();

		EXPECT_FALSE(true) << "Series add first completed and second not completed ElementLists - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Series add first completed and second not completed ElementLists - exception expected";
	}

	try
	{
		Series series;
		GenericMsg genericMsg;

		genericMsg.streamId(1);

		series.add(genericMsg);
		series.complete();

		EXPECT_TRUE(true) << "Series add not completed GenericMsg - exception not expected";
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "Series add not completed GenericMsg - exception not expected";
	}

	try
	{
		Series series;
		OmmOpaque opaque;

		char* string = const_cast<char*>("OPQRST");
		EmaBuffer buffer(string, 6);
		opaque.set(buffer);

		series.add(opaque);
		series.complete();

		EXPECT_TRUE(true) << "Series add OmmOpaque - exception not expected";
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "Series add OmmOpaque - exception not expected";
	}

	try
	{
		Series series;
		ElementList elementList;
		OmmOpaque opaque;

		char* string = const_cast<char*>("OPQRST");
		EmaBuffer buffer(string, 6);
		opaque.set(buffer);

		series.add(opaque);
		series.add(elementList);
		series.complete();

		EXPECT_FALSE(true) << "Series add not completed ElementList after Opaque - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Series add not completed ElementList after Opaque - exception expected";
	}

	try
	{
		Series series;;
		ElementList elementList;
		GenericMsg genericMsg;

		series.add(genericMsg);
		series.add(elementList);
		series.complete();

		EXPECT_FALSE(true) << "Series add not completed ElementList after GenericMsg - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Series add not completed ElementList after GenericMsg - exception expected";
	}
}
