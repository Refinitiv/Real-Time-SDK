///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.SeriesEntry;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.ElementList;

import junit.framework.TestCase;

public class SeriesTests extends TestCase
{

	public SeriesTests(String nase)
	{
		super(nase);
	}

	public void testSeriesContainselementLists_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testSeriesContainselementLists_EncodeDecodeAll","Encode Series that contains elementLists with EMA and Decode Series with EMA");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with elementList), Delete, elementList-Add, elementList-Add, elementList-Update
			Series seriesEnc = EmaFactory.createSeries();
			TestUtilities.EmaEncodeSeriesAllWithElementList( seriesEnc);

			//Now do EMA decoding of Map
			Series seriesDec = JUnitTestConnect.createSeries();
			JUnitTestConnect.setRsslData(seriesDec, seriesEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

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
		
			TestUtilities.checkResult( !seriesIter.hasNext(), "Series contains elementList - final maphasNext()" );

			TestUtilities.checkResult( true, "Series contains elementList - exception not expected" );
			} catch ( OmmException excp  ) {
				TestUtilities.checkResult( false, "Series contains elementList - exception not expected" );
				System.out.println(excp.getMessage());
		}
	}
	
	 public void testSeries_EncodeUPASeriesWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with UPA for FieldList type, Encode it to another Series.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(5120));
        
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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
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
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);

        System.out.println("\testSeries_EncodeUPASeriesWithFieldListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeUPASeriesWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with UPA for ElementList type, Encode it to another Series.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(5120));
        
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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
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
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);

        System.out.println("\testSeries_EncodeUPASeriesWithElementListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeUPASeriesWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with UPA for FilterList type, Encode it to another Series.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(18000));
        
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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
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
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);

        System.out.println("\testSeries_EncodeUPASeriesWithFilterListType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeUPASeriesWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with UPA for Series type, Encode it to another Series.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(18000));
        
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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
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
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.SERIES);

        System.out.println("\testSeries_EncodeUPASeriesWithSeriesType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeUPASeriesWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with UPA for Vector type, Encode it to another Series.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(18000));
        
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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
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
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.VECTOR);

        System.out.println("\testSeries_EncodeUPASeriesWithVectorType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeUPASeriesWithMapType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithMapType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Map with UPA for Vector type, Encode it to another Series.");

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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Series with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        System.out.println(series);
        
        // Copy decoded entries into a different Series with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
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
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.MAP);

        System.out.println("\testSeries_EncodeUPASeriesWithMapType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 public void testSeries_EncodeUPASeriesWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testSeries_EncodeUPASeriesWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode", "Encode Series with UPA for RefreshMsg type, Encode it to another Series.");

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

        // Encode Series with UPA.
        if ((retVal = TestUtilities.upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeSeriesAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FilterList with EMA.
        com.thomsonreuters.ema.access.Series series = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(series, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different FilterList with EMA     
        com.thomsonreuters.ema.access.Series seriesCopy = EmaFactory.createSeries();
        
        seriesCopy.totalCountHint(series.totalCountHint());
        seriesCopy.summaryData(series.summaryData().data());
     
        Iterator<SeriesEntry> iterator = series.iterator();
        while (iterator.hasNext())
        {
        	seriesCopy.add(iterator.next());
        }
        
        assertEquals(seriesCopy.size(), series.size());
        
        com.thomsonreuters.ema.access.Series decSeriesCopy = JUnitTestConnect.createSeries();
        JUnitTestConnect.setRsslData(decSeriesCopy, seriesCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPASeriesAll(decSeriesCopy, com.thomsonreuters.upa.codec.DataTypes.MSG);

        System.out.println("\testSeries_EncodeUPASeriesWithRefreshMsgType_EncodeEMA_ToAnotherSeries_EMADecode passed");
	 }
	 
	 
}
