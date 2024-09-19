/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using System.Collections.Generic;

using System;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;
using Array = LSEG.Eta.Codec.Array;
using LSEG.Eta.Common;
using System.Linq;
using static LSEG.Eta.Rdm.Dictionary;
using System.Threading;

namespace LSEG.Eta.ValuedAdd.Tests;

internal class SymbolAction
{
    public MapEntryActions Action { get; set; }

    public Buffer ItemName { get; set; } = new Buffer();
}

[Collection("ValueAdded")]
public class WatchlistItemDomainsTest
{
    #region Common variables and methods

    //private readonly ITestOutputHelper output;

    private const string DICTIONARY_FILE_NAME = "../../../../Src/Tests/test_FieldDictionary";
    private const string ENUM_TYPE_FILE_NAME = "../../../../Src/Tests/test_enumtype.def";

    private const string USER_NAME = "User's Name";
    private const string USER_PASSWORD = "Their password";
    private const string LOGIN_ACCEPTED = "Login accepted by test host";
    private const string APPLICATION_ID = "256";
    private const string APPLICATION_NAME = "ETA Test Provider";

    private const string VENDOR = "Refinitiv";
    private const string FIELD_DICTIONARY_NAME = "RWFFld";
    private const string ENUM_TYPE_DICTIONARY_NAME = "RWFEnum";
    private const string LINK_NAME = "ETA Provider Link";
    private const int OPEN_LIMIT = 10;


    private static readonly List<string> DEFAULT_VIEW_FIELD_LIST = new List<string> { "6", "22", "25", "32" };

    private DataDictionary m_Dictionary = new();

    const int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;

    // Support encoding for Real data type of the 6, 12, 13, 19, 21, 22, 25, 30, 31 and 1465 fids.
    private void EncodeViewDataForFieldId(ReactorChannel reactorChannel, IRefreshMsg msg, List<int> viewFieldIdList)
    {
        EncodeIterator encodeIt = new();
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();
        Buffer buffer = new();
        Real real = new();

        if (viewFieldIdList.Count > 0)
        {
            ByteBuffer byteBuffer = new(1024);
            buffer.Data(byteBuffer);
            encodeIt.Clear();
            Assert.True(encodeIt.SetBufferAndRWFVersion(buffer, reactorChannel.MajorVersion,
                    reactorChannel.MinorVersion) == CodecReturnCode.SUCCESS);

            fieldList.Clear();
            fieldList.ApplyHasStandardData();
            CodecReturnCode ret = fieldList.EncodeInit(encodeIt, null, 0);

            Assert.False(ret < CodecReturnCode.SUCCESS);

            var it = viewFieldIdList.GetEnumerator();
            while (it.MoveNext())
            {
                int fid = it.Current;
                fieldEntry.Clear();
                switch (fid)
                {
                    case 6:
                    case 12:
                    case 13:
                    case 19:
                    case 21:
                    case 22:
                    case 25:
                    case 30:
                    case 31:
                    case 1465:
                        {
                            fieldEntry.FieldId = fid;
                            fieldEntry.DataType = DataTypes.REAL;
                            real.Clear();
                            real.Value(fid, RealHints.EXPONENT1);
                            Assert.True(fieldEntry.Encode(encodeIt, real) == CodecReturnCode.SUCCESS);
                            break;
                        }
                    default:
                        {
                            Assert.False(true); // Support only the above fields
                            break;
                        }
                }
            }

            Assert.True(fieldList.EncodeComplete(encodeIt, true) == CodecReturnCode.SUCCESS);
            msg.ContainerType = DataTypes.FIELD_LIST;
            msg.EncodedDataBody = buffer;
        }
    }

    private void DecodeViewDataForFieldId(ReactorChannel reactorChannel, IRequestMsg msg, List<int> viewFieldIdList)
    {
        DecodeIterator decodeIt = new();
        ElementList elementList = new ElementList();
        ElementEntry elementEntry = new ElementEntry();
        UInt viewType = new();
        Int fieldId = new();
        Array viewArray = new();
        ArrayEntry viewEntry = new();
        Buffer viewData = new Buffer();
        CodecReturnCode ret;

        viewFieldIdList.Clear();
        Assert.Equal(DataTypes.ELEMENT_LIST, msg.ContainerType);

        decodeIt.Clear();
        decodeIt.SetBufferAndRWFVersion(msg.EncodedDataBody, reactorChannel.MajorVersion, reactorChannel.MinorVersion);

        elementList.Clear();
        Assert.True((ret = elementList.Decode(decodeIt, null)) == CodecReturnCode.SUCCESS);

        elementEntry.Clear();
        // Decoding the list of Field IDs
        while ((ret = elementEntry.Decode(decodeIt)) != CodecReturnCode.END_OF_CONTAINER)
        {
            Assert.False(ret < CodecReturnCode.SUCCESS);

            if (elementEntry.Name.Equals(ElementNames.VIEW_TYPE) || elementEntry.Name.Equals(ElementNames.VIEW_DATA))
            {
                switch (elementEntry.DataType)
                {
                    case DataTypes.UINT:
                        {
                            Assert.True(viewType.Decode(decodeIt) == CodecReturnCode.SUCCESS);
                            break;
                        }
                    case DataTypes.ARRAY:
                        {
                            viewData = elementEntry.EncodedData;
                            break;
                        }
                }
            }
        }

        decodeIt.Clear();
        decodeIt.SetBufferAndRWFVersion(viewData, reactorChannel.MajorVersion,
                reactorChannel.MinorVersion);

        // Support only as a list of Field IDs
        Assert.True(viewType.ToLong() == ViewTypes.FIELD_ID_LIST);
        viewArray.Clear();
        Assert.True(viewArray.Decode(decodeIt) == CodecReturnCode.SUCCESS);
        Assert.True(viewArray.PrimitiveType == DataTypes.INT);
        while ((ret = viewEntry.Decode(decodeIt)) != CodecReturnCode.END_OF_CONTAINER)
        {
            Assert.False(ret < CodecReturnCode.SUCCESS);
            if ((ret = fieldId.Decode(decodeIt)) == CodecReturnCode.SUCCESS)
            {
                Assert.False(fieldId.ToLong() < short.MinValue || fieldId.ToLong() > short.MaxValue);
                viewFieldIdList.Add((int)fieldId.ToLong());
            }
        }// while
    }

    // Encodes the payload for batch and/or view requests
    internal static void EncodeBatchWithView(ReactorChannel reactorChannel, IRequestMsg requestMsg, List<string> batchList, List<int> fieldIdList)
    {
        Buffer buf = new Buffer();
        buf.Data(new ByteBuffer(1024));
        EncodeIterator encodeIter = new EncodeIterator();

        /* clear encode iterator */
        encodeIter.Clear();

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);

        // encode payload
        ElementList eList = new ElementList();
        ElementEntry eEntry = new ElementEntry();
        Array elementArray = new Array();
        ArrayEntry ae = new ArrayEntry();
        Buffer itemName = new Buffer();

