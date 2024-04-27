/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using static LSEG.Eta.Rdm.Dictionary;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests;

/// Simple handler for <see cref="ProviderTest"/> to handle dictionary requests and
/// let <see cref="DictionaryCallbackClientTests"/> run its tests
internal class DictionaryHandler
{
    internal const string FIELD_DICTIONARY_NAME = "RWFFld";
    internal const string ENUM_TYPE_DICTIONARY_NAME = "RWFEnum";

    internal const string FIELD_DICTIONARY_FILENAME = "../../../ComplexTypeTests/RDMFieldDictionary";
    internal const string ENUM_TABLE_FILENAME = "../../../ComplexTypeTests/enumtype.def";

    private const int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;
    private const int MAX_FIELD_DICTIONARY_MSG_SIZE = 448000;
    private const int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;

    private string FieldDictionaryDownloadName = FIELD_DICTIONARY_NAME;
    private string EnumTypeDictionaryDownloadName = ENUM_TYPE_DICTIONARY_NAME;

    private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

    private EncodeIterator m_EncodeIter = new EncodeIterator();
    private DictionaryRefresh m_DictionaryRefresh = new DictionaryRefresh();
    private DictionaryStatus m_DictionaryStatus = new DictionaryStatus();

    internal DataDictionary DataDictionary { get; private set; } = new DataDictionary();
    private ProviderSessionOptions m_ProviderSessionOptions;

    public DictionaryHandler(ProviderTest providerTest)
    {
        m_ProviderSessionOptions = providerTest.ProviderSessionOptions;

        if (DataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME, out CodecError fdError) < 0)
        {
            Assert.Fail($"Failed to load FieldDictionary, error: {fdError?.Text}");
        }

