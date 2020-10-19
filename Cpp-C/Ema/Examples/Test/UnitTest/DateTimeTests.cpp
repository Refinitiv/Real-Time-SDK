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

TEST(DateTimeTests, testDateTimeStringFormat)
{	
	DateTimeStringFormat dtFmt;
	try
	{
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_ISO8601);
		EXPECT_TRUE( dtFmt.getDateTimeStringFormatType() == DateTimeStringFormat::STR_DATETIME_ISO8601 ) << "DateTimeStringFormat Setting - expected STR_DATETIME_ISO8601";
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_RSSL);
		EXPECT_TRUE( dtFmt.getDateTimeStringFormatType() == DateTimeStringFormat::STR_DATETIME_RSSL ) << "DateTimeStringFormat Setting - expected STR_DATETIME_RSSL";
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "DateTimeStringFormat Setting - exception not expected" ;
	}

	try
	{
		dtFmt.dateTimeStringFormatType(static_cast<DateTimeStringFormat::DateTimeStringFormatTypes>(3));
		EXPECT_FALSE( true ) << "DateTimeStringFormat Setting Invalid format- exception expected";
	}
	catch ( const OmmException& exc)
	{
		EXPECT_TRUE( true ) << "DateTimeStringFormat Setting Invalid format - exception "<<exc.getExceptionType() << " expected.";
	}
}

TEST(DateTimeTests, testDateWithDateTimeStringFormat)
{
	// Date 
	char dateTestName[12][24] = {		
		"Blank Date1", "Blank Date2", "Blank Date3", "Blank Date4", "NonBlank Date",
		"Blank Date6", "Blank Date7", "Blank Date8", "Valid Date1", "Valid Date2",
		"Valid Date3", "Valid Date4"
	};
	int iDate[12][3] = {
		{0,0,0}, {0,1,0}, {0,1,2011}, {1,0,0},	{1,1,2011}, 
    	{1,0,2011}, {0,0,2011}, {1,1,0}, {25,5,2010}, {29,9,2017},
    	{20,01,0001}, {30,10,2010}
	};

	char expRsslDate[12][24] = {		
		"(blank data)", "   JAN     ", "   JAN 2011", "01         ", "01 JAN 2011", 
		"01     2011", "       2011", "01 JAN     ", "25 MAY 2010", "29 SEP 2017", 
		"20 JAN    1", "30 OCT 2010"
	};
	char expIso8601Date[12][24] = {		
		"(blank data)", "--01", "2011-01", "--  -01", "2011-01-01", 
		"2011-  -01", "2011", "--01-01", "2010-05-25", "2017-09-29", 
		"0001-01-20", "2010-10-30"
	};

	DateTimeStringFormat dtFmt;
	int i = 0;

	EmaString iStr;
	EmaString oStr;

	ElementList elEnc;
	try
	{
		// Encode Date values in ElementList
		for(i=0; i < 12; ++i)
		{
			elEnc.addDate( EmaString( dateTestName[i]), iDate[i][2], iDate[i][1], iDate[i][0] );
		}
		elEnc.complete();
		
		// Test ISO8601 Date.
		StaticDecoder::setData( &elEnc, 0 );
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_ISO8601);
		elEnc.forth();
		for(i=0; i < 12; ++i)
		{
			const ElementEntry& elEntry = elEnc.getEntry();
			if(elEntry.getLoadType() == DataType::DateEnum)
			{
				if ( elEntry.getCode() != Data::BlankEnum )
				{
					iStr = expIso8601Date[i];
					oStr = dtFmt.dateAsString(const_cast<OmmDate &>(elEntry.getDate()));
					EXPECT_EQ( iStr, oStr ) << dateTestName[i]; 
				}
			}
			elEnc.forth();
		}

		// Test RSSL Date.
		StaticDecoder::setData( &elEnc, 0 );
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_RSSL);
		elEnc.forth();
		cout<<endl;
		for(i=0; i < 12; ++i)
		{
			const ElementEntry& elEntry = elEnc.getEntry();
			if(elEntry.getLoadType() == DataType::DateEnum)
			{
				if ( elEntry.getCode() != Data::BlankEnum )
				{
					iStr = expRsslDate[i];
					oStr = dtFmt.dateAsString(const_cast<OmmDate &>(elEntry.getDate()));
					EXPECT_EQ( iStr, oStr ) << dateTestName[i]; 
				}
			}
			elEnc.forth();
		}

		cout<<endl;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "testDateWithDateTimeStringFormat - exception not expected " <<dateTestName[i] <<"Failed";
	}

}

