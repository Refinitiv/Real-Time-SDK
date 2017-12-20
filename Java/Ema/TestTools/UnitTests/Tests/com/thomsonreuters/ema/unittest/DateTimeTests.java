///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------


package com.thomsonreuters.ema.unittest;

import com.thomsonreuters.ema.access.DateTimeStringFormat;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmDate;
import com.thomsonreuters.ema.access.OmmDateTime;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmTime;


import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.DateTimeStringFormat.DateTimeStringFormatTypes;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.ElementEntry;

import junit.framework.TestCase;

public class DateTimeTests extends TestCase {
	
	DateTimeStringFormat _dtStringFmt = EmaFactory.createDateTimeStringFormat();
	ElementList _elementList = EmaFactory.createElementList();
	ElementList _decodeElementList = JUnitTestConnect.createElementList();
	String _oString = "";
	String _ename = "Valid Date: ";
	int _iDate[][] = {{0,0,0}, {0,1,0}, {0,1,2011}, {1,0,0},	{1,1,2011}, 
    		{1,0,2011}, {0,0,2011}, {1,1,0}, {25,5,2010}, {29,9,2017},
    		{20,01,0001}, {30,10,2010}};
	
	String[] _dateTestName = {
    		"Blank Date1", "Blank Date2", "Blank Date3", "Blank Date4", "NonBlank Date",
    		"Blank Date6", "Blank Date7", "Blank Date8", "Valid Date1", "Valid Date2",
    		"Valid Date3", "Valid Date4"	
     };
	
	String[] _expIso8601Date = {
        	"(blank data)", "--01", "2011-01", "-- --01", "2011-01-01", 
        	"2011- --01", "2011", "--01-01", "2010-05-25", "2017-09-29", 
       		"0001-01-20", "2010-10-30"
       	};
	String[] _expRsslDate = {
			"(blank data)", "JAN     ", "JAN 2011", "01         ", "01 JAN 2011",
			"01     2011", "    2011", "01 JAN     ", "25 MAY 2010", "29 SEP 2017",
			"20 JAN    1", "30 OCT 2010"	
	};
	
    int _iTime[][] = {
    		{0,0,0,0,0,0}, {255,255,255,65535,2047,2047}, {15,255,255,65535,2047,2047}, {12,30,255,65535,2047,2047},  {12,30,6,65535,2047,2047},
    		{12,30,56,600,2047,2047}, {12,30,56,809,900,2047}, {12,30,56,800,900,200}, {11,20,30,10,90,40}, {11,1,2,1,9,4}, 
    		{15,25,0,0,0,0}, {12,36,40,0,0,0}, {10,20,3,0,0,0}, {12,36,40,200,0,0}, {12,36,40,20,0,0}, 
    		{12,36,40,2,0,0}, {11,22,33,400,500,0}, {11,22,33,400,50,0}, {11,22,33,400,5,0}, {11,22,33,400,5,700}, 
    		{11,22,33,400,5,70}, {11,22,33,400,5,7}, {2,2,2,2,2,2}
    	};
	
    String[] _timeTestName = {
    		"Time AllZeros", "Time AllBlank", "Time HH NotBlank", "Time HH:MM NotBlank", "Time HH:MM:SS NotBlank",
    		"Time Micro:Nano Blank", "Time Nano Blank", "Time Trail_0 3Digits", "Time Trail_0 2Digits", "Time 1Digit",
    		"Time SecMilMicroNano_0", "Time MilMicroNano_0 SecTrail0", "Time MilMicroNano_0 Sec1Digit", "Time MicroNano_0  Mil3Digit_Trail0", "Time MicroNano_0 Mil2Digit_Trail0",		
    		"Time MicroNano_0 Mil1Digit", "Time Nano_0 Micro3Digit_Trail0", "Time Nano_0 Micro2Digit_Trail0", "Time Nano_0 Micro1Digit", "Time Nano3Digits_Trail0",
    		"Time Nano2Digits_Trail0","Time Nano1Digit", "Time All 1Digit"
    };

    String[] _expIso8601Time = {
    		"00:00:00", "(blank data)", "15", "12:30", "12:30:06", 
    		"12:30:56.6", "12:30:56.8099", "12:30:56.8009002", "11:20:30.01009004", "11:01:02.001009004", 
    		"15:25:00", "12:36:40", "10:20:03", "12:36:40.2", "12:36:40.02", 
    		"12:36:40.002", "11:22:33.4005", "11:22:33.40005", "11:22:33.400005", "11:22:33.4000057", 
    		"11:22:33.40000507", "11:22:33.400005007", "02:02:02.002002002"
    };   
    