        if (DataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME, out CodecError edError) < 0)
        {
            Assert.Fail($"Failed to load EnumType Dictionary, error: {edError?.Text}");
        }
    }

    public DictionaryHandler(ProviderSessionOptions providerSessionOptions)
    {
        m_ProviderSessionOptions = providerSessionOptions;

        if (DataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME, out CodecError fdError) < 0)
        {
            Assert.Fail($"Failed to load FieldDictionary, error: {fdError?.Text}");
        }

        if (DataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME, out CodecError edError) < 0)
        {
            Assert.Fail($"Failed to load EnumType Dictionary, error: {edError?.Text}");
        }
    }

    public ReactorReturnCode HandleDictionaryMsgEvent(RDMDictionaryMsgEvent reactorEvent)
    {
        DictionaryMsg dictionaryMsg = reactorEvent.DictionaryMsg!;
        ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;

        ReactorReturnCode ret;

        switch (dictionaryMsg.DictionaryMsgType)
        {
            case DictionaryMsgType.REQUEST:
                {
                    DictionaryRequest dictionaryRequest = dictionaryMsg.DictionaryRequest!;

                    IRequestMsg requestMsg = (IRequestMsg)reactorEvent.Msg!;

                    if (!requestMsg.CheckNoRefresh()) // Check whether the consumer side requires a refresh
                    {
                        if (FieldDictionaryDownloadName.Equals(dictionaryRequest.DictionaryName.ToString()))
                        {
                            // Name matches field dictionary. Send the field dictionary refresh
                            if ((ret = SendFieldDictionaryResponse(reactorChannel!, dictionaryRequest)) != ReactorReturnCode.SUCCESS)
                            {
                                Assert.Fail($"SendFieldDictionaryResponse() failed: {ret}");
                            }
                        }
                        else if (EnumTypeDictionaryDownloadName.Equals(dictionaryRequest.DictionaryName.ToString()))
                        {
                            // Name matches the enum types dictionary. Send the enum types dictionary refresh
                            if ((ret = SendEnumTypeDictionaryResponse(reactorChannel!, dictionaryRequest)) != ReactorReturnCode.SUCCESS)
                            {
                                Assert.Fail($"SendEnumTypeDictionaryResponse() failed: {ret}");
                            }
                        }
                        else
                        {
                            if ((ret = SendRequestReject(reactorChannel!, reactorEvent.Msg!.StreamId)) != ReactorReturnCode.SUCCESS)
                            {
                                Assert.Fail($"SendRequestReject() failed: {ret}");
                            }
                        }
                    }
                    break;
                }

            case DictionaryMsgType.CLOSE:
                CloseStream(dictionaryMsg.StreamId);
                break;

            default:
                Assert.Fail($"Received Unhandled Dictionary Msg Type: {dictionaryMsg.DictionaryMsgType}");
                break;
        }

        return ReactorReturnCode.SUCCESS;
    }

    private void CloseStream(int streamId)
    {
        Assert.True(true);
    }

    private ReactorReturnCode SendRequestReject(ReactorChannel chnl, int streamId)
    {
        // get a buffer for the dictionary request reject status
        ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out _);

        if (msgBuf != null)
        {
            // encode dictionary request reject status
            ReactorReturnCode ret = EncodeDictionaryRequestReject(chnl, streamId, msgBuf);
            if (ret != ReactorReturnCode.SUCCESS)
            {
                return ret;
            }

            // send request reject status
            return chnl.Submit(msgBuf, m_SubmitOptions, out _);
        }
        else
        {
            return ReactorReturnCode.FAILURE;
        }
    }

    private ReactorReturnCode EncodeDictionaryRequestReject(ReactorChannel chnl, int streamId, ITransportBuffer msgBuf)
    {
        // clear encode iterator
        m_EncodeIter.Clear();

        // set-up message
        m_DictionaryStatus.Clear();
        m_DictionaryStatus.StreamId = streamId;
        m_DictionaryStatus.HasState = true;
        m_DictionaryStatus.State.DataState(DataStates.SUSPECT);

        // DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME:
        m_DictionaryStatus.State.Code(StateCodes.NOT_FOUND);
        m_DictionaryStatus.State.StreamState(StreamStates.CLOSED);
        m_DictionaryStatus.State.Text().Data($"Dictionary request rejected for stream id {streamId} - dictionary name unknown");

        // encode message
        CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Assert.Fail($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
            return ReactorReturnCode.FAILURE;
        }

        ret = m_DictionaryStatus.Encode(m_EncodeIter);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Assert.Fail("DictionaryStatus.Encode() failed");
            return ReactorReturnCode.FAILURE;
        }

        return ReactorReturnCode.SUCCESS;
    }

    private ReactorReturnCode SendEnumTypeDictionaryResponse(ReactorChannel chnl, DictionaryRequest dictionaryRequest)
    {
        m_DictionaryRefresh.Clear();

        m_DictionaryRefresh.StreamId = dictionaryRequest.StreamId;
        m_DictionaryRefresh.DictionaryType = Types.ENUM_TABLES;
        m_DictionaryRefresh.DataDictionary = DataDictionary;
        m_DictionaryRefresh.ServiceId = dictionaryRequest.ServiceId;
        m_DictionaryRefresh.Verbosity = dictionaryRequest.Verbosity;
        m_DictionaryRefresh.Solicited = true;

        m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
        m_DictionaryRefresh.State.DataState(DataStates.OK);
        m_DictionaryRefresh.State.Code(StateCodes.NONE);

        // dictionaryName
        m_DictionaryRefresh.DictionaryName = dictionaryRequest.DictionaryName;

        bool firstMultiPart = true;

        while (true)
        {
            // get a buffer for the dictionary response
            ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, out _);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_DictionaryRefresh.State.Text().Data("Enum Type Dictionary Refresh (starting enum " + m_DictionaryRefresh.StartEnumTableCount + ")");

            msgBuf.Data.Limit = MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE;

            // clear encode iterator
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Assert.Fail($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ReactorReturnCode.FAILURE;
            }

            if (firstMultiPart)
            {
                m_DictionaryRefresh.ClearCache = true;
                firstMultiPart = false;
            }
            else
                m_DictionaryRefresh.Flags = DictionaryRefreshFlags.SOLICITED;

            // encode message
            ret = m_DictionaryRefresh.Encode(m_EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Assert.Fail("DictionaryRefresh.Encode() failed");
            }

            // send dictionary response
            ReactorReturnCode submitCode;
            if ((submitCode = chnl.Submit(msgBuf, m_SubmitOptions, out var submitError)) != ReactorReturnCode.SUCCESS)
            {
                Assert.Fail($"ReactorChannel.Submit failed ({submitCode}): {submitError}");
                return ReactorReturnCode.FAILURE;
            }

            // break out of loop when all dictionary responses sent
            if (ret == CodecReturnCode.SUCCESS)
            {
                break;
            }

            // sleep between dictionary responses
            try
            {
                System.Threading.Thread.Sleep(1);
            }
            catch (Exception)
            {
            }
        }

        return ReactorReturnCode.SUCCESS;
    }

    private ReactorReturnCode SendFieldDictionaryResponse(ReactorChannel chnl, DictionaryRequest dictionaryRequest)
    {
        m_DictionaryRefresh.Clear();

        m_DictionaryRefresh.StreamId = dictionaryRequest.StreamId;
        m_DictionaryRefresh.DictionaryType = Types.FIELD_DEFINITIONS;
        m_DictionaryRefresh.DataDictionary = DataDictionary;
        m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
        m_DictionaryRefresh.State.DataState(DataStates.OK);
        m_DictionaryRefresh.State.Code(StateCodes.NONE);
        m_DictionaryRefresh.Verbosity = dictionaryRequest.Verbosity;
        m_DictionaryRefresh.ServiceId = dictionaryRequest.ServiceId;
        m_DictionaryRefresh.DictionaryName = dictionaryRequest.DictionaryName;
        m_DictionaryRefresh.Solicited = true;

        bool firstMultiPart = true;

        while (true)
        {
            // get a buffer for the dictionary response
            ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_FIELD_DICTIONARY_MSG_SIZE, false, out _);
            if (msgBuf == null)
            {
                Assert.True(false);
                return ReactorReturnCode.FAILURE;
            }

            m_DictionaryRefresh.State.Text().Data($"Field Dictionary Refresh (starting fid {m_DictionaryRefresh.StartFid})");

            msgBuf.Data.Limit = MAX_FIELD_DICTIONARY_MSG_SIZE;

            // clear encode iterator
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Assert.Fail($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ReactorReturnCode.FAILURE;
            }

            if (firstMultiPart)
            {
                m_DictionaryRefresh.ClearCache = true;
                firstMultiPart = false;
            }
            else
                m_DictionaryRefresh.Flags = DictionaryRefreshFlags.SOLICITED;

            // encode message
            ret = m_DictionaryRefresh.Encode(m_EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Assert.Fail("DictionaryRefresh.Encode() failed");
                return ReactorReturnCode.FAILURE;
            }

            // send dictionary response
            if (chnl.Submit(msgBuf, m_SubmitOptions, out var submitError) != ReactorReturnCode.SUCCESS)
            {
                Assert.Fail($"ReactorChannel.Submit failed: {submitError}");
                return ReactorReturnCode.FAILURE;
            }

            // break out of loop when all dictionary responses sent
            if (ret == CodecReturnCode.SUCCESS)
            {
                break;
            }

            // sleep between dictionary responses
            try
            {
                System.Threading.Thread.Sleep(1);
            }
            catch (Exception)
            {
            }
        }

        return ReactorReturnCode.SUCCESS;
    }
}