TEST(DateTimeTests, testTimeWithDateTimeStringFormat)
{
	// Time 
	char timeTestName[23][40] = {		
    		"Time AllZeros", "Time AllBlank", "Time HH NotBlank", "Time HH:MM NotBlank", "Time HH:MM:SS NotBlank",
    		"Time Micro:Nano Blank", "Time Nano Blank", "Time Trail_0 3Digits", "Time Trail_0 2Digits", "Time 1Digit",
    		"Time SecMilMicroNano_0", "Time MilMicroNano_0 SecTrail0", "Time MilMicroNano_0 Sec1Digit", "Time MicroNano_0  Mil3Digit_Trail0", "Time MicroNano_0 Mil2Digit_Trail0",		
    		"Time MicroNano_0 Mil1Digit", "Time Nano_0 Micro3Digit_Trail0", "Time Nano_0 Micro2Digit_Trail0", "Time Nano_0 Micro1Digit", "Time Nano3Digits_Trail0",
    		"Time Nano2Digits_Trail0","Time Nano1Digit", "Time All 1Digit"
	};
	int iTime[23][6] = {
    		{0,0,0,0,0,0}, {255,255,255,65535,2047,2047}, {15,255,255,65535,2047,2047}, {12,30,255,65535,2047,2047},  {12,30,6,65535,2047,2047},
    		{12,30,56,600,2047,2047}, {12,30,56,809,900,2047}, {12,30,56,800,900,200}, {11,20,30,10,90,40}, {11,1,2,1,9,4}, 
    		{15,25,0,0,0,0}, {12,36,40,0,0,0}, {10,20,3,0,0,0}, {12,36,40,200,0,0}, {12,36,40,20,0,0}, 
    		{12,36,40,2,0,0}, {11,22,33,400,500,0}, {11,22,33,400,50,0}, {11,22,33,400,5,0}, {11,22,33,400,5,700}, 
    		{11,22,33,400,5,70}, {11,22,33,400,5,7}, {2,2,2,2,2,2}
	};

	char expRsslTime[23][24] = {		
		"00:00:00:000:000:000", "(blank data)",  "15",  "12:30",  "12:30:06", 
		 "12:30:56:600", "12:30:56:809:900", "12:30:56:800:900:200",  "11:20:30:010:090:040",  "11:01:02:001:009:004", 
		 "15:25:00:000:000:000",  "12:36:40:000:000:000",  "10:20:03:000:000:000",  "12:36:40:200:000:000",  "12:36:40:020:000:000", 
		 "12:36:40:002:000:000",  "11:22:33:400:500:000",  "11:22:33:400:050:000",  "11:22:33:400:005:000",  "11:22:33:400:005:700",
		  "11:22:33:400:005:070",  "11:22:33:400:005:007",  "02:02:02:002:002:002"
	};
	char expIso8601Time[23][24] = {		
		"00:00:00", "(blank data)",  "15",  "12:30",  "12:30:06", 
		"12:30:56.6",  "12:30:56.8099",  "12:30:56.8009002",  "11:20:30.01009004",  "11:01:02.001009004", 
		 "15:25:00",  "12:36:40", "10:20:03",  "12:36:40.2",  "12:36:40.02", 
		 "12:36:40.002",  "11:22:33.4005", "11:22:33.40005",  "11:22:33.400005",  "11:22:33.4000057", 
		 "11:22:33.40000507",  "11:22:33.400005007",  "02:02:02.002002002"
	};

	DateTimeStringFormat dtFmt;
	int i = 0;

	EmaString iStr;
	EmaString oStr;

	ElementList elEnc;
	try
	{
		// Encode Date values in ElementList
		for(i=0; i < 23; ++i)
		{
			elEnc.addTime( EmaString( timeTestName[i]), 
				iTime[i][0], iTime[i][1], iTime[i][2], iTime[i][3], iTime[i][4], iTime[i][5]);
		}
		elEnc.complete();
		
		// Test ISO8601 Time.
		StaticDecoder::setData( &elEnc, 0 );
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_ISO8601);
		elEnc.forth();
		for(i=0; i < 23; ++i)
		{
			const ElementEntry& elEntry = elEnc.getEntry();
			if(elEntry.getLoadType() == DataType::TimeEnum)
			{
				if ( elEntry.getCode() != Data::BlankEnum )
				{
					iStr = expIso8601Time[i];
					oStr = dtFmt.timeAsString(const_cast<OmmTime &>(elEntry.getTime()));
					EXPECT_EQ( iStr, oStr ) << timeTestName[i]; 
				}
			}
			elEnc.forth();
		}
	
		// Test RSSL Date.
		StaticDecoder::setData( &elEnc, 0 );
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_RSSL);
		elEnc.forth();
		for(i=0; i < 23; ++i)
		{
			const ElementEntry& elEntry = elEnc.getEntry();
			if(elEntry.getLoadType() == DataType::DateEnum)
			{
				if ( elEntry.getCode() != Data::BlankEnum )
				{
					iStr = expRsslTime[i];
					oStr = dtFmt.timeAsString(const_cast<OmmTime &>(elEntry.getTime()));
					EXPECT_EQ( iStr, oStr ) << timeTestName[i]; 
				}
			}
			elEnc.forth();
		}

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "testTimeWithDateTimeStringFormat - exception not expected " <<timeTestName[i] <<"Failed";
	}

}

