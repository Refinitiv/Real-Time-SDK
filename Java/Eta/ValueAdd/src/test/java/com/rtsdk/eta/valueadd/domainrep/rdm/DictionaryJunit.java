///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.valueadd.domainrep.rdm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;

import org.junit.Test;

import com.rtsdk.eta.codec.*;
import com.rtsdk.eta.rdm.Dictionary;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefreshFlags;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequestFlags;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.rtsdk.eta.valueadd.domainrep.rdm.TypedMessageTestUtil;

public class DictionaryJunit
{
    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private Msg msg = CodecFactory.createMsg();

    @Test
    public void dictionaryRefreshTests()
    {
        String enumDictionaryText = "!tag Filename    ENUMTYPE.001\n" +
                "!tag Desc        IDN Marketstream enumerated tables\n" +
                "!tag RT_Version  4.00\n" +
                "!tag DT_Version  12.11\n" +
                "!tag Date        13-Aug-2010\n" +
                "PRCTCK_1      14\n" +
                "      0          \" \"   no tick\n" +
                "      1         #DE#   up tick or zero uptick\n" +
                "      2         #FE#   down tick or zero downtick\n" +
                "      3          \" \"   unchanged tick\n";

        DataDictionary encDictionary = CodecFactory.createDataDictionary();
        DataDictionary decDictionary = CodecFactory.createDataDictionary();

        System.out.println("DictionaryRefresh tests...");
        DictionaryRefresh encRDMMsg = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        DictionaryRefresh decRDMMsg = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DictionaryMsgType.REFRESH);
        encRDMMsg.rdmMsgType(DictionaryMsgType.REFRESH);
        int streamId = -5;
        long sequenceNumber = 11152011;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        String enumDictName = "RWFEnum";
        Int enumDictType = CodecFactory.createInt();
        enumDictType.value(Dictionary.Types.ENUM_TABLES);
        int flagsBase[] =
        {
                DictionaryRefreshFlags.SOLICITED,
                DictionaryRefreshFlags.HAS_SEQ_NUM,
                DictionaryRefreshFlags.CLEAR_CACHE,
        };
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Error error = TransportFactory.createError();
        File tmpEnumFile = null;
        try
        {
            tmpEnumFile = File.createTempFile("tmpEnumDictionary", ".txt");
            tmpEnumFile.deleteOnExit();
            BufferedWriter out = new BufferedWriter(new FileWriter(tmpEnumFile));
            out.write(enumDictionaryText);
            out.close();
            int ret = encDictionary.loadEnumTypeDictionary(tmpEnumFile.getAbsolutePath(), error);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
        }
        catch (IOException e)
        {
            assertTrue(e.toString(), false);
        }

