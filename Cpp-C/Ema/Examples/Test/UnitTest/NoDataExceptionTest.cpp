/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "EmaUnitTestConnect.h"

#include "Access/Impl/NoDataImpl.h"
#include "Access/Impl/OmmIntDecoder.h"
#include "Access/Impl/OmmUIntDecoder.h"
#include "Access/Impl/OmmXmlDecoder.h"
#include "Access/Impl/OmmJsonDecoder.h"
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
#include "Access/Impl/AckMsgImpl.h"
#include "Access/Impl/GenericMsgImpl.h"
#include "Access/Impl/PostMsgImpl.h"
#include "Access/Impl/ReqMsgImpl.h"
#include "Access/Impl/RefreshMsgImpl.h"
#include "Access/Impl/StatusMsgImpl.h"
#include "Access/Impl/UpdateMsgImpl.h"

using namespace refinitiv::ema::access;
using namespace std;

static void testNoEncoder(const Data& data)
{
	try
	{
		EXPECT_FALSE(EmaUnitTestConnect::hasEncoder(data));
		EXPECT_THROW(EmaUnitTestConnect::getEncoder(data), OmmInvalidUsageException);
		EXPECT_FALSE(EmaUnitTestConnect::hasEncoder(data));
	}
	catch (const OmmException& excp)
	{
		GTEST_FAIL() << "unexpected OmmException";
		cout << excp << endl;
	}
	catch (...)
	{
		GTEST_FAIL() << "unexpected exception";
	}
}

static void testHasEncoder(const Data& data)
{
	try
	{
		EXPECT_FALSE(EmaUnitTestConnect::hasEncoder(data));
		EXPECT_NO_THROW(EmaUnitTestConnect::getEncoder(data));
		EXPECT_TRUE(EmaUnitTestConnect::hasEncoder(data));
	}
	catch (const OmmException& excp)
	{
		GTEST_FAIL() << "unexpected OmmException";
		cout << excp << endl;
	}
	catch (...)
	{
		GTEST_FAIL() << "unexpected exception";
	}
}

static void testHasSetEncoder(Data& data)
{
	try
	{
		EXPECT_FALSE(EmaUnitTestConnect::hasEncoder(data));
		EXPECT_THROW(EmaUnitTestConnect::getEncoder(data), OmmInvalidUsageException);
		EXPECT_FALSE(EmaUnitTestConnect::hasEncoder(data));
		switch (data.getDataType())
		{
			case DataType::AnsiPageEnum:
				EXPECT_NO_THROW((static_cast<OmmAnsiPage&>(data).set(EmaBuffer{ "no data", 7 })));
				break;
			case DataType::JsonEnum:
				EXPECT_NO_THROW((static_cast<OmmJson&>(data).set(EmaBuffer{ "{}", 2 })));
				break;
			case DataType::XmlEnum:
				EXPECT_NO_THROW((static_cast<OmmXml&>(data).set(EmaBuffer{ "<data/>", 7 })));
				break;
			default:
				GTEST_FAIL() << "unexpected data type [OmmAnsiPage, OmmJson, OmmXml]";
				break;
		}
		EXPECT_TRUE(EmaUnitTestConnect::hasEncoder(data));
		EXPECT_NO_THROW(EmaUnitTestConnect::getEncoder(data));
	}
	catch (const OmmException& excp)
	{
		GTEST_FAIL() << "unexpected OmmException";
		cout << excp << endl;
	}
	catch (...)
	{
		GTEST_FAIL() << "unexpected exception";
	}
}

TEST(NoDataExceptionTest, testNoDataEncoderException)
{
	testNoEncoder(NoDataImpl{});

	// using StaticDecoder::morph to create other instances with private constructors
	NoDataImpl noDataImplBase;

	StaticDecoder::morph(&noDataImplBase, DataType::AsciiEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::BufferEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::DateEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::DateTimeEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::DoubleEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::EnumEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::ErrorEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::FloatEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::IntEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::QosEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::RealEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::RmtesEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::StateEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::TimeEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::UIntEnum);
	testNoEncoder(noDataImplBase);
	StaticDecoder::morph(&noDataImplBase, DataType::Utf8Enum);
	testNoEncoder(noDataImplBase);

	// need to set local object back to NoDataImpl instance to destruct it properly
	StaticDecoder::morph(&noDataImplBase, DataType::NoDataEnum);
}

TEST(NoDataExceptionTest, testDataEncoderNoException)
{
	testHasEncoder(Vector{});
	testHasEncoder(ElementList{});
	testHasEncoder(FieldList{});
	testHasEncoder(FilterList{});
	testHasEncoder(Map{});
	testHasEncoder(OmmArray{});
	testHasEncoder(OmmOpaque{});
	testHasEncoder(Series{});

	OmmAnsiPage ansiPage;
	OmmJson json;
	OmmXml xml;

	testHasSetEncoder(ansiPage);
	testHasSetEncoder(json);
	testHasSetEncoder(xml);
}