        eList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.Name = ElementNames.BATCH_ITEM_LIST;
        eEntry.DataType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.PrimitiveType = DataTypes.ASCII_STRING;
        elementArray.ItemLength = 0;

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));

        foreach (var name in batchList)
        {
            itemName.Data(name);
            Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));

        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));

        if (fieldIdList != null)
        {
            UInt tempUInt = new();
            Int tempInt = new();

            // encode view request
            eEntry.Clear();
            eEntry.Name = ElementNames.VIEW_TYPE;
            eEntry.DataType = DataTypes.UINT;

            tempUInt.Value(ViewTypes.FIELD_ID_LIST);

            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.Encode(encodeIter, tempUInt));

            eEntry.Clear();
            eEntry.Name = ElementNames.VIEW_DATA;
            eEntry.DataType = DataTypes.ARRAY;
            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));
            elementArray.Clear();
            elementArray.PrimitiveType = DataTypes.INT;
            elementArray.ItemLength = 0;

            Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));

            foreach (int viewField in fieldIdList)
            {
                ae.Clear();
                tempInt.Value(viewField);
                Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, tempInt));
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeComplete(encodeIter, true));

        requestMsg.EncodedDataBody = buf;
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
    }
    /* Encodes an ElementEntry requesting enhanced symbol list behaviors. */
    internal static void EncodeSymbolListBehaviorsElement(EncodeIterator encIter, long dataStreamFlags)
    {
        CodecReturnCode ret;
        ElementList behaviorList = new ElementList();
        ElementEntry elementEntry = new ElementEntry();
        ElementEntry dataStreamEntry = new ElementEntry();
        UInt tempUInt = new UInt();

        elementEntry.Clear();
        elementEntry.Name.Data(":SymbolListBehaviors");
        elementEntry.DataType = DataTypes.ELEMENT_LIST;

        ret = elementEntry.EncodeInit(encIter, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        behaviorList.Clear();
        behaviorList.ApplyHasStandardData();
        ret = behaviorList.EncodeInit(encIter, null, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        dataStreamEntry.Clear();
        dataStreamEntry.Name.Data(":DataStreams");
        dataStreamEntry.DataType = DataTypes.UINT;
        tempUInt.Value(dataStreamFlags);
        ret = dataStreamEntry.Encode(encIter, tempUInt);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        ret = behaviorList.EncodeComplete(encIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        ret = elementEntry.EncodeComplete(encIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
    }

    internal static void EncodeViewFieldIdList(ReactorChannel reactorChannel, List<int> fieldIdList, IRequestMsg msg)
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        Int tempInt = new();
        UInt tempUInt = new();
        Array viewArray = new();
        EncodeIterator encodeIter = new();
        encodeIter.SetBufferAndRWFVersion(buf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
        ElementList elementList = new();
        ElementEntry elementEntry = new();
        ArrayEntry arrayEntry = new();

        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 0));

        elementEntry.Clear();
        elementEntry.Name = ElementNames.VIEW_TYPE;
        elementEntry.DataType = DataTypes.UINT;

        tempUInt.Value(ViewTypes.FIELD_ID_LIST);

        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(encodeIter, tempUInt));

        elementEntry.Clear();
        elementEntry.Name = ElementNames.VIEW_DATA;
        elementEntry.DataType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(encodeIter, 0));
        viewArray.PrimitiveType = DataTypes.INT;
        viewArray.ItemLength = 2;

        Assert.Equal(CodecReturnCode.SUCCESS, viewArray.EncodeInit(encodeIter));

        foreach (int viewField in fieldIdList)
        {
            arrayEntry.Clear();
            tempInt.Value(viewField);
            Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(encodeIter, tempInt));
        }
        Assert.Equal(CodecReturnCode.SUCCESS, viewArray.EncodeComplete(encodeIter, true));
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeComplete(encodeIter, true));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        msg.ContainerType = DataTypes.ELEMENT_LIST;
        msg.EncodedDataBody = buf;
    }

    internal static void EncodeViewElementNameList(ReactorChannel reactorChannel, List<string> elementNameList, IRequestMsg msg)
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(2024));
        Buffer tempBuf = new();
        UInt tempUInt = new();
        Array viewArray = new();
        EncodeIterator encodeIter = new();
        encodeIter.SetBufferAndRWFVersion(buf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
        ElementList elementList = new();
        ElementEntry elementEntry = new();
        ArrayEntry arrayEntry = new();

        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 0));

        elementEntry.Clear();
        elementEntry.Name = ElementNames.VIEW_TYPE;
        elementEntry.DataType = DataTypes.UINT;

        tempUInt.Value(ViewTypes.ELEMENT_NAME_LIST);

        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(encodeIter, tempUInt));

        elementEntry.Clear();
        elementEntry.Name = ElementNames.VIEW_DATA;
        elementEntry.DataType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(encodeIter, 0));
        viewArray.PrimitiveType = DataTypes.ASCII_STRING;
        viewArray.ItemLength = 0;

        Assert.Equal(CodecReturnCode.SUCCESS, viewArray.EncodeInit(encodeIter));

        foreach (string name in elementNameList)
        {
            arrayEntry.Clear();
            tempBuf.Data(name);
            Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(encodeIter, tempBuf));
        }
        Assert.Equal(CodecReturnCode.SUCCESS, viewArray.EncodeComplete(encodeIter, true));
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeComplete(encodeIter, true));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        msg.ContainerType = DataTypes.ELEMENT_LIST;
        msg.EncodedDataBody = buf;
    }

    private void TearDownConsumerAndProvider(TestReactor consumerReactor, TestReactor providerReactor, Consumer consumer, Provider provider)
    {
        consumerReactor.Close();
        providerReactor.Close();
    }

    internal static bool CheckHasCorrectView(Provider provider, IRequestMsg requestMsg, List<int> viewFieldList)
    {
        ElementList elementList = new();
        ElementEntry elementEntry = new();
        DecodeIterator dIter = new();
        Buffer viewDataElement = null;
        elementEntry.Clear();
        elementList.Clear();
        elementEntry.Clear();
        dIter.Clear();
        int numOfFields = 0;
        CodecReturnCode ret;
        int majorVersion = provider.ReactorChannel.MajorVersion;
        int minorVersion = provider.ReactorChannel.MinorVersion;

        dIter.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, majorVersion, minorVersion);

        if (requestMsg.ContainerType != DataTypes.ELEMENT_LIST)
            return false;

        if (elementList.Decode(dIter, null) != CodecReturnCode.SUCCESS)
            return false;

        bool viewDataFound = false;
        bool hasViewType = false;
        while ((ret = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
                return false;
            else
            {
                if (elementEntry.Name.Equals(ElementNames.VIEW_TYPE) &&
                        elementEntry.DataType == DataTypes.UINT)
                {
                    hasViewType = true;
                }

                if (elementEntry.Name.Equals(ElementNames.VIEW_DATA) &&
                    elementEntry.DataType == DataTypes.ARRAY)
                {
                    viewDataElement = elementEntry.EncodedData;
                    viewDataFound = true;
                }
            }
        } // while


        if (!viewDataFound || !hasViewType)
        {
            return false;
        }
        else
        {
            dIter.Clear();
            dIter.SetBufferAndRWFVersion(viewDataElement, majorVersion, minorVersion);

            Array viewArray = new();
            ArrayEntry viewArrayEntry = new();
            viewArray.Clear();
            Int fieldId = new();

            if ((ret = viewArray.Decode(dIter)) == CodecReturnCode.SUCCESS)
            {
                if (viewArray.PrimitiveType != DataTypes.INT)
                    return false;

                while ((ret = viewArrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (ret < CodecReturnCode.SUCCESS)
                        return false;
                    else
                    {
                        if ((ret = fieldId.Decode(dIter)) == CodecReturnCode.SUCCESS)
                        {
                            if (!viewFieldList.Contains((int)fieldId.ToLong()))
                                return false;
                            numOfFields++;
                        }
                        else
                            return false;
                    }
                }// while
            }
            else
                return false;
        }

        if (numOfFields > viewFieldList.Count)
            return false;

        return true;
    }

    internal static bool CheckHasCorrectViewEname(Provider provider, IRequestMsg requestMsg, List<string> viewFieldList)
    {
        ElementList elementList = new();
        ElementEntry elementEntry = new();
        DecodeIterator dIter = new();
        Buffer viewDataElement = null;
        elementEntry.Clear();
        elementList.Clear();
        elementEntry.Clear();
        dIter.Clear();
        int numOfFields = 0;
        CodecReturnCode ret;
        int majorVersion = provider.ReactorChannel.MajorVersion;
        int minorVersion = provider.ReactorChannel.MinorVersion;

        dIter.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, majorVersion, minorVersion);

        if (requestMsg.ContainerType != DataTypes.ELEMENT_LIST)
            return false;

        if (elementList.Decode(dIter, null) != CodecReturnCode.SUCCESS)
            return false;

        bool viewDataFound = false;
        bool hasViewType = false;
        while ((ret = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
                return false;
            else
            {
                if (elementEntry.Name.Equals(ElementNames.VIEW_TYPE) &&
                        elementEntry.DataType == DataTypes.UINT)
                {
                    hasViewType = true;
                }

                if (elementEntry.Name.Equals(ElementNames.VIEW_DATA) &&
                    elementEntry.DataType == DataTypes.ARRAY)
                {
                    viewDataElement = elementEntry.EncodedData;
                    viewDataFound = true;
                }
            }
        } // while


        if (!viewDataFound || !hasViewType)
        {
            return false;
        }
        else
        {
            dIter.Clear();
            dIter.SetBufferAndRWFVersion(viewDataElement, majorVersion, minorVersion);

            Array viewArray = new();
            ArrayEntry viewArrayEntry = new();
            viewArray.Clear();
            Buffer elementName = new();

            if ((ret = viewArray.Decode(dIter)) == CodecReturnCode.SUCCESS)
            {
                if (viewArray.PrimitiveType != DataTypes.ASCII_STRING)
                    return false;

                while ((ret = viewArrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (ret < CodecReturnCode.SUCCESS)
                        return false;
                    else
                    {
                        if ((ret = elementName.Decode(dIter)) == CodecReturnCode.SUCCESS)
                        {
                            if (!viewFieldList.Contains(elementName.ToString()))
                                return false;
                            numOfFields++;
                        }
                        else
                            return false;
                    }
                }// while
            }
            else
                return false;
        }

        if (numOfFields > viewFieldList.Count)
            return false;

        return true;
    }

    internal static void EncodeSymbolListDataBody(ReactorChannel reactorChannel, Buffer dataBodyBuf, SymbolAction[] symbolList)
    {
        Map tempMap = new();
        MapEntry tempMapEntry = new();
        EncodeIterator encodeIter = new();

        /* encode symbolist data body */
        encodeIter.SetBufferAndRWFVersion(dataBodyBuf, reactorChannel.MajorVersion,
                reactorChannel.MinorVersion);

        tempMap.Clear();
        tempMap.Flags = 0;
        tempMap.ContainerType = DataTypes.NO_DATA;
        tempMap.KeyPrimitiveType = DataTypes.BUFFER;
        CodecReturnCode ret = tempMap.EncodeInit(encodeIter, 0, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        /* encode map entry */
        tempMapEntry.Clear();
        tempMapEntry.Flags = MapEntryFlags.NONE;

        for (int i = 0; i < symbolList.Length; i++)
        {
            tempMapEntry.Action = symbolList[i].Action;
            ret = tempMapEntry.Encode(encodeIter, symbolList[i].ItemName);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
        }
        ret = tempMap.EncodeComplete(encodeIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
    }

    private void EncodeBatchWithSymbolList(ReactorChannel rc, IRequestMsg msg, bool hasBehaviors)
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, rc.MajorVersion, rc.MinorVersion);

        // encode payload

        ElementList eList = new();
        ElementEntry eEntry = new();
        Array elementArray = new ();
        ArrayEntry ae = new();
        Buffer itemName = new();

        eList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.Name = ElementNames.BATCH_ITEM_LIST;
        eEntry.DataType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.PrimitiveType = DataTypes.ASCII_STRING;
        elementArray.ItemLength = 0;

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));
        itemName.Data("TRI.N");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));
        itemName.Data("IBM.N");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));

        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));

        if (hasBehaviors)
        {
            ElementList behaviorList = new();
            ElementEntry dataStreamEntry = new();
            UInt tempUInt = new();

            eEntry.Clear();
            eEntry.Name.Data(":SymbolListBehaviors");
            eEntry.DataType = DataTypes.ELEMENT_LIST;

            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

            behaviorList.Clear();
            behaviorList.ApplyHasStandardData();

            Assert.Equal(CodecReturnCode.SUCCESS, behaviorList.EncodeInit(encodeIter, null, 0));

            dataStreamEntry.Clear();
            dataStreamEntry.Name.Data(":DataStreams");
            dataStreamEntry.DataType = DataTypes.UINT;
            tempUInt.Value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            Assert.Equal(CodecReturnCode.SUCCESS, dataStreamEntry.Encode(encodeIter, tempUInt));

            Assert.Equal(CodecReturnCode.SUCCESS, behaviorList.EncodeComplete(encodeIter, true));

            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeComplete(encodeIter, true));

        msg.EncodedDataBody = buf;
    }

    #endregion

    public WatchlistItemDomainsTest()
    {
    }

    [Fact]
    public void EmptyStatusMsgTest()
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        IStatusMsg statusMsg = new Msg();
        IStatusMsg receivedStatusMsg;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = new Msg();
        IUpdateMsg receivedUpdateMsg;

        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);


        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives first refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends an empty status msg .*/
        statusMsg.Clear();
        statusMsg.MsgClass = MsgClasses.STATUS;
        statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
        statusMsg.StreamId = providerStreamId;
        statusMsg.ContainerType = DataTypes.NO_DATA;
        statusMsg.ApplyHasMsgKey();
        statusMsg.MsgKey.ApplyHasServiceId();
        statusMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        statusMsg.MsgKey.ApplyHasName();
        statusMsg.MsgKey.Name.Data("TRI.N");

        Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives status msg. */
        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.True(receivedStatusMsg.CheckHasMsgKey());
        Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedStatusMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedStatusMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.False(receivedStatusMsg.CheckHasState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void PrivateStreamSubmitTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit private stream request message
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyPrivateStream();
        requestMsg.ApplyHasExtendedHdr();
        Buffer extendedHdrBuffer = new Buffer();
        extendedHdrBuffer.Data("EXTENDED HEADER");
        requestMsg.ExtendedHeader = extendedHdrBuffer;
        requestMsg.ContainerType = DataTypes.OPAQUE;
        Buffer encodeDataBodyBuffer = new Buffer();
        encodeDataBodyBuffer.Data("ENCODED DATA BODY");
        requestMsg.EncodedDataBody = encodeDataBodyBuffer;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr());
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType);
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyPrivateStream();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckPrivateStream());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SnapshotOnStreamingAggregationTest_Socket()
    {
        SnapshotOnStreamingAggregationTest(false);
    }

    [Fact]
    public void SnapshotOnStreamingAggregationTest_DispatchItemRequests_Socket()
    {
        SnapshotOnStreamingAggregationTest(true);
    }

    private void SnapshotOnStreamingAggregationTest(bool dispatchBetweenItemRequests)
    {
        /* Test aggregation of a snapshot request onto a streaming request. */
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = new Msg();
        IUpdateMsg receivedUpdateMsg;
        int providerStreamId;
        string userSpec = "997";

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends streaming request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = userSpec;
        if (dispatchBetweenItemRequests)
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        else
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer sends snapshot request for same item (does not apply streaming flag). */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = userSpec;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives one request, priority 1/1 (snapshot doesn't count towards priority). */
        providerReactor.Dispatch(1);

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refreshes, one with state OPEN on stream 5, and one NON-STREAMING on 6. */
        consumerReactor.Dispatch(2);

        /* Streaming refresh. */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(userSpec, msgEvent.StreamInfo.UserSpec);

        /* Snapshot refresh. */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(userSpec, msgEvent.StreamInfo.UserSpec);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the update, only on stream 5. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Request the snapshot again on stream 6. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = userSpec;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives nonstreaming refresh, only on stream 6 (refresh was solicited by stream 6; stream 5 doesn't need it). */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(userSpec, msgEvent.StreamInfo.UserSpec);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the update, only on stream 5. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);

    } // End SnapshotOnStreamingAggregationTest

    [Fact]
    public void BatchRequestTest()
    {
        /* Test a simple batch request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        IMsg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Received status message with closed batch stream
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal("DEFAULT_SERVICE", msgEvent.StreamInfo.ServiceName);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());

        /* Provider receives request. */
        providerReactor.Dispatch(2);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        // Provider processes request for IBM.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message with item IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestWithViewTest()
    {
        /* Test a simple batch view request/refresh exchange with the watchlist enabled. */
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);
        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();
        requestMsg.ApplyHasView();

        List<int> viewFieldList = new();
        viewFieldList.Add(6);
        viewFieldList.Add(12);
        viewFieldList.Add(13);

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, viewFieldList);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Received status message with closed batch stream
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal("DEFAULT_SERVICE", msgEvent.StreamInfo.ServiceName);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());

        List<int> decodedViewFieldList = new();

        /* Provider receives request. */
        providerReactor.Dispatch(2);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        // Checks whether the provider receives the view request
        DecodeViewDataForFieldId(consumer.ReactorChannel, receivedRequestMsg, decodedViewFieldList);
        Assert.True(Enumerable.SequenceEqual(viewFieldList, decodedViewFieldList));

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.FIELD_LIST;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        // Encodes Fieldlist as payload for the view
        EncodeViewDataForFieldId(provider.ReactorChannel, refreshMsg, decodedViewFieldList);

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.FIELD_LIST, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.EncodedDataBody.Equals(refreshMsg.EncodedDataBody));

        // Provider processes request for IBM.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        // Checks whether the provider receives the view request
        DecodeViewDataForFieldId(consumer.ReactorChannel, receivedRequestMsg, decodedViewFieldList);
        Assert.True(Enumerable.SequenceEqual(viewFieldList, decodedViewFieldList));

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.FIELD_LIST;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        // Encodes Fieldlist as payload for the view
        EncodeViewDataForFieldId(provider.ReactorChannel, refreshMsg, decodedViewFieldList);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message with item IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.FIELD_LIST, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.EncodedDataBody.Equals(refreshMsg.EncodedDataBody)); //transformation to json alters the hint and value of the real number (without altering the real number)

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SymbolListDataStreamTest()
    {
        /* Test a symbolList data stream request/refresh exchange with the watchlist enabled. */
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        requestMsg.ApplyStreaming();

        Buffer payload = new();
        payload.Data(new ByteBuffer(1024));

        EncodeIterator encIter = new EncodeIterator();
        encIter.Clear();
        encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MajorVersion);

        requestMsg.Clear();
        /* set-up message */
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyHasQos();
        requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
        requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 1;
        requestMsg.ApplyStreaming();

        ElementList elementList = new();
        elementList.Clear();
        elementList.ApplyHasStandardData();

        CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
        ret = elementList.EncodeComplete(encIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        requestMsg.EncodedDataBody = payload;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;

        refreshMsg.ContainerType = DataTypes.MAP;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Map tempMap = new();
        MapEntry tempMapEntry = new();
        Buffer tempBuffer = new();
        EncodeIterator encodeIter = new();
        Buffer msgBuf = new();
        msgBuf.Data(new ByteBuffer(2048));

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MinorVersion);
        tempMap.Clear();
        tempMap.Flags = 0;
        tempMap.ContainerType = DataTypes.NO_DATA;
        tempMap.KeyPrimitiveType = DataTypes.BUFFER;
        ret = tempMap.EncodeInit(encodeIter, 0, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        /* encode map entry */
        tempMapEntry.Clear();
        tempMapEntry.Flags = MapEntryFlags.NONE;
        tempBuffer.Clear();

        tempBuffer.Data("FB.O");
        tempMapEntry.Action = MapEntryActions.ADD;

        ret = tempMapEntry.EncodeInit(encodeIter, tempBuffer, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        ret = tempMapEntry.EncodeComplete(encodeIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        tempBuffer.Data("AAPL.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        tempBuffer.Data("NFLX.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        tempBuffer.Data("GOOGL.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        ret = tempMap.EncodeComplete(encodeIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        refreshMsg.EncodedDataBody = msgBuf;
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // watchlist receives a symbol list data refresh and send out each item request
        consumerReactor.Reactor.m_ReactorOptions.XmlTracing = false;
        consumerReactor.Dispatch(1);

        consumerReactor.m_EventQueue.Clear();

        // provider receives request and send refresh back for the first market price item
        providerReactor.Dispatch(4);

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name = receivedRequestMsg.MsgKey.Name;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        providerReactor.PollEvent();
        providerReactor.PollEvent();
        providerReactor.PollEvent();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);


        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal(receivedRefreshMsg.MsgKey.Name.ToString(), receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemViewAggregateStreamingTest()
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg1 = new Msg();
        IRequestMsg requestMsg2 = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
        List<int> viewFieldList = new();

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit two aggregated request messages
        requestMsg1.Clear();
        requestMsg1.MsgClass = MsgClasses.REQUEST;
        requestMsg1.StreamId = 5;
        requestMsg1.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg1.ApplyStreaming();
        requestMsg1.ApplyHasView();
        requestMsg1.MsgKey.ApplyHasName();
        requestMsg1.MsgKey.Name.Data("VRX");
        viewFieldList.Add(22);
        viewFieldList.Add(22);
        viewFieldList.Add(6);
        viewFieldList.Add(0);
        viewFieldList.Add(130);
        viewFieldList.Add(1131);
        viewFieldList.Add(1025);
        EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg1);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg1, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives one request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        providerStreamId = receivedRequestMsg.StreamId;

        List<int> requestedViewList = new();
        List<int> expectedViewList = new()
        {
            0,
            6,
            22,
            130,
            1025,
            1131
        };

        // Checks the request view list.
        DecodeViewDataForFieldId(consumer.ReactorChannel, receivedRequestMsg, requestedViewList);

        Assert.True(Enumerable.SequenceEqual(expectedViewList, requestedViewList));

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("VRX");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
  
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName,Provider.DefaultService.Info.ServiceName.ToString());        
        
        
        // Consumer send 2nd request
        requestMsg2.Clear();
        requestMsg2.MsgClass = MsgClasses.REQUEST;
        requestMsg2.StreamId = 6;
        requestMsg2.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg2.ApplyStreaming();
        requestMsg2.ApplyHasView();
        requestMsg2.MsgKey.ApplyHasName();
        requestMsg2.MsgKey.Name.Data("VRX");
        viewFieldList.Clear();
        viewFieldList.Add(8);
        viewFieldList.Add(88);
        viewFieldList.Add(130);
        viewFieldList.Add(24);
        viewFieldList.Add(989);
        viewFieldList.Add(45);
        viewFieldList.Add(45);
        EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg2);
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg2, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives one request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        providerStreamId = receivedRequestMsg.StreamId;

        expectedViewList = new()
        {
            0,
            6,
            8,
            22,
            24,
            45,
            88,
            130,
            989,
            1025,
            1131
        };

        // Checks the request view list.
        DecodeViewDataForFieldId(consumer.ReactorChannel, receivedRequestMsg, requestedViewList);

        Assert.True(Enumerable.SequenceEqual(expectedViewList, requestedViewList));

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("VRX");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        // 2nd event to consumer
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /////////////////////////////////////////////////////////////////
        // consumer reissues 2nd request without specifying the view flag
        requestMsg2.Clear();
        requestMsg2.MsgClass =MsgClasses.REQUEST;
        requestMsg2.StreamId = 6;
        requestMsg2.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg2.ApplyStreaming();
        requestMsg2.MsgKey.ApplyHasName();
        requestMsg2.MsgKey.Name.Data("VRX");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg2, submitOptions) >= ReactorReturnCode.SUCCESS);
              
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        // NO View
        Assert.False(receivedRequestMsg.CheckHasView());
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType);
        providerStreamId = receivedRequestMsg.StreamId;

         /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("VRX");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();


        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
         
        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent(); 
        testEvent = consumerReactor.PollEvent(); 
        
        // consumer reissues 1st request with pause
        requestMsg1.ApplyPause();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg1, submitOptions) >= ReactorReturnCode.SUCCESS);
                 
        // consumer reissues 2nd request with pause
        requestMsg2.ApplyPause();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg2, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Provider receives pause request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming()); 
        // PAUSE 
        Assert.True(receivedRequestMsg.CheckPause()); 
        providerStreamId = receivedRequestMsg.StreamId;

        // resume
        // consumer reissues 1st request with no pause
        requestMsg1.Flags &= ~RequestMsgFlags.PAUSE;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg1, submitOptions) >= ReactorReturnCode.SUCCESS);
               
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        // NO PAUSE now
        Assert.False(receivedRequestMsg.CheckPause()); 
        providerStreamId = receivedRequestMsg.StreamId;
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

     class ItemViewAggregateSnapshotBeforeChannelReady : Consumer
     {
       public ItemViewAggregateSnapshotBeforeChannelReady(TestReactor testReactor):
            base(testReactor)
       {
       }

       public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
       {
           ReactorSubmitOptions submitOptions = new();
           Msg msg = new();
           IRequestMsg requestMsg = (IRequestMsg)msg;
           List<int> viewFieldList = new List<int>();
           
           if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
           {
               base.ReactorChannelEventCallback(evt);
               
            // submit request view streaming message
               requestMsg.Clear();
               requestMsg.MsgClass = MsgClasses.REQUEST;
               requestMsg.StreamId = 5;
               requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
               requestMsg.ApplyStreaming();
               requestMsg.ApplyHasView();
               requestMsg.MsgKey.ApplyHasName();
               requestMsg.MsgKey.Name.Data("VRX");
               viewFieldList.Add(22);
               viewFieldList.Add(22);
               viewFieldList.Add(6);
               viewFieldList.Add(0);
               viewFieldList.Add(130);
               viewFieldList.Add(1131);
               viewFieldList.Add(1025);                
               EncodeViewFieldIdList(ReactorChannel, viewFieldList, requestMsg);

               submitOptions.Clear();
               submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
               Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
               
               // submit request non-view snapshot message
               requestMsg.Clear();
               requestMsg.MsgClass = MsgClasses.REQUEST;
               requestMsg.StreamId = 6;
               requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
               requestMsg.MsgKey.ApplyHasName();
               requestMsg.MsgKey.Name.Data("VRX");         

               submitOptions.Clear();
               submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
               Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
           }
           else
           {
               return base.ReactorChannelEventCallback(evt);
           }
           
           return ReactorCallbackReturnCode.SUCCESS;
       }
   }

    [Fact]
    public void ItemViewAggregateSnapshotBeforeChannelReadyTest()
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new ItemViewAggregateSnapshotBeforeChannelReady(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        opts.NumStatusEvents = 2; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);

        /* Provider receives one request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType);
        Assert.False(receivedRequestMsg.CheckHasView());

        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("VRX");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
  
        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        List<int> viewFieldList = new List<int>();
        viewFieldList.Add(22);
        viewFieldList.Add(22);
        viewFieldList.Add(6);
        viewFieldList.Add(0);
        viewFieldList.Add(130);
        viewFieldList.Add(1131);
        viewFieldList.Add(1025);

        /* Provider receives one view request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(DataTypes.ELEMENT_LIST, receivedRequestMsg.ContainerType);
        Assert.True(receivedRequestMsg.CheckHasView());
        Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList));

        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("VRX");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
       
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    private void SetUp(bool setUpLogin, bool setUpDirectory, out Consumer consumer, out Provider provider, out TestReactor consumerReactor, out TestReactor providerReactor)
    {
        consumerReactor = new TestReactor();
        providerReactor = new TestReactor();

        consumer = new Consumer(consumerReactor);
        provider = new Provider(providerReactor);

        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        if (setUpLogin)
        {
            consumerRole.InitDefaultRDMLoginRequest();
        }
        if (setUpDirectory)
        {
            consumerRole.InitDefaultRDMDirectoryRequest();
        }

        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        // Connect the consumer and provider
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = setUpLogin;
        opts.SetupDefaultDirectoryStream = setUpDirectory;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
    }

    [Fact]
    public void DirectoryUserRequestTest()
    {
        // Test the user calling the initial directory SubmitRequest with no default directory request
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        submitOptions.Clear();

        SetUp(true, true, out var consumer, out var provider, out var consumerReactor, out var providerReactor);
        
        // Consumer submits source directory request
        DirectoryRequest directoryRequest = new DirectoryRequest();
        directoryRequest.StreamId = 2;
        directoryRequest.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;
        directoryRequest.Streaming = true;
        Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives directory refresh
        consumerReactor.Dispatch(1);
        var evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        
        // Check Codec Message
        Assert.Equal(MsgClasses.REFRESH, directoryMsgEvent.Msg.MsgClass);
        IRefreshMsg refreshMsg = (IRefreshMsg)directoryMsgEvent.Msg;
        Assert.Equal((int)DomainType.SOURCE, refreshMsg.DomainType);
        Assert.Equal(2, refreshMsg.StreamId);
        Assert.True(refreshMsg.CheckSolicited());
        Assert.True(refreshMsg.CheckRefreshComplete());

        // Check RDM Message
        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
        DirectoryRefresh directoryRefresh = (DirectoryRefresh)directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
        Assert.Equal(2, directoryRefresh.StreamId);
        Assert.True(directoryRefresh.Solicited);
        Assert.True(directoryRefresh.ServiceList[0].HasInfo);
        Assert.True(directoryRefresh.ServiceList[0].HasState);
        Assert.True(directoryRefresh.ServiceList[0].GroupStateList.Count == 0); // cached refresh doesn't have group stuff even though user requests it
        
        // reissue same source directory request
        Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives directory refresh
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        
        // Check Codec Message
        Assert.Equal(MsgClasses.REFRESH, directoryMsgEvent.Msg.MsgClass);
        refreshMsg = (IRefreshMsg)directoryMsgEvent.Msg;
        Assert.Equal((int)DomainType.SOURCE, refreshMsg.DomainType);
        Assert.Equal(2, refreshMsg.StreamId);
        Assert.True(refreshMsg.CheckSolicited());
        Assert.True(refreshMsg.CheckRefreshComplete());

        // Check RDM Message
        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
        directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
        Assert.Equal(2, directoryRefresh.StreamId);
        Assert.True(directoryRefresh.Solicited);
        Assert.True(directoryRefresh.ServiceList[0].HasInfo);
        Assert.True(directoryRefresh.ServiceList[0].HasState);
        Assert.True(directoryRefresh.ServiceList[0].GroupStateList.Count == 0); // cached refresh doesn't have group stuff even though user requests it

        // reissue same source directory request with no state filter and extra load filter 
        directoryRequest.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.GROUP;
        Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives directory refresh
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        
        // Check Codec Message
        Assert.Equal(MsgClasses.REFRESH, directoryMsgEvent.Msg.MsgClass);
        refreshMsg = (IRefreshMsg)directoryMsgEvent.Msg;
        Assert.Equal((int)DomainType.SOURCE, refreshMsg.DomainType);
        Assert.Equal(2, refreshMsg.StreamId);
        Assert.True(refreshMsg.CheckSolicited());
        Assert.True(refreshMsg.CheckRefreshComplete());

        // Check RDM Message
        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
        directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
        Assert.Equal(2, directoryRefresh.StreamId);
        Assert.True(directoryRefresh.Solicited);
        Assert.True(directoryRefresh.ServiceList[0].HasInfo);
        Assert.False(directoryRefresh.ServiceList[0].HasState);
        Assert.True(directoryRefresh.ServiceList[0].GroupStateList.Count == 0); // cached refresh doesn't have group stuff even though user requests it
        Assert.False(directoryRefresh.ServiceList[0].HasLoad); // cached refresh doesn't have load stuff even though user requests it

        // reissue same source directory request with state filter back on and extra load filter 
        directoryRequest.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.GROUP;
        Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives directory refresh
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;

        // Check Codec Message
        Assert.Equal(MsgClasses.REFRESH, directoryMsgEvent.Msg.MsgClass);
        refreshMsg = (IRefreshMsg)directoryMsgEvent.Msg;
        Assert.Equal((int)DomainType.SOURCE, refreshMsg.DomainType);
        Assert.Equal(2, refreshMsg.StreamId);
        Assert.True(refreshMsg.CheckSolicited());
        Assert.True(refreshMsg.CheckRefreshComplete());

        // Check RDM Message
        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
        directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
        Assert.Equal(2, directoryRefresh.StreamId);
        Assert.True(directoryRefresh.Solicited);
        Assert.True(directoryRefresh.ServiceList[0].HasInfo);
        Assert.True(directoryRefresh.ServiceList[0].HasState);
        Assert.True(directoryRefresh.ServiceList[0].GroupStateList.Count == 0); // cached refresh doesn't have group stuff even though user requests it
        Assert.False(directoryRefresh.ServiceList[0].HasLoad); // cached refresh doesn't have load stuff even though user requests it

        // Consumer submits source directory request with a different stream id and no info filter
        directoryRequest.StreamId = 3;
        directoryRequest.Filter = Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;
        Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives directory refresh
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;

        // Check Codec Message
        Assert.Equal(MsgClasses.REFRESH, directoryMsgEvent.Msg.MsgClass);
        refreshMsg = (IRefreshMsg)directoryMsgEvent.Msg;
        Assert.Equal((int)DomainType.SOURCE, refreshMsg.DomainType);
        Assert.Equal(3, refreshMsg.StreamId);
        Assert.True(refreshMsg.CheckSolicited());
        Assert.True(refreshMsg.CheckRefreshComplete());

        // Check RDM Message
        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
        directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
        Assert.Equal(3, directoryRefresh.StreamId);
        Assert.True(directoryRefresh.Solicited);
        Assert.False(directoryRefresh.ServiceList[0].HasInfo);
        Assert.True(directoryRefresh.ServiceList[0].HasState);
        Assert.True(directoryRefresh.ServiceList[0].GroupStateList.Count == 0); // cached refresh doesn't have group stuff even though user requests it
        
        // Consumer submits source directory request with a different stream id and all filters
        directoryRequest.StreamId = 4;
        directoryRequest.Filter = Directory.ServiceFilterFlags.DATA |
                                Directory.ServiceFilterFlags.GROUP |
                                Directory.ServiceFilterFlags.INFO |
                                Directory.ServiceFilterFlags.LINK |
                                Directory.ServiceFilterFlags.LOAD |
                                Directory.ServiceFilterFlags.STATE;
        Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives directory refresh
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;

        // Check Codec Message
        Assert.Equal(MsgClasses.REFRESH, directoryMsgEvent.Msg.MsgClass);
        refreshMsg = (IRefreshMsg)directoryMsgEvent.Msg;
        Assert.Equal((int)DomainType.SOURCE, refreshMsg.DomainType);
        Assert.Equal(4, refreshMsg.StreamId);
        Assert.True(refreshMsg.CheckSolicited());
        Assert.True(refreshMsg.CheckRefreshComplete());

        // Check RDM Message
        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
        directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
        Assert.Equal(4, directoryRefresh.StreamId);
        Assert.True(directoryRefresh.Solicited);
        Assert.True(directoryRefresh.ServiceList[0].HasInfo);
        Assert.True(directoryRefresh.ServiceList[0].HasState);
        Assert.True(directoryRefresh.ServiceList[0].GroupStateList.Count == 0); // cached refresh doesn't have group stuff even though user requests it
        Assert.False(directoryRefresh.ServiceList[0].HasData);
        Assert.False(directoryRefresh.ServiceList[0].HasLink);
        Assert.False(directoryRefresh.ServiceList[0].HasLoad);

        TestReactorComponent.CloseSession(consumer, provider);
        consumerReactor.Close();
        providerReactor.Close();
    }

    class CloseUserRequestFromDirectoryCallbackConsumer : Consumer
    {
        public CloseUserRequestFromDirectoryCallbackConsumer(TestReactor testReactor) : base(testReactor)
        {
        }

        public override ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent evt)
        {
            base.RdmDirectoryMsgCallback(evt);

            // close user stream 7
            ICloseMsg closeMsg = (ICloseMsg)new Msg();
            closeMsg.Clear();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = 7;
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
            closeMsg.ContainerType = DataTypes.NO_DATA;

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            ReactorErrorInfo errorInfo = new ReactorErrorInfo();
            if (evt.ReactorChannel.Submit((Msg)closeMsg, submitOptions, out errorInfo) != ReactorReturnCode.SUCCESS)
            {
                Assert.True(false);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void ServiceDownCloseItemRecoverTest()
    {
        /* Test a simple request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg receivedUpdateMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new CloseUserRequestFromDirectoryCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);


        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("IBM.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.ApplyRefreshComplete();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 7;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(2, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives 1 refreshe - for each TRI.N. */
        consumerReactor.Dispatch(1);
        // first refresh
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends service update to bring service down.*/
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.State.HasStatus = true;
        wlService.RdmService.State.Action = FilterEntryActions.SET;
        wlService.RdmService.State.Status.DataState(DataStates.SUSPECT);
        wlService.RdmService.State.Status.StreamState(StreamStates.CLOSED_RECOVER);
        wlService.RdmService.State.HasAcceptingRequests = true;
        wlService.RdmService.State.AcceptingRequests = 1;
        wlService.RdmService.State.ServiceStateVal = 0;
        wlService.RdmService.ServiceId = 1;

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives directory update. Consumer receives status messages for 5, 7, and 6.
         * Consumer receives status for 5 and 6 again (due to recovery attempt), but not 7 since it was closed during rdmDirectoryMsgCallback. */
        consumerReactor.Dispatch(6);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(7, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(6, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.True(receivedUpdateMsg.CheckHasMsgKey());
        Assert.Equal((int)DomainType.SOURCE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedUpdateMsg.ContainerType);

        //Assert.Equal(0, consumerReactor._componentList.Get(0).ReactorChannel().watchlist().DirectoryHandler().Service(1).StreamList().Size());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(6, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends service update to bring service back up.*/
        wlService.RdmService.State.Status.DataState(DataStates.OK);
        wlService.RdmService.State.Status.StreamState(StreamStates.OPEN);
        wlService.RdmService.State.ServiceStateVal = 1;
        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // dispatch service update sent to consumer
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();

        /* Provider receives 2 requests for recovery (stream ids 5 and 6). */
        providerReactor.Dispatch(2);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        TestReactorComponent.CloseSession(consumer, provider);
        consumerReactor.Close();
        providerReactor.Close();
    }

    [Fact]
    public void GroupMergeAndStatusFanoutTest()
    {

        /* Test updating an item group via item-specific message and group status. 
         * - Open an item (and another item)
         * - Change the first item's group via item message. Send a group close on the original group to make sure it moved.
         * - Change the first item's group again via group status. Send a group close to close it. Send a group close on the 
         *   previous group to make sure nothing happens.
         */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        RDMDirectoryMsgEvent directoryMsgEvent;
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        DirectoryUpdate receivedUpdateMsg;
        Service service = new Service();
        ServiceGroup serviceGroup = new ServiceGroup();
        int providerStreamId;

        Buffer groupId1 = new Buffer();
        groupId1.Data("ONE");

        Buffer groupId2 = new Buffer();
        groupId2.Data("TWO");

        Buffer groupId3 = new Buffer();
        groupId3.Data("TREE");

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh, setting item on group ONE. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.GroupId = groupId1;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends refresh, moving item to group TWO. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.GroupId = groupId2;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends group update on group ONE. This should not close any items. */
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        service.Clear();
        service.HasState = true;
        service.Action = MapEntryActions.UPDATE;
        service.ServiceId = 1;
        serviceGroup.Clear();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(DataStates.SUSPECT);
        serviceGroup.Status.StreamState(StreamStates.CLOSED);
        serviceGroup.Group = groupId1;
        service.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(service);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(1);

        /* Directory update with the group status. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Merge group TWO to group TREE. */
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        service.Clear();
        service.HasState = true;
        service.Action = MapEntryActions.UPDATE;
        service.ServiceId = 1;
        serviceGroup.Clear();
        serviceGroup.Group = groupId2;
        serviceGroup.HasMergedToGroup = true;
        serviceGroup.MergedToGroup = groupId3;
        service.GroupStateList.Add(serviceGroup);
        directoryUpdateMsg.ServiceList.Add(service);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Consumer requests second item. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("IBM.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh, putting the second item in group TWO. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        refreshMsg.GroupId = groupId2;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends group update on group TREE, to close first item. */
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        service.Clear();
        service.HasState = true;
        service.Action = MapEntryActions.UPDATE;
        service.ServiceId = 1;
        serviceGroup.Clear();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(DataStates.SUSPECT);
        serviceGroup.Status.StreamState(StreamStates.CLOSED);
        serviceGroup.Group = groupId3;
        service.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(service);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(2);

        /* Consumer receives StatusMsg closing the first item. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Directory update with the group status. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Provider sends the same group status again (group TREE). */
        service.Clear();
        service.HasState = true;
        service.Action = MapEntryActions.UPDATE;
        service.ServiceId = 1;
        serviceGroup.Clear();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(DataStates.SUSPECT);
        serviceGroup.Status.StreamState(StreamStates.CLOSED);
        serviceGroup.Group = groupId3;
        service.GroupStateList.Add(serviceGroup);
        directoryUpdateMsg.ServiceList.Add(service);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the directory update, but no StatusMsg. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ServiceDownThenUpDirectoryRequestTest()
    {
        /* Test directory request and get first response not found and then get directory update with service. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IUpdateMsg receivedUpdateMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        DirectoryRequest directoryRequest = new DirectoryRequest();
        directoryRequest.StreamId = 2;
        directoryRequest.Filter = 29;
        directoryRequest.HasServiceId = true;
        directoryRequest.ServiceId = 2;
        directoryRequest.Streaming = true;

        consumerRole.RdmDirectoryRequest = directoryRequest;
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Provider sends service update with original requested service of serviceId 2.*/
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        Provider.DefaultService2.Copy(wlService.RdmService);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives directory update for stream 2. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(2, receivedUpdateMsg.StreamId);
        Assert.True(receivedUpdateMsg.CheckHasMsgKey());
        Assert.Equal((int)DomainType.SOURCE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedUpdateMsg.ContainerType);
        Assert.Equal(1, receivedUpdateMsg.MsgKey.Filter); // make sure filter is 1 since only INFO and STATE filter received

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ServiceDownOpenItemRecoverTest()
    {
        /* Test a simple request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;


        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("IBM.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.ApplyRefreshComplete();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 7;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(2, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives 1 refreshes - for TRI.N. */
        consumerReactor.Dispatch(1);
        // first refresh
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends service update to bring service down.*/
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.State.HasStatus = true;
        wlService.RdmService.State.Action = FilterEntryActions.SET;
        wlService.RdmService.State.Status.DataState(DataStates.OK);
        wlService.RdmService.State.Status.StreamState(StreamStates.OPEN);
        wlService.RdmService.State.HasAcceptingRequests = true;
        wlService.RdmService.State.AcceptingRequests = 0;
        wlService.RdmService.State.ServiceStateVal = 1;
        wlService.RdmService.ServiceId = 1;

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update. Consumer receives 3 updates for stream id 5, 7 and 6 plus service update. */
        consumerReactor.Dispatch(4);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(7, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.Equal((int)DomainType.SOURCE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);

        // open 2 new items - one existing name and one new name 
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 8;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("IBM.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 9;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("MSI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // provider should receive no request since service is not accepting requests
        providerReactor.Dispatch(0);

        /* Provider sends service update to bring service back up.*/
        wlService.RdmService.State.AcceptingRequests = 1;
        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update. Consumer receives 2 open/suspect updates for stream ids 8 and 9, 6 open/ok updates for stream id 5, 6, 7, 8 and 9 plus 1 service update. */
        consumerReactor.Dispatch(8);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(8, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(9, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(7, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(8, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(9, receivedRefreshMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.Equal((int)DomainType.SOURCE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);

        // provider should now receive two requests for 2 new items submitted when service was down
        providerReactor.Dispatch(2);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(2, receivedRequestMsg.Priority.Count);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("MSI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemServiceUpdatedTest()
    {
        /* Test a simple request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        RDMDirectoryMsgEvent directoryMsgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends service update .*/
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.State.HasStatus = true;
        wlService.RdmService.State.Status.DataState(2);
        wlService.RdmService.State.Status.StreamState(12);
        wlService.RdmService.ServiceId = 1;

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(2);

        /* Consumer receives status with StreamState 12 and dataState 2. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(12, receivedStatusMsg.State.StreamState());
        Assert.Equal(2, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        DirectoryUpdate receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 0);

        /* Stream should be considered closed. */
        //Assert.Equal(0, consumerReactor.ComponentList[0].ReactorChannel.Watchlist.DirectoryHandler.Service(1).StreamList().Size());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemServiceUpDownMultipleItems()
    {
        /* Test a simple request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedIRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg receivedIUpdateMsg;
        IStatusMsg receivedIStatusMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedIRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedIRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedIRequestMsg.CheckStreaming());
        Assert.False(receivedIRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedIRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedIRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedIRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedIRequestMsg.DomainType);

        providerStreamId = receivedIRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId= groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("IBM.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedIRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedIRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedIRequestMsg.CheckStreaming());
        Assert.False(receivedIRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedIRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedIRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedIRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedIRequestMsg.DomainType);

        providerStreamId = receivedIRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.ApplyRefreshComplete();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends service update .*/
        DirectoryUpdate directoryIUpdateMsg = new DirectoryUpdate();
        directoryIUpdateMsg.Clear();
        directoryIUpdateMsg.StreamId = 2;
        directoryIUpdateMsg.HasFilter = true;
        directoryIUpdateMsg.Filter = Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.State.HasStatus = true;
        wlService.RdmService.State.Action = FilterEntryActions.SET;
        wlService.RdmService.State.Status.DataState(DataStates.SUSPECT);
        wlService.RdmService.State.Status.StreamState(StreamStates.CLOSED_RECOVER);
        wlService.RdmService.State.HasAcceptingRequests = true;
        wlService.RdmService.State.AcceptingRequests = 1;
        wlService.RdmService.State.ServiceStateVal = 0;
        wlService.RdmService.ServiceId = 1;

        directoryIUpdateMsg.ServiceList.Add(wlService.RdmService);

        Assert.True(provider.SubmitAndDispatch(directoryIUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives group status and item status for the items (the second pair of item status is from the recovery attempt). */
        consumerReactor.Dispatch(5);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedIStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedIStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedIStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedIStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedIStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedIStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedIStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(6, receivedIStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedIStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedIStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedIStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedIStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

        receivedIUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.True(receivedIUpdateMsg.CheckHasMsgKey());
        Assert.Equal((int)DomainType.SOURCE, receivedIUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedIUpdateMsg.ContainerType);

        // Assert.True(consumerReactor.ComponentList[0].ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.Service(1).StreamIdDlList.Count() == 0);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedIStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedIStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedIStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedIStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedIStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedIStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedIStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(6, receivedIStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedIStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedIStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedIStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedIStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by privateStreamOpenCallbackSubmitTest and privateStreamOpenCallbackSubmitReSubmitTest. */
    class SendItemsFromOpenCallbackConsumer : Consumer
    {
        bool _privateStream;

        public SendItemsFromOpenCallbackConsumer(TestReactor testReactor, bool privateStream) : base(testReactor)
        {

            _privateStream = privateStream;
        }

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;

            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                base.ReactorChannelEventCallback(evt);

                /* Consumer sends private stream request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                if (_privateStream)
                {
                    requestMsg.ApplyPrivateStream();
                }
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }
            else
            {
                return base.ReactorChannelEventCallback(evt);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void PrivateStreamOpenCallbackSubmitTest()
    {
        TestReactorEvent evt;
        ReactorChannelEvent chnlEvent;
        ReactorMsgEvent msgEvent;
        IStatusMsg receivedStatusMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SendItemsFromOpenCallbackConsumer(consumerReactor, true);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        provider.Bind(opts);

        // connect consumer
        consumerReactor.Connect(opts, consumer, provider.ServerPort);

        // Consumer receives CHANNEL_OPENED evt
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        chnlEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_OPENED, chnlEvent.EventType);

        // Consumer receives "Closed, Recoverable/Suspect" StatusMsg from request submitted in channel open callback
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(StreamStates.CLOSED_RECOVER, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void FanoutTest()
    {

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IGenericMsg genericMsg = (IGenericMsg)new Msg();
        IGenericMsg receivedGenericMsg;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        DirectoryRequest _directoryRequest = new DirectoryRequest();
        DirectoryRefresh directoryRefresh = new DirectoryRefresh();

        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;


        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer sends second request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives first refresh. */
        consumerReactor.Dispatch(2);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        /* Consumer receives second refresh on other stream. */
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends generic msg .*/
        genericMsg.Clear();
        genericMsg.MsgClass = MsgClasses.GENERIC;
        genericMsg.DomainType = (int)DomainType.MARKET_PRICE;
        genericMsg.StreamId = providerStreamId;
        genericMsg.ContainerType = DataTypes.NO_DATA;
        genericMsg.ApplyHasMsgKey();
        genericMsg.MsgKey.ApplyHasServiceId();
        genericMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        genericMsg.MsgKey.ApplyHasName();
        genericMsg.MsgKey.Name.Data("TRI.N");

        Assert.True(provider.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives generic msg. */
        consumerReactor.Dispatch(2);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);


        receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
        Assert.True(receivedGenericMsg.CheckHasMsgKey());
        Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedGenericMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedGenericMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedGenericMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedGenericMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consume receives second generic msg on other stream */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);


        receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
        Assert.True(receivedGenericMsg.CheckHasMsgKey());
        Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedGenericMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedGenericMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedGenericMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedGenericMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer sends second directory request */
        _directoryRequest.Clear();
        _directoryRequest.StreamId = 10;
        _directoryRequest.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;
        _directoryRequest.Streaming = true;
        submitOptions.Clear();
        Assert.True(consumer.Submit(_directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives second directory refresh on second stream */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        Assert.Equal(10, msgEvent.Msg.StreamId);


        /* Provider sends generic msg on source directory stream.*/
        genericMsg.Clear();
        genericMsg.MsgClass = MsgClasses.GENERIC;
        genericMsg.DomainType = (int)DomainType.SOURCE;
        genericMsg.StreamId = provider.DefaultSessionDirectoryStreamId;
        genericMsg.ContainerType = DataTypes.NO_DATA;
        genericMsg.ApplyHasMsgKey();
        genericMsg.MsgKey.ApplyHasServiceId();
        genericMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        genericMsg.MsgKey.ApplyHasName();
        genericMsg.MsgKey.Name.Data("More Source");

        Assert.True(provider.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives generic msg on source directory. */
        consumerReactor.Dispatch(2);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);


        receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
        Assert.True(receivedGenericMsg.CheckHasMsgKey());
        Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedGenericMsg.MsgKey.CheckHasName());
        Assert.Equal("More Source", receivedGenericMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SOURCE, receivedGenericMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedGenericMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        /* Consume receives second generic msg on other source directory stream */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);


        receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
        Assert.True(receivedGenericMsg.CheckHasMsgKey());
        Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedGenericMsg.MsgKey.CheckHasName());
        Assert.Equal("More Source", receivedGenericMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SOURCE, receivedGenericMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedGenericMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider sends generic msg on login stream */
        genericMsg.Clear();
        genericMsg.MsgClass = MsgClasses.GENERIC;
        genericMsg.DomainType = (int)DomainType.LOGIN;
        genericMsg.StreamId = provider.DefaultSessionLoginStreamId;
        genericMsg.ContainerType = DataTypes.NO_DATA;
        genericMsg.ApplyHasMsgKey();
        genericMsg.MsgKey.ApplyHasServiceId();
        genericMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        genericMsg.MsgKey.ApplyHasName();
        genericMsg.MsgKey.Name.Data("More User");

        Assert.True(provider.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives generic msg on login stream */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);


        receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
        Assert.True(receivedGenericMsg.CheckHasMsgKey());
        Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedGenericMsg.MsgKey.CheckHasName());
        Assert.Equal("More User", receivedGenericMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.LOGIN, receivedGenericMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedGenericMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemGroupUpdatedTest()
    {
        /* Test a simple request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        RDMDirectoryMsgEvent directoryMsgEvent;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends group update .*/
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.ServiceId = 1;
        ServiceGroup serviceGroup = new ServiceGroup();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(DataStates.SUSPECT);
        serviceGroup.Status.StreamState(StreamStates.OPEN);
        serviceGroup.Group = groupId;
        wlService.RdmService.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        updateMsg.Clear();

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(2);

        /* Consumer receives Open/Suspect item status. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        DirectoryUpdate receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Provider sends group update with mergeToGroup.*/
        directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.ServiceId = 1;
        serviceGroup = new ServiceGroup();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(DataStates.OK);
        serviceGroup.Status.StreamState(StreamStates.OPEN);
        serviceGroup.Group = groupId;
        Buffer groupToMerge = new Buffer();
        groupToMerge.Data("43211234");
        serviceGroup.HasMergedToGroup = true;
        serviceGroup.MergedToGroup = groupToMerge;
        wlService.RdmService.GroupStateList.Clear();
        wlService.RdmService.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        updateMsg = (IUpdateMsg)new Msg();
        updateMsg.Clear();

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(2);

        /* Consumer receives Open/Ok item status. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Provider sends group update on a group we don't have, shouldn't make any changes.*/
        directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.ServiceId = 1;
        serviceGroup = new ServiceGroup();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(4);
        serviceGroup.Status.StreamState(14);
        Buffer badBuffer = new Buffer();
        badBuffer.Data("This Isn't correct, clearly");
        serviceGroup.Group = badBuffer;
        wlService.RdmService.GroupStateList.Clear();
        wlService.RdmService.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        updateMsg = (IUpdateMsg)new Msg();
        updateMsg.Clear();

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Provider sends group update on a groupToMerge we don't have, shouldn't make any changes.*/
        directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.ServiceId = 1;
        serviceGroup = new ServiceGroup();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(5);
        serviceGroup.Status.StreamState(15);
        serviceGroup.Group = badBuffer;
        Buffer groupToMergeTwo = new Buffer();
        groupToMergeTwo.Data("12345");
        serviceGroup.HasMergedToGroup = true;
        serviceGroup.MergedToGroup = groupToMergeTwo;
        wlService.RdmService.GroupStateList.Clear();
        wlService.RdmService.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        updateMsg = (IUpdateMsg)new Msg();
        updateMsg.Clear();

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Provider sends group update to groupToMerge of the same group Id */
        directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.ServiceId = 1;
        serviceGroup = new ServiceGroup();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(DataStates.SUSPECT);
        serviceGroup.Status.StreamState(StreamStates.OPEN);
        serviceGroup.Group = groupToMerge;
        serviceGroup.HasMergedToGroup = true;
        serviceGroup.MergedToGroup = groupToMerge;
        wlService.RdmService.GroupStateList.Clear();
        wlService.RdmService.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        updateMsg = (IUpdateMsg)new Msg();
        updateMsg.Clear();

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(2);

        /* Consumer receives Open/Suspect item status. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Provider sends group update that also has status update. Group update takes precedence as it happens second */
        directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.State.Status.DataState(3);
        wlService.RdmService.State.Status.StreamState(13);
        wlService.RdmService.HasState = true;
        wlService.RdmService.State.HasStatus = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.ServiceId = 1;
        serviceGroup = new ServiceGroup();
        serviceGroup.HasStatus = true;
        serviceGroup.Status.DataState(4);
        serviceGroup.Status.StreamState(14);
        serviceGroup.Group = groupToMerge;
        wlService.RdmService.GroupStateList.Clear();
        wlService.RdmService.GroupStateList.Add(serviceGroup);

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        updateMsg = (IUpdateMsg)new Msg();
        updateMsg.Clear();

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update. */
        consumerReactor.Dispatch(2);

        /* Consumer receives status with StreamState 14 and dataState 4. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(13, receivedStatusMsg.State.StreamState());
        Assert.Equal(3, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);
        Assert.True(receivedUpdateMsg.ServiceList.Count == 1);
        Assert.True(receivedUpdateMsg.ServiceList[0].HasState);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasInfo);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasData);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLink);
        Assert.False(receivedUpdateMsg.ServiceList[0].HasLoad);
        Assert.True(receivedUpdateMsg.ServiceList[0].GroupStateList.Count == 1);

        /* Stream should be considered closed. */
        // Assert.True(consumerReactor.ComponentList[0].ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.Service(1).StreamIdDlList.Count() == 0);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void PrivateStreamOpenCallbackSubmitReSubmitTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SendItemsFromOpenCallbackConsumer(consumerReactor, true);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        opts.NumStatusEvents = 1; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);

        // resubmit private stream request message
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyPrivateStream();
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyPrivateStream();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckPrivateStream());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void PrivateStreamAggregationTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit private stream request message twice
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyPrivateStream();
        requestMsg.ApplyHasExtendedHdr();
        Buffer extendedHdrBuffer = new Buffer();
        extendedHdrBuffer.Data("EXTENDED HEADER");
        requestMsg.ExtendedHeader = extendedHdrBuffer;
        requestMsg.ContainerType = DataTypes.OPAQUE;
        Buffer encodeDataBodyBuffer = new Buffer();
        encodeDataBodyBuffer.Data("ENCODED DATA BODY");
        requestMsg.EncodedDataBody = encodeDataBodyBuffer;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        requestMsg.StreamId = 6;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives two requests since requests aren't aggregated. */
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckHasPriority());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr());
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType);
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(4, receivedRequestMsg.StreamId); // stream id should be different from first request
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckHasPriority());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr());
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType);
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void PrivateStreamNonPrivateStreamAggregationTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit non private stream request first then private stream request second
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyHasExtendedHdr();
        Buffer extendedHdrBuffer = new Buffer();
        extendedHdrBuffer.Data("EXTENDED HEADER");
        requestMsg.ExtendedHeader = extendedHdrBuffer;
        requestMsg.ContainerType = DataTypes.OPAQUE;
        Buffer encodeDataBodyBuffer = new Buffer();
        encodeDataBodyBuffer.Data("ENCODED DATA BODY");
        requestMsg.EncodedDataBody = encodeDataBodyBuffer;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        requestMsg.ApplyPrivateStream();
        requestMsg.StreamId = 6;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives two requests since requests aren't aggregated. */
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckHasPriority()); // non private stream request should have priority
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.False(receivedRequestMsg.CheckHasExtendedHdr()); // non private stream request should not have extended header
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType); // non private stream request should not have data body

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(4, receivedRequestMsg.StreamId); // stream id should be different from first request
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckHasPriority()); // private stream request should not have priority
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr()); // private stream request should have extended header
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType); // private stream request should have data body
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void MsgKeyInUpdatesTest()
    {

        /* Test that the requestMsg.MsgKeyInUpdates flag does not appear on the wire even if requested,
         * and that the WL adds keys to Refresh/Update/Status/Generic/AckMsgs if it is requested.
         * Tests with and without RequestMsg.ApplyMsgKeyInUpdates().
         * Tests requesting both by service name and service ID. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        IGenericMsg genericMsg = (IGenericMsg)new Msg();
        IGenericMsg receivedGenericMsg;
        IStatusMsg statusMsg = (IStatusMsg)new Msg();
        IStatusMsg receivedStatusMsg;
        IPostMsg postMsg = (IPostMsg)new Msg();
        IPostMsg receivedPostMsg;
        IAckMsg ackMsg = (IAckMsg)new Msg();
        IAckMsg receivedAckMsg;
        int providerStreamId;

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                bool applyMsgKeyInUpdates = (i == 0);
                bool requestByServiceName = (j == 0);

                /* Create reactors. */
                TestReactor consumerReactor = new TestReactor();
                TestReactor providerReactor = new TestReactor();

                /* Create consumer. */
                Consumer consumer = new Consumer(consumerReactor);
                ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
                consumerRole.InitDefaultRDMLoginRequest();
                consumerRole.InitDefaultRDMDirectoryRequest();
                consumerRole.ChannelEventCallback = consumer;
                consumerRole.LoginMsgCallback = consumer;
                consumerRole.DirectoryMsgCallback = consumer;
                consumerRole.DictionaryMsgCallback = consumer;
                consumerRole.DefaultMsgCallback = consumer;
                consumerRole.WatchlistOptions.EnableWatchlist = true;
                consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
                consumerRole.WatchlistOptions.RequestTimeout = 1000;
                consumerRole.WatchlistOptions.PostAckTimeout = 1000;

                /* Create provider. */
                Provider provider = new Provider(providerReactor);
                ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
                providerRole.ChannelEventCallback = provider;
                providerRole.LoginMsgCallback = provider;
                providerRole.DirectoryMsgCallback = provider;
                providerRole.DictionaryMsgCallback = provider;
                providerRole.DefaultMsgCallback = provider;

                /* Connect the consumer and provider. Setup login & directory streams automatically. */
                ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
                opts.SetupDefaultLoginStream = true;
                opts.SetupDefaultDirectoryStream = true;

                provider.Bind(opts);

                TestReactor.OpenSession(consumer, provider, opts);

                /* Consumer sends request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                if (applyMsgKeyInUpdates)
                    requestMsg.ApplyMsgKeyInUpdates();
                submitOptions.Clear();
                if (requestByServiceName)
                    submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                else
                {
                    requestMsg.MsgKey.ApplyHasServiceId();
                    requestMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
                }
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.False(receivedRequestMsg.CheckMsgKeyInUpdates());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

                providerStreamId = receivedRequestMsg.StreamId;

                /* Provider sends refresh, with no key .*/
                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
                refreshMsg.StreamId = providerStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyRefreshComplete();

                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives refresh, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                    Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
                    Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
                }
                else
                    Assert.False(receivedRefreshMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* Provider sends update, with no key. */
                updateMsg.Clear();
                updateMsg.MsgClass = MsgClasses.UPDATE;
                updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
                updateMsg.StreamId = providerStreamId;
                updateMsg.ContainerType = DataTypes.NO_DATA;

                Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives update, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

                receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;

                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedUpdateMsg.CheckHasMsgKey());
                    Assert.True(receivedUpdateMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedUpdateMsg.MsgKey.ServiceId);
                    Assert.True(receivedUpdateMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedUpdateMsg.MsgKey.Name.ToString());
                }
                else
                    Assert.False(receivedUpdateMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* Provider sends generic message, with no key.*/
                genericMsg.Clear();
                genericMsg.MsgClass = MsgClasses.GENERIC;
                genericMsg.DomainType = (int)DomainType.MARKET_PRICE;
                genericMsg.StreamId = providerStreamId;
                genericMsg.ContainerType = DataTypes.NO_DATA;
                Assert.True(provider.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives generic, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);

                receivedGenericMsg = (IGenericMsg)msgEvent.Msg;

                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedGenericMsg.CheckHasMsgKey());
                    Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedGenericMsg.MsgKey.ServiceId);
                    Assert.True(receivedGenericMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedGenericMsg.MsgKey.Name.ToString());
                }
                else
                    Assert.False(receivedGenericMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedGenericMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedGenericMsg.ContainerType);
                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* Consumer sends post (so it can receive AckMsg). */
                postMsg.Clear();
                postMsg.MsgClass = MsgClasses.POST;
                postMsg.StreamId = 5;
                postMsg.DomainType = (int)DomainType.MARKET_PRICE;
                postMsg.ContainerType = DataTypes.NO_DATA;
                postMsg.ApplyHasPostId();
                postMsg.ApplyAck();
                postMsg.PostId = 7;
                postMsg.ApplyPostComplete();
                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives post. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);

                receivedPostMsg = (IPostMsg)msgEvent.Msg;
                Assert.Equal(providerStreamId, receivedPostMsg.StreamId);
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
                Assert.True(receivedPostMsg.CheckAck());
                Assert.True(receivedPostMsg.CheckHasPostId());
                Assert.Equal(7, receivedPostMsg.PostId);
                Assert.True(receivedPostMsg.CheckPostComplete());

                /* Provider sends AckMsg for post, with no key. */
                ackMsg.MsgClass = MsgClasses.ACK;
                ackMsg.StreamId = providerStreamId;
                ackMsg.DomainType = (int)DomainType.MARKET_PRICE;
                ackMsg.AckId = 7;
                Assert.True(provider.SubmitAndDispatch((Msg)ackMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives AckMsg, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);

                receivedAckMsg = (IAckMsg)msgEvent.Msg;
                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedAckMsg.CheckHasMsgKey());
                    Assert.True(receivedAckMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedAckMsg.MsgKey.ServiceId);
                    Assert.True(receivedAckMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedAckMsg.MsgKey.Name.ToString());
                }
                else
                    Assert.False(receivedAckMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);

                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* -- Test internally-generated timeout nack -- */

                /* Consumer sends post. This will be nacked. */
                postMsg.Clear();
                postMsg.MsgClass = MsgClasses.POST;
                postMsg.StreamId = 5;
                postMsg.DomainType = (int)DomainType.MARKET_PRICE;
                postMsg.ContainerType = DataTypes.NO_DATA;
                postMsg.ApplyHasPostId();
                postMsg.ApplyAck();
                postMsg.PostId = 7;
                postMsg.ApplyPostComplete();
                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives post. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);

                receivedPostMsg = (IPostMsg)msgEvent.Msg;
                Assert.Equal(providerStreamId, receivedPostMsg.StreamId);
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
                Assert.True(receivedPostMsg.CheckAck());
                Assert.True(receivedPostMsg.CheckHasPostId());
                Assert.Equal(7, receivedPostMsg.PostId);
                Assert.True(receivedPostMsg.CheckPostComplete());

                /* Provider does not respond. */

                /* Consumer receives nack, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);

                receivedAckMsg = (IAckMsg)msgEvent.Msg;
                Assert.True(receivedAckMsg.CheckHasNakCode());
                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedAckMsg.CheckHasMsgKey());
                    Assert.True(receivedAckMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedAckMsg.MsgKey.ServiceId);
                    Assert.True(receivedAckMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedAckMsg.MsgKey.Name.ToString());
                }
                else
                    Assert.False(receivedAckMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);

                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* Provider sends status, with no key .*/
                statusMsg.Clear();
                statusMsg.MsgClass = MsgClasses.STATUS;
                statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
                statusMsg.StreamId = providerStreamId;
                statusMsg.ContainerType = DataTypes.NO_DATA;
                statusMsg.ApplyHasState();
                statusMsg.State.StreamState(StreamStates.OPEN);
                statusMsg.State.DataState(DataStates.SUSPECT);

                Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives status, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

                receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedStatusMsg.CheckHasMsgKey());
                    Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedStatusMsg.MsgKey.ServiceId);
                    Assert.True(receivedStatusMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedStatusMsg.MsgKey.Name.ToString());
                }
                else
                    Assert.False(receivedStatusMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
                Assert.True(statusMsg.CheckHasState());
                Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
                Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* -- Test internally-generated status msg. -- */

                /* Consumer sends request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                if (applyMsgKeyInUpdates)
                    requestMsg.ApplyMsgKeyInUpdates();
                submitOptions.Clear();
                if (requestByServiceName)
                    submitOptions.ServiceName = "UNKNOWN_SERVICE";
                else
                {
                    requestMsg.MsgKey.ApplyHasServiceId();
                    requestMsg.MsgKey.ServiceId = 1 + Provider.DefaultService.ServiceId;
                }
                Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives nothing. */
                providerReactor.Dispatch(0);

                /* Consumer receives status, with key present if requested. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

                receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedStatusMsg.CheckHasMsgKey());
                    Assert.Equal("TRI.N", receivedStatusMsg.MsgKey.Name.ToString());

                    if (requestByServiceName)
                    {
                        Assert.False(receivedStatusMsg.MsgKey.CheckHasServiceId()); /* Request used name of a nonexistent service, so WL doesn't have an ID to add. */
                        Assert.True(receivedStatusMsg.MsgKey.CheckHasName());

                    }
                    else
                    {
                        Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
                        Assert.Equal(1 + Provider.DefaultService.ServiceId, receivedStatusMsg.MsgKey.ServiceId);
                    }
                }
                else
                    Assert.False(receivedStatusMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
                Assert.True(statusMsg.CheckHasState());
                Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
                Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal("UNKNOWN_SERVICE", msgEvent.StreamInfo.ServiceName);
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);

                /* -- Test internally-generated request-timeout StatusMsg. -- */

                /* Consumer sends request for a new item. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 7;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("IBM.N");
                if (applyMsgKeyInUpdates)
                    requestMsg.ApplyMsgKeyInUpdates();
                submitOptions.Clear();
                if (requestByServiceName)
                    submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                else
                {
                    requestMsg.MsgKey.ApplyHasServiceId();
                    requestMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
                }
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.False(receivedRequestMsg.CheckMsgKeyInUpdates());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

                /* Provider sends no response. */

                /* Consumer receives status, with key present if requested. */
                /* Waits for 2 seconds in order to get the request timeout status message.*/
                consumerReactor.Dispatch(1, new TimeSpan(0,0,2));
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

                receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
                if (applyMsgKeyInUpdates)
                {
                    Assert.True(receivedStatusMsg.CheckHasMsgKey());
                    Assert.True(receivedStatusMsg.MsgKey.CheckHasName());
                    Assert.Equal("IBM.N", receivedStatusMsg.MsgKey.Name.ToString());
                    Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedStatusMsg.MsgKey.ServiceId);
                }
                else
                    Assert.False(receivedStatusMsg.CheckHasMsgKey());

                Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
                Assert.True(statusMsg.CheckHasState());
                Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
                Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                if (requestByServiceName)
                {
                    Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                    Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                }
                else
                    Assert.Null(msgEvent.StreamInfo.ServiceName);


                /* Provider receives close & re-request again. */
                providerReactor.Dispatch(2);

                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);

                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.False(receivedRequestMsg.CheckMsgKeyInUpdates());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

                TestReactorComponent.CloseSession(consumer, provider);
                TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
            }
        }
    }

    [Fact]
    public void DualRequestWithDifferentServiceTest()
    {
        /* Test sending two requests with different services and watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.SetupSecondDefaultDirectoryStream = true;

        provider.Bind(opts);
        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends first request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer sends second request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService2.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives 2 requests. */
        providerReactor.Dispatch(2);

        /* Provider receives first request. */
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends first refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives second request. */
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService2.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends second refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService2.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives two refreshes for TRI.N
        consumerReactor.Dispatch(2);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received first refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received second refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    internal class TestData
    {
        public TestData(int idStream, int idService, String nameService)
        {
            StreamId = idStream;
            ServiceId = idService;
            ServiceName = nameService;
        }
        public int StreamId { get; set; }
        public int ServiceId { get; set; }
        public String ServiceName { get; set; }
    }

    [Fact]
    public void CloseWhileDisconnectedTest()
    {
        foreach (bool useServiceNames in new bool[] { true, false })
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;
            ICloseMsg closeMsg = (ICloseMsg)msg;

            /* Create reactors. */
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            /* Create consumer. */
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;

            /* Create provider. */
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);


            List<TestData> requestData = new List<TestData>();
            requestData.Add(new TestData(5, 1, Provider.DefaultService.Info.ServiceName.ToString()));
            requestData.Add(new TestData(6, 2, "DIRECT_FEED2"));
            requestData.Add(new TestData(7, 3, "DIRECT_FEED3"));

            // send consumer requests
            foreach (TestData td in requestData)
            {
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = td.StreamId;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");

                submitOptions.Clear();
                if (useServiceNames)
                    submitOptions.ServiceName = td.ServiceName;
                else
                {
                    requestMsg.MsgKey.ApplyHasServiceId();
                    requestMsg.MsgKey.ServiceId = td.ServiceId;
                }
                Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            /* Consumer receives status. */
            consumerReactor.Dispatch(2);
            consumerReactor.PollEvent();
            consumerReactor.PollEvent();

            /* Provider receives the request for the known service. */
            providerReactor.Dispatch(1);
            providerReactor.PollEvent();

            /* Provider disconnects. */
            provider.CloseChannel();

            // Consumer receives four evts
            consumerReactor.Dispatch(4);
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, consumerReactor.PollEvent().EventType);
            Assert.Equal(TestReactorEventType.LOGIN_MSG, consumerReactor.PollEvent().EventType);
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, consumerReactor.PollEvent().EventType);
            Assert.Equal(TestReactorEventType.MSG, consumerReactor.PollEvent().EventType);


            /* Consumer closes requests. */
            foreach (TestData td in requestData)
            {
                closeMsg.Clear();
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = td.StreamId;
                closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
                closeMsg.ContainerType = DataTypes.NO_DATA;
                submitOptions.Clear();
                submitOptions.ServiceName = td.ServiceName;
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
        }
    }

    [Fact]
    public void PrivateStreamSubmitReissueTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit private stream request message
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyPrivateStream();
        requestMsg.ApplyHasExtendedHdr();
        Buffer extendedHdrBuffer = new Buffer();
        extendedHdrBuffer.Data("EXTENDED HEADER");
        requestMsg.ExtendedHeader = extendedHdrBuffer;
        requestMsg.ContainerType = DataTypes.OPAQUE;
        Buffer encodeDataBodyBuffer = new Buffer();
        encodeDataBodyBuffer.Data("ENCODED DATA BODY");
        requestMsg.EncodedDataBody = encodeDataBodyBuffer;
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.Count = 11;
        requestMsg.Priority.PriorityClass = 22;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr());
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType);
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(11, receivedRequestMsg.Priority.Count);
        Assert.Equal(22, receivedRequestMsg.Priority.PriorityClass);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyPrivateStream();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckPrivateStream());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // consumer reissues original request with different priority
        requestMsg.Priority.Count = 5;
        requestMsg.Priority.PriorityClass = 6;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr());
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType);
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(5, receivedRequestMsg.Priority.Count);
        Assert.Equal(6, receivedRequestMsg.Priority.PriorityClass);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyPrivateStream();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckPrivateStream());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    private void StreamingAggregation(bool dispatchBetweenItemRequests)
    {
        /* Test aggregation of two streaming items. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends streaming request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        if (dispatchBetweenItemRequests)
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        else
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer sends streaming request for same item . */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        providerReactor.Dispatch(2);

        /* Provider receives request. */
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider receives priority change due to second request. */
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(2, receivedRequestMsg.Priority.Count);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives two refreshes. */
        consumerReactor.Dispatch(2);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the update on both streams. */
        consumerReactor.Dispatch(2);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(6, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void StreamingAggregationTest()
    {
        StreamingAggregation(false);
    }

    [Fact]
    public void StreamingAggregation_DispatchBetweenItemRequestsTest()
    {
        StreamingAggregation(true);
    }

    /* Used by snapshotStreamingBeforeChannelReadyTest Test. */
    internal class SnapshotStreamingBeforeChannelReady : Consumer
    {
        public SnapshotStreamingBeforeChannelReady(TestReactor testReactor) : base(testReactor)
        {
        }

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;
            int testUserSpecObj = 997;

            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                base.ReactorChannelEventCallback(evt);

                /* Consumer sends snapshot request (does not apply streaming flag). */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer sends streaming request for same request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }
            else
            {
                return base.ReactorChannelEventCallback(evt);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    /* Used by streamingSnapshotBeforeChannelReadyTest Test. */
    internal class StreamingSnapshotBeforeChannelReady : Consumer
    {
        public StreamingSnapshotBeforeChannelReady(TestReactor testReactor) : base(testReactor)
        {
        }
        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;
            int testUserSpecObj = 997;

            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                base.ReactorChannelEventCallback(evt);

                /* Consumer sends streaming request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer sends snapshot request for same item (does not apply streaming flag). */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }
            else
            {
                return base.ReactorChannelEventCallback(evt);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void StreamingSnapshotBeforeChannelReadyTest()
    {
        /* Test aggregation of streaming then Snapshot requests before channel ready */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new StreamingSnapshotBeforeChannelReady(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;


        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        opts.NumStatusEvents = 2; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);


        /* Provider receives request. */
        providerReactor.Dispatch(1);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refreshes, one with state OPEN on stream 5, one NON-STREAMING on 6 */
        consumerReactor.Dispatch(2);

        /* Streaming refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the update, only on stream 5. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SnapshotStreamingBeforeChannelReadyTest()
    {
        /* Test aggregation of streaming then Snapshot requests before channel ready */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SnapshotStreamingBeforeChannelReady(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;


        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);
        opts.NumStatusEvents = 2; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);


        /* Provider receives request. */
        providerReactor.Dispatch(1);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refreshes, one with state OPEN on stream 5, one NON-STREAMING on 6 */
        consumerReactor.Dispatch(2);

        /* Streaming refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the update, only on stream 6. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(6, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SnapshotAggregationOnClosedStreamTest()
    {

        /* Test aggregation of a snapshot request, using a previously closed stream. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;


        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends snapshot request for TRI1, TRI2, TRI3 */

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI1");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI2");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 7;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI3");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(3);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI1", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI2", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI3", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI1");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh */
        consumerReactor.Dispatch(1);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI1", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Consumer sends TRI2 snapshot reusing TRI1 stream */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI2");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives nothing, because of aggregation. */
        providerReactor.Dispatch(0);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = 4;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI2");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh */
        consumerReactor.Dispatch(2);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI2", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI2", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = 5;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI3");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh */
        consumerReactor.Dispatch(1);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(7, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI3", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SnapshotBeforeStreamingRequest()
    {

        /* Test aggregation of a snapshot request onto a streaming request. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;


        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends snapshot request */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh, NON-STREAMING on 5. */
        consumerReactor.Dispatch(1);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer sends streaming request for same item */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh, STREAMING on 6. */
        consumerReactor.Dispatch(1);

        /* Streaming refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives the update, only on stream 6. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(6, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void PrivateStreamRecoveryTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
        IStatusMsg receivedStatusMsg;
        ReactorChannelEvent chnlEvent;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = 1;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit private stream request message
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyPrivateStream();
        requestMsg.ApplyHasExtendedHdr();
        Buffer extendedHdrBuffer = new Buffer();
        extendedHdrBuffer.Data("EXTENDED HEADER");
        requestMsg.ExtendedHeader = extendedHdrBuffer;
        requestMsg.ContainerType = DataTypes.OPAQUE;
        Buffer encodeDataBodyBuffer = new Buffer();
        encodeDataBodyBuffer.Data("ENCODED DATA BODY");
        requestMsg.EncodedDataBody = encodeDataBodyBuffer;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckPrivateStream());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasExtendedHdr());
        Assert.Equal("EXTENDED HEADER", receivedRequestMsg.ExtendedHeader.ToString());
        Assert.Equal(DataTypes.OPAQUE, receivedRequestMsg.ContainerType);
        Assert.Equal("ENCODED DATA BODY", receivedRequestMsg.EncodedDataBody.ToString());

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyPrivateStream();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckPrivateStream());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // force recovery
        provider.CloseChannel();

        // dispatch all evts (login, directory, directory, msg, channel)
        consumer.TestReactor.Dispatch(4);

        // ignore first 3 evts which are login/directory recovery messages and channel down 
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        chnlEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, chnlEvent.EventType);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal((int)DomainType.LOGIN, msgEvent.Msg.DomainType);
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal((int)DomainType.SOURCE, msgEvent.Msg.DomainType);
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

        // Consumer receives "Closed, Recoverable/Suspect" StatusMsg and CHANNEL_DOWN_RECONNECTING evt 
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(StreamStates.CLOSED_RECOVER, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SingleOpenZeroOpenCallbackSubmitRecoverTest()
    {
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer with SingleOpen and AllowSuspect of 0. */
        Consumer consumer = new SendItemsFromOpenCallbackConsumer(consumerReactor, false);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.RdmLoginRequest.HasAttrib = true;
        consumerRole.RdmLoginRequest.LoginAttrib.HasSingleOpen = true;
        consumerRole.RdmLoginRequest.LoginAttrib.SingleOpen = 0;
        consumerRole.RdmLoginRequest.LoginAttrib.HasAllowSuspectData = true;
        consumerRole.RdmLoginRequest.LoginAttrib.AllowSuspectData = 0;
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        opts.NumStatusEvents = 1; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);

        /* Provider should never receive request since no single open. */
        providerReactor.Dispatch(0);
        //evt = providerReactor.PollEvent();
        //Assert.Null(evt);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemDoubleCloseTest()
    {

        /* Test a simple request/refresh exchange with the watchlist enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        ICloseMsg closeMsg = (ICloseMsg)msg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer sends close. */
        closeMsg.Clear();
        closeMsg.MsgClass = MsgClasses.CLOSE;
        closeMsg.StreamId = 5;
        closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer sends close again. */
        closeMsg.Clear();
        closeMsg.MsgClass = MsgClasses.CLOSE;
        closeMsg.StreamId = 5;
        closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by submitOffstreamPostOnItemRefeshTest. */
    internal class PriorityChangeFromCallbackConsumer : Consumer
    {
        public PriorityChangeFromCallbackConsumer(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            base.DefaultMsgCallback(evt);

            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyNoRefresh();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasPriority();
            requestMsg.Priority.PriorityClass = 1;
            requestMsg.Priority.Count = 2;
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void PriorityChangeInAndOutOfCallbackTest()
    {

        /* Simple test of changing priority both inside and outside a callback.
         * Reproduced ETA-2144 (a NullPointerException when changing priority) */
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new PriorityChangeFromCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider receives priority change. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckNoRefresh());

        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(2, receivedRequestMsg.Priority.Count);

        /* Consumer sends priority change, not in callback. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyNoRefresh();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 3;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives priority change. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(3, receivedRequestMsg.Priority.Count);

        consumerReactor.Dispatch(0);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by snapshotAggregationBeforeChannelReadyTestd Test. */
    internal class SendMultipleSnapshotsBeforeChannelReady : Consumer
    {
        public SendMultipleSnapshotsBeforeChannelReady(TestReactor testReactor) : base(testReactor)
        {}

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;

            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                base.ReactorChannelEventCallback(evt);

                /* Consumer sends first snapshot request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer sends second aggregated snapshot request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }
            else
            {
                return base.ReactorChannelEventCallback(evt);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void SnapshotAggregationBeforeChannelReady()
    {
        /* Test aggregation of a snapshot request onto a streaming request. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SendMultipleSnapshotsBeforeChannelReady(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        opts.NumStatusEvents = 2; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);

        /* Provider receives request. */
        providerReactor.Dispatch(1);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refreshes, one with state OPEN on stream 5, and one NON-STREAMING on 6. */
        consumerReactor.Dispatch(2);

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Snapshot refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by itemCloseAndReopenTest.
    * Closes TRI on stream 5 and reopens it. */
    class ItemCloseAndReopenConsumer : Consumer
    {
        public ItemCloseAndReopenConsumer(TestReactor testReactor) : base(testReactor)
        { }
        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            ICloseMsg closeMsg = (ICloseMsg)new Msg();
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            base.DefaultMsgCallback(evt);

            if (evt.Msg.MsgClass == MsgClasses.UPDATE)
            {
                closeMsg.Clear();
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = 5;
                closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
                submitOptions.Clear();
                Assert.True(Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void ItemCloseAndReopenTest()
    {
        /* Test closing and reopening an item inside and outside the msg callback 
         * (the former reproduced ETA-2163). */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        ICloseMsg closeMsg = (ICloseMsg)new Msg();
        ICloseMsg receivedCloseMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new ItemCloseAndReopenConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        providerStreamId = receivedRequestMsg.StreamId;

        for (int i = 0; i < 3; ++i)
        {
            /* Provider sends an update. */
            updateMsg.Clear();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer does not receive it (no refresh received yet). */
            consumerReactor.Dispatch(0);

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.ApplySolicited();
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.ApplyRefreshComplete();
            Buffer groupId = new Buffer();
            groupId.Data("5555");
            refreshMsg.GroupId = groupId;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Provider sends an update. */
            updateMsg.Clear();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives update (closes/reopens request in callback) */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
            receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
            Assert.Equal(5, receivedUpdateMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Provider receives close and re-request. */
            providerReactor.Dispatch(2);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            providerStreamId = receivedRequestMsg.StreamId;

        }

        for (int i = 0; i < 3; ++i)
        {
            /* Provider sends an update. */
            updateMsg.Clear();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer does not receive it. */
            consumerReactor.Dispatch(0);

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.ApplySolicited();
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.ApplyRefreshComplete();
            Buffer groupId = new Buffer();
            groupId.Data("5555");
            refreshMsg.GroupId = groupId;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Consumer sends close. */
            closeMsg.Clear();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = 5;
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);

            receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);

            /* Consumer sends request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);

            providerStreamId = receivedRequestMsg.StreamId;

        }

        consumerReactor.Dispatch(0);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by CloseFromCallbackTest. */
    class CloseFromCallbackConsumer : Consumer
    {
        public CloseFromCallbackConsumer(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            ICloseMsg closeMsg = (ICloseMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            base.DefaultMsgCallback(evt);

            if (evt.Msg.MsgClass == MsgClasses.UPDATE || evt.Msg.MsgClass == MsgClasses.STATUS)
            {
                closeMsg.Clear();
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = 5;
                closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
                submitOptions.Clear();
                Assert.True(Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                closeMsg.Clear();
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = 6;
                closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
                submitOptions.Clear();
                Assert.True(Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void ItemCloseFromCallbackTest()
    {

        /* Opening two streams for an item and closing both within the callback. 
         * Tested in response to an update message as well as a group status. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IUpdateMsg receivedUpdateMsg;
        ICloseMsg receivedCloseMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new CloseFromCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Test consumer closes in response to an update. */
        for (int i = 0; i < 3; ++i)
        {
            /* Consumer sends request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Consumer sends request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 6;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.ApplySolicited();
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.ApplyRefreshComplete();
            Buffer groupId = new Buffer();
            groupId.Data("1234431");
            refreshMsg.GroupId = groupId;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(2);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(5, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(6, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Provider sends an update. */
            updateMsg.Clear();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives update (closes on first update fanout, so only receives one). */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
            receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
            Assert.Equal(5, receivedUpdateMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());


            /* Provider receives reissue and close. */
            providerReactor.Dispatch(2);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            providerStreamId = receivedRequestMsg.StreamId;

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);
        }

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemCloseFromCallbackTestOpen()
    {
        /* Opening two streams for an item and closing both within the callback.
         * Tested in response to an update message as well as a group status. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        IStatusMsg receivedStatusMsg;
        ICloseMsg receivedCloseMsg;
        RDMDirectoryMsgEvent directoryMsgEvent;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new CloseFromCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Test consumer closes in response to group status. */
        for (int i = 0; i < 2; ++i)
        {
            /* Test closing in response to open & closed group status messages. */
            int msgStreamState = (i == 0) ? StreamStates.OPEN : StreamStates.CLOSED;

            for (int j = 0; j < 3; ++j)
            {
                /* Consumer sends request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(1, receivedRequestMsg.Priority.Count);
                providerStreamId = receivedRequestMsg.StreamId;

                /* Consumer sends request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(2, receivedRequestMsg.Priority.Count);
                providerStreamId = receivedRequestMsg.StreamId;

                /* Provider sends refresh .*/
                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.ApplySolicited();
                refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
                refreshMsg.StreamId = providerStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplyHasMsgKey();
                refreshMsg.MsgKey.ApplyHasServiceId();
                refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
                refreshMsg.MsgKey.ApplyHasName();
                refreshMsg.MsgKey.Name.Data("TRI.N");
                refreshMsg.ApplyRefreshComplete();
                Buffer groupId = new Buffer();
                groupId.Data("5555");
                refreshMsg.GroupId = groupId;
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);

                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives refresh. */
                consumerReactor.Dispatch(2);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(5, receivedRefreshMsg.StreamId);
                Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(6, receivedRefreshMsg.StreamId);
                Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

                /* Provider sends group status .*/
                ServiceGroup serviceGroup = new ServiceGroup();
                serviceGroup.Clear();
                serviceGroup.Group.Data("5555");
                serviceGroup.HasMergedToGroup = true;
                serviceGroup.MergedToGroup.Data("7777");
                serviceGroup.HasStatus = true;
                serviceGroup.Status.StreamState(msgStreamState);
                serviceGroup.Status.DataState(DataStates.SUSPECT);

                Service service = new Service();
                service.Clear();
                service.ServiceId = Provider.DefaultService.ServiceId;
                service.Action = MapEntryActions.UPDATE;
                service.GroupStateList.Add(serviceGroup);

                DirectoryUpdate directoryUpdate = new DirectoryUpdate();

                directoryUpdate.Clear();
                directoryUpdate.StreamId = provider.DefaultSessionDirectoryStreamId;
                directoryUpdate.ServiceList.Add(service);
                Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives directory update,
                 * and open/suspect status (closes on first update fanout, so only receives one). */
                consumerReactor.Dispatch(2);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
                receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
                Assert.Equal(5, receivedStatusMsg.StreamId);
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
                Assert.Equal(msgStreamState, receivedStatusMsg.State.StreamState());
                Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
                Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

                if (msgStreamState == StreamStates.OPEN)
                {
                    /* Provider receives reissue and close. */
                    providerReactor.Dispatch(2);

                    evt = providerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                    receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                    Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                    Assert.True(receivedRequestMsg.CheckStreaming());
                    Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                    Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                    Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
                    Assert.True(receivedRequestMsg.CheckHasPriority());
                    Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                    Assert.Equal(1, receivedRequestMsg.Priority.Count);
                    providerStreamId = receivedRequestMsg.StreamId;

                    evt = providerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
                    receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
                    Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);
                }
                else
                {
                    /* Provider receives nothing (stream is already closed). */
                    providerReactor.Dispatch(0);
                }
            }
        }

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by closeFromCallbackTest. */
    internal class CloseLoginStreamFromCallbackConsumer : Consumer
    {
        public CloseLoginStreamFromCallbackConsumer(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;

            base.ReactorChannelEventCallback(evt);

            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                /* Consumer sends item request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public override ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
        {
            ICloseMsg closeMsg = (ICloseMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

            base.RdmLoginMsgCallback(evt);

            Assert.Equal(LoginMsgType.STATUS, evt.LoginMsg.LoginMsgType);
            closeMsg.Clear();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = 1;
            closeMsg.DomainType = (int)DomainType.LOGIN;
            submitOptions.Clear();
            Assert.True(Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void LoginCloseFromCallbackTest()
    {

        for (int i = 0; i < 2; ++i)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            RDMLoginMsgEvent loginMsgEvent;
            LoginStatus receivedLoginStatus;
            int provLoginStreamId;

            /* Create reactors. */
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            /* Create consumer. */
            Consumer consumer = new CloseLoginStreamFromCallbackConsumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;

            /* Create provider. */
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. */
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;
            opts.NumStatusEvents = 1;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Provider receives login request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            LoginRequest receivedLoginRequest = (LoginRequest)loginMsgEvent.LoginMsg.LoginRequest;
            provLoginStreamId = receivedLoginRequest.StreamId;

            /* Provider sends login refresh. */
            LoginStatus loginStatus = new LoginStatus();

            loginStatus.Clear();
            loginStatus.HasState = true;
            loginStatus.StreamId = provLoginStreamId;

            if (i == 0)
                loginStatus.State.StreamState(StreamStates.OPEN);
            else
                loginStatus.State.StreamState(StreamStates.CLOSED);

            loginStatus.State.DataState(DataStates.SUSPECT);
            loginStatus.State.Code(StateCodes.NOT_ENTITLED);
            loginStatus.State.Text().Data("Not permissioned.");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives login status. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);
            receivedLoginStatus = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.Equal(consumerRole.RdmLoginRequest.StreamId, receivedLoginStatus.StreamId);
            Assert.True(receivedLoginStatus.HasState);

            if (i == 0)
                Assert.Equal(StreamStates.OPEN, receivedLoginStatus.State.StreamState());
            else
                Assert.Equal(StreamStates.CLOSED, receivedLoginStatus.State.StreamState());

            Assert.Equal(DataStates.SUSPECT, receivedLoginStatus.State.DataState());

            /* Provider receives login close if stream was was open. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.CLOSE, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(provLoginStreamId, loginMsgEvent.LoginMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
        }
    }
    
     class SnapshotStreamingViewMixAggregationBeforeChannelReady : Consumer
    {
       public SnapshotStreamingViewMixAggregationBeforeChannelReady(TestReactor testReactor)
            : base(testReactor)
       {
       }

       public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
       {
           ReactorSubmitOptions submitOptions = new();
           Msg msg = new ();
           IRequestMsg requestMsg = msg;
           List<int> viewFieldList = new();
           string testUserSpecObj = "997";
           
           if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
           {
               base.ReactorChannelEventCallback(evt);
               
               /* Consumer sends streaming request. */
               requestMsg.Clear();
               requestMsg.MsgClass = MsgClasses.REQUEST;
               requestMsg.StreamId = 5;
               requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
               requestMsg.ApplyStreaming();
               requestMsg.MsgKey.ApplyHasName();
               requestMsg.MsgKey.Name.Data("TRI.N");
               submitOptions.Clear();
               submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
               submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
               Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

               /* Consumer sends snapshot request for same item (does not apply streaming flag). */
               requestMsg.Clear();
               requestMsg.MsgClass = MsgClasses.REQUEST;
               requestMsg.StreamId = 6;
               requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
               requestMsg.MsgKey.ApplyHasName();
               requestMsg.MsgKey.Name.Data("TRI.N");
               submitOptions.Clear();
               submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
               submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
               Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
               
               /* Consumer sends streaming request for same item with view. */
               
               requestMsg.Clear();
               requestMsg.MsgClass = MsgClasses.REQUEST;
               requestMsg.StreamId = 7;
               requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
               requestMsg.ApplyStreaming();
               requestMsg.ApplyHasView();
               requestMsg.MsgKey.ApplyHasName();
               requestMsg.MsgKey.Name.Data("TRI.N");
               viewFieldList.Add(22);
               viewFieldList.Add(22);
               viewFieldList.Add(6);
               viewFieldList.Add(0);
               viewFieldList.Add(130);
               viewFieldList.Add(1131);
               viewFieldList.Add(1025);                
               EncodeViewFieldIdList(ReactorChannel, viewFieldList, requestMsg);

               submitOptions.Clear();
               submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
               submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
               Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

               /* Consumer sends snapshot request with view for same item (does not apply streaming flag). */
               requestMsg.Clear();
               requestMsg.MsgClass = MsgClasses.REQUEST;
               requestMsg.StreamId = 8;
               requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
               requestMsg.ApplyHasView();
               requestMsg.MsgKey.ApplyHasName();
               requestMsg.MsgKey.Name.Data("TRI.N");
               viewFieldList.Add(22);
               viewFieldList.Add(22);
               viewFieldList.Add(6);
               viewFieldList.Add(0);
               viewFieldList.Add(130);
               viewFieldList.Add(1131);
               viewFieldList.Add(1025);                
               EncodeViewFieldIdList(ReactorChannel, viewFieldList, requestMsg);

               submitOptions.Clear();
               submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
               submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
               Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
           }
           else
           {
               return base.ReactorChannelEventCallback(evt);
           }

            return ReactorCallbackReturnCode.SUCCESS;
       }
   }

    [Fact]
    public void SnapshotStreamingViewMixAggregationBeforeChannelReadyTest()
    {
        /* Test aggregation of 4 requests, Snapshot, Snapshot-View, Streaming, and Streaming-View */
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = new Msg();
        IUpdateMsg receivedUpdateMsg;
        int providerStreamId;
        string testUserSpecObj = "997";
        
        /* Create reactors. */
        TestReactor.EnableReactorXmlTracing = true;
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new SnapshotStreamingViewMixAggregationBeforeChannelReady(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        
        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);
        opts.NumStatusEvents = 4; // set number of expected status message from request submitted in channel open callback
        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refreshes, one with state OPEN on stream 5, one NON-STREAMING on 6, one OPEN on stream 7, and NON-STREAMING on stream 8. */
        consumerReactor.Dispatch(4);
        
        /* Streaming refresh. */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Snapshot refresh. */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);
        
        /* Streaming refresh. */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(7, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Snapshot refresh. */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(8, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);
        
        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives the update, fans out to 2 streams */
        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(7, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ChangeViewByReissueRequestWhilePendingRefreshTest()
    {
        /* Test changing a view on an item by another request of same user stream while waiting for that item's refresh. */
        /* steps:
         * request an item request "TRI" on stream 5 by one user
         * receive refresh on "TRI"
         * reissue item request "TRI" with View (22,25,-32768,32767) on stream 5
         * reissue item request "TRI" with View (25, 1025) on stream 5
         * receive refresh on "TRI"
         * no refresh fanout to consumer
         */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;

        int providerStreamId;
        List<int> viewFieldList = new();
        
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        
        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Request TRI (no view). */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());   
        
        /* Reissue TRI with BID/ASK view. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasView();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI");
        viewFieldList.Add(22);
        viewFieldList.Add(25);
        viewFieldList.Add(-32768);
        viewFieldList.Add(32767);        
        EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        providerReactor.Dispatch(1);
        
        /* Provider receives reissued request. */
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckHasView());
        Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList));
        
        /* Reissue TRI again, now with ASK/QUOTIM. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasView();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI");
        viewFieldList.Add(25);
        viewFieldList.Add(1025);
        EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives nothing (still waiting on refresh for original BID/ASK view). */
        providerReactor.Dispatch(0);
        
        /* Provider sends refresh (this would be for the original view). */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());   
        
        /* Provider receives updated view request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckHasView()); 
        Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList)); 

        /* Provider sends refresh */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ChangeViewByPartialReissueRequestWhilePendingRefreshTest()
    {
         /* Test changing a view on an item by another request of diff user stream while waiting for that item's refresh. */
        /* steps:
         * request an item request "TRI" on stream 5 by one user
         * receive refresh on "TRI"
         * reissue item request "TRI" with View (22,25) on stream 5
         * request third item request "TRI" with View (25, 1025) on stream 6 by second user
         * receive refresh from provider
         * will fanout to two users
         */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;

        int providerStreamId;
        List<int> viewFieldList = new();
        
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Request TRI (no view). */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        providerStreamId = receivedRequestMsg.StreamId;


        /* Provider sends refresh */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());   

        
        /* Reissue TRI with BID/ASK view. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasView();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI");
        viewFieldList.Add(22);
        viewFieldList.Add(25);
        EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        providerReactor.Dispatch(1);
        
        /* Provider receives reissued request. */
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckHasView()); 
        Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList)); 
        
        /* Request TRI again (not reissue), now with ASK/QUOTIM. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6; //from diff user
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasView();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI");
        viewFieldList.Add(25);
        viewFieldList.Add(1025);
        EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives nothing (still waiting on refresh for original BID/ASK view). */
        providerReactor.Dispatch(0);
        
        /* Provider sends refresh (this would be for the original view). */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider receives updated view request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckHasView()); 
        Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList)); 

        /* Provider sends refresh */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives one refresh, will fan out diff user (streamid 5 and 6) . */
        consumerReactor.Dispatch(2);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());        

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ViewAggregateAndReconnectTest()
    {
         /* Test recovering an aggregated view.
         * - Send a request with a view.
         * - Send a request for the same item, with a different view. This request will be waiting for the first to get its refresh.
         * - In one case (i == 0 below), refresh the items. In the other case (i == 1), don't refresh them.
         * - Disconnect the provider. Both view requests should get the status indicating recovery.
         * - Reconnect. Provider should get a request for the aggregate view. */

        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;

        int providerStreamId;
        List<int> viewFieldList = new();

        for (int i = 0; i < 2; ++i)
        {
            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = -1;
            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            /* Request TRI with BID/ASK view. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasView();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI");
            viewFieldList.Add(22);
            viewFieldList.Add(25);
            EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            testEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasView()); 
            Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList));
            providerStreamId = receivedRequestMsg.StreamId;
            
            /* Request TRI on another stream with a different view. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 6;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasView();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI");
            viewFieldList.Clear();
            viewFieldList.Add(25);
            viewFieldList.Add(1025);
            EncodeViewFieldIdList(consumer.ReactorChannel, viewFieldList, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives nothing (still waiting on refresh for original BID/ASK view). */
            providerReactor.Dispatch(0);

            if (i == 0) /* If i is 0, refresh the items. */
            {
                /* Provider sends refresh. */
                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
                refreshMsg.StreamId = providerStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplyHasMsgKey();
                refreshMsg.MsgKey.ApplyHasServiceId();
                refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
                refreshMsg.MsgKey.ApplyHasName();
                refreshMsg.MsgKey.Name.Data("TRI");
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives refresh for first request. */
                consumerReactor.Dispatch(1);

                testEvent = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
                msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(5, receivedRefreshMsg.StreamId);
                Assert.True(receivedRefreshMsg.CheckSolicited()); 
                Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());     

                /* Provider receives request with aggregated view. */
                providerReactor.Dispatch(1);
                testEvent = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
                msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasView());
                viewFieldList.Clear();
                viewFieldList.Add(22);
                viewFieldList.Add(25);
                viewFieldList.Add(1025);
                Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList));
                Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
                
                /* Provider sends refresh. */
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives refreshes for each request. */
                consumerReactor.Dispatch(2);

                testEvent = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
                msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(5, receivedRefreshMsg.StreamId);
                Assert.False(receivedRefreshMsg.CheckSolicited()); 
                Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());     

                testEvent = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
                msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(6, receivedRefreshMsg.StreamId);
                Assert.True(receivedRefreshMsg.CheckSolicited());
                Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
                Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.NotNull(msgEvent.StreamInfo);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
                
                /* Reissue this request, but don't change the view. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.ApplyPause();
                requestMsg.ApplyHasView();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI");

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
                
                /* This has no effect, so provider receives nothing. */
                providerReactor.Dispatch(0);
            }

            /* Disconnect provider. */
            provider.CloseChannel();

            /* Consumer receives channel event, Login Status, Directory Update, and status for the item streams. */
            consumerReactor.Dispatch(5);
            testEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, testEvent.EventType);
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)testEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            RDMLoginMsgEvent loginMsgEvent;                
            testEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, testEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);  

            RDMDirectoryMsgEvent directoryMsgEvent;                
            testEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, testEvent.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);   

            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(5, receivedStatusMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(6, receivedStatusMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Reconnect and reestablish login/directory streams. */
            TestReactor.OpenSession(consumer, provider, opts, true);

            /* Provider receives request again. */
            providerReactor.Dispatch(1);
            testEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasView());
            viewFieldList.Clear();
            viewFieldList.Add(22);
            viewFieldList.Add(25);
            viewFieldList.Add(1025);
            Assert.True(CheckHasCorrectView(provider, receivedRequestMsg, viewFieldList));
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refreshes for each request. */
            consumerReactor.Dispatch(2);

            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(5, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckSolicited()); 
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());     

            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(6, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
            
            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
        }
    }

    [Fact]
    public void BatchRequestSymbolListTest()
    {
        /* Test a batch request/refresh exchange that has a symbolList, with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;	       
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;	       
        requestMsg.ApplyHasQos();
        requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
        requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 1;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        EncodeBatchWithSymbolList(consumer.ReactorChannel, requestMsg, true);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        
        // Received status message with closed batch stream
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
        
        /* Provider receives request. */
        providerReactor.Dispatch(2);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS /* Don't call dispatch, need to process other request */);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        
        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message for TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        // Provider processes request for IBM.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message for IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    void BatchRequestSymbolListTestNoBehaviorsTest()
    {
         /* Test a batch request/refresh exchange that has a symbolList, with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyHasQos();
        requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
        requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 1;	      
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        EncodeBatchWithSymbolList(consumer.ReactorChannel, requestMsg, false);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        
        // Received status message with closed batch stream
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
        
        /* Provider receives request. */
        providerReactor.Dispatch(2);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();


        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS /* Don't call dispatch, need to process other request */);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        
        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message for TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        // Provider processes request for IBM.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRequestMsg.ContainerType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message for IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SymbolListDataSnapshotTest()
    {
        /* Test a symbol list snapshot request, that also requests data snapshots. */
       ReactorSubmitOptions submitOptions = new();
       TestReactorEvent testEvent;
       ReactorMsgEvent msgEvent;
       Msg msg = new();
       IRequestMsg requestMsg = new Msg();
       IRequestMsg receivedRequestMsg;
       IRefreshMsg refreshMsg = new Msg();
       IRefreshMsg receivedRefreshMsg;
       IUpdateMsg updateMsg = new Msg();
       int providerStreamId;

       /* Create reactors. */
       TestReactor consumerReactor = new();
       TestReactor providerReactor = new();

       /* Create consumer. */
       Consumer consumer = new(consumerReactor);
       ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
       consumerRole.InitDefaultRDMLoginRequest();
       consumerRole.InitDefaultRDMDirectoryRequest();
       consumerRole.ChannelEventCallback = consumer;
       consumerRole.LoginMsgCallback = consumer;
       consumerRole.DirectoryMsgCallback = consumer;
       consumerRole.DictionaryMsgCallback = consumer;
       consumerRole.DefaultMsgCallback = consumer;
       consumerRole.WatchlistOptions.EnableWatchlist = true;
       consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
       consumerRole.WatchlistOptions.RequestTimeout = 3000;

       /* Create provider. */
       Provider provider = new(providerReactor);
       ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
       providerRole.ChannelEventCallback = provider;
       providerRole.LoginMsgCallback = provider;
       providerRole.DirectoryMsgCallback = provider;
       providerRole.DictionaryMsgCallback = provider;
       providerRole.DefaultMsgCallback = provider;

       /* Connect the consumer and provider. Setup login & directory streams automatically. */
       ConsumerProviderSessionOptions opts = new();
       opts.SetupDefaultLoginStream = true;
       opts.SetupDefaultDirectoryStream = true;
       opts.ReconnectAttemptLimit = -1;

       provider.Bind(opts);

       TestReactor.OpenSession(consumer, provider, opts);

       Buffer payload = new(); 
       payload.Data(new ByteBuffer(1024));

       EncodeIterator encIter = new();
       encIter.Clear();
       encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
               consumer.ReactorChannel.MinorVersion);

       requestMsg.Clear();
       /* set-up message */
       requestMsg.MsgClass = MsgClasses.REQUEST;
       requestMsg.StreamId = 5;
       requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
       requestMsg.MsgKey.ApplyHasName();
       requestMsg.MsgKey.Name.Data("SYMBOL_LIST");
       requestMsg.ContainerType = DataTypes.ELEMENT_LIST;	       
       requestMsg.ApplyHasQos();
       requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
       requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
       requestMsg.ApplyHasPriority();
       requestMsg.Priority.PriorityClass = 1;
       requestMsg.Priority.Count = 1;	      

       ElementList elementList = new();
       elementList.Clear();
       elementList.ApplyHasStandardData();

       CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
       Assert.True(ret >= CodecReturnCode.SUCCESS);

       EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS);
       ret = elementList.EncodeComplete(encIter, true);
       Assert.True(ret >= CodecReturnCode.SUCCESS);
       requestMsg.EncodedDataBody = payload;        
       submitOptions.Clear();
       submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
       Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       /* Provider receives request. */
       providerReactor.Dispatch(1);
       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.False(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
       Assert.Equal("SYMBOL_LIST", receivedRequestMsg.MsgKey.Name.ToString());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);        
       providerStreamId = receivedRequestMsg.StreamId;

       /* Provider sends non-streaming refresh, with symbol list payload.*/
       refreshMsg.Clear();
       refreshMsg.MsgClass = MsgClasses.REFRESH;
       refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
       refreshMsg.StreamId = providerStreamId;
       refreshMsg.ContainerType = DataTypes.MAP; 
       refreshMsg.ApplySolicited();
       refreshMsg.ApplyRefreshComplete();
       refreshMsg.ApplyHasMsgKey();
       refreshMsg.MsgKey.ApplyHasServiceId();
       refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
       refreshMsg.MsgKey.ApplyHasName();
       refreshMsg.MsgKey.Name.Data("SYMBOL_LIST");
       refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
       refreshMsg.State.DataState(DataStates.OK);

       Map tempMap = new();
       MapEntry tempMapEntry = new();
       Buffer tempBuffer = new();
       EncodeIterator encodeIter = new();
       Buffer msgBuf = new(); 
       msgBuf.Data(new ByteBuffer(2048));

       /* encode message */
       encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
               consumer.ReactorChannel.MinorVersion);
       tempMap.Clear();
       tempMap.Flags = 0;
       tempMap.ContainerType = DataTypes.NO_DATA;
       tempMap.KeyPrimitiveType = DataTypes.BUFFER;
       ret = tempMap.EncodeInit(encodeIter, 0, 0);
       Assert.True(ret >= CodecReturnCode.SUCCESS);

       /* encode map entry */
       tempMapEntry.Clear();
       tempMapEntry.Flags = MapEntryFlags.NONE;
       tempBuffer.Clear();

       tempBuffer.Data("FB.O");
       tempMapEntry.Action = MapEntryActions.ADD;

       ret = tempMapEntry.EncodeInit(encodeIter, tempBuffer, 0);
       Assert.True(ret >= CodecReturnCode.SUCCESS);

       ret = tempMapEntry.EncodeComplete(encodeIter, true);
       Assert.True(ret >= CodecReturnCode.SUCCESS);
       tempBuffer.Data("AAPL.O");
       ret = tempMapEntry.Encode(encodeIter, tempBuffer);
       Assert.True(ret >= CodecReturnCode.SUCCESS);
       tempBuffer.Data("NFLX.O");
       ret = tempMapEntry.Encode(encodeIter, tempBuffer);
       Assert.True(ret >= CodecReturnCode.SUCCESS);
       tempBuffer.Data("GOOGL.O");
       ret = tempMapEntry.Encode(encodeIter, tempBuffer);
       Assert.True(ret >= CodecReturnCode.SUCCESS);
       ret = tempMap.EncodeComplete(encodeIter, true);
       Assert.True(ret >= CodecReturnCode.SUCCESS);         
       refreshMsg.EncodedDataBody = msgBuf;         
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

       /* Watchlist recevies SymbolList refresh, and internally sends requests for each item. */
       consumerReactor.Dispatch(1);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.Equal(5, receivedRefreshMsg.StreamId);
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("SYMBOL_LIST", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);

       /* Provider receives requests for the items in the symbol list. */
       providerReactor.Dispatch(4);

       int itemStreamId1, itemStreamId2, itemStreamId3, itemStreamId4;

       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.False(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
       Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
       itemStreamId1 = receivedRequestMsg.StreamId;

       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.False(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
       Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
       itemStreamId2 = receivedRequestMsg.StreamId;

       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.False(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
       Assert.Equal("NFLX.O", receivedRequestMsg.MsgKey.Name.ToString());
       itemStreamId3 = receivedRequestMsg.StreamId;

       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.False(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
       Assert.Equal("GOOGL.O", receivedRequestMsg.MsgKey.Name.ToString());
       itemStreamId4 = receivedRequestMsg.StreamId;

       /* Provider sends refreshes .*/
       refreshMsg.Clear();
       refreshMsg.MsgClass = MsgClasses.REFRESH;
       refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
       refreshMsg.StreamId = itemStreamId1;
       refreshMsg.ContainerType = DataTypes.NO_DATA;
       refreshMsg.ApplySolicited();
       refreshMsg.ApplyRefreshComplete();
       refreshMsg.ApplyHasMsgKey();
       refreshMsg.MsgKey.ApplyHasServiceId();
       refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
       refreshMsg.MsgKey.ApplyHasName();
       refreshMsg.MsgKey.Name.Data("FB.O");
       refreshMsg.ApplyRefreshComplete();        
       refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
       refreshMsg.State.DataState(DataStates.OK);
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       refreshMsg.MsgKey.Name.Data("AAPL.O");
       refreshMsg.StreamId = itemStreamId2;
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       refreshMsg.MsgKey.Name.Data("NFLX.O");
       refreshMsg.StreamId = itemStreamId3;
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       refreshMsg.MsgKey.Name.Data("GOOGL.O");
       refreshMsg.StreamId = itemStreamId4;
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       /* Consumer receives refreshes. */
       consumerReactor.Dispatch(4);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.StreamId < 0);
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("FB.O", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.StreamId < 0);
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("AAPL.O", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.StreamId < 0);
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("NFLX.O", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.StreamId < 0);
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("GOOGL.O", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);

       /* Send updates. Consumer should not receive them. */
       updateMsg.Clear();
       updateMsg.MsgClass = MsgClasses.UPDATE;
       updateMsg.StreamId = itemStreamId1;
       updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
       updateMsg.ContainerType = DataTypes.NO_DATA;
       Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       updateMsg.StreamId = itemStreamId2;
       Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       updateMsg.StreamId = itemStreamId3;
       Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       updateMsg.StreamId = itemStreamId4;
       Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       consumerReactor.Dispatch(0);

       /* Provider doesn't get CloseMsgs for the unwanted updates. */
       providerReactor.Dispatch(0);

       /* Disconnect provider. */
       provider.CloseChannel();

       /* Consumer receives Channel Event, Login Status, Directory Update, nothing else (symbol list and items were snapshots, and
        * so should not be re-requested). */
       consumerReactor.Dispatch(3);
       testEvent = consumer.TestReactor.PollEvent();
       Assert.Equal(TestReactorEventType.CHANNEL_EVENT, testEvent.EventType);
       ReactorChannelEvent channelEvent = (ReactorChannelEvent)testEvent.ReactorEvent;
       Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

       RDMLoginMsgEvent loginMsgEvent;                
       testEvent = consumer.TestReactor.PollEvent();
       Assert.Equal(TestReactorEventType.LOGIN_MSG, testEvent.EventType);
       loginMsgEvent = (RDMLoginMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);  

       RDMDirectoryMsgEvent directoryMsgEvent;                
       testEvent = consumer.TestReactor.PollEvent();
       Assert.Equal(TestReactorEventType.DIRECTORY_MSG, testEvent.EventType);
       directoryMsgEvent = (RDMDirectoryMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);   

       /* Reconnect and reestablish login/directory streams. */
       TestReactor.OpenSession(consumer, provider, opts, true);

       /* Provider receives nothing else (no recovery). */
       providerReactor.Dispatch(0);
        
       TestReactorComponent.CloseSession(consumer, provider);
       TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SymbolListDataStreamTest_ReconnectTest()
    {
        /* Test recovery of symbol list data streams after reconnect:
         * 1) Open a symbol list stream, requesting data streams
         * 2) Establish symbol list and item streams
         * 3) Kill/restart connection
         * 4) Re-establish symbol list and item streams */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true ;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends symbol list request, which requests data streams. */
        Buffer payload = new(); 
        payload.Data(new ByteBuffer(1024));

        EncodeIterator encIter = new();
        encIter.Clear();
        encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MinorVersion);
            
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyHasQos();
        requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
        requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 1;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("SYM_LIST");
                
        ElementList elementList = new();
        elementList.Clear();
        elementList.ApplyHasStandardData();

        CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
        ret = elementList.EncodeComplete(encIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        requestMsg.EncodedDataBody = payload;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
                
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal("SYM_LIST", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
           
        refreshMsg.ContainerType = DataTypes.MAP; 
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("SYM_LIST");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Map tempMap = new();
        MapEntry tempMapEntry = new();
        Buffer tempBuffer = new();
        EncodeIterator encodeIter = new();
        Buffer msgBuf = new(); 
        msgBuf.Data(new ByteBuffer(2048));
 
        /* encode message */
        encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MinorVersion);
        tempMap.Clear();
        tempMap.Flags = 0;
        tempMap.ContainerType = DataTypes.NO_DATA;
        tempMap.KeyPrimitiveType = DataTypes.BUFFER;
        ret = tempMap.EncodeInit(encodeIter, 0, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        /* encode map entry */
        tempMapEntry.Clear();
        tempMapEntry.Flags = MapEntryFlags.NONE;
        tempBuffer.Clear();
 
        tempBuffer.Data("FB.O");
        tempMapEntry.Action = MapEntryActions.ADD;
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        tempBuffer.Data("AAPL.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        ret = tempMap.EncodeComplete(encodeIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);         

        refreshMsg.EncodedDataBody = msgBuf;         
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       
                
        // watchlist receives a symbol list data refresh and send out each item request
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("SYM_LIST", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        /* Provider receives requests for FB and AAPL. */
        providerReactor.Dispatch(2);

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        int provFBStreamId = receivedRequestMsg.StreamId;

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        int provAAPLStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refreshes for FB and AAPL .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = provFBStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("FB.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = provAAPLStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("AAPL.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refreshes. */
        consumerReactor.Dispatch(2);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("FB.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.True(receivedRefreshMsg.StreamId < 0);
        int consFBStreamId = receivedRefreshMsg.StreamId;

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("AAPL.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.True(receivedRefreshMsg.StreamId < 0);
        int consAAPLStreamId = receivedRefreshMsg.StreamId;

        /* Disconnect provider. */
        provider.CloseChannel();
        
        /* Consumer receives Login Status, Directory Update, and item status for SYM_LIST, FB.O & AAPL.O. */
        consumerReactor.Dispatch(6);
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, testEvent.EventType);
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)testEvent.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

        RDMLoginMsgEvent loginMsgEvent;
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, testEvent.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);  

        RDMDirectoryMsgEvent directoryMsgEvent;
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, testEvent.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);   

        /* SYM_LIST */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.Equal(5, receivedStatusMsg.StreamId);

        /* FB.O */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.Equal(consFBStreamId, receivedStatusMsg.StreamId);

        /* AAPL.O */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.Equal(consAAPLStreamId, receivedStatusMsg.StreamId);

        /* Reconnect and reestablish login/directory streams. */
        TestReactor.OpenSession(consumer, provider, opts, true);
        
        /* Provider receives requests for SYM_LIST, FB.O, and AAPL.O. */
        providerReactor.Dispatch(3);
        
        /* SYM_LIST */
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal("SYM_LIST", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* FB.O */
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
        provFBStreamId = receivedRequestMsg.StreamId;
        
        /* AAPL.O */
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
        provAAPLStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refreshes for SYM_LIST, FB and AAPL .*/

        /* SYM_LIST (with same payload) */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.MAP; 
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("SYM_LIST");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.EncodedDataBody = msgBuf;
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       
                    
        /* FB.O */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = provFBStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("FB.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* AAPL.O */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = provAAPLStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("AAPL.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refreshes. */
        consumerReactor.Dispatch(3);

        /* SYM_LIST */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("SYM_LIST", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        /* FB.O */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("FB.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.Equal(consFBStreamId, receivedRefreshMsg.StreamId);

        /* AAPL.O */
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("AAPL.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.Equal(consAAPLStreamId, receivedRefreshMsg.StreamId);
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SymbolListDataStreamUpdateSymbolListTest()
    {
        /* Test updates to a symbol list with a consumer requesting symbol list data streams:
         * - Initially send a symbol list refresh with three items, which will be automatically requested. Refresh these items.
         * - Send an update to add a fourth item, which will be automatically requested. Refresh this item (with a non-streaming refresh).
         * - Send an update to add the same fourth item again. Since it is not open, it will be automatically requested. Refresh it (streaming this time).
         * - Close the fourth item from the consumer.
         * - Send an update to add the same fourth item yet again. Since it is not open, it will be automatically requested. Refresh it.
         * - Send an update to add the same fourth item yet again. This time, the item is still open, so it shouldn't be requested again.
         */ 
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IUpdateMsg updateMsg = new Msg();
        IUpdateMsg receivedUpdateMsg;
        ICloseMsg closeMsg = new Msg();
        ICloseMsg receivedCloseMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        Buffer payload = new(); 
        payload.Data(new ByteBuffer(1024));

        EncodeIterator encIter = new();
        encIter.Clear();
        encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MinorVersion);

        requestMsg.Clear();
        /* set-up message */
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("SYMBOL_LIST");
        requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyHasQos();
        requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
        requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 1;
        requestMsg.ApplyStreaming();

        ElementList elementList = new();
        elementList.Clear();
        elementList.ApplyHasStandardData();

        CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
        ret = elementList.EncodeComplete(encIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        requestMsg.EncodedDataBody = payload;        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("SYMBOL_LIST", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh, with symbol list payload.*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.MAP; 
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("SYMBOL_LIST");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Map tempMap = new();
        MapEntry tempMapEntry = new();
        Buffer tempBuffer = new();
        EncodeIterator encodeIter = new();
        Buffer msgBuf = new(); 
        msgBuf.Data(new ByteBuffer(2048));

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MinorVersion);
        tempMap.Clear();
        tempMap.Flags = 0;
        tempMap.ContainerType = DataTypes.NO_DATA;
        tempMap.KeyPrimitiveType = DataTypes.BUFFER;
        ret = tempMap.EncodeInit(encodeIter, 0, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        /* encode map entry */
        tempMapEntry.Clear();
        tempMapEntry.Flags = MapEntryFlags.NONE;
        tempBuffer.Clear();

        tempBuffer.Data("FB.O");
        tempMapEntry.Action = MapEntryActions.ADD;
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        tempBuffer.Data("AAPL.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        tempBuffer.Data("NFLX.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        ret = tempMap.EncodeComplete(encodeIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        refreshMsg.EncodedDataBody = msgBuf;
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

        /* Watchlist recevies SymbolList refresh, and internally sends requests for each item. */
        consumerReactor.Dispatch(1);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("SYMBOL_LIST", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider receives requests for the items in the symbol list. */
        providerReactor.Dispatch(3);

        int itemStreamId1, itemStreamId2, itemStreamId3;

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
        Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
        itemStreamId1 = receivedRequestMsg.StreamId;

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
        itemStreamId2 = receivedRequestMsg.StreamId;

        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal("NFLX.O", receivedRequestMsg.MsgKey.Name.ToString());
        itemStreamId3 = receivedRequestMsg.StreamId;

        /* Provider sends refreshes .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = itemStreamId1;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("FB.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
        refreshMsg.State.DataState(DataStates.OK);
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        refreshMsg.MsgKey.Name.Data("AAPL.O");
        refreshMsg.StreamId = itemStreamId2;
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        refreshMsg.MsgKey.Name.Data("NFLX.O");
        refreshMsg.StreamId = itemStreamId3;
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refreshes. */
        consumerReactor.Dispatch(3);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.StreamId < 0);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("FB.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.StreamId < 0);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("AAPL.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.StreamId < 0);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("NFLX.O", receivedRefreshMsg.MsgKey.Name.ToString());

        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider sends an update with a new item. */
        encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
                consumer.ReactorChannel.MinorVersion);
        tempMap.Clear();
        tempMap.Flags = 0;
        tempMap.ContainerType = DataTypes.NO_DATA;
        tempMap.KeyPrimitiveType = DataTypes.BUFFER;
        ret = tempMap.EncodeInit(encodeIter, 0, 0);
        Assert.True(ret >= CodecReturnCode.SUCCESS);

        tempMapEntry.Clear();
        tempMapEntry.Flags = MapEntryFlags.NONE;
        tempMapEntry.Action = MapEntryActions.ADD;
        tempBuffer.Clear();
        tempBuffer.Data("GOOGL.O");
        ret = tempMapEntry.Encode(encodeIter, tempBuffer);
        Assert.True(ret >= CodecReturnCode.SUCCESS);
        ret = tempMap.EncodeComplete(encodeIter, true);
        Assert.True(ret >= CodecReturnCode.SUCCESS);         

        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        updateMsg.ContainerType = DataTypes.MAP;
        updateMsg.EncodedDataBody = msgBuf;         
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        int itemStreamId4;

        /* Provider receives request for new item. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal("GOOGL.O", receivedRequestMsg.MsgKey.Name.ToString());
        itemStreamId4 = receivedRequestMsg.StreamId;

        /* Provider sends non-streaming refresh for the item. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = itemStreamId4;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("GOOGL.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
        refreshMsg.State.DataState(DataStates.OK);
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.StreamId < 0);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("GOOGL.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider sends symbol list update with the new item again. */
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider receives request for new item. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
        Assert.Equal("GOOGL.O", receivedRequestMsg.MsgKey.Name.ToString());
        itemStreamId4 = receivedRequestMsg.StreamId;

        /* Provider sends streaming refresh for the item. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = itemStreamId4;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("GOOGL.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.StreamId < 0);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("GOOGL.O",receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        
        int consumerItemStreamId4 = receivedRefreshMsg.StreamId;

        /* Consumer closes item. */
        closeMsg.Clear();
        closeMsg.MsgClass = MsgClasses.CLOSE;
        closeMsg.StreamId = consumerItemStreamId4;
        closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
        closeMsg.ContainerType = DataTypes.NO_DATA; 
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives close. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
        receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
        Assert.Equal(itemStreamId4, receivedCloseMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);

        /* Provider sends symbol list update with the new item yet again. */
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider receives request for new item. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal("GOOGL.O", receivedRequestMsg.MsgKey.Name.ToString());
        itemStreamId4 = receivedRequestMsg.StreamId;

        /* Provider sends streaming refresh for the item. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = itemStreamId4;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("GOOGL.O");
        refreshMsg.ApplyRefreshComplete();        
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.StreamId < 0);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("GOOGL.O", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);

        /* Provider sends symbol list update with the new item yet again. */
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
        receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
        Assert.Equal(5, receivedUpdateMsg.StreamId);
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedUpdateMsg.DomainType);
        Assert.Equal(DataTypes.MAP, receivedUpdateMsg.ContainerType);
        Assert.NotNull(msgEvent.StreamInfo);

        /* Item is already open this time, so provider does not receive a request for it. */
        providerReactor.Dispatch(0);
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void SymbolListDataStreamTest_MsgKeyTest()
    {
        /* Test that the watchlist adds the MsgKey to symbol list data streams when appropriate --
         * namely when the initial response does not contain one. */

        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg statusMsg = new Msg();
        IStatusMsg receivedStatusMsg;
        int providerStreamId;

        int[] streamStates = new[]{ StreamStates.OPEN, StreamStates.CLOSED, StreamStates.CLOSED_RECOVER};

        for (int i = 0; i < streamStates.Length; ++i)
        {
            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = -1;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Consumer sends symbol list request, which requests data streams. */
            Buffer payload = new();
            payload.Data(new ByteBuffer(1024));

            EncodeIterator encIter = new();
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
                    consumer.ReactorChannel.MinorVersion);

            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
            requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.ApplyHasPriority();
            requestMsg.Priority.PriorityClass = 1;
            requestMsg.Priority.Count = 1;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("SYM_LIST");

            ElementList elementList = new();
            elementList.Clear();
            elementList.ApplyHasStandardData();

            CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            ret = elementList.EncodeComplete(encIter, true);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            requestMsg.EncodedDataBody = payload;
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            testEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.Equal("SYM_LIST", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
            refreshMsg.StreamId = providerStreamId;

            refreshMsg.ContainerType = DataTypes.MAP;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("SYM_LIST");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            Map tempMap = new();
            MapEntry tempMapEntry = new();
            Buffer tempBuffer = new();
            EncodeIterator encodeIter = new();
            Buffer msgBuf = new();
            msgBuf.Data(new ByteBuffer(2048));

            /* encode message */
            encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
                    consumer.ReactorChannel.MinorVersion);
            tempMap.Clear();
            tempMap.Flags = MapFlags.NONE;
            tempMap.ContainerType = DataTypes.NO_DATA;
            tempMap.KeyPrimitiveType = DataTypes.BUFFER;
            ret = tempMap.EncodeInit(encodeIter, 0, 0);
            Assert.True(ret >= CodecReturnCode.SUCCESS);

            /* encode map entry */
            tempMapEntry.Clear();
            tempMapEntry.Flags = MapEntryFlags.NONE;
            tempBuffer.Clear();

            tempBuffer.Data("FB.O");
            tempMapEntry.Action = MapEntryActions.ADD;
            ret = tempMapEntry.Encode(encodeIter, tempBuffer);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            tempBuffer.Data("AAPL.O");
            ret = tempMapEntry.Encode(encodeIter, tempBuffer);
            Assert.True(ret >= CodecReturnCode.SUCCESS);

            ret = tempMap.EncodeComplete(encodeIter, true);
            Assert.True(ret >= CodecReturnCode.SUCCESS);

            refreshMsg.EncodedDataBody = msgBuf;
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // watchlist receives a symbol list data refresh and send out each item request
            consumerReactor.Dispatch(1);
            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("SYM_LIST", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Provider receives requests for FB and AAPL. */
            providerReactor.Dispatch(2);

            testEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            int provFBStreamId = receivedRequestMsg.StreamId;

            testEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            int provAAPLStreamId = receivedRequestMsg.StreamId;

            /* Provider sends a refresh for FB with no key. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = provFBStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider sends a StatusMsg with no key for FB. */
            statusMsg.Clear();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.StreamId = provFBStreamId;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(streamStates[i]);
            statusMsg.State.DataState(DataStates.SUSPECT);
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider sends a StatusMsg with no key for AAPL. */
            statusMsg.Clear();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.StreamId = provAAPLStreamId;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(streamStates[i]);
            statusMsg.State.DataState(DataStates.SUSPECT);
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives messages. */
            consumerReactor.Dispatch(3);

            /* Refresh for FB. WL should add key since this is the initial response. */
            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("FB.O", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.True(receivedRefreshMsg.StreamId < 0);
            int consFBStreamId = receivedRefreshMsg.StreamId;

            /* StatusMsg for FB. No key should be present as it was not sent and this is not the initial response. */
            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.False(receivedStatusMsg.CheckHasMsgKey());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            if (streamStates[i] == StreamStates.CLOSED_RECOVER)
                Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            else
                Assert.Equal(streamStates[i], receivedStatusMsg.State.StreamState());

            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.True(receivedStatusMsg.StreamId < 0);
            Assert.Equal(consFBStreamId, receivedStatusMsg.StreamId);

            /* StatusMsg for AAPL. WL should add key since this is the initial response. */
            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
            msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatusMsg.CheckHasMsgKey());
            Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedStatusMsg.MsgKey.ServiceId);
            Assert.True(receivedStatusMsg.MsgKey.CheckHasName());
            Assert.Equal("AAPL.O", receivedStatusMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            if (streamStates[i] == StreamStates.CLOSED_RECOVER)
                Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            else
                Assert.Equal(streamStates[i], receivedStatusMsg.State.StreamState());

            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.True(receivedStatusMsg.StreamId < 0);

            if (streamStates[i] == StreamStates.CLOSED_RECOVER)
            {
                /* Provider sent ClosedRecover status, so it receives the requests again. */
                providerReactor.Dispatch(2);

                testEvent = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
                msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

                testEvent = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
                msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
                Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
        }
    }

    /* Used by symbolListDataStreamTest_FromCallback. */
    class SymbolListRequestFromCallbackConsumer : Consumer
    {
        public SymbolListRequestFromCallbackConsumer(TestReactor testReactor):
            base(testReactor)
        {
        }

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            base.ReactorChannelEventCallback(evt);
           
            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                ReactorSubmitOptions submitOptions = new();
                IRequestMsg requestMsg = new Msg();

                /* Consumer sends symbol list request, which requests data streams. */
                Buffer payload = new Buffer(); 
                payload.Data(new ByteBuffer(1024));

                EncodeIterator encIter = new();
                encIter.Clear();
                encIter.SetBufferAndRWFVersion(payload, ReactorChannel.MajorVersion,
                        ReactorChannel.MinorVersion);

                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;	       
                requestMsg.ContainerType = DataTypes.ELEMENT_LIST;	       
                requestMsg.ApplyHasQos();
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.ApplyHasPriority();
                requestMsg.Priority.PriorityClass = 1;
                requestMsg.Priority.Count = 1;	      
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("SYM_LIST");

                ElementList elementList = new();
                elementList.Clear();
                elementList.ApplyHasStandardData();

                CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
                Assert.True(ret >= CodecReturnCode.SUCCESS);
                EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                ret = elementList.EncodeComplete(encIter, true);
                Assert.True(ret >= CodecReturnCode.SUCCESS);
                requestMsg.EncodedDataBody = payload;        
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            }
           
            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void SymbolListDataStreamTest_FromChannelOpenCallbackTest()
    {
        /* Test requesting a symbol list with data streams from the channel-open callback. */

       ReactorSubmitOptions submitOptions = new();
       TestReactorEvent testEvent;
       ReactorMsgEvent msgEvent;
       Msg msg = new();
       IRequestMsg receivedRequestMsg;
       IRefreshMsg refreshMsg = msg;
       IRefreshMsg receivedRefreshMsg;
       int providerStreamId;
       CodecReturnCode ret;

       /* Create reactors. */
       TestReactor consumerReactor = new();
       TestReactor providerReactor = new();

       /* Create consumer. */
       Consumer consumer = new SymbolListRequestFromCallbackConsumer(consumerReactor);
       ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
       consumerRole.InitDefaultRDMLoginRequest();
       consumerRole.InitDefaultRDMDirectoryRequest();
       consumerRole.ChannelEventCallback = consumer;
       consumerRole.LoginMsgCallback = consumer;
       consumerRole.DirectoryMsgCallback = consumer;
       consumerRole.DictionaryMsgCallback = consumer;
       consumerRole.DefaultMsgCallback = consumer;
       consumerRole.WatchlistOptions.EnableWatchlist = true;
       consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
       consumerRole.WatchlistOptions.RequestTimeout = 3000;

       /* Create provider. */
       Provider provider = new (providerReactor);
       ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
       providerRole.ChannelEventCallback = provider;
       providerRole.LoginMsgCallback = provider;
       providerRole.DirectoryMsgCallback = provider;
       providerRole.DictionaryMsgCallback = provider;
       providerRole.DefaultMsgCallback = provider;

       /* Connect the consumer and provider. Setup login & directory streams automatically. */
       ConsumerProviderSessionOptions opts = new ();
       opts.SetupDefaultLoginStream = true;
       opts.SetupDefaultDirectoryStream = true;
       opts.ReconnectAttemptLimit = -1;
       opts.NumStatusEvents = 1;

       provider.Bind(opts);

       TestReactor.OpenSession(consumer, provider, opts);

       /* Provider receives symbol list request. */
       providerReactor.Dispatch(1);
       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal("SYM_LIST", receivedRequestMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);        
       providerStreamId = receivedRequestMsg.StreamId;

       /* Provider sends refresh .*/
       refreshMsg.Clear();
       refreshMsg.MsgClass = MsgClasses.REFRESH;
       refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
       refreshMsg.StreamId = providerStreamId;

       refreshMsg.ContainerType = DataTypes.MAP; 
       refreshMsg.ApplyHasMsgKey();
       refreshMsg.MsgKey.ApplyHasServiceId();
       refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
       refreshMsg.MsgKey.ApplyHasName();
       refreshMsg.MsgKey.Name.Data("SYM_LIST");
       refreshMsg.State.StreamState(StreamStates.OPEN);
       refreshMsg.State.DataState(DataStates.OK);
       refreshMsg.ApplySolicited();

       Map tempMap = new();
       MapEntry tempMapEntry = new();
       Buffer tempBuffer = new();
       EncodeIterator encodeIter = new();
       Buffer msgBuf = new(); 
       msgBuf.Data(new ByteBuffer(2048));

       /* encode message */
       encodeIter.SetBufferAndRWFVersion(msgBuf, consumer.ReactorChannel.MajorVersion,
               consumer.ReactorChannel.MinorVersion);
       tempMap.Clear();
        tempMap.Flags = MapFlags.NONE;
       tempMap.ContainerType = DataTypes.NO_DATA;
       tempMap.KeyPrimitiveType = DataTypes.BUFFER;
       ret = tempMap.EncodeInit(encodeIter, 0, 0);
       Assert.True(ret >= CodecReturnCode.SUCCESS);

       /* encode map entry */
       tempMapEntry.Clear();
       tempMapEntry.Flags = MapEntryFlags.NONE;
       tempBuffer.Clear();

       tempBuffer.Data("FB.O");
       tempMapEntry.Action = MapEntryActions.ADD;
       ret = tempMapEntry.Encode(encodeIter, tempBuffer);
       Assert.True(ret >= CodecReturnCode.SUCCESS);
       tempBuffer.Data("AAPL.O");
       ret = tempMapEntry.Encode(encodeIter, tempBuffer);
       Assert.True(ret >= CodecReturnCode.SUCCESS);

       ret = tempMap.EncodeComplete(encodeIter, true);
       Assert.True(ret >= CodecReturnCode.SUCCESS);         

       refreshMsg.EncodedDataBody = msgBuf;         
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);       

       // watchlist receives a symbol list data refresh and send out each item request
       consumerReactor.Dispatch(1);
       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("SYM_LIST", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);
       Assert.NotNull(msgEvent.StreamInfo.ServiceName);
       Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

       /* Provider receives requests for FB and AAPL. */
       providerReactor.Dispatch(2);

       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal("FB.O", receivedRequestMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
       int provFBStreamId = receivedRequestMsg.StreamId;

       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming());
       Assert.False(receivedRequestMsg.CheckNoRefresh());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.Equal("AAPL.O", receivedRequestMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);        
       int provAAPLStreamId = receivedRequestMsg.StreamId;

       /* Provider sends refreshes for FB and AAPL .*/
       refreshMsg.Clear();
       refreshMsg.MsgClass = MsgClasses.REFRESH;
       refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
       refreshMsg.StreamId = provFBStreamId;
       refreshMsg.ContainerType = DataTypes.NO_DATA;
       refreshMsg.ApplyHasMsgKey();
       refreshMsg.MsgKey.ApplyHasServiceId();
       refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
       refreshMsg.MsgKey.ApplyHasName();
       refreshMsg.MsgKey.Name.Data("FB.O");
       refreshMsg.ApplyRefreshComplete();        
       refreshMsg.State.StreamState(StreamStates.OPEN);
       refreshMsg.State.DataState(DataStates.OK);
       refreshMsg.ApplySolicited();
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       refreshMsg.Clear();
       refreshMsg.MsgClass = MsgClasses.REFRESH;
       refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
       refreshMsg.StreamId = provAAPLStreamId;
       refreshMsg.ContainerType = DataTypes.NO_DATA;
       refreshMsg.ApplyHasMsgKey();
       refreshMsg.MsgKey.ApplyHasServiceId();
       refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
       refreshMsg.MsgKey.ApplyHasName();
       refreshMsg.MsgKey.Name.Data("AAPL.O");
       refreshMsg.ApplyRefreshComplete();        
       refreshMsg.State.StreamState(StreamStates.OPEN);
       refreshMsg.State.DataState(DataStates.OK);
       refreshMsg.ApplySolicited();
       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

       /* Consumer receives refreshes. */
       consumerReactor.Dispatch(2);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("FB.O", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);
       Assert.True(receivedRefreshMsg.StreamId < 0);

       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal("AAPL.O", receivedRefreshMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);
       Assert.True(receivedRefreshMsg.StreamId < 0);
       
       TestReactorComponent.CloseSession(consumer, provider);
       TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
   }

    [Fact]
    public void DictionaryRecoveryTest()
    {
        /* Test dictionary recovery behavior --
         * - A dictionary that has not received a complete refresh is recovered
         * - A dictionary that has received a complete refresh is not recovered */
        DataDictionary dictionary = new();
        CodecError error = new();
        ReactorErrorInfo errorInfo = new();
        EncodeIterator eIter = new();

        Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary(DICTIONARY_FILE_NAME, out error));

        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        RDMDictionaryMsgEvent dictionaryMsgEvent;
        DictionaryRequest dictionaryRequest = new();
        DictionaryRequest receivedDictionaryRequest;
        DictionaryStatus receivedDictionaryStatus;
        DictionaryRefresh dictionaryRefresh = new();
        DictionaryRefresh receivedDictionaryRefresh;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new (consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();

        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 20000;

        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;
        opts.NumOfGuaranteedBuffers = 1000;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends dictionary request. */
        dictionaryRequest.Clear();
        dictionaryRequest.StreamId = 5;
        dictionaryRequest.Streaming = true;
        dictionaryRequest.DictionaryName.Data("RWFFld");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch(dictionaryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives dictionary request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
        dictionaryMsgEvent = (RDMDictionaryMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(DictionaryMsgType.REQUEST, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);

        receivedDictionaryRequest = dictionaryMsgEvent.DictionaryMsg.DictionaryRequest;
        Assert.True(receivedDictionaryRequest.Streaming);
        Assert.Equal(Provider.DefaultService.ServiceId, receivedDictionaryRequest.ServiceId);
        Assert.Equal("RWFFld", receivedDictionaryRequest.DictionaryName.ToString());
        Assert.Equal((int)DomainType.DICTIONARY, receivedDictionaryRequest.DomainType);

        /* Disconnect Provider. */
        provider.CloseChannel();
        
        /* Consumer receives channel testEvent, Login Status, Directory Update, and open/suspect status for the dictionary. */
        consumerReactor.Dispatch(4);
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, testEvent.EventType);
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)testEvent.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

        RDMLoginMsgEvent loginMsgEvent;
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, testEvent.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);

        RDMDirectoryMsgEvent directoryMsgEvent;
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, testEvent.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
        dictionaryMsgEvent = (RDMDictionaryMsgEvent) testEvent.ReactorEvent;
        Assert.Equal(DictionaryMsgType.STATUS, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);
        receivedDictionaryStatus = dictionaryMsgEvent.DictionaryMsg.DictionaryStatus;
        Assert.Equal(5, receivedDictionaryStatus.StreamId);
        Assert.True(receivedDictionaryStatus.HasState);
        Assert.Equal(StreamStates.OPEN, receivedDictionaryStatus.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedDictionaryStatus.State.DataState());
        Assert.NotNull(dictionaryMsgEvent.StreamInfo);
        Assert.NotNull(dictionaryMsgEvent.StreamInfo.ServiceName);
        Assert.Equal(dictionaryMsgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Reconnect and reestablish login/directory streams. */
        TestReactor.OpenSession(consumer, provider, opts, true);

        /* Provider receives dictionary request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
        dictionaryMsgEvent = (RDMDictionaryMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(DictionaryMsgType.REQUEST, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);

        receivedDictionaryRequest = dictionaryMsgEvent.DictionaryMsg.DictionaryRequest;
        Assert.True(receivedDictionaryRequest.Streaming);
        Assert.Equal(Provider.DefaultService.ServiceId, receivedDictionaryRequest.ServiceId);
        Assert.Equal("RWFFld", receivedDictionaryRequest.DictionaryName.ToString());
        Assert.Equal(VerbosityValues.NORMAL, receivedDictionaryRequest.Verbosity);
        providerStreamId = receivedDictionaryRequest.StreamId;

        /* Encode full dictionary refresh. */
        dictionaryRefresh.Clear();
        dictionaryRefresh.StreamId = providerStreamId;
        dictionaryRefresh.Solicited = true;
        dictionaryRefresh.DictionaryName.Data("RWFFld");
        dictionaryRefresh.DictionaryType = Types.FIELD_DEFINITIONS;
        dictionaryRefresh.Verbosity = VerbosityValues.NORMAL;
        dictionaryRefresh.ServiceId = Provider.DefaultService.ServiceId;
        dictionaryRefresh.State.StreamState(StreamStates.OPEN);
        dictionaryRefresh.State.DataState(DataStates.OK);
        dictionaryRefresh.DataDictionary = dictionary;
        
        ITransportBuffer buffer = provider.ReactorChannel.GetBuffer(1000000, false, out errorInfo);
        eIter.Clear();
        eIter.SetBufferAndRWFVersion(buffer, provider.ReactorChannel.MajorVersion, provider.ReactorChannel.MinorVersion);
        Assert.Equal(CodecReturnCode.SUCCESS, dictionaryRefresh.Encode(eIter));
        submitOptions.Clear();

        ReactorReturnCode ret = 0;
        
        do
        {
            ret = provider.ReactorChannel.Submit(buffer, submitOptions, out errorInfo);
        }while(ret == ReactorReturnCode.WRITE_CALL_AGAIN);
        
        Assert.True(ret >= ReactorReturnCode.SUCCESS);
        providerReactor.Dispatch(0);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
        dictionaryMsgEvent = (RDMDictionaryMsgEvent) testEvent.ReactorEvent;
        Assert.Equal(DictionaryMsgType.REFRESH, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);

        receivedDictionaryRefresh = dictionaryMsgEvent.DictionaryMsg.DictionaryRefresh;
        Assert.Equal(5, receivedDictionaryRefresh.StreamId);
        Assert.True(receivedDictionaryRefresh.Solicited); 
        Assert.True(receivedDictionaryRefresh.RefreshComplete); 
        Assert.Equal(StreamStates.OPEN, receivedDictionaryRefresh.State.StreamState());
        Assert.Equal(DataStates.OK, receivedDictionaryRefresh.State.DataState());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedDictionaryRefresh.ServiceId);
        Assert.NotNull(dictionaryMsgEvent.StreamInfo);
        Assert.NotNull(dictionaryMsgEvent.StreamInfo.ServiceName);
        Assert.Equal(dictionaryMsgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());     

        /* Disconnect Provider. */
        provider.CloseChannel();

        providerReactor.Dispatch(0);
        
        /* Consumer receives channel testEvent, Login Status, Directory Update, and closed-recover/suspect status for the dictionary. */
        consumerReactor.Dispatch(4);
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, testEvent.EventType);
        channelEvent = (ReactorChannelEvent)testEvent.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);
              
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, testEvent.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);  
            
        testEvent = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, testEvent.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);   
        
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
        dictionaryMsgEvent = (RDMDictionaryMsgEvent) testEvent.ReactorEvent;
        Assert.Equal(DictionaryMsgType.STATUS, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);
        receivedDictionaryStatus = dictionaryMsgEvent.DictionaryMsg.DictionaryStatus;
        Assert.Equal(5, receivedDictionaryStatus.StreamId);
        Assert.True(receivedDictionaryStatus.HasState);
        Assert.Equal(StreamStates.CLOSED_RECOVER, receivedDictionaryStatus.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedDictionaryStatus.State.DataState());
        Assert.NotNull(dictionaryMsgEvent.StreamInfo);
        Assert.NotNull(dictionaryMsgEvent.StreamInfo.ServiceName);
        Assert.Equal(dictionaryMsgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Reconnect and reestablish login/directory streams. */
        TestReactor.OpenSession(consumer, provider, opts, true);

        /* Provider receives nothing (dictionary not recovered). */
        providerReactor.Dispatch(0);
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    private void CheckDictionaryResponseMessages(TestReactor consumerReactor, int numOfMessages, int streamId, string itemName)
    {
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(numOfMessages);
        
        TestReactorEvent testEvent;
        RDMDictionaryMsgEvent dictionaryMsgEvent;
        DictionaryRefresh receivedDictionaryRefresh;

        for(int i = 1; i <= numOfMessages; ++i)
        {
            testEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
            dictionaryMsgEvent = (RDMDictionaryMsgEvent) testEvent.ReactorEvent;
            Assert.Equal(DictionaryMsgType.REFRESH, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);
    
            receivedDictionaryRefresh = dictionaryMsgEvent.DictionaryMsg.DictionaryRefresh;
            Assert.Equal(streamId, receivedDictionaryRefresh.StreamId);
            Assert.True(receivedDictionaryRefresh.Solicited); 
            Assert.Equal(itemName, receivedDictionaryRefresh.DictionaryName.ToString());
            Assert.Equal(StreamStates.OPEN, receivedDictionaryRefresh.State.StreamState());
            Assert.Equal(DataStates.OK, receivedDictionaryRefresh.State.DataState());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedDictionaryRefresh.ServiceId);
            Assert.NotNull(dictionaryMsgEvent.StreamInfo);
            Assert.NotNull(dictionaryMsgEvent.StreamInfo.ServiceName);
            Assert.Equal(dictionaryMsgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
            
            if(i == numOfMessages)
            {
                Assert.True(receivedDictionaryRefresh.RefreshComplete);
            }
            else
            {
                if( i == 1)
                {
                    /* Checks clear cache flag for the first message. */
                    Assert.True(receivedDictionaryRefresh.ClearCache); 
                }
                
                Assert.False(receivedDictionaryRefresh.RefreshComplete);
            }
        }
    }

    [Fact]
    public void DownloadDictionaryCompressionTest_ZLIBTest()
    {
        DataDictionary dictionary = new();
        Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary(DICTIONARY_FILE_NAME, out _));
        Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadEnumTypeDictionary(ENUM_TYPE_FILE_NAME, out _));

        ReactorErrorInfo errorInfo = new();
        EncodeIterator eIter = new();
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        RDMDictionaryMsgEvent dictionaryMsgEvent;
        DictionaryRequest dictionaryRequest = new();
        DictionaryRequest receivedDictionaryRequest;
        DictionaryRefresh dictionaryRefresh = new();
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();

        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 20000;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;
        opts.NumOfGuaranteedBuffers = 1000;
        opts.CompressionType = Transports.CompressionType.ZLIB;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends dictionary request. */
        dictionaryRequest.Clear();
        dictionaryRequest.StreamId = 5;
        dictionaryRequest.Streaming = true;
        dictionaryRequest.DictionaryName.Data("RWFEnum");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch(dictionaryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives dictionary request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, testEvent.EventType);
        dictionaryMsgEvent = (RDMDictionaryMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(DictionaryMsgType.REQUEST, dictionaryMsgEvent.DictionaryMsg.DictionaryMsgType);

        receivedDictionaryRequest = dictionaryMsgEvent.DictionaryMsg.DictionaryRequest;
        Assert.True(receivedDictionaryRequest.Streaming);
        Assert.Equal(Provider.DefaultService.ServiceId, receivedDictionaryRequest.ServiceId);
        Assert.Equal("RWFEnum", receivedDictionaryRequest.DictionaryName.ToString());
        Assert.Equal((int)DomainType.DICTIONARY, receivedDictionaryRequest.DomainType);
        
        providerStreamId = receivedDictionaryRequest.StreamId;

        /* Encode fragmented dictionary refresh messages. */
        dictionaryRefresh.Clear();
        dictionaryRefresh.StreamId = providerStreamId;
        dictionaryRefresh.Solicited = true;
        dictionaryRefresh.DictionaryName.Data("RWFEnum");
        dictionaryRefresh.DictionaryType = Types.ENUM_TABLES;
        dictionaryRefresh.Verbosity = VerbosityValues.NORMAL;
        dictionaryRefresh.ServiceId = Provider.DefaultService.ServiceId;
        dictionaryRefresh.State.StreamState(StreamStates.OPEN);
        dictionaryRefresh.State.DataState(DataStates.OK);
        dictionaryRefresh.State.Code(StateCodes.NONE);
        dictionaryRefresh.DataDictionary = dictionary;
        
        bool firstMultiPart = true;
        ITransportBuffer msgBuf = null;
        
        int numOfMessages = 0;
        
        while (true)
        {
            // get a buffer for the dictionary response
            msgBuf = provider.ReactorChannel.GetBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, out errorInfo);
            
            Assert.NotNull(msgBuf);
                    
            dictionaryRefresh.State.Text().Data("Enum Type Dictionary Refresh (starting enum " + dictionaryRefresh.StartEnumTableCount + ")");

            msgBuf.Data.Limit = MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE;
            
            // clear encode iterator
            eIter.Clear();
            eIter.SetBufferAndRWFVersion(msgBuf, provider.ReactorChannel.MajorVersion, 
                provider.ReactorChannel.MinorVersion);
    
            if (firstMultiPart)
            {
                dictionaryRefresh.ClearCache = true;
                firstMultiPart = false;
            }
            else
                dictionaryRefresh.Flags =  DictionaryRefreshFlags.SOLICITED;

            // encode message
            CodecReturnCode ret = dictionaryRefresh.Encode(eIter);
            
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            
            Assert.Equal(ReactorReturnCode.SUCCESS, provider.ReactorChannel.Submit(msgBuf, submitOptions, out errorInfo));
            
            ++numOfMessages;
            
            // break out of loop when all dictionary responses sent
            if (ret == CodecReturnCode.SUCCESS)
            {
                break;
            }
        }
        
        providerReactor.Dispatch(0);
        
        CheckDictionaryResponseMessages(consumerReactor, numOfMessages, 5, "RWFEnum");
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemPauseResumeTest()
    {
       ReactorSubmitOptions submitOptions = new();
       TestReactorEvent testEvent;
       ReactorMsgEvent msgEvent;
       Msg msg = new();
       IRequestMsg requestMsg1 = new Msg();
       IRequestMsg requestMsg2 = new Msg();
       IRequestMsg receivedRequestMsg;
       IRefreshMsg refreshMsg = msg;
       IRefreshMsg receivedRefreshMsg;
       int providerStreamId;
               
       /* Create reactors. */
       TestReactor consumerReactor = new();
       TestReactor providerReactor = new();
               
       /* Create consumer. */
       Consumer consumer = new(consumerReactor);
       ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
       consumerRole.InitDefaultRDMLoginRequest();
       consumerRole.InitDefaultRDMDirectoryRequest();
       consumerRole.ChannelEventCallback = consumer;
       consumerRole.LoginMsgCallback = consumer;
       consumerRole.DirectoryMsgCallback = consumer;
       consumerRole.DictionaryMsgCallback = consumer;
       consumerRole.DefaultMsgCallback = consumer;
       consumerRole.WatchlistOptions.EnableWatchlist = true;
       consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
       
       /* Create provider. */
       Provider provider = new(providerReactor);
       ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
       providerRole.ChannelEventCallback = provider;
       providerRole.LoginMsgCallback = provider;
       providerRole.DirectoryMsgCallback = provider;
       providerRole.DictionaryMsgCallback = provider;
       providerRole.DefaultMsgCallback = provider;

       /* Connect the consumer and provider. Setup login & directory streams automatically. */
       ConsumerProviderSessionOptions opts = new();
       opts.SetupDefaultLoginStream = true;
       opts.SetupDefaultDirectoryStream = true;

       provider.Bind(opts);

       TestReactor.OpenSession(consumer, provider, opts);
       
       // submit two aggregated request messages
       requestMsg1.Clear();
       requestMsg1.MsgClass = MsgClasses.REQUEST;
       requestMsg1.StreamId = 5;
       requestMsg1.DomainType = (int)DomainType.MARKET_PRICE;
       requestMsg1.ApplyStreaming();
       requestMsg1.MsgKey.ApplyHasName();
       requestMsg1.MsgKey.Name.Data("LUV.N");

       requestMsg1.ApplyHasPriority();
       requestMsg1.Priority.Count = 11;
       requestMsg1.Priority.PriorityClass = 22;
       submitOptions.Clear();
       submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
       Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg1, submitOptions) >= ReactorReturnCode.SUCCESS);
       
       requestMsg2.Clear();
       requestMsg2.MsgClass = MsgClasses.REQUEST;
       requestMsg2.StreamId = 6;
       requestMsg2.DomainType = (int)DomainType.MARKET_PRICE;
       requestMsg2.ApplyStreaming();
       requestMsg2.MsgKey.ApplyHasName();
       requestMsg2.MsgKey.Name.Data("LUV.N");

       requestMsg2.ApplyHasPriority();
       requestMsg2.Priority.Count = 10;
       requestMsg2.Priority.PriorityClass = 22;
       submitOptions.Clear();
       submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
       Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg2, submitOptions) >= ReactorReturnCode.SUCCESS);
              
       /* Provider receives requests. */
       providerReactor.Dispatch(2);
       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.Equal(3, receivedRequestMsg.StreamId);
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
       Assert.Equal("LUV.N", receivedRequestMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
       Assert.True(receivedRequestMsg.CheckHasPriority());
       Assert.Equal(11, receivedRequestMsg.Priority.Count);
       Assert.Equal(22, receivedRequestMsg.Priority.PriorityClass);
       
       providerStreamId = receivedRequestMsg.StreamId;
       
       // 2nd request 
       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.Equal(3, receivedRequestMsg.StreamId);
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
       Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
       Assert.Equal("LUV.N", receivedRequestMsg.MsgKey.Name.ToString());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
       Assert.True(receivedRequestMsg.CheckHasPriority());
       Assert.Equal(21, receivedRequestMsg.Priority.Count);
       Assert.Equal(22, receivedRequestMsg.Priority.PriorityClass);
       
       providerStreamId = receivedRequestMsg.StreamId;
                     
       /* Provider sends refresh .*/
       refreshMsg.Clear();
       refreshMsg.MsgClass = MsgClasses.REFRESH;
       refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
       refreshMsg.StreamId = providerStreamId;
       refreshMsg.ContainerType = DataTypes.NO_DATA;
       refreshMsg.ApplyRefreshComplete();
       refreshMsg.ApplyHasMsgKey();
       refreshMsg.MsgKey.ApplyHasServiceId();
       refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
       refreshMsg.MsgKey.ApplyHasName();
       refreshMsg.MsgKey.Name.Data("LUV.N");
       refreshMsg.State.StreamState(StreamStates.OPEN);
       refreshMsg.State.DataState(DataStates.OK);
       refreshMsg.ApplySolicited();

       Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
       
       consumerReactor.Dispatch(2);
       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);
       Assert.NotNull(msgEvent.StreamInfo.ServiceName);
       Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
       
       // 2nd testEvent to consumer
       testEvent = consumerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       
       receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
       Assert.True(receivedRefreshMsg.CheckHasMsgKey());
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
       Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
       Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
       Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
       Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
       Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
       Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
       Assert.NotNull(msgEvent.StreamInfo);
       Assert.NotNull(msgEvent.StreamInfo.ServiceName);
       Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());       
       
       // consumer reissues 2nd request with pause
       requestMsg2.ApplyPause();
       Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg2, submitOptions) >= ReactorReturnCode.SUCCESS);
       
       // consumer reissues 1st request with pause
       requestMsg1.ApplyPause();
       Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg1, submitOptions) >= ReactorReturnCode.SUCCESS);
       
       /* Provider receives pause request. */
       providerReactor.Dispatch(1);
       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming()); 
       // PAUSE
       Assert.True(receivedRequestMsg.CheckPause()); 
       providerStreamId = receivedRequestMsg.StreamId;
       
       // resume
       // consumer reissues 1st request with no pause
       requestMsg1.Flags &= ~RequestMsgFlags.PAUSE;
       Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg1, submitOptions) >= ReactorReturnCode.SUCCESS);
              
       /* Provider receives request. */
       providerReactor.Dispatch(1);
       testEvent = providerReactor.PollEvent();
       Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
       msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
       Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       
       receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
       Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request
       Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
       Assert.True(receivedRequestMsg.CheckStreaming());
       // NO PAUSE now
       Assert.False(receivedRequestMsg.CheckPause()); 
       providerStreamId = receivedRequestMsg.StreamId;
       
       TestReactorComponent.CloseSession(consumer, provider);
       TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestNormalRequestBeforeTest()
    {
        /* Test a simple batch request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testevent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testevent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testevent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Received status message with closed batch stream
        consumerReactor.Dispatch(1);
        testevent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal("DEFAULT_SERVICE", msgEvent.StreamInfo.ServiceName);
        
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
        
        /* Provider receives request. */
        providerReactor.Dispatch(2);
        testevent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        
        // Received Refresh for TRI.N
        testevent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        // Provider processes request for IBM.N
        testevent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        testevent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testevent.EventType);
        msgEvent = (ReactorMsgEvent)testevent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestNormalRequestAfterTest()
    {
        /* Test a simple batch request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Received status message with closed batch stream
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal("DEFAULT_SERVICE", msgEvent.StreamInfo.ServiceName);
        
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
        
        /* Provider receives request. */
        providerReactor.Dispatch(2);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        
        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        // Provider processes request for IBM.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 15;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestOutOfOrderTest()
    {
        /* Test a batch request/refresh exchange with the watchlist enabled, where two requests are sent
         * before both are received. */

        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent testEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;
        string testUserSpecObjOne = "997";
        string testUserSpecObjTwo = "998";

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObjOne;
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer sends second request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 10;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObjTwo;
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Received status messages with closed batch stream
        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal("DEFAULT_SERVICE", msgEvent.StreamInfo.ServiceName);
        Assert.Equal(testUserSpecObjOne, msgEvent.StreamInfo.UserSpec);
        
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
        Assert.Equal(testUserSpecObjOne, msgEvent.StreamInfo.UserSpec);
        
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal("DEFAULT_SERVICE", msgEvent.StreamInfo.ServiceName);
        Assert.Equal(testUserSpecObjTwo, msgEvent.StreamInfo.UserSpec);
        
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
        Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
        Assert.Equal(testUserSpecObjTwo, msgEvent.StreamInfo.UserSpec);
        
        /* Provider receives request. */
        providerReactor.Dispatch(4);
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();


        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(2);
        
        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        // Received Refresh for TRI.N
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        // Provider processes request for IBM.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        // Provider processes request for TRI.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        // Provider processes request for TRI.N
        testEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();


        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(2);
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        testEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, testEvent.EventType);
        msgEvent = (ReactorMsgEvent)testEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        // Received refresh message with item IBM.N (again)
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestOverlappingStreamsTest()
    {
        /* Test a batch request where a stream we would create overlaps an already created stream, and fails. */
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent eventTest;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();
                
        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
        
        /* Consumer sends request on stream id 6. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("ABC.D");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        eventTest = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, eventTest.EventType);
        msgEvent = (ReactorMsgEvent)eventTest.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("ABC.D", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        
        providerStreamId = receivedRequestMsg.StreamId;
        
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.ApplySolicited();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("ABC.D");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        eventTest = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, eventTest.EventType);
        msgEvent = (ReactorMsgEvent)eventTest.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("ABC.D", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);
        
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        
        ReactorErrorInfo errorInfo = new();
        
        Assert.True(consumer.ReactorChannel.Submit((Msg)requestMsg, submitOptions, out errorInfo) == ReactorReturnCode.FAILURE);
        Assert.Equal("Item in batch has same ID as existing stream.", errorInfo.Error.Text);
        
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestServiceIdAndServiceNameTest()
    {
        /* Test a batch request where we set the serviceId on the request, as well as serviceName in watchlist, to fail. */

        ReactorSubmitOptions submitOptions = new();
        Msg msg = new();
        IRequestMsg requestMsg = msg;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();
        requestMsg.ApplyMsgKeyInUpdates();
        requestMsg.MsgKey.ApplyHasServiceId();
        requestMsg.MsgKey.ServiceId = 10;

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();

        ReactorErrorInfo errorInfo = new();

        Assert.True(consumer.ReactorChannel.Submit((Msg)requestMsg, submitOptions, out errorInfo) == ReactorReturnCode.INVALID_USAGE);
        Assert.Equal("Cannot submit request with both service name and service id specified.", errorInfo.Error.Text);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void BatchRequestMsgKeyItemNameTest()
    {
        /* Test a batch request where MsgKey ItemName is set on the request, which should fail */

        ReactorSubmitOptions submitOptions = new();
        Msg msg = new();
        IRequestMsg requestMsg = msg;

        /* Create reactors. */
        TestReactor consumerReactor = new();
        TestReactor providerReactor = new();

        /* Create consumer. */
        Consumer consumer = new(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();
        requestMsg.ApplyMsgKeyInUpdates();
        requestMsg.MsgKey.ApplyHasName();
        Buffer itemName = new();
        itemName.Data("BAD");
        requestMsg.MsgKey.Name = itemName;

        List<string> batchList = new List<string>() { "TRI.N", "IBM.N" };

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, batchList, null);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();

        ReactorErrorInfo errorInfo = new();

        Assert.True(consumer.ReactorChannel.Submit((Msg)requestMsg, submitOptions, out errorInfo) == ReactorReturnCode.FAILURE);
        Assert.Equal("Requested batch has name in message key.", errorInfo.Error.Text);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void CloseConsumerChannelTest()
    {
        /* Test opening some streams and then closing the channel from the consumer side. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;


        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N",receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Close consumer channel. */
        consumer.Close();

        /* Provider receives channel-down evt. */
        providerReactor.Dispatch(1);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    class SendItemsFromDefaultMsgCallbackConsumer : Consumer
    {
        public SendItemsFromDefaultMsgCallbackConsumer(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            base.DefaultMsgCallback(evt);

            if (evt.Msg.MsgClass == MsgClasses.REFRESH && evt.Msg.StreamId == 5)
            {
                // sending snapshot request which has been open, then closed before
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 7;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                //sending realtime request
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 8;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("CallbackItem2");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void SendItemsFromDefaultMsgCallbackConsumerTest()
    {

        /* Opening two items, one snapshot, another one realtime, 
         * and resend one same item, one diff item during the callback. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SendItemsFromDefaultMsgCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 25000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends snapshot request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives snapshot request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        int providerStreamId1 = receivedRequestMsg.StreamId;

        /* Consumer sends streaming request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("CallbackItem1");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem1", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        int providerStreamId2 = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId1;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider receives request from callback. */
        providerReactor.Dispatch(2);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        int providerStreamId3 = receivedRequestMsg.StreamId;
        Assert.False(providerStreamId1 == providerStreamId3);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem2", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        int providerStreamId4 = receivedRequestMsg.StreamId;

        /* Provider sends refresh for "CallbackItem1".*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId2;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("CallbackItem1");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId1 = new Buffer();
        groupId1.Data("1234431");
        refreshMsg.GroupId = groupId1;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem1", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends refresh for "TRI.N" .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId3;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        groupId1 = new Buffer();
        groupId1.Data("1234431");
        refreshMsg.GroupId = groupId1;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(7, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());


        /* Provider sends refresh for "CallbackItem2" .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId4;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("CallbackItem2");
        refreshMsg.ApplyRefreshComplete();
        groupId1 = new Buffer();
        groupId1.Data("1234431");
        refreshMsg.GroupId = groupId1;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(8, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem2" , receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    class SendItemsFromDefaultMsgCallbackConsumer1 : Consumer
    {
        public SendItemsFromDefaultMsgCallbackConsumer1(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            base.DefaultMsgCallback(evt);

            if (evt.Msg.MsgClass == MsgClasses.REFRESH && evt.Msg.StreamId == 5)
            {
                //sending snapshot request which has been open, still waiting for refresh or response
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 7;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("CallbackItem1");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void SendItemsFromDefaultMsgCallbackConsumer1Test()
    {

        /* Opening two items, one snapshot, another one realtime, 
         * and resend one same item which is waiting for refresh msg in watchlist. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SendItemsFromDefaultMsgCallbackConsumer1(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends snapshot request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives snapshot request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.False(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        int providerStreamId1 = receivedRequestMsg.StreamId;

        /* Consumer sends streaming request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("CallbackItem1");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem1", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        int providerStreamId2 = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId1;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new Buffer();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends refresh for "CallbackItem1".*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId2;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("CallbackItem1");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId1 = new Buffer();
        groupId1.Data("1234431");
        refreshMsg.GroupId = groupId1;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(2);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(6, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem1", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(7, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("CallbackItem1", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Consumer that reissues TRI on stream 5 with priority to 1,2 (this priority will be used
     * for the item when the watchlist recovers it). 
     * Also submits a PostMsg and GenericMsg on stream 5, which will fail because there is no longer a stream to the provider. */
    class SubmitOnDisconnectConsumer : Consumer
    {
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IPostMsg postMsg = (IPostMsg)new Msg();
        IGenericMsg genericMsg = (IGenericMsg)new Msg();
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        ReactorErrorInfo errorInfo = new ReactorErrorInfo();

        public SubmitOnDisconnectConsumer(TestReactor testReactor) : base(testReactor) { }

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            if (evt.EventType == ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING)
            {
                /* Re-request TRI. Set priority to 1,2. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                requestMsg.ApplyHasPriority();
                requestMsg.Priority.PriorityClass = 1;
                requestMsg.Priority.Count = 2;
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Submit a PostMsg. This should fail since the stream to the provider is gone. */
                postMsg.Clear();
                postMsg.MsgClass = MsgClasses.POST;
                postMsg.StreamId = 5;
                postMsg.DomainType = (int)DomainType.MARKET_PRICE;
                postMsg.ContainerType = DataTypes.NO_DATA;
                postMsg.ApplyPostComplete();
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.Equal(ReactorReturnCode.INVALID_USAGE, ReactorChannel.Submit((Msg)postMsg, submitOptions, out errorInfo));

                /* Submit a GenericMsg. This should fail since the stream to the provider is gone. */
                genericMsg.Clear();
                genericMsg.MsgClass = MsgClasses.GENERIC;
                genericMsg.StreamId = 5;
                genericMsg.DomainType = (int)DomainType.MARKET_PRICE;
                genericMsg.ContainerType = DataTypes.NO_DATA;
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.Equal(ReactorReturnCode.INVALID_USAGE, ReactorChannel.Submit((Msg)genericMsg, submitOptions, out errorInfo));

            }

            return base.ReactorChannelEventCallback(evt);
        }
    }

    [Fact]
    public void ChannelDownCallbackSubmitTest()
    {

        /* Test submitting messages when the channel goes down. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SubmitOnDisconnectConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3150;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer requests TRI. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request for TRI. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh. */
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N" , receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider disconnects. */
        provider.CloseChannel();

        /* Consumer receives channel-down evt, login status, directory update, and TRI status. */
        consumerReactor.Dispatch(4);
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

        RDMLoginMsgEvent loginMsgEvent;
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
        Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);

        RDMDirectoryMsgEvent directoryMsgEvent;
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.Equal(5, receivedStatusMsg.StreamId);



        /* Reconnect and reestablish login/directory streams. */
        TestReactor.OpenSession(consumer, provider, opts, true);

        /* Provider receives request for TRI, priority 1,2 (changed by consumer in callback). */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(2, receivedRequestMsg.Priority.Count);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    /* Used by StreamReopenTest. 
     * Receives a close for TRI, then opens IBM on the same stream. */
    class StreamReopenConsumer : Consumer
    {
        public StreamReopenConsumer(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            base.DefaultMsgCallback(evt);

            if (evt.Msg.MsgClass == MsgClasses.STATUS)
            {
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("IBM.N");
                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    private void StreamReopenTest(bool singleOpen)
    {
        /* Test reusing an item stream to open another item inside a callback (i.E. the item stream is closed, and another item
         * is requested using the same streamId while inside the callback) . */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IStatusMsg statusMsg = (IStatusMsg)new Msg();
        IStatusMsg receivedStatusMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new StreamReopenConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();

        if (!singleOpen)
        {
            consumerRole.RdmLoginRequest.LoginAttrib.HasSingleOpen = true;
            consumerRole.RdmLoginRequest.LoginAttrib.SingleOpen = 0;
            consumerRole.RdmLoginRequest.LoginAttrib.HasAllowSuspectData = true;
            consumerRole.RdmLoginRequest.LoginAttrib.AllowSuspectData = 0;
        }

        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        provider.Bind(opts);
        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request for TRI. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N" , receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends status msg .*/
        statusMsg.Clear();
        statusMsg.MsgClass = MsgClasses.STATUS;
        statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
        statusMsg.StreamId = providerStreamId;
        statusMsg.ContainerType = DataTypes.NO_DATA;
        statusMsg.ApplyHasState();
        statusMsg.State.StreamState(StreamStates.CLOSED);
        statusMsg.State.DataState(DataStates.SUSPECT);
        statusMsg.ApplyHasMsgKey();
        statusMsg.MsgKey.ApplyHasServiceId();
        statusMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        statusMsg.MsgKey.ApplyHasName();
        statusMsg.MsgKey.Name.Data("TRI.N");

        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives status msg. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.True(receivedStatusMsg.CheckHasMsgKey());
        Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedStatusMsg.MsgKey.ServiceId);
        Assert.True(receivedStatusMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedStatusMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());


        /* Provider receives request for IBM. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyRefreshComplete();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.CheckSolicited());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void StreamReopenTest_SingleOpenOn()
    {
        StreamReopenTest(true);
    }

    [Fact]
    public void StreamReopenTest_SingleOpenOff()
    {
        StreamReopenTest(false);
    }

    private void OpenWindowReconnectTest(bool singleOpen)
    {
        /* Test that an item waiting on the OpenWindow is recovered after a disconnect. 
         * - Start a session where the Provider's Service's OpenWindow is 1.
         * - Send a request for two items. The second item is left waiting on the open window.
         * - Disconnect the provider and ensure both items receive status messages.
         * - Reconnect and ensure both items are recovered if singleOpen is enabled. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;

        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        if (!singleOpen)
        {
            consumerRole.RdmLoginRequest.LoginAttrib.HasSingleOpen = true;
            consumerRole.RdmLoginRequest.LoginAttrib.SingleOpen = 0;
            consumerRole.RdmLoginRequest.LoginAttrib.HasAllowSuspectData = true;
            consumerRole.RdmLoginRequest.LoginAttrib.AllowSuspectData = 0;
        }

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;
        opts.OpenWindow = 1;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Request TRI. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Request IBM. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("IBM.N");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Request GOOG. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 7;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("GOOG.O");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider does not receive this request since TRI & GOOG are in the OpenWindow. */
        providerReactor.Dispatch(0);

        /* Disconnect provider. */
        provider.CloseChannel();

        /* Consumer receives channel event, Login Status, Directory Update, and status for the item streams. */
        consumerReactor.Dispatch(6);
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

        RDMLoginMsgEvent loginMsgEvent;
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
        Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);

        RDMDirectoryMsgEvent directoryMsgEvent;
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId); Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(6, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(7, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.True(receivedStatusMsg.CheckHasState());
        Assert.Equal(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Reconnect and reestablish login/directory streams. */
        TestReactor.OpenSession(consumer, provider, opts, true);

        if (singleOpen)
        {
            /* Provider receives TRI request again. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(5, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.True(receivedRefreshMsg.CheckRefreshComplete());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N",receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Provider receives IBM request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("IBM.N",receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("IBM.N");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(6, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.True(receivedRefreshMsg.CheckRefreshComplete());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* Provider receives GOOG request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("GOOG.O", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("GOOG.O");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal(7, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.True(receivedRefreshMsg.CheckRefreshComplete());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("GOOG.O", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        }
        else
        {
            /* Provider receives nothing (no items recovered). */
            providerReactor.Dispatch(0);
        }

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void OpenWindowReconnectTest_SingleOpenOn()
    {
        OpenWindowReconnectTest(true);
    }

    [Fact]
    public void OpenWindowReconnectTest_SingleOpenOff()
    {
        OpenWindowReconnectTest(false);
    }

    [Fact]
    public void ConsumerLoginClose()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        ICloseMsg closeMsg = new Msg();
        RDMLoginMsgEvent loginMsgEvent;
        LoginClose receivedLoginClose;
        IUpdateMsg updateMsg = new Msg();
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;

        provider.Bind(opts);
        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplySolicited();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer closes login stream. */
        closeMsg.Clear();
        closeMsg.MsgClass = MsgClasses.CLOSE;
        closeMsg.StreamId = consumer.DefaultSessionLoginStreamId;
        closeMsg.DomainType = (int)DomainType.LOGIN;
        Assert.True(consumer.Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives status for TRI. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());

        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // Consumer receives Directory Update message to delete service
        RDMDirectoryMsgEvent directoryMsgEvent;
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

        /* Provider receives login close. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;

        loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
        Assert.Equal(LoginMsgType.CLOSE, loginMsgEvent.LoginMsg.LoginMsgType);

        receivedLoginClose = loginMsgEvent.LoginMsg.LoginClose;
        Assert.Equal(provider.DefaultSessionLoginStreamId, receivedLoginClose.StreamId);

        /* Provider sends an update. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer does not receive it. */
        consumerReactor.Dispatch(0);

        /* Provider receives close for the update. */
        providerReactor.Dispatch(0);

        /* Disconnect provider. */
        provider.CloseChannel();

        /* Consumer receives Channel Event, nothing else (only item is closed). */
        consumerReactor.Dispatch(1);
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    class PostFromDefaultMsgCallbackConsumer : Consumer
    {
       public PostFromDefaultMsgCallbackConsumer(TestReactor testReactor):
            base(testReactor)
       {
       }

       public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
       {
           base.DefaultMsgCallback(evt);
           
           IMsg msg = evt.Msg;
           
           switch (msg.MsgClass)
           {
               case MsgClasses.REFRESH:
                    // send post message
                    IPostMsg postMsg = new Msg();
                    postMsg.Clear();
                    postMsg.MsgClass = MsgClasses.POST;
                    postMsg.StreamId = msg.StreamId;
                    postMsg.DomainType = msg.DomainType;
                    postMsg.ContainerType = DataTypes.NO_DATA;
                    postMsg.ApplyHasMsgKey();
                    postMsg.MsgKey.ApplyHasName();
                    postMsg.MsgKey.Name.Data("TRI.N");

                    ReactorSubmitOptions submitOptions = new();
                    submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                    if (evt.ReactorChannel.Submit((Msg)postMsg, submitOptions, out _) !=  ReactorReturnCode.SUCCESS)
                    {
                        Assert.Fail("DefaultMsgCallback() submit post failed");
                    }
                    break;
               default:
                   break;
           }
           
           return ReactorCallbackReturnCode.SUCCESS;
       }
   }

    [Fact]
    public void SubmitPostOnItemRefeshTest()
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
               
        /* Create reactors. */
        TestReactor consumerReactor = new ();
        TestReactor providerReactor = new ();
               
        /* Create consumer. */
        Consumer consumer = new PostFromDefaultMsgCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
       
        /* Create provider. */
        Provider provider = new (providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);
       
        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
       
        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
       
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N",receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(1, receivedRequestMsg.Priority.Count);
       
        providerStreamId = receivedRequestMsg.StreamId;
       
        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Buffer groupId = new();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();


        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
       
        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
       
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
              
        /* Provider receives post. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
        IPostMsg receivedPostMsg = (IPostMsg)msgEvent.Msg;
        Assert.True(receivedPostMsg.CheckHasMsgKey());
        Assert.True(receivedPostMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedPostMsg.MsgKey.ServiceId);
        Assert.True(receivedPostMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedPostMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
        Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void ItemMultipartRefreshTimeoutTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // wait for multi-part request timeout
        try
        {
            Thread.Sleep(5000);
        }
        catch { }
        // dispatch for timeout
        consumerReactor.Dispatch(1);

        /* Provider receives close and re-request. */
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        /* Consumer receives status message for timeout. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        IStatusMsg statusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.True(statusMsg.CheckHasState());
        Assert.Equal("Request timeout", statusMsg.State.Text().ToString());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void StreamingAggregationCloseReuseStreamTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
        int testUserSpecObj = 997;

        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends streaming request for TRI1, TRI2, TRI3 */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI1");
        requestMsg.ApplyStreaming();
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 6;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI2");
        requestMsg.ApplyStreaming();
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 7;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI3");
        requestMsg.ApplyStreaming();
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(3);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI1",receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        providerStreamId = receivedRequestMsg.StreamId;

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI2", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI3", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI1");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplySolicited();
        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh */
        consumerReactor.Dispatch(1);

        /* Streaming refresh. */
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal(5, receivedRefreshMsg.StreamId);
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI1", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());
        Assert.NotNull(msgEvent.StreamInfo.UserSpec);
        Assert.Equal(testUserSpecObj, msgEvent.StreamInfo.UserSpec);

        /* Consumer sends TRI1 close */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.CLOSE;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer sends TRI2 streaming reusing TRI1 stream */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI2");
        requestMsg.ApplyStreaming();
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(2);

        evt = providerReactor.PollEvent(); // Skip close message

        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.True(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI2", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }

    [Fact]
    public void RedirectedTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg statusMsg = new Msg();
        IStatusMsg receivedStatusMsg;
        int providerStreamId;

        for (int i = 0; i < 2; ++i)
        {
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            bool applyMsgKeyInUpdates = i == 0;

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* -- Redirecting StatusMsg -- */

            /* Consumer sends request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            if (applyMsgKeyInUpdates)
                requestMsg.ApplyMsgKeyInUpdates();
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends redirecting StatusMsg .*/
            statusMsg.Clear();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.StreamId = providerStreamId;
            statusMsg.ContainerType = DataTypes.NO_DATA;

            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.REDIRECTED);
            statusMsg.State.DataState(DataStates.OK);

            statusMsg.ApplyHasMsgKey();
            statusMsg.MsgKey.ApplyHasServiceId();
            statusMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            statusMsg.MsgKey.ApplyHasName();
            statusMsg.MsgKey.Name.Data("RTRSY.O");

            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives redirecting status with new key. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

            receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatusMsg.CheckHasMsgKey());
            Assert.True(receivedStatusMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedStatusMsg.MsgKey.ServiceId);
            Assert.True(receivedStatusMsg.MsgKey.CheckHasName());
            Assert.Equal("RTRSY.O", receivedStatusMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.REDIRECTED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            /* -- Redirecting RefreshMsg -- */

            /* Consumer sends request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            if (applyMsgKeyInUpdates)
                requestMsg.ApplyMsgKeyInUpdates();
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends redirecting RefreshMsg .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;

            refreshMsg.State.StreamState(StreamStates.REDIRECTED);
            refreshMsg.State.DataState(DataStates.OK);

            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("RTRSY.O");
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives redirecting refresh with new key. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("RTRSY.O", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.REDIRECTED, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
        }
    }

    [Fact]
    public void ItemRequestMultipleTimeoutTestWithServiceUpdate()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = msg;
        IRequestMsg receivedRequestMsg;

        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        consumerRole.WatchlistOptions.EnableWatchlist = true;
        consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
        consumerRole.WatchlistOptions.RequestTimeout = 3000;

        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Provider waits for request timeout. */
        try
        {
            Thread.Sleep(5000);
        }
        catch
        {
            Assert.False(true);
        }
        /* Consumer dispatches timeout and gets status message. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        IStatusMsg statusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.True(statusMsg.CheckHasState());
        Assert.Equal("Request timeout", statusMsg.State.Text().ToString());

        /* Provider receives close and request. */
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Provider waits for request timeout. */
        try
        {
            Thread.Sleep(5000);
        }
        catch
        {
            Assert.False(true);
        }
        /* Consumer dispatches timeout and gets status message. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        statusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.True(statusMsg.CheckHasState());
        Assert.Equal("Request timeout", statusMsg.State.Text().ToString());

        /* Provider receives close and request. */
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Provider sends service update with ServiceState of 0.*/
        DirectoryUpdate directoryUpdateMsg = new DirectoryUpdate();
        directoryUpdateMsg.Clear();
        directoryUpdateMsg.StreamId = 2;
        directoryUpdateMsg.HasFilter = true;
        directoryUpdateMsg.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;

        WlService wlService = new WlService();
        wlService.RdmService = new Service();
        wlService.RdmService.HasState = true;
        wlService.RdmService.Action = MapEntryActions.UPDATE;
        wlService.RdmService.State.HasAcceptingRequests = true;
        wlService.RdmService.State.AcceptingRequests = 1;
        wlService.RdmService.State.ServiceStateVal = 0;
        wlService.RdmService.State.HasStatus = true;
        wlService.RdmService.State.Status.DataState(DataStates.SUSPECT);
        wlService.RdmService.State.Status.StreamState(StreamStates.OPEN);
        wlService.RdmService.ServiceId = 1;

        directoryUpdateMsg.ServiceList.Add(wlService.RdmService);

        Assert.True(provider.SubmitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives update and status message. */
        consumerReactor.Dispatch(2);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, directoryMsgEvent.Msg.MsgClass);
        DirectoryUpdate receivedUpdateMsg = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
        Assert.True(receivedUpdateMsg.HasFilter);
        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.Filter);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor, consumer, provider);
    }
}