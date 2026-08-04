/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.*;

public class DirectoryStatusImplTest
{

    private DirectoryStatusImpl status;

    @Before
    public void setUp()
    {
        status = new DirectoryStatusImpl();
        status.clear();
    }

    @Test
    public void givenNewDirectoryStatus_whenCreated_thenAllFlagsAreFalseAndDomainIsSource()
    {
        assertFalse(status.checkHasFilter());
        assertFalse(status.checkHasServiceId());
        assertFalse(status.checkHasState());
        assertFalse(status.checkClearCache());
        assertFalse(status.checkHasPermData());
        assertEquals(DomainTypes.SOURCE, status.domainType());
    }

    @Test
    public void givenDirectoryStatus_whenApplyHasFilter_thenCheckHasFilterReturnsTrue()
    {
        status.applyHasFilter();
        assertTrue(status.checkHasFilter());
    }

    @Test
    public void givenDirectoryStatus_whenApplyHasServiceId_thenCheckHasServiceIdReturnsTrue()
    {
        status.applyHasServiceId();
        assertTrue(status.checkHasServiceId());
    }

    @Test
    public void givenDirectoryStatus_whenApplyHasState_thenCheckHasStateReturnsTrue()
    {
        status.applyHasState();
        assertTrue(status.checkHasState());
    }

    @Test
    public void givenDirectoryStatus_whenApplyClearCache_thenCheckClearCacheReturnsTrue()
    {
        status.applyClearCache();
        assertTrue(status.checkClearCache());
    }

    @Test
    public void givenDirectoryStatus_whenApplyHasPermData_thenCheckHasPermDataReturnsTrue()
    {
        status.applyHasPermData();
        assertTrue(status.checkHasPermData());
    }

    @Test
    public void givenDirectoryStatusWithHasFilter_whenSetFilter_thenFilterValueIsReturned()
    {
        status.applyHasFilter();
        status.filter(42L);
        assertEquals(42L, status.filter());
    }

    @Test
    public void givenDirectoryStatusWithHasServiceId_whenSetServiceId_thenServiceIdValueIsReturned()
    {
        status.applyHasServiceId();
        status.serviceId(10);
        assertEquals(10, status.serviceId());
    }

    @Test
    public void givenDirectoryStatusWithHasState_whenSetState_thenStateValuesAreReturned()
    {
        State state = CodecFactory.createState();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);

        status.applyHasState();
        status.state(state);

