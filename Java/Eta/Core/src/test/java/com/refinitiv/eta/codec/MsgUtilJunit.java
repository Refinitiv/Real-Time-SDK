///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.UpdateEventTypes;
import com.refinitiv.eta.rdm.ViewTypes;

public class MsgUtilJunit 
{

    @Test
    public void keyMessageTest()
    {
        MsgKey key1 = CodecFactory.createMsgKey();
        MsgKey key2 = CodecFactory.createMsgKey();
        Buffer name1 = CodecFactory.createBuffer();
        name1.data("etaj");
        Buffer name2 = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(7);
        byte[] bts = { 1, 2, 0, 10, 57, 0x7F, 4 };
        bb.put(bts);
        name2.data(bb, 0, 7);
        Buffer attrib = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(4);
        byte[] bts1 = { 4, 8, 5, 10 };
        bb1.put(bts1);
        name2.data(bb1, 0, 4);

        key1.applyHasFilter();
        key1.filter(67);
        key1.applyHasName();
        key1.name(name1);
        key1.applyHasAttrib();
        key1.encodedAttrib(attrib);
        key1.applyHasIdentifier();
        key1.identifier(7);
        key1.applyHasNameType();
        key1.nameType(4);
        key1.applyHasServiceId();
        key1.serviceId(667);
        key1.attribContainerType();

        key1.copy(key2);
        assertEquals(true, key1.equals(key2));
        assertEquals(true, key2.equals(key1));


       //  test the Name Type Scenarios
        /*
         *  MsgKey1.nametype	MsgKey2.NameType	Result
		  1 Not specified       Not Specified       Equal
		  2	Not Specified       1                   Equal
		  3	1                   Not Specified       Equal
		  4	1                   1                   Equal
		  5	X                   X                   Equal
		  6	X                   Y                   NOT Equal
		  7	X                   Not Specified       NOT Equal
		  8	Not Specified       X                   NOT Equal
         */
        // 1
        key1.flags(key1.flags()& ~MsgKeyFlags.HAS_NAME_TYPE);  //  remove the Name Type from the Key
        key2.flags(key2.flags()& ~MsgKeyFlags.HAS_NAME_TYPE);  //  remove the Name Type from the Key
        assertEquals(true, key1.equals(key2));
        
        // 2
        key2.applyHasNameType();
        key2.nameType(1);        
        assertEquals(true, key1.equals(key2));

        // 3
        key1.applyHasNameType();
        key1.nameType(1);        
        key2.flags(key2.flags()& ~MsgKeyFlags.HAS_NAME_TYPE);  //  remove the Name Type from the Key
        assertEquals(true, key1.equals(key2));
        
        // 4
        //  key1 is set from the last test
        key2.applyHasNameType();
        key2.nameType(1);        
        assertEquals(true, key1.equals(key2));
        
        // 5
        key1.applyHasNameType();
        key1.nameType(4);        
        key2.applyHasNameType();
        key2.nameType(4);        
        assertEquals(true, key1.equals(key2));
        
        // 6 
        //  key 1 set from above       
        key2.applyHasNameType();
        key2.nameType(6);        
        assertEquals(false, key1.equals(key2));
        
        // 7
        //  key 1 set from above       
        key2.flags(key2.flags()& ~MsgKeyFlags.HAS_NAME_TYPE);  //  remove the Name Type from the Key
        assertEquals(false, key1.equals(key2));

        // 8 
        key1.flags(key1.flags()& ~MsgKeyFlags.HAS_NAME_TYPE);  //  remove the Name Type from the Key
        key2.applyHasNameType();
        key2.nameType(4);        
        assertEquals(false, key1.equals(key2));
        
               
        // reset them to be equal again
        key1.applyHasNameType();
        key1.nameType(4);
        key1.copy(key2);  

        key2.name(name2);
        assertEquals(false, key1.equals(key2));
        assertEquals(false, key2.equals(key1));
        
        key2.name(name1);
        key1.flags(7);
        assertEquals(false, key1.equals(key2));
        assertEquals(false, key2.equals(key1));
        
        key1.flags(0);
        assertEquals(CodecReturnCodes.FAILURE, key1.addFilterId(7));
        assertEquals(false, key1.checkHasFilterId(7));
        key1.applyHasFilter();
        key1.filter(0);
        assertEquals(CodecReturnCodes.INVALID_DATA, key1.addFilterId(32));
        assertEquals(false, key1.checkHasFilterId(32));
        assertEquals(CodecReturnCodes.SUCCESS, key1.addFilterId(20));
        assertEquals(true, key1.checkHasFilterId(20));
        assertEquals(false, key1.checkHasFilterId(32));
        assertEquals(false, key1.checkHasFilterId(7));
    }
    
