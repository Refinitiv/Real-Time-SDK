///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;
import java.util.Arrays;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayImpl;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.ElementListImpl;
import com.thomsonreuters.upa.codec.ElementSetDef;
import com.thomsonreuters.upa.codec.ElementSetDefImpl;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.EncodeIteratorImpl;
import com.thomsonreuters.upa.codec.Encoders;
import com.thomsonreuters.upa.codec.EncodingLevel;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.FilterListImpl;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.LocalElementSetDefDb;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.MapImpl;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyImpl;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.Series;
import com.thomsonreuters.upa.codec.SeriesEntry;
import com.thomsonreuters.upa.codec.SeriesImpl;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.codec.Vector;
import com.thomsonreuters.upa.codec.VectorEntry;
import com.thomsonreuters.upa.codec.VectorEntryActions;
import com.thomsonreuters.upa.codec.VectorImpl;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.UpdateEventTypes;
import com.thomsonreuters.upa.rdm.ViewTypes;

public class EncodersJunit
{
    @BeforeClass
    public static void setRunningInJunits()
    {
        Encoders._runningInJunits = true;
    }

    @AfterClass
    public static void clearRunningInJunits()
    {
    	Encoders._runningInJunits = false;
    }
    
 
    
    
    /**
     * This tests the encodeFilterListInit() method. It contains three test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Negative test for unsupported container type<BR>
     * 3. Positive test for everything good<BR>
     */
    @Test
    public void encodeFilterListInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(1));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(32));
        FilterList filterList = CodecFactory.createFilterList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
     
        filterList.applyHasPerEntryPermData();
        filterList.applyHasTotalCountHint();
        filterList.totalCountHint(5);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeFilterListInitTest");
        ((FilterListImpl)filterList).encodedEntries(txt);
        filterList.containerType(DataTypes.FIELD_LIST);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        // 2. Negative test for unsupported container type
        ((FilterListImpl)filterList)._containerType = DataTypes.ARRAY;
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.UNSUPPORTED_DATA_TYPE);
        
        // reset container type
        filterList.containerType(DataTypes.FIELD_LIST);
        
        // 3. Positive test for everything good
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);

        assertEquals(2, buf.data().get(0));
        assertEquals(4, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
    }

    /**
     * This tests the encodeFilterListComplete() method. It contains two test cases.<BR>
     * 
     * 1. Positive test for everything good with success flag true<BR>
     * 2. Positive test for everything good with success flag false<BR>
     */
    @Test
    public void encodeFilterListCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(32));
        FilterList filterList = CodecFactory.createFilterList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        filterList.applyHasPerEntryPermData();
        filterList.applyHasTotalCountHint();
        filterList.totalCountHint(5);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeFilterListCompleteTest");
        ((FilterListImpl)filterList).encodedEntries(txt);
        filterList.containerType(DataTypes.FIELD_LIST);                
        
        // 1. Positive test for everything good with success flag true
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterList.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        assertEquals(2, buf.data().get(0));
        assertEquals(4, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(0, buf.data().get(3));
        
        // 2. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterList.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        assertEquals(0, buf.data().position());
    }

    /**
     * This tests the encodeFilterEntryInit() method. It contains five test cases.<BR>
     * 
     * 1. Negative test for buffer too small to set id<BR>
     * 2. Negative test for buffer too small to set data format<BR>
     * 3. Positive test branch where filterList has container type<BR>
     *    and filter entry does not<BR>
     * 4. Positive test branch where filterList has container type<BR>
     *    and filter entry has container type and action different then Clear<BR>
     * 5. Positive test branch where filterList does not have container type<BR>
     */
    @Test
    public void encodeFilterEntryInitTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(6));
        FilterList filterList = CodecFactory.createFilterList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
     
        filterList.applyHasPerEntryPermData();
        filterList.applyHasTotalCountHint();
        filterList.totalCountHint(5);
        filterList.containerType(DataTypes.FIELD_LIST);
        
        FilterEntry entry = CodecFactory.createFilterEntry();
        entry.id(2);

        // 1. Buffer too small to set id
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.BUFFER_TOO_SMALL);  
        
        // 2. Buffer too small to set data format
        
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(6));
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.BUFFER_TOO_SMALL);                  
                
        // 3. Positive test case of
        // filterList._containerType != RsslDataTypes.NO_DATA) && 
        // !((entry._flags & RsslFilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(9));
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.SUCCESS);
        
        assertEquals(2, buf.data().get(0));
        assertEquals(4, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(2, buf.data().get(5));

        // 4. Positive test case of
        // (entry._containerType != RsslDataTypes.NO_DATA) && 
        // ((entry._flags & RsslFilterEntryFlags.HAS_CONTAINER_TYPE) > 0)) &&
		// (entry._action != RsslFilterEntryActions.CLEAR)        
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10));
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        entry.applyHasContainerType();
        entry.containerType(DataTypes.MAP);
        entry.action(FilterEntryActions.SET);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.SUCCESS);
        
        assertEquals(2, buf.data().get(0));
        assertEquals(4, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(0, buf.data().get(3));
        assertEquals(34, buf.data().get(4));
        assertEquals(2, buf.data().get(5));
        assertEquals(9, buf.data().get(6));
        
        // 5. Positive test case of ~3 & ~4
        filterList.containerType(DataTypes.NO_DATA);
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10));
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        entry.applyHasContainerType();
        entry.containerType(DataTypes.MAP);
        entry.action(FilterEntryActions.SET);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.SUCCESS);
        
        assertEquals(2, buf.data().get(0));
        assertEquals(0, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(0, buf.data().get(3));
        assertEquals(34, buf.data().get(4));
        assertEquals(2, buf.data().get(5));
        assertEquals(9, buf.data().get(6));
        
        
        // 6. Buffer too small when permData is specified.
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(9));
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeInit(iter));
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.allocate(20));
        entry.applyHasPermData();
        entry.permData(permData);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, entry.encodeInit(iter, 100));
        
        filterList.containerType(DataTypes.NO_DATA);
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10));
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        entry.clear();
        entry.id(2);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.SUCCESS);
        
        assertEquals(2, buf.data().get(0));
        assertEquals(0, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(0, buf.data().get(3));
        assertEquals(0, buf.data().get(4));
        assertEquals(2, buf.data().get(5));
        assertEquals(0, buf.data().get(6));
    }

    /**
     * This tests the encodeFilterEntry() method. It contains four test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good without permData<BR>
     * 3. Positive test for everything good without permData but permData flag set<BR>
     * 4. Positive test for everything good with permData<BR>
     */
    @Test
    public void encodeFilterEntryTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(6));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        filterList.applyHasPerEntryPermData();
        filterList.applyHasTotalCountHint();
        filterList.totalCountHint(5);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeFilterEntryTest");
        ((FilterListImpl)filterList).encodedEntries(txt);
        filterList.containerType(DataTypes.FIELD_LIST);
        
        filterEntry.applyHasContainerType();
        filterEntry.action(FilterEntryActions.SET);
        filterEntry.containerType(DataTypes.FIELD_LIST);
        filterEntry.id(1);
        txt.data("encodeFilterEntryTest3");
        filterEntry.encodedData(txt);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());

        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterEntry.encode(iter) == CodecReturnCodes.BUFFER_TOO_SMALL);

        int ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(smallBuf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = filterEntry.encode(iter);
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);  
        assertTrue(filterList.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);
        
        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        // 2. Positive test for everything good without permData
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterEntry.encode(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterList.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedDataNoPermData =
            {0x02, 0x04, 0x05, 0x01, 0x22, 0x01, 0x04, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64,
             0x65, 0x46, 0x69, 0x6c, 0x74, 0x65, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54,
             0x65, 0x73, 0x74, 0x33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        
        bufBytes = new byte[expectedDataNoPermData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedDataNoPermData, bufBytes);
        
        // 3. Positive test for everything good without permData but permData flag set
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        filterEntry.applyHasPermData();
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterEntry.encode(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterList.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedDataNoPermDataButFlagSet =
            {0x03, 0x04, 0x05, 0x01, 0x32, 0x01, 0x04, 0x00, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64,
             0x65, 0x46, 0x69, 0x6c, 0x74, 0x65, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65,
             0x73, 0x74, 0x33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedDataNoPermDataButFlagSet.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedDataNoPermDataButFlagSet, bufBytes);
        
        // 4. Positive test for everything good with permData
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        filterEntry.applyHasPermData();
        txt.data("encodeFilterEntryTest2");
        filterEntry.permData(txt);
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterEntry.encode(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(filterList.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedDataPermData =
            {3, 4, 5, 1, 50, 1, 4, 22, 101, 110, 99, 111, 100, 101, 70, 105, 108, 116, 101, 114,
             69, 110, 116, 114, 121, 84, 101, 115, 116, 50, 22, 101, 110, 99, 111, 100, 101, 70,
             105, 108, 116, 101, 114, 69, 110, 116, 114, 121, 84, 101, 115, 116, 51, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedDataPermData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedDataPermData, bufBytes);
    }
    
    /**
     * This tests the encodeFilterEntryComplete() method. It contains two test cases.<BR>
     * 
     * 1. Positive test for everything good with success flag true<BR>
     * 2. Positive test for everything good with success flag false<BR>
     */
    @Test
    public void encodeFilterEntryCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(100));
        FilterList filterList = CodecFactory.createFilterList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        FilterEntry entry = CodecFactory.createFilterEntry();

        // 1. introduce an error into the RsslEncodeSizeMark in order to execute
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        filterList.containerType(DataTypes.FIELD_LIST);
        assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeInit(iter));
        EncodingLevel levelInfo = ((EncodeIteratorImpl)iter)._levelInfo[((EncodeIteratorImpl)iter)._encodingLevel];
        int origPos = levelInfo._internalMark._sizePos;
        levelInfo._internalMark._sizePos = 99;
        assertEquals(CodecReturnCodes.INVALID_DATA, entry.encodeComplete(iter, true));
        levelInfo._internalMark._sizePos = origPos;
        
        
        filterList.applyHasPerEntryPermData();
        filterList.applyHasTotalCountHint();
        filterList.totalCountHint(5);
        
        // 2. a simple no data test with success flag true
        filterList.containerType(DataTypes.FIELD_LIST);
        entry.id(2);
        buf.data().clear();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);
               
        assertEquals(2, buf.data().get(0));
        assertEquals(4, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(2, buf.data().get(5));
        
        // 3. a simple no data test with success flag false
        buf.data().clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeInit(iter, 100) == CodecReturnCodes.SUCCESS);
        assertTrue(entry.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
               
        assertEquals(2, buf.data().get(0));
        assertEquals(4, buf.data().get(1));
        assertEquals(5, buf.data().get(2));
        assertEquals(2, buf.data().get(5));
        
        // 4. cause RsslEncoders.finishU16Mark() to fail in order to execute
               ((EncodeIteratorImpl)iter)._curBufPos = 1000;
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, entry.encodeComplete(iter, true));
    }

    /**
     * This tests the encodeMapInit() method. It contains eleven test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Negative test for unsupported container type<BR>
     * 3. Negative test for unsupported key primitive type<BR>
     * 4. Negative test for buffer too small without summaryData but flag set<BR>
     * 5. Positive test for everything good without summaryData but flag set
     * 6. Negative test for buffer too small without setDefs but flag set
     * 7. Positive test for everything good without setDefs but flag set<BR>
     * 8. Positive test for everything good<BR>
     * 9. Positive test for everything good and summary data complete with success true<BR>
     * 10.Positive test for everything good and summary data complete with success false<BR>
     * 11.Negative test for buffer too small and summary data complete with success true<BR>
     * 11.Negative test for buffer that is big enough for map header but not summary data<BR>
     */
    @Test
    public void encodeMapInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(1));
        Buffer mediumBuf = CodecFactory.createBuffer();
        mediumBuf.data(ByteBuffer.allocate(6));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Buffer summaryDataSmallBuf = CodecFactory.createBuffer();
        summaryDataSmallBuf.data(ByteBuffer.allocate(26));
        Map map = CodecFactory.createMap();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
     
        map.applyHasKeyFieldId();
        map.applyHasPerEntryPermData();
        map.applyHasTotalCountHint();
        map.totalCountHint(5);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeMapInitTest");
        ((MapImpl)map).encodedEntries(txt);
        map.containerType(DataTypes.FIELD_LIST);
        map.keyPrimitiveType(DataTypes.INT);
        map.keyFieldId(22);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        // 2. Negative test for unsupported container type
        ((MapImpl)map)._containerType = DataTypes.ARRAY;
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.UNSUPPORTED_DATA_TYPE);
        
        // reset container type
        map.containerType(DataTypes.FIELD_LIST);
        
        // 3. Negative test for unsupported key primitive type
        ((MapImpl)map)._keyPrimitiveType = DataTypes.INT_1;
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.UNSUPPORTED_DATA_TYPE);
        
        // reset key primitive type
        map.keyPrimitiveType(DataTypes.INT);
        
        // 4. Negative test for buffer too small without summaryData but flag set
        iter.setBufferAndRWFVersion(mediumBuf, Codec.majorVersion(), Codec.minorVersion());
        map.applyHasSummaryData();
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // 5. Positive test for everything good without summaryData but flag set
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        
        assertEquals(0x1A, buf.data().get(0));
        assertEquals(0x03, buf.data().get(1));
        assertEquals(0x04, buf.data().get(2));
        assertEquals(0x00, buf.data().get(3));
        assertEquals(0x16, buf.data().get(4));
        
        // 6. Negative test for buffer too small without setDefs but flag set
        mediumBuf.data().rewind();
        iter.setBufferAndRWFVersion(mediumBuf, Codec.majorVersion(), Codec.minorVersion());
        map.applyHasSetDefs();
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);

        // 7. Positive test for everything good without setDefs but flag set
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        
        assertEquals(0x1B, buf.data().get(0));
        assertEquals(0x03, buf.data().get(1));
        assertEquals(0x04, buf.data().get(2));
        assertEquals(0x00, buf.data().get(3));
        assertEquals(0x16, buf.data().get(4));

        // 8. Positive test for everything good
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        txt.data("encodeMapInitTest");
        map.encodedSetDefs(txt);
        txt.data("encodeMapInitTest");
        map.encodedSummaryData(txt);
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{27, 3, 4, 0, 22, 17, 101, 110, 99, 111,
        	 100, 101, 77, 97, 112, 73, 110, 105, 116, 84,
        	 101, 115, 116, 17, 101, 110, 99, 111, 100, 101,
        	 77, 97, 112, 73, 110, 105, 116, 84, 101, 115,
        	 116, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        
        // 9. Positive test for everything good and summary data complete with success true
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 64 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        map.encodedSummaryData().clear();
        map.applyHasSummaryData();
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeSummaryDataComplete(iter, true) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData2 =
        	{27, 3, 4, 0, 22, 17, 101, 110, 99, 111, 100, 101, 77, 97, 112, 73,
        	 110, 105, 116, 84, 101, 115, 116, (byte)128, 0, 5, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes2 = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes2));
        
        assertEquals(28, buf.data().position());
        assertArrayEquals(expectedData2, bufBytes2);

        // 10. Positive test for everything good and summary data complete with success false
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 64 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeSummaryDataComplete(iter, false) == CodecReturnCodes.SUCCESS);

        byte[] expectedData3 =
        	{27, 3, 4, 0, 22, 17, 101, 110, 99, 111, 100, 101, 77, 97, 112, 73,
        	 110, 105, 116, 84, 101, 115, 116, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes3 = new byte[expectedData3.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes3));
        
        // Should be at start of summary data encoding.
        assertEquals(25, buf.data().position());
        assertArrayEquals(expectedData3, bufBytes3);
        
        // 11. Negative test for buffer too small and summary data complete with success true
        iter.setBufferAndRWFVersion(summaryDataSmallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);
}

    /**
     * This tests the encodeMapComplete() method. It contains two test cases.<BR>
     * 
     * 1. Positive test for everything good with success flag true<BR>
     * 2. Positive test for everything good with success flag false<BR>
     */
    @Test
    public void encodeMapCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Map map = CodecFactory.createMap();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        map.applyHasKeyFieldId();
        map.applyHasPerEntryPermData();
        map.applyHasTotalCountHint();
        map.applyHasSummaryData();
        map.applyHasSetDefs();
        map.totalCountHint(5);
        txt.data("encodeMapCompleteTest");
        ((MapImpl)map).encodedEntries(txt);
        txt.data("encodeMapCompleteTest");
        map.encodedSetDefs(txt);
        map.encodedSummaryData(txt);
        map.containerType(DataTypes.FIELD_LIST);                
        map.keyPrimitiveType(DataTypes.INT);
        map.keyFieldId(22);
        
        // 1. Positive test for everything good with success flag true
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x15, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54,
        	 0x65, 0x73, 0x74, 0x15, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61,
        	 0x70, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73,
        	 0x74, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        
        // 2. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        assertEquals(0, buf.data().position());
    }
    
    /**
     * This tests the encodeMapEntryInit() method. It contains five test cases.<BR>
     * 
     * 1. Positive test for everything good without permData but flag set<BR>
     * 2. Negative test for buffer too small<BR>
     * 3. Positive test for everything good with DELETE entry<BR>
     * 4. Positive test for everything good without pre-encoded data<BR>
     * 5. Positive test for everything good with pre-encoded data<BR>
     */
    @Test
    public void encodeMapEntryInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(64));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        Int intKey = CodecFactory.createInt();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer txt = CodecFactory.createBuffer();
     
        map.applyHasKeyFieldId();
        map.applyHasPerEntryPermData();
        map.applyHasTotalCountHint();
        map.applyHasSummaryData();
        map.applyHasSetDefs();
        map.totalCountHint(5);
        txt.data("encodeMapEntryInitTest");
        ((MapImpl)map).encodedEntries(txt);
        map.encodedSetDefs(txt);
        map.encodedSummaryData(txt);
        map.containerType(DataTypes.FIELD_LIST);                
        map.keyPrimitiveType(DataTypes.INT);
        map.keyFieldId(22);
        mapEntry.applyHasPermData();
        mapEntry.action(MapEntryActions.ADD);
        intKey.value(33);
                        
        // 1. Positive test for everything good without permData but flag set
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
        	 0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
        	 0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x00, 0x01, 0x21, 0x00, 0x00,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        
        // 2. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.permData(txt);
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);

        // 3. Positive test for everything good with DELETE entry
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.action(MapEntryActions.DELETE);
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData2 =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
        	 0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
        	 0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x13, 0x16, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x01, 0x21, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData2, bufBytes);

        // 4. Positive test for everything good without pre-encoded data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.action(MapEntryActions.ADD);
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData3 =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
        	 0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
        	 0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x16, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x01, 0x21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData3.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData3, bufBytes);

        // 5. Positive test for everything good with pre-encoded data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        mapEntry.encodedKey(txt);
        assertTrue(mapEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData4 =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
        	 0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
        	 0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x16, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64,
        	 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69,
        	 0x74, 0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData4.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData4, bufBytes);
    }

    /**
     * This tests the encodeMapEntryComplete() method. It contains three test cases.<BR>
     * 
     * 1. Positive test for everything good with success flag true<BR>
     * 2. Positive test for everything good with success flag false<BR>
     * 3. Positive test for everything good with DELETE entry<BR>
     */
    @Test
    public void encodeMapEntryCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Buffer buf2 = CodecFactory.createBuffer();
        buf2.data(ByteBuffer.allocate(128));
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        Int intKey = CodecFactory.createInt();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        map.applyHasKeyFieldId();
        map.applyHasPerEntryPermData();
        map.applyHasTotalCountHint();
        map.applyHasSummaryData();
        map.applyHasSetDefs();
        map.totalCountHint(5);
        txt.data("encodeMapEntryComplete");
        ((MapImpl)map).encodedEntries(txt);
        map.encodedSetDefs(txt);
        map.encodedSummaryData(txt);
        map.containerType(DataTypes.FIELD_LIST);                
        map.keyPrimitiveType(DataTypes.INT);
        map.keyFieldId(22);
        mapEntry.applyHasPermData();
        mapEntry.permData(txt);
        mapEntry.action(MapEntryActions.ADD);
        intKey.value(33);
        
        // 1. Positive test for everything good with success flag true
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70,
        	 0x6c, 0x65, 0x74, 0x65, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c,
        	 0x65, 0x74, 0x65, 0x05, 0x00, 0x00, 0x12, 0x16, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
        	 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x01, 0x21, (byte)0xfe, 0x00, 0x00,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        
        // 2. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData2 =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70,
        	 0x6c, 0x65, 0x74, 0x65, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c,
        	 0x65, 0x74, 0x65, 0x05, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        // compare only first 54 bytes here even though more was encoded
        // success = false moves position back to 54
        assertEquals(54, buf.data().position());
        for (int i = 0; i < 54; i++)
        {
        	assertTrue(bufBytes[i] == expectedData2[i]);
        }

        // 3. Positive test for everything good with DELETE entry
        iter.setBufferAndRWFVersion(buf2, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.action(MapEntryActions.DELETE);
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeInit(iter, intKey, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData3 =
        	{0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70,
        	 0x6c, 0x65, 0x74, 0x65, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
        	 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c,
        	 0x65, 0x74, 0x65, 0x05, 0x00, 0x00, 0x13, 0x16, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
        	 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x01, 0x21, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData3.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf2.copy(bufBytes));

        assertArrayEquals(expectedData3, bufBytes);
    }
    
    /**
     * This tests the encodeMapEntry() method. It contains four test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good without permData<BR>
     * 3. Positive test for everything good without permData but permData flag set<BR>
     * 4. Positive test for everything good with permData<BR>
     */
    @Test
    public void encodeMapEntryTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(49));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Int intKey = CodecFactory.createInt();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeMapEntryTest");
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        map.applyHasKeyFieldId();
        map.applyHasPerEntryPermData();
        map.applyHasTotalCountHint();
        map.applyHasSummaryData();
        map.applyHasSetDefs();
        map.totalCountHint(5);
        ((MapImpl)map).encodedEntries(txt);
        map.encodedSetDefs(txt);
        map.encodedSummaryData(txt);
        map.containerType(DataTypes.FIELD_LIST);                
        map.keyPrimitiveType(DataTypes.INT);
        map.keyFieldId(22);
        mapEntry.action(MapEntryActions.ADD);
        mapEntry.encodedData().data(ByteBuffer.allocate(0));
        intKey.value(33);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());

        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encode(iter, intKey) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        int ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(smallBuf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = mapEntry.encode(iter, intKey);        
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);  
   
        
        assertTrue(mapEntry.encodeComplete(iter, true) == CodecReturnCodes.INVALID_DATA);

        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        // 2. Positive test for everything good without permData
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encode(iter, intKey) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedDataNoPermData =
            {0x1b, 0x03, 0x04, 0x00, 0x16, 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74,
        	 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e,
        	 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x01, 0x02, 0x01,
        	 0x21, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedDataNoPermData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedDataNoPermData, bufBytes);
        
        // 3. Positive test for everything good without permData but permData flag set
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.applyHasPermData();
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encode(iter, intKey) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedDataNoPermDataButFlagSet =
            {0x1f, 0x03, 0x04, 0x00, 0x16, 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74,
        	 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e,
        	 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x01, 0x12, 0x00,
        	 0x01, 0x21, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedDataNoPermDataButFlagSet.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedDataNoPermDataButFlagSet, bufBytes);
        
        // 4. Positive test for everything good with permData
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.applyHasPermData();
        mapEntry.permData(txt);
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encode(iter, intKey) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedDataPermData =
            {0x1f, 0x03, 0x04, 0x00, 0x16, 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
        	 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74,
        	 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e,
        	 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x01, 0x12, 0x12,
        	 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74,
        	 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x01, 0x21, 0x00, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedDataPermData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedDataPermData, bufBytes);
        
        // 5. Positive test to encode a Entry Key as ASCII to excercise RsslEncoders.encBuffer with RsslEncodeIteratorStates.PRIMITIVE_U15.
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        mapEntry.clear();
        map.keyPrimitiveType(DataTypes.ASCII_STRING);
        mapEntry.action(MapEntryActions.ADD);
        mapEntry.encodedData().data(ByteBuffer.allocate(0));
        Buffer asciiKey = CodecFactory.createBuffer();
        asciiKey.data("test ascii key");
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encode(iter, asciiKey) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);
    }
    
    /**
     * This tests the encodeElementListInit method.
     * <br>
     * <ol>
     * <li>Negative case with Buffer to small.</li> 
     * <li>Negative case with Buffer to small with info specified.</li>
     * <li>Positive case with Info specified. Encoded bytes verified.</li>
     * <li>Negative test with SetData BUFFER_TO_SMALL</li>
     * <li>Negative test with SetData with negative set id</li>
     * <li>Negative test with SetData, setId, but no setDef</li>
     * <li>Negative test with SetData, no setId, no setDef</li>
     * <li>Positive test with setData, with an empty setDef</li>
     * <li>Positive test with setData, setId and with a setDef with zero entries.</li>
     * <li>Positive test with setData, setId and with a setDef with one entry.</li>
     * <li>Positive test with setDef and standard data</li>
     * <li>Positive test with setDef with count=0 and standard data</li>
     * <li>Negative test with setDef with count=1, standard data and encodedSetData too large, BUFFER_TO_SMALL.</li>
     * <li>Positive test with setDef with count=1, standard data and encodedSetData</li>
     * <li>Positive test with setdef count=1, w/o standard data, w/ encodedSetData.</li>
     * <li>Negative test with setdef count=1, w/o standard data, w/ encodedSetData, BUFFER_TO_SMALL.</li>
     * <li>Negative test with standard data, buffer_TO_SMALL.</li>
     * <li>Positive test with standard data. Encoded bytes verified.</li>
     * </ol>
     */
    @Test
    public void encodeElementListInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(2));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(32));
        ElementList elementList = CodecFactory.createElementList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. Neg test BUFFER_TO_SMALL
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(iter, null, 2));
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 2));
        
        // 2. Neg test BUFFER_TO_SMALL with Info
        elementList.applyHasInfo();
        elementList.elementListNum(1);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(iter, null, 2));
        
        // 3. test with Info (which was previously set)
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        
        // verification data taken from UPAC DataTest
        assertEquals(1, buf.data().get(0));
        assertEquals(2, buf.data().get(1));
        assertEquals(0, buf.data().get(2));
        assertEquals(1, buf.data().get(3));
        
        // 4. Neg test with SetData BUFFER_TO_SMALL
        smallBuf.data().rewind();
        elementList.clear();
        elementList.applyHasSetData();
        iter.clear();
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(iter, null, 2));
        
        // 6. Neg test with SetData, setId, but no setDef
        buf.data().rewind();
        elementList.clear();
        elementList.applyHasSetData();
        elementList.applyHasSetId();
        elementList.setId(0);
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SET_DEF_NOT_PROVIDED, elementList.encodeInit(iter, null, 32));
        
        // 7. Neg test with SetData, no setId, no setDef
        buf.data().rewind();
        elementList.clear();
        elementList.applyHasSetData();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SET_DEF_NOT_PROVIDED, elementList.encodeInit(iter, null, 32));
        
        // 8. Neg test with setData, with an empty setDef
        buf.data().rewind();
        iter.clear();
        LocalElementSetDefDb elementSetDefDb = CodecFactory.createLocalElementSetDefDb();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SET_DEF_NOT_PROVIDED, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 9. test with setData, setId and with a setDef with zero entries.
        ElementSetDef elementSetDef = CodecFactory.createElementSetDef();
        elementSetDefDb.definitions()[0].count(elementSetDef.count());
        elementSetDefDb.definitions()[0].setId(elementSetDef.setId());
        if (elementSetDef.entries() != null)
        {
        	elementSetDefDb.definitions()[0].entries(elementSetDef.entries());
        }
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));

        // 10. test with setData, setId and with a setDef with one entry.
        elementSetDefDb.definitions()[0].count(1);
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));

        // 11. test with setDef and standard data
        elementList.applyHasStandardData();
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 12. test with setDef with count=0 and standard data
        ((ElementSetDefImpl)elementSetDefDb.definitions()[0])._count = 0;
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 13. neg test with setDef with count=1, standard data and encodedSetData too large, BUFFER_TO_SMALL.
        elementSetDefDb.definitions()[0].count(1);
        Buffer encodedSetData = CodecFactory.createBuffer();
        encodedSetData.data(ByteBuffer.allocate(40), 0, 30);
        elementList.encodedSetData(encodedSetData);
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 14. test with setDef with count=1, standard data and encodedSetData 
        encodedSetData.data(encodedSetData.data(), 0, 10);
        elementList.encodedSetData(encodedSetData);
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 15. test with setdef count=1, w/o standard data, w/ encodedSetData.
        ((ElementListImpl)elementList)._flags &= ~ElementListFlags.HAS_STANDARD_DATA;
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 16. Neg test with setdef count=1, w/o standard data, w/ encodedSetData, BUFFER_TO_SMALL.
        encodedSetData.data().compact(); // reset everything for this test
        assertEquals(CodecReturnCodes.SUCCESS, encodedSetData.data(encodedSetData.data(), 0, 40));
        elementList.encodedSetData(encodedSetData);
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(iter, elementSetDefDb, 40));
       
        // 17. neg test with standard data, buffer_TO_SMALL.
        elementList.clear();
        elementList.applyHasStandardData();
        smallBuf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // 18. test with standard data.
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, elementSetDefDb, 32));
        
        // verification data taken from UPAC DataTest
        assertEquals(0x08, buf.data().get(0));
        // Note that bytes 1 and 2 are skipped and not written, so they are not checked.
    }
    
    /**
     * This tests the encodeElementListComplete method.
     * <br>
     * <ol>
     * <li>Positive case with standard data with boolean true. Encoded bytes verified.</li>
     * <li>Positive case with standard data with boolean false. Encoded bytes verified.</li>
     * </ol> 
     */
    @Test
    public void encodeElementListCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(32));
        ElementList elementList = CodecFactory.createElementList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. Positive case with standard data with boolean true.
        elementList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        
        // verification data taken from UPAC DataTest
        assertEquals(0x08, buf.data().get(0));
        // Note that bytes 1 and 2 are skipped and not written, so they are not checked.
        
        // 2. Positive case with standard data with boolean false.
        iter.clear();
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, false)); // rollback
        
        // verification data taken from UPAC DataTest
        assertEquals(0x08, buf.data().get(0));
        // Note that bytes 1 and 2 are skipped and not written, so they are not checked.
    }
    
    /**
     * This tests the encodeElementEntry method.
     * <br>
     * <ol>
     * <li>Positive case - encode one element entry into an element list. Encoded bytes verified.</li>
     * <li>Negative case - encode one element entry into an element list with BUFFER_TOO_SMALL.</li>
     * <li>Negative case - encode one element entry into an element list with entry.name too big.</li>
     * <li>Negative case - encode one preencoded element entry into an element list.</li>
     * <li>Positive case - encode one preencoded element entry into an element list. Encoded bytes verified.</li>
     * <li>Positive case - encode one empty preencoded element entry into an element list. Encoded bytes verified.</li>
     * <li>Positive Case - verify encDate (and blank date) EncTime and encEnum. Encoded bytes verified.</li>
     * <li>Negative case - encode one empty preencoded element entry into an element list.</li>
     * </ol>
     */
    @Test
    public void encodeElementEntryTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(32));
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. Positive case - encode one element entry into an element list.
        elementList.applyHasStandardData();
        elementList.applyHasInfo();
        elementList.elementListNum(29731); // 0x7423
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        
        Buffer name = CodecFactory.createBuffer();
        name.data("uint");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.UINT);
        UInt uint = CodecFactory.createUInt();
        uint.value(1234);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, uint));

        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        
        byte[] expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/001_elementList_one_entry.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 2. Neg case - encode one element entry into an element list with BUFFER_TOO_SMALL.
        elementList.clear();
        elementList.applyHasStandardData();
        buf.data().rewind();
        buf.data(buf.data(), 0, 8);
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(iter, uint));
        
        int ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(buf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = elementEntry.encode(iter, uint);       
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);        
        
        // 3. Neg case - encode one element entry into an element list with entry.name too big.
        buf.data().rewind();
        buf.data(buf.data(), 0, 10);
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        name.data("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // over 0x80
        elementEntry.name(name);
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(iter, uint));
        
        ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(buf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = elementEntry.encode(iter, uint);   
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);        
        
        // 4. Neg case - encode one preencoded element entry into an element list.
        Buffer bigBuf = CodecFactory.createBuffer();
        bigBuf.data(ByteBuffer.allocate(300));
        iter.clear();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        Buffer preencoded = CodecFactory.createBuffer();
        preencoded.data("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // len=260
        elementEntry.encodedData(preencoded);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(iter));
        
        ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(bigBuf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = elementEntry.encode(iter);  
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);        
        
        // 5. Positive case - encode one preencoded element entry into an element list.
        bigBuf.data(bigBuf.data(), 0, bigBuf.data().position());        
        bigBuf.data().rewind();
        name.data("ABCDEFGHIJK");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ASCII_STRING);
        iter.clear();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 300));
        elementEntry.encodedData(preencoded);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));

        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/003_elementList_oneEntry_preencodedAscii.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(bigBuf.data()));
        
        // 6. Positive case - encode one empty preencoded element entry into an element list.
        bigBuf.data(bigBuf.data(), 0, bigBuf.data().position());
        bigBuf.data().rewind();
        name.data("ABCDEFGHIJK");
        elementEntry.name(name);
        preencoded.clear();
        iter.clear();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 300));
        elementEntry.encodedData(preencoded);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));

        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/002_elementList_oneEntry_emptyPreEncodedAscii.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(bigBuf.data()));
        
        // 7. Positive Case - verify encDate (and blank date) EncTime and encEnum
        bigBuf.data().rewind();
        bigBuf.data().limit(bigBuf.data().capacity());
        bigBuf.data(bigBuf.data(), 0, 300);
        iter.clear();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 300));
        // test RsslEncoders.encDate
        name.data("Date test");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.DATE);
        Date date = CodecFactory.createDate();
        date.day(23);
        date.month(5);
        date.year(2012);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, date));
        // test RsslEncoders.encDate blank
        name.data("Blank Date test");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.DATE);
        date = CodecFactory.createDate();
        date.blank();
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, date));
        // test RsslEncoders.encTime
        name.data("Time test");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.TIME);
        Time time = CodecFactory.createTime();
        time.hour(11);
        time.minute(30);
        time.second(15);
        time.millisecond(7);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, time));
        // test RsslEncoders.encTime blank
        name.data("Blank Time test");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.TIME);
        time = CodecFactory.createTime();
        time.blank();
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, time));
        //test RsslEncoders.encEum
        name.data("Enum test");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ENUM);
        Enum enumeration = CodecFactory.createEnum();
        enumeration.value(127);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, enumeration));
        // encode element list complete
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
     
        // verify encoded data against UPAC.
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/007_elementList_entries_wdate_time_enum.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(bigBuf.data()));
        
        // 8. Neg case - encode one empty preencoded element entry into an element list.
        bigBuf.data().rewind();
        bigBuf.data(bigBuf.data(), 0, 32);
        name.data("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // over 0x80
        elementEntry.name(name);
        preencoded.clear();
        iter.clear();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 32));
        elementEntry.encodedData(preencoded);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(iter));
        
        ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(buf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = elementEntry.encode(iter);     
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);           
        
    }
    
    /**
     * This tests the encodeStatusMsg() method. It contains four test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good without permData<BR>
     * 3. Positive test for everything good without permData but permData flag set<BR>
     * 4. Positive test for everything good with permData<BR>
     */
    @Test
    public void encodeStatusMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(11));
        StatusMsg msg = (StatusMsg) CodecFactory.createMsg();
        msg.domainType(4);
        msg.msgClass(MsgClasses.STATUS);
        msg.streamId(24);
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
             
       // 1. None of the data is set
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[12];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        byte[] expectedData =
              {0, 8, 3, 4, 0, 0, 0, 24, 0, 0, 0, 0};       
        assertArrayEquals(expectedData, bufBytes); 
        
        // 2. Has state and buffer too small
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.applyHasState();
        msg.state().code(StateCodes.TOO_MANY_ITEMS);
        msg.state().dataState(DataStates.SUSPECT);
        msg.state().streamState(StreamStates.CLOSED_RECOVER);
        Buffer textBuffer = CodecFactory.createBuffer();
        textBuffer.data("encodeStateMsgTest");
        msg.state().text(textBuffer);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encode(iter));
        
        // 3. Has group and buffer too small
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasGroupId();
        byte [] gib = {11, 123, 8, 3, 76, 2};
        Buffer groupId = CodecFactory.createBuffer();
        groupId.data(ByteBuffer.wrap(gib));
        msg.groupId(groupId);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encode(iter));

        // 4. Has perm data and buffer too small
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasPermData();
        byte [] pb = {10, 5, 3, 9};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(pb));
        msg.permData(permData);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encode(iter));

        // 4. Has key and buffer too small
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasMsgKey();
        msg.msgKey().applyHasFilter();
        msg.msgKey().filter(7);        
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encode(iter));


        // 5. Has ext header and buffer too small
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasExtendedHdr();
        byte [] ehb = {67, 1, 2, 3, 4, 5, 6, 7, 8, 9, 67};
        Buffer extHeader = CodecFactory.createBuffer();
        extHeader.data(ByteBuffer.wrap(ehb));
        msg.extendedHeader(extHeader);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encode(iter));

        // 5. Has post user info and buffer too small
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasPostUserInfo();
        long userAddr = 1234L;
        msg.postUserInfo().userAddr(userAddr);
        long userId = 567L;        
        msg.postUserInfo().userId(userId);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encode(iter));

        // 4. Has state and ok
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(31));
        msg.flags(0);
        msg.applyHasState();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[32];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
                
        byte[] expectedData1 =
              {0, 29, 3, 4, 0, 0, 0, 24, 32, 0, 26, 13, 18, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
        		0x74,  0x61, 0x74, 0x65, 0x4D, 0x73, 0x67, 0x54, 0x65, 0x73, 0x74, 0};       
        assertArrayEquals(expectedData1, bufBytes); 
        
        // 5. Has group id and ok
        clearBuffer(buf.data());
        msg.flags(0);
        msg.applyHasGroupId();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[31];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        byte[] expectedData2 =
        	{0, 15, 3, 4, 0, 0, 0, 24, 16, 0, 6, 11, 123, 8, 3, 76, 2};  
        assertArrayEquals(expectedData2, Arrays.copyOf(bufBytes, expectedData2.length)); 

        // 6. Has perm data and ok
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasPermData();
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[31];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        byte[] expectedData3 =
        	{0, 13, 3, 4, 0, 0, 0, 24, 2, 0, 4, 10, 5, 3, 9};  
        assertArrayEquals(expectedData3, Arrays.copyOf(bufBytes, expectedData3.length)); 

        // 7. Has key and ok
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasMsgKey();
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[31];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        byte[] expectedData4 =
        	{0, 15, 3, 4, 0, 0, 0, 24, 8, 0, -128, 5, 8, 0, 0, 0, 7};  
        assertArrayEquals(expectedData4, Arrays.copyOf(bufBytes, expectedData4.length)); 

        // 8. Has ext header and ok
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasExtendedHdr();
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[31];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        byte[] expectedData5 =
        	{0, 20, 3, 4, 0, 0, 0, 24, 1, 0, 11, 67, 1, 2, 3, 4, 5, 6, 7, 8, 9, 67};  
        assertArrayEquals(expectedData5, Arrays.copyOf(bufBytes, expectedData5.length)); 

        // 9. Has post user info and ok
        clearBuffer(buf.data());
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.flags(0);
        msg.applyHasPostUserInfo();
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[31];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        byte[] expectedData6 =
        	{0, 17, 3, 4, 0, 0, 0, 24, -127, 0, 0, 0, 0, 4, -46, 0, 0, 2, 55 };  
        assertArrayEquals(expectedData6, Arrays.copyOf(bufBytes, expectedData6.length)); 

        // 10. Has all
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(71));
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        msg.applyHasPostUserInfo();
        msg.applyHasExtendedHdr();
        msg.applyHasGroupId();
        msg.applyHasMsgKey();
        msg.applyHasPermData();
        msg.applyHasState();
        msg.applyClearCache();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        bufBytes = new byte[71];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        byte[] expectedData7 =
        	{0, 0x45, 0x03, 0x04, 0, 0, 0, 0x18, 
        		-0x7f, 0x7b, 0, 
        		0x1a, 0x0d, 0x12, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
        		0x74,  0x61, 0x74, 0x65, 0x4D, 0x73, 0x67, 0x54, 0x65, 0x73, 0x74, 
        		0x06, 0x0b, 0x7b, 0x08, 0x03, 0x4c, 0x02, 
        		0x04, 0x0a, 0x05, 0x03, 0x09,
        		-0x80, 0x05, 0x08, 0, 0, 0, 0x07, 
        		0x0b, 0x43, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x43, 
        		0, 0, 0x04, -0x2e, 0, 0, 0x02, 0x37};  
        assertArrayEquals(expectedData7, Arrays.copyOf(bufBytes, expectedData7.length)); 

        
    }  
    
    private void clearBuffer(ByteBuffer buffer)
    {
    	buffer.rewind();
    	for (int i = 0; i< buffer.capacity(); i++)
    	{
    		buffer.put((byte) 0);
    	}
    	buffer.rewind();
    }

    /**
     * This tests the encodeElementEntryInit and encodeElementEntryComplete method.
     * <br>
     * <ol>
     * <li>Positive case - encode three element entry into an element list. Middle element is an element list. Encoded bytes verified.</li>
     * <li>Negative case - test elementEntryInit() with an element name that is too big (BUFFER_TO_SMALL)</li>
     * <li>Positive case - test elementEntryInit() with entry with a container of NO_DATA.</li>
     * <li>Positive case - test encodeElementEntryComplete with false. Encoded bytes verified.</li>
     * <li>Negative case - test encodeElementEntryComplete with internalMerk._sizeBytes > 0.</li>
     * </ol>
     */
    @Test
    public void encodeElementEntryInitCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(105));
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        /* 1. Positive case - encode three element entry into an element list. 
         * Middle element is an element list. Encoded bytes verified.
         *    The first entry is a UINT. 
         *    The second entry is an element list with an ASCII string as an element.
         *    The third entry is a UINT.
         *      ElementListInit
         *        entry1 - "uint type" UINT 1234
         *        entry2 - "container type" ElementList
         *          ElementListInit
         *            entry1: "string type" ASCII "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
         *          ElementListComplete\
         *        entry3 - "uint type" UINT 987654321
         *      ElementListComplete
         */ 
        elementList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 105));
        
        // first entry - uint 1234
        Buffer name = CodecFactory.createBuffer();
        name.data("uint type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.UINT);
        UInt uint = CodecFactory.createUInt();
        uint.value(1234);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, uint));

        // second entry - element list
        name.data("container type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ELEMENT_LIST);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(iter, 0));
        // has standard data
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 50));
        name.data("string type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ASCII_STRING);
        Buffer stringBuffer = CodecFactory.createBuffer();
        stringBuffer.data("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, stringBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(iter, true));
        
        // third entry - uint 987654321
        name.data("another element");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.UINT);
        uint.value(987654321);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, uint));
        
        // complete the list
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        
        byte[] expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/004_elementList_wthrreEntries_middleEntryHasElementList.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 2. Negative case - test elementEntryInit() with an element name that is too big (BUFFER_TO_SMALL).
        elementList.clear();
        buf.data().rewind();
        buf.data().limit(buf.data().capacity());
        buf.data(buf.data(), 0, 25);
        iter.clear();
        elementList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 25));

        name.data("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // 180
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ELEMENT_LIST);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encodeInit(iter, 0));

        // 3. Positive case - test elementEntryInit() with entry with a container of NO_DATA.
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 25));

        name.data("no data type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.NO_DATA);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(iter, 0));

        // 4. Positive case - test encodeElementEntryComplete with false. Encoded bytes verified.
        buf.data().rewind();
        buf.data(buf.data(), 0, 105);
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        elementList.clear();
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 105));
        
        // first entry - uint 1234
        name.data("uint type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.UINT);
        uint.value(1234);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, uint));

        // second entry - element list
        name.data("container type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ELEMENT_LIST);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(iter, 0));
        // has standard data
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 50));
        name.data("string type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ASCII_STRING);
        stringBuffer.data("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, stringBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(iter, false));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/005_elementList_secondEntryElementList_rollback.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 5. Negative case - test encodeElementEntryComplete with internalMerk._sizeBytes > 0.
        Buffer bigBuf = CodecFactory.createBuffer();
        bigBuf.data(ByteBuffer.allocate(312));
        iter.clear();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        elementList.clear();
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 105));
        
        // first entry - uint 1234
        name.data("uint type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.UINT);
        uint.value(1234);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, uint));

        // second entry - element list
        name.data("container type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ELEMENT_LIST);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(iter, 0));
        // has standard data
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(iter, null, 50));
        name.data("string type");
        elementEntry.name(name);
        elementEntry.dataType(DataTypes.ASCII_STRING);
        stringBuffer.data("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZA"); // 209
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(iter, stringBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(iter, true));
        // force encodeElementEntryComplete() to call finishU16Mark with dataLength >= 0xFE,
        // by setting sizeBytes to 1. 
        ((EncodeIteratorImpl)iter)._levelInfo[((EncodeIteratorImpl)iter)._encodingLevel]._internalMark._sizeBytes = 1;
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encodeComplete(iter, true));
    }

    /**
     * This tests the encodeFieldListInit method.
     * <br>
     * <ol>
     * <li>Negative case - field list without anything, BUFFER_TO_SMALL.</li>
     * <li>Negative case - field list with info, BUFFER_TO_SMALL.</li>
     * <li>Negative case - dictionaryID with negative number - INVALID_DATA.</li>
     * </ol>
     * Note that the positive case is covered in encodeFieldListTest().
     */
    @Test
    public void encodeFieldListInitTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(105));
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(0));
        FieldList fieldList = CodecFactory.createFieldList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. Negative case - field list without anything, BUFFER_TO_SMALL.
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldList.encodeInit(iter, null, 105));
        
        
        // 2. Negative case - field list with info, BUFFER_TO_SMALL.
        fieldList.applyHasInfo();
        smallBuf.data(ByteBuffer.allocate(4));

        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldList.encodeInit(iter, null, 105));
                
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(10);
        fieldList.applyHasStandardData();
        smallBuf.data(ByteBuffer.allocate(6));
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldList.encodeInit(iter, null, 105));
    }
    
    /**
     * This tests the encodeFieldListComplete method.
     * <br>
     * <ol>
     * <li>encodeFieldListComplete with false and verify iterator roll back.</li>
     * </ol>
     * Note that the positive case is covered in encodeFieldListTest().
     */
    @Test
    public void encodeFieldListCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(105));
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. encodeFieldListComplete with false and verify iterator roll back. 
        fieldList.applyHasInfo();
        fieldList.dictionaryId(0);
        fieldList.fieldListNum(0);
        fieldList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 105));
        
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.REAL);
        Real real = CodecFactory.createReal();
        real.value(123456789, RealHints.EXPONENT_4);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter, real));
        
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(iter, false));
        assertEquals(0, ((EncodeIteratorImpl)iter)._curBufPos); // verify rollback
    }
    
    /**
     * This tests the encodeFieldEntry method.
     * <br>
     * <ol>
     * <li>test with buffer_to_small at encodeFieldEntry() data!=null.</li>
     * <li>test with unsupported data type in entry.</li>
     * <li>test with encodedData.</li>
     * <li>test with encdata, buffer_to_small.</li>
     * <li>test with encoding blank.</li>
     * <li>test with encoding blank, buffer_to_small.</li>
     * </ol>
     * Note that the positive case is covered in encodeFieldListTest().
     */
    @Test
    public void encodeFieldEntryTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(7));
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. test with buffer_to_small at encodeFieldEntry() data!=null.
        fieldList.applyHasInfo();
        fieldList.dictionaryId(0);
        fieldList.fieldListNum(0);
        fieldList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 7));
        
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.UINT);
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(iter, uint));
        
        int ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(buf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = fieldEntry.encode(iter, uint);
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);           
        
        // 2. test with unsupported data type in entry.
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(60));
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 60));
        fieldEntry.dataType(254);
        assertEquals(CodecReturnCodes.UNSUPPORTED_DATA_TYPE, fieldEntry.encode(iter, uint));
        
        // 3. test with encodedData
        buf.data().clear();
        fieldEntry.clear();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 60));
        Buffer encodedData = CodecFactory.createBuffer();
        encodedData.data("abcdefg");
        fieldEntry.fieldId(100);
        fieldEntry.dataType(DataTypes.OPAQUE);
        fieldEntry.encodedData(encodedData);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter));
        
        // 4. test with encdata, buffer_to_small
        buf.data().clear();
        fieldEntry.clear();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 256));
        encodedData = CodecFactory.createBuffer();
        encodedData.data("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ0123");
        fieldEntry.fieldId(100);
        fieldEntry.dataType(DataTypes.OPAQUE);
        fieldEntry.encodedData(encodedData);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(iter));
        
        ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(buf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = fieldEntry.encode(iter);     
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);            
        
        // 5. test with encoding blank.
        buf.data().clear();
        fieldEntry.clear();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 256));
        fieldEntry.fieldId(30);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeBlank(iter));
        
        // 6. test with encoding blank, buffer_to_small
        iter.clear();
        buf.data().clear();
        buf.data(buf.data(), 0, 8);
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 8));
        fieldEntry.fieldId(30);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encodeBlank(iter));

        ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(buf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)        
        {
            ret = fieldEntry.encodeBlank(iter);   
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);           
    }
    
    /**
     * This tests the encodeFieldEntryInit() method. It contains two test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good<BR>
     */
    @Test
    public void encodeFieldEntryInitTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(7));
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        // 1. Negative test for buffer too small.
        fieldList.applyHasInfo();
        fieldList.dictionaryId(0);
        fieldList.fieldListNum(0);
        fieldList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 7));
        
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeFieldEntryInitTest");
        fieldEntry.encodedData(txt);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encodeInit(iter, 0));
        
        // 2. Positive test for everything good.
        buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(60));
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 60));
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeInit(iter, 0));
        
        assertEquals(9, buf.data().get(0));
        assertEquals(3, buf.data().get(1));
        assertEquals(0x16, buf.data().get(8));
        assertEquals(12, buf.data().position());
    }
    
    /**
     * This tests the encodeFieldEntryComplete() method. It contains two test cases.<BR>
     * 
     * 1. Positive test for everything good, success flag true<BR>
     * 2. Positive test for everything good, success flag false<BR>
     */
    @Test
    public void encodeFieldEntryCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(60));
        Buffer buf2 = CodecFactory.createBuffer();
        buf2.data(ByteBuffer.allocate(60));
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeFieldEntryInitTest");
        
        // 1. Positive test for everything good, success flag true.
        fieldList.applyHasInfo();
        fieldList.dictionaryId(0);
        fieldList.fieldListNum(0);
        fieldList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());        
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        fieldEntry.encodedData(txt);
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 60));
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeInit(iter, 0));
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(iter, true));
        
        assertEquals(9, buf.data().get(0));
        assertEquals(3, buf.data().get(1));
        assertEquals(0x16, buf.data().get(8));
        assertEquals(-2, buf.data().get(9));
        assertEquals(12, buf.data().position());
        
        // 2. Positive test for everything good, success flag false.
        iter.clear();
        iter.setBufferAndRWFVersion(buf2, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 60));
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeInit(iter, 0));
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(iter, false));
        
        assertEquals(9, buf2.data().get(0));
        assertEquals(3, buf2.data().get(1));
        assertEquals(7, buf2.data().position());
    }
    
    /**
     * Positive test. Encode a field list as follows, and verify encoded data.
     * <ul>
     * <li>FieldListInit</li>
     * <ul><li>FieldEntry - (22)  UINT 12345</li></ul>
     * <li>FieldListComplete false (roll-back)</li>
     * <li>FieldListInit</li>
     * <ul>
     * <li>FieldEntry - (10)  REAL Blank - blank to encoder.</li>
     * <li>FieldEntry - (175) pre-encoded data. (ABCDEFG)</li>
     * <li>FieldEntry - (32)  UINT 554433</li>
     * <li>FieldEntry - (111) REAL 867564 EXPONENT_4.</li>
     * <li>FieldEntry - (54)  REAL Blank - real.isBlank</li>
     * </ul>
     * <li>FieldListComplete</li>
     */
    @Test
    public void encodeFieldListTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(40));
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        
        /* 1. Positive test. Encode a field list as follows, and verify encoded data.
         *    FieldListInit
         *      FieldEntry - (22)  UINT 12345
         *    FieldListComplete false (roll-back)
         *    FieldListInit
         *      FieldEntry - (10)  REAL Blank - blank to encoder.
         *      FieldEntry - (175) pre-encoded data. (ABCDEFG)
         *      FieldEntry - (32)  UINT 554433
         *      FieldEntry - (111) REAL 867564 EXPONENT_4.
         *      FieldEntry - (54)  REAL Blank - real.isBlank
         *    FieldListComplete
         */
        
        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 40));
        
        // encode (22) UINT 12345
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.UINT);
        UInt uint = CodecFactory.createUInt();
        uint.value(12345);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter, uint));
        
        // roll-back.
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(iter, false));
        
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(iter, null, 40));
        
        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter, (Real) null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter));
        
        // encode (32) UINT 554433
        uint.clear();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter, uint));
        
        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter, real));
        
        // encode (54) REAL Blank - real.isBlank
        real.clear();
        real.blank();
        fieldEntry.fieldId(54);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(iter, real));
        
        // encodeFieldListComplete
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(iter, true));

        // verify data with UPAC
        byte[] expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/006_encodeFieldList_wEntries_andRollBack.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    /**
     * This tests the encodeArrayInit, encodeArrayEntry and
     * encodeArrayComplete methods.
     * <br>
     * <ol>
     * <li>Negative case - set primitive array type to an unsupported data type.</li>
     * <li>Negative case - use a buffer that is too small.</li>
     * <li>Negative case - invalid item length (negative value).</li>
     * <li>Positive case - valid item length.</li>
     * <li>Positive case - encodeArrayComplete with true</li>
     * <li>Positive case - encodeArrayComplete with false.</li>
     * <li>Negative case - encodeArrayEntry with length = 0 and an invalid primitive type.</li>
     * <li>Negative case - encodeArrayEntry with length = 0</li>
     * <li>Negative case - encodeArrayEntry with length = 0 and unsupported dataType of UTF8 (returned by encodePrimitive()).</li>
     * <li>Negative Case - set item length, a valid primitive type, but a buffer too small.</li>
     * <li>Positive Case - set item length and a valid primitive type. Verify encoded data.</li>
     * <li>Positive case - Array with encodedData, itemLength > 0. Verify encoded data.</li>
     * <li>Positive case - Array with encodedData, itemLength=0. Verify encoded data.</li>
     * <li>Negative case - ArrayEntry with encodedData=null and data=null.</li>
     * <li>Positive case - Array populated with UInts. verify encoded data.</li>
     * </ol>
     * Note that the positive case is covered in encodeFieldListTest().
     */
    @Test
    public void encodeArrayInitEntryCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(3));
        Array array = CodecFactory.createArray();
        Int Int = CodecFactory.createInt();
        
        // 1. Negative case - set primitive array type to an unsupported data type.
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        ((ArrayImpl)array)._primitiveType = DataTypes.INT_2;
        assertEquals(CodecReturnCodes.UNSUPPORTED_DATA_TYPE, Encoders.encodeArrayInit(iter, array));
        
        // 2. Negative case - use a buffer that is too small.
        array.primitiveType(DataTypes.INT);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, Encoders.encodeArrayInit(iter, array));
        
        // 4. Positive case - valid item length.
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.itemLength(1);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));
        
        // 5. Positive case - encodeArrayComplete with true.
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // 6. Positive case - encodeArrayComplete with false.
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, false));

        // 7. Negative case - encodeArrayEntry with length = 0 and an invalid primitive type.
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.RMTES_STRING + 1);
        array.itemLength(0);
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, Encoders.encodeArrayEntry(iter, Int));

        // 8. Negative case - encodeArrayEntry with length = 0
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.RMTES_STRING + 1);
        array.itemLength(0);
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, Encoders.encodeArrayEntry(iter, Int));



        // 9. Negative Case - set item length, a valid primitive type, but a buffer too small. 
        smallBuf.data(ByteBuffer.allocate(10));
        iter.clear();
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.ASCII_STRING);
        array.itemLength(11);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, Encoders.encodeArrayEntry(iter, smallBuf));

        // 10. Positive Case - set item length and a valid primitive type. Verify encoded data.
        smallBuf.data("Presidente");
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.ASCII_STRING);
        array.itemLength(10);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        smallBuf.data("VicePresid");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        smallBuf.data("aSecretary");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify data with UPAC
        byte[] expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/008_array_entries_ascii.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 11. Positive case - Array with encodedData, itemLength > 0. Verify encoded data.
        //     ArrayInit primitiveType=Buffer itemLength =  
        buf.data(buf.data(), 0, buf.data().position());
        buf.data().rewind();
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.BUFFER);
        array.itemLength(5);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        smallBuf.data("abcde");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        smallBuf.data("01234");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        smallBuf.data("ABCDE");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/010_array_entries_encData_len2.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 12. Positive case - Array with encodedData, itemLength=0. Verify encoded data.
        buf.data().rewind();
        buf.data().limit(buf.data().capacity());
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.clear();
        array.primitiveType(DataTypes.BUFFER);
        array.itemLength(0);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        smallBuf.data("abcde");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        smallBuf.data("0123456789");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        smallBuf.data("ABCDEFG");
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, smallBuf));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/011_array_entries_encData_len0.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 13. Positive case - ArrayEntry with itemLength > 0 and encodedData.length=0. Verify encoded data.
        buf.data().rewind();
        buf.data().limit(buf.data().capacity());
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.clear();
        array.primitiveType(DataTypes.BUFFER);
        array.itemLength(4);
        Buffer emptyBuffer = CodecFactory.createBuffer(); 
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, emptyBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, emptyBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, emptyBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/012_array_entry_blankEncData_len4.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // 14. Positive case - ArrayEntry with itemLength > 0 and encodedData.length=0. Verify encoded data.
        buf.data().rewind();
        buf.data().limit(buf.data().capacity());
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.clear();
        array.primitiveType(DataTypes.BUFFER);
        array.itemLength(0);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, emptyBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, emptyBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, emptyBuffer));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/013_array_entry_blankEncData_len0.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
      

        
        // 15. Positive case - Array populated with UInts. Verify encoded data. 
        //     ArrayInit primitiveType=UInt, itemLength=2;
        //       ArrayEntry 0
        //       ArrayEntry 255        // one byte
        //       ArrayEntry 65535      // two bytes
        //     ArrayComplete
        buf.data().rewind();
        buf.data().limit(buf.data().capacity());
        iter.clear();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.UINT);
        array.itemLength(2);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        UInt uint = CodecFactory.createUInt();
        uint.value(0);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, uint));
        uint.value(255);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, uint));
        uint.value(65535);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, uint));
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/009_array_entries_uint.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    @Test
    public void encodePrimitiveTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Array array = CodecFactory.createArray();
        Int Int = CodecFactory.createInt();
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        array.primitiveType(DataTypes.INT);
        array.itemLength(0);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayInit(iter, array));        
        // one byte needed
        Int.value(0);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(-1);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(127);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(-128);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        // two byte needed
        Int.value(128);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(32767);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(-32768);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        // three bytes needed
        Int.value(32768);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(8388607);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(-8388608);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        // four bytes needed
        Int.value(8388608);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(2147483647);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        Int.value(-2147483648);
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayEntry(iter, Int));
        // encodeArrayComplete
        assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeArrayComplete(iter, true));

        // verify encoded data
        byte[] expectedData= ParseHexFile.parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/014_encodePrimitive_int_len0.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    
    /**
     * encode a request message and compare encoded contexts with UPAC.
     */
    @Test
    public void encodeRequestMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        RequestMsg msg = (RequestMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory
                .createBuffer();
        Buffer position = CodecFactory.createBuffer();
        // RsslBuffer password = RsslCodecFactory.createBuffer(), instanceId =
        // RsslCodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.REQUEST);
        msg.streamId(100);
        msg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.ELEMENT_LIST);
        msg.applyStreaming();
        msg.applyConfInfoInUpdates();
        msg.applyMsgKeyInUpdates();
        msg.applyHasBatch();
        msg.applyHasView();

        /* set priority */
        msg.applyHasPriority();
        msg.priority().priorityClass(3);
        msg.priority().count(4);

        /* set QoS and Worst QoS */
        msg.applyHasQos();
        msg.applyHasWorstQos();
        msg.qos().timeliness(QosTimeliness.REALTIME);
        msg.qos().rate(QosRates.TICK_BY_TICK);
        msg.worstQos().timeliness(QosTimeliness.DELAYED);
        msg.worstQos().timeInfo(65532);
        msg.worstQos().rate(QosRates.TIME_CONFLATED);
        msg.worstQos().rateInfo(65533);

        /* set msgKey members */
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();

        msg.msgKey().name().data("Batch_Request");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);
        
        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload

        ElementList eList = CodecFactory.createElementList();
        ElementEntry eEntry = CodecFactory.createElementEntry();
        Array elementArray = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        Buffer itemName = CodecFactory.createBuffer();

        eList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, eList.encodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.name(ElementNames.BATCH_ITEM_LIST);
        eEntry.dataType(DataTypes.ARRAY);
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.primitiveType(DataTypes.ASCII_STRING);
        elementArray.itemLength(0);

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeInit(encodeIter));
        itemName.data("TRI.N");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));
        itemName.data("IBM.N");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));
        itemName.data("CSCO.O");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeComplete(encodeIter, true));

        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeComplete(encodeIter, true));

        // encode View (22 = BID, 25 = ASK)
        int[] viewList = { 22, 25 };
        int viewListCount = 2;
        int i;
        UInt tempUInt = CodecFactory.createUInt();

        eEntry.clear();
        eEntry.name(ElementNames.VIEW_TYPE);
        eEntry.dataType(DataTypes.UINT);
        tempUInt.value(ViewTypes.FIELD_ID_LIST);
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encode(encodeIter, tempUInt));

        eEntry.clear();
        eEntry.name(ElementNames.VIEW_DATA);
        eEntry.dataType(DataTypes.ARRAY);
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeInit(encodeIter, 0));

        elementArray.clear();
        elementArray.primitiveType(DataTypes.UINT);
        elementArray.itemLength(2); // fixed length values

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeInit(encodeIter));

        for (i = 0; i < viewListCount; i++)
        {
            tempUInt.value(viewList[i]);
            assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, tempUInt));
        }

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeComplete(encodeIter, true));

        /* complete encoding of complex element entry. */
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeComplete(encodeIter, true));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, eList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData= ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/001_requestMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    /**
     * Encode a refresh message and compare encoded contexts with UPAC.
     */
    @Test
    public void encodeRefreshMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory
                .createBuffer();
        Buffer position = CodecFactory.createBuffer();
        // RsslBuffer password = RsslCodecFactory.createBuffer(), instanceId =
        // RsslCodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
        msg.streamId(Integer.MAX_VALUE);
        msg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.FIELD_LIST);
        msg.applyRefreshComplete();
        msg.applySolicited();
        msg.applyClearCache();
        msg.applyDoNotCache();

        /* set state */
        msg.state().streamState(StreamStates.OPEN);
        msg.state().dataState(DataStates.OK);
        msg.state().code(StateCodes.NONE);
        Buffer text = CodecFactory.createBuffer();
        text.data("some text info");
        msg.state().text(text);

        /* set groupId */
        Buffer groupId = CodecFactory.createBuffer();
        groupId.data("10203040");
        msg.groupId(groupId);
        
        /* set QoS */
        msg.applyHasQos();
        msg.qos().timeliness(QosTimeliness.REALTIME);
        msg.qos().rate(QosRates.TICK_BY_TICK);

        /* set part number */
        msg.applyHasPartNum();
        msg.partNum(32767);

        /* set sequence number */
        msg.applyHasSeqNum();
        msg.seqNum(1234567890L);
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* set permData */
        msg.applyHasPermData();
        byte [] ba = {0x10, 0x11, 0x12, 0x13};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(ba));
        msg.permData(permData);
        
        /* set post user info */
        msg.applyHasPostUserInfo();
        msg.postUserInfo().userAddr(4294967290L); // 0xFFFFFFFA
        msg.postUserInfo().userId(4294967295L); // 0xFFFFFFFF

        /* set msgKey members */
        msg.applyHasMsgKey();
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasServiceId();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        msg.msgKey().serviceId(32639);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */


        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter));

        // encode (32) UINT 554433
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, uint));

        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, real));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData = ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/002_refreashMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    /**
     * Encode a close message and compare encoded contexts with UPAC.
     */
    @Test
    public void encodeCloseMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        CloseMsg msg = (CloseMsg)CodecFactory.createMsg();

        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.CLOSE);
        msg.streamId(Integer.MAX_VALUE);
        msg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.NO_DATA);
        msg.applyAck();
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(encodeIter));

        byte[] expectedData = ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/003_closeMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
      
    /**
     * Encode an update message and compare encoded contexts with UPAC.
     */
    @Test
    public void encodeUpdateMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        UpdateMsg msg = (UpdateMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory
                .createBuffer();
        Buffer position = CodecFactory.createBuffer();
        // RsslBuffer password = RsslCodecFactory.createBuffer(), instanceId =
        // RsslCodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.UPDATE);
        msg.streamId(2146290601); // 0x7FEDCBA9
        msg.domainType(DomainTypes.LOGIN);
        msg.containerType(DataTypes.FIELD_LIST);
        msg.updateType(UpdateEventTypes.QUOTE);
        msg.applyDoNotCache();
        msg.applyDoNotConflate();
        msg.applyDoNotRipple();

        /* set sequence number */
        msg.applyHasSeqNum();
        msg.seqNum(1234567890L);
        
        /* conflation Count and Conflation Time */
        msg.applyHasConfInfo();
        msg.conflationCount(10);
        msg.conflationTime(500); // ms
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* set permData */
        msg.applyHasPermData();
        byte [] pd = {0x10, 0x11, 0x12, 0x13};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(pd));
        msg.permData(permData);
        
        /* set post user info */
        msg.applyHasPostUserInfo();
        msg.postUserInfo().userAddr(4294967290L); // 0xFFFFFFFA
        msg.postUserInfo().userId(4294967295L); // 0xFFFFFFFF

        /* set msgKey members */
        msg.applyHasMsgKey();
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasServiceId();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        msg.msgKey().serviceId(32639);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
		 * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter));

        // encode (32) UINT 554433
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, uint));

        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, real));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData= ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/004_updateMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    @Test
    public void encodeRequestWithPrivateStreamTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        RequestMsg msg = (RequestMsg)CodecFactory.createMsg();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.REQUEST);
        msg.streamId(100);
        msg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.NO_DATA);
        msg.applyPrivateStream();
        
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(encodeIter));
        
        byte[] expectedData = new byte[] {0x00, 0x0C, 0x01, 0x06, 0x00, 0x00, 0x00, 0x64, -0x7f, 0x00, 0x00, -0x80, 0x01, 0x00 };
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
    }
    
    @Test
    public void encodeRequestWithIdAndEncAttribTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        RequestMsg msg = (RequestMsg)CodecFactory.createMsg();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.REQUEST);
        msg.streamId(100);
        msg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.NO_DATA);

        /* set msgKey members */
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasIdentifier();
        msg.msgKey().applyHasAttrib();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().identifier(0x7fff);
        
        Buffer encodedAttrib = CodecFactory.createBuffer();
        encodedAttrib.data("ENCODED ATTRIB");
        msg.msgKey().attribContainerType(DataTypes.OPAQUE);
        msg.msgKey().encodedAttrib(encodedAttrib);
        
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(encodeIter));
        
        // prepare ByteBuffer for printing.
        buf.data(buf.data(), 0, buf.data().position());
        System.out.println(buf.toHexString());

        byte[] expectedData = ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/001_requestMsgWithIdAndEncAttrib.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    /*
     * Test RsslMsgKey.copy() and RsslMsgKey.copyReferences().
     */
    @Test
    public void msgKeyImplCopyReferencesTest()
    {
        MsgKey msgKey = CodecFactory.createMsgKey();
        MsgKey msgKeyCopy = CodecFactory.createMsgKey();

        // set msgKey members
        msgKey.applyHasNameType();
        msgKey.applyHasName();
        msgKey.applyHasIdentifier();
        msgKey.applyHasAttrib();

        Buffer name = CodecFactory.createBuffer();
        name.data("TRI.N");
        msgKey.name(name);
        msgKey.nameType(InstrumentNameTypes.RIC);
        msgKey.identifier(0x7fff);

        Buffer encodedAttrib = CodecFactory.createBuffer();
        encodedAttrib.data("ENCODED ATTRIB");
        msgKey.attribContainerType(DataTypes.OPAQUE);
        msgKey.encodedAttrib(encodedAttrib);

        // perform the copyReferences
        ((MsgKeyImpl)msgKeyCopy).copyReferences(msgKey);

        // verify
        assertEquals(msgKey.flags(), msgKeyCopy.flags());
        assertEquals(true, msgKeyCopy.checkHasName());
        assertEquals(true, msgKeyCopy.checkHasNameType());
        assertEquals(true, msgKeyCopy.checkHasIdentifier());
        assertEquals(true, msgKeyCopy.checkHasAttrib());
        assertEquals(DataTypes.OPAQUE, msgKeyCopy.attribContainerType());

        // name references should match.
        assertEquals(msgKey.name().data(), msgKeyCopy.name().data());
        assertEquals(msgKey.nameType(), msgKeyCopy.nameType());
        assertEquals(msgKey.identifier(), msgKeyCopy.identifier());
        assertEquals(msgKey.attribContainerType(), msgKeyCopy.attribContainerType());
        // encodedAttrib references should match.
        assertEquals(msgKey.encodedAttrib().data(), msgKeyCopy.encodedAttrib().data());
    }

    /*
     * Test RsslMsgImpl.msgKey() (which tests RsslMsgKey.copyReferences()).
     */
    @Test
    public void MsgMsgKeyTest()
    {
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        msg.msgClass(MsgClasses.REFRESH);
        msg.applyHasMsgKey();

        MsgKey msgKey = msg.msgKey();
        MsgKey msgKeyFromRsslMsg;

        // set msgKey members
        msgKey.applyHasNameType();
        msgKey.applyHasName();
        msgKey.applyHasIdentifier();
        msgKey.applyHasAttrib();

        Buffer name = CodecFactory.createBuffer();
        name.data("TRI.N");
        msgKey.name(name);
        msgKey.nameType(InstrumentNameTypes.RIC);
        msgKey.identifier(0x7fff);

        Buffer encodedAttrib = CodecFactory.createBuffer();
        encodedAttrib.data("ENCODED ATTRIB");
        msgKey.attribContainerType(DataTypes.OPAQUE);
        msgKey.encodedAttrib(encodedAttrib);

        // get the msgKey from the RsslMsg.
        msgKeyFromRsslMsg = msg.msgKey();

        // verify that the msgKey from the RsslMsg has the references to our msgKey.
        assertEquals(msgKey.flags(), msgKeyFromRsslMsg.flags());
        assertEquals(true, msgKeyFromRsslMsg.checkHasName());
        assertEquals(true, msgKeyFromRsslMsg.checkHasNameType());
        assertEquals(true, msgKeyFromRsslMsg.checkHasIdentifier());
        assertEquals(true, msgKeyFromRsslMsg.checkHasAttrib());
        assertEquals(DataTypes.OPAQUE, msgKeyFromRsslMsg.attribContainerType());

        // name references should match.
        assertEquals(msgKey.name().data(), msgKeyFromRsslMsg.name().data());
        assertEquals(msgKey.nameType(), msgKeyFromRsslMsg.nameType());
        assertEquals(msgKey.identifier(), msgKeyFromRsslMsg.identifier());
        assertEquals(msgKey.attribContainerType(), msgKeyFromRsslMsg.attribContainerType());
        // encodedAttrib references should match.
        assertEquals(msgKey.encodedAttrib().data(), msgKeyFromRsslMsg.encodedAttrib().data());
    }

    @Test
    public void tmpTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Int intKey = CodecFactory.createInt();

        map.applyHasKeyFieldId();
        map.applyHasPerEntryPermData();
        map.applyHasTotalCountHint();
        map.applyHasSummaryData();
        map.applyHasSetDefs();
        map.totalCountHint(5);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeMapEntryTest");
        ((MapImpl)map).encodedEntries(txt);
        map.encodedSetDefs(txt);
        map.encodedSummaryData(txt);
        map.containerType(DataTypes.FIELD_LIST);
        map.keyPrimitiveType(DataTypes.INT);
        map.keyFieldId(22);
        mapEntry.action(MapEntryActions.ADD);
        mapEntry.encodedData().data(ByteBuffer.allocate(0));
        intKey.value(33);

        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        // 2. Positive test for everything good without permData
        assertTrue(map.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(mapEntry.encode(iter, intKey) == CodecReturnCodes.SUCCESS);
        assertTrue(map.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        //byte[] expectedData = ParseHexFile.parse("tmpTestData.txt");
        //assertNotNull(expectedData);
        //assertArrayEquals(expectedData, convertToByteArray(buf.data()));
        
        // prepare ByteBuffer for printing.
        //buf.data(buf.data(), 0, buf.data().position());
        //System.out.println(buf.toHexString());
    }
    
    // copy encoded data into byte[]
    private byte[] convertToByteArray(ByteBuffer bb)
    {
        bb.flip(); // prepare for writing.
        byte[] ba = new byte[bb.limit()];
        bb.get(ba);
        return ba;
    }
    
    /**
     * This tests the encodeSeriesInit() method. It contains five test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Negative test for unsupported container type<BR>
     * 3. Positive test for everything good without summaryData but flag set
     * 4. Positive test for everything good without setDefs but flag set<BR>
     * 5. Positive test for everything good<BR>
     */
    @Test
    public void encodeSeriesInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(1));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Series series = CodecFactory.createSeries();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer txt = CodecFactory.createBuffer();
     
        series.applyHasTotalCountHint();
        series.totalCountHint(5);
        txt.data("encodeSeriesInitTest");
        ((SeriesImpl)series).encodedEntries(txt);
        series.containerType(DataTypes.FIELD_LIST);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        // 2. Negative test for unsupported container type
        ((SeriesImpl)series)._containerType = DataTypes.ARRAY;
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.UNSUPPORTED_DATA_TYPE);
        
        // reset container type
        series.containerType(DataTypes.FIELD_LIST);
        
        // 3. Positive test for everything good without summaryData but flag set
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        series.applyHasSummaryData();
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        
        assertEquals(0x06, buf.data().get(0));
        assertEquals(0x04, buf.data().get(1));
        assertEquals(0x00, buf.data().get(2));
        assertEquals(0x00, buf.data().get(3));
        assertEquals(4, buf.data().position());
        
        // 4. Positive test for everything good without setDefs but flag set
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        series.applyHasSetDefs();
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        
        assertEquals(0x07, buf.data().get(0));
        assertEquals(0x04, buf.data().get(1));
        assertEquals(0x00, buf.data().get(2));
        assertEquals(0x00, buf.data().get(3));
        assertEquals(4, buf.data().position());

        // 5. Positive test for everything good
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        series.encodedSetDefs(txt);
        series.encodedSummaryData(txt);
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x07, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x49,
        	 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65,
        	 0x72, 0x69, 0x65, 0x73, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(47, buf.data().position());
    }
    
    /**
     * This tests the encodeSeriesComplete() method. It contains two test cases.<BR>
     * 
     * 1. Positive test for everything good with success flag true<BR>
     * 2. Positive test for everything good with success flag false<BR>
     */
    @Test
    public void encodeSeriesCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Series series = CodecFactory.createSeries();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeSeriesCompleteTest");
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        series.applyHasTotalCountHint();
        series.applyHasSummaryData();
        series.applyHasSetDefs();
        series.totalCountHint(5);
        ((SeriesImpl)series).encodedEntries(txt);
        series.encodedSetDefs(txt);
        series.encodedSummaryData(txt);
        series.containerType(DataTypes.FIELD_LIST);                
        
        // 1. Positive test for everything good with success flag true
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(series.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x07, 0x04, 0x18, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x43,
        	 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x18, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65,
        	 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(55, buf.data().position());
        
        // 2. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(series.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        assertEquals(0, buf.data().position());
    }

    /**
     * This tests the encodeSeriesEntryInit() method. It contains three test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good without pre-encoded data<BR>
     * 3. Positive test for everything good with pre-encoded data<BR>
     */
    @Test
    public void encodeSeriesEntryInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(60));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeSeriesEntryInitTest");
     
        series.applyHasTotalCountHint();
        series.applyHasSummaryData();
        series.applyHasSetDefs();
        /* Use a larger totalCountHint. The seriesEntry.encodeInit will write a 3-byte length, 
         * but series.encodeInit's overrun check for totalCount assumes the worst-case (4 bytes),
         * so seriesEntry.encodeInit's fail case can only be checked if at least two bytes were 
         * used to write totalCountHint. */
        series.totalCountHint(255); 
        ((SeriesImpl)series).encodedEntries(txt);
        series.encodedSetDefs(txt);
        series.encodedSummaryData(txt);
        series.containerType(DataTypes.FIELD_LIST);                
                        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);

        // 2. Positive test for everything good without pre-encoded data
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
        	 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, (byte)0x80, (byte)0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData, bufBytes);
        assertEquals(61, buf.data().position());
        
        // 3. Positive test for everything good with pre-encoded data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        seriesEntry.encodedData(txt);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData2 =
        	{0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
           	 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
           	 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
           	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, (byte)0x80, (byte)0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData2, bufBytes);
        assertEquals(61, buf.data().position());
    }

    /**
     * This tests the encodeSeriesEntryComplete() method. It contains four test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Negative test for invalid data<BR>
     * 3. Positive test for everything good with success flag true<BR>
     * 4. Positive test for everything good with success flag false<BR>
     */
    @Test
    public void encodeSeriesEntryCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeSeriesEntryComplete");
             
        series.applyHasTotalCountHint();
        series.applyHasSummaryData();
        series.applyHasSetDefs();
        series.totalCountHint(5);
        ((SeriesImpl)series).encodedEntries(txt);
        series.encodedSetDefs(txt);
        series.encodedSummaryData(txt);
        series.containerType(DataTypes.FIELD_LIST);                
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        ((EncodeIteratorImpl)iter)._curBufPos = 100000;
        assertTrue(seriesEntry.encodeComplete(iter, true) == CodecReturnCodes.BUFFER_TOO_SMALL);

        // 2. Negative test for invalid data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        ((EncodeIteratorImpl)iter)._curBufPos = 100000;
        ((EncodeIteratorImpl)iter)._levelInfo[((EncodeIteratorImpl)iter)._encodingLevel]._internalMark._sizeBytes = 0;
        assertTrue(seriesEntry.encodeComplete(iter, true) == CodecReturnCodes.INVALID_DATA);
        
        // 3. Positive test for everything good with success flag true
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        		{0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
        		 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x19, 0x65, 0x6e, 0x63,
        		 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
        		 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x05, 0, 0, (byte)0xfe, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(60, buf.data().position());
        
        // 4. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData2 =
        	{0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
       		 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x19, 0x65, 0x6e, 0x63,
       		 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
       		 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        // compare only first 57 bytes here even though more was encoded
        // success = false moves position back to 54
        assertEquals(57, buf.data().position());
        for (int i = 0; i < 57; i++)
        {
        	assertTrue(bufBytes[i] == expectedData2[i]);
        }
    }
    
    private Buffer growByOneAndCopy(Buffer src)
    {
    	ByteBuffer newBuf =  ByteBuffer.allocate(src.data().capacity() + 1) ;
    	src.data().flip();
    	newBuf.put(src.data());
    	src.data(newBuf);
    	return src;
    }
    
    /**
     * This tests the encodeSeriesEntry() method. It contains five test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good without pre-encoded data<BR>
     * 3. Positive test for everything good with pre-encoded data<BR>
     * 4. Positive test for everything good with big pre-encoded data<BR>
     * 5. Negative test for everything good with too big pre-encoded data<BR>
     */
    @Test
    public void encodeSeriesEntryTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(52));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(80));
        Buffer bigBuf = CodecFactory.createBuffer();
        Buffer bigBuf2 = CodecFactory.createBuffer();
        bigBuf.data(ByteBuffer.allocate(65700));
        bigBuf2.data(ByteBuffer.allocate(65700));
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeSeriesEntryTest");
     
        series.applyHasTotalCountHint();
        series.applyHasSummaryData();
        series.applyHasSetDefs();

        /* Use a larger totalCountHint. The seriesEntry.encode will write a 1-byte length
         * (payload is empty), but series.encodeInit's overrun check for totalCount assumes the 
         * worst-case (4 bytes), so seriesEntry.encodeInit's fail case can only be checked if 
         * four bytes were used to write totalCountHint. */
        series.totalCountHint(555555555);
        ((SeriesImpl)series).encodedEntries(txt);
        series.encodedSetDefs(txt);
        series.encodedSummaryData(txt);
        series.containerType(DataTypes.FIELD_LIST);                
                        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encode(iter) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // need to copy into new buffer
        int ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(smallBuf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)
        {
            ret = seriesEntry.encode(iter);       
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);        

        // 2. Positive test for everything good without pre-encoded data
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(seriesEntry.encode(iter) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x07, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
        	 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
        	 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, (byte)0xE1, 0x1D, 
        	 0x1A, (byte)0xE3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0 };
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData, bufBytes);
        assertEquals(53, buf.data().position());
        
        // 3. Positive test for everything good with pre-encoded data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        seriesEntry.encodedData(txt);
        assertTrue(seriesEntry.encode(iter) == CodecReturnCodes.SUCCESS);

        byte[] expectedData2 =
        	{0x07, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
           	 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
           	 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, (byte)0xE1, 0x1D, 
             0x1A, (byte)0xE3, 0x00, 0x00, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 
             0x73, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0,
             0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData2, bufBytes);
        assertEquals(74, buf.data().position());
        
        // 4. Positive test for everything good with big pre-encoded data
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        ByteBuffer bb = bigBuf2.data();
        for (int i = 0; i < 65535; i++)
        {
        	bb.put((byte)i);
        }
        seriesEntry.clear();
        seriesEntry.encodedData(bigBuf2);
        assertTrue(seriesEntry.encode(iter) == CodecReturnCodes.SUCCESS);
        
        // 5. Negative test for everything good with too big pre-encoded data
        bigBuf.data().rewind();
        bigBuf2.data().rewind();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(series.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        bb = bigBuf2.data();
        for (int i = 0; i < 65536; i++)
        {
        	bb.put((byte)i);
        }
        seriesEntry.clear();
        seriesEntry.encodedData(bigBuf2);
        assertTrue(seriesEntry.encode(iter) == CodecReturnCodes.INVALID_DATA);
    }
    
    /**
     * This tests the encodeVectorInit() method. It contains eight test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Negative test for unsupported container type<BR>
     * 3. Positive test for everything good without summaryData but flag set
     * 4. Positive test for everything good without setDefs but flag set<BR>
     * 5. Positive test for everything good<BR>
     * 6. Positive test for everything good and summary data complete with success true<BR>
     * 7. Positive test for everything good and summary data complete with success false<BR>
     * 8. Negative test for buffer that is big enough for vector header but not summary data<BR>
     */
    @Test
    public void encodeVectorInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(1));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Buffer summaryDataSmallBuf = CodecFactory.createBuffer();
        summaryDataSmallBuf.data(ByteBuffer.allocate(26));
        Vector vector = CodecFactory.createVector();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeVectorInitTest");
     
        vector.applyHasPerEntryPermData();
        vector.applyHasTotalCountHint();
        vector.applySupportsSorting();
        vector.totalCountHint(5);
        ((VectorImpl)vector).encodedEntries(txt);
        vector.containerType(DataTypes.FIELD_LIST);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // reset to bigger buffer
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        // 2. Negative test for unsupported container type
        ((VectorImpl)vector)._containerType = DataTypes.ARRAY;
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.UNSUPPORTED_DATA_TYPE);
        
        // reset container type
        vector.containerType(DataTypes.FIELD_LIST);
        
        // 3. Positive test for everything good without summaryData but flag set
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vector.applyHasSummaryData();
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        
        assertEquals(0x1a, buf.data().get(0));
        assertEquals(0x04, buf.data().get(1));
        assertEquals(0x00, buf.data().get(2));
        assertEquals(0x00, buf.data().get(3));
        assertEquals(4, buf.data().position());
        
        // 4. Positive test for everything good without setDefs but flag set
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vector.applyHasSetDefs();
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        
        assertEquals(0x1b, buf.data().get(0));
        assertEquals(0x04, buf.data().get(1));
        assertEquals(0x00, buf.data().get(2));
        assertEquals(0x00, buf.data().get(3));
        assertEquals(4, buf.data().position());

        // 5. Positive test for everything good
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vector.encodedSetDefs(txt);
        vector.encodedSummaryData(txt);
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x1b, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x49,
        	 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65,
        	 0x63, 0x74, 0x6f, 0x72, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 5, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(47, buf.data().position());

        // 6. Positive test for everything good and summary data complete with success true
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 64 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vector.encodedSummaryData().clear();
        vector.applyHasSummaryData();
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vector.encodeSummaryDataComplete(iter, true) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData2 =
        	{0x1b, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x49,
        	 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, (byte)0x80, 0, 5, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes2 = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes2));
        
        assertEquals(28, buf.data().position());
        assertArrayEquals(expectedData2, bufBytes2);

        // 7. Positive test for everything good and summary data complete with success false
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 64 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vector.encodeSummaryDataComplete(iter, false) == CodecReturnCodes.SUCCESS);

        byte[] expectedData3 =
        	{0x1b, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x49,
        	 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes3 = new byte[expectedData3.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes3));
        
        // Should be at start of summary data encoding.
        assertEquals(25, buf.data().position());
        assertArrayEquals(expectedData3, bufBytes3);
        
        // 8. Negative test for buffer that is big enough for vector header but not summary data
        iter.setBufferAndRWFVersion(summaryDataSmallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);
    }
    
    /**
     * This tests the encodeVectorComplete() method. It contains two test cases.<BR>
     * 
     * 1. Positive test for everything good with success flag true<BR>
     * 2. Positive test for everything good with success flag false<BR>
     */
    @Test
    public void encodeVectorCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(64));
        Vector vector = CodecFactory.createVector();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeVectorCompleteTest");
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
     
        vector.applyHasPerEntryPermData();
        vector.applyHasTotalCountHint();
        vector.applySupportsSorting();
        vector.applyHasSummaryData();
        vector.applyHasSetDefs();
        vector.totalCountHint(5);
        ((VectorImpl)vector).encodedEntries(txt);
        vector.encodedSetDefs(txt);
        vector.encodedSummaryData(txt);
        vector.containerType(DataTypes.FIELD_LIST);                
        
        // 1. Positive test for everything good with success flag true
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vector.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x1b, 0x04, 0x18, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x43,
        	 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x18, 0x65, 0x6e, 0x63, 0x6f,
        	 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65,
        	 0x54, 0x65, 0x73, 0x74, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(55, buf.data().position());
        
        // 2. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vector.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        assertEquals(0, buf.data().position());
    }

    /**
     * This tests the encodeVectorEntryInit() method. It contains five test cases.<BR>
     * 
     * 1. Positive test for everything good without permData but flag set<BR>
     * 2. Negative test for buffer too small<BR>
     * 3. Positive test for everything good with DELETE entry<BR>
     * 4. Positive test for everything good without pre-encoded data<BR>
     * 5. Positive test for everything good with pre-encoded data<BR>
     */
    @Test
    public void encodeVectorEntryInitTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(64));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeVectorEntryInitTest");
     
        vector.applySupportsSorting();
        vector.applyHasPerEntryPermData();
        vector.applyHasTotalCountHint();
        vector.applyHasSummaryData();
        vector.applyHasSetDefs();
        vector.totalCountHint(5);
        ((VectorImpl)vector).encodedEntries(txt);
        vector.encodedSetDefs(txt);
        vector.encodedSummaryData(txt);
        vector.containerType(DataTypes.FIELD_LIST);                
        vectorEntry.applyHasPermData();
        vectorEntry.action(VectorEntryActions.SET);
        vectorEntry.index(11);
                        
        // 1. Positive test for everything good without permData but flag set
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData =
        	{0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
        	 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x0b, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(63, buf.data().position());
        
        // 2. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        vectorEntry.permData(txt);
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.BUFFER_TOO_SMALL);

        // 3. Positive test for everything good with DELETE entry
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 128 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vectorEntry.action(VectorEntryActions.DELETE);
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData2 =
        	{0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
        	 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0x15, 0x0b, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64,
        	 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
        	 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData2, bufBytes);
        assertEquals(85, buf.data().position());

        // 4. Positive test for everything good without pre-encoded data
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 128 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vectorEntry.action(VectorEntryActions.SET);
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData3 =
        	{0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
        	 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
        	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0x12, 0x0b, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64,
        	 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
        	 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData3.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData3, bufBytes);
        assertEquals(88, buf.data().position());

        // 5. Positive test for everything good with pre-encoded data
        buf.data().rewind();
        // clear buffer
        for (int i = 0; i < 128 ; i++)
        {
        	buf.data().put(i, (byte)0);
        }
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        vectorEntry.encodedData(txt);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);

        byte[] expectedData4 =
        	{0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
           	 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
           	 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
           	 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0x12, 0x0b, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64,
           	 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
           	 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData4.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData4, bufBytes);
        assertEquals(88, buf.data().position());
    }
    
    /**
     * This tests the encodeVectorEntryComplete() method. It contains five test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Negative test for invalid data<BR>
     * 3. Positive test for everything good with success flag true<BR>
     * 4. Positive test for everything good with success flag false<BR>
     * 5. Positive test for everything good with DELETE entry<BR>
     */
    @Test
    public void encodeVectorEntryCompleteTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(128));
        Buffer buf2 = CodecFactory.createBuffer();
        buf2.data(ByteBuffer.allocate(128));
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        byte[] bufBytes;
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeVectorEntryCompleteTest");		
        
        vector.applySupportsSorting();
        vector.applyHasPerEntryPermData();
        vector.applyHasTotalCountHint();
        vector.applyHasSummaryData();
        vector.applyHasSetDefs();
        vector.totalCountHint(5);
        ((VectorImpl)vector).encodedEntries(txt);
        vector.encodedSetDefs(txt);
        vector.encodedSummaryData(txt);
        vector.containerType(DataTypes.FIELD_LIST);                
        vectorEntry.applyHasPermData();
        vectorEntry.permData(txt);
        vectorEntry.action(VectorEntryActions.SET);
        vectorEntry.index(11);
        vectorEntry.encodedData(txt);
        
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        ((EncodeIteratorImpl)iter)._curBufPos = 100000;
        assertTrue(vectorEntry.encodeComplete(iter, true) == CodecReturnCodes.BUFFER_TOO_SMALL);   

        // 2. Negative test for invalid data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        ((EncodeIteratorImpl)iter)._curBufPos = 100000;
        ((EncodeIteratorImpl)iter)._levelInfo[((EncodeIteratorImpl)iter)._encodingLevel]._internalMark._sizeBytes = 0;
        assertTrue(vectorEntry.encodeComplete(iter, true) == CodecReturnCodes.INVALID_DATA);

        // 3. Positive test for everything good with success flag true
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x1b, 0x04, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74,
        	 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74,
        	 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x05, 0,
        	 0, 0x12, 0x0b, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72,
        	 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73,
        	 0x74, (byte)0xfe, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        assertArrayEquals(expectedData, bufBytes);
        assertEquals(100, buf.data().position());
        
        // 4. Positive test for everything good with success flag false
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeComplete(iter, false) == CodecReturnCodes.SUCCESS);
        
        byte[] expectedData2 =
        	{0x1b, 0x04, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74,
        	 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74,
        	 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x05, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));
        
        // compare only first 65 bytes here even though more was encoded
        // success = false moves position back to 65
        assertEquals(65, buf.data().position());
        for (int i = 0; i < 65; i++)
        {
        	assertTrue(bufBytes[i] == expectedData2[i]);
        }

        // 5. Positive test for everything good with DELETE entry
        iter.setBufferAndRWFVersion(buf2, Codec.majorVersion(), Codec.minorVersion());
        vectorEntry.action(VectorEntryActions.DELETE);
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeInit(iter, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encodeComplete(iter, true) == CodecReturnCodes.SUCCESS);

        byte[] expectedData3 =
        	{0x1b, 0x04, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
        	 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74,
        	 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74,
        	 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x05, 0,
        	 0, 0x15, 0x0b, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72,
        	 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73,
        	 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData3.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf2.copy(bufBytes));

        assertArrayEquals(expectedData3, bufBytes);
        assertEquals(97, buf2.data().position());
    }
    
    /**
     * This tests the encodeVectorEntryTest() method. It contains five test cases.<BR>
     * 
     * 1. Negative test for buffer too small<BR>
     * 2. Positive test for everything good without pre-encoded data<BR>
     * 3. Positive test for everything good with pre-encoded data<BR>
     * 4. Positive test for everything good with big pre-encoded data<BR>
     * 5. Negative test for everything good with too big pre-encoded data<BR>
     */
    @Test
    public void encodeVectorEntryTest()
    {
        Buffer smallBuf = CodecFactory.createBuffer();
        smallBuf.data(ByteBuffer.allocate(64));
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(100));
        Buffer bigBuf = CodecFactory.createBuffer();
        Buffer bigBuf2 = CodecFactory.createBuffer();
        bigBuf.data(ByteBuffer.allocate(65700));
        bigBuf2.data(ByteBuffer.allocate(65700));
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeVectorEntryTest");
     
        vector.applySupportsSorting();
        vector.applyHasPerEntryPermData();
        vector.applyHasTotalCountHint();
        vector.applyHasSummaryData();
        vector.applyHasSetDefs();
        vector.totalCountHint(5);
        ((VectorImpl)vector).encodedEntries(txt);
        vector.encodedSetDefs(txt);
        vector.encodedSummaryData(txt);
        vector.containerType(DataTypes.FIELD_LIST);                
        vectorEntry.applyHasPermData();
        vectorEntry.action(VectorEntryActions.SET);
        vectorEntry.index(11);
        vectorEntry.permData(txt);
                                
        // 1. Negative test for buffer too small
        iter.setBufferAndRWFVersion(smallBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encode(iter) == CodecReturnCodes.BUFFER_TOO_SMALL);
        
        // 2. Try to grow the buffer to hit the edge cases
        int ret = CodecReturnCodes.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCodes.BUFFER_TOO_SMALL) && 
        		iter.setBufferAndRWFVersion(growByOneAndCopy(smallBuf), Codec.majorVersion(), Codec.minorVersion()) == CodecReturnCodes.SUCCESS)
        {
            ret = vectorEntry.encode(iter);     
        }
        assertTrue(ret == CodecReturnCodes.SUCCESS);

        // 3. Positive test for everything good without pre-encoded data
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        assertTrue(vectorEntry.encode(iter) == CodecReturnCodes.SUCCESS);

        byte[] expectedData =
        	{0x1B, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45,
        	 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56,
        	 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
        	 0x00, 0x12, 0x0B, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72,
        	 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0,
           	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData, bufBytes);
        assertEquals(74, buf.data().position());

        // 3. Positive test for everything good with pre-encoded data
        buf.data().rewind();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        vectorEntry.encodedData(txt);
        assertTrue(vectorEntry.encode(iter) == CodecReturnCodes.SUCCESS);

        byte[] expectedData2 =
        	{0x1B, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45,
        	 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56,
        	 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
        	 0x00, 0x12, 0x0B, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72,
        	 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65,
        	 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0};
        bufBytes = new byte[expectedData2.length];
        assertEquals(CodecReturnCodes.SUCCESS, buf.copy(bufBytes));

        assertArrayEquals(expectedData2, bufBytes);
        assertEquals(95, buf.data().position());
        
        // 4. Positive test for everything good with big pre-encoded data
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        ByteBuffer bb = bigBuf2.data();
        for (int i = 0; i < 65535; i++)
        {
        	bb.put((byte)i);
        }
        vectorEntry.clear();
        vectorEntry.encodedData(bigBuf2);
        assertTrue(vectorEntry.encode(iter) == CodecReturnCodes.SUCCESS);
        
        // 5. Negative test for everything good with too big pre-encoded data
        bigBuf.data().rewind();
        bigBuf2.data().rewind();
        iter.setBufferAndRWFVersion(bigBuf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(vector.encodeInit(iter, 0, 0) == CodecReturnCodes.SUCCESS);
        bb = bigBuf2.data();
        for (int i = 0; i < 65536; i++)
        {
        	bb.put((byte)i);
        }
        vectorEntry.clear();
        vectorEntry.encodedData(bigBuf2);
        assertTrue(vectorEntry.encode(iter) == CodecReturnCodes.INVALID_DATA);
    }
    
    /**
     * Encode a generic message and compare encoded contents with UPAC.
     */
    @Test
    public void encodeGenericMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        GenericMsg msg = (GenericMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory.createBuffer();
        Buffer position = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.GENERIC);
        msg.streamId(2146290601); // 0x7FEDCBA9
        msg.domainType(DomainTypes.LOGIN);
        msg.containerType(DataTypes.FIELD_LIST);
        msg.applyMessageComplete();

        /* set sequence number */
        msg.applyHasSeqNum();
        msg.seqNum(1234567890L);
        
        /* set secondary sequence number */
        msg.applyHasSecondarySeqNum();
        msg.secondarySeqNum(1122334455L);

        /* set part number */
        msg.applyHasPartNum();
        msg.partNum(12345);
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* set permData */
        msg.applyHasPermData();
        byte [] pd = {0x10, 0x11, 0x12, 0x13};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(pd));
        msg.permData(permData);

        /* set msgKey members */
        msg.applyHasMsgKey();
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasServiceId();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        msg.msgKey().serviceId(32639);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter));

        // encode (32) UINT 554433
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, uint));

        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, real));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData= ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/018_genericMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    /**
     * Encode an ack message and compare encoded contents with UPAC.
     */
    @Test
    public void encodeAckMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        AckMsg msg = (AckMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory.createBuffer();
        Buffer position = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.ACK);
        msg.streamId(2146290601); // 0x7FEDCBA9
        msg.domainType(DomainTypes.LOGIN);
        msg.containerType(DataTypes.FIELD_LIST);
        msg.ackId(12345);

        /* set sequence number */
        msg.applyHasSeqNum();
        msg.seqNum(1234567890L);
        
        /* set nak code */
        msg.applyHasNakCode();
        msg.nakCode(NakCodes.NOT_OPEN);

        /* set private stream flag */
        msg.applyPrivateStream();
        
        /* set ack text */
        msg.applyHasText();
        Buffer ackText = CodecFactory.createBuffer();
        ackText.data("ACK TEXT");
        msg.text(ackText);
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* set msgKey members */
        msg.applyHasMsgKey();
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasServiceId();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        msg.msgKey().serviceId(32639);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter));

        // encode (32) UINT 554433
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, uint));

        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, real));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData= ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/022_ackMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
    /**
     * Encode a post message and compare encoded contexts with UPAC.
     */
    @Test
    public void encodePostMsgTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        PostMsg msg = (PostMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory
                .createBuffer();
        Buffer position = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.POST);
        msg.streamId(2146290601); // 0x7FEDCBA9
        msg.domainType(DomainTypes.LOGIN);
        msg.containerType(DataTypes.FIELD_LIST);
        msg.applyAck();
        msg.applyPostComplete();
        
    	// set sequence number
        msg.applyHasSeqNum();
    	msg.seqNum(1234567890L);

    	// set post id
        msg.applyHasPostId();
        msg.postId(12345);
        
    	// set part number
    	msg.applyHasPartNum();
    	msg.partNum(23456);

    	// set post user rights
    	msg.applyHasPostUserRights();
    	msg.postUserRights(PostUserRights.MODIFY_PERM);
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* set permData */
        msg.applyHasPermData();
        byte [] pd = {0x10, 0x11, 0x12, 0x13};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(pd));
        msg.permData(permData);
        
        /* set post user info */
        msg.postUserInfo().userAddr(4294967290L); // 0xFFFFFFFA
        msg.postUserInfo().userId(4294967295L); // 0xFFFFFFFF

        /* set msgKey members */
        msg.applyHasMsgKey();
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasServiceId();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        msg.msgKey().serviceId(32639);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to useEncodeMsgInit
		 * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter));

        // encode (32) UINT 554433
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, uint));

        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, real));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData= ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/023_postMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf.data()));
    }
    
	/**
     * change buffer during encoding a generic message and compare encoded contents with UPAC.
     */
    @Test
    public void realignBufferTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        buf1.data(ByteBuffer.allocate(10));
        GenericMsg msg = (GenericMsg)CodecFactory.createMsg();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory.createBuffer();
        Buffer position = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* set-up message */
        msg.msgClass(MsgClasses.GENERIC);
        msg.streamId(2146290601); // 0x7FEDCBA9
        msg.domainType(DomainTypes.LOGIN);
        msg.containerType(DataTypes.FIELD_LIST);
        msg.applyMessageComplete();

        /* set sequence number */
        msg.applyHasSeqNum();
        msg.seqNum(1234567890L);
        
        /* set secondary sequence number */
        msg.applyHasSecondarySeqNum();
        msg.secondarySeqNum(1122334455L);

        /* set part number */
        msg.applyHasPartNum();
        msg.partNum(12345);
        
        /* extended header */
        msg.applyHasExtendedHdr();
        Buffer extendedHeader = CodecFactory.createBuffer();
        extendedHeader.data("EXTENDED HEADER");
        msg.extendedHeader(extendedHeader);

        /* set permData */
        msg.applyHasPermData();
        byte [] pd = {0x10, 0x11, 0x12, 0x13};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(pd));
        msg.permData(permData);

        /* set msgKey members */
        msg.applyHasMsgKey();
        msg.msgKey().applyHasAttrib();
        msg.msgKey().applyHasNameType();
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasServiceId();

        msg.msgKey().name().data("TRI.N");
        msg.msgKey().nameType(InstrumentNameTypes.RIC);
        msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        msg.msgKey().serviceId(32639);

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf1, Codec.majorVersion(), Codec.minorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
		 * EncodeMsgInit should return and inform us to encode our key opaque
         */
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, msg.encodeInit(encodeIter, 0));
        Buffer buf2 = CodecFactory.createBuffer();
        buf2.data(ByteBuffer.allocate(100));
        encodeIter.realignBuffer(buf2);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, msg.encodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        applicationId.data("256");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationId));

        /* ApplicationName */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        applicationName.data("rsslConsumer");
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, applicationName));

        /* Position */
        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        position.data("localhost");
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, element.encode(encodeIter, position));
        Buffer buf3 = CodecFactory.createBuffer();
        buf3.data(ByteBuffer.allocate(150));
        encodeIter.realignBuffer(buf3);
        assertEquals(CodecReturnCodes.SUCCESS, element.encode(encodeIter, position));

        /* complete encode element list */
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER,
                     msg.encodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.applyHasInfo();
        fieldList.dictionaryId(2);
        fieldList.fieldListNum(3);
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.fieldId(175);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        Buffer preEncoded = CodecFactory.createBuffer();
        preEncoded.data("ABCDEFG");
        fieldEntry.encodedData(preEncoded);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter));

        // encode (32) UINT 554433
        UInt uint = CodecFactory.createUInt();
        uint.value(554433);
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UINT);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(encodeIter, uint));
        Buffer buf4 = CodecFactory.createBuffer();
        buf4.data(ByteBuffer.allocate(200));
        encodeIter.realignBuffer(buf4);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, uint));

        // encode (111) REAL 867564 Exponent_4
        Real real = CodecFactory.createReal();
        real.value(867564, RealHints.EXPONENT_4);
        fieldEntry.fieldId(111);
        fieldEntry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(encodeIter, real));

        /* complete the encoding of the payload */
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));

        /* complete encode message */
        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        byte[] expectedData= ParseHexFile
                .parse("src/test/resources/com/thomsonreuters/upa/data/RsslEncodersJunit/018_genericMsg.txt");
        assertNotNull(expectedData);
        assertArrayEquals(expectedData, convertToByteArray(buf4.data()));
    }
    
    @Test
    public void encodeInNon0PosBufferTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(35), 3, 32);
        FilterList filterList = CodecFactory.createFilterList();
        EncodeIterator iter = CodecFactory.createEncodeIterator();
     
        filterList.applyHasPerEntryPermData();
        filterList.applyHasTotalCountHint();
        filterList.totalCountHint(5);
        Buffer txt = CodecFactory.createBuffer();
        txt.data("encodeFilterListInitTest");
        ((FilterListImpl)filterList).encodedEntries(txt);
        filterList.containerType(DataTypes.FIELD_LIST);
        
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        filterList.containerType(DataTypes.FIELD_LIST);
        
        // 3. Positive test for everything good
        assertTrue(filterList.encodeInit(iter) == CodecReturnCodes.SUCCESS);

    }
}
