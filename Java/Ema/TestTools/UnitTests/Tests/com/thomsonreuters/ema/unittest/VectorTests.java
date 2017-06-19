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
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.Vector;
import com.thomsonreuters.ema.access.VectorEntry;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import junit.framework.TestCase;

public class VectorTests extends TestCase
{

	public VectorTests(String name)
	{
		super(name);
	}

	public void testVectorEmpty_Encode()
	{
		TestUtilities.printTestHead("testVectorEmpty_Encode","Encode Vector of no entry with EMA");
		
		try {
			Vector vector =EmaFactory.createVector() ;
			FieldEntry fieldEntry = EmaFactory.createFieldEntry();
			
			fieldEntry.vector(3, vector);
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( true, "empty Vector - exception expected: " +  excp.getMessage()  );
			return;
		}

		TestUtilities.checkResult( false, "empty Vector - did not get expected exception" );
	}
	
	public void testVectorContainsFieldLists_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testVectorContainsFieldLists_EncodeDecodeAll","Encode Vector that contains FieldLists with EMA and Decode Vector with EMA");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding
			// encoding order:  SummaryData(with FieldList), Delete, FieldList-Add, FieldList-Add, FieldList-Update

			Vector vectorEnc = EmaFactory.createVector() ;
			TestUtilities.EmaEncodeVectorAllWithFieldList( vectorEnc);

			//Now do EMA decoding of Vector
			Vector vectorDec = JUnitTestConnect.createVector() ;
			JUnitTestConnect.setRsslData(vectorDec, vectorEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(vectorDec);

			Iterator<VectorEntry> vectorIter = vectorDec.iterator();
			
			TestUtilities.checkResult( vectorDec.hasTotalCountHint(), "Vector contains FieldList - hasTotalCountHint()" );
			TestUtilities.checkResult( vectorDec.totalCountHint() == 5, "Vector contains FieldList - getTotalCountHint()" );
			
			switch ( vectorDec.summaryData().dataType() )
			{
				case DataType.DataTypes.FIELD_LIST :
				{
					FieldList fl = vectorDec.summaryData().fieldList();
					TestUtilities.EmaDecodeFieldListAll(fl);
				}
				break;
				default :
					TestUtilities.checkResult( false, "Vector Decode Summary FieldList - vector.summaryType() not expected" );
				break;
			}

			TestUtilities.checkResult( vectorIter.hasNext(), "Vector contains FieldList - first vectorhasNext()" );

			VectorEntry ve1 = vectorIter.next();

			TestUtilities.checkResult(  ve1.position() == 1, "VectorEntry::getPostion()" );
			TestUtilities.checkResult( ve1.action() == VectorEntry.VectorAction.DELETE, "VectorEntry.action() == VectorEntry.vectorAction.DELETE" );
			TestUtilities.checkResult( ve1.load().dataType() == DataTypes.NO_DATA, "VectorEntry.load().dataType() == DataTypes.NO_DATA" );


			vectorIter = vectorDec.iterator();
			{
				TestUtilities.checkResult( vectorIter.hasNext(), "Vector contains FieldList - vectorhasNext() after regenerating iterator" );

				ve1 = vectorIter.next();

				TestUtilities.checkResult(  ve1.position() == 1, "VectorEntry::getPostion()" );
				TestUtilities.checkResult( ve1.action() == VectorEntry.VectorAction.DELETE, "VectorEntry.action() == VectorEntry.vectorAction.DELETE" );
				TestUtilities.checkResult( ve1.load().dataType() == DataTypes.NO_DATA, "VectorEntry.load().dataType() == DataTypes.NO_DATA" );
			}


			TestUtilities.checkResult( vectorIter.hasNext(), "Vector contains FieldList - second vectorhasNext()" );

			VectorEntry ve2 = vectorIter.next();

			TestUtilities.checkResult(  ve2.position() == 1, "VectorEntry::getPostion()" );
			TestUtilities.checkResult( ve2.action() == VectorEntry.VectorAction.SET, "VectorEntry.action() == VectorEntry.vectorAction.ADD" );
			TestUtilities.checkResult( ve2.load().dataType() == DataType.DataTypes.FIELD_LIST, "VectorEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeFieldListAll( ve2.fieldList() );

			TestUtilities.checkResult( vectorIter.hasNext(), "Vector contains FieldList - third vectorhasNext()" );

			VectorEntry ve3 = vectorIter.next();

			TestUtilities.checkResult(  ve3.position() == 2, "VectorEntry::getPostion()" );
			TestUtilities.checkResult( ve3.action() == VectorEntry.VectorAction.SET, "VectorEntry.action() == VectorEntry.vectorAction.ADD" );
			TestUtilities.checkResult( ve3.load().dataType() == DataType.DataTypes.FIELD_LIST, "VectorEntry.load().dataType() == DataTypes.NO_DATA" );
			TestUtilities.EmaDecodeFieldListAll( ve3.fieldList() );
			
			TestUtilities.checkResult( vectorIter.hasNext(), "Vector contains FieldList - fourth vectorhasNext()" );

			VectorEntry ve4 = vectorIter.next();

			TestUtilities.checkResult(  ve4.position() == 3, "VectorEntry::getPostion()" );
			TestUtilities.checkResult( ve4.action() == VectorEntry.VectorAction.UPDATE, "VectorEntry.action() == VectorEntry.vectorAction.UPDATE" );
			TestUtilities.checkResult( ve4.load().dataType() == DataType.DataTypes.FIELD_LIST, "VectorEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );

			TestUtilities.checkResult( !vectorIter.hasNext(), "Vector contains FieldList - final vectorhasNext()" );

			TestUtilities.checkResult( true, "Vector contains FieldList - exception not expected" );
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "Vector contains FieldList - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}
	
	 public void testVector_EncodeUPAVectorWithFieldList_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithFieldList_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for FieldList, Encode it to another Vector.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(8096));
        
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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);

        System.out.println("\testVector_EncodeUPAVectorWithFieldList_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
	 public void testVector_EncodeUPAVectorWithElementList_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithElementList_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for ElementList, Encode it to another Vector.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(8096));
        
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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);

