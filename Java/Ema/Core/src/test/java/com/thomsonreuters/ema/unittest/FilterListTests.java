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
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.thomsonreuters.ema.access.DataType.DataTypes;

import junit.framework.TestCase;

public class FilterListTests extends TestCase
{

	public FilterListTests(String name)
	{
		super(name);
	}
	
	public void testFilterListEmpty_Encode()
	{
		TestUtilities.printTestHead("testFilterListEmpty_Encode","Encode FilterList of no entry with EMA");
		
		try {
			FilterList filterList = EmaFactory.createFilterList() ;
			FieldEntry fieldEntry = EmaFactory.createFieldEntry();
			
			fieldEntry.filterList(3, filterList);
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( false, "empty FilterList - exception not expected: " +  excp.getMessage()  );
			return;
		}
	}
	
	public void testFilterEntryWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testFilterEntryWithNoPayload_Encode_Decode","Encode multiple Filter entry with no payload");
		
		com.thomsonreuters.upa.codec.DataDictionary dataDictionary = TestUtilities.getDataDictionary();
		
		try
		{
		ByteBuffer permissionData = ByteBuffer.allocate(5);
		permissionData.putInt(12345).flip();
		
		FieldList fieldListEnc = EmaFactory.createFieldList();
		fieldListEnc.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
		
		FilterList filterList = EmaFactory.createFilterList();
		
		FilterEntry fe = EmaFactory.createFilterEntry().noData(1, FilterEntry.FilterAction.SET);
		TestUtilities.checkResult("FilterEntry.toString() == toString() not supported", fe.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		filterList.add(fe);
		TestUtilities.checkResult("FilterList.toString() == toString() not supported", filterList.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		filterList.add(EmaFactory.createFilterEntry().noData(2, FilterEntry.FilterAction.UPDATE, permissionData));
		TestUtilities.checkResult("FilterList.toString() == toString() not supported", filterList.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

		filterList.add(EmaFactory.createFilterEntry().fieldList(3, FilterEntry.FilterAction.SET, fieldListEnc));
		TestUtilities.checkResult("FilterList.toString() == toString() not supported", filterList.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		FilterList filterListDec = JUnitTestConnect.createFilterList();
		JUnitTestConnect.setRsslData(filterListDec, filterList, Codec.majorVersion(), Codec.minorVersion(), dataDictionary, null);
		// check that we can still get the toString on encoded/decoded container.
		TestUtilities.checkResult("FilterList.toString() != toString() not supported", !(filterListDec.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));			

		Iterator<FilterEntry> filerListIt = filterListDec.iterator();
		
		FilterEntry filterEntry = filerListIt.next();
		// check that we can still get the toString on encoded/decoded entry.
		TestUtilities.checkResult("FilterEntry.toString() != toString() not supported", !(filterEntry.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));
		
		TestUtilities.checkResult( filterEntry.filterId() == 1, "Check the filter ID of the first entry");
		TestUtilities.checkResult( filterEntry.action() == FilterEntry.FilterAction.SET, "Check the action of the first entry");
		TestUtilities.checkResult( filterEntry.hasPermissionData() == false , "Check has permission data for first entry");
		
		filterEntry = filerListIt.next();
		TestUtilities.checkResult( filterEntry.filterId() == 2, "Check the filter ID of the second entry");
		TestUtilities.checkResult( filterEntry.action() == FilterEntry.FilterAction.UPDATE, "Check the action of the second entry");
		TestUtilities.checkResult( filterEntry.hasPermissionData() , "Check has permission data for second entry");
		TestUtilities.checkResult( filterEntry.permissionData().equals(permissionData) , "Check the permission data for the second entry");
		
		filterEntry = filerListIt.next();
		TestUtilities.checkResult( filterEntry.filterId() == 3, "Check the filter ID of the third entry");
		TestUtilities.checkResult( filterEntry.action() == FilterEntry.FilterAction.SET, "Check the action of the third entry");
		TestUtilities.checkResult( filterEntry.hasPermissionData() == false , "Check has permission data for third entry");
		
		FieldList fieldListDec = filterEntry.fieldList();
		
		Iterator<FieldEntry> fieldIt = fieldListDec.iterator();
		
		FieldEntry fieldEntry = fieldIt.next();
		
		TestUtilities.checkResult( fieldEntry.fieldId() == 1, "Check the field ID of the first field entry" );
		TestUtilities.checkResult( fieldEntry.uintValue() == 3056, "Check the value of the first field entry" );
		TestUtilities.checkResult( fieldIt.hasNext() == false, "Check whether this is an entry from FieldList");
		
		TestUtilities.checkResult( filerListIt.hasNext() == false , "Check to make sure there is no more entry");
		
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to Encode multiple FilterList entry with no payload - exception not expected with text : " +  excp.getMessage()  );
			return;
		}	
	}
	
	public void testFilterListClear_Encode_Decode()
	{
		TestUtilities.printTestHead("testFilterListClear_Encode_Decode","Test Clear FilterList before encoding");
		
		try
		{
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().uintValue(1, 4563));
		
		FilterList filterList = EmaFactory.createFilterList();
		filterList.totalCountHint(1).add(EmaFactory.createFilterEntry().fieldList(3, FilterEntry.FilterAction.SET, fieldList));
		
		FilterList filterListDec = JUnitTestConnect.createFilterList();
		JUnitTestConnect.setRsslData(filterListDec, filterList, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		TestUtilities.checkResult( filterListDec.iterator().next().loadType() == DataType.DataTypes.FIELD_LIST, "Check data type of an entry before calling the clear method" );
		
		filterList.clear();
		
		TestUtilities.checkResult( filterList.isEmpty() , "Check whether FilterList is empty after calling the clear method" );
		
		filterList.add(EmaFactory.createFilterEntry().noData(5, FilterEntry.FilterAction.UPDATE));
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().filterList("1", filterList));
		
		ElementList elementListDec = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<ElementEntry> elementListIt = elementListDec.iterator();
		
		ElementEntry elementEntry = elementListIt.next();
		
		TestUtilities.checkResult( elementEntry.name().equals("1"), "Check element list key value" );
		
		Iterator<FilterEntry> filterIt = elementEntry.filterList().iterator();
		
		FilterEntry filterEntry = filterIt.next();
		
		TestUtilities.checkResult( filterEntry.filterId() == 5, "Check the filter ID value of the first FilterList entry" );
		TestUtilities.checkResult( filterEntry.action() == FilterEntry.FilterAction.UPDATE, "Check the action of the first FilterList entry" );
		TestUtilities.checkResult( filterEntry.loadType() == DataType.DataTypes.NO_DATA, "Check the load type of the first FilterList entry" );
		
		TestUtilities.checkResult( filterIt.hasNext() == false, "Check whether there is another FilterList entry" );
		
		TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether there is another Element entry" );
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode after calling the clear method - exception not expected with text : " +  excp.getMessage()  );
			return;
		}		
	}

	public void testFilterListContainsFieldListsElementLists_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testFilterListContainsFieldListsElementLists_EncodeDecodeAll","Encode FilterList that contains FieldLists, ElementLists with EMA and Decode FilterList with EMA");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		try {
			//EMA Encoding

			FilterList filterListEnc = EmaFactory.createFilterList();
			TestUtilities.EmaEncodeFilterListAllWithFieldListElementList( filterListEnc);

			//Now do EMA decoding of FilterList
			FilterList filterListDec = JUnitTestConnect.createFilterList();
			JUnitTestConnect.setRsslData(filterListDec, filterListEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(filterListDec);

			Iterator<FilterEntry> filterListIter = filterListDec.iterator();
			
			TestUtilities.checkResult( filterListDec.hasTotalCountHint(), "FilterList contains FieldList - hasTotalCountHint()" );
			TestUtilities.checkResult( filterListDec.totalCountHint() == 3, "FilterList contains FieldList - getTotalCountHint()" );
			
			TestUtilities.checkResult( filterListIter.hasNext(), "FilterList contains FieldList - first FilterList hasNext()" );

			FilterEntry fe1 = filterListIter.next();

			TestUtilities.checkResult( fe1.action() == FilterEntry.FilterAction.CLEAR, "FilterEntry.action() == FilterEntry.FilterAction.CLEAR" );
			TestUtilities.checkResult( fe1.load().dataType() == DataTypes.NO_DATA, "FilterEntry.load().dataType() == DataTypes.NO_DATA" );


			filterListIter = filterListDec.iterator();
			{
				TestUtilities.checkResult( filterListIter.hasNext(), "FilterList contains FieldList - first FilterList hasNext() after regenerating iterator" );

				fe1 = filterListIter.next();

				TestUtilities.checkResult( fe1.action() == FilterEntry.FilterAction.CLEAR, "FilterEntry.action() == FilterEntry.FilterAction.CLEAR" );
				TestUtilities.checkResult( fe1.load().dataType() == DataTypes.NO_DATA, "FilterEntry.load().dataType() == DataTypes.NO_DATA" );
			}


			TestUtilities.checkResult( filterListIter.hasNext(), "FilterList contains FieldList - second FilterList hasNext()" );

			FilterEntry fe2 = filterListIter.next();

			TestUtilities.checkResult( fe2.action() == FilterEntry.FilterAction.SET, "FilterEntry.action() == FilterEntry.FilterAction.SET" );
			TestUtilities.checkResult( fe2.load().dataType() == DataType.DataTypes.FIELD_LIST, "FilterEntry.load().dataType() == DataType.DataTypes.FIELD_LIST" );
			TestUtilities.EmaDecodeFieldListAll( fe2.fieldList() );

			TestUtilities.checkResult( filterListIter.hasNext(), "FilterList contains FieldList - third FilterList hasNext()" );

			FilterEntry fe3 = filterListIter.next();

			TestUtilities.checkResult( fe3.action() == FilterEntry.FilterAction.UPDATE, "FilterEntry.action() == FilterEntry.FilterAction.UPDATE" );
			TestUtilities.checkResult( fe3.load().dataType() == DataType.DataTypes.ELEMENT_LIST, "FilterEntry.load().dataType() == DataTypes.ELEMENT_LIST" );
			TestUtilities.EmaDecodeElementListAll( fe3.elementList() );
			
			TestUtilities.checkResult( !filterListIter.hasNext(), "FilterList contains FieldList - final FilterList hasNext()" );

			TestUtilities.checkResult( true, "FilterList contains FieldList - exception not expected" );
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( false, "FilterList contains FieldList - exception not expected" );
			System.out.println(excp.getMessage());
		}
	}
	
	 public void testFilterList_EncodeUPAFilterListWithContainerTypes_EncodeEMA_ToAnotherFilterList_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testFilterList_EncodeUPAFilterListWithContainerTypes_EncodeEMA_ToAnotherFilterList_EMADecode", "Encode FilterList with UPA for container types, Encode it to another FilterList.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(20000));
        
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