        assertEquals(StreamStates.OPEN, status.state().streamState());
        assertEquals(DataStates.OK, status.state().dataState());
        assertEquals(StateCodes.NONE, status.state().code());
    }

    @Test
    public void givenDirectoryStatusWithHasPermData_whenSetPermData_thenPermDataLengthMatches()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data("testPermData");

        status.applyHasPermData();
        status.permData(buf);

        assertEquals(buf.length(), status.permData().length());
    }

    @Test
    public void givenDirectoryStatusWithFlagsSet_whenClear_thenAllFlagsAndFieldsAreReset()
    {
        status.applyHasFilter();
        status.applyHasServiceId();
        status.applyHasState();
        status.applyClearCache();
        status.clear();

        assertFalse(status.checkHasFilter());
        assertFalse(status.checkHasServiceId());
        assertFalse(status.checkHasState());
        assertFalse(status.checkClearCache());
        assertEquals(0, status.serviceId());
        assertEquals(0L, status.filter());
    }

    @Test
    public void givenPopulatedDirectoryStatus_whenCopied_thenAllFieldsAreCopiedToDestination()
    {
        status.streamId(5);
        status.applyHasFilter();
        status.filter(99L);
        status.applyHasServiceId();
        status.serviceId(7);
        status.applyClearCache();
        status.applyHasState();
        State state = CodecFactory.createState();
        state.streamState(StreamStates.CLOSED);
        state.dataState(DataStates.SUSPECT);
        state.code(StateCodes.NOT_FOUND);
        status.state(state);
        status.applyHasPermData();
        Buffer permData = CodecFactory.createBuffer();
        permData.data(java.nio.ByteBuffer.wrap(new byte[]{0x01, 0x02, 0x03}));
        status.permData(permData);

        DirectoryStatus dest = (DirectoryStatus) DirectoryMsgFactory.createMsg();
        dest.rdmMsgType(DirectoryMsgType.STATUS);
        dest.clear();
        int ret = status.copy(dest);

        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(5, dest.streamId());
        assertTrue(dest.checkHasFilter());
        assertEquals(99L, dest.filter());
        assertTrue(dest.checkHasServiceId());
        assertEquals(7, dest.serviceId());
        assertTrue(dest.checkClearCache());
        assertTrue(dest.checkHasState());
        assertEquals(StreamStates.CLOSED, dest.state().streamState());
        assertEquals(DataStates.SUSPECT, dest.state().dataState());
        assertEquals(StateCodes.NOT_FOUND, dest.state().code());
        assertTrue(dest.checkHasPermData());
        assertArrayEquals(bufferToBytes(permData), bufferToBytes(dest.permData()));
    }

    @Test
    public void givenPopulatedDirectoryStatus_whenEncodedAndDecoded_thenAllFieldsMatch()
    {
        status.streamId(1);
        status.applyHasState();
        State state = CodecFactory.createState();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
        status.state(state);
        status.applyHasServiceId();
        status.serviceId(3);
        status.applyHasFilter();
        status.filter(15L);
        status.applyHasPermData();
        Buffer permData = CodecFactory.createBuffer();
        permData.data(java.nio.ByteBuffer.wrap(new byte[]{0x0A, 0x0B, 0x0C}));
        status.permData(permData);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buf = CodecFactory.createBuffer();
        buf.data(java.nio.ByteBuffer.allocate(1024));
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        int ret = status.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        Msg msg = CodecFactory.createMsg();
        msg.decode(decIter);

        DirectoryStatusImpl decoded = new DirectoryStatusImpl();
        decoded.clear();
        ret = decoded.decode(decIter, msg);

        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(1, decoded.streamId());
        assertTrue(decoded.checkHasState());
        assertEquals(StreamStates.OPEN, decoded.state().streamState());
        assertEquals(DataStates.OK, decoded.state().dataState());
        assertTrue(decoded.checkHasServiceId());
        assertEquals(3, decoded.serviceId());
        assertTrue(decoded.checkHasFilter());
        assertEquals(15L, decoded.filter());
        assertTrue(decoded.checkHasPermData());
        assertArrayEquals(bufferToBytes(permData), bufferToBytes(decoded.permData()));
    }

    @Test
    public void givenDirectoryStatus_whenDecodeWithNonStatusMsg_thenReturnsFailure()
    {
        Msg msg = CodecFactory.createMsg();
        msg.msgClass(MsgClasses.REFRESH);
        DecodeIterator decIter = CodecFactory.createDecodeIterator();

        int ret = status.decode(decIter, msg);
        assertEquals(CodecReturnCodes.FAILURE, ret);
    }

    @Test
    public void givenDirectoryStatusWithServiceIdFilterAndState_whenToString_thenContainsExpectedFields()
    {
        status.applyHasServiceId();
        status.serviceId(5);
        status.applyHasFilter();
        status.filter(10L);
        status.applyHasState();

        String result = status.toString();
        assertTrue(result.contains("DirectoryStatus"));
        assertTrue(result.contains("serviceId: 5"));
        assertTrue(result.contains("filter: 10"));
        assertTrue(result.contains("state:"));
    }

    @Test
    public void givenDirectoryStatus_whenSetFlags_thenFlagsAreReturnedAndChecksPass()
    {
        status.flags(DirectoryStatusFlags.HAS_FILTER | DirectoryStatusFlags.HAS_STATE);
        assertEquals(DirectoryStatusFlags.HAS_FILTER | DirectoryStatusFlags.HAS_STATE, status.flags());
        assertTrue(status.checkHasFilter());
        assertTrue(status.checkHasState());
    }

    @Test
    public void givenNewDirectoryStatus_whenCleared_thenStateDefaultsAreOpenOkNone()
    {
        assertEquals(StreamStates.OPEN, status.state().streamState());
        assertEquals(DataStates.OK, status.state().dataState());
        assertEquals(StateCodes.NONE, status.state().code());
    }

    @Test
    public void givenDirectoryStatus_whenCopiedWithOnlyStreamId_thenDestHasNoOptionalFlags()
    {
        status.streamId(3);

        DirectoryStatus dest = (DirectoryStatus) DirectoryMsgFactory.createMsg();
        dest.rdmMsgType(DirectoryMsgType.STATUS);
        dest.clear();
        int ret = status.copy(dest);

        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(3, dest.streamId());
        assertFalse(dest.checkHasFilter());
        assertFalse(dest.checkHasServiceId());
        assertFalse(dest.checkHasState());
        assertFalse(dest.checkClearCache());
        assertFalse(dest.checkHasPermData());
    }

    @Test
    public void givenDirectoryStatusWithStateText_whenCopied_thenStateTextIsCopied()
    {
        status.streamId(1);
        status.applyHasState();
        State state = CodecFactory.createState();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
        Buffer textBuf = CodecFactory.createBuffer();
        textBuf.data("Source Mirror Full");
        state.text(textBuf);
        status.state(state);

        DirectoryStatus dest = (DirectoryStatus) DirectoryMsgFactory.createMsg();
        dest.rdmMsgType(DirectoryMsgType.STATUS);
        dest.clear();
        int ret = status.copy(dest);

        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertTrue(dest.checkHasState());
        assertEquals("Source Mirror Full", dest.state().text().toString());
    }

    @Test
    public void givenEncodedDirectoryStatusWithClearCache_whenDecoded_thenClearCacheIsSet()
    {
        status.streamId(2);
        status.applyClearCache();

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buf = CodecFactory.createBuffer();
        buf.data(java.nio.ByteBuffer.allocate(512));
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, status.encode(encIter));

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        Msg msg = CodecFactory.createMsg();
        msg.decode(decIter);

        DirectoryStatusImpl decoded = new DirectoryStatusImpl();
        decoded.clear();
        assertEquals(CodecReturnCodes.SUCCESS, decoded.decode(decIter, msg));
        assertTrue(decoded.checkClearCache());
    }

    @Test
    public void givenEncodedDirectoryStatusWithNoMsgKey_whenDecoded_thenNoServiceIdOrFilter()
    {
        status.streamId(4);
        status.applyHasState();
        State state = CodecFactory.createState();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
        status.state(state);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buf = CodecFactory.createBuffer();
        buf.data(java.nio.ByteBuffer.allocate(512));
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, status.encode(encIter));

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        Msg msg = CodecFactory.createMsg();
        msg.decode(decIter);

        DirectoryStatusImpl decoded = new DirectoryStatusImpl();
        decoded.clear();
        assertEquals(CodecReturnCodes.SUCCESS, decoded.decode(decIter, msg));
        assertFalse(decoded.checkHasServiceId());
        assertFalse(decoded.checkHasFilter());
        assertTrue(decoded.checkHasState());
    }

    @Test
    public void givenDirectoryStatusWithPermData_whenToString_thenContainsPermData()
    {
        status.applyHasPermData();
        Buffer buf = CodecFactory.createBuffer();
        buf.data(java.nio.ByteBuffer.wrap(new byte[]{0x0A, 0x0B}));
        status.permData(buf);

        String result = status.toString();
        assertTrue(result.contains("permData"));
    }

    private static byte[] bufferToBytes(Buffer buffer)
    {
        byte[] bytes = new byte[buffer.length()];
        buffer.data().position(buffer.position());
        buffer.data().get(bytes);
        return bytes;
    }
}
