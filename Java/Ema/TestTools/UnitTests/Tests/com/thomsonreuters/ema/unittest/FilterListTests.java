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
