/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

#include "Access/Impl/NoDataImpl.h"
#include "Access/Impl/OmmIntDecoder.h"
#include "Access/Impl/OmmUIntDecoder.h"
#include "Access/Impl/OmmXmlDecoder.h"
#include "Access/Impl/OmmAnsiPageDecoder.h"
#include "Access/Impl/OmmAsciiDecoder.h"
#include "Access/Impl/OmmDateDecoder.h"
#include "Access/Impl/OmmTimeDecoder.h"
#include "Access/Impl/OmmDateTimeDecoder.h"
#include "Access/Impl/OmmBufferDecoder.h"
#include "Access/Impl/OmmFloatDecoder.h"
#include "Access/Impl/OmmStateDecoder.h"
#include "Access/Impl/OmmQosDecoder.h"
#include "Access/Impl/OmmOpaqueDecoder.h"
#include "Access/Impl/OmmUtf8Decoder.h"
#include "Access/Impl/OmmRmtesDecoder.h"
#include "Access/Impl/OmmEnumDecoder.h"
#include "Access/Impl/OmmErrorDecoder.h"
#include "Access/Impl/OmmDoubleDecoder.h"
#include "Access/Impl/OmmRealDecoder.h"
#include "Access/Impl/OmmArrayDecoder.h"
#include "Access/Impl/ElementListDecoder.h"
#include "Access/Impl/FieldListDecoder.h"
#include "Access/Impl/FilterListDecoder.h"
#include "Access/Impl/MapDecoder.h"
#include "Access/Impl/SeriesDecoder.h"
#include "Access/Impl/VectorDecoder.h"
#include "Access/Impl/AckMsgDecoder.h"
#include "Access/Impl/GenericMsgDecoder.h"
#include "Access/Impl/PostMsgDecoder.h"
#include "Access/Impl/ReqMsgDecoder.h"
#include "Access/Impl/RefreshMsgDecoder.h"
#include "Access/Impl/StatusMsgDecoder.h"
#include "Access/Impl/UpdateMsgDecoder.h"

using namespace thomsonreuters::ema::access;
using namespace std;