        System.out.println("\testVector_EncodeUPAVectorWithElementList_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
	 public void testVector_EncodeUPAVectorWithFilterList_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithFilterList_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for FilterList, Encode it to another Vector.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(16000));
        
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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);

        System.out.println("\testVector_EncodeUPAVectorWithFilterList_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
	 public void testVector_EncodeUPAVectorWithSeries_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithSeries_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for Series, Encode it to another Vector.");

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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.SERIES);

        System.out.println("\testVector_EncodeUPAVectorWithSeries_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
	 public void testVector_EncodeUPAVectorWithVector_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithVector_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for Vector, Encode it to another Vector.");

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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.VECTOR);

        System.out.println("\testVector_EncodeUPAVectorWithVector_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
	 public void testVector_EncodeUPAVectorWithMap_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithMap_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for Map, Encode it to another Vector.");

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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.MAP);

        System.out.println("\testVector_EncodeUPAVectorWithMap_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
	 public void testVector_EncodeUPAVectorWithRefreshMsg_EncodeEMA_ToAnotherVector_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testVector_EncodeUPAVectorWithRefreshMsg_EncodeEMA_ToAnotherVector_EMADecode", "Encode Vector with UPA for RefreshMsg, Encode it to another Vector.");

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

        // Encode Vector with UPA.
        if ((retVal = TestUtilities.upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeVectorAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode Vector with EMA.
        com.thomsonreuters.ema.access.Vector vector = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(vector, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different Vector with EMA     
        com.thomsonreuters.ema.access.Vector vectorCopy = EmaFactory.createVector();
        
        vectorCopy.totalCountHint(vector.totalCountHint());
        vectorCopy.summaryData(vector.summaryData().data());
     
        Iterator<VectorEntry> iterator = vector.iterator();
        while (iterator.hasNext())
        {
        	vectorCopy.add(iterator.next());
        }
        
        assertEquals(vectorCopy.size(), vector.size());
        
        com.thomsonreuters.ema.access.Vector DecVectorCopy = JUnitTestConnect.createVector();
        JUnitTestConnect.setRsslData(DecVectorCopy, vectorCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAVectorAll(DecVectorCopy, com.thomsonreuters.upa.codec.DataTypes.MSG);

        System.out.println("\testVector_EncodeUPAVectorWithRefreshMsg_EncodeEMA_ToAnotherVector_EMADecode passed");
	 }
	 
}
