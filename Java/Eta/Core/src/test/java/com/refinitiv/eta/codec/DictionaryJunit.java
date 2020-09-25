///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.nio.ByteBuffer;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportFactory;

public class DictionaryJunit
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
     * Load a field dictionary with boundary values and verify contents.
     */
    @Test
    public void loadFieldDictionaryTest()
    {
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadFieldDictionary(null, error));        
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadFieldDictionary("xyz", error));        
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryBoundary", error));
        assertEquals(8, dictionary.numberOfEntries());
        assertEquals(32767, dictionary.maxFid());
        assertEquals(-32768, dictionary.minFid());
        
        // verify tags
        assertArrayEquals("RWF.DAT".getBytes(), convertToByteArray(dictionary.infoFieldFilename().data()));
        assertArrayEquals("RDF-D RWF field set".getBytes(), convertToByteArray(dictionary.infoFieldDesc().data()));
        assertArrayEquals("4.10.11".getBytes(), convertToByteArray(dictionary.infoFieldVersion().data()));
        assertArrayEquals("1".getBytes(), convertToByteArray(dictionary.infoFieldBuild().data()));
        assertArrayEquals("01-Jun-2012".getBytes(), convertToByteArray(dictionary.infoFieldDate().data()));
        
        // verify contents
        // FID 1
        DictionaryEntry entry = dictionary.entry(1);
        assertNotNull(entry);
        assertArrayEquals("PROD_PERM".getBytes(), convertToByteArray(entry.acronym().data()));
        assertArrayEquals("PERMISSION".getBytes(), convertToByteArray(entry.ddeAcronym().data()));
        assertEquals(1, entry.fid());
        assertEquals(0, entry.rippleToField());
        assertEquals(MfFieldTypes.INTEGER, entry.fieldType());
        assertEquals(5, entry.length());
        assertEquals(0, entry.enumLength());
        assertEquals(DataTypes.UINT, entry.rwfType());
        assertEquals(2, entry.rwfLength());

        // FID 32767
        entry = dictionary.entry(32767);
        assertNotNull(entry);
        assertArrayEquals("MAX_FID".getBytes(), convertToByteArray(entry.acronym().data()));
        assertArrayEquals("MAX_FID".getBytes(), convertToByteArray(entry.ddeAcronym().data()));
        assertEquals(32767, entry.fid());
        assertEquals(0, entry.rippleToField());
        assertEquals(MfFieldTypes.ENUMERATED, entry.fieldType());
        assertEquals(3, entry.length());
        assertEquals(3, entry.enumLength());
        assertEquals(DataTypes.ENUM, entry.rwfType());
        assertEquals(1, entry.rwfLength());

        // FID -32768
        entry = dictionary.entry(-32768);
        assertNotNull(entry);
        assertArrayEquals("MIN_FID".getBytes(), convertToByteArray(entry.acronym().data()));
        assertArrayEquals("MIN_FID".getBytes(), convertToByteArray(entry.ddeAcronym().data()));
        assertEquals(-32768, entry.fid());
        assertEquals(0, entry.rippleToField());
        assertEquals(MfFieldTypes.ENUMERATED, entry.fieldType());
        assertEquals(3, entry.length());
        assertEquals(3, entry.enumLength());
        assertEquals(DataTypes.ENUM, entry.rwfType());
        assertEquals(1, entry.rwfLength());
        
        // FID 6
        entry = dictionary.entry(6);
        assertNotNull(entry);
        assertArrayEquals("TRDPRC_1".getBytes(), convertToByteArray(entry.acronym().data()));
        assertArrayEquals("LAST".getBytes(), convertToByteArray(entry.ddeAcronym().data()));
        assertEquals(6, entry.fid());
        assertEquals(7, entry.rippleToField());
        assertEquals(MfFieldTypes.PRICE, entry.fieldType());
        assertEquals(17, entry.length());
        assertEquals(0, entry.enumLength());
        assertEquals(DataTypes.REAL, entry.rwfType());
        assertEquals(7, entry.rwfLength());
}

    /**
     * Load a enum type dictionary with boundary values and verify contents.
     */
    @Test
    public void loadEnumTypeDictionaryTest()
    {
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        
        // load enumType dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadEnumTypeDictionary(null, error));        
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadEnumTypeDictionary("xyz", error));        
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("src/test/resources/com/refinitiv/eta/data/Codec/enumtypeBoundary.def", error));
        
        // verify tags
        assertArrayEquals("ENUMTYPE.001".getBytes(), convertToByteArray(dictionary.infoEnumFilename().data()));
        assertArrayEquals("IDN Marketstream enumerated tables".getBytes(), convertToByteArray(dictionary.infoEnumDesc().data()));
        assertArrayEquals("4.10.11".getBytes(), convertToByteArray(dictionary.infoEnumRTVersion().data()));
        assertArrayEquals("13.11".getBytes(), convertToByteArray(dictionary.infoEnumDTVersion().data()));
        assertArrayEquals("26-Jun-2012".getBytes(), convertToByteArray(dictionary.infoEnumDate().data()));
        
        // verify contents
        EnumTypeTable[] enumTypeTable = dictionary.enumTables();
        assertNotNull(enumTypeTable[0]);
        assertEquals(null, enumTypeTable[1]);
        EnumTypeTable enumTypeTableEntry = enumTypeTable[0];
        assertEquals(2, enumTypeTableEntry.maxValue());
        assertEquals(2, enumTypeTableEntry.fidReferenceCount());
        int[] fidRefs = enumTypeTableEntry.fidReferences();
        assertEquals(32767, fidRefs[0]);        
        assertEquals(-32768, fidRefs[1]);        
        EnumType[] enumTypes = enumTypeTableEntry.enumTypes();
        assertEquals(3, enumTypes.length);
        assertEquals(0, enumTypes[0].value());
        assertArrayEquals("   ".getBytes(), convertToByteArray(enumTypes[0].display().data()));
        assertEquals(1, enumTypes[1].value());
        assertArrayEquals("MAX".getBytes(), convertToByteArray(enumTypes[1].display().data()));
        assertEquals(2, enumTypes[2].value());
        assertArrayEquals("MIN".getBytes(), convertToByteArray(enumTypes[2].display().data()));
   }
    
    /**
     * Load a field and enum type dictionary with boundary values and verify enum types.
     */
    @Test
    public void verifyEnumTypesTest()
    {
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        
        // load field and enumType dictionaries
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("src/test/resources/com/refinitiv/eta/data/Codec/enumtypeBoundary.def", error));
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryBoundary", error));
               
        // verify enum types
        EnumType enumType = null;
        Enum tempEnum = CodecFactory.createEnum();

        // FID 1
        DictionaryEntry entry = dictionary.entry(1);
        tempEnum.value(0);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertEquals(null, enumType); // should be null since fid 1 type is not enum 

        // FID 32767
        entry = dictionary.entry(32767);
        tempEnum.value(0);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(0, enumType.value());
        assertArrayEquals("   ".getBytes(), convertToByteArray(enumType.display().data()));
        tempEnum.value(1);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(1, enumType.value());
        assertArrayEquals("MAX".getBytes(), convertToByteArray(enumType.display().data()));
        tempEnum.value(2);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(2, enumType.value());
        assertArrayEquals("MIN".getBytes(), convertToByteArray(enumType.display().data()));

        // FID -32768
        entry = dictionary.entry(-32768);
        tempEnum.value(0);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(0, enumType.value());
        assertArrayEquals("   ".getBytes(), convertToByteArray(enumType.display().data()));
        tempEnum.value(1);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(1, enumType.value());
        assertArrayEquals("MAX".getBytes(), convertToByteArray(enumType.display().data()));
        tempEnum.value(2);
        enumType = dictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(2, enumType.value());
        assertArrayEquals("MIN".getBytes(), convertToByteArray(enumType.display().data()));
    }

    /**
     * Load a field dictionary with FID of 0 and verify failure.
     */
    @Test
    public void loadZeroFidDictionaryTest()
    {
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryZeroFid", error));
    }

    /**
     * Load a field dictionary with invalid RWF type and verify failure.
     */
    @Test
    public void loadInvalidRWFTypeDictionaryTest()
    {
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryInvalidRWFType", error));
    }
    
    /**
     * Load a field dictionary with duplicate FIDs and verify failure.
     */
    @Test
    public void loadDuplicateFidDictionaryTest()
    {
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.FAILURE, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryDuplicateFid", error));
    }
    
    /**
     * Encode/decode complete field and enum type dictionaries and verify success.
     */
    @Test
    public void encodeDecodeDictionariesTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1000000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        DataDictionary decodedDictionary = CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode field dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
        
        /* decode field dictionary */
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        Int dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.FIELD_DEFINITIONS, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        // load enumType dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
        
        /* set-up message */
        msg.clear();
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Enum Type Dictionary Refresh");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFEnum");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(4);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        buf.data().clear();
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode enum type dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        /* decode enum type dictionary */
        msg.clear();
        len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.ENUM_TABLES, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
    }
    
    /**
     * Encode/decode complete field dictionary with INFO verbosity and verify success.
     */
    @Test
    public void encodeDecodeFieldDictionaryINFOTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1000000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        DataDictionary decodedDictionary = CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
        msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
        msg.state().streamState(StreamStates.OPEN);
        msg.state().dataState(DataStates.OK);
        msg.state().code(StateCodes.NONE);
        msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");      
        msg.applyHasMsgKey();
        msg.msgKey().filter(Dictionary.VerbosityValues.INFO);
        msg.msgKey().applyHasName();
        msg.msgKey().applyHasFilter();
        msg.msgKey().applyHasServiceId();
        msg.msgKey().serviceId(0);
        
        /* DictionaryName */
        nameBuf.data("RWFFld");
        msg.msgKey().name(nameBuf);

        /* StreamId */
        msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode field dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.INFO, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
        
        /* decode field dictionary */
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        Int dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.FIELD_DEFINITIONS, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.INFO, error));
    }
    
    /**
     * Decode invalid field and enum type dictionaries and verify failure.
     */
    @Test
    public void decodeInvalidDictionariesTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        Buffer bufSmall = CodecFactory.createBuffer();
        Buffer bufSmall2 = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1000000));
        bufSmall.data(ByteBuffer.allocate(10));
        bufSmall2.data(ByteBuffer.allocate(1000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        DataDictionary decodedDictionary = CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode field dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
        
        /* decode field dictionary with data not set on decode iterator */
        msg.clear();
        decodeIter.clear();
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, msg.decode(decodeIter));
        Int dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        /* decode field dictionary with truncated data on decode iterator */
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        for (int i = 0; i < bufSmall.data().capacity(); i++)
        {
        	bufSmall.data().put(i, buf.data().get(i));
        }
        bufSmall.data(bufSmall.data(), 0, bufSmall.data().capacity());
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(bufSmall, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        /* decode field dictionary with truncated data on decode iterator */
        msg.clear();
        len = buf.length();
        buf.data(buf.data(), 0, len);
        for (int i = 0; i < bufSmall2.data().capacity(); i++)
        {
        	bufSmall2.data().put(i, buf.data().get(i));
        }
        bufSmall2.data(bufSmall2.data(), 0, bufSmall2.data().capacity());
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(bufSmall2, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        /* decode field dictionary with invalid container type */
        msg.clear();
        buf.data().clear();
        buf.data().put(78, (byte)6);
        len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        // load enumType dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
        
        /* set-up message */
        msg.clear();
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Enum Type Dictionary Refresh");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFEnum");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(4);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        buf.data().clear();
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode enum type dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        /* decode enum type dictionary with data not set on decode iterator */
        msg.clear();
        len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        /* decode enum type dictionary with truncated data on decode iterator */
        msg.clear();
        len = buf.length();
        buf.data(buf.data(), 0, len);
        for (int i = 0; i < bufSmall.data().capacity(); i++)
        {
        	bufSmall.data().put(i, buf.data().get(i));
        }
        bufSmall.data(bufSmall.data(), 0, bufSmall.data().capacity());
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(bufSmall, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        /* decode enum type dictionary with truncated data on decode iterator */
        msg.clear();
        len = buf.length();
        buf.data(buf.data(), 0, len);
        for (int i = 0; i < bufSmall2.data().capacity(); i++)
        {
        	bufSmall2.data().put(i, buf.data().get(i));
        }
        bufSmall2.data(bufSmall2.data(), 0, bufSmall2.data().capacity());
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(bufSmall2, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        /* decode enum type dictionary with invalid container type */
        msg.clear();
        buf.data().clear();
        buf.data().put(61, (byte)6);
        len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
    }
    
    /**
     * Set up various mock objects for failure and verify encodeFieldDictionary() failure.
     */
    @Test
    public void encodeFieldDictionaryFailureTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionaryImpl dictionary = (DataDictionaryImpl)CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(1);
        
        // load field and enumType dictionaries
        dictionary.clear();
		assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
		
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));
        
        dictionary._isInitialized = false;
        assertEquals(CodecReturnCodes.FAILURE, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.NORMAL, error));
        dictionary._isInitialized = true;
        
        // create a mock Objects for testing
        SeriesImpl mockSeries = Mockito.mock(SeriesImpl.class);
        Mockito.stub(mockSeries.encodeInit((EncodeIterator)Mockito.anyObject(), Mockito.anyInt(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockSeries.encodeSetDefsComplete((EncodeIterator)Mockito.anyObject(), Mockito.anyBoolean())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.series = mockSeries;
        LocalElementSetDefDbImpl mockLocalElementSetDefDb = Mockito.mock(LocalElementSetDefDbImpl.class);
        Mockito.stub(mockLocalElementSetDefDb.encode((EncodeIterator)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockLocalElementSetDefDb.definitions()).toReturn(dictionary.setDb.definitions());
        dictionary.setDb = mockLocalElementSetDefDb;
        ElementListImpl mockElemList = Mockito.mock(ElementListImpl.class);
        Mockito.stub(mockElemList.encodeInit((EncodeIterator)Mockito.anyObject(), (LocalElementSetDefDb)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemList = mockElemList;
        SeriesEntryImpl mockSeriesEntry = Mockito.mock(SeriesEntryImpl.class);
        Mockito.stub(mockSeriesEntry.encodeInit((EncodeIterator)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.FAILURE);
        dictionary.seriesEntry = mockSeriesEntry;

        /* verify encodeFieldDictionary() failures */
        assertEquals(CodecReturnCodes.FAILURE, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.NORMAL, error));        
    }
    
    /**
     * Set up various mock objects for failure and verify encodeDataDictSummaryData() failure.
     */
    @Test
    public void encodeDataDictSummaryDataFailureTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionaryImpl dictionary = (DataDictionaryImpl)CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(1);
        
        // load field and enumType dictionaries
        dictionary.clear();
		assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));
        
        // create a mock Objects for testing
        ElementListImpl mockElemList = Mockito.mock(ElementListImpl.class);
        Mockito.stub(mockElemList.encodeInit((EncodeIterator)Mockito.anyObject(), (LocalElementSetDefDb)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemList = mockElemList;
        ElementEntryImpl mockElem = Mockito.mock(ElementEntryImpl.class);
        Mockito.stub(mockElem.encode((EncodeIterator)Mockito.anyObject(), (Int)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockElem.encode((EncodeIterator)Mockito.anyObject(), (Buffer)Mockito.anyObject())).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemEntry = mockElem;
        
        /* verify encodeDataDictSummaryData() failures */
        assertEquals(CodecReturnCodes.FAILURE, dictionary.encodeDataDictSummaryData(encodeIter, Dictionary.Types.FIELD_DEFINITIONS, dictionary.series, error));    
    }

    /**
     * Set up various mock objects for failure and verify encodeDataDictEntry() failure.
     */
    @Test
    public void encodeDataDictEntryFailureTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionaryImpl dictionary = (DataDictionaryImpl)CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(1);
        
        // load field and enumType dictionaries
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
		
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));
        
        // create a mock Objects for testing
        SeriesEntryImpl mockSeriesEntry = Mockito.mock(SeriesEntryImpl.class);
        Mockito.stub(mockSeriesEntry.encodeInit((EncodeIterator)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.seriesEntry = mockSeriesEntry;
        ElementListImpl mockElemList = Mockito.mock(ElementListImpl.class);
        Mockito.stub(mockElemList.encodeInit((EncodeIterator)Mockito.anyObject(), (LocalElementSetDefDb)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemList = mockElemList;
        ElementEntryImpl mockElem = Mockito.mock(ElementEntryImpl.class);
        Mockito.stub(mockElem.encode((EncodeIterator)Mockito.anyObject(), (Buffer)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockElem.encode((EncodeIterator)Mockito.anyObject(), (Int)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockElem.encode((EncodeIterator)Mockito.anyObject(), (UInt)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemEntry = mockElem;
        
        /* verify encodeDataDictEntry() failures */
        assertEquals(CodecReturnCodes.FAILURE, dictionary.encodeDataDictEntry(encodeIter, dictionary._entriesArray[32769], Dictionary.VerbosityValues.NORMAL, error, dictionary.setDb));      
    }
    
    /**
     * Set up various mock objects for failure and verify encodeEnumTypeDictionary() failure.
     */
    @Test
    public void encodeEnumTypeDictionaryFailureTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionaryImpl dictionary = (DataDictionaryImpl)CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(1);
        
        // load field and enumType dictionaries
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
		
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Enum Type Dictionary Refresh");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFEnum");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(4);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));
        
        dictionary._isInitialized = false;
        assertEquals(CodecReturnCodes.FAILURE, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));
        dictionary._isInitialized = true;
        
        // create a mock Objects for testing
        SeriesImpl mockSeries = Mockito.mock(SeriesImpl.class);
        Mockito.stub(mockSeries.encodeInit((EncodeIterator)Mockito.anyObject(), Mockito.anyInt(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockSeries.encodeSetDefsComplete((EncodeIterator)Mockito.anyObject(), Mockito.anyBoolean())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.series = mockSeries;
        LocalElementSetDefDbImpl mockLocalElementSetDefDb = Mockito.mock(LocalElementSetDefDbImpl.class);
        Mockito.stub(mockLocalElementSetDefDb.encode((EncodeIterator)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockLocalElementSetDefDb.definitions()).toReturn(dictionary.setDb.definitions());
        dictionary.setDb = mockLocalElementSetDefDb;
        ElementListImpl mockElemList = Mockito.mock(ElementListImpl.class);
        Mockito.stub(mockElemList.encodeInit((EncodeIterator)Mockito.anyObject(), (LocalElementSetDefDb)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemList = mockElemList;
        SeriesEntryImpl mockSeriesEntry = Mockito.mock(SeriesEntryImpl.class);
        Mockito.stub(mockSeriesEntry.encodeInit((EncodeIterator)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.seriesEntry = mockSeriesEntry;
        ElementEntryImpl mockElem = Mockito.mock(ElementEntryImpl.class);
        Mockito.stub(mockElem.encodeInit((EncodeIterator)Mockito.anyObject(), Mockito.anyInt())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.elemEntry = mockElem;
        ArrayImpl mockArray = Mockito.mock(ArrayImpl.class);
        Mockito.stub(mockArray.encodeInit((EncodeIterator)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.arr = mockArray;
        ArrayEntryImpl mockArrayEntry = Mockito.mock(ArrayEntryImpl.class);
        Mockito.stub(mockArrayEntry.encode((EncodeIterator)Mockito.anyObject(), (Int)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        dictionary.arrEntry = mockArrayEntry;
        Mockito.stub(mockArray.encodeComplete((EncodeIterator)Mockito.anyObject(), Mockito.anyBoolean())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockElem.encodeComplete((EncodeIterator)Mockito.anyObject(), Mockito.anyBoolean())).toReturn(CodecReturnCodes.FAILURE);

        /* verify encodeFieldDictionary() failures */
        assertEquals(CodecReturnCodes.FAILURE, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));
    }
    
    /**
     * Set up various mock objects for failure and verify decodeEnumTypeDictionary() failure.
     */
    @Test
    public void decodeEnumTypeDictionaryFailureTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1000000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionaryImpl dictionary = (DataDictionaryImpl)CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(1);
        
        // load field and enumType dictionaries
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("../../etc/RDMFieldDictionary", error));
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("../../etc/enumtype.def", error));
		
        /* set-up message */
        msg.clear();
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Enum Type Dictionary Refresh");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFEnum");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(4);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        buf.data().clear();
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode enum type dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        /* decode enum type dictionary */
        class DataDictionaryImplTest extends DataDictionaryImpl
        {
        	@Override
        	int decodeDictionaryTag(DecodeIterator iter, ElementEntryImpl element, int type, com.refinitiv.eta.transport.Error error)
            {
        		return CodecReturnCodes.FAILURE;
            }
        }
        DataDictionaryImpl decodedDictionary = new DataDictionaryImplTest();
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        
        ElementListImpl mockElemList = Mockito.mock(ElementListImpl.class);
        Mockito.stub(mockElemList.decode((DecodeIterator)Mockito.anyObject(), (LocalElementSetDefDb)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        decodedDictionary.elemList = mockElemList;
        ElementEntryImpl mockElem = Mockito.mock(ElementEntryImpl.class);
        Mockito.stub(mockElem.decode((DecodeIterator)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.END_OF_CONTAINER).toReturn(CodecReturnCodes.END_OF_CONTAINER).toReturn(CodecReturnCodes.END_OF_CONTAINER).toReturn(CodecReturnCodes.FAILURE).toReturn(CodecReturnCodes.END_OF_CONTAINER).toReturn(CodecReturnCodes.END_OF_CONTAINER).toReturn(CodecReturnCodes.END_OF_CONTAINER).toReturn(CodecReturnCodes.FAILURE);
        decodedDictionary.elemEntry = mockElem;
        LocalElementSetDefDbImpl mockLocalElementSetDefDb = Mockito.mock(LocalElementSetDefDbImpl.class);
        Mockito.stub(mockLocalElementSetDefDb.decode((DecodeIterator)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        Mockito.stub(mockLocalElementSetDefDb.definitions()).toReturn(dictionary.setDb.definitions());
        decodedDictionary.setDb = mockLocalElementSetDefDb;
        SeriesEntryImpl mockSeriesEntry = Mockito.mock(SeriesEntryImpl.class);
        Mockito.stub(mockSeriesEntry.decode((DecodeIterator)Mockito.anyObject())).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.SUCCESS).toReturn(CodecReturnCodes.FAILURE);
        decodedDictionary.seriesEntry = mockSeriesEntry;
        
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        buf.data().rewind();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        assertEquals(CodecReturnCodes.FAILURE, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
    }

    // copy encoded data into byte[]
    private byte[] convertToByteArray(ByteBuffer bb)
    {
        byte[] ba = new byte[bb.limit()];
        bb.get(ba);
        return ba;
    }
}