    String[] _expRsslTime = {
    		"00:00:00:000:000:000", "(blank data)", "15", "12:30", "12:30:06",
    		"12:30:56:600", "12:30:56:809:900", "12:30:56:800:900:200", "11:20:30:010:090:040", "11:01:02:001:009:004",
    		"15:25:00:000:000:000", "12:36:40:000:000:000", "10:20:03:000:000:000", "12:36:40:200:000:000", "12:36:40:020:000:000",
    		"12:36:40:002:000:000", "11:22:33:400:500:000", "11:22:33:400:050:000", "11:22:33:400:005:000", "11:22:33:400:005:700",
    		"11:22:33:400:005:070", "11:22:33:400:005:007", "02:02:02:002:002:002" 
    };
    
	public void testDateTime_FormatTypes()
	{
		// Test setting invalid DateTimeFormatType
		TestUtilities.printTestHead("testDateTime_FormatTypes", "Set Invalid format");
		try {
			_dtStringFmt.format(3);
			TestUtilities.checkResult( false, "DateTimeFormatType - did not get expected exception"  );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( true, "Exception expected: " +  excp.getMessage()  );
		}
		
		// Test setting STR_DATETIME_ISO8601 format
		TestUtilities.printTestHead("testDateTime_FormatTypes", "Set STR_DATETIME_ISO8601 format");
		try {
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
			TestUtilities.checkResult( _dtStringFmt.format() == DateTimeStringFormatTypes.STR_DATETIME_ISO8601, "DateTimeFormatType -STR_DATETIME_ISO8601"  );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "DateTimeFormatType Exception not expected: " +  excp.getMessage()  );
			return;
		}		
		
