///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.transport.*;
import com.thomsonreuters.upa.transport.Error;

import org.junit.Test;

public class FieldSetDefDbJunit
{

    @Test
    public void fieldSetDefEncDecTest()
    {
        int i, j;
        
        Int fid = CodecFactory.createInt();
        Int type = CodecFactory.createInt();
        Int currentSetDef = CodecFactory.createInt();
        
        Int tempInt = CodecFactory.createInt();
        
        Buffer versionBuffer = CodecFactory.createBuffer();

        FieldSetDefEntry[] setEntryArray;
        FieldSetDef[] setArray;
        
        
        GlobalFieldSetDefDb encDb = CodecFactory.createGlobalFieldSetDefDb(), decDb = CodecFactory.createGlobalFieldSetDefDb();
        
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));
        
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        
        Error error = TransportFactory.createError();
        
        setEntryArray = new FieldSetDefEntry[8];
        

        setEntryArray[0] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[0].fieldId(1);
        setEntryArray[0].dataType(DataTypes.INT);
        
        
        setEntryArray[1] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[1].fieldId(2);
        setEntryArray[1].dataType(DataTypes.DOUBLE);
        
        
        setEntryArray[2] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[2].fieldId(3);
        setEntryArray[2].dataType(DataTypes.REAL);
        
        
        setEntryArray[3] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[3].fieldId(4);
        setEntryArray[3].dataType(DataTypes.DATE);
        
        fid.value(5);
        type.value(DataTypes.TIME);
        setEntryArray[4] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[4].fieldId(5);
        setEntryArray[4].dataType(DataTypes.TIME);
        
        fid.value(6);
        type.value(DataTypes.DATETIME);
        setEntryArray[5] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[5].fieldId(6);
        setEntryArray[5].dataType(DataTypes.DATETIME);
        
        setEntryArray[6] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[6].fieldId(7);
        setEntryArray[6].dataType(DataTypes.ARRAY);
        
        setEntryArray[7] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[7].fieldId(8);
        setEntryArray[7].dataType(DataTypes.UINT);
        
        setArray = new FieldSetDef[8];
        
        setArray[0] = CodecFactory.createFieldSetDef();
        setArray[0].entries(setEntryArray);
        tempInt.value(1);
        setArray[0].count(1);
        setArray[0].setId(16);
        
        setArray[1] = CodecFactory.createFieldSetDef();
        setArray[1].entries(setEntryArray);
        tempInt.value(2);
        setArray[1].count(2);
        setArray[1].setId(17);
        
        setArray[2] = CodecFactory.createFieldSetDef();
        setArray[2].entries(setEntryArray);
        tempInt.value(3);
        setArray[2].count(3);
        setArray[2].setId(18);
        
        setArray[3] = CodecFactory.createFieldSetDef();
        setArray[3].entries(setEntryArray);
        tempInt.value(4);
        setArray[3].count(4);
        setArray[3].setId(19);
        
        setArray[4] = CodecFactory.createFieldSetDef();
        setArray[4].entries(setEntryArray);
        tempInt.value(1);
        setArray[4].count(5);
        setArray[4].setId(20);
        
        setArray[5] = CodecFactory.createFieldSetDef();
        setArray[5].entries(setEntryArray);
        tempInt.value(1);
        setArray[5].count(6);
        setArray[5].setId(21);
        
        setArray[6] = CodecFactory.createFieldSetDef();
        setArray[6].entries(setEntryArray);
        tempInt.value(1);
        setArray[6].count(7);
        setArray[6].setId(22);
        
        setArray[7] = CodecFactory.createFieldSetDef();
        setArray[7].entries(setEntryArray);
        tempInt.value(1);
        setArray[7].count(8);
        setArray[7].setId(23);
        
        versionBuffer.data("1.0.1");
        
        for(i = 0; i < 8; i++)
        {
            assertEquals(encDb.addSetDef(setArray[i], error), CodecReturnCodes.SUCCESS);
        }

        encDb.info_DictionaryID(10);
        encDb.info_version(versionBuffer);
        
        buf.data().position(0);
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        currentSetDef.value(0);
        assertEquals(CodecReturnCodes.SUCCESS, encDb.encode(encIter, currentSetDef, Dictionary.VerbosityValues.VERBOSE, error));
        assertEquals(24, (int)currentSetDef.toLong());

        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, decDb.decode(decIter, Dictionary.VerbosityValues.VERBOSE, error));
        assertTrue(versionBuffer.equals(decDb.info_version()));
        assertEquals(10, decDb.info_DictionaryID());
        
        for(i = 0; i < 8; i++)
        {
            assertTrue(decDb.definitions()[(int)setArray[i].setId()].setId() == setArray[i].setId());
            assertTrue(decDb.definitions()[(int)setArray[i].setId()].count() == setArray[i].count());
            for(j = 0; j < decDb.definitions()[(int)setArray[i].setId()].count(); j++)
            {
                assertTrue(decDb.definitions()[(int)setArray[i].setId()].entries()[j].fieldId() == setArray[i].entries()[j].fieldId());
                assertTrue(decDb.definitions()[(int)setArray[i].setId()].entries()[j].dataType() == setArray[i].entries()[j].dataType());
            }
        }
    }

    @Test
    public void fieldSetEncDecTest()
    {
        FieldList encList = CodecFactory.createFieldList(), decList = CodecFactory.createFieldList();
        FieldEntry encEntry = CodecFactory.createFieldEntry(), decEntry = CodecFactory.createFieldEntry();
        
        Int encInt = CodecFactory.createInt(), decInt = CodecFactory.createInt();
        UInt encUInt = CodecFactory.createUInt(), decUInt = CodecFactory.createUInt();
        Real encReal = CodecFactory.createReal(), decReal = CodecFactory.createReal();
        Buffer encBuf = CodecFactory.createBuffer();
        Date encDate = CodecFactory.createDate(), decDate = CodecFactory.createDate();
        Time encTime = CodecFactory.createTime(), decTime = CodecFactory.createTime();
        DateTime encDateTime = CodecFactory.createDateTime(), decDateTime = CodecFactory.createDateTime();
        Double encDouble = CodecFactory.createDouble(), decDouble = CodecFactory.createDouble();
        
        
        encInt.value(-2000);
        encUInt.value(2500);
        encReal.value(15000, RealHints.EXPONENT_3);
        encBuf.data("test");
        encDate.value("25 JAN 2015");
        encTime.value("12:10:5");
        encDateTime.value("25 JAN 2015 12:10:5");
        encDouble.value(1.23);
        
        int i;
        
        Int tempInt = CodecFactory.createInt();
        
        Buffer versionBuffer = CodecFactory.createBuffer();

        FieldSetDefEntry[] setEntryArray;
        FieldSetDef[] setArray;
        
        
        GlobalFieldSetDefDb encDb = CodecFactory.createGlobalFieldSetDefDb();
        
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));
        
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        
        Error error = TransportFactory.createError();
        
        setEntryArray = new FieldSetDefEntry[7];
        
        setEntryArray[0] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[0].fieldId(1);
        setEntryArray[0].dataType(DataTypes.INT);
        
        
        setEntryArray[1] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[1].fieldId(2);
        setEntryArray[1].dataType(DataTypes.DOUBLE);
        
        
        setEntryArray[2] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[2].fieldId(3);
        setEntryArray[2].dataType(DataTypes.REAL);
        
        
        setEntryArray[3] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[3].fieldId(4);
        setEntryArray[3].dataType(DataTypes.DATE);
        
        setEntryArray[4] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[4].fieldId(5);
        setEntryArray[4].dataType(DataTypes.TIME);
        
        setEntryArray[5] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[5].fieldId(6);
        setEntryArray[5].dataType(DataTypes.DATETIME);
        
        setEntryArray[6] = CodecFactory.createFieldSetDefEntry();
        setEntryArray[6].fieldId(7);
        setEntryArray[6].dataType(DataTypes.UINT);
        
        setArray = new FieldSetDef[8];
        
        setArray[0] = CodecFactory.createFieldSetDef();
        setArray[0].entries(setEntryArray);
        tempInt.value(1);
        setArray[0].count(1);
        setArray[0].setId(16);
        
        setArray[1] = CodecFactory.createFieldSetDef();
        setArray[1].entries(setEntryArray);
        tempInt.value(2);
        setArray[1].count(2);
        setArray[1].setId(17);
        
        setArray[2] = CodecFactory.createFieldSetDef();
        setArray[2].entries(setEntryArray);
        tempInt.value(3);
        setArray[2].count(3);
        setArray[2].setId(18);
        
        setArray[3] = CodecFactory.createFieldSetDef();
        setArray[3].entries(setEntryArray);
        tempInt.value(4);
        setArray[3].count(4);
        setArray[3].setId(19);
        
        setArray[4] = CodecFactory.createFieldSetDef();
        setArray[4].entries(setEntryArray);
        tempInt.value(5);
        setArray[4].count(5);
        setArray[4].setId(20);
        
        setArray[5] = CodecFactory.createFieldSetDef();
        setArray[5].entries(setEntryArray);
        tempInt.value(6);
        setArray[5].count(6);
        setArray[5].setId(21);

        setArray[6] = CodecFactory.createFieldSetDef();
        setArray[6].entries(setEntryArray);
        tempInt.value(7);
        setArray[6].count(7);
        setArray[6].setId(22);
        
        versionBuffer.data("1.0.1");
        
        for(i = 0; i < 7; i++)
        {
            assertEquals(encDb.addSetDef(setArray[i], error), CodecReturnCodes.SUCCESS);
        }

        encDb.info_DictionaryID(10);
        encDb.info_version(versionBuffer);
        
        for(i = 0; i <= 6; i++)
        {
            buf.data().rewind();
            
            encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
            encIter.setGlobalFieldSetDefDb(encDb);
            
            encList.clear();
            encList.applyHasSetData();
            encList.applyHasSetId();
            
            encList.setId(i+16);
            
            if(i < 6)
                encList.applyHasStandardData();
            
            assertEquals(CodecReturnCodes.SUCCESS, encList.encodeInit(encIter, null, 0));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.INT);
            encEntry.fieldId(1);
            
            assertEquals((i == 0 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encInt));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.DOUBLE);
            encEntry.fieldId(2);
            
            assertEquals((i == 1 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encDouble));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.REAL);
            encEntry.fieldId(3);
            
            assertEquals((i == 2 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encReal));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.DATE);
            encEntry.fieldId(4);
            
            assertEquals((i == 3 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encDate));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.TIME);
            encEntry.fieldId(5);
            
            assertEquals((i == 4 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encTime));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.DATETIME);
            encEntry.fieldId(6);
            
            assertEquals((i == 5 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encDateTime));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.UINT);
            encEntry.fieldId(7);
            
            assertEquals((i == 6 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encUInt));
            
            assertEquals(CodecReturnCodes.SUCCESS, encList.encodeComplete(encIter, true));
            
            decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
            decIter.setGlobalFieldSetDefDb(encDb);
            
            decList.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decList.decode(decIter, null));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[0].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decInt.decode(decIter));
            assertTrue(decInt.equals(encInt));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[1].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decDouble.decode(decIter));
            assertTrue(decDouble.equals(encDouble));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[2].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decReal.decode(decIter));
            assertTrue(decReal.equals(encReal));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[3].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decDate.decode(decIter));
            assertTrue(decDate.equals(encDate));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[4].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decTime.decode(decIter));
            assertTrue(decTime.equals(encTime));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[5].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decDateTime.decode(decIter));
            assertTrue(decDateTime.equals(encDateTime));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertEquals(setEntryArray[6].fieldId(), decEntry.fieldId());
            
            assertEquals(CodecReturnCodes.SUCCESS, decUInt.decode(decIter));
            assertTrue(decUInt.equals(encUInt));
            
        }
    }
}