TEST(DateTimeTests, testDateTimeWithDateTimeStringFormat)
{
	// Date 
	char dateTestName[12][24] = {		
		"Blank Date1", "Blank Date2", "Blank Date3", "Blank Date4", "NonBlank Date",
		"Blank Date6", "Blank Date7", "Blank Date8", "Valid Date1", "Valid Date2",
		"Valid Date3", "Valid Date4"
	};
	int iDate[12][3] = {
		{0,0,0}, {0,1,0}, {0,1,2011}, {1,0,0},	{1,1,2011}, 
    	{1,0,2011}, {0,0,2011}, {1,1,0}, {25,5,2010}, {29,9,2017},
    	{20,01,0001}, {30,10,2010}
	};

	char expRsslDate[12][24] = {		
		"(blank data)", "   JAN     ", "   JAN 2011", "01         ", "01 JAN 2011", 
		"01     2011", "       2011", "01 JAN     ", "25 MAY 2010", "29 SEP 2017", 
		"20 JAN    1", "30 OCT 2010"
	};
	char expIso8601Date[12][24] = {		
		"(blank data)", "--01", "2011-01", "--  -01", "2011-01-01", 
		"2011-  -01", "2011", "--01-01", "2010-05-25", "2017-09-29", 
		"0001-01-20", "2010-10-30"
	};

	// Time 
	char timeTestName[23][40] = {		
    		"Time AllZeros", "Time AllBlank", "Time HH NotBlank", "Time HH:MM NotBlank", "Time HH:MM:SS NotBlank",
    		"Time Micro:Nano Blank", "Time Nano Blank", "Time Trail_0 3Digits", "Time Trail_0 2Digits", "Time 1Digit",
    		"Time SecMilMicroNano_0", "Time MilMicroNano_0 SecTrail0", "Time MilMicroNano_0 Sec1Digit", "Time MicroNano_0  Mil3Digit_Trail0", "Time MicroNano_0 Mil2Digit_Trail0",		
    		"Time MicroNano_0 Mil1Digit", "Time Nano_0 Micro3Digit_Trail0", "Time Nano_0 Micro2Digit_Trail0", "Time Nano_0 Micro1Digit", "Time Nano3Digits_Trail0",
    		"Time Nano2Digits_Trail0","Time Nano1Digit", "Time All 1Digit"
	};
	int iTime[23][6] = {
    		{0,0,0,0,0,0}, {255,255,255,65535,2047,2047}, {15,255,255,65535,2047,2047}, {12,30,255,65535,2047,2047},  {12,30,6,65535,2047,2047},
    		{12,30,56,600,2047,2047}, {12,30,56,809,900,2047}, {12,30,56,800,900,200}, {11,20,30,10,90,40}, {11,1,2,1,9,4}, 
    		{15,25,0,0,0,0}, {12,36,40,0,0,0}, {10,20,3,0,0,0}, {12,36,40,200,0,0}, {12,36,40,20,0,0}, 
    		{12,36,40,2,0,0}, {11,22,33,400,500,0}, {11,22,33,400,50,0}, {11,22,33,400,5,0}, {11,22,33,400,5,700}, 
    		{11,22,33,400,5,70}, {11,22,33,400,5,7}, {2,2,2,2,2,2}
	};

	char expRsslTime[23][24] = {		
		"00:00:00:000:000:000", "(blank data)",  "15",  "12:30",  "12:30:06", 
		 "12:30:56:600", "12:30:56:809:900", "12:30:56:800:900:200",  "11:20:30:010:090:040",  "11:01:02:001:009:004", 
		 "15:25:00:000:000:000",  "12:36:40:000:000:000",  "10:20:03:000:000:000",  "12:36:40:200:000:000",  "12:36:40:020:000:000", 
		 "12:36:40:002:000:000",  "11:22:33:400:500:000",  "11:22:33:400:050:000",  "11:22:33:400:005:000",  "11:22:33:400:005:700",
		  "11:22:33:400:005:070",  "11:22:33:400:005:007",  "02:02:02:002:002:002"
	};
	char expIso8601Time[23][24] = {		
		"00:00:00", "(blank data)",  "15",  "12:30",  "12:30:06", 
		"12:30:56.6",  "12:30:56.8099",  "12:30:56.8009002",  "11:20:30.01009004",  "11:01:02.001009004", 
		 "15:25:00",  "12:36:40", "10:20:03",  "12:36:40.2",  "12:36:40.02", 
		 "12:36:40.002",  "11:22:33.4005", "11:22:33.40005",  "11:22:33.400005",  "11:22:33.4000057", 
		 "11:22:33.40000507",  "11:22:33.400005007",  "02:02:02.002002002"
	};

	DateTimeStringFormat dtFmt;
	int i = 0;
	int j = 0;
	EmaString iStr;
	EmaString oStr;
	EmaString tName;
	ElementList elEnc;
	try
	{
		// Encode Date values in ElementList
		for(i = 0; i < 12; ++i)
		{			
			for(j = 0; j < 23; ++j)
			{
				tName = dateTestName[i];
				tName.append("_");
				tName.append(timeTestName[j]);
				elEnc.addDateTime( tName, 
					iDate[i][2], iDate[i][1], iDate[i][0],
					iTime[j][0], iTime[j][1], iTime[j][2], iTime[j][3], iTime[j][4], iTime[j][5]);
			}
		}
		elEnc.complete();
		
		// Test ISO8601 Time.
		StaticDecoder::setData( &elEnc, 0 );
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_ISO8601);
		elEnc.forth();
		for(i = 0; i < 12; ++i)
		{			
			for(j=0; j < 23; ++j)
			{
				const ElementEntry& elEntry = elEnc.getEntry();
				if(elEntry.getLoadType() == DataType::DateTimeEnum)
				{
					if ( elEntry.getCode() != Data::BlankEnum )
					{
						if( j == 1)
							iStr = expIso8601Date[i];
						else
						{
							iStr = (i > 0) ? expIso8601Date[i] : "";
							iStr.append("T");
							iStr.append(expIso8601Time[j]);
						}
						oStr = dtFmt.dateTimeAsString(const_cast<OmmDateTime &>(elEntry.getDateTime()));
						EXPECT_EQ( iStr, oStr ) << elEntry.getName(); 
					}
				}
				elEnc.forth();
			}
		}
	
		// Test RSSL Date.
		StaticDecoder::setData( &elEnc, 0 );
		dtFmt.dateTimeStringFormatType(DateTimeStringFormat::STR_DATETIME_RSSL);
		elEnc.forth();
		for(i = 0; i < 12; ++i)
		{			
			for(j=0; j < 23; ++j)
			{
				const ElementEntry& elEntry = elEnc.getEntry();
				if(elEntry.getLoadType() == DataType::DateTimeEnum)
				{
					if ( elEntry.getCode() != Data::BlankEnum )
					{
						iStr = (i > 0) ? expRsslDate[i] : "";
						iStr.append(" ");
						iStr += (j != 1) ? expRsslTime[j] : "";
						oStr = dtFmt.dateTimeAsString(const_cast<OmmDateTime &>(elEntry.getDateTime()));
						EXPECT_EQ( iStr, oStr ) << elEntry.getName(); 
					}
				}
				elEnc.forth();
			}
		}
		cout<<endl;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "testDateWithDateTimeStringFormat - exception not expected " <<timeTestName[i] <<"Failed";
	}

}
