/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.IO;
using Xunit;
using static LSEG.Eta.Rdm.Dictionary;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class DictionaryTest
    {
        private DecodeIterator m_DecodeIterator = new DecodeIterator();
        private EncodeIterator m_EncodeIterator = new EncodeIterator();
        private Msg m_Msg = new Msg();

        [Fact]
        public void DictionaryRefreshTests()
        {
            string enumDictionaryText = "!tag Filename    ENUMTYPE.001\n" +
                    "!tag Desc        IDN Marketstream enumerated tables\n" +
                    "!tag RT_Version  4.00\n" +
                    "!tag DT_Version  12.11\n" +
                    "!tag Date        13-Aug-2010\n" +
                    "PRCTCK_1      14\n" +
                    "      0          \" \"   no tick\n" +
                    "      1         #DE#   up tick or zero uptick\n" +
                    "      2         #FE#   down tick or zero downtick\n" +
                    "      3          \" \"   unchanged tick\n";

            DataDictionary encDictionary = new DataDictionary();
            DataDictionary decDictionary = new DataDictionary();

            CodecError codecError;

            Console.WriteLine("DictionaryRefresh tests...");
            DictionaryRefresh encRDMMsg = new DictionaryRefresh();
            DictionaryRefresh decRDMMsg = new DictionaryRefresh();

            int streamId = -5;
            long sequenceNumber = 11152011;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            string enumDictName = "RWFEnum";
            Int enumDictType = new Int();
            enumDictType.Value(Types.ENUM_TABLES);
            DictionaryRefreshFlags[] flagsBase =
            {
                DictionaryRefreshFlags.SOLICITED,
                DictionaryRefreshFlags.HAS_SEQ_NUM,
                DictionaryRefreshFlags.CLEAR_CACHE,
            };
            DictionaryRefreshFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            StreamWriter tmpEnumFile;
            try
            {
                tmpEnumFile = new StreamWriter("tmpEnumDictionary.txt");
                tmpEnumFile.Write(enumDictionaryText);
                tmpEnumFile.Close();
                CodecReturnCode ret = encDictionary.LoadEnumTypeDictionary("tmpEnumDictionary.txt", out codecError);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                File.Delete("tmpEnumDictionary.txt");
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
                Assert.True(false);
            }

            for (int i = 0; i < flagsList.Length; i++)
            {
                m_DecodeIterator.Clear();
                encRDMMsg.Clear();
                encRDMMsg.Flags = flagsList[i];

                encRDMMsg.StreamId = streamId;

                encRDMMsg.State.Code(state.Code());
                encRDMMsg.State.DataState(state.DataState());
                encRDMMsg.State.Text().Data("state");
                encRDMMsg.State.StreamState(state.StreamState());

                encRDMMsg.DataDictionary = encDictionary;
                encRDMMsg.DictionaryName.Data(enumDictName);
                encRDMMsg.DictionaryType = (int)enumDictType.ToLong();
                if (encRDMMsg.HasSequenceNumber)
                    encRDMMsg.SequenceNumber = sequenceNumber;

                Buffer membuf = new Buffer();
                membuf.Data(new ByteBuffer(1024));
                m_EncodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                CodecReturnCode ret = encRDMMsg.Encode(m_EncodeIterator);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                m_DecodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = m_Msg.Decode(m_DecodeIterator);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decRDMMsg.Decode(m_DecodeIterator, m_Msg);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(streamId, decRDMMsg.StreamId);
                // decoding adds these two extra flags
                Assert.Equal(encRDMMsg.Flags | DictionaryRefreshFlags.HAS_INFO | DictionaryRefreshFlags.IS_COMPLETE, decRDMMsg.Flags);
                if (decRDMMsg.HasSequenceNumber)
                {
                    Assert.Equal(sequenceNumber, decRDMMsg.SequenceNumber);
                }

                Assert.Equal(enumDictType.ToLong(), decRDMMsg.DictionaryType);
                {
                    State decState = decRDMMsg.State;
                    Assert.NotNull(decState);
                    Assert.Equal(state.Code(), decState.Code());
                    Assert.Equal(state.DataState(), decState.DataState());
                    Assert.Equal(state.StreamState(), decState.StreamState());
                    Assert.Equal(state.Text().ToString(), decState.Text().ToString());
                }
                Assert.True(encRDMMsg.DictionaryName.Equals(decRDMMsg.DictionaryName));
                // Message was encoded and decoded. Try to decode the payload.
                m_DecodeIterator.SetBufferAndRWFVersion(decRDMMsg.DataBody, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                decDictionary.Clear();
                ret = decDictionary.DecodeEnumTypeDictionary(m_DecodeIterator, (int)decRDMMsg.Verbosity, out codecError);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
            }

            Console.WriteLine("Done.");
        } 

        [Fact]
        public void DictionaryRefreshToStringTests()
        {
            DictionaryRefresh refreshRDMMsg1 = new DictionaryRefresh();

            /* Parameters to test with */
            int streamId = -5;
            int serviceId = 273;
            int verbosity = VerbosityValues.VERBOSE | VerbosityValues.NORMAL | VerbosityValues.MINIMAL | VerbosityValues.INFO;
            Buffer dictionaryName = new Buffer();
            dictionaryName.Data("RWFFld");
            Console.WriteLine("DictionaryRefresh ToString tests...");

            refreshRDMMsg1.StreamId = streamId;
            refreshRDMMsg1.ServiceId = serviceId;
            refreshRDMMsg1.Verbosity = verbosity;
            refreshRDMMsg1.DictionaryType = Types.FIELD_DEFINITIONS;
            refreshRDMMsg1.Flags = DictionaryRefreshFlags.CLEAR_CACHE 
                | DictionaryRefreshFlags.HAS_INFO 
                | DictionaryRefreshFlags.HAS_SEQ_NUM 
                | DictionaryRefreshFlags.IS_COMPLETE 
                | DictionaryRefreshFlags.SOLICITED;
            refreshRDMMsg1.DictionaryName = dictionaryName;
            refreshRDMMsg1.ServiceId = serviceId;

            string dictionaryRefreshStr = refreshRDMMsg1.ToString();
            Assert.Contains("DictionaryRefresh: ", dictionaryRefreshStr);
            Assert.Contains("streamId: -5", dictionaryRefreshStr);
            Assert.Contains("serviceId: 273", dictionaryRefreshStr);
            Assert.Contains("verbosity: INFO | MINIMAL | NORMAL | VERBOSE", dictionaryRefreshStr);
            Assert.Contains("dictionaryName: RWFFld", dictionaryRefreshStr);
            Assert.Contains("isClearCache: True", dictionaryRefreshStr);
            Assert.Contains("isRefreshComplete: True", dictionaryRefreshStr);
            Assert.Contains("isSolicited: True", dictionaryRefreshStr);
            
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryCloseTests()
        {
            DictionaryClose encRDMMsg = new DictionaryClose();
            DictionaryClose decRDMMsg = new DictionaryClose();

            int streamId = -5;

            m_DecodeIterator.Clear();
            m_EncodeIterator.Clear();
            Buffer membuf = new Buffer();
            membuf.Data(new ByteBuffer(1024));

            Console.WriteLine("DictionaryClose tests...");
            encRDMMsg.Clear();

            encRDMMsg.StreamId = streamId;
            m_EncodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            CodecReturnCode ret = encRDMMsg.Encode(m_EncodeIterator);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            m_DecodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = m_Msg.Decode(m_DecodeIterator);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = decRDMMsg.Decode(m_DecodeIterator, m_Msg);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.Equal(streamId, decRDMMsg.StreamId);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryCloseCopyTests()
        {
            DictionaryClose closeRDMMsg1 = new DictionaryClose();
            DictionaryClose closeRDMMsg2 = new DictionaryClose();

            int streamId = -5;
            closeRDMMsg1.StreamId = streamId;
            Console.WriteLine("DictionaryClose copy tests...");
            // deep copy
            CodecReturnCode ret = closeRDMMsg1.Copy(closeRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // verify deep copy
            Assert.Equal(closeRDMMsg1.StreamId, closeRDMMsg2.StreamId);
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryCloseToStringTests()
        {
            DictionaryClose closeRDMMsg1 = new DictionaryClose();
            int streamId = -5;
            closeRDMMsg1.StreamId = streamId;

            Console.WriteLine("DictionaryClose tostring coverage tests...");
            Assert.NotNull(closeRDMMsg1.ToString());
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryStatusCopyTests()
        {
            DictionaryStatus statusRDMMsg1 = new DictionaryStatus();
            DictionaryStatus statusRDMMsg2 = new DictionaryStatus();

            Console.WriteLine("DictionaryStatus copy tests...");

            // Parameter setup
            int streamId = -5;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            statusRDMMsg1.Clear();
            statusRDMMsg1.StreamId = streamId;
            statusRDMMsg1.HasState = true;
            statusRDMMsg1.State.Code(state.Code());
            statusRDMMsg1.State.DataState(state.DataState());
            statusRDMMsg1.State.Text().Data("state");
            statusRDMMsg1.State.StreamState(state.StreamState());
            statusRDMMsg1.ClearCache = true;

            CodecReturnCode ret = statusRDMMsg1.Copy(statusRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.Equal(streamId, statusRDMMsg2.StreamId);
            Assert.Equal(statusRDMMsg1.Flags, statusRDMMsg2.Flags);
            Assert.Equal(statusRDMMsg1.ClearCache, statusRDMMsg2.ClearCache);

            State refState1 = statusRDMMsg1.State;
            State refState2 = statusRDMMsg2.State;
            Assert.NotNull(refState2);
            Assert.Equal(refState1.Code(), refState2.Code());
            Assert.Equal(refState1.DataState(), refState2.DataState());
            Assert.Equal(refState1.StreamState(), refState2.StreamState());
            Assert.Equal(refState1.Text().ToString(), refState2.Text().ToString());
            Assert.True(refState1.Text() != refState2.Text());

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryStatusTests()
        {
            DictionaryStatus encRDMMsg = new DictionaryStatus();
            DictionaryStatus decRDMMsg = new DictionaryStatus();

            Console.WriteLine("DictionaryStatus tests...");

            /* Parameter setup */
            int streamId = -5;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            m_DecodeIterator.Clear();
            m_EncodeIterator.Clear();
            encRDMMsg.Clear();
            Buffer membuf = new Buffer();
            membuf.Data(new ByteBuffer(1024));

            encRDMMsg.StreamId = streamId;
            encRDMMsg.HasState = true;
            encRDMMsg.State.Code(state.Code());
            encRDMMsg.State.DataState(state.DataState());
            encRDMMsg.State.Text().Data("state");
            encRDMMsg.State.StreamState(state.StreamState());

            encRDMMsg.ClearCache = true;

            m_EncodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            CodecReturnCode ret = encRDMMsg.Encode(m_EncodeIterator);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            m_DecodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = m_Msg.Decode(m_DecodeIterator);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = decRDMMsg.Decode(m_DecodeIterator, m_Msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Check parameters
            Assert.Equal(streamId, decRDMMsg.StreamId);
            Assert.Equal(decRDMMsg.Flags, encRDMMsg.Flags);
            Assert.Equal(encRDMMsg.ClearCache, decRDMMsg.ClearCache);
            Assert.True(decRDMMsg.HasState);

            State decState = decRDMMsg.State;
            Assert.NotNull(decState);
            Assert.Equal(state.Code(), decState.Code());
            Assert.Equal(state.DataState(), decState.DataState());
            Assert.Equal(state.StreamState(), decState.StreamState());
            Assert.Equal(state.Text().ToString(), decState.Text().ToString());

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryStatusToStringTests()
        {
            DictionaryStatus statusRDMMsg1 = new DictionaryStatus();
            int streamId = -5;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);
            statusRDMMsg1.StreamId = streamId;
            statusRDMMsg1.HasState = true;
            statusRDMMsg1.State.Code(state.Code());
            statusRDMMsg1.State.DataState(state.DataState());
            statusRDMMsg1.State.Text().Data("state");
            statusRDMMsg1.State.StreamState(state.StreamState());

            Console.WriteLine("DictionaryStatus tostring tests.");

            statusRDMMsg1.ToString();
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryRequestToStringTests()
        {
            DictionaryRequest reqRDMMsg1 = new DictionaryRequest();
            /* Parameters to test with */
            int streamId = -5;
            int serviceId = 273;
            int verbosity = VerbosityValues.VERBOSE | VerbosityValues.NORMAL | VerbosityValues.MINIMAL | VerbosityValues.INFO;
            string dictionaryName = "RWFFld";
            Console.WriteLine("DictionaryRequest tostring tests...");

            reqRDMMsg1.StreamId = streamId;
            reqRDMMsg1.ServiceId = serviceId;
            reqRDMMsg1.Verbosity = verbosity;
            reqRDMMsg1.DictionaryName.Data(dictionaryName);
            reqRDMMsg1.Streaming = true;
            Assert.NotNull(reqRDMMsg1.ToString());

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryRequestTests()
        {
            DictionaryRequest encRDMMsg = new DictionaryRequest();
            DictionaryRequest decRDMMsg = new DictionaryRequest();

            /* Parameters to test with */
            int streamId = -5;
            int serviceId = 273;
            int verbosity = VerbosityValues.VERBOSE;
            string dictionaryName = "RWFFld";

            DictionaryRequestFlags[] flagsBase =
            {
                DictionaryRequestFlags.STREAMING
            };
            DictionaryRequestFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            Console.WriteLine("DictionaryRequest tests...");

            foreach (DictionaryRequestFlags flags in flagsList)
            {
                m_EncodeIterator.Clear();
                m_DecodeIterator.Clear();
                Buffer membuf = new Buffer();
                membuf.Data(new ByteBuffer(1024));

                encRDMMsg.Clear();
                encRDMMsg.StreamId = streamId;
                encRDMMsg.Flags = flags;
                encRDMMsg.ServiceId = serviceId;
                encRDMMsg.Verbosity = verbosity;
                encRDMMsg.DictionaryName.Data(dictionaryName);
                if ((flags & DictionaryRequestFlags.STREAMING) != 0)
                {
                    Assert.True(encRDMMsg.Streaming);
                }

                m_EncodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = encRDMMsg.Encode(m_EncodeIterator);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                m_DecodeIterator.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = m_Msg.Decode(m_DecodeIterator);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decRDMMsg.Decode(m_DecodeIterator, m_Msg);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                Assert.Equal(encRDMMsg.StreamId, decRDMMsg.StreamId);
                Assert.Equal(encRDMMsg.ServiceId, decRDMMsg.ServiceId);
                Assert.Equal(encRDMMsg.Verbosity, decRDMMsg.Verbosity);
                Assert.True(encRDMMsg.DictionaryName.Equals(decRDMMsg.DictionaryName));
                Assert.Equal(encRDMMsg.Flags, decRDMMsg.Flags);
            }

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryRequestCopyTests()
        {
            DictionaryRequest reqRDMMsg1 = new DictionaryRequest();
            DictionaryRequest reqRDMMsg2 = new DictionaryRequest();

            int streamId = -5;
            int serviceId = 273;
            int verbosity = VerbosityValues.VERBOSE;
            string dictionaryName = "RWFFld";

            Console.WriteLine("DictionaryRequest copy tests...");

            reqRDMMsg1.StreamId = streamId;
            reqRDMMsg1.ServiceId = serviceId;
            reqRDMMsg1.Verbosity = verbosity;
            reqRDMMsg1.DictionaryName.Data(dictionaryName);
            reqRDMMsg1.Streaming = true;

            CodecReturnCode ret = reqRDMMsg1.Copy(reqRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(reqRDMMsg1.DictionaryName != reqRDMMsg2.DictionaryName);
            Assert.Equal(reqRDMMsg1.DictionaryName.ToString(), reqRDMMsg2.DictionaryName.ToString());
            Assert.Equal(reqRDMMsg1.Flags, reqRDMMsg2.Flags);
            Assert.Equal(reqRDMMsg1.ServiceId, reqRDMMsg2.ServiceId);
            Assert.Equal(reqRDMMsg1.Streaming, reqRDMMsg2.Streaming);
            Assert.Equal(reqRDMMsg1.StreamId, reqRDMMsg2.StreamId);
            Assert.Equal(reqRDMMsg1.Verbosity, reqRDMMsg2.Verbosity);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DictionaryRefreshCopyTests()
        {
            DictionaryRefresh refreshRDMMsg1 = new DictionaryRefresh();
            DictionaryRefresh refreshRDMMsg2 = new DictionaryRefresh();

            int streamId = -5;
            int serviceId = 273;
            int verbosity = VerbosityValues.VERBOSE;
            string dictionaryName = "RWFFld";

            Console.WriteLine("DictionaryRefresh copy tests...");

            refreshRDMMsg1.StreamId = streamId;
            refreshRDMMsg1.ServiceId = serviceId;
            refreshRDMMsg1.Verbosity = verbosity;
            refreshRDMMsg1.ClearCache = true;
            refreshRDMMsg1.RefreshComplete = true;
            refreshRDMMsg1.Solicited = true;

            refreshRDMMsg1.DictionaryName.Data(dictionaryName);
            refreshRDMMsg1.Flags = DictionaryRefreshFlags.CLEAR_CACHE 
                | DictionaryRefreshFlags.HAS_INFO 
                | DictionaryRefreshFlags.HAS_SEQ_NUM 
                | DictionaryRefreshFlags.IS_COMPLETE 
                | DictionaryRefreshFlags.IS_COMPLETE 
                | DictionaryRefreshFlags.SOLICITED;

            CodecReturnCode ret = refreshRDMMsg1.Copy(refreshRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(refreshRDMMsg1.DictionaryName != refreshRDMMsg2.DictionaryName);
            Assert.Equal(dictionaryName, refreshRDMMsg2.DictionaryName.ToString());
            Assert.Equal(refreshRDMMsg1.Flags, refreshRDMMsg2.Flags);
            Assert.Equal(refreshRDMMsg1.ClearCache, refreshRDMMsg2.ClearCache);
            Assert.Equal(refreshRDMMsg1.RefreshComplete, refreshRDMMsg2.RefreshComplete);
            Assert.Equal(refreshRDMMsg1.Solicited, refreshRDMMsg2.Solicited);

            Assert.Equal(refreshRDMMsg1.ServiceId, refreshRDMMsg2.ServiceId);
            Assert.Equal(refreshRDMMsg1.StreamId, refreshRDMMsg2.StreamId);
            Assert.Equal(refreshRDMMsg1.Verbosity, refreshRDMMsg2.Verbosity);

            Console.WriteLine("Done.");
        }
    }
}
