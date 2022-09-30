using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Rdm;
using System;
using System.Collections.Generic;
using Xunit;
using Xunit.Categories;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.Tests
{

    [Category("XMLDecoders")]
    public class XMLDecodersTests
    {
        int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST, DataTypes.FILTER_LIST, DataTypes.SERIES, DataTypes.VECTOR, DataTypes.MAP };
        int[] domainTypes = { (int)DomainType.LOGIN, (int)DomainType.SOURCE, (int)DomainType.DICTIONARY, (int)DomainType.MARKET_PRICE, (int)DomainType.MARKET_BY_ORDER };
        Boolean[] values = { true, false };

        [Fact]
        public void CanXMLDecodeRequestMsg()
        {
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean streaming in values)
                            foreach (Boolean hasQos in values)
                                foreach (Boolean noRefresh in values)
                                    foreach (Boolean hasPriority in values)
                                        foreach (Boolean privateStream in values)
                                            foreach (Boolean pause in values)
                                                foreach (Boolean hasMsgKeyType in values)
                                                    foreach (Boolean hasMsgKeyInUpdates in values)
                                                        foreach (Boolean hasConfInfoInUpdates in values)
                                                            foreach (Boolean hasWorstQos in values)
                                                                foreach (Boolean qualifiedStream in values)
                                                                {
                                                                    MsgParameters msgParameters = new MsgParameters()
                                                                    {
                                                                        MsgClass = MsgClasses.REQUEST,
                                                                        MsgDomainType = dt,
                                                                        StreamId = 1,
                                                                        HasExtendedHeader = hasExtHeader,
                                                                        HasMsgKey = true,
                                                                        PayloadType = pt,
                                                                        Streaming = streaming,
                                                                        HasQos = hasQos,
                                                                        Pause = pause,
                                                                        NoRefresh = noRefresh,
                                                                        HasPriority = hasPriority,
                                                                        PrivateStream = privateStream,
                                                                        HasMsgKeyType = hasMsgKeyType,
                                                                        HasMsgKeyInUpdates = hasMsgKeyInUpdates,
                                                                        HasConfInfoInUpdates = hasConfInfoInUpdates,
                                                                        HasWorstQos = hasWorstQos,
                                                                        QualifiedStream = qualifiedStream
                                                                    };
                                                                    EncodeIterator encIter = new EncodeIterator();
                                                                    Buffer buffer = new Buffer();
                                                                    buffer.Data(new ByteBuffer(10000));
                                                                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                                    CodecTestUtil.EncodeMessage(encIter, msgParameters);
                                                                    CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                                                }
        }

        [Fact]
        public void CanXMLDecodeRefreshMsg()
        {
            int[] domainTypes = { (int)DomainType.LOGIN, (int)DomainType.DICTIONARY, (int)DomainType.MARKET_BY_ORDER };
            int[] payloadTypes = { DataTypes.FIELD_LIST, DataTypes.SERIES, DataTypes.MAP };
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean hasMsgKey in values)
                            foreach (Boolean hasPermData in values)
                                foreach (Boolean doNotCache in values)
                                    foreach (Boolean refreshComplete in values)
                                        foreach (Boolean streaming in values)
                                            foreach (Boolean hasQos in values)
                                                foreach (Boolean noRefresh in values)
                                                    foreach (Boolean hasPriority in values)
                                                        foreach (Boolean solicited in values)
                                                            foreach (Boolean clearCache in values)
                                                                foreach (Boolean hasPostUserInfo in values)
                                                                    foreach (Boolean hasPartNum in values)
                                                                    {
                                                                        MsgParameters msgParameters = new MsgParameters()
                                                                        {
                                                                            MsgClass = MsgClasses.REFRESH,
                                                                            MsgDomainType = dt,
                                                                            StreamId = 1,
                                                                            HasExtendedHeader = hasExtHeader,
                                                                            HasMsgKey = hasMsgKey,
                                                                            HasPermData = hasPermData,
                                                                            PayloadType = pt,
                                                                            DoNotCache = doNotCache,
                                                                            SeqNum = 2,
                                                                            RefreshComplete = refreshComplete,
                                                                            HasQos = hasQos,
                                                                            PrivateStream = true,
                                                                            HasMsgKeyType = true,
                                                                            Solicited = solicited,
                                                                            ClearCache = clearCache,
                                                                            HasPostUserInfo = hasPostUserInfo,
                                                                            HasPartNum = hasPartNum,
                                                                            QualifiedStream = true
                                                                        };
                                                                        EncodeIterator encIter = new EncodeIterator();
                                                                        Buffer buffer = new Buffer();
                                                                        buffer.Data(new ByteBuffer(10000));
                                                                        encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                                        CodecTestUtil.EncodeRefreshMsg(encIter, msgParameters);
                                                                        CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                                                    }

        }

        [Fact]
        public void CanXMLDecodeStatusMsg()
        {
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean hasPermData in values)
                            foreach (Boolean hasMsgKey in values)
                                foreach (Boolean clearCache in values)
                                    foreach (Boolean hasGroupId in values)
                                        foreach (Boolean hasState in values)
                                            foreach (Boolean hasPostUserInfo in values)
                                                foreach (Boolean privateStream in values)
                                            {
                                                MsgParameters msgParameters = new MsgParameters()
                                                {
                                                    MsgClass = MsgClasses.STATUS,
                                                    MsgDomainType = dt,
                                                    StreamId = 1,
                                                    HasExtendedHeader = hasExtHeader,
                                                    HasMsgKey = hasMsgKey,
                                                    HasPermData = hasPermData,
                                                    PayloadType = pt,
                                                    HasState = hasState,
                                                    PrivateStream = privateStream,
                                                    HasGroupId = hasGroupId,
                                                    ClearCache = clearCache,
                                                    HasPostUserInfo = hasPostUserInfo

                                                };
                                                EncodeIterator encIter = new EncodeIterator();
                                                Buffer buffer = new Buffer();
                                                buffer.Data(new ByteBuffer(10000));
                                                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                CodecTestUtil.EncodeMessage(encIter, msgParameters);
                                                CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                            }



        }

        [Fact]
        public void CanXMLDecodeUpdateMsg()
        {
            int[] domainTypes = { (int)DomainType.LOGIN, (int)DomainType.DICTIONARY, (int)DomainType.MARKET_BY_ORDER };
            int[] payloadTypes = { DataTypes.FIELD_LIST, DataTypes.VECTOR, DataTypes.FILTER_LIST };
            int[] seqNums = { 0, 1 };
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean hasMsgKey in values)
                            foreach (Boolean hasPermData in values)
                                foreach (Boolean doNotConflate in values)
                                    foreach (int seqNum in seqNums)
                                        foreach (Boolean hasState in values)
                                            foreach (Boolean privateStream in values)
                                                foreach (Boolean hasPostUserInfo in values)
                                                    foreach (Boolean doNotCache in values)
                                                        foreach (Boolean doNotRipple in values)
                                                            foreach (Boolean hasConfInfo in values)
                                                                foreach (Boolean discardable in values)
                                                                {
                                                                    MsgParameters msgParameters = new MsgParameters()
                                                                    {
                                                                        MsgClass = MsgClasses.UPDATE,
                                                                        MsgDomainType = dt,
                                                                        StreamId = 1,
                                                                        HasExtendedHeader = hasExtHeader,
                                                                        HasMsgKey = hasMsgKey,
                                                                        HasPermData = hasPermData,
                                                                        PayloadType = pt,
                                                                        DoNotCache = doNotCache,
                                                                        DoNotConflate = doNotConflate,
                                                                        DoNotRipple = doNotRipple,
                                                                        SeqNum = seqNum,
                                                                        HasMsgKeyType = true,
                                                                        HasConfInfo = hasConfInfo,
                                                                        Discardable = discardable,
                                                                        HasPostUserInfo = hasPostUserInfo
                                                                    };
                                                                    EncodeIterator encIter = new EncodeIterator();
                                                                    Buffer buffer = new Buffer();
                                                                    buffer.Data(new ByteBuffer(10000));
                                                                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                                    CodecTestUtil.EncodeMessage(encIter, msgParameters);
                                                                    CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                                                }



        }

        [Fact]
        public void CanXMLDecodeAckMsg()
        {
            int[] seqNums = { 0, 1 };
            int[] nackCodes = { 0, 1 };
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean hasMsgKey in values)
                            foreach (Boolean hasText in values)
                                foreach (Boolean privateStream in values)
                                    foreach (int seqNum in seqNums)
                                        foreach (int nackCode in nackCodes)
                                            foreach (Boolean qualifiedStream in values)
                                            {
                                                MsgParameters msgParameters = new MsgParameters()
                                                {
                                                    MsgClass = MsgClasses.ACK,
                                                    MsgDomainType = dt,
                                                    StreamId = 1,
                                                    HasExtendedHeader = hasExtHeader,
                                                    HasMsgKey = hasMsgKey,
                                                    PayloadType = pt,
                                                    SeqNum = seqNum,
                                                    NackCode = nackCode,
                                                    HasText = hasText,
                                                    PrivateStream = privateStream,
                                                    QualifiedStream = qualifiedStream
                                                };
                                                EncodeIterator encIter = new EncodeIterator();
                                                Buffer buffer = new Buffer();
                                                buffer.Data(new ByteBuffer(10000));
                                                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                CodecTestUtil.EncodeAckMsg(encIter, msgParameters);
                                                CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                            }


        }

        [Fact]
        public void CanXMLDecodePostMsg()
        {
            int[] domainTypes = { (int)DomainType.MARKET_BY_PRICE, (int)DomainType.SOURCE, (int)DomainType.DICTIONARY };
            int[] payloadTypes = { DataTypes.MAP, DataTypes.ELEMENT_LIST, DataTypes.FILTER_LIST };
            int[] seqNums = { 0, 1 };
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean hasPermData in values)
                            foreach (Boolean hasMsgKey in values)
                                foreach (Boolean postComplete in values)
                                    foreach (Boolean privateStream in values)
                                        foreach (int seqNum in seqNums)
                                            foreach (Boolean hasPartNum in values)
                                                foreach (Boolean ack in values)
                                                    foreach (Boolean hasPostUserRights in values)
                                                            foreach (Boolean hasMsgKeyType in values)
                                                                foreach (Boolean hasPostId in values)
                                                                {
                                                                    MsgParameters msgParameters = new MsgParameters()
                                                                    {
                                                                        MsgClass = MsgClasses.POST,
                                                                        MsgDomainType = dt,
                                                                        StreamId = 1,
                                                                        HasExtendedHeader = hasExtHeader,
                                                                        HasMsgKey = hasMsgKey,
                                                                        HasPermData = hasPermData,
                                                                        PayloadType = pt,
                                                                        SeqNum = seqNum,
                                                                        Ack = ack,
                                                                        PostComplete = postComplete,
                                                                        HasPostUserRights = hasPostUserRights,
                                                                        PrivateStream = privateStream,
                                                                        HasMsgKeyType = hasMsgKeyType,
                                                                        HasPartNum = hasPartNum,
                                                                        HasPostId = hasPostId
                                                                    };
                                                                    EncodeIterator encIter = new EncodeIterator();
                                                                    Buffer buffer = new Buffer();
                                                                    buffer.Data(new ByteBuffer(10000));
                                                                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                                    CodecTestUtil.EncodePostMsg(encIter, msgParameters);
                                                                    CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                                                }

        }

        [Fact]
        public void CanXMLDecodeGenericMsg()
        {
            int[] seqNums = { 0, 1 };
            int[] domainTypes = { (int)DomainType.LOGIN, (int)DomainType.SOURCE, (int)DomainType.DICTIONARY };
            int[] payloadTypes = { DataTypes.MSG, DataTypes.VECTOR, DataTypes.FILTER_LIST };
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean hasPermData in values)
                            foreach (Boolean hasMsgKey in values)
                                foreach (Boolean msgComplete in values)
                                    foreach (int seqNum in seqNums)
                                        foreach (Boolean hasMsgKeyType in values)
                                            foreach (int secSeqNum in seqNums)
                                            {
                                                MsgParameters msgParameters = new MsgParameters()
                                                {
                                                    MsgClass = MsgClasses.GENERIC,
                                                    MsgDomainType = dt,
                                                    StreamId = 1,
                                                    HasExtendedHeader = hasExtHeader,
                                                    HasMsgKey = hasMsgKey,
                                                    HasPermData = hasPermData,
                                                    PayloadType = pt,
                                                    SeqNum = seqNum,
                                                    HasMsgKeyType = hasMsgKeyType,
                                                    MessageComplete = msgComplete,
                                                    SecondarySeqNum = secSeqNum
                                                };
                                                EncodeIterator encIter = new EncodeIterator();
                                                Buffer buffer = new Buffer();
                                                buffer.Data(new ByteBuffer(10000));
                                                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                                                CodecTestUtil.EncodeMessage(encIter, msgParameters);
                                                CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                                            }


        }

        [Fact]
        public void CanXMLDecodeCloseMsg()
        {
            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (Boolean hasExtHeader in values)
                        foreach (Boolean ack in values)
                        {
                            MsgParameters msgParameters = new MsgParameters()
                            {
                                MsgClass = MsgClasses.CLOSE,
                                MsgDomainType = dt,
                                StreamId = 1,
                                HasExtendedHeader = hasExtHeader,
                                PayloadType = pt,
                                Ack = ack
                            };
                            EncodeIterator encIter = new EncodeIterator();
                            Buffer buffer = new Buffer();
                            buffer.Data(new ByteBuffer(10000));
                            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                            CodecTestUtil.EncodeMessage(encIter, msgParameters);
                            CodecTestUtil.DecodeMsgToXMLAndCheck(buffer, msgParameters);
                        }
        }

        [Fact]
        public void CanXMLDecodeArray()
        {
            int[] dataTypes = { DataTypes.QOS, 
                DataTypes.REAL, 
                DataTypes.INT, DataTypes.UINT, DataTypes.FLOAT, DataTypes.DATETIME, 
                DataTypes.TIME, DataTypes.STATE, DataTypes.DATE, DataTypes.ASCII_STRING, DataTypes.ENUM };

            foreach (var type in dataTypes)
            {
                EncodeIterator encIter = new EncodeIterator();
                Buffer buffer = new Buffer();
                buffer.Data(new ByteBuffer(10000));
                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                CodecTestUtil.EncodeSimpleArray(encIter, type);
                CodecTestUtil.DecodeXMLArrayAndCheck(buffer, type);
            }          
        }

        [Fact]
        public void CanXMLDecodeElementList()
        {
            int[] types1 = { DataTypes.INT, DataTypes.UINT, DataTypes.TIME, DataTypes.DATE, DataTypes.DATETIME, DataTypes.ASCII_STRING,
                DataTypes.VECTOR, DataTypes.MAP };
            int[] types2 = { DataTypes.FIELD_LIST, DataTypes.SERIES, DataTypes.OPAQUE, DataTypes.JSON, DataTypes.MSG, DataTypes.DOUBLE, DataTypes.REAL };
            int[][] types = { types1, types2 };

            foreach (var typeArr in types)
            {
                EncodeIterator encIter = new EncodeIterator();
                Buffer buffer = new Buffer();
                buffer.Data(new ByteBuffer(100000));
                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                CodecTestUtil.EncodeSimpleElementList(encIter, typeArr);
                CodecTestUtil.DecodeXMLElementListAndCheck(buffer, typeArr);
            }
        }

        [Fact]
        public void CanXMLDecodeFieldList()
        {
            EncodeIterator encIter = new EncodeIterator();
            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(10000));
            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            int[] types = { DataTypes.REAL, DataTypes.DATE, DataTypes.TIME, DataTypes.ASCII_STRING, DataTypes.VECTOR, DataTypes.MAP, DataTypes.ELEMENT_LIST };
            CodecTestUtil.EncodeSimpleFieldList(encIter, types);
            CodecTestUtil.DecodeXMLFieldListAndCheck(buffer, types);
        }

        [Fact]
        public void CanXMLDecodeVector()
        {
            int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST, DataTypes.FILTER_LIST, DataTypes.SERIES, DataTypes.MAP, DataTypes.OPAQUE, DataTypes.JSON, DataTypes.MSG };
            Boolean[] values = { true, false };
            VectorEntryActions[] actions = { VectorEntryActions.CLEAR, VectorEntryActions.INSERT, VectorEntryActions.DELETE, VectorEntryActions.UPDATE, VectorEntryActions.SET };
            Boolean[] hasPermData = { false, true, true, false, false };

            foreach (var type in payloadTypes)
                foreach (var hasSummary in values)
                    foreach (var hasTotalCountHint in values)
                        foreach (var supportsSorting in values)
                        {
                            EncodeIterator encIter = new EncodeIterator();
                            Buffer buffer = new Buffer();
                            buffer.Data(new ByteBuffer(10000));
                            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                            CodecTestUtil.EncodeSimpleVector(encIter, type, hasSummary, hasTotalCountHint, supportsSorting, actions, hasPermData);
                            CodecTestUtil.DecodeXMLVectorAndCheck(buffer, type, hasSummary, hasTotalCountHint, supportsSorting, actions, hasPermData);
                        }

        }

        [Fact]
        public void CanXMLDecodeMap()
        {
            int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST, DataTypes.FILTER_LIST, DataTypes.SERIES, DataTypes.VECTOR, DataTypes.OPAQUE, DataTypes.JSON, DataTypes.MSG };
            Boolean[] values = { true, false };
            MapEntryActions[] actions = { MapEntryActions.ADD, MapEntryActions.DELETE, MapEntryActions.UPDATE };
            Boolean[] hasPermData = { false, true, true };
            int[] keyTypes = { DataTypes.QOS, DataTypes.INT, DataTypes.REAL, DataTypes.DATE, DataTypes.TIME, DataTypes.STATE };

            foreach (var type in payloadTypes)
                foreach (var hasSummary in values)
                    foreach (var hasTotalCountHint in values)
                        foreach (var hasKeyFieldId in values)
                            foreach (var keyType in keyTypes)
                        {
                            EncodeIterator encIter = new EncodeIterator();
                            Buffer buffer = new Buffer();
                            buffer.Data(new ByteBuffer(100000));
                            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                            CodecTestUtil.EncodeSimpleMap(encIter, type,
                                           actions,
                                           hasPermData,
                                           keyType,
                                           hasSummary,
                                           hasKeyFieldId,
                                           hasTotalCountHint);
                            CodecTestUtil.DecodeXMLMapAndCheck(buffer, type,
                                           actions,
                                           hasPermData,
                                           keyType,
                                           hasSummary,
                                           hasKeyFieldId,
                                           hasTotalCountHint);
                        }

        }

        [Fact]
        public void CanXMLDecodeSeries()
        {
            int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.MAP, DataTypes.FILTER_LIST, DataTypes.FIELD_LIST, DataTypes.VECTOR, DataTypes.OPAQUE, DataTypes.MSG, DataTypes.JSON };
            Boolean[] values = { true, false };

            foreach (var type in payloadTypes)
                foreach (var hasSummary in values)
                    foreach (var hasTotalCountHint in values)
                    {
                        EncodeIterator encIter = new EncodeIterator();
                        Buffer buffer = new Buffer();
                        buffer.Data(new ByteBuffer(80000));
                        encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                        CodecTestUtil.EncodeSimpleSeries(encIter, type, hasTotalCountHint, hasSummary, 4, 3);
                        CodecTestUtil.DecodeXMLSeriesAndCheck(buffer, type, hasTotalCountHint, hasSummary, 4, 3);
                    }
        }

        [Fact]
        public void CanXMLDecodeFilterList()
        {
            int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST, DataTypes.SERIES, DataTypes.VECTOR, DataTypes.MAP };
            FilterEntryActions[] filterActions = { FilterEntryActions.SET, FilterEntryActions.CLEAR, FilterEntryActions.UPDATE };
            int[] dataTypes1 = { DataTypes.ELEMENT_LIST, DataTypes.MAP, DataTypes.MAP };
            int[] dataTypes2 = { DataTypes.FIELD_LIST, DataTypes.VECTOR, DataTypes.MSG };
            int[] dataTypes3 = { DataTypes.JSON, DataTypes.OPAQUE, DataTypes.MSG };
            int[] dataTypes4 = { DataTypes.JSON, DataTypes.MSG, DataTypes.SERIES };
            int[][] dataTypes = { dataTypes1, dataTypes2, dataTypes3, dataTypes4 };
            Boolean[] hasPermData = { true, false, true };
            Boolean[] values = { true, false };

            foreach (var type in payloadTypes)
                foreach (var hasSummary in values)
                    foreach (var hasTotalCountHint in values)
                        foreach (var dataType in dataTypes)
                        {
                            EncodeIterator encIter = new EncodeIterator();
                            Buffer buffer = new Buffer();
                            buffer.Data(new ByteBuffer(80000));
                            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                            CodecTestUtil.EncodeSimpleFilterList(encIter, type, hasTotalCountHint, filterActions, dataType, hasPermData, 5);
                            CodecTestUtil.DecodeXMLFilterListAndCheck(buffer, type, hasTotalCountHint, filterActions, dataType, hasPermData, 5);
                        }
        }

    }
    
}
