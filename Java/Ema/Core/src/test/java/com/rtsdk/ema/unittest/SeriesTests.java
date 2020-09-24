///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.ElementEntry;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.JUnitTestConnect;
import com.rtsdk.ema.access.Map;
import com.rtsdk.ema.access.MapEntry;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.SeriesEntry;
import com.rtsdk.ema.access.Series;
import com.rtsdk.ema.access.ElementList;

import junit.framework.TestCase;

public class SeriesTests extends TestCase
{

	public SeriesTests(String nase)
	{
		super(nase);
	}
	
	public void testSeriesEmpty_Encode()
	{
		TestUtilities.printTestHead("testSeriesEmpty_Encode","Encode Series of no entry with EMA");
		
		try {
			Series series =EmaFactory.createSeries();
			FieldEntry fieldEntry = EmaFactory.createFieldEntry();
			
			fieldEntry.series(3, series);
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "empty Series - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testSeriesWithSummaryDataButNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testSeriesWithSummaryDataButNoPayload_Encode_Decode","Encode Series with summary data but no entry with EMA");
		
		com.rtsdk.eta.codec.DataDictionary dataDictionary = TestUtilities.getDataDictionary();
		
		try
		{
			FieldList summaryData = EmaFactory.createFieldList();
			summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
			summaryData.add(EmaFactory.createFieldEntry().enumValue(15, 840));
			summaryData.add(EmaFactory.createFieldEntry().date(3386, 2018, 2, 28));
			
			Series series = EmaFactory.createSeries();
			series.totalCountHint(0).summaryData(summaryData);
			
			ElementList elementList = EmaFactory.createElementList();
			elementList.info(1).add(EmaFactory.createElementEntry().series("1", series));
			
			ElementList elementListDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), dataDictionary, null);
			
			TestUtilities.checkResult( elementListDec.infoElementListNum() == 1, "Check info of ElementList" );
			
			Iterator<ElementEntry> elementListIt = elementListDec.iterator();
			
			ElementEntry elementEntry = elementListIt.next();
			
			Series seriesDec = elementEntry.series();
			
			TestUtilities.checkResult( seriesDec.totalCountHint() == 0, "Check totoal count hint of Series" );
			
			FieldList summaryDataDec = seriesDec.summaryData().fieldList();
			
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
			
			TestUtilities.checkResult( seriesDec.iterator().hasNext() == false, "Check whether this is an entry from Series");
			
			TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether this is an entry from ElementList");
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode no entry Series - exception not expected with text : " +  excp.getMessage()  );
		}
	}
	
	public void testSeriesWithSummaryDataButMisMatchEntryPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testSeriesWithSummaryDataButMisMatchEntryPayload_Encode_Decode","Encode Series with summary data but mismatch entry's payload with EMA");
		
		try
		{
			FieldList summaryData = EmaFactory.createFieldList();
			summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
			
			ElementList elementList = EmaFactory.createElementList();
			elementList.add(EmaFactory.createElementEntry().codeAscii("Name1"));
		
			Series series = EmaFactory.createSeries();
			series.totalCountHint(0).summaryData(summaryData).add(EmaFactory.createSeriesEntry()
					.elementList(elementList));
			
			Map map = EmaFactory.createMap();
			map.add(EmaFactory.createMapEntry().keyAscii("key", MapEntry.MapAction.ADD, series));
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( true, "Fails to encode Series with summary data but mismatch entry's payload - exception expected with text : " +  excp.getMessage());
			TestUtilities.checkResult( excp.getMessage().equals("Attempt to add entry of ELEMENT_LIST while Series entry load type is set to FIELD_LIST with summaryData() method")
					, "Check exception text");
			
			return;
		}
		
		TestUtilities.checkResult( false, "Fails to encode Series with summary data but mismatch entry's payload - exception expected with");
	}
	
	public void testSeriesEntryWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testSeriesEntryWithNoPayload_Encode_Decode","Encode multiple Series entry with no payload");
		
		try
		{
		ByteBuffer permissionData = ByteBuffer.allocate(5);
		permissionData.putInt(12345).flip();
		
		Series series = EmaFactory.createSeries();
		series.add(EmaFactory.createSeriesEntry().noData());
		series.add(EmaFactory.createSeriesEntry().noData());
		
		Series seriesDec = JUnitTestConnect.createSeries();
		JUnitTestConnect.setRsslData(seriesDec, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<SeriesEntry> seriesIt = seriesDec.iterator();
		
		SeriesEntry seriesEntry = seriesIt.next();
		TestUtilities.checkResult( seriesEntry.loadType() == DataType.DataTypes.NO_DATA, "Check load type of the first entry");
		
		seriesEntry = seriesIt.next();
		TestUtilities.checkResult( seriesEntry.loadType() == DataType.DataTypes.NO_DATA, "Check load type of the second entry");
		
		TestUtilities.checkResult( seriesIt.hasNext() == false , "Check to make sure there is no more entry");
		
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to Encode multiple Series entry with no payload - exception not expected with text : " +  excp.getMessage()  );
			return;
		}	
	}
	
	public void testSeriesClear_Encode_Decode()
	{
		TestUtilities.printTestHead("testSeriesClear_Encode_Decode","Test Clear Series before encoding");
		
		try
		{
		FieldList summaryData = EmaFactory.createFieldList();
		summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 4563));
		
		Series series = EmaFactory.createSeries();
		series.summaryData(summaryData);
		
		Series seriesDec = JUnitTestConnect.createSeries();
		JUnitTestConnect.setRsslData(seriesDec, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		TestUtilities.checkResult( seriesDec.summaryData().dataType() == DataType.DataTypes.FIELD_LIST, "Check data type of summary data before calling the clear method" );
		
		series.clear();
		
		TestUtilities.checkResult( series.summaryData().dataType() == DataType.DataTypes.NO_DATA, "Check data type of summary data after calling the clear method" );
		
		series.add(EmaFactory.createSeriesEntry().noData());
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().series("1", series));
		
		ElementList elementListDec = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<ElementEntry> elementListIt = elementListDec.iterator();
		
		ElementEntry elementEntry = elementListIt.next();
		
		TestUtilities.checkResult( elementEntry.name().equals("1"), "Check element list key value" );
		
		Iterator<SeriesEntry> seriesIt = elementEntry.series().iterator();
		
		SeriesEntry seriesEntry = seriesIt.next();
		
		TestUtilities.checkResult( seriesEntry.loadType() == DataType.DataTypes.NO_DATA, "Check the load type of the first Series entry" );
		
		TestUtilities.checkResult( seriesIt.hasNext() == false, "Check whether there is another Series entry" );
		
		TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether there is another Element entry" );
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails encode after calling the clear method - exception not expected with text : " +  excp.getMessage()  );
			return;
		}		
	}

	public void testSeriesContainselementLists_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testSeriesContainselementLists_EncodeDecodeAll","Encode Series that contains elementLists with EMA and Decode Series with EMA");
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with elementList), Delete, elementList-Add, elementList-Add, elementList-Update
			Series seriesEnc = EmaFactory.createSeries();
			TestUtilities.EmaEncodeSeriesAllWithElementList( seriesEnc);
			TestUtilities.checkResult("Series.toString() == toString() not supported", seriesEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			ElementList elEnc = EmaFactory.createElementList(); 
			TestUtilities.EmaEncodeElementListAll(elEnc);			
			SeriesEntry se = EmaFactory.createSeriesEntry().elementList(elEnc);
			TestUtilities.checkResult("SeriesEntry.toString() == toString() not supported", se.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			seriesEnc.add(se);
			
			//Now do EMA decoding of Map
			Series seriesDec = JUnitTestConnect.createSeries();
			JUnitTestConnect.setRsslData(seriesDec, seriesEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			// check that we can still get the toString on encoded/decoded container.
			TestUtilities.checkResult("Series.toString() != toString() not supported", !(seriesDec.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));			

			System.out.println(seriesDec);

			Iterator<SeriesEntry> seriesIter = seriesDec.iterator();

			TestUtilities.checkResult( seriesDec.hasTotalCountHint(), "Series contains elementList - hasTotalCountHint()" );
			TestUtilities.checkResult( seriesDec.totalCountHint() == 5, "Series contains ElelementList - getTotalCountHint()" );

			switch ( seriesDec.summaryData().dataType() )
			{
				case DataType.DataTypes.ELEMENT_LIST :
				{
					ElementList el = seriesDec.summaryData().elementList();
					TestUtilities.EmaDecodeElementListAll( el );
				}
				break;
				default :
					TestUtilities.checkResult( false, "Series Decode Summary elementList - map.summaryType() not expected" );
				break;
			}
			
			TestUtilities.checkResult( seriesIter.hasNext(), "Series contains elementList - first Series hasNext()" );

			SeriesEntry se1 = seriesIter.next();
			TestUtilities.checkResult( se1.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "SeriesEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeElementListAll( se1.elementList() );
	
			TestUtilities.checkResult( seriesIter.hasNext(), "Series contains elementList - third maphasNext()" );
	
			SeriesEntry se2 = seriesIter.next();
	
			TestUtilities.checkResult( se2.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "SeriesEntry.load().dataType() == DataTypes.NO_DATA" );
			TestUtilities.EmaDecodeElementListAll( se2.elementList() );
			
			TestUtilities.checkResult( seriesIter.hasNext(), "Series contains elementList - fourth maphasNext()" );
	
			SeriesEntry se3 = seriesIter.next();
	
			TestUtilities.checkResult( se3.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "SeriesEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeElementListAll(se3.elementList() );
		
			SeriesEntry se4 = seriesIter.next();
			
			TestUtilities.checkResult( se4.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "SeriesEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeElementListAll(se4.elementList() );
			
			// check that we can still get the toString on encoded/decoded entry.
			TestUtilities.checkResult("SeriesEntry.toString() != toString() not supported", !(se4.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));
			
			
			TestUtilities.checkResult( !seriesIter.hasNext(), "Series contains elementList - final maphasNext()" );

			TestUtilities.checkResult( true, "Series contains elementList - exception not expected" );
			} catch ( OmmException excp  ) {
				TestUtilities.checkResult( false, "Series contains elementList - exception not expected" );
				System.out.println(excp.getMessage());
		}
	}
	
	 public void testSeries_EncodeETASeriesWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with ETA for FieldList type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(5120));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        SeriesEntry seriesEntry;
        while (iterator.hasNext())
        {
        	seriesEntry = iterator.next();
        	seriesCopy.add(seriesEntry);
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.FIELD_LIST);

        System.out.println("\testSeries_EncodeETASeriesWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeETASeriesWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with ETA for ElementList type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(5120));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        SeriesEntry seriesEntry;
        while (iterator.hasNext())
        {
        	seriesEntry = iterator.next();
        	seriesCopy.add(seriesEntry);
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);

        System.out.println("\testSeries_EncodeETASeriesWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeETASeriesWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with ETA for FilterList type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(18000));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        SeriesEntry seriesEntry;
        while (iterator.hasNext())
        {
        	seriesEntry = iterator.next();
        	seriesCopy.add(seriesEntry);
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.FILTER_LIST);

        System.out.println("\testSeries_EncodeETASeriesWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeETASeriesWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with ETA for Series type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(18000));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        SeriesEntry seriesEntry;
        while (iterator.hasNext())
        {
        	seriesEntry = iterator.next();
        	seriesCopy.add(seriesEntry);
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.SERIES);

        System.out.println("\testSeries_EncodeETASeriesWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeETASeriesWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with ETA for Vector type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(18000));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        SeriesEntry seriesEntry;
        while (iterator.hasNext())
        {
        	seriesEntry = iterator.next();
        	seriesCopy.add(seriesEntry);
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.VECTOR);

        System.out.println("\testSeries_EncodeETASeriesWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeETASeriesWithMapType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithMapType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with ETA for Vector type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(14000));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        SeriesEntry seriesEntry;
        while (iterator.hasNext())
        {
        	seriesEntry = iterator.next();
        	seriesCopy.add(seriesEntry);
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.MAP);

        System.out.println("\testSeries_EncodeETASeriesWithMapType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeETASeriesWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeETASeriesWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with ETA for RefreshMsg type, Encode it to another Series.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}

        // Encode Series with ETA.
        if ((retVal = TestUtilities.eta_EncodeSeriesAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FilterList with EMA.
        com.rtsdk.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different FilterList with EMA     
        com.rtsdk.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        while (iterator.hasNext())
        {
        	seriesCopy.add(iterator.next());
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.rtsdk.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETASeriesAll(decSeriesCopy, com.rtsdk.eta.codec.DataTypes.MSG);

        System.out.println("\testSeries_EncodeETASeriesWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 
}
