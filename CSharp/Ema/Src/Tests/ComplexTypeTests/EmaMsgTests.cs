/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Tests;
using System;
using System.Collections.Generic;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access.Tests
{
    public class EmaMsgTests
    {
        bool[] values = { true, false };
        int[] msgDataTypes = {(int)DataType.DataTypes.UPDATE_MSG, (int)DataType.DataTypes.REFRESH_MSG, (int)DataType.DataTypes.STATUS_MSG,
            (int)DataType.DataTypes.REQ_MSG, (int)DataType.DataTypes.ACK_MSG, (int)DataType.DataTypes.GENERIC_MSG, (int)DataType.DataTypes.POST_MSG };

        private EmaObjectManager m_objectManager = new EmaObjectManager();

        private void LoadEnumTypeDictionary(DataDictionary dataDictionary)
        {
            if (dataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def", out CodecError error) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary. Error Text: {error.Text}");
                Assert.True(false);
            }
        }

        private void LoadFieldDictionary(DataDictionary dataDictionary)
        {
            if (dataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary", out CodecError error) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary. Error Text: {error.Text}");
                Assert.True(false);
            }
        }

        [Fact]
        public void EncodeAndDecodeRequestMsg()
        {
            DataDictionary dictionary = new DataDictionary();
            
            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.LOGIN, (int)DomainType.MARKET_PRICE };
            int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.FILTER_LIST, DataTypes.MAP };
            int[] attribTypes = { DataTypes.FIELD_LIST, DataTypes.VECTOR };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in values)
                        foreach (bool streaming in values)
                            foreach (bool hasQos in values)
                                foreach (bool noRefresh in values)
                                    foreach (bool hasPriority in values)
                                        foreach (bool privateStream in values)
                                            foreach (bool pause in values)
                                                foreach (bool hasMsgKeyType in values)
                                                    foreach (bool hasFilter in values)
                                                        foreach (bool hasId in values)
                                                            foreach (bool hasAttrib in values)
                                                                foreach (int attribType in attribTypes)
                                                                {
                                                                    MsgParameters msgParameters = new MsgParameters()
                                                                    {
                                                                        MsgClass = MsgClasses.REQUEST,
                                                                        MsgDomainType = dt,
                                                                        StreamId = 1,
                                                                        HasExtendedHeader = hasExtHeader,
                                                                        HasMsgKey = true,
                                                                        HasPayload = true,
                                                                        PayloadType = pt,
                                                                        ContainerType = pt,
                                                                        Streaming = streaming,
                                                                        HasQos = hasQos,
                                                                        Pause = pause,
                                                                        NoRefresh = noRefresh,
                                                                        HasPriority = hasPriority,
                                                                        PrivateStream = privateStream,
                                                                        HasMsgKeyType = hasMsgKeyType,
                                                                        HasFilter = hasFilter,
                                                                        HasAttrib = hasAttrib,
                                                                        AttribContainerType = attribType,
                                                                        HasIdentifier = hasId
                                                                    };

                                                                    RequestMsg reqMsg = m_objectManager.GetOmmRequestMsg();
                                                                    EmaComplexTypeHandler.EncodeRequestMessage(msgParameters, reqMsg);

                                                                    var body = reqMsg.m_requestMsgEncoder.m_encodeIterator!.Buffer();

                                                                    RequestMsg decodeMsg = m_objectManager.GetOmmRequestMsg();
                                                                    EmaComplexTypeHandler.DecodeAndCheckRequestMessage(msgParameters, decodeMsg, body, dictionary);

                                                                    reqMsg.ClearAndReturnToPool_All();
                                                                    decodeMsg.ClearAndReturnToPool_All();
                                                                }
            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeRefreshMsg()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.OPAQUE, DataTypes.MAP };
            int[] attribTypes = { DataTypes.VECTOR, DataTypes.SERIES };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasQos in values)
                        foreach (bool privateStream in values)
                            foreach (bool hasSeqNum in values)
                                foreach (bool hasPartNum in values)
                                    foreach (bool doNotCache in values)
                                        foreach (bool clearCache in values)
                                            foreach (bool solicited in values)
                                                foreach (bool hasPublisherId in values)
                                                    foreach (bool hasGroupId in values)
                                                        foreach (bool msgComplete in values)
                                                            foreach (bool hasMsgKey in values)
                                                                foreach (bool hasAttrib in values)
                                                                    foreach (int attribType in attribTypes)
                                                                        foreach (bool preenc in values)
                                                                        {
                                                                            MsgParameters msgParameters = new MsgParameters()
                                                                            {
                                                                                MsgClass = MsgClasses.REFRESH,
                                                                                MsgDomainType = dt,
                                                                                StreamId = 1,
                                                                                HasExtendedHeader = true,
                                                                                HasMsgKey = hasMsgKey,
                                                                                HasPayload = true,
                                                                                PayloadType = pt,
                                                                                ContainerType = pt,
                                                                                HasQos = hasQos,
                                                                                PrivateStream = privateStream,
                                                                                HasMsgKeyType = true,
                                                                                HasFilter = false,
                                                                                HasAttrib = hasAttrib,
                                                                                AttribContainerType = attribType,
                                                                                HasIdentifier = true,
                                                                                HasState = true,
                                                                                ClearCache = clearCache,
                                                                                DoNotCache = doNotCache,
                                                                                HasGroupId = hasGroupId,
                                                                                MessageComplete = msgComplete,
                                                                                HasPublisherId = hasPublisherId,
                                                                                HasPartNum = hasPartNum,
                                                                                Solicited = solicited,
                                                                                HasSeqNum = hasSeqNum,
                                                                                Preencoded = preenc
                                                                            };

                                                                            RefreshMsg refMsg = m_objectManager.GetOmmRefreshMsg();
                                                                            EmaComplexTypeHandler.EncodeRefreshMessage(msgParameters, refMsg);

                                                                            var body = refMsg.m_refreshMsgEncoder.m_encodeIterator!.Buffer();

                                                                            RefreshMsg decodeMsg = m_objectManager.GetOmmRefreshMsg();
                                                                            EmaComplexTypeHandler.DecodeAndCheckRefreshMessage(msgParameters, decodeMsg, body, dictionary);

                                                                            refMsg.ClearAndReturnToPool_All();
                                                                            decodeMsg.ClearAndReturnToPool_All();                                                                            
                                                                        }
            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeUpdateMsg()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.MARKET_PRICE, (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.XML, DataTypes.ELEMENT_LIST };
            int[] attribTypes = { DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in values)
                        foreach (bool hasPermData in values)
                            foreach (bool hasSeqNum in values)
                                foreach (bool hasConflated in values)
                                    foreach (bool doNotCache in values)
                                        foreach (bool doNotConflate in values)
                                            foreach (bool doNotRipple in values)
                                                foreach (bool hasPublisherId in values)
                                                    foreach (bool hasMsgKey in values)
                                                        foreach (bool hasAttrib in values)
                                                            foreach (int attribType in attribTypes)
                                                            {
                                                                MsgParameters msgParameters = new MsgParameters()
                                                                {
                                                                    MsgClass = MsgClasses.UPDATE,
                                                                    MsgDomainType = dt,
                                                                    StreamId = 1,
                                                                    HasExtendedHeader = hasExtHeader,
                                                                    HasMsgKey = hasMsgKey,
                                                                    HasPayload = true,
                                                                    PayloadType = pt,
                                                                    ContainerType = pt,
                                                                    HasQos = false,
                                                                    PrivateStream = false,
                                                                    HasMsgKeyType = true,
                                                                    HasFilter = false,
                                                                    HasAttrib = hasAttrib,
                                                                    AttribContainerType = attribType,
                                                                    HasIdentifier = true,
                                                                    HasState = true,
                                                                    ClearCache = false,
                                                                    DoNotCache = doNotCache,
                                                                    HasGroupId = false,
                                                                    MessageComplete = false,
                                                                    HasPublisherId = hasPublisherId,
                                                                    HasPartNum = false,
                                                                    Solicited = false,
                                                                    HasSeqNum = hasSeqNum,
                                                                    DoNotConflate = doNotConflate,
                                                                    DoNotRipple = doNotRipple,
                                                                    HasPermData = hasPermData,
                                                                    HasConfInfo = hasConflated
                                                                };

                                                                UpdateMsg updateMsg = m_objectManager.GetOmmUpdateMsg();
                                                                EmaComplexTypeHandler.EncodeUpdateMessage(msgParameters, updateMsg);

                                                                var body = updateMsg.m_updateMsgEncoder.m_encodeIterator!.Buffer();

                                                                UpdateMsg decodeMsg = m_objectManager.GetOmmUpdateMsg();
                                                                EmaComplexTypeHandler.DecodeAndCheckUpdateMessage(msgParameters, decodeMsg, body, dictionary);

                                                                updateMsg.ClearAndReturnToPool_All();
                                                                decodeMsg.ClearAndReturnToPool_All();
                                                            }
            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeStatusMsg()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.LOGIN };
            int[] payloadTypes = { DataTypes.ANSI_PAGE, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.FIELD_LIST, DataTypes.MAP };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in values)
                        foreach (bool hasPermData in values)
                            foreach (bool clearCache in values)
                                foreach (bool hasItemGroup in values)
                                    foreach (bool hasState in values)
                                        foreach (bool hasPublisherId in values)
                                            foreach (bool privateStream in values)
                                                foreach (bool hasMsgKey in values)
                                                    foreach (bool hasAttrib in values)
                                                        foreach (int attribType in attribTypes)
                                                            foreach (bool preenc in values)
                                                            {
                                                                MsgParameters msgParameters = new MsgParameters()
                                                                {
                                                                    MsgClass = MsgClasses.STATUS,
                                                                    MsgDomainType = dt,
                                                                    StreamId = 1,
                                                                    HasExtendedHeader = hasExtHeader,
                                                                    HasMsgKey = hasMsgKey,
                                                                    HasPayload = true,
                                                                    PayloadType = pt,
                                                                    ContainerType = pt,
                                                                    HasQos = false,
                                                                    PrivateStream = privateStream,
                                                                    HasMsgKeyType = true,
                                                                    HasFilter = false,
                                                                    HasAttrib = hasAttrib,
                                                                    AttribContainerType = attribType,
                                                                    HasIdentifier = true,
                                                                    HasState = hasState,
                                                                    ClearCache = clearCache,
                                                                    HasGroupId = hasItemGroup,
                                                                    MessageComplete = false,
                                                                    HasPublisherId = hasPublisherId,
                                                                    HasPartNum = false,
                                                                    Solicited = false,
                                                                    HasPermData = hasPermData,
                                                                    Preencoded = preenc
                                                                };

                                                                StatusMsg statusMsg = m_objectManager.GetOmmStatusMsg();
                                                                EmaComplexTypeHandler.EncodeStatusMessage(msgParameters, statusMsg);

                                                                var body = statusMsg.m_statusMsgEncoder.m_encodeIterator!.Buffer();

                                                                StatusMsg decodeMsg = m_objectManager.GetOmmStatusMsg();
                                                                EmaComplexTypeHandler.DecodeAndCheckStatusMessage(msgParameters, decodeMsg, body, dictionary);

                                                                statusMsg.ClearAndReturnToPool_All();
                                                                decodeMsg.ClearAndReturnToPool_All();
                                                            }
            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeGenericMsg()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.LOGIN };
            int[] payloadTypes = { DataTypes.ANSI_PAGE, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.FIELD_LIST, DataTypes.MAP };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in values)
                        foreach (bool hasPermData in values)
                            foreach (bool providerDriven in values)
                                foreach (bool hasSecSeqNum in values)
                                    foreach (bool hasPartNum in values)
                                        foreach (bool hasSeqNum in values)
                                            foreach (bool isComplete in values)
                                                foreach (bool hasMsgKey in values)
                                                    foreach (bool hasAttrib in values)
                                                        foreach (int attribType in attribTypes)
                                                            foreach (bool preenc in values)
                                                            {
                                                                MsgParameters msgParameters = new MsgParameters()
                                                                {
                                                                    MsgClass = MsgClasses.GENERIC,
                                                                    MsgDomainType = dt,
                                                                    StreamId = 1,
                                                                    HasExtendedHeader = hasExtHeader,
                                                                    HasMsgKey = hasMsgKey,
                                                                    HasPayload = true,
                                                                    PayloadType = pt,
                                                                    ContainerType = pt,
                                                                    HasQos = false,
                                                                    HasMsgKeyType = true,
                                                                    HasFilter = false,
                                                                    HasAttrib = hasAttrib,
                                                                    AttribContainerType = attribType,
                                                                    HasIdentifier = true,
                                                                    MessageComplete = isComplete,
                                                                    HasPartNum = hasPartNum,
                                                                    HasSecondarySeqNum = hasSecSeqNum,
                                                                    HasSeqNum = hasSeqNum,
                                                                    Solicited = false,
                                                                    HasPermData = hasPermData,
                                                                    ProviderDriven = providerDriven,
                                                                    Preencoded = preenc
                                                                };

                                                                GenericMsg genericMsg = m_objectManager.GetOmmGenericMsg();
                                                                EmaComplexTypeHandler.EncodeGenericMessage(msgParameters, genericMsg);

                                                                var body = genericMsg.m_genericMsgEncoder.m_encodeIterator!.Buffer();

                                                                GenericMsg decodeMsg = m_objectManager.GetOmmGenericMsg();
                                                                EmaComplexTypeHandler.DecodeAndCheckGenericMessage(msgParameters, decodeMsg, body, dictionary);

                                                                genericMsg.ClearAndReturnToPool_All();
                                                                decodeMsg.ClearAndReturnToPool_All();
                                                            }


            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodePostMsg()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.MARKET_BY_ORDER, (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.OPAQUE, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.XML, DataTypes.SERIES };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in values)
                        foreach (bool hasPermData in values)
                            foreach (bool hasPostId in values)
                                foreach (bool hasSeqNum in values)
                                    foreach (bool hasPartNum in values)
                                        foreach (bool hasPostUserRights in values)
                                            foreach (bool solicitAck in values)
                                                foreach (bool isComplete in values)
                                                    foreach (bool hasMsgKey in values)
                                                        foreach (bool hasAttrib in values)
                                                            foreach (int attribType in attribTypes)
                                                            {
                                                                MsgParameters msgParameters = new MsgParameters()
                                                                {
                                                                    MsgClass = MsgClasses.POST,
                                                                    MsgDomainType = dt,
                                                                    StreamId = 1,
                                                                    HasExtendedHeader = hasExtHeader,
                                                                    HasMsgKey = hasMsgKey,
                                                                    HasPayload = true,
                                                                    PayloadType = pt,
                                                                    ContainerType = pt,
                                                                    HasMsgKeyType = true,
                                                                    HasFilter = true,
                                                                    HasAttrib = hasAttrib,
                                                                    AttribContainerType = attribType,
                                                                    HasIdentifier = true,
                                                                    MessageComplete = isComplete,
                                                                    HasPartNum = hasPartNum,
                                                                    HasPostId = hasPostId,
                                                                    HasSeqNum = hasSeqNum,
                                                                    Solicited = false,
                                                                    HasPermData = hasPermData,
                                                                    HasPostUserRights = hasPostUserRights,
                                                                    SolicitAck = solicitAck
                                                                };

                                                                PostMsg postMsg = m_objectManager.GetOmmPostMsg();
                                                                EmaComplexTypeHandler.EncodePostMessage(msgParameters, postMsg);

                                                                var body = postMsg.m_postMsgEncoder.m_encodeIterator!.Buffer();

                                                                PostMsg decodeMsg = m_objectManager.GetOmmPostMsg();
                                                                EmaComplexTypeHandler.DecodeAndCheckPostMessage(msgParameters, decodeMsg, body, dictionary);

                                                                postMsg.ClearAndReturnToPool_All();
                                                                decodeMsg.ClearAndReturnToPool_All();
                                                            }
            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeAckMsg()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            int[] domainTypes = { (int)DomainType.MARKET_BY_ORDER, (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.OPAQUE, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.VECTOR, DataTypes.SERIES };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in values)
                        foreach (bool hasNackCode in values)
                            foreach (bool hasSeqNum in values)
                                foreach (bool privateStream in values)
                                    foreach (bool hasText in values)
                                        foreach (bool hasMsgKey in values)
                                            foreach (bool hasAttrib in values)
                                                foreach (int attribType in attribTypes)
                                                    foreach (bool preenc in values)
                                                    {
                                                    MsgParameters msgParameters = new MsgParameters()
                                                    {
                                                        MsgClass = MsgClasses.ACK,
                                                        MsgDomainType = dt,
                                                        StreamId = 1,
                                                        HasExtendedHeader = hasExtHeader,
                                                        HasMsgKey = hasMsgKey,
                                                        ContainerType = pt,
                                                        HasPayload = true,
                                                        HasMsgKeyType = true,
                                                        HasFilter = true,
                                                        HasAttrib = hasAttrib,
                                                        AttribContainerType = attribType,
                                                        HasIdentifier = true,
                                                        HasNackCode = hasNackCode,
                                                        HasText = hasText,
                                                        HasSeqNum = hasSeqNum,
                                                        Solicited = false,
                                                        Preencoded = preenc
                                                    };

                                                    AckMsg ackMsg = m_objectManager.GetOmmAckMsg();
                                                    EmaComplexTypeHandler.EncodeAckMessage(msgParameters, ackMsg);

                                                    var body = ackMsg.m_ackMsgEncoder.m_encodeIterator!.Buffer();

                                                    AckMsg decodeMsg = m_objectManager.GetOmmAckMsg();
                                                    EmaComplexTypeHandler.DecodeAndCheckAckMessage(msgParameters, decodeMsg, body, dictionary);

                                                    ackMsg.ClearAndReturnToPool_All();
                                                    decodeMsg.ClearAndReturnToPool_All();
                                                }

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeBatchRequest_PreEncoded_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg reqMsg = m_objectManager.GetOmmRequestMsg();

            reqMsg.ServiceId(1);
            reqMsg.DomainType((int)DomainType.MARKET_PRICE);
            reqMsg.StreamId(5);

            ElementList payload = m_objectManager.GetOmmElementList();
            OmmArray array = m_objectManager.GetOmmArray();          
            payload.AddArray(":ItemList", array);
            array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
            payload.Complete();
            reqMsg.Payload(payload);
            reqMsg.EncodeComplete();

            Assert.Equal(2, ((RequestMsgEncoder)reqMsg.Encoder!).BatchItemList!.Count);
            Assert.True(reqMsg.m_rsslMsg.CheckHasBatch());

            var body = reqMsg.Encoder!.m_encodeIterator!.Buffer();
            RequestMsg decodedReqMsg = m_objectManager.GetOmmRequestMsg();

            Assert.Equal(CodecReturnCode.SUCCESS, decodedReqMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));
            var decodedPayload = decodedReqMsg.Payload();
            Assert.Equal(DataTypes.ELEMENT_LIST, decodedPayload.DataType);
            var elementList = decodedPayload.ElementList();
            int count = 0;
            bool foundBatch = false;
            foreach (var element in elementList)
            {
                count++;
                if (element.Name.Equals(":ItemList"))
                {
                    foundBatch = true;
                    Assert.Equal(DataTypes.ARRAY, element.LoadType);
                    var ommArray = element.OmmArrayValue();
                    HashSet<string> foundItems = new HashSet<string>();
                    foreach (var arrElement in ommArray)
                    {
                        Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                        foundItems.Add(arrElement.OmmAsciiValue().Value);
                    }
                    Assert.Equal(2, foundItems.Count);
                    Assert.Contains("TRI.N", foundItems);
                    Assert.Contains("IBM.N", foundItems);
                }
            }
            Assert.Equal(1, count);
            Assert.True(foundBatch);

            reqMsg.ClearAndReturnToPool_All();
            decodedReqMsg.ClearAndReturnToPool_All();
            payload.ClearAndReturnToPool_All();
            array.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeBatchRequest_PostEncoded_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg reqMsg = m_objectManager.GetOmmRequestMsg();

            reqMsg.ServiceId(1);
            reqMsg.DomainType((int)DomainType.MARKET_PRICE);
            reqMsg.StreamId(5);

            ElementList payload = m_objectManager.GetOmmElementList();
            reqMsg.Payload(payload);
            OmmArray array = m_objectManager.GetOmmArray();
            payload.AddArray(":ItemList", array);
            array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
            payload.Complete();       
            reqMsg.EncodeComplete();

            Assert.Equal(2, ((RequestMsgEncoder)reqMsg.Encoder!).BatchItemList!.Count);
            Assert.True(reqMsg.m_rsslMsg.CheckHasBatch());

            var body = reqMsg.Encoder!.m_encodeIterator!.Buffer();
            RequestMsg decodedReqMsg = m_objectManager.GetOmmRequestMsg();

            Assert.Equal(CodecReturnCode.SUCCESS, decodedReqMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));
            var decodedPayload = decodedReqMsg.Payload();
            Assert.Equal(DataTypes.ELEMENT_LIST, decodedPayload.DataType);
            var elementList = decodedPayload.ElementList();
            int count = 0;
            bool foundBatch = false;
            foreach (var element in elementList)
            {
                count++;
                if (element.Name.Equals(":ItemList"))
                {
                    foundBatch = true;
                    Assert.Equal(DataTypes.ARRAY, element.LoadType);
                    var ommArray = element.OmmArrayValue();
                    HashSet<string> foundItems = new HashSet<string>();
                    foreach (var arrElement in ommArray)
                    {
                        Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                        foundItems.Add(arrElement.OmmAsciiValue().Value);
                    }
                    Assert.Equal(2, foundItems.Count);
                    Assert.Contains("TRI.N", foundItems);
                    Assert.Contains("IBM.N", foundItems);
                }
            }
            Assert.Equal(1, count);
            Assert.True(foundBatch);

            reqMsg.ClearAndReturnToPool_All();
            decodedReqMsg.ClearAndReturnToPool_All();
            payload.ClearAndReturnToPool_All();
            array.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeUsingPreviouslyDecodedContainer()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg reqMsg = m_objectManager.GetOmmRequestMsg();

            reqMsg.ServiceId(1);
            reqMsg.DomainType((int)DomainType.MARKET_PRICE);
            reqMsg.StreamId(5);

            ElementList elementList = m_objectManager.GetOmmElementList();
            OmmArray array = m_objectManager.GetOmmArray();
            elementList.AddArray(":ItemList", array);
            array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
            elementList.Complete();

            ElementList decodedElementList = m_objectManager.GetOmmElementList();
            var elementListBuffer = elementList.Encoder!.m_encodeIterator!.Buffer();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), elementListBuffer, dictionary, null));

            reqMsg.Payload(decodedElementList);
            reqMsg.EncodeComplete();

            Assert.Equal(2, ((RequestMsgEncoder)reqMsg.Encoder!).BatchItemList!.Count);
            Assert.True(reqMsg.m_rsslMsg.CheckHasBatch());

            var body = reqMsg.Encoder!.m_encodeIterator!.Buffer();
            RequestMsg decodedReqMsg = m_objectManager.GetOmmRequestMsg();

            Assert.Equal(CodecReturnCode.SUCCESS, decodedReqMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));
            var decodedPayload = decodedReqMsg.Payload();
            Assert.Equal(DataTypes.ELEMENT_LIST, decodedPayload.DataType);
            var finalElementList = decodedPayload.ElementList();
            int count = 0;
            bool foundBatch = false;
            foreach (var element in finalElementList)
            {
                count++;
                if (element.Name.Equals(":ItemList"))
                {
                    foundBatch = true;
                    Assert.Equal(DataTypes.ARRAY, element.LoadType);
                    var ommArray = element.OmmArrayValue();
                    HashSet<string> foundItems = new HashSet<string>();
                    foreach (var arrElement in ommArray)
                    {
                        Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                        foundItems.Add(arrElement.OmmAsciiValue().Value);
                    }
                    Assert.Equal(2, foundItems.Count);
                    Assert.Contains("TRI.N", foundItems);
                    Assert.Contains("IBM.N", foundItems);
                }
            }
            Assert.Equal(1, count);
            Assert.True(foundBatch);

            elementList.ClearAndReturnToPool_All();
            array.ClearAndReturnToPool_All();
            reqMsg.ClearAndReturnToPool_All();
            decodedReqMsg.ClearAndReturnToPool_All();
            decodedElementList.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void CopyFromEncodedMessageTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            foreach (bool completeEnc in values)
            {
                RequestMsg sourceMsg = m_objectManager.GetOmmRequestMsg();

                sourceMsg.ServiceId(1);
                sourceMsg.DomainType((int)DomainType.MARKET_PRICE);
                sourceMsg.StreamId(5);

                ElementList elementList = m_objectManager.GetOmmElementList();
                OmmArray array = m_objectManager.GetOmmArray();
                elementList.AddArray(":ItemList", array);
                array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
                elementList.Complete();

                ElementList decodedElementList = m_objectManager.GetOmmElementList();
                var elementListBuffer = elementList.Encoder!.m_encodeIterator!.Buffer();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), elementListBuffer, dictionary, null));

                sourceMsg.Payload(decodedElementList);
                if (completeEnc) sourceMsg.EncodeComplete();

                RequestMsg destinationMsg = m_objectManager.GetOmmRequestMsg();

                sourceMsg.CopyMsg(destinationMsg);
                sourceMsg.Clear(); // ensure that we have a deep copy so that data in destination persists even after the source is cleared

                var payload = destinationMsg.Payload();
                Assert.Equal(DataTypes.ELEMENT_LIST, payload.DataType);
                var finalElementList = payload.ElementList();
                int count = 0;
                bool foundBatch = false;
                foreach (var element in finalElementList)
                {
                    count++;
                    if (element.Name.Equals(":ItemList"))
                    {
                        foundBatch = true;
                        Assert.Equal(DataTypes.ARRAY, element.LoadType);
                        var ommArray = element.OmmArrayValue();
                        HashSet<string> foundItems = new HashSet<string>();
                        foreach (var arrElement in ommArray)
                        {
                            Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                            foundItems.Add(arrElement.OmmAsciiValue().Value);
                        }
                        Assert.Equal(2, foundItems.Count);
                        Assert.Contains("TRI.N", foundItems);
                        Assert.Contains("IBM.N", foundItems);
                    }
                }
                Assert.Equal(1, count);
                Assert.True(foundBatch);

                elementList.ClearAndReturnToPool_All();
                decodedElementList.ClearAndReturnToPool_All();
                array.ClearAndReturnToPool_All();
                sourceMsg.ClearAndReturnToPool_All();
                destinationMsg.ClearAndReturnToPool_All();

                CheckEtaGlobalPoolSizes();
                CheckEmaObjectManagerPoolSizes(m_objectManager);
            }
            
        }

        [Fact]
        public void CopyFromDecodedMessageTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg msg = m_objectManager.GetOmmRequestMsg();

            msg.ServiceId(1);
            msg.DomainType((int)DomainType.MARKET_PRICE);
            msg.StreamId(5);

            ElementList elementList = m_objectManager.GetOmmElementList();
            OmmArray array = m_objectManager.GetOmmArray();
            elementList.AddArray(":ItemList", array);
            array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
            elementList.Complete();

            ElementList decodedElementList = m_objectManager.GetOmmElementList();
            var elementListBuffer = elementList.Encoder!.m_encodeIterator!.Buffer();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), elementListBuffer, dictionary, null));

            msg.Payload(decodedElementList);
            msg.EncodeComplete();

            var body = msg.Encoder!.m_encodeIterator!.Buffer();
            RequestMsg decodedSourceMsg = m_objectManager.GetOmmRequestMsg();

            Assert.Equal(CodecReturnCode.SUCCESS, decodedSourceMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));

            RequestMsg destinationMsg = m_objectManager.GetOmmRequestMsg();

            decodedSourceMsg.CopyMsg(destinationMsg);
            decodedSourceMsg.Clear(); // ensure that we have a deep copy so that data in destnation persists even after the source is cleared

            var payload = destinationMsg.Payload();
            Assert.Equal(DataTypes.ELEMENT_LIST, payload.DataType);
            var finalElementList = payload.ElementList();
            int count = 0;
            bool foundBatch = false;
            foreach (var element in finalElementList)
            {
                count++;
                if (element.Name.Equals(":ItemList"))
                {
                    foundBatch = true;
                    Assert.Equal(DataTypes.ARRAY, element.LoadType);
                    var ommArray = element.OmmArrayValue();
                    HashSet<string> foundItems = new HashSet<string>();
                    foreach (var arrElement in ommArray)
                    {
                        Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                        foundItems.Add(arrElement.OmmAsciiValue().Value);
                    }
                    Assert.Equal(2, foundItems.Count);
                    Assert.Contains("TRI.N", foundItems);
                    Assert.Contains("IBM.N", foundItems);
                }
            }
            Assert.Equal(1, count);
            Assert.True(foundBatch);

            elementList.ClearAndReturnToPool_All();
            decodedElementList.ClearAndReturnToPool_All();
            array.ClearAndReturnToPool_All();
            msg.ClearAndReturnToPool_All();
            decodedSourceMsg.ClearAndReturnToPool_All();
            destinationMsg.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void CopyConstructorFromDecodedMessageTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg msg = m_objectManager.GetOmmRequestMsg();

            msg.ServiceId(1);
            msg.DomainType((int)DomainType.MARKET_PRICE);
            msg.StreamId(5);

            ElementList elementList = m_objectManager.GetOmmElementList();
            OmmArray array = m_objectManager.GetOmmArray();
            elementList.AddArray(":ItemList", array);
            array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
            elementList.Complete();

            ElementList decodedElementList = m_objectManager.GetOmmElementList();
            var elementListBuffer = elementList.Encoder!.m_encodeIterator!.Buffer();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), elementListBuffer, dictionary, null));

            msg.Payload(decodedElementList);
            msg.EncodeComplete();

            var body = msg.Encoder!.m_encodeIterator!.Buffer();
            RequestMsg decodedSourceMsg = m_objectManager.GetOmmRequestMsg();

            Assert.Equal(CodecReturnCode.SUCCESS, decodedSourceMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));

            RequestMsg destinationMsg = new RequestMsg(decodedSourceMsg);

            decodedSourceMsg.Clear(); // ensure that we have a deep copy so that data in destnation persists even after the source is cleared

            var payload = destinationMsg.Payload();
            Assert.Equal(DataTypes.ELEMENT_LIST, payload.DataType);
            var finalElementList = payload.ElementList();
            int count = 0;
            bool foundBatch = false;
            foreach (var element in finalElementList)
            {
                count++;
                if (element.Name.Equals(":ItemList"))
                {
                    foundBatch = true;
                    Assert.Equal(DataTypes.ARRAY, element.LoadType);
                    var ommArray = element.OmmArrayValue();
                    HashSet<string> foundItems = new HashSet<string>();
                    foreach (var arrElement in ommArray)
                    {
                        Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                        foundItems.Add(arrElement.OmmAsciiValue().Value);
                    }
                    Assert.Equal(2, foundItems.Count);
                    Assert.Contains("TRI.N", foundItems);
                    Assert.Contains("IBM.N", foundItems);
                }
            }
            Assert.Equal(1, count);
            Assert.True(foundBatch);

            elementList.ClearAndReturnToPool_All();
            decodedElementList.ClearAndReturnToPool_All();
            array.ClearAndReturnToPool_All();
            msg.ClearAndReturnToPool_All();
            decodedSourceMsg.ClearAndReturnToPool_All();
            destinationMsg.Clear();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void CloneFromEncodedMessageTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            foreach (bool completeEnc in values)
            {
                RequestMsg sourceMsg = m_objectManager.GetOmmRequestMsg();

                sourceMsg.ServiceId(1);
                sourceMsg.DomainType((int)DomainType.MARKET_PRICE);
                sourceMsg.StreamId(5);

                ElementList elementList = m_objectManager.GetOmmElementList();
                OmmArray array = m_objectManager.GetOmmArray();
                elementList.AddArray(":ItemList", array);
                array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
                elementList.Complete();

                ElementList decodedElementList = m_objectManager.GetOmmElementList();
                var elementListBuffer = elementList.Encoder!.m_encodeIterator!.Buffer();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), elementListBuffer, dictionary, null));

                sourceMsg.Payload(decodedElementList);
                if (completeEnc) sourceMsg.EncodeComplete();

                RequestMsg destinationMsg = sourceMsg.Clone();

                sourceMsg.Clear(); // ensure that we have a deep copy so that data in destination persists even after the source is cleared

                var payload = destinationMsg.Payload();
                Assert.Equal(DataTypes.ELEMENT_LIST, payload.DataType);
                var finalElementList = payload.ElementList();
                int count = 0;
                bool foundBatch = false;
                foreach (var element in finalElementList)
                {
                    count++;
                    if (element.Name.Equals(":ItemList"))
                    {
                        foundBatch = true;
                        Assert.Equal(DataTypes.ARRAY, element.LoadType);
                        var ommArray = element.OmmArrayValue();
                        HashSet<string> foundItems = new HashSet<string>();
                        foreach (var arrElement in ommArray)
                        {
                            Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                            foundItems.Add(arrElement.OmmAsciiValue().Value);
                        }
                        Assert.Equal(2, foundItems.Count);
                        Assert.Contains("TRI.N", foundItems);
                        Assert.Contains("IBM.N", foundItems);
                    }
                }
                Assert.Equal(1, count);
                Assert.True(foundBatch);

                elementList.ClearAndReturnToPool_All();
                decodedElementList.ClearAndReturnToPool_All();
                array.ClearAndReturnToPool_All();
                sourceMsg.ClearAndReturnToPool_All();

                CheckEtaGlobalPoolSizes();
                CheckEmaObjectManagerPoolSizes(m_objectManager);
            }
        }

        [Fact]
        public void EncodeAndDecodeMsgWithCopyFlagSetTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg msg = m_objectManager.GetOmmRequestMsg();
            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = true;

            msg.ServiceId(1);
            msg.DomainType((int)DomainType.MARKET_PRICE);
            msg.StreamId(5);
            msg.ExtendedHeader(EmaComplexTypeHandler.extendedHdrBuffer);

            ElementList elementList = m_objectManager.GetOmmElementList();
            OmmArray array = m_objectManager.GetOmmArray();
            elementList.AddArray(":ItemList", array);
            array.AddAscii("TRI.N").AddAscii("IBM.N").Complete();
            elementList.Complete();

            ElementList attribElementList = m_objectManager.GetOmmElementList();
            attribElementList.AddInt("Int1", 25);
            attribElementList.AddAscii("Ascii1", "SomeAscii");

            msg.Payload(elementList);
            msg.Attrib(attribElementList);
            msg.EncodeComplete();

            Assert.Equal(attribElementList.Encoder!.m_encodeIterator!.Buffer().Data(), msg.m_rsslMsg.MsgKey.EncodedAttrib.Data());
            Assert.Equal(elementList.Encoder!.m_encodeIterator!.Buffer().Data(), msg.m_rsslMsg.EncodedDataBody.Data());

            var body = msg.Encoder!.m_encodeIterator!.Buffer();
            RequestMsg decodedMsg = m_objectManager.GetOmmRequestMsg();

            Assert.Equal(CodecReturnCode.SUCCESS, decodedMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));

            var payload = decodedMsg.Payload();
            Assert.Equal(DataTypes.ELEMENT_LIST, payload.DataType);
            var finalElementList = payload.ElementList();
            int count = 0;
            bool foundBatch = false;
            foreach (var element in finalElementList)
            {
                count++;
                if (element.Name.Equals(":ItemList"))
                {
                    foundBatch = true;
                    Assert.Equal(DataTypes.ARRAY, element.LoadType);
                    var ommArray = element.OmmArrayValue();
                    HashSet<string> foundItems = new HashSet<string>();
                    foreach (var arrElement in ommArray)
                    {
                        Assert.Equal(DataTypes.ASCII_STRING, arrElement.LoadType);
                        foundItems.Add(arrElement.OmmAsciiValue().Value);
                    }
                    Assert.Equal(2, foundItems.Count);
                    Assert.Contains("TRI.N", foundItems);
                    Assert.Contains("IBM.N", foundItems);
                }
            }
            Assert.Equal(1, count);
            Assert.True(foundBatch);

            elementList.ClearAndReturnToPool_All();
            attribElementList.ClearAndReturnToPool_All();
            array.ClearAndReturnToPool_All();
            msg.ClearAndReturnToPool_All();
            decodedMsg.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void DecodeFromCodecMsg_PayloadAndAttribPresent_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg requestMsg = new RequestMsg();

            Eta.Codec.Msg codecMsg = new Eta.Codec.Msg();
            codecMsg.MsgClass = MsgClasses.REQUEST;
            codecMsg.DomainType = (int)DomainType.MARKET_PRICE;
            codecMsg.StreamId = 5;
            codecMsg.ApplyHasMsgKey();
            codecMsg.MsgKey.ServiceId = 3;
            codecMsg.MsgKey.ApplyHasName();
            Buffer name = new Buffer();
            name.Data("Codec");
            codecMsg.MsgKey.Name = name;
            codecMsg.MsgKey.ApplyHasNameType();
            codecMsg.MsgKey.NameType = InstrumentNameTypes.UNSPECIFIED;

            EncodeIterator encIter = new EncodeIterator();
            Buffer payloadBuf = new Buffer();
            payloadBuf.Data(new ByteBuffer(512));
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(payloadBuf, Codec.MajorVersion(), Codec.MinorVersion());
            CodecTestUtil.EncodeDefaultContainer(encIter, DataTypes.ELEMENT_LIST);
            codecMsg.ContainerType = DataTypes.ELEMENT_LIST;
            codecMsg.EncodedDataBody = payloadBuf;

            Buffer attribBuf = new Buffer();
            attribBuf.Data(new ByteBuffer(512));
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(attribBuf, Codec.MajorVersion(), Codec.MinorVersion());
            CodecTestUtil.EncodeDefaultContainer(encIter, DataTypes.FIELD_LIST);
            codecMsg.MsgKey.ApplyHasAttrib();
            codecMsg.MsgKey.AttribContainerType = DataTypes.FIELD_LIST;
            codecMsg.MsgKey.EncodedAttrib = attribBuf;

            Assert.Equal(CodecReturnCode.SUCCESS, requestMsg.Decode(codecMsg, Codec.MajorVersion(), Codec.MinorVersion(), dictionary));

            Assert.Equal(5, requestMsg.StreamId());
            Assert.Equal((int)DomainType.MARKET_PRICE, requestMsg.DomainType());
            Assert.True(requestMsg.HasMsgKey);
            Assert.Equal(Access.DataType.DataTypes.ELEMENT_LIST, requestMsg.Payload().DataType);
            Assert.Equal(Access.DataType.DataTypes.FIELD_LIST, requestMsg.Attrib().DataType);
            Assert.True(requestMsg.HasName);
            Assert.True(requestMsg.HasNameType);

            requestMsg.Clear();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void DecodeFromCodecMsg_PayloadPresentAttribEmpty_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg requestMsg = new RequestMsg();

            Eta.Codec.Msg codecMsg = new Eta.Codec.Msg();
            codecMsg.MsgClass = MsgClasses.REQUEST;
            codecMsg.DomainType = (int)DomainType.MARKET_PRICE;
            codecMsg.StreamId = 5;
            codecMsg.ApplyHasMsgKey();
            codecMsg.MsgKey.ServiceId = 3;
            codecMsg.MsgKey.ApplyHasName();
            Buffer name = new Buffer();
            name.Data("Codec");
            codecMsg.MsgKey.Name = name;
            codecMsg.MsgKey.ApplyHasNameType();
            codecMsg.MsgKey.NameType = InstrumentNameTypes.UNSPECIFIED;

            EncodeIterator encIter = new EncodeIterator();
            Buffer payloadBuf = new Buffer();
            payloadBuf.Data(new ByteBuffer(512));
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(payloadBuf, Codec.MajorVersion(), Codec.MinorVersion());
            CodecTestUtil.EncodeDefaultContainer(encIter, DataTypes.ELEMENT_LIST);
            codecMsg.ContainerType = DataTypes.ELEMENT_LIST;
            codecMsg.EncodedDataBody = payloadBuf;

            Assert.Equal(CodecReturnCode.SUCCESS, requestMsg.Decode(codecMsg, Codec.MajorVersion(), Codec.MinorVersion(), dictionary));

            Assert.Equal(5, requestMsg.StreamId());
            Assert.Equal((int)DomainType.MARKET_PRICE, requestMsg.DomainType());
            Assert.True(requestMsg.HasMsgKey);
            Assert.Equal(Access.DataType.DataTypes.ELEMENT_LIST, requestMsg.Payload().DataType);
            Assert.True(requestMsg.HasName);
            Assert.True(requestMsg.HasNameType);

            requestMsg.Clear();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void DecodeFromCodecMsg_PayloadAttribEmpty_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            RequestMsg requestMsg = new RequestMsg();

            Eta.Codec.Msg codecMsg = new Eta.Codec.Msg();
            codecMsg.MsgClass = MsgClasses.REQUEST;
            codecMsg.DomainType = (int)DomainType.MARKET_PRICE;
            codecMsg.StreamId = 5;
            codecMsg.ApplyHasMsgKey();
            codecMsg.MsgKey.ServiceId = 3;
            codecMsg.MsgKey.ApplyHasName();
            Buffer name = new Buffer();
            name.Data("Codec");
            codecMsg.MsgKey.Name = name;
            codecMsg.MsgKey.ApplyHasNameType();
            codecMsg.MsgKey.NameType = InstrumentNameTypes.UNSPECIFIED;

            Assert.Equal(CodecReturnCode.SUCCESS, requestMsg.Decode(codecMsg, Codec.MajorVersion(), Codec.MinorVersion(), dictionary));

            Assert.Equal(5, requestMsg.StreamId());
            Assert.Equal((int)DomainType.MARKET_PRICE, requestMsg.DomainType());
            Assert.True(requestMsg.HasMsgKey);
            Assert.Equal(3, codecMsg.MsgKey.ServiceId);
            Assert.True(requestMsg.HasName);
            Assert.True(requestMsg.HasNameType);

            requestMsg.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeElementListWithMessagePayloadTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            ElementList elementList = m_objectManager.GetOmmElementList();
            
            EmaComplexTypeHandler.EncodeElementList(elementList, msgDataTypes, true);

            var body = elementList.Encoder!.m_encodeIterator!.Buffer();

            ElementList decodedElementList = m_objectManager.GetOmmElementList();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
            EmaComplexTypeHandler.DecodeAndCheckElementList(decodedElementList, msgDataTypes);

            elementList.ClearAndReturnToPool_All();
            decodedElementList.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeElementListWithMessagePayload_PreencodedMsgPayload_NotEncodeCompleted_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            SetDefaultMsgParameters();

            ElementList elementList = m_objectManager.GetOmmElementList();

            EmaComplexTypeHandler.EncodeElementList(elementList, msgDataTypes, true);

            var body = elementList.Encoder!.m_encodeIterator!.Buffer();

            ElementList decodedElementList = m_objectManager.GetOmmElementList();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
            EmaComplexTypeHandler.DecodeAndCheckElementList(decodedElementList, msgDataTypes);

            elementList.ClearAndReturnToPool_All();
            decodedElementList.ClearAndReturnToPool_All();

            ResetDefaultMsgParameters();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeVectorWithMessagePayloadTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            foreach (var msgType in msgDataTypes)
            {
                Vector vector = m_objectManager.GetOmmVector();

                EmaComplexTypeHandler.EncodeVector(vector, msgType, 
                    false, true, true, 
                    EmaComplexTypeHandler.defaultVectorActions, 
                    EmaComplexTypeHandler.defaultVectorEntryHasPermData, true);

                var body = vector.Encoder!.m_encodeIterator!.Buffer();

                Vector decodedVector = m_objectManager.GetOmmVector();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedVector.DecodeVector(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckVector(decodedVector, msgType, 
                    false, true, true,
                    EmaComplexTypeHandler.defaultVectorActions, 
                    EmaComplexTypeHandler.defaultVectorEntryHasPermData);

                vector.ClearAndReturnToPool_All();
                decodedVector.ClearAndReturnToPool_All();
            }

            EmaComplexTypeHandler.defaultRefreshMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultRequestMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultAckMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultStatusMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultGenericMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultPostMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.Preencoded = false;

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeVectorWithMessagePayload_PreencodedMsgPayload_NotEncodeCompleted_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            SetDefaultMsgParameters();

            foreach (var msgType in msgDataTypes)
            {
                Vector vector = m_objectManager.GetOmmVector();

                EmaComplexTypeHandler.EncodeVector(vector, msgType,
                    false, true, true,
                    EmaComplexTypeHandler.defaultVectorActions,
                    EmaComplexTypeHandler.defaultVectorEntryHasPermData, true);

                var body = vector.Encoder!.m_encodeIterator!.Buffer();

                Vector decodedVector = m_objectManager.GetOmmVector();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedVector.DecodeVector(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckVector(decodedVector, msgType,
                    false, true, true,
                    EmaComplexTypeHandler.defaultVectorActions,
                    EmaComplexTypeHandler.defaultVectorEntryHasPermData);

                vector.ClearAndReturnToPool_All();
                decodedVector.ClearAndReturnToPool_All();
            }

            ResetDefaultMsgParameters();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeSeriesWithMessagePayloadTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            foreach (var msgType in msgDataTypes)
            {
                Series series = m_objectManager.GetOmmSeries();

                EmaComplexTypeHandler.EncodeSeries(series, msgType, true, false, 
                    EmaComplexTypeHandler.defaultSeriesCountHint, EmaComplexTypeHandler.defaultSeriesCountHint, true);

                var body = series.Encoder!.m_encodeIterator!.Buffer();

                Series decodedSeries = m_objectManager.GetOmmSeries();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedSeries.DecodeSeries(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckSeries(decodedSeries, msgType, true, false, 
                    EmaComplexTypeHandler.defaultSeriesCountHint, EmaComplexTypeHandler.defaultSeriesCountHint);

                series.ClearAndReturnToPool_All();
                decodedSeries.ClearAndReturnToPool_All();
            }

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeSeriesWithMessagePayload_PreencodedMsgPayload_NotEncodeCompleted_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            SetDefaultMsgParameters();

            foreach (var msgType in msgDataTypes)
            {
                Series series = m_objectManager.GetOmmSeries();

                EmaComplexTypeHandler.EncodeSeries(series, msgType, true, false,
                    EmaComplexTypeHandler.defaultSeriesCountHint, EmaComplexTypeHandler.defaultSeriesCountHint, true);

                var body = series.Encoder!.m_encodeIterator!.Buffer();

                Series decodedSeries = m_objectManager.GetOmmSeries();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedSeries.DecodeSeries(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckSeries(decodedSeries, msgType, true, false,
                    EmaComplexTypeHandler.defaultSeriesCountHint, EmaComplexTypeHandler.defaultSeriesCountHint);

                series.ClearAndReturnToPool_All();
                decodedSeries.ClearAndReturnToPool_All();
            }

            ResetDefaultMsgParameters();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeMapWithMessagePayloadTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            foreach (var msgType in msgDataTypes)
            {
                Map map = m_objectManager.GetOmmMap();

                EmaComplexTypeHandler.EncodeMap(map, msgType, 
                    EmaComplexTypeHandler.defaultMapAction, EmaComplexTypeHandler.defaultMapEntryHasPermData, 
                    EmaComplexTypeHandler.defaultMapKeyType, false, true, true, true);

                var body = map.Encoder!.m_encodeIterator!.Buffer();

                Map decodedMap = m_objectManager.GetOmmMap();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedMap.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckMap(decodedMap, msgType,
                    EmaComplexTypeHandler.defaultMapAction, EmaComplexTypeHandler.defaultMapEntryHasPermData,
                    EmaComplexTypeHandler.defaultMapKeyType, false, true, true);

                map.ClearAndReturnToPool_All();
                decodedMap.ClearAndReturnToPool_All();
            }

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeMapWithMessagePayload_PreencodedMsgPayload_NotEncodeCompleted_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            SetDefaultMsgParameters();

            foreach (var msgType in msgDataTypes)
            {
                Map map = m_objectManager.GetOmmMap();

                EmaComplexTypeHandler.EncodeMap(map, msgType,
                    EmaComplexTypeHandler.defaultMapAction, EmaComplexTypeHandler.defaultMapEntryHasPermData,
                    EmaComplexTypeHandler.defaultMapKeyType, false, true, true, true);

                var body = map.Encoder!.m_encodeIterator!.Buffer();

                Map decodedMap = m_objectManager.GetOmmMap();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedMap.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckMap(decodedMap, msgType,
                    EmaComplexTypeHandler.defaultMapAction, EmaComplexTypeHandler.defaultMapEntryHasPermData,
                    EmaComplexTypeHandler.defaultMapKeyType, false, true, true);

                map.ClearAndReturnToPool_All();
                decodedMap.ClearAndReturnToPool_All();
            }

            ResetDefaultMsgParameters();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeFilterListWithMessagePayloadTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            FilterList filterList = m_objectManager.GetOmmFilterList();

            EmaComplexTypeHandler.EncodeFilterList(filterList, DataTypes.MSG,
                true, EmaComplexTypeHandler.defaultFilterListActions, msgDataTypes,
                EmaComplexTypeHandler.defaultFilterEntryHasPermData, EmaComplexTypeHandler.defaultFilterListCountHint, true);

            var body = filterList.Encoder!.m_encodeIterator!.Buffer();

            FilterList decodedFilterList = m_objectManager.GetOmmFilterList();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedFilterList.DecodeFilterList(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
            EmaComplexTypeHandler.DecodeAndCheckFilterList(decodedFilterList, DataTypes.MSG,
                true, EmaComplexTypeHandler.defaultFilterListActions, msgDataTypes,
                EmaComplexTypeHandler.defaultFilterEntryHasPermData, EmaComplexTypeHandler.defaultFilterListCountHint);

            filterList.ClearAndReturnToPool_All();
            decodedFilterList.ClearAndReturnToPool_All();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodeFilterListWithMessagePayload_PreencodedMsgPayload_NotEncodeCompleted_Test()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            SetDefaultMsgParameters();

            FilterList filterList = m_objectManager.GetOmmFilterList();

            EmaComplexTypeHandler.EncodeFilterList(filterList, DataTypes.MSG,
                true, EmaComplexTypeHandler.defaultFilterListActions, msgDataTypes,
                EmaComplexTypeHandler.defaultFilterEntryHasPermData, EmaComplexTypeHandler.defaultFilterListCountHint, true);

            var body = filterList.Encoder!.m_encodeIterator!.Buffer();

            FilterList decodedFilterList = m_objectManager.GetOmmFilterList();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedFilterList.DecodeFilterList(Codec.MajorVersion(), Codec.MinorVersion(), body, dictionary, null));
            EmaComplexTypeHandler.DecodeAndCheckFilterList(decodedFilterList, DataTypes.MSG,
                true, EmaComplexTypeHandler.defaultFilterListActions, msgDataTypes,
                EmaComplexTypeHandler.defaultFilterEntryHasPermData, EmaComplexTypeHandler.defaultFilterListCountHint);

            filterList.ClearAndReturnToPool_All();
            decodedFilterList.ClearAndReturnToPool_All();

            ResetDefaultMsgParameters();

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact(Skip = "Obsolete")]
        public void EncodeAndDecodeFilterListWithMessagePayload_NonPreencodedMsgPayload_NotEncodeCompleted_FailTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            EmaComplexTypeHandler.defaultRefreshMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultRequestMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultAckMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultStatusMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultGenericMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultPostMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.CompleteMsgEncoding = false;

            FilterList filterList = m_objectManager.GetOmmFilterList();

            try
            {
                // Containers should not allow adding empty messages that contain no preencoded data
                Assert.Throws<OmmInvalidUsageException>(() => EmaComplexTypeHandler.EncodeFilterList(filterList, DataTypes.MSG,
                    true, EmaComplexTypeHandler.defaultFilterListActions, msgDataTypes,
                    EmaComplexTypeHandler.defaultFilterEntryHasPermData, EmaComplexTypeHandler.defaultFilterListCountHint, false));
            }
            finally
            {
                filterList.ClearAndReturnToPool_All();
            }

            EmaComplexTypeHandler.defaultRefreshMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultRequestMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultAckMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultStatusMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultGenericMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultPostMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.CompleteMsgEncoding = true;

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact(Skip = "Obsolete")]
        public void EncodeAndDecodeElementListWithMessagePayload_NonPreencodedMsgPayload_NotEncodeCompleted_FailTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            EmaComplexTypeHandler.defaultRefreshMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultRequestMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultAckMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultStatusMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultGenericMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultPostMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.CompleteMsgEncoding = false;

            ElementList elementList = m_objectManager.GetOmmElementList();

            try
            {
                // Containers should not allow adding empty messages that contain no preencoded data
                Assert.Throws<OmmInvalidUsageException>(() => EmaComplexTypeHandler.EncodeElementList(elementList, msgDataTypes, false));
            }
            finally
            {
                elementList.ClearAndReturnToPool_All();
            }

            EmaComplexTypeHandler.defaultRefreshMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultRequestMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultAckMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultStatusMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultGenericMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultPostMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.CompleteMsgEncoding = true;

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeAndDecodePostMsgWithMsgPayloadTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            foreach (var dataType in msgDataTypes)
            {
                MsgParameters msgParameters = new MsgParameters()
                {
                    MsgClass = MsgClasses.POST,
                    MsgDomainType = (int)DomainType.MARKET_PRICE,
                    StreamId = 1,
                    HasExtendedHeader = true,
                    HasMsgKey = true,
                    HasPayload = true,
                    PayloadType = dataType,
                    ContainerType = dataType,
                    HasMsgKeyType = true,
                    HasFilter = true,
                    HasAttrib = true,
                    AttribContainerType = dataType,
                    HasIdentifier = true,
                    MessageComplete = true,
                    HasPartNum = true,
                    HasPostId = true,
                    HasSeqNum = false,
                    Solicited = false,
                    HasPermData = true,
                    HasPostUserRights = true,
                    SolicitAck = true,
                    Preencoded = true
                };
                PostMsg postMsg = m_objectManager.GetOmmPostMsg();
                EmaComplexTypeHandler.EncodePostMessage(msgParameters, postMsg);

                var body = postMsg.Encoder!.m_encodeIterator!.Buffer();

                PostMsg decodedPostMsg = m_objectManager.GetOmmPostMsg();
                Assert.Equal(CodecReturnCode.SUCCESS, decodedPostMsg.Decode(body,
                    Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null));
                EmaComplexTypeHandler.DecodeAndCheckPostMessage(msgParameters, decodedPostMsg);

                postMsg.ClearAndReturnToPool_All();
                decodedPostMsg.ClearAndReturnToPool_All();
            }

            CheckEtaGlobalPoolSizes();
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        private void SetDefaultMsgParameters()
        {
            EmaComplexTypeHandler.defaultRefreshMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultRequestMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultAckMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultStatusMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultGenericMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultPostMsgParameters.CompleteMsgEncoding = false;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.CompleteMsgEncoding = false;

            EmaComplexTypeHandler.defaultRefreshMsgParameters.Preencoded = true;
            EmaComplexTypeHandler.defaultRequestMsgParameters.Preencoded = true;
            EmaComplexTypeHandler.defaultAckMsgParameters.Preencoded = true;
            EmaComplexTypeHandler.defaultStatusMsgParameters.Preencoded = true;
            EmaComplexTypeHandler.defaultGenericMsgParameters.Preencoded = true;
            EmaComplexTypeHandler.defaultPostMsgParameters.Preencoded = true;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.Preencoded = true;

            EmaComplexTypeHandler.defaultRefreshMsgParameters.SetCopyByteBuffersFlag = true;
            EmaComplexTypeHandler.defaultRequestMsgParameters.SetCopyByteBuffersFlag = true;
            EmaComplexTypeHandler.defaultAckMsgParameters.SetCopyByteBuffersFlag = true;
            EmaComplexTypeHandler.defaultStatusMsgParameters.SetCopyByteBuffersFlag = true;
            EmaComplexTypeHandler.defaultGenericMsgParameters.SetCopyByteBuffersFlag = true;
            EmaComplexTypeHandler.defaultPostMsgParameters.SetCopyByteBuffersFlag = true;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.SetCopyByteBuffersFlag = true;
        }

        private void ResetDefaultMsgParameters()
        {
            EmaComplexTypeHandler.defaultRefreshMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultRequestMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultAckMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultStatusMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultGenericMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultPostMsgParameters.CompleteMsgEncoding = true;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.CompleteMsgEncoding = true;

            EmaComplexTypeHandler.defaultRefreshMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultRequestMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultAckMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultStatusMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultGenericMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultPostMsgParameters.Preencoded = false;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.Preencoded = false;

            EmaComplexTypeHandler.defaultRefreshMsgParameters.SetCopyByteBuffersFlag = false;
            EmaComplexTypeHandler.defaultRequestMsgParameters.SetCopyByteBuffersFlag = false;
            EmaComplexTypeHandler.defaultAckMsgParameters.SetCopyByteBuffersFlag = false;
            EmaComplexTypeHandler.defaultStatusMsgParameters.SetCopyByteBuffersFlag = false;
            EmaComplexTypeHandler.defaultGenericMsgParameters.SetCopyByteBuffersFlag = false;
            EmaComplexTypeHandler.defaultPostMsgParameters.SetCopyByteBuffersFlag = false;
            EmaComplexTypeHandler.defaultUpdateMsgParameters.SetCopyByteBuffersFlag = false;
        }

        private void CheckEmaObjectManagerPoolSizes(EmaObjectManager objectManager)
        {
            int[] dataTypes = {
                DataType.DataTypes.INT,
                DataType.DataTypes.UINT,
                DataType.DataTypes.FLOAT,
                DataType.DataTypes.DOUBLE,
                DataType.DataTypes.TIME,
                DataType.DataTypes.DATETIME,
                DataType.DataTypes.REAL,
                DataType.DataTypes.QOS,
                DataType.DataTypes.STATE,
                DataType.DataTypes.ARRAY,
                DataType.DataTypes.ENUM,
                DataType.DataTypes.ASCII,
                DataType.DataTypes.RMTES,
                DataType.DataTypes.BUFFER,
                DataType.DataTypes.FILTER_LIST,
                DataType.DataTypes.ELEMENT_LIST,
                DataType.DataTypes.FIELD_LIST,
                DataType.DataTypes.MAP,
                DataType.DataTypes.VECTOR,
                DataType.DataTypes.OPAQUE,
                DataType.DataTypes.XML,
                DataType.DataTypes.ANSI_PAGE };

            foreach (var dataType in dataTypes)
            {
                Assert.Equal(EmaObjectManager.INITIAL_POOL_SIZE, objectManager.GetDataObjectPoolSize(dataType));
            }

            Assert.Equal(EmaObjectManager.INITIAL_POOL_SIZE, objectManager.primitivePool.Count);
            Assert.Equal(EmaObjectManager.INITIAL_POOL_SIZE, objectManager.complexTypePool.Count);
            Assert.Equal(EmaObjectManager.INITIAL_POOL_SIZE, objectManager.msgTypePool.Count);
        }

        private void CheckEtaGlobalPoolSizes()
        {
            var pool = EtaObjectGlobalPool.Instance;
            Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaBufferPool.Count);
            Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaEncodeIteratorPool.Count);
            foreach (var keyVal in pool.m_etaByteBufferBySizePool)
            {
                Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, keyVal.Value.Count);
            }
        }
    }
}
