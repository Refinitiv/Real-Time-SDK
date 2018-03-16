///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
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
			TestUtilities.checkResult( false, "empty Vector - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testVectorWithSummaryDataButNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testVectorWithSummaryDataButNoPayload_Encode_Decode","Encode Vector with summary data but no entry with EMA");
		
		com.thomsonreuters.upa.codec.DataDictionary dataDictionary = TestUtilities.getDataDictionary();
		
		try
		{
			FieldList summaryData = EmaFactory.createFieldList();
			summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
			summaryData.add(EmaFactory.createFieldEntry().enumValue(15, 840));
			summaryData.add(EmaFactory.createFieldEntry().date(3386, 2018, 2, 28));
			
			Vector vector = EmaFactory.createVector();
			vector.totalCountHint(0).summaryData(summaryData);
			
			ElementList elementList = EmaFactory.createElementList();
			elementList.info(1).add(EmaFactory.createElementEntry().vector("1", vector));
			
			ElementList elementListDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), dataDictionary, null);
			
			TestUtilities.checkResult( elementListDec.infoElementListNum() == 1, "Check info of ElementList" );
			
			Iterator<ElementEntry> elementListIt = elementListDec.iterator();
			
			ElementEntry elementEntry = elementListIt.next();
			
			Vector vectorDec = elementEntry.vector();
			
			TestUtilities.checkResult( vectorDec.totalCountHint() == 0, "Check totoal count hint of Vector" );
			
			FieldList summaryDataDec = vectorDec.summaryData().fieldList();
			
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
			
			TestUtilities.checkResult( vectorDec.iterator().hasNext() == false, "Check whether this is an entry from Vector");
			
			TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether this is an entry from ElementList");
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode no entry Vector - exception not expected with text : " +  excp.getMessage()  );
		}
	}
	
	public void testVectorWithSummaryDataButMisMatchEntryPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testVectorWithSummaryDataButMisMatchEntryPayload_Encode_Decode","Encode Vector with summary data but mismatch entry's payload with EMA");
		
		try
		{
			FieldList summaryData = EmaFactory.createFieldList();
			summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
			
			ElementList elementList = EmaFactory.createElementList();
			elementList.add(EmaFactory.createElementEntry().codeAscii("Name1"));
		
			Vector vector = EmaFactory.createVector();
			vector.totalCountHint(0).summaryData(summaryData).add(EmaFactory.createVectorEntry()
					.elementList(1, VectorEntry.VectorAction.INSERT, elementList));
			
			Map map = EmaFactory.createMap();
			map.add(EmaFactory.createMapEntry().keyAscii("key", MapEntry.MapAction.ADD, vector));
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( true, "Fails to encode Vector with summary data but mismatch entry's payload - exception expected with text : " +  excp.getMessage());
			TestUtilities.checkResult( excp.getMessage().equals("Attempt to add entry of ELEMENT_LIST while Vector entry load type is set to FIELD_LIST with summaryData() method")
					, "Check exception text");
			
			return;
		}
		
		TestUtilities.checkResult( false, "Fails to encode Vector with summary data but mismatch entry's payload - exception expected with");
	}
	
	public void testVectorEntryWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testVectorEntryWithNoPayload_Encode_Decode","Encode multiple Vector entry with no payload");
		
		try
		{
		ByteBuffer permissionData = ByteBuffer.allocate(5);
		permissionData.putInt(12345).flip();
		
		Vector vector = EmaFactory.createVector();
		vector.add(EmaFactory.createVectorEntry().noData(1, VectorEntry.VectorAction.INSERT));
		vector.add(EmaFactory.createVectorEntry().noData(2, VectorEntry.VectorAction.SET, permissionData));
		
		Vector vectorDec = JUnitTestConnect.createVector();
		JUnitTestConnect.setRsslData(vectorDec, vector, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<VectorEntry> vectorIt = vectorDec.iterator();
		
		VectorEntry vectorEntry = vectorIt.next();
		TestUtilities.checkResult( vectorEntry.position() == 1, "Check the positoin of the first entry");
		TestUtilities.checkResult( vectorEntry.action() == VectorEntry.VectorAction.INSERT, "Check the action of the first entry");
		
		
		vectorEntry = vectorIt.next();
		TestUtilities.checkResult( vectorEntry.position() == 2, "Check the positoin of the second entry");
		TestUtilities.checkResult( vectorEntry.action() == VectorEntry.VectorAction.SET, "Check the action of the second entry");
		TestUtilities.checkResult( vectorEntry.hasPermissionData() , "Check has permission data for second entry");
		TestUtilities.checkResult( vectorEntry.permissionData().equals(permissionData) , "Check the permission data for the second entry");
		
		TestUtilities.checkResult( vectorIt.hasNext() == false , "Check to make sure there is no more entry");
		
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to Encode multiple Vector entry with no payload - exception not expected with text : " +  excp.getMessage()  );
			return;
		}	
	}
	
	public void testVectorClear_Encode_Decode()
	{
		TestUtilities.printTestHead("testVectorClear_Encode_Decode","Test Clear Vector before encoding");
		
		try
		{
		FieldList summaryData = EmaFactory.createFieldList();
		summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 4563));
		
		Vector vector = EmaFactory.createVector();
		vector.summaryData(summaryData);
		
		Vector vectorDec = JUnitTestConnect.createVector();
		JUnitTestConnect.setRsslData(vectorDec, vector, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		TestUtilities.checkResult( vectorDec.summaryData().dataType() == DataType.DataTypes.FIELD_LIST, "Check data type of summary data before calling the clear method" );
		
		vector.clear();
		
		TestUtilities.checkResult( vector.summaryData().dataType() == DataType.DataTypes.NO_DATA, "Check data type of summary data after calling the clear method" );
		
		vector.add(EmaFactory.createVectorEntry().noData(5, VectorEntry.VectorAction.INSERT));
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().vector("1", vector));
		
		ElementList elementListDec = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<ElementEntry> elementListIt = elementListDec.iterator();
		
		ElementEntry elementEntry = elementListIt.next();
		
		TestUtilities.checkResult( elementEntry.name().equals("1"), "Check element list key value" );
		
		Iterator<VectorEntry> vectorIt = elementEntry.vector().iterator();
		
		VectorEntry vectorEntry = vectorIt.next();
		
		TestUtilities.checkResult( vectorEntry.position() == 5, "Check the position value of the first Vector entry" );
		TestUtilities.checkResult( vectorEntry.action() == VectorEntry.VectorAction.INSERT, "Check the action of the first Vector entry" );
		TestUtilities.checkResult( vectorEntry.loadType() == DataType.DataTypes.NO_DATA, "Check the load type of the first Vector entry" );
		
		TestUtilities.checkResult( vectorIt.hasNext() == false, "Check whether there is another Vector entry" );
		
		TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether there is another Element entry" );
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode after calling the clear method - exception not expected with text : " +  excp.getMessage()  );
			return;
		}		
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