    @Test
    public void copyUpdateMessageTest()
    {
        UpdateMsg msg = (UpdateMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));
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
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);
        
    	UpdateMsg updCopyMsg = (UpdateMsg) CodecFactory.createMsg();
    	msg.copy(updCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(updCopyMsg.containerType(), msg.containerType());
    	assertEquals(updCopyMsg.domainType(), msg.domainType());
    	assertEquals(updCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(updCopyMsg.streamId(), msg.streamId());
    	assertEquals(updCopyMsg.checkDiscardable(), msg.checkDiscardable());
    	assertEquals(updCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(updCopyMsg.checkDoNotConflate(), msg.checkDoNotConflate());
    	assertEquals(updCopyMsg.checkDoNotRipple(), msg.checkDoNotRipple());
    	assertEquals(updCopyMsg.checkHasConfInfo(), msg.checkHasConfInfo());
    	assertEquals(true, msg.checkHasPostUserInfo());
    	assertEquals(updCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(updCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(updCopyMsg.conflationCount(), msg.conflationCount());
    	assertEquals(updCopyMsg.conflationTime(), msg.conflationTime());
    	assertEquals(updCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(updCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(updCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(updCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(updCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(true, updCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, updCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, updCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, updCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, updCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	assertEquals(true, updCopyMsg.encodedDataBody().equals(msg.encodedDataBody()));
    	
    	msg.copy(updCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(updCopyMsg.containerType(), msg.containerType());
    	assertEquals(updCopyMsg.domainType(), msg.domainType());
    	assertEquals(updCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(updCopyMsg.streamId(), msg.streamId());
    	assertEquals(updCopyMsg.checkDiscardable(), msg.checkDiscardable());
    	assertEquals(updCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(updCopyMsg.checkDoNotConflate(), msg.checkDoNotConflate());
    	assertEquals(updCopyMsg.checkDoNotRipple(), msg.checkDoNotRipple());
    	assertEquals(updCopyMsg.checkHasConfInfo(), msg.checkHasConfInfo());
    	assertEquals(updCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(updCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(updCopyMsg.conflationCount(), msg.conflationCount());
    	assertEquals(updCopyMsg.conflationTime(), msg.conflationTime());
    	assertEquals(updCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(updCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(false, updCopyMsg.checkHasExtendedHdr());
    	assertEquals(updCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(false, updCopyMsg.checkHasPermData());
    	assertEquals(false, updCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(false, updCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	
    	msg.copy(updCopyMsg, CopyMsgFlags.MSG_BUFFER);
    	assertEquals(updCopyMsg.containerType(), msg.containerType());
    	assertEquals(updCopyMsg.domainType(), msg.domainType());
    	assertEquals(updCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(updCopyMsg.streamId(), msg.streamId());
    	assertEquals(updCopyMsg.checkDiscardable(), msg.checkDiscardable());
    	assertEquals(updCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(updCopyMsg.checkDoNotConflate(), msg.checkDoNotConflate());
    	assertEquals(updCopyMsg.checkDoNotRipple(), msg.checkDoNotRipple());
    	assertEquals(updCopyMsg.checkHasConfInfo(), msg.checkHasConfInfo());
    	assertEquals(updCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(updCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(updCopyMsg.conflationCount(), msg.conflationCount());
    	assertEquals(updCopyMsg.conflationTime(), msg.conflationTime());
    	assertEquals(updCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(updCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(true, updCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	assertEquals(false, updCopyMsg.checkHasExtendedHdr());
    	assertEquals(true, updCopyMsg.checkHasMsgKey());
    	assertEquals(false, updCopyMsg.msgKey().checkHasName());
    	assertEquals(false, updCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, updCopyMsg.checkHasPermData());
    	
    	msg.copy(updCopyMsg, CopyMsgFlags.EXTENDED_HEADER | CopyMsgFlags.KEY);
    	assertEquals(updCopyMsg.containerType(), msg.containerType());
    	assertEquals(updCopyMsg.domainType(), msg.domainType());
    	assertEquals(updCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(updCopyMsg.streamId(), msg.streamId());
    	assertEquals(updCopyMsg.checkDiscardable(), msg.checkDiscardable());
    	assertEquals(updCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(updCopyMsg.checkDoNotConflate(), msg.checkDoNotConflate());
    	assertEquals(updCopyMsg.checkDoNotRipple(), msg.checkDoNotRipple());
    	assertEquals(updCopyMsg.checkHasConfInfo(), msg.checkHasConfInfo());
    	assertEquals(updCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(updCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(updCopyMsg.conflationCount(), msg.conflationCount());
    	assertEquals(updCopyMsg.conflationTime(), msg.conflationTime());
    	assertEquals(updCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(updCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(updCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(updCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(false, updCopyMsg.checkHasPermData());
    	assertEquals(true, updCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, updCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, updCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(0, updCopyMsg.encodedMsgBuffer().length());
    	
    	msg.copy(updCopyMsg, CopyMsgFlags.PERM_DATA                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | CopyMsgFlags.KEY_NAME);
    	assertEquals(updCopyMsg.containerType(), msg.containerType());
    	assertEquals(updCopyMsg.domainType(), msg.domainType());
    	assertEquals(updCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(updCopyMsg.streamId(), msg.streamId());
    	assertEquals(updCopyMsg.checkDiscardable(), msg.checkDiscardable());
    	assertEquals(updCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(updCopyMsg.checkDoNotConflate(), msg.checkDoNotConflate());
    	assertEquals(updCopyMsg.checkDoNotRipple(), msg.checkDoNotRipple());
    	assertEquals(updCopyMsg.checkHasConfInfo(), msg.checkHasConfInfo());
    	assertEquals(updCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(updCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(updCopyMsg.conflationCount(), msg.conflationCount());
    	assertEquals(updCopyMsg.conflationTime(), msg.conflationTime());
    	assertEquals(updCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(updCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(false, updCopyMsg.checkHasExtendedHdr());
    	assertEquals(updCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(updCopyMsg.checkHasPermData(), updCopyMsg.checkHasPermData());
    	assertEquals(false, updCopyMsg.msgKey().equals(msg.msgKey()));
    	assertEquals(true, updCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(false, updCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(true, updCopyMsg.permData().equals(msg.permData()));
    	assertEquals(0, updCopyMsg.encodedMsgBuffer().length());        
    }

    @Test
    public void copyRefreshMessageTest()
    {
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));
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
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);

    	RefreshMsg rfCopyMsg = (RefreshMsg) CodecFactory.createMsg();
    	msg.copy(rfCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(rfCopyMsg.containerType(), msg.containerType());
    	assertEquals(rfCopyMsg.domainType(), msg.domainType());
    	assertEquals(rfCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(rfCopyMsg.streamId(), msg.streamId());
    	assertEquals(rfCopyMsg.checkClearCache(), msg.checkClearCache());
    	assertEquals(rfCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(rfCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(rfCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(rfCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(rfCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(rfCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(rfCopyMsg.checkHasQos(), msg.checkHasQos());
    	assertEquals(rfCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(rfCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(rfCopyMsg.checkRefreshComplete(), msg.checkRefreshComplete());
    	assertEquals(rfCopyMsg.checkSolicited(), msg.checkSolicited());
    	assertEquals(rfCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(rfCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(rfCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(true, rfCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, rfCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, rfCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, rfCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, rfCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	assertEquals(true, rfCopyMsg.groupId().equals(msg.groupId()));
    	
    	msg.copy(rfCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(rfCopyMsg.containerType(), msg.containerType());
    	assertEquals(rfCopyMsg.domainType(), msg.domainType());
    	assertEquals(rfCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(rfCopyMsg.streamId(), msg.streamId());
    	msg.copy(rfCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(rfCopyMsg.checkClearCache(), msg.checkClearCache());
    	assertEquals(rfCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(false, rfCopyMsg.checkHasExtendedHdr());
    	assertEquals(rfCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(rfCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(rfCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(rfCopyMsg.checkHasQos(), msg.checkHasQos());
    	assertEquals(rfCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(rfCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(rfCopyMsg.checkRefreshComplete(), msg.checkRefreshComplete());
    	assertEquals(rfCopyMsg.checkSolicited(), msg.checkSolicited());
    	assertEquals(rfCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(rfCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(false, rfCopyMsg.checkHasPermData());
    	assertEquals(false, rfCopyMsg.checkHasExtendedHdr());
    	assertEquals(false, rfCopyMsg.msgKey().checkHasName());
    	assertEquals(false, rfCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, rfCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	assertEquals(false, rfCopyMsg.groupId().equals(msg.groupId()));
    	
    	msg.copy(rfCopyMsg, CopyMsgFlags.PERM_DATA | CopyMsgFlags.KEY_NAME);
    	assertEquals(rfCopyMsg.containerType(), msg.containerType());
    	assertEquals(rfCopyMsg.domainType(), msg.domainType());
    	assertEquals(rfCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(rfCopyMsg.streamId(), msg.streamId());
    	assertEquals(rfCopyMsg.checkClearCache(), msg.checkClearCache());
    	assertEquals(rfCopyMsg.checkDoNotCache(), msg.checkDoNotCache());
    	assertEquals(false, rfCopyMsg.checkHasExtendedHdr());
    	assertEquals(rfCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(rfCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(rfCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(rfCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(rfCopyMsg.checkHasQos(), msg.checkHasQos());
    	assertEquals(rfCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(rfCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(rfCopyMsg.checkRefreshComplete(), msg.checkRefreshComplete());
    	assertEquals(rfCopyMsg.checkSolicited(), msg.checkSolicited());
    	assertEquals(rfCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(rfCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(rfCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(true, rfCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, rfCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(false, rfCopyMsg.msgKey().checkHasAttrib());    	
    }

    @Test
    public void copyRequestMessageTest()
    {
        RequestMsg msg = (RequestMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));
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
        msg.domainType(0);  // testing domain 0
                
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
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);

    	RequestMsg rqCopyMsg = (RequestMsg) CodecFactory.createMsg();
    	msg.copy(rqCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(rqCopyMsg.checkConfInfoInUpdates(), msg.checkConfInfoInUpdates());
    	assertEquals(rqCopyMsg.checkHasBatch(), msg.checkHasBatch());
    	assertEquals(rqCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(rqCopyMsg.checkHasPriority(), msg.checkHasPriority());
    	assertEquals(rqCopyMsg.checkHasQos(), msg.checkHasQos());
    	assertEquals(rqCopyMsg.checkHasView(), msg.checkHasView());
    	assertEquals(rqCopyMsg.checkHasWorstQos(), msg.checkHasWorstQos());
    	assertEquals(rqCopyMsg.checkMsgKeyInUpdates(), msg.checkMsgKeyInUpdates());
    	assertEquals(rqCopyMsg.checkNoRefresh(), msg.checkNoRefresh());
    	assertEquals(rqCopyMsg.checkPause(), msg.checkPause());
    	assertEquals(rqCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(rqCopyMsg.checkStreaming(), msg.checkStreaming());
    	assertEquals(rqCopyMsg.containerType(), msg.containerType());
    	assertEquals(rqCopyMsg.domainType(), msg.domainType());
    	assertEquals(rqCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(rqCopyMsg.streamId(), msg.streamId());
    	assertEquals(rqCopyMsg.priority().priorityClass(), msg.priority().priorityClass());
    	assertEquals(rqCopyMsg.priority().count(), msg.priority().count());
    	assertEquals(true, rqCopyMsg.qos().equals(msg.qos()));
    	assertEquals(true, rqCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, rqCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, rqCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, rqCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));


    	msg.copy(rqCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(rqCopyMsg.checkConfInfoInUpdates(), msg.checkConfInfoInUpdates());
    	assertEquals(rqCopyMsg.checkHasBatch(), msg.checkHasBatch());
    	assertEquals(false, rqCopyMsg.checkHasExtendedHdr());
    	assertEquals(rqCopyMsg.checkHasPriority(), msg.checkHasPriority());
    	assertEquals(rqCopyMsg.checkHasQos(), msg.checkHasQos());
    	assertEquals(rqCopyMsg.checkHasView(), msg.checkHasView());
    	assertEquals(rqCopyMsg.checkHasWorstQos(), msg.checkHasWorstQos());
    	assertEquals(rqCopyMsg.checkMsgKeyInUpdates(), msg.checkMsgKeyInUpdates());
    	assertEquals(rqCopyMsg.checkNoRefresh(), msg.checkNoRefresh());
    	assertEquals(rqCopyMsg.checkPause(), msg.checkPause());
    	assertEquals(rqCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(rqCopyMsg.checkStreaming(), msg.checkStreaming());
    	assertEquals(rqCopyMsg.containerType(), msg.containerType());
    	assertEquals(rqCopyMsg.domainType(), msg.domainType());
    	assertEquals(rqCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(rqCopyMsg.streamId(), msg.streamId());
    	assertEquals(rqCopyMsg.priority().priorityClass(), msg.priority().priorityClass());
    	assertEquals(rqCopyMsg.priority().count(), msg.priority().count());
    	assertEquals(true, rqCopyMsg.qos().equals(msg.qos()));
    	assertEquals(false, rqCopyMsg.msgKey().checkHasName());
    	assertEquals(false, rqCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, rqCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    	msg.copy(rqCopyMsg, CopyMsgFlags.KEY_ATTRIB | CopyMsgFlags.MSG_BUFFER);
    	assertEquals(rqCopyMsg.checkConfInfoInUpdates(), msg.checkConfInfoInUpdates());
    	assertEquals(rqCopyMsg.checkHasBatch(), msg.checkHasBatch());
    	assertEquals(false, rqCopyMsg.checkHasExtendedHdr());
    	assertEquals(rqCopyMsg.checkHasPriority(), msg.checkHasPriority());
    	assertEquals(rqCopyMsg.checkHasQos(), msg.checkHasQos());
    	assertEquals(rqCopyMsg.checkHasView(), msg.checkHasView());
    	assertEquals(rqCopyMsg.checkHasWorstQos(), msg.checkHasWorstQos());
    	assertEquals(rqCopyMsg.checkMsgKeyInUpdates(), msg.checkMsgKeyInUpdates());
    	assertEquals(rqCopyMsg.checkNoRefresh(), msg.checkNoRefresh());
    	assertEquals(rqCopyMsg.checkPause(), msg.checkPause());
    	assertEquals(rqCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(rqCopyMsg.checkStreaming(), msg.checkStreaming());
    	assertEquals(rqCopyMsg.containerType(), msg.containerType());
    	assertEquals(rqCopyMsg.domainType(), msg.domainType());
    	assertEquals(rqCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(rqCopyMsg.streamId(), msg.streamId());
    	assertEquals(rqCopyMsg.priority().priorityClass(), msg.priority().priorityClass());
    	assertEquals(rqCopyMsg.priority().count(), msg.priority().count());
    	assertEquals(true, rqCopyMsg.qos().equals(msg.qos()));
    	assertEquals(false, rqCopyMsg.msgKey().checkHasName());
    	assertEquals(true, rqCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, rqCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    }

    @Test
    public void copyPostMessageTest()
    {
        PostMsg msg = (PostMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));
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
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);

    	PostMsg postCopyMsg = (PostMsg) CodecFactory.createMsg();
    	msg.copy(postCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(postCopyMsg.checkAck(), msg.checkAck());
    	assertEquals(postCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(postCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(postCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(postCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(postCopyMsg.checkHasPostId(), msg.checkHasPostId());
    	assertEquals(postCopyMsg.checkHasPostUserRights(), msg.checkHasPostUserRights());
    	assertEquals(postCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(postCopyMsg.checkPostComplete(), msg.checkPostComplete());
    	assertEquals(postCopyMsg.containerType(), msg.containerType());
    	assertEquals(postCopyMsg.domainType(), msg.domainType());
    	assertEquals(postCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(postCopyMsg.streamId(), msg.streamId());
    	assertEquals(postCopyMsg.partNum(), msg.partNum());
    	assertEquals(postCopyMsg.postId(), msg.postId());
    	assertEquals(postCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(postCopyMsg.postUserRights(), msg.postUserRights());
    	assertEquals(postCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(postCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(true, postCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, postCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, postCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, postCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, postCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    	msg.copy(postCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(postCopyMsg.checkAck(), msg.checkAck());
    	assertEquals(false, postCopyMsg.checkHasExtendedHdr());
    	assertEquals(postCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(postCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(false, postCopyMsg.checkHasPermData());
    	assertEquals(postCopyMsg.checkHasPostId(), msg.checkHasPostId());
    	assertEquals(postCopyMsg.checkHasPostUserRights(), msg.checkHasPostUserRights());
    	assertEquals(postCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(postCopyMsg.checkPostComplete(), msg.checkPostComplete());
    	assertEquals(postCopyMsg.containerType(), msg.containerType());
    	assertEquals(postCopyMsg.domainType(), msg.domainType());
    	assertEquals(postCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(postCopyMsg.streamId(), msg.streamId());
    	assertEquals(postCopyMsg.partNum(), msg.partNum());
    	assertEquals(postCopyMsg.postId(), msg.postId());
    	assertEquals(postCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(postCopyMsg.postUserRights(), msg.postUserRights());
    	assertEquals(postCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(postCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(false, postCopyMsg.msgKey().checkHasName());
    	assertEquals(false, postCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, postCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    	msg.copy(postCopyMsg, CopyMsgFlags.KEY_NAME | CopyMsgFlags.PERM_DATA);
    	assertEquals(postCopyMsg.checkAck(), msg.checkAck());
    	assertEquals(false, postCopyMsg.checkHasExtendedHdr());
    	assertEquals(postCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(postCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(postCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(postCopyMsg.checkHasPostId(), msg.checkHasPostId());
    	assertEquals(postCopyMsg.checkHasPostUserRights(), msg.checkHasPostUserRights());
    	assertEquals(postCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(postCopyMsg.checkPostComplete(), msg.checkPostComplete());
    	assertEquals(postCopyMsg.containerType(), msg.containerType());
    	assertEquals(postCopyMsg.domainType(), msg.domainType());
    	assertEquals(postCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(postCopyMsg.streamId(), msg.streamId());
    	assertEquals(postCopyMsg.partNum(), msg.partNum());
    	assertEquals(postCopyMsg.postId(), msg.postId());
    	assertEquals(postCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(postCopyMsg.postUserRights(), msg.postUserRights());
    	assertEquals(postCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(postCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(true, postCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, postCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(false, postCopyMsg.msgKey().checkHasAttrib());

    }

    @Test
    public void copyGenericMessageTest()
    {
        GenericMsg msg = (GenericMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));
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
        /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should
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
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);

    	GenericMsg genCopyMsg = (GenericMsg) CodecFactory.createMsg();
    	msg.copy(genCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(genCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(genCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(genCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(genCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(genCopyMsg.checkHasSecondarySeqNum(), msg.checkHasSecondarySeqNum());
    	assertEquals(genCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(genCopyMsg.containerType(), msg.containerType());
    	assertEquals(genCopyMsg.domainType(), msg.domainType());
    	assertEquals(genCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(genCopyMsg.streamId(), msg.streamId());
    	assertEquals(genCopyMsg.partNum(), msg.partNum());
    	assertEquals(genCopyMsg.secondarySeqNum(), msg.secondarySeqNum());
    	assertEquals(genCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(true, genCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, genCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, genCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, genCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, genCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    	msg.copy(genCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(false, genCopyMsg.checkHasExtendedHdr());
    	assertEquals(genCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(genCopyMsg.checkHasPartNum(), msg.checkHasPartNum());
    	assertEquals(false, genCopyMsg.checkHasPermData());
    	assertEquals(genCopyMsg.checkHasSecondarySeqNum(), msg.checkHasSecondarySeqNum());
    	assertEquals(genCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(genCopyMsg.containerType(), msg.containerType());
    	assertEquals(genCopyMsg.domainType(), msg.domainType());
    	assertEquals(genCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(genCopyMsg.streamId(), msg.streamId());
    	assertEquals(genCopyMsg.partNum(), msg.partNum());
    	assertEquals(genCopyMsg.secondarySeqNum(), msg.secondarySeqNum());
    	assertEquals(genCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(false, genCopyMsg.msgKey().checkHasName());
    	assertEquals(false, genCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, genCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    }
    
    @Test
    public void copyCloseMessageTest()
    {
        CloseMsg msg = (CloseMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));

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
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);

    	CloseMsg clCopyMsg = (CloseMsg) CodecFactory.createMsg();
    	msg.copy(clCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(clCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(clCopyMsg.checkAck(), msg.checkAck());
    	assertEquals(clCopyMsg.containerType(), msg.containerType());
    	assertEquals(clCopyMsg.domainType(), msg.domainType());
    	assertEquals(clCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(clCopyMsg.streamId(), msg.streamId());
    	assertEquals(true, clCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, clCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    	msg.copy(clCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(false, clCopyMsg.checkHasExtendedHdr());
    	assertEquals(clCopyMsg.checkAck(), msg.checkAck());
    	assertEquals(clCopyMsg.containerType(), msg.containerType());
    	assertEquals(clCopyMsg.domainType(), msg.domainType());
    	assertEquals(clCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(clCopyMsg.streamId(), msg.streamId());
    	assertEquals(false, clCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    }
    
    @Test
    public void copyStatusMessageTest()
    {
        StatusMsg msg = (StatusMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));

        msg.domainType(4);
        msg.msgClass(MsgClasses.STATUS);
        msg.streamId(24);
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
             
        msg.applyHasState();
        msg.state().code(StateCodes.TOO_MANY_ITEMS);
        msg.state().dataState(DataStates.SUSPECT);
        msg.state().streamState(StreamStates.CLOSED_RECOVER);
        Buffer textBuffer = CodecFactory.createBuffer();
        textBuffer.data("encodeStateMsgTest");
        msg.state().text(textBuffer);
        
        msg.applyHasGroupId();
        byte [] gib = {11, 123, 8, 3, 76, 2};
        Buffer groupId = CodecFactory.createBuffer();
        groupId.data(ByteBuffer.wrap(gib));
        msg.groupId(groupId);

        msg.applyHasPermData();
        byte [] pb = {10, 5, 3, 9};
        Buffer permData = CodecFactory.createBuffer();
        permData.data(ByteBuffer.wrap(pb));
        msg.permData(permData);

        msg.applyHasMsgKey();
        msg.msgKey().applyHasFilter();
        msg.msgKey().filter(7);        

        msg.applyHasExtendedHdr();
        byte [] ehb = {67, 1, 2, 3, 4, 5, 6, 7, 8, 9, 67};
        Buffer extHeader = CodecFactory.createBuffer();
        extHeader.data(ByteBuffer.wrap(ehb));
        msg.extendedHeader(extHeader);

        msg.applyHasPostUserInfo();
        long userAddr = 1234L;
        msg.postUserInfo().userAddr(userAddr);
        long userId = 567L;        
        msg.postUserInfo().userId(userId);
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(iter));
        int length = msg.encodedMsgBuffer().data().position();
        ((BufferImpl)msg.encodedMsgBuffer()).data_internal(msg.encodedMsgBuffer().data(), 0, length);

    	StatusMsg stCopyMsg = (StatusMsg) CodecFactory.createMsg();
    	msg.copy(stCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(stCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(stCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(stCopyMsg.checkClearCache(), msg.checkClearCache());
    	assertEquals(stCopyMsg.checkHasPermData(), msg.checkHasPermData());
    	assertEquals(stCopyMsg.checkHasGroupId(), msg.checkHasGroupId());
    	assertEquals(stCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(stCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(stCopyMsg.checkHasState(), msg.checkHasState());
    	assertEquals(stCopyMsg.domainType(), msg.domainType());
    	assertEquals(stCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(stCopyMsg.streamId(), msg.streamId());
    	assertEquals(stCopyMsg.containerType(), msg.containerType());
    	assertEquals(stCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(stCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(true, stCopyMsg.permData().equals(msg.permData()));
    	assertEquals(true, stCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, stCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, stCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, stCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	assertEquals(true, stCopyMsg.groupId().equals(msg.groupId()));
    	assertEquals(true, stCopyMsg.state().text().equals(msg.state().text()));
    	assertEquals(stCopyMsg.state().code(), msg.state().code());
    	assertEquals(stCopyMsg.state().dataState(), msg.state().dataState());
    	assertEquals(stCopyMsg.state().streamState(), msg.state().streamState());

    	msg.copy(stCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(false, stCopyMsg.checkHasExtendedHdr());
    	assertEquals(stCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(stCopyMsg.checkClearCache(), msg.checkClearCache());
    	assertEquals(false, stCopyMsg.checkHasPermData());
    	assertEquals(false, stCopyMsg.checkHasGroupId());
    	assertEquals(stCopyMsg.checkHasPostUserInfo(), msg.checkHasPostUserInfo());
    	assertEquals(stCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(stCopyMsg.checkHasState(), msg.checkHasState());
    	assertEquals(stCopyMsg.domainType(), msg.domainType());
    	assertEquals(stCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(stCopyMsg.streamId(), msg.streamId());
    	assertEquals(stCopyMsg.containerType(), msg.containerType());
    	assertEquals(stCopyMsg.postUserInfo().userAddr(), msg.postUserInfo().userAddr());
    	assertEquals(stCopyMsg.postUserInfo().userId(), msg.postUserInfo().userId());
    	assertEquals(false, stCopyMsg.msgKey().checkHasName());
    	assertEquals(false, stCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, stCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    	assertEquals(false, stCopyMsg.state().text().equals(msg.state().text()));
    	assertEquals(stCopyMsg.state().code(), msg.state().code());
    	assertEquals(stCopyMsg.state().dataState(), msg.state().dataState());
    	assertEquals(stCopyMsg.state().streamState(), msg.state().streamState());
    }

    @Test
    public void copyAckMessageTest()
    {
        AckMsg msg = (AckMsg)CodecFactory.createMsg();
        Buffer buf = msg.encodedMsgBuffer();
        buf.data(ByteBuffer.allocate(1024));
        
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
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit.
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

    	AckMsg ackCopyMsg = (AckMsg) CodecFactory.createMsg();
    	msg.copy(ackCopyMsg, CopyMsgFlags.ALL_FLAGS);
    	assertEquals(ackCopyMsg.checkHasExtendedHdr(), msg.checkHasExtendedHdr());
    	assertEquals(ackCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(ackCopyMsg.checkHasNakCode(), msg.checkHasNakCode());
    	assertEquals(ackCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(ackCopyMsg.checkHasText(), msg.checkHasText());
    	assertEquals(ackCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(ackCopyMsg.domainType(), msg.domainType());
    	assertEquals(ackCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(ackCopyMsg.streamId(), msg.streamId());
    	assertEquals(ackCopyMsg.containerType(), msg.containerType());
    	assertEquals(ackCopyMsg.nakCode(), msg.nakCode());
    	assertEquals(ackCopyMsg.ackId(), msg.ackId());
    	assertEquals(ackCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(true, ackCopyMsg.text().equals(msg.text()));
    	assertEquals(true, ackCopyMsg.extendedHeader().equals(msg.extendedHeader()));
    	assertEquals(true, ackCopyMsg.msgKey().name().equals(msg.msgKey().name()));
    	assertEquals(true, ackCopyMsg.msgKey().encodedAttrib().equals(msg.msgKey().encodedAttrib()));
    	assertEquals(true, ackCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));

    	msg.copy(ackCopyMsg, CopyMsgFlags.NONE);
    	assertEquals(false, ackCopyMsg.checkHasExtendedHdr());
    	assertEquals(ackCopyMsg.checkHasMsgKey(), msg.checkHasMsgKey());
    	assertEquals(ackCopyMsg.checkHasNakCode(), msg.checkHasNakCode());
    	assertEquals(ackCopyMsg.checkHasSeqNum(), msg.checkHasSeqNum());
    	assertEquals(false, ackCopyMsg.checkHasText());
    	assertEquals(ackCopyMsg.checkPrivateStream(), msg.checkPrivateStream());
    	assertEquals(ackCopyMsg.domainType(), msg.domainType());
    	assertEquals(ackCopyMsg.msgClass(), msg.msgClass());
    	assertEquals(ackCopyMsg.streamId(), msg.streamId());
    	assertEquals(ackCopyMsg.containerType(), msg.containerType());
    	assertEquals(ackCopyMsg.nakCode(), msg.nakCode());
    	assertEquals(ackCopyMsg.ackId(), msg.ackId());
    	assertEquals(ackCopyMsg.seqNum(), msg.seqNum());
    	assertEquals(false, ackCopyMsg.msgKey().checkHasAttrib());
    	assertEquals(false, ackCopyMsg.msgKey().checkHasName());
    	assertEquals(false, ackCopyMsg.encodedMsgBuffer().equals(msg.encodedMsgBuffer()));
    }

    @Test
    public void isFinalMessageTest()
    {
        AckMsg ackMsg = (AckMsg)CodecFactory.createMsg();
        ackMsg.msgClass(MsgClasses.ACK);
        assertEquals(false, ackMsg.isFinalMsg());

        CloseMsg clMsg = (CloseMsg)CodecFactory.createMsg();
        clMsg.msgClass(MsgClasses.CLOSE);
        assertEquals(true, clMsg.isFinalMsg());

        GenericMsg genMsg = (GenericMsg)CodecFactory.createMsg();
        genMsg.msgClass(MsgClasses.GENERIC);
        assertEquals(false, genMsg.isFinalMsg());

        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        postMsg.msgClass(MsgClasses.POST);
        assertEquals(false, postMsg.isFinalMsg());

        RefreshMsg refMsg = (RefreshMsg)CodecFactory.createMsg();
        refMsg.msgClass(MsgClasses.REFRESH);
        assertEquals(false, refMsg.isFinalMsg());
        
        refMsg.state().dataState(DataStates.NO_CHANGE);
        refMsg.state().streamState(StreamStates.CLOSED);
        assertEquals(true, refMsg.isFinalMsg());
        
        refMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        assertEquals(true, refMsg.isFinalMsg());
        
        refMsg.state().streamState(StreamStates.OPEN);
        assertEquals(false, refMsg.isFinalMsg());
        
        refMsg.state().streamState(StreamStates.REDIRECTED);
        assertEquals(true, refMsg.isFinalMsg());
        
        refMsg.state().streamState(StreamStates.UNSPECIFIED);
        assertEquals(false, refMsg.isFinalMsg());
        
        refMsg.state().streamState(StreamStates.NON_STREAMING);
        assertEquals(false, refMsg.isFinalMsg());
        
        refMsg.applyRefreshComplete();
        assertEquals(true, refMsg.isFinalMsg());

        RequestMsg reqMsg = (RequestMsg)CodecFactory.createMsg();
        reqMsg.msgClass(MsgClasses.REQUEST);
        assertEquals(false, reqMsg.isFinalMsg());
        reqMsg.applyStreaming();
        assertEquals(false, reqMsg.isFinalMsg());

        StatusMsg stMsg = (StatusMsg)CodecFactory.createMsg();
        stMsg.msgClass(MsgClasses.STATUS);
        assertEquals(false, stMsg.isFinalMsg());
        stMsg.applyHasState();
        stMsg.state().dataState(DataStates.NO_CHANGE);
        stMsg.state().streamState(StreamStates.CLOSED);
        assertEquals(true, stMsg.isFinalMsg());
        stMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        assertEquals(true, stMsg.isFinalMsg());
        stMsg.state().streamState(StreamStates.OPEN);
        assertEquals(false, stMsg.isFinalMsg());
        stMsg.state().streamState(StreamStates.REDIRECTED);
        assertEquals(true, stMsg.isFinalMsg());
        stMsg.state().streamState(StreamStates.UNSPECIFIED);
        assertEquals(false, stMsg.isFinalMsg());

        UpdateMsg updMsg = (UpdateMsg)CodecFactory.createMsg();
        updMsg.msgClass(MsgClasses.UPDATE);
        assertEquals(false, updMsg.isFinalMsg());
     }

}