		// Test setting STR_DATETIME_RSSL format
		TestUtilities.printTestHead("testDateTime_FormatTypes", "Set STR_DATETIME_RSSL format");
		try {
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_RSSL);
			TestUtilities.checkResult( _dtStringFmt.format() == DateTimeStringFormatTypes.STR_DATETIME_RSSL, "DateTimeFormatType -STR_DATETIME_RSSL"  );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "DateTimeFormatType Exception  not expected: " +  excp.getMessage()  );
			return;
		}		
	}
	
	public void testOmmDateToString_OuputFormat()
	{
		TestUtilities.printTestHead("testOmmDateToString_OuputFormat", "STR_DATETIME_ISO8601 format");
		
		try {			
			
			// Simple valid date test
			_elementList.clear();
			_elementList.add(EmaFactory.createElementEntry().date(_ename, 2010, 11, 30));			
			
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.DATE)
				{
					_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
					_oString = _dtStringFmt.dateAsString((OmmDate)elementEntry.load());
					TestUtilities.checkResult(_oString.equalsIgnoreCase("2010-11-30"), "Test1");
				}				
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
		
		// Different values of dates -ISO8601 output format
		int i =0;

		try {
			_elementList.clear();
			 for(i=0; i < 12; ++i)
			 {
				 _elementList.add(EmaFactory.createElementEntry().date( _dateTestName[i], 
						 _iDate[i][2], _iDate[i][1], _iDate[i][0]));
			 }
			 
			 _decodeElementList.clear();
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			i = 0;
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.DATE)
				{
					_oString = _dtStringFmt.dateAsString((OmmDate)elementEntry.load());
					TestUtilities.checkResult(_oString.equalsIgnoreCase(_expIso8601Date[i]), _dateTestName[i]);
				}	
				++i;
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
		
		// Different values of dates -RSSL output format
		TestUtilities.printTestHead("testOmmDateToString_OuputFormat", "STR_DATETIME_RSSL format");
		try {
	 
			 _decodeElementList.clear();
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			i = 0;
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_RSSL);
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.DATE)
				{
					_oString = _dtStringFmt.dateAsString((OmmDate)elementEntry.load());
					TestUtilities.checkResult(_oString.equalsIgnoreCase(_expRsslDate[i]), _dateTestName[i]);
				}	
				++i;
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}	
	}
	
	public void testOmmTimeToString_OuputFormat()
	{
		TestUtilities.printTestHead("testOmmTimeToString_OuputFormat", "STR_DATETIME_ISO8601 format");
		
		// Different values of times -ISO8601 output format
		int i =0;

		try {
			_elementList.clear();
			 for(i=0; i < 23; ++i)
			 {
				 _elementList.add(EmaFactory.createElementEntry().time( _timeTestName[i], 
						 _iTime[i][0], _iTime[i][1], _iTime[i][2],_iTime[i][3],_iTime[i][4],_iTime[i][5]));
			 }
			 
			 _decodeElementList.clear();
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			i = 0;
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.TIME)
				{
					_oString = _dtStringFmt.timeAsString((OmmTime)elementEntry.load());
					TestUtilities.checkResult(_oString.equalsIgnoreCase(_expIso8601Time[i]), _timeTestName[i]);
				}	
				++i;
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
		
		TestUtilities.printTestHead("testOmmTimeToString_OuputFormat", "STR_DATETIME_RSSL format");
		
		// Different values of times -RSSL output format
		try {	 
			 _decodeElementList.clear();
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			i = 0;
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_RSSL);
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.TIME)
				{
					_oString = _dtStringFmt.timeAsString((OmmTime)elementEntry.load());
					TestUtilities.checkResult(_oString.equalsIgnoreCase(_expRsslTime[i]), _timeTestName[i]);
				}	
				++i;
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testOmmDateTimeToString_OuputFormat()
	{
		
		int i = 0;
		int j = 0;
		String testName = "";
		String _expDateTime = "";

		// Encode different DateTime values
		try {
			_elementList.clear();
			for(i=0; i < 12; ++i)
			{		
				 for(j=0; j < 23; ++j)
				 {
					 testName = _dateTestName[i] + "-" + _timeTestName[j];
					 _elementList.add(EmaFactory.createElementEntry().dateTime(testName, 
							 _iDate[i][2], _iDate[i][1], _iDate[i][0], 
							 _iTime[j][0], _iTime[j][1], _iTime[j][2],_iTime[j][3],_iTime[j][4],_iTime[j][5]));
				 }
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
		
		// Different values of times -ISO8601 output format		
		TestUtilities.printTestHead("testOmmDateTimeToString_OuputFormat", "STR_DATETIME_ISO8601 format");		
		try {
			i = 0;
			j = 0;
			 _decodeElementList.clear();
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.DATETIME)
				{
					_oString = _dtStringFmt.dateTimeAsString((OmmDateTime)elementEntry.load());
					if(i == 0)
							_expDateTime = _expIso8601Time[j];
					else
					{
						if(j == 1)
							_expDateTime = _expIso8601Date[i];
						else
							_expDateTime = _expIso8601Date[i] + "T" + _expIso8601Time[j];
					}
					TestUtilities.checkResult(_oString.equalsIgnoreCase(_expDateTime),  _dateTestName[i] + "_" + _timeTestName[j]);
				}	
				
				if(j < 22)
					++j;
				else
				{
					++i;
					j = 0;
				}
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
		
		// Different values of times -RSSL output format		
		TestUtilities.printTestHead("testOmmDateTimeToString_OuputFormat", "STR_DATETIME_RSSL format");		
		try {
			i = 0;
			j = 0;
			 _decodeElementList.clear();
			JUnitTestConnect.setRsslData(_decodeElementList, _elementList,  Codec.majorVersion(), Codec.minorVersion(), null, null);
			_dtStringFmt.format(DateTimeStringFormatTypes.STR_DATETIME_RSSL);
			for (ElementEntry elementEntry : _decodeElementList)
			{
				if(elementEntry.load().dataType() == DataTypes.DATETIME)
				{
					_oString = _dtStringFmt.dateTimeAsString((OmmDateTime)elementEntry.load());
					if(i == 0)
							_expDateTime = _expRsslTime[j];
					else
					{
						if(j == 1)
							_expDateTime = _expRsslDate[i].trim();
						else
							_expDateTime = _expRsslDate[i].trim() + " " + _expRsslTime[j];
					}
					TestUtilities.checkResult(_oString.equalsIgnoreCase(_expDateTime),  _dateTestName[i] + "_" + _timeTestName[j]);
			}	
				
				if(j < 22)
					++j;
				else
				{
					++i;
					j = 0;
				}
			}
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "Exception not expected: " +  excp.getMessage()  );
			return;
		}
			
	}
}