        for (int i = 0; i < flagsList.length; i++)
        {
            dIter.clear();
            encRDMMsg.clear();
            encRDMMsg.flags(flagsList[i]);

            encRDMMsg.streamId(streamId);

            encRDMMsg.state().code(state.code());
            encRDMMsg.state().dataState(state.dataState());
            encRDMMsg.state().text().data("state");
            encRDMMsg.state().streamState(state.streamState());

            encRDMMsg.dictionary(encDictionary);
            encRDMMsg.dictionaryName().data(enumDictName);
            encRDMMsg.dictionaryType((int)enumDictType.toLong());
            if (encRDMMsg.checkHasSequenceNumber())
                encRDMMsg.sequenceNumber(sequenceNumber);

            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));
            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());

            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);

            assertEquals(CodecReturnCodes.SUCCESS, ret);

            assertEquals(streamId, decRDMMsg.streamId());
			// decoding adds these two extra flags
            assertEquals(encRDMMsg.flags() | DictionaryRefreshFlags.HAS_INFO | DictionaryRefreshFlags.IS_COMPLETE, decRDMMsg.flags()); 
            if (decRDMMsg.checkHasSequenceNumber())
                assertEquals(sequenceNumber, decRDMMsg.sequenceNumber());
            assertEquals(enumDictType.toLong(), decRDMMsg.dictionaryType());
            {
                State decState = decRDMMsg.state();
                assertNotNull(decState);
                assertEquals(state.code(), decState.code());
                assertEquals(state.dataState(), decState.dataState());
                assertEquals(state.streamState(), decState.streamState());
                assertEquals(state.text().toString(), decState.text().toString());
            }
            assertTrue(encRDMMsg.dictionaryName().equals(decRDMMsg.dictionaryName()));
            /* Message was encoded and decoded. Try to decode the payload. */
            dIter.setBufferAndRWFVersion(decRDMMsg.dataBody(),
                                         Codec.majorVersion(), Codec.minorVersion());
            decDictionary.clear();
            ret = decDictionary.decodeEnumTypeDictionary(dIter,
                                                         decRDMMsg.verbosity(), error);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
        }

        System.out.println("Done.");
    }

    @Test
    public void dictionaryRefreshToStringTests()
    {
        DictionaryRefresh refreshRDMMsg1 = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        refreshRDMMsg1.rdmMsgType(DictionaryMsgType.REFRESH);

        /* Parameters to test with */
        int streamId = -5;
        int serviceId = 273;
        int verbosity = Dictionary.VerbosityValues.VERBOSE | Dictionary.VerbosityValues.NORMAL | Dictionary.VerbosityValues.MINIMAL | Dictionary.VerbosityValues.INFO;
        Buffer dictionaryName = CodecFactory.createBuffer();
        dictionaryName.data("RWFFld");
        System.out.println("DictionaryRefresh toString tests...");

        refreshRDMMsg1.streamId(streamId);
        refreshRDMMsg1.serviceId(serviceId);
        refreshRDMMsg1.verbosity(verbosity);
        refreshRDMMsg1.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
        refreshRDMMsg1.flags(DictionaryRefreshFlags.CLEAR_CACHE | DictionaryRefreshFlags.HAS_INFO | DictionaryRefreshFlags.HAS_SEQ_NUM | DictionaryRefreshFlags.IS_COMPLETE | DictionaryRefreshFlags.SOLICITED);
        refreshRDMMsg1.toString();
        System.out.println("Done.");
    }

    @Test
    public void dictionaryCloseTests()
    {
        DictionaryClose encRDMMsg = (DictionaryClose)DictionaryMsgFactory.createMsg();
        DictionaryClose decRDMMsg = (DictionaryClose)DictionaryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DictionaryMsgType.CLOSE);
        encRDMMsg.rdmMsgType(DictionaryMsgType.CLOSE);

        int streamId = -5;

        dIter.clear();
        encIter.clear();
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));

        System.out.println("DictionaryClose tests...");
        encRDMMsg.clear();

        encRDMMsg.streamId(streamId);
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());

        int ret = encRDMMsg.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                     Codec.minorVersion());
        ret = msg.decode(dIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        ret = decRDMMsg.decode(dIter, msg);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        assertEquals(streamId, decRDMMsg.streamId());

        System.out.println("Done.");
    }

    @Test
    public void dictionaryCloseCopyTests()
    {
        DictionaryClose closeRDMMsg1 = (DictionaryClose)DictionaryMsgFactory.createMsg();
        DictionaryClose closeRDMMsg2 = (DictionaryClose)DictionaryMsgFactory.createMsg();
        closeRDMMsg1.rdmMsgType(DictionaryMsgType.CLOSE);
        closeRDMMsg2.rdmMsgType(DictionaryMsgType.CLOSE);
        int streamId = -5;
        closeRDMMsg1.streamId(streamId);
        System.out.println("DictionaryClose copy tests...");
        // deep copy
        int ret = closeRDMMsg1.copy(closeRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        // verify deep copy
        assertEquals(closeRDMMsg1.streamId(), closeRDMMsg2.streamId());
        System.out.println("Done.");
    }

    @Test
    public void dictionaryCloseToStringTests()
    {
        DictionaryClose closeRDMMsg1 = (DictionaryClose)DictionaryMsgFactory.createMsg();
        closeRDMMsg1.rdmMsgType(DictionaryMsgType.CLOSE);
        int streamId = -5;
        closeRDMMsg1.streamId(streamId);

        System.out.println("DictionaryClose toString coverage tests...");
        assertNotNull(closeRDMMsg1.toString());
        System.out.println("Done.");
    }

    @Test
    public void dictionaryStatusCopyTests()
    {
        DictionaryStatus statusRDMMsg1 = (DictionaryStatus)DictionaryMsgFactory.createMsg();
        statusRDMMsg1.rdmMsgType(DictionaryMsgType.STATUS);
        DictionaryStatus statusRDMMsg2 = (DictionaryStatus)DictionaryMsgFactory.createMsg();
        statusRDMMsg2.rdmMsgType(DictionaryMsgType.STATUS);

        System.out.println("DictionaryStatus copy tests...");

        /* Parameter setup */
        int streamId = -5;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        statusRDMMsg1.clear();
        statusRDMMsg1.streamId(streamId);
        statusRDMMsg1.applyHasState();
        statusRDMMsg1.applyHasState();
        statusRDMMsg1.state().code(state.code());
        statusRDMMsg1.state().dataState(state.dataState());
        statusRDMMsg1.state().text().data("state");
        statusRDMMsg1.state().streamState(state.streamState());
        statusRDMMsg1.applyClearCache();

        int ret = statusRDMMsg1.copy(statusRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(streamId, statusRDMMsg2.streamId());
        assertEquals(statusRDMMsg1.flags(), statusRDMMsg2.flags());
        assertEquals(statusRDMMsg1.checkClearCache(), statusRDMMsg2.checkClearCache());

        State refState1 = statusRDMMsg1.state();
        State refState2 = statusRDMMsg2.state();
        assertNotNull(refState2);
        assertEquals(refState1.code(), refState2.code());
        assertEquals(refState1.dataState(), refState2.dataState());
        assertEquals(refState1.streamState(), refState2.streamState());
        assertEquals(refState1.text().toString(), refState2.text().toString());
        assertTrue(refState1.text() != refState2.text());

        System.out.println("Done.");
    }

    @Test
    public void dictionaryStatusTests()
    {
        DictionaryStatus encRDMMsg = (DictionaryStatus)DictionaryMsgFactory.createMsg();
        DictionaryStatus decRDMMsg = (DictionaryStatus)DictionaryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DictionaryMsgType.STATUS);
        encRDMMsg.rdmMsgType(DictionaryMsgType.STATUS);

        System.out.println("RsslRDMLoginStatus tests...");

        /* Parameter setup */
        int streamId = -5;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        dIter.clear();
        encIter.clear();
        encRDMMsg.clear();
        encRDMMsg.rdmMsgType(DictionaryMsgType.STATUS);
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));

        encRDMMsg.streamId(streamId);
        encRDMMsg.applyHasState();
        encRDMMsg.state().code(state.code());
        encRDMMsg.state().dataState(state.dataState());
        encRDMMsg.state().text().data("state");
        encRDMMsg.state().streamState(state.streamState());
        
        encRDMMsg.applyClearCache();

        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        int ret = encRDMMsg.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = msg.decode(dIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        ret = decRDMMsg.decode(dIter, msg);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        /* Check parameters */
        assertEquals(streamId, decRDMMsg.streamId());
        assertEquals(decRDMMsg.flags(), encRDMMsg.flags());
        assertEquals(encRDMMsg.checkClearCache(), decRDMMsg.checkClearCache());
        assertTrue(decRDMMsg.checkHasState());

        {
            State decState = decRDMMsg.state();
            assertNotNull(decState);
            assertEquals(state.code(), decState.code());
            assertEquals(state.dataState(), decState.dataState());
            assertEquals(state.streamState(), decState.streamState());
            assertEquals(state.text().toString(), decState.text().toString());
        }

        System.out.println("Done.");
    }

    @Test
    public void dictionaryStatusToStringTests()
    {
        DictionaryStatus statusRDMMsg1 = (DictionaryStatus)DictionaryMsgFactory.createMsg();
        statusRDMMsg1.rdmMsgType(DictionaryMsgType.STATUS);
        int streamId = -5;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);
        statusRDMMsg1.streamId(streamId);
        statusRDMMsg1.applyHasState();
        statusRDMMsg1.state().code(state.code());
        statusRDMMsg1.state().dataState(state.dataState());
        statusRDMMsg1.state().text().data("state");
        statusRDMMsg1.state().streamState(state.streamState());

        System.out.println("DictionaryStatus toString tests.");

        statusRDMMsg1.toString();
        System.out.println("Done.");
    }

    @Test
    public void dictionaryRequestToStringTests()
    {
        DictionaryRequest reqRDMMsg1 = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        reqRDMMsg1.rdmMsgType(DictionaryMsgType.REQUEST);
        /* Parameters to test with */
        int streamId = -5;
        int serviceId = 273;
        int verbosity = Dictionary.VerbosityValues.VERBOSE | Dictionary.VerbosityValues.NORMAL | Dictionary.VerbosityValues.MINIMAL | Dictionary.VerbosityValues.INFO;
        String dictionaryName = "RWFFld";
        System.out.println("DictionaryRequest toString tests...");

        reqRDMMsg1.streamId(streamId);
        reqRDMMsg1.serviceId(serviceId);
        reqRDMMsg1.verbosity(verbosity);
        reqRDMMsg1.dictionaryName().data(dictionaryName);
        reqRDMMsg1.applyStreaming();
        assertNotNull(reqRDMMsg1.toString());

        System.out.println("Done.");
    }

    @Test
    public void dictionaryRequestTests()
    {
        DictionaryRequest encRDMMsg = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DictionaryMsgType.REQUEST);
        DictionaryRequest decRDMMsg = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DictionaryMsgType.REQUEST);

        /* Parameters to test with */
        int streamId = -5;
        int serviceId = 273;
        int verbosity = Dictionary.VerbosityValues.VERBOSE;
        String dictionaryName = "RWFFld";

        int flagsBase[] =
        {
                DictionaryRequestFlags.STREAMING
        };
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        System.out.println("DictionaryRequest tests...");

        for (int flags : flagsList)
        {
            encIter.clear();
            dIter.clear();
            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            /*** Encode ***/
            encRDMMsg.clear();

            encRDMMsg.streamId(streamId);

            encRDMMsg.flags(flags);
            encRDMMsg.serviceId(serviceId);
            encRDMMsg.verbosity(verbosity);
            encRDMMsg.dictionaryName().data(dictionaryName);
            if ((flags & DictionaryRequestFlags.STREAMING) != 0)
                assertTrue(encRDMMsg.checkStreaming());

            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            assertEquals(decRDMMsg.rdmMsgType(),
                         DictionaryMsgType.REQUEST);
            assertEquals(encRDMMsg.streamId(), decRDMMsg.streamId());
            assertEquals(encRDMMsg.serviceId(), decRDMMsg.serviceId());
            assertEquals(encRDMMsg.verbosity(), decRDMMsg.verbosity());
            assertTrue(encRDMMsg.dictionaryName().equals(decRDMMsg.dictionaryName()));
            assertEquals(encRDMMsg.flags(), decRDMMsg.flags());
        }

        System.out.println("Done.");
    }

    @Test
    public void dictionaryRequestCopyTests()
    {
        DictionaryRequest reqRDMMsg1 = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        reqRDMMsg1.rdmMsgType(DictionaryMsgType.REQUEST);
        reqRDMMsg1.applyStreaming();
        DictionaryRequest reqRDMMsg2 = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        reqRDMMsg2.rdmMsgType(DictionaryMsgType.REQUEST);

        int streamId = -5;
        int serviceId = 273;
        int verbosity = Dictionary.VerbosityValues.VERBOSE;
        String dictionaryName = "RWFFld";

        System.out.println("DictionaryRequest copy tests...");
        reqRDMMsg1.rdmMsgType(DictionaryMsgType.REQUEST);

        reqRDMMsg1.streamId(streamId);
        reqRDMMsg1.serviceId(serviceId);
        reqRDMMsg1.verbosity(verbosity);
        reqRDMMsg1.dictionaryName().data(dictionaryName);
        reqRDMMsg1.applyStreaming();

        int ret = reqRDMMsg1.copy(reqRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertTrue(reqRDMMsg1.dictionaryName() != reqRDMMsg2.dictionaryName());
        assertEquals(reqRDMMsg1.dictionaryName().toString(), reqRDMMsg2.dictionaryName().toString());
        assertEquals(reqRDMMsg1.flags(), reqRDMMsg2.flags());
        assertEquals(reqRDMMsg1.serviceId(), reqRDMMsg2.serviceId());
        assertEquals(reqRDMMsg1.checkStreaming(), reqRDMMsg2.checkStreaming());
        assertEquals(reqRDMMsg1.streamId(), reqRDMMsg2.streamId());
        assertEquals(reqRDMMsg1.verbosity(), reqRDMMsg2.verbosity());

        System.out.println("Done.");
    }

    @Test
    public void dictionaryRefreshCopyTests()
    {
        DictionaryRefresh refreshRDMMsg1 = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        refreshRDMMsg1.rdmMsgType(DictionaryMsgType.REFRESH);
        DictionaryRefresh refreshRDMMsg2 = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        refreshRDMMsg2.rdmMsgType(DictionaryMsgType.REFRESH);

        int streamId = -5;
        int serviceId = 273;
        int verbosity = Dictionary.VerbosityValues.VERBOSE;
        String dictionaryName = "RWFFld";

        System.out.println("DictionaryRefresh copy tests...");

        refreshRDMMsg1.streamId(streamId);
        refreshRDMMsg1.serviceId(serviceId);
        refreshRDMMsg1.verbosity(verbosity);
        refreshRDMMsg1.applyClearCache();
        refreshRDMMsg1.applyRefreshComplete();
        refreshRDMMsg1.applySolicited();

        refreshRDMMsg1.dictionaryName().data(dictionaryName);
        refreshRDMMsg1.flags(DictionaryRefreshFlags.CLEAR_CACHE | DictionaryRefreshFlags.HAS_INFO | DictionaryRefreshFlags.HAS_SEQ_NUM | DictionaryRefreshFlags.IS_COMPLETE | DictionaryRefreshFlags.IS_COMPLETE | DictionaryRefreshFlags.SOLICITED);

        int ret = refreshRDMMsg1.copy(refreshRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertTrue(refreshRDMMsg1.dictionaryName() != refreshRDMMsg2.dictionaryName());
        assertEquals(dictionaryName, refreshRDMMsg2.dictionaryName().toString());
        assertEquals(refreshRDMMsg1.flags(), refreshRDMMsg2.flags());
        assertEquals(refreshRDMMsg1.checkClearCache(), refreshRDMMsg2.checkClearCache());
        assertEquals(refreshRDMMsg1.checkRefreshComplete(), refreshRDMMsg2.checkRefreshComplete());
        assertEquals(refreshRDMMsg1.checkSolicited(), refreshRDMMsg2.checkSolicited());

        assertEquals(refreshRDMMsg1.serviceId(), refreshRDMMsg2.serviceId());
        assertEquals(refreshRDMMsg1.streamId(), refreshRDMMsg2.streamId());
        assertEquals(refreshRDMMsg1.verbosity(), refreshRDMMsg2.verbosity());

        System.out.println("Done.");
    }
}