        // Encode FilterList with UPA.
        if ((retVal = TestUtilities.upa_EncodeFilterListAll(encodeIter, TestUtilities.EncodingTypeFlags.CONTAINER_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeFilterListAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FilterList with EMA.
        com.thomsonreuters.ema.access.FilterList filterList = JUnitTestConnect.createFilterList();
        JUnitTestConnect.setRsslData(filterList, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different FilterList with EMA     
        com.thomsonreuters.ema.access.FilterList filterListCopy = EmaFactory.createFilterList();
        
        filterListCopy.totalCountHint(filterList.totalCountHint());
     
        Iterator<FilterEntry> iterator = filterList.iterator();
        while (iterator.hasNext())
        {
        	filterListCopy.add(iterator.next());
        }
        
        assertEquals(filterListCopy.size(), filterList.size());
        
        com.thomsonreuters.ema.access.FilterList filterListDecCopy = JUnitTestConnect.createFilterList();
        
        JUnitTestConnect.setRsslData(filterListDecCopy, filterListCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAFilterListAll(filterListDecCopy, EncodingTypeFlags.CONTAINER_TYPES);

        System.out.println("\ntestFilterList_EncodeUPAFilterListWithContainerTypes_EncodeEMA_ToAnotherFilterList_EMADecode passed");
	 }
	 
	 public void testFilterList_EncodeUPAFilterListWithMsgTypes_EncodeEMA_ToAnotherFilterList_EMADecode() 
	 {
		 int retVal;
		 
		 TestUtilities.printTestHead("testFilterList_EncodeUPAFilterListWithMsgTypes_EncodeEMA_ToAnotherFilterList_EMADecode", "Encode FilterList with UPA for message types, Encode it to another FilterList.");

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

        // Encode FilterList with UPA.
        if ((retVal = TestUtilities.upa_EncodeFilterListAll(encodeIter, TestUtilities.EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeFilterListAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FilterList with EMA.
        com.thomsonreuters.ema.access.FilterList filterList = JUnitTestConnect.createFilterList();
        JUnitTestConnect.setRsslData(filterList, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different FilterList with EMA     
        com.thomsonreuters.ema.access.FilterList filterListCopy = EmaFactory.createFilterList();
        
        filterListCopy.totalCountHint(filterList.totalCountHint());
     
        Iterator<FilterEntry> iterator = filterList.iterator();
        while (iterator.hasNext())
        {
        	filterListCopy.add(iterator.next());
        }
        
        assertEquals(filterListCopy.size(), filterList.size());
        
        com.thomsonreuters.ema.access.FilterList filterListDecCopy = JUnitTestConnect.createFilterList();
        
        JUnitTestConnect.setRsslData(filterListDecCopy, filterListCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_UPAFilterListAll(filterListCopy, EncodingTypeFlags.MESSAGE_TYPES);

        System.out.println("\testFilterList_EncodeUPAFilterListWithMsgTypes_EncodeEMA_ToAnotherFilterList_EMADecode passed");
	 }
	 
}