TEST(NoDataSizeTest, testDataVsDecoderSize)
{
	try
	{
		size_t dataSize = sizeof( Data );
		size_t voidPtrSize = sizeof( void* );

		size_t integerSize = sizeof( OmmInt );
		size_t uIntegerSize = sizeof( OmmUInt );
		size_t asciiSize = sizeof( OmmAscii );
		size_t bufferSize = sizeof( OmmBuffer );
		size_t floatSize = sizeof( OmmFloat );
		size_t doubleSize = sizeof( OmmDouble );
		size_t realSize = sizeof( OmmReal );
		size_t dateSize = sizeof( OmmDate );
		size_t timeSize = sizeof( OmmTime );
		size_t dateTimeSize = sizeof( OmmDateTime );
		size_t stateSize = sizeof( OmmState );
		size_t qosSize = sizeof( OmmQos );
		size_t ansiPageSize = sizeof( OmmAnsiPage );
		size_t enumSize = sizeof( OmmEnum );
		size_t opaqueSize = sizeof( OmmOpaque );
		size_t rmtesSize = sizeof( OmmRmtes );
		size_t utf8Size = sizeof( OmmUtf8 );
		size_t xmlSize = sizeof( OmmXml );

		size_t errorSize = sizeof( OmmError );

		size_t integerDecoderSize = sizeof( OmmIntDecoder );
		size_t uIntegerDecoderSize = sizeof( OmmUIntDecoder );
		size_t asciiDecoderSize = sizeof( OmmAsciiDecoder );
		size_t bufferDecoderSize = sizeof( OmmBufferDecoder );
		size_t floatDecoderSize = sizeof( OmmFloatDecoder );
		size_t doubleDecoderSize = sizeof( OmmDoubleDecoder );
		size_t realDecoderSize = sizeof( OmmRealDecoder );
		size_t dateDecoderSize = sizeof( OmmDateDecoder );
		size_t timeDecoderSize = sizeof( OmmTimeDecoder );
		size_t dateTimeDecoderSize = sizeof( OmmDateTimeDecoder );
		size_t stateDecoderSize = sizeof( OmmStateDecoder );
		size_t qosDecoderSize = sizeof( OmmQosDecoder );
		size_t ansiPageDecoderSize = sizeof( OmmAnsiPageDecoder );
		size_t enumDecoderSize = sizeof( OmmEnumDecoder );
		size_t opaqueDecoderSize = sizeof( OmmOpaqueDecoder );
		size_t rmtesDecoderSize = sizeof( OmmRmtesDecoder );
		size_t utf8DecoderSize = sizeof( OmmUtf8Decoder );
		size_t xmlDecoderSize = sizeof( OmmXmlDecoder );

		size_t errorDecoderSize = sizeof( OmmErrorDecoder );

		EXPECT_TRUE( integerSize >= integerDecoderSize + dataSize + voidPtrSize ) << "OmmInt >= OmmIntDecoder" ;
		EXPECT_TRUE( uIntegerSize >= uIntegerDecoderSize + dataSize + voidPtrSize ) << "OmmUInt >= OmmUIntDecoder" ;
		EXPECT_TRUE( asciiSize >= asciiDecoderSize + dataSize + voidPtrSize ) << "OmmAscii >= OmmAsciiDecoder" ;
		EXPECT_TRUE( bufferSize >= bufferDecoderSize + dataSize + voidPtrSize ) << "OmmBuffer >= OmmBufferDecoder" ;
		EXPECT_TRUE( floatSize >= floatDecoderSize + dataSize + voidPtrSize ) << "OmmFloat >= OmmFloatDecoder" ;
		EXPECT_TRUE( doubleSize >= doubleDecoderSize + dataSize + voidPtrSize ) << "OmmDouble >= OmmDoubleDecoder" ;
		EXPECT_TRUE( realSize >= realDecoderSize + dataSize + voidPtrSize ) << "OmmReal >= OmmRealDecoder" ;
		EXPECT_TRUE( dateSize >= dateDecoderSize + dataSize + voidPtrSize ) << "OmmDate >= OmmDateDecoder" ;
		EXPECT_TRUE( timeSize >= timeDecoderSize + dataSize + voidPtrSize ) << "OmmTime >= OmmTimeDecoder" ;
		EXPECT_TRUE( dateTimeSize >= dateTimeDecoderSize + dataSize + voidPtrSize ) << "OmmDateTime >= OmmDateTimeDecoder" ;
		EXPECT_TRUE( stateSize >= stateDecoderSize + dataSize + voidPtrSize ) << "OmmState >= OmmStateDecoder" ;
		EXPECT_TRUE( qosSize >= qosDecoderSize + dataSize + voidPtrSize ) << "OmmQos >= OmmQosDecoder" ;
		EXPECT_TRUE( ansiPageSize >= ansiPageDecoderSize + voidPtrSize + voidPtrSize + sizeof( EmaString ) ) << "OmmAnsiPage >= OmmAnsiPageDecoder" ;
		EXPECT_TRUE( enumSize >= enumDecoderSize + dataSize + voidPtrSize ) << "OmmEnum >= OmmEnumDecoder" ;
		EXPECT_TRUE( opaqueSize >= opaqueDecoderSize + dataSize + voidPtrSize + voidPtrSize + sizeof( EmaString ) ) << "OmmOpaque >= OmmOpaqueDecoder" ;
		EXPECT_TRUE( rmtesSize >= rmtesDecoderSize + dataSize + voidPtrSize ) << "OmmRmtes >= OmmRmtesDecoder" ;
		EXPECT_TRUE( utf8Size >= utf8DecoderSize + dataSize + voidPtrSize ) << "OmmUtf8 >= OmmUtf8Decoder" ;
		EXPECT_TRUE( xmlSize >= xmlDecoderSize + dataSize + voidPtrSize + voidPtrSize + sizeof( EmaString ) ) << "OmmXml >= OmmXmlDecoder" ;

		EXPECT_TRUE( errorSize >= errorDecoderSize + dataSize + voidPtrSize + sizeof( EmaString ) ) << "OmmError >= OmmErrorDecoder" ;

		EXPECT_TRUE( true ) << "Data vs Decoder Size - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "Data vs Decoder Size - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(NoDataSizeTest, testNoDataSize)
{
	try
	{
		size_t noDataSize = sizeof( NoDataImpl );

		size_t integerSize = sizeof( OmmInt );
		size_t uIntegerSize = sizeof( OmmUInt );
		size_t asciiSize = sizeof( OmmAscii );
		size_t bufferSize = sizeof( OmmBuffer );
		size_t floatSize = sizeof( OmmFloat );
		size_t doubleSize = sizeof( OmmDouble );
		size_t realSize = sizeof( OmmReal );
		size_t dateSize = sizeof( OmmDate );
		size_t timeSize = sizeof( OmmTime );
		size_t dateTimeSize = sizeof( OmmDateTime );
		size_t stateSize = sizeof( OmmState );
		size_t qosSize = sizeof( OmmQos );
		size_t ansiPageSize = sizeof( OmmAnsiPage );
		size_t enumSize = sizeof( OmmEnum );
		size_t opaqueSize = sizeof( OmmOpaque );
		size_t rmtesSize = sizeof( OmmRmtes );
		size_t utf8Size = sizeof( OmmUtf8 );
		size_t xmlSize = sizeof( OmmXml );

		size_t arraySize = sizeof( OmmArray );
		size_t fieldListSize = sizeof( FieldList );
		size_t mapSize = sizeof( Map );
		size_t vectorSize = sizeof( Vector );
		size_t seriesSize = sizeof( Series );
		size_t filterListSize = sizeof( FilterList );

		size_t errorSize = sizeof( OmmError );

		size_t ackMsgSize = sizeof( AckMsg );
		size_t genMsgSize = sizeof( GenericMsg );
		size_t postMsgSize = sizeof( PostMsg );
		size_t reqMsgSize = sizeof( ReqMsg );
		size_t refreshMsgSize = sizeof( RefreshMsg );
		size_t updateMsgSize = sizeof( UpdateMsg );
		size_t statusMsgSize = sizeof( StatusMsg );

		EXPECT_TRUE(noDataSize >= integerSize) << "NoData >= OmmInt" ;
		EXPECT_TRUE(noDataSize >= uIntegerSize) << "NoData >= OmmUInt" ;
		EXPECT_TRUE(noDataSize >= asciiSize) << "NoData >= OmmAscii" ;
		EXPECT_TRUE(noDataSize >= bufferSize) << "NoData >= OmmBuffer" ;
		EXPECT_TRUE(noDataSize >= floatSize) << "NoData >= OmmFloat" ;
		EXPECT_TRUE(noDataSize >= doubleSize) << "NoData >= OmmDouble" ;
		EXPECT_TRUE(noDataSize >= realSize) << "NoData >= OmmReal" ;
		EXPECT_TRUE(noDataSize >= dateSize) << "NoData >= OmmDate" ;
		EXPECT_TRUE(noDataSize >= timeSize) << "NoData >= OmmTime" ;
		EXPECT_TRUE(noDataSize >= dateTimeSize) << "NoData >= OmmDateTime" ;
		EXPECT_TRUE(noDataSize >= stateSize) << "NoData >= OmmState" ;
		EXPECT_TRUE(noDataSize >= qosSize) << "NoData >= OmmQos" ;
		EXPECT_TRUE(noDataSize >= ansiPageSize) << "NoData >= OmmAnsiPage" ;
		EXPECT_TRUE(noDataSize >= enumSize) << "NoData >= OmmEnum" ;
		EXPECT_TRUE(noDataSize >= opaqueSize) << "NoData >= OmmOpaque" ;
		EXPECT_TRUE(noDataSize >= rmtesSize) << "NoData >= OmmRmtes" ;
		EXPECT_TRUE( noDataSize >= utf8Size ) << "NoData >= OmmUtf8" ;
		EXPECT_TRUE(noDataSize >= xmlSize) << "NoData >= OmmXml" ;

		EXPECT_TRUE(noDataSize >= arraySize) << "NoData >= OmmArray" ;
		EXPECT_TRUE(noDataSize >= fieldListSize) << "NoData >= FieldList" ;
		EXPECT_TRUE(noDataSize >= mapSize) << "NoData >= Map" ;
		EXPECT_TRUE(noDataSize >= vectorSize) << "NoData >= Vector" ;
		EXPECT_TRUE(noDataSize >= seriesSize) << "NoData >= Series" ;
		EXPECT_TRUE(noDataSize >= filterListSize) << "NoData >= FilterList" ;

		EXPECT_TRUE(noDataSize >= ackMsgSize) << "NoData >= AckMsg" ;
		EXPECT_TRUE(noDataSize >= genMsgSize) << "NoData >= GenericMsg" ;
		EXPECT_TRUE(noDataSize >= postMsgSize) << "NoData >= PostMsg" ;
		EXPECT_TRUE(noDataSize >= reqMsgSize) << "NoData >= ReqMsg" ;
		EXPECT_TRUE(noDataSize >= refreshMsgSize) << "NoData >= RefreshMsg" ;
		EXPECT_TRUE(noDataSize >= updateMsgSize) << "NoData >= UpdateMsg" ;
		EXPECT_TRUE(noDataSize >= statusMsgSize) << "NoData >= StatusMsg" ;

		EXPECT_TRUE(noDataSize >= errorSize) << "NoData >= OmmError" ;

		EXPECT_TRUE( true ) << "NoData Size - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "NoData Size - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(NoDataSizeTest, testNoDataDecoderSize)
{
	try
	{
		size_t noDataSize = 2160;

		size_t integerSize = sizeof( OmmIntDecoder );
		size_t uIntegerSize = sizeof( OmmUIntDecoder );
		size_t asciiSize = sizeof( OmmAsciiDecoder );
		size_t bufferSize = sizeof( OmmBufferDecoder );
		size_t floatSize = sizeof( OmmFloatDecoder );
		size_t doubleSize = sizeof( OmmDoubleDecoder );
		size_t realSize = sizeof( OmmRealDecoder );
		size_t dateSize = sizeof( OmmDateDecoder );
		size_t timeSize = sizeof( OmmTimeDecoder );
		size_t dateTimeSize = sizeof( OmmDateTimeDecoder );
		size_t stateSize = sizeof( OmmStateDecoder );
		size_t qosSize = sizeof( OmmQosDecoder );
		size_t ansiPageSize = sizeof( OmmAnsiPageDecoder );
		size_t enumSize = sizeof( OmmEnumDecoder );
		size_t opaqueSize = sizeof( OmmOpaqueDecoder );
		size_t rmtesSize = sizeof( OmmRmtesDecoder );
		size_t utf8Size = sizeof( OmmUtf8Decoder );
		size_t xmlSize = sizeof( OmmXmlDecoder );

		size_t arraySize = sizeof( OmmArrayDecoder );
		size_t fieldListSize = sizeof( FieldListDecoder );
		size_t mapSize = sizeof( MapDecoder );
		size_t vectorSize = sizeof( VectorDecoder );
		size_t seriesSize = sizeof( SeriesDecoder );
		size_t filterListSize = sizeof( FilterListDecoder );

		size_t errorSize = sizeof( OmmErrorDecoder );

		size_t ackMsgSize = sizeof( AckMsgDecoder );
		size_t genMsgSize = sizeof( GenericMsgDecoder );
		size_t postMsgSize = sizeof( PostMsgDecoder );
		size_t reqMsgSize = sizeof( ReqMsgDecoder );
		size_t refreshMsgSize = sizeof( RefreshMsgDecoder );
		size_t updateMsgSize = sizeof( UpdateMsgDecoder );
		size_t statusMsgSize = sizeof( StatusMsgDecoder );

		EXPECT_TRUE(noDataSize >= integerSize) << "2160 >= OmmIntDecoder" ;
		EXPECT_TRUE(noDataSize >= uIntegerSize) << "2160 >= OmmUIntDecoder" ;
		EXPECT_TRUE(noDataSize >= asciiSize) << "2160 >= OmmAsciiDecoder" ;
		EXPECT_TRUE(noDataSize >= bufferSize) << "2160 >= OmmBufferDecoder" ;
		EXPECT_TRUE(noDataSize >= floatSize) << "2160 >= OmmFloatDecoder" ;
		EXPECT_TRUE(noDataSize >= doubleSize) << "2160 >= OmmDoubleDecoder" ;
		EXPECT_TRUE(noDataSize >= realSize) << "2160 >= OmmRealDecoder" ;
		EXPECT_TRUE(noDataSize >= dateSize) << "2160 >= OmmDateDecoder" ;
		EXPECT_TRUE(noDataSize >= timeSize) << "2160 >= OmmTimeDecoder" ;
		EXPECT_TRUE(noDataSize >= dateTimeSize) << "2160 >= OmmDateTimeDecoder" ;
		EXPECT_TRUE(noDataSize >= stateSize) << "2160 >= OmmStateDecoder" ;
		EXPECT_TRUE(noDataSize >= qosSize) << "2160 >= OmmQosDecoder" ;
		EXPECT_TRUE(noDataSize >= ansiPageSize) << "2160 >= OmmAnsiPageDecoder" ;
		EXPECT_TRUE(noDataSize >= enumSize) << "2160 >= OmmEnumDecoder" ;
		EXPECT_TRUE(noDataSize >= opaqueSize) << "2160 >= OmmOpaqueDecoder" ;
		EXPECT_TRUE(noDataSize >= rmtesSize) << "2160 >= OmmRmtesDecoder" ;
		EXPECT_TRUE( noDataSize >= utf8Size ) << "2160 >= OmmUtf8Decoder" ;
		EXPECT_TRUE(noDataSize >= xmlSize) << "2160 >= OmmXmlDecoder" ;

		EXPECT_TRUE(noDataSize >= arraySize) << "2160 >= ArrayDecoder" ;
		EXPECT_TRUE(noDataSize >= fieldListSize) << "2160 >= FieldListDecoder" ;
		EXPECT_TRUE(noDataSize >= mapSize) << "2160 >= MapDecoder" ;
		EXPECT_TRUE(noDataSize >= vectorSize) << "2160 >= VectorDecoder" ;
		EXPECT_TRUE(noDataSize >= seriesSize) << "2160 >= SeriesDecoder" ;
		EXPECT_TRUE(noDataSize >= filterListSize) << "2160 >= FilterListDecoder" ;

		EXPECT_TRUE(noDataSize >= ackMsgSize) << "2160 >= AckMsgDecoder" ;
		EXPECT_TRUE(noDataSize >= genMsgSize) << "2160 >= GenericMsgDecoder" ;
		EXPECT_TRUE(noDataSize >= postMsgSize) << "2160 >= PostMsgDecoder" ;
		EXPECT_TRUE(noDataSize >= reqMsgSize) << "2160 >= ReqMsgDecoder" ;
		EXPECT_TRUE(noDataSize >= refreshMsgSize) << "2160 >= RefreshMsgDecoder" ;
		EXPECT_TRUE(noDataSize >= updateMsgSize) << "2160 >= UpdateMsgDecoder" ;
		EXPECT_TRUE(noDataSize >= statusMsgSize) << "2160 >= StatusMsgDecoder" ;

		EXPECT_TRUE(noDataSize >= errorSize) << "2160 >= OmmErrorDecoder" ;

		EXPECT_TRUE( true ) << "NoData Size - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "NoData Size - exception not expected" ;
		cout << excp << endl;
	}
}
