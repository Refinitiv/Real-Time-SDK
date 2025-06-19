/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;

public class ElementSetDefDbJunit
{

    @Test
    public void elementSetDefEncDecTest()
    {
        int i, j;
        
        Int currentSetDef = CodecFactory.createInt();
        
        Int tempInt = CodecFactory.createInt();
        
        Buffer versionBuffer = CodecFactory.createBuffer();

        ElementSetDefEntry[] setEntryArray;
        ElementSetDef[] setArray;
        
        
        GlobalElementSetDefDb encDb = CodecFactory.createGlobalElementSetDefDb(), decDb = CodecFactory.createGlobalElementSetDefDb();
        
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));
        Buffer nameBuffer = CodecFactory.createBuffer();
        
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        
        Error error = TransportFactory.createError();
        
        setEntryArray = new ElementSetDefEntry[8];
        

        setEntryArray[0] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("INT");
        setEntryArray[0].name(nameBuffer);
        setEntryArray[0].dataType(DataTypes.INT);
        
        
        setEntryArray[1] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("DOUBLE");
        setEntryArray[1].name(nameBuffer);
        setEntryArray[1].dataType(DataTypes.DOUBLE);
        
        
        setEntryArray[2] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("REAL");
        setEntryArray[2].name(nameBuffer);
        setEntryArray[2].dataType(DataTypes.REAL);
        
        
        setEntryArray[3] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("DATE");
        setEntryArray[3].name(nameBuffer);
        setEntryArray[3].dataType(DataTypes.DATE);
        
        setEntryArray[4] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("TIME");
        setEntryArray[4].name(nameBuffer);
        setEntryArray[4].dataType(DataTypes.TIME);
        
        setEntryArray[5] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("DATETIME");
        setEntryArray[5].name(nameBuffer);
        setEntryArray[5].dataType(DataTypes.DATETIME);
        
        setEntryArray[6] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("ARRAY");
        setEntryArray[6].name(nameBuffer);
        setEntryArray[6].dataType(DataTypes.ARRAY);
        
        setEntryArray[7] = CodecFactory.createElementSetDefEntry();
        nameBuffer.data("UINT");
        setEntryArray[7].name(nameBuffer);
        setEntryArray[7].dataType(DataTypes.UINT);
        
        setArray = new ElementSetDef[8];
        
        setArray[0] = CodecFactory.createElementSetDef();
        setArray[0].entries(setEntryArray);
        tempInt.value(1);
        setArray[0].count(1);
        setArray[0].setId(16);
        
        setArray[1] = CodecFactory.createElementSetDef();
        setArray[1].entries(setEntryArray);
        tempInt.value(2);
        setArray[1].count(2);
        setArray[1].setId(17);
        
        setArray[2] = CodecFactory.createElementSetDef();
        setArray[2].entries(setEntryArray);
        tempInt.value(3);
        setArray[2].count(3);
        setArray[2].setId(18);
        
        setArray[3] = CodecFactory.createElementSetDef();
        setArray[3].entries(setEntryArray);
        tempInt.value(4);
        setArray[3].count(4);
        setArray[3].setId(19);
        
        setArray[4] = CodecFactory.createElementSetDef();
        setArray[4].entries(setEntryArray);
        tempInt.value(1);
        setArray[4].count(5);
        setArray[4].setId(20);
        
        setArray[5] = CodecFactory.createElementSetDef();
        setArray[5].entries(setEntryArray);
        tempInt.value(1);
        setArray[5].count(6);
        setArray[5].setId(21);
        
        setArray[6] = CodecFactory.createElementSetDef();
        setArray[6].entries(setEntryArray);
        tempInt.value(1);
        setArray[6].count(7);
        setArray[6].setId(22);
        
        setArray[7] = CodecFactory.createElementSetDef();
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
            assertTrue(decDb.definitions()[setArray[i].setId()].setId() == setArray[i].setId());
            assertTrue(decDb.definitions()[setArray[i].setId()].count() == setArray[i].count());
            for(j = 0; j < decDb.definitions()[setArray[i].setId()].count(); j++)
            {
                assertTrue(decDb.definitions()[setArray[i].setId()].entries()[j].name().equals(setArray[i].entries()[j].name()));
                assertTrue(decDb.definitions()[setArray[i].setId()].entries()[j].dataType() == setArray[i].entries()[j].dataType());
            }
        }
    }

    @Test
    public void elementSetEncDecTest()
    {
        ElementList encList = CodecFactory.createElementList(), decList = CodecFactory.createElementList();
        ElementEntry encEntry = CodecFactory.createElementEntry(), decEntry = CodecFactory.createElementEntry();
        
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

        ElementSetDefEntry[] setEntryArray;
        ElementSetDef[] setArray;
        
        
        GlobalElementSetDefDb encDb = CodecFactory.createGlobalElementSetDefDb();
        
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));
        
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        
        Error error = TransportFactory.createError();
        
        setEntryArray = new ElementSetDefEntry[7];
        
        setEntryArray[0] = CodecFactory.createElementSetDefEntry();
        setEntryArray[0].name().data("INT");
        setEntryArray[0].dataType(DataTypes.INT);
        
        
        setEntryArray[1] = CodecFactory.createElementSetDefEntry();
        setEntryArray[1].name().data("DOUBLE");
        setEntryArray[1].dataType(DataTypes.DOUBLE);
        
        
        setEntryArray[2] = CodecFactory.createElementSetDefEntry();
        setEntryArray[2].name().data("REAL");
        setEntryArray[2].dataType(DataTypes.REAL);
        
        
        setEntryArray[3] = CodecFactory.createElementSetDefEntry();
        setEntryArray[3].name().data("DATE");
        setEntryArray[3].dataType(DataTypes.DATE);
        
        setEntryArray[4] = CodecFactory.createElementSetDefEntry();
        setEntryArray[4].name().data("TIME");
        setEntryArray[4].dataType(DataTypes.TIME);
        
        setEntryArray[5] = CodecFactory.createElementSetDefEntry();
        setEntryArray[5].name().data("DATETIME");
        setEntryArray[5].dataType(DataTypes.DATETIME);
        
        setEntryArray[6] = CodecFactory.createElementSetDefEntry();
        setEntryArray[6].name().data("UINT");
        setEntryArray[6].dataType(DataTypes.UINT);
        
        setArray = new ElementSetDef[8];
        
        setArray[0] = CodecFactory.createElementSetDef();
        setArray[0].entries(setEntryArray);
        tempInt.value(1);
        setArray[0].count(1);
        setArray[0].setId(16);
        
        setArray[1] = CodecFactory.createElementSetDef();
        setArray[1].entries(setEntryArray);
        tempInt.value(2);
        setArray[1].count(2);
        setArray[1].setId(17);
        
        setArray[2] = CodecFactory.createElementSetDef();
        setArray[2].entries(setEntryArray);
        tempInt.value(3);
        setArray[2].count(3);
        setArray[2].setId(18);
        
        setArray[3] = CodecFactory.createElementSetDef();
        setArray[3].entries(setEntryArray);
        tempInt.value(4);
        setArray[3].count(4);
        setArray[3].setId(19);
        
        setArray[4] = CodecFactory.createElementSetDef();
        setArray[4].entries(setEntryArray);
        tempInt.value(5);
        setArray[4].count(5);
        setArray[4].setId(20);
        
        setArray[5] = CodecFactory.createElementSetDef();
        setArray[5].entries(setEntryArray);
        tempInt.value(6);
        setArray[5].count(6);
        setArray[5].setId(21);

        setArray[6] = CodecFactory.createElementSetDef();
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
            encIter.setGlobalElementSetDefDb(encDb);
            
            encList.clear();
            encList.applyHasSetData();
            encList.applyHasSetId();
            
            encList.setId(i+16);
            
            if(i < 6)
                encList.applyHasStandardData();
            
            assertEquals(CodecReturnCodes.SUCCESS, encList.encodeInit(encIter, null, 0));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.INT);
            encEntry.name().data("INT");
            
            assertEquals((i == 0 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encInt));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.DOUBLE);
            encEntry.name().data("DOUBLE");
            
            assertEquals((i == 1 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encDouble));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.REAL);
            encEntry.name().data("REAL");
            
            assertEquals((i == 2 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encReal));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.DATE);
            encEntry.name().data("DATE");
            
            assertEquals((i == 3 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encDate));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.TIME);
            encEntry.name().data("TIME");
            
            assertEquals((i == 4 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encTime));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.DATETIME);
            encEntry.name().data("DATETIME");
            
            assertEquals((i == 5 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encDateTime));
            
            encEntry.clear();
            encEntry.dataType(DataTypes.UINT);
            encEntry.name().data("UINT");
            
            assertEquals((i == 6 ? CodecReturnCodes.SET_COMPLETE : CodecReturnCodes.SUCCESS), encEntry.encode(encIter, encUInt));
            
            assertEquals(CodecReturnCodes.SUCCESS, encList.encodeComplete(encIter, true));
            
            decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
            decIter.setGlobalElementSetDefDb(encDb);
            
            decList.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decList.decode(decIter, null));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[0].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decInt.decode(decIter));
            assertTrue(decInt.equals(encInt));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[1].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decDouble.decode(decIter));
            assertTrue(decDouble.equals(encDouble));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[2].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decReal.decode(decIter));
            assertTrue(decReal.equals(encReal));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[3].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decDate.decode(decIter));
            assertTrue(decDate.equals(encDate));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[4].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decTime.decode(decIter));
            assertTrue(decTime.equals(encTime));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[5].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decDateTime.decode(decIter));
            assertTrue(decDateTime.equals(encDateTime));
            
            decEntry.clear();
            assertEquals(CodecReturnCodes.SUCCESS, decEntry.decode(decIter));
            assertTrue(setEntryArray[6].name().equals(decEntry.name()));
            
            assertEquals(CodecReturnCodes.SUCCESS, decUInt.decode(decIter));
            assertTrue(decUInt.equals(encUInt));
            
        }
    }
}
