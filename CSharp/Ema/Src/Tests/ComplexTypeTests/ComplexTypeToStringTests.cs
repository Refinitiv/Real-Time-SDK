/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System;
using System.Text;
using static LSEG.Ema.Access.OmmReal;
using DataDictionary = LSEG.Eta.Codec.DataDictionary;

namespace LSEG.Ema.Access.Tests.ComplexTypeTests
{
    public class ComplexTypeToStringTests
    {
        private EmaObjectManager m_objectManager = new EmaObjectManager();

        private int[] containerTypes = { DataType.DataTypes.FILTER_LIST,
                DataType.DataTypes.ELEMENT_LIST,
                DataType.DataTypes.FIELD_LIST,
                DataType.DataTypes.MAP,
                DataType.DataTypes.VECTOR,
                DataType.DataTypes.SERIES,
                DataType.DataTypes.OPAQUE,
                DataType.DataTypes.XML,
                DataType.DataTypes.ANSI_PAGE
        };

        private bool[] boolValues = { true, false };

        private void LoadEnumTypeDictionary(DataDictionary dataDictionary)
        {
            var result = dataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def", out CodecError error);
            Assert.False(result < 0, $"Unable to load enum dictionary. Error Text: {error?.Text}");
        }

        private void LoadFieldDictionary(DataDictionary dataDictionary)
        {
            var result = dataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary", out CodecError error);
            Assert.False(result < 0, $"Unable to load field dictionary. Error Text: {error?.Text}");
        }

        private void LoadEmaEnumTypeDictionary(Ema.Rdm.DataDictionary dataDictionary)
        {
            dataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def");
        }

        private void LoadEmaFieldDictionary(Ema.Rdm.DataDictionary dataDictionary)
        {
            dataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary");
        }

        [Fact]
        public void ElementListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[][] dataTypesArray = {
                new int[] { DataType.DataTypes.INT, DataType.DataTypes.FIELD_LIST, DataType.DataTypes.STATE, DataType.DataTypes.ENUM, DataType.DataTypes.DATE, DataType.DataTypes.DATETIME },
                new int[] { DataType.DataTypes.UINT, DataType.DataTypes.ASCII, DataType.DataTypes.FIELD_LIST, DataType.DataTypes.FIELD_LIST },
                new int[] { DataType.DataTypes.DOUBLE, DataType.DataTypes.REAL, DataType.DataTypes.VECTOR }
            };

            foreach (var dataTypes in dataTypesArray)
            {
                ElementList elementList = m_objectManager.GetOmmElementList();
                EmaComplexTypeHandler.EncodeElementList(elementList, dataTypes, false);

                string encodedDataDictionaryString = elementList.ToString(emaDataDictionary);

                var buffer = elementList.Encoder!.m_encodeIterator!.Buffer();
                var decodedElementList = m_objectManager.GetOmmElementList();

                decodedElementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);
                string decodedString = decodedElementList.ToString();

                CheckElementListString(dataTypes, decodedString);
                Assert.Equal(decodedString, encodedDataDictionaryString);

                elementList.ClearAndReturnToPool_All();
                decodedElementList.ClearAndReturnToPool_All();
            }
        }

        [Fact]
        public void SeriesTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] lengths = { 3, 8, 1 };
            int[] countHints = { 4, 5, 2 };
            foreach (var dataType in containerTypes)
                foreach (var countHintPresent in boolValues)
                    foreach (var summaryPresent in boolValues)
                        foreach (var countHint in countHints)
                            foreach (var length in lengths)
                            {
                                Series series = m_objectManager.GetOmmSeries();
                                EmaComplexTypeHandler.EncodeSeries(series, dataType, countHintPresent, summaryPresent, countHint, length, false);

                                var encodedString = series.ToString(emaDataDictionary);

                                var buffer = series.Encoder!.m_encodeIterator!.Buffer();
                                Series decodedSeries = m_objectManager.GetOmmSeries();
                                decodedSeries.DecodeSeries(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                var decodedString = decodedSeries.ToString();
                                CheckSeriesString(dataType, countHintPresent, summaryPresent, countHint, length, decodedString);

                                Assert.Equal(decodedString, encodedString);

                                series.ClearAndReturnToPool_All();
                                decodedSeries.ClearAndReturnToPool_All();
                            }
        }

        [Fact]
        public void MapTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] mapContainerTypes = {
                DataType.DataTypes.FILTER_LIST,
                DataType.DataTypes.ELEMENT_LIST,
                DataType.DataTypes.FIELD_LIST,
                DataType.DataTypes.MAP,
                DataType.DataTypes.VECTOR };

            int[][] mapActionsArray = {
                new int[]{ MapAction.ADD, MapAction.UPDATE, MapAction.DELETE }
            };

            int[] keyTypes = {
                DataType.DataTypes.TIME,
                DataType.DataTypes.DATETIME
            };

            bool[][] hasPermDataArray = {
                new bool[] { false, false, false },
                new bool[] { true, false, true },
            };

            foreach (var dataType in mapContainerTypes)
                foreach (var mapActions in mapActionsArray)
                    foreach (var keyType in keyTypes)
                        foreach (var summaryPresent in boolValues)
                            foreach (var keyFieldIdPresent in boolValues)
                                foreach (var countHintPresent in boolValues)
                                    foreach (var permDataPresent in hasPermDataArray)
                                    {
                                        Map map = m_objectManager.GetOmmMap();
                                        EmaComplexTypeHandler.EncodeMap(map,
                                            dataType,
                                            mapActions,
                                            permDataPresent,
                                            keyType,
                                            summaryPresent,
                                            keyFieldIdPresent,
                                            countHintPresent,
                                            false);

                                        string encodedString = map.ToString(emaDataDictionary);

                                        var buffer = map.Encoder!.m_encodeIterator!.Buffer();

                                        Map decodedMap = m_objectManager.GetOmmMap();
                                        decodedMap.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                        string decodedString = decodedMap.ToString();

                                        CheckMapString(dataType,
                                            mapActions,
                                            permDataPresent,
                                            keyType,
                                            summaryPresent,
                                            keyFieldIdPresent,
                                            countHintPresent, decodedString);

                                        Assert.Equal(encodedString, decodedString);

                                        map.ClearAndReturnToPool_All();
                                        decodedMap.ClearAndReturnToPool_All();
                                    }
        }

        [Fact]
        public void VectorTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[][] vectorActionsArray = new int[][]
            {
                new int[] { VectorAction.CLEAR, VectorAction.UPDATE, VectorAction.DELETE, VectorAction.INSERT, VectorAction.SET },
                new int[] { VectorAction.SET, VectorAction.SET, VectorAction.SET, VectorAction.SET, VectorAction.SET }
            };

            bool[][] hasPermDataArray = {
                new bool[] { false, false, false, false, false },
                new bool[] { true, false, true, false, true },
            };

            foreach (var dataType in containerTypes)
                foreach (var vectorActions in vectorActionsArray)
                    foreach (var hasPermData in hasPermDataArray)
                        foreach (var summaryPresent in boolValues)
                            foreach (var supportsSorting in boolValues)
                                foreach (var hasTotalCountHint in boolValues)
                                {
                                    Vector vector = m_objectManager.GetOmmVector();
                                    Vector decVector = m_objectManager.GetOmmVector();
                                    EmaComplexTypeHandler.EncodeVector(vector,
                                        dataType,
                                        true,
                                        hasTotalCountHint,
                                        supportsSorting,
                                        vectorActions,
                                        hasPermData,
                                        false,
                                        false);

                                    string encodedString = vector.ToString(emaDataDictionary);

                                    var buffer = vector.Encoder!.m_encodeIterator!.Buffer();
                                    decVector.DecodeVector(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                    string decodedString = decVector.ToString();

                                    Assert.Equal(decodedString, encodedString);

                                    CheckVectorString(dataType, true,
                                            hasTotalCountHint,
                                            supportsSorting,
                                            vectorActions,
                                            hasPermData, decodedString);

                                    EmaComplexTypeHandler.DecodeAndCheckVector(decVector,
                                        dataType,
                                        true,
                                        hasTotalCountHint,
                                        supportsSorting,
                                        vectorActions,
                                        hasPermData);

                                    vector.ClearAndReturnToPool_All();
                                    decVector.ClearAndReturnToPool_All();
                                }
        }

        [Fact]
        public void FilterListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[][] filterActionsArray = new int[][]
            {
                new int[] { FilterAction.SET, FilterAction.UPDATE, FilterAction.SET, FilterAction.UPDATE },
                new int[] { FilterAction.CLEAR, FilterAction.UPDATE, FilterAction.SET, FilterAction.CLEAR }
            };

            bool[][] hasPermDataArray = {
                new bool[] { true, true, true, true },
                new bool[] { true, false, true, false },
            };

            int[][] dataTypesArray = new int[][]
            {
                new int[] { DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.MAP, DataType.DataTypes.ELEMENT_LIST },
                new int[] { DataType.DataTypes.VECTOR, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.VECTOR, DataType.DataTypes.VECTOR },
                new int[] { DataType.DataTypes.MAP, DataType.DataTypes.MAP, DataType.DataTypes.FIELD_LIST, DataType.DataTypes.MAP },
                new int[] { DataType.DataTypes.XML, DataType.DataTypes.MAP, DataType.DataTypes.MAP, DataType.DataTypes.MAP }
            };

            int[] countHintArray = new int[] { 3, 5, 6 };

            bool[][] noDataEntriesArray =
            {
                new bool[] { false, false, false, false }
            };

            foreach (var hasCountHint in boolValues)
                foreach (var filterActions in filterActionsArray)
                    foreach (var dataTypes in dataTypesArray)
                        foreach (var hasPermData in hasPermDataArray)
                            foreach (var countHint in countHintArray)
                                foreach (var noDataEntries in noDataEntriesArray)
                                {
                                    FilterList filterList = m_objectManager.GetOmmFilterList();
                                    EmaComplexTypeHandler.EncodeFilterList(filterList,
                                        DataType.DataTypes.NO_DATA, // variable not necessary
                                        hasCountHint,
                                        filterActions,
                                        dataTypes,
                                        hasPermData,
                                        countHint,
                                        false,
                                        noDataEntries);

                                    string encodedString = filterList.ToString(emaDataDictionary);

                                    var buffer = filterList.Encoder!.m_encodeIterator!.Buffer();

                                    var decFilterList = m_objectManager.GetOmmFilterList();
                                    decFilterList.DecodeFilterList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                    string decodedString = decFilterList.ToString();

                                    Assert.Equal(decodedString, encodedString);

                                    CheckFilterListString(hasCountHint, filterActions,
                                        dataTypes,
                                        hasPermData,
                                        countHint, encodedString);

                                    EmaComplexTypeHandler.DecodeAndCheckFilterList(decFilterList,
                                        DataType.DataTypes.NO_DATA, // variable not necessary
                                        hasCountHint,
                                        filterActions,
                                        dataTypes,
                                        hasPermData,
                                        countHint,
                                        noDataEntries);

                                    filterList.ClearAndReturnToPool_All();
                                    decFilterList.ClearAndReturnToPool_All();
                                }
        }

        [Fact]
        public void FieldListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[][] dataTypesArray = new int[][]
            {
                new int[] { DataType.DataTypes.REAL, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.DATE, DataType.DataTypes.TIME },
                new int[] { DataType.DataTypes.MAP, DataType.DataTypes.REAL, DataType.DataTypes.VECTOR },
                new int[] { DataType.DataTypes.ARRAY, DataType.DataTypes.TIME, DataType.DataTypes.VECTOR, DataType.DataTypes.ARRAY }
            };

            foreach (var dataTypes in dataTypesArray)
            {
                FieldList fieldList = m_objectManager.GetOmmFieldList();
                EmaComplexTypeHandler.EncodeFieldList(fieldList, dataTypes, false);

                string encodedString = fieldList.ToString(emaDataDictionary);

                var buffer = fieldList.Encoder!.m_encodeIterator!.Buffer();

                var decFieldList = m_objectManager.GetOmmFieldList();

                decFieldList.DecodeFieldList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                string decodedString = decFieldList.ToString();
                Assert.Equal(encodedString, decodedString);
                CheckFieldListString(dataTypes, encodedString);

                EmaComplexTypeHandler.DecodeAndCheckFieldList(decFieldList, dataTypes);

                fieldList.ClearAndReturnToPool_All();
                decFieldList.ClearAndReturnToPool_All();
            }
        }

        [Fact]
        public void AckMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmAckMsg();
            var expected = Decode(msg);
            var actual = msg.ToString();
            Assert.Equal(expected, actual);
        }

        [Fact]
        public void AckMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.MARKET_BY_ORDER, (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.OPAQUE, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.VECTOR, DataTypes.SERIES };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in boolValues)
                        foreach (bool hasNackCode in boolValues)
                            foreach (bool hasSeqNum in boolValues)
                                foreach (bool privateStream in boolValues)
                                    foreach (bool hasText in boolValues)
                                        foreach (bool hasMsgKey in boolValues)
                                            foreach (bool hasAttrib in boolValues)
                                                foreach (int attribType in attribTypes)
                                                {
                                                    MsgParameters msgParameters = new MsgParameters()
                                                    {
                                                        MsgClass = MsgClasses.ACK,
                                                        MsgDomainType = dt,
                                                        StreamId = 8,
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
                                                        CompleteMsgEncoding = false
                                                    };

                                                    AckMsg ackMsg = m_objectManager.GetOmmAckMsg();
                                                    EmaComplexTypeHandler.EncodeAckMessage(msgParameters, ackMsg);

                                                    var body = ackMsg.m_ackMsgEncoder.m_encodeIterator!.Buffer();

                                                    AckMsg decodeMsg = m_objectManager.GetOmmAckMsg();
                                                    decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                    string encodedString = ackMsg.ToString(emaDataDictionary);
                                                    string decodedString = decodeMsg.ToString();

                                                    Assert.Equal(encodedString, decodedString);
                                                    CheckAckMsgString(decodedString, msgParameters);

                                                    ackMsg.ClearAndReturnToPool_All();
                                                    decodeMsg.ClearAndReturnToPool_All();
                                                }
        }

        [Fact]
        public void GenericMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmGenericMsg();
            var expected = Decode(msg);
            var actualStr = msg.ToString();
            Assert.Equal(expected, actualStr);
        }

        [Fact]
        public void GenericMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.LOGIN };
            int[] payloadTypes = { DataTypes.ELEMENT_LIST, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.FIELD_LIST };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in boolValues)
                        foreach (bool hasPermData in boolValues)
                            foreach (bool providerDriven in boolValues)
                                foreach (bool hasSecSeqNum in boolValues)
                                    foreach (bool hasPartNum in boolValues)
                                        foreach (bool hasSeqNum in boolValues)
                                            foreach (bool isComplete in boolValues)
                                                foreach (bool hasMsgKey in boolValues)
                                                    foreach (bool hasAttrib in boolValues)
                                                        foreach (int attribType in attribTypes)
                                                        {
                                                            MsgParameters msgParameters = new MsgParameters()
                                                            {
                                                                MsgClass = MsgClasses.GENERIC,
                                                                MsgDomainType = dt,
                                                                StreamId = 4,
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
                                                                CompleteMsgEncoding = false
                                                            };

                                                            GenericMsg genericMsg = m_objectManager.GetOmmGenericMsg();
                                                            EmaComplexTypeHandler.EncodeGenericMessage(msgParameters, genericMsg);

                                                            var body = genericMsg.m_genericMsgEncoder.m_encodeIterator!.Buffer();

                                                            GenericMsg decodeMsg = m_objectManager.GetOmmGenericMsg();
                                                            decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                            string encodedString = genericMsg.ToString(emaDataDictionary);
                                                            string decodedString = decodeMsg.ToString();

                                                            Assert.Equal(encodedString, decodedString);
                                                            CheckGenericMsgString(decodedString, msgParameters);

                                                            genericMsg.ClearAndReturnToPool_All();
                                                            decodeMsg.ClearAndReturnToPool_All();
                                                        }
        }

        [Fact]
        public void PostMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmPostMsg();
            var expected = Decode(msg);
            var actualStr = msg.ToString();
            Assert.Equal(expected, actualStr);
        }

        [Fact]
        public void PostMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.XML, DataTypes.SERIES };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in boolValues)
                        foreach (bool hasPermData in boolValues)
                            foreach (bool hasPostId in boolValues)
                                foreach (bool hasSeqNum in boolValues)
                                    foreach (bool hasPartNum in boolValues)
                                        foreach (bool hasPostUserRights in boolValues)
                                            foreach (bool solicitAck in boolValues)
                                                foreach (bool isComplete in boolValues)
                                                    foreach (bool hasMsgKey in boolValues)
                                                        foreach (bool hasAttrib in boolValues)
                                                            foreach (int attribType in attribTypes)
                                                            {
                                                                MsgParameters msgParameters = new MsgParameters()
                                                                {
                                                                    MsgClass = MsgClasses.POST,
                                                                    MsgDomainType = dt,
                                                                    StreamId = 4,
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
                                                                    SolicitAck = solicitAck,
                                                                    CompleteMsgEncoding = false
                                                                };

                                                                PostMsg postMsg = m_objectManager.GetOmmPostMsg();
                                                                EmaComplexTypeHandler.EncodePostMessage(msgParameters, postMsg);

                                                                var body = postMsg.m_postMsgEncoder.m_encodeIterator!.Buffer();

                                                                PostMsg decodeMsg = m_objectManager.GetOmmPostMsg();
                                                                decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                                string encodedString = postMsg.ToString(emaDataDictionary);
                                                                string decodedString = decodeMsg.ToString();

                                                                Assert.Equal(encodedString, decodedString);
                                                                CheckPostMsgString(decodedString, msgParameters);

                                                                postMsg.ClearAndReturnToPool_All();
                                                                decodeMsg.ClearAndReturnToPool_All();
                                                            }
        }

        [Fact]
        public void RefreshMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmRefreshMsg();
            var expected = Decode(msg);
            var actualStr = msg.ToString();
            Assert.Equal(expected, actualStr);
        }

        [Fact]
        public void RefreshMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.MAP };
            int[] attribTypes = { DataTypes.SERIES };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasQos in boolValues)
                        foreach (bool privateStream in boolValues)
                            foreach (bool hasSeqNum in boolValues)
                                foreach (bool hasPartNum in boolValues)
                                    foreach (bool doNotCache in boolValues)
                                        foreach (bool clearCache in boolValues)
                                            foreach (bool hasGroupId in boolValues)
                                                foreach (bool msgComplete in boolValues)
                                                    foreach (bool hasMsgKey in boolValues)
                                                        foreach (bool hasAttrib in boolValues)
                                                            foreach (int attribType in attribTypes)
                                                            {
                                                                MsgParameters msgParameters = new MsgParameters()
                                                                {
                                                                    MsgClass = MsgClasses.REFRESH,
                                                                    MsgDomainType = dt,
                                                                    StreamId = 5,
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
                                                                    HasPublisherId = false,
                                                                    HasPartNum = hasPartNum,
                                                                    Solicited = true,
                                                                    HasSeqNum = hasSeqNum,
                                                                    CompleteMsgEncoding = false
                                                                };

                                                                RefreshMsg refMsg = m_objectManager.GetOmmRefreshMsg();
                                                                EmaComplexTypeHandler.EncodeRefreshMessage(msgParameters, refMsg);

                                                                var body = refMsg.m_refreshMsgEncoder.m_encodeIterator!.Buffer();

                                                                RefreshMsg decodeMsg = m_objectManager.GetOmmRefreshMsg();
                                                                decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                                string encodedString = refMsg.ToString(emaDataDictionary);
                                                                string decodedString = decodeMsg.ToString();

                                                                Assert.Equal(encodedString, decodedString);
                                                                CheckRefreshMsgString(decodedString, msgParameters);

                                                                refMsg.ClearAndReturnToPool_All();
                                                                decodeMsg.ClearAndReturnToPool_All();
                                                            }
        }

        [Fact]
        public void RequestMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmRequestMsg();
            var expected = Decode(msg);
            var actualStr = msg.ToString();
            Assert.Equal(expected, actualStr);
        }

        [Fact]
        public void RequestMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.LOGIN, (int)DomainType.MARKET_PRICE };
            int[] payloadTypes = { DataTypes.ELEMENT_LIST };
            int[] attribTypes = { DataTypes.FIELD_LIST };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in boolValues)
                        foreach (bool streaming in boolValues)
                            foreach (bool hasQos in boolValues)
                                foreach (bool noRefresh in boolValues)
                                    foreach (bool hasPriority in boolValues)
                                        foreach (bool privateStream in boolValues)
                                            foreach (bool pause in boolValues)
                                                foreach (bool hasMsgKeyType in boolValues)
                                                    foreach (bool hasFilter in boolValues)
                                                        foreach (bool hasId in boolValues)
                                                            foreach (bool hasAttrib in boolValues)
                                                                foreach (int attribType in attribTypes)
                                                                {
                                                                    MsgParameters msgParameters = new MsgParameters()
                                                                    {
                                                                        MsgClass = MsgClasses.REQUEST,
                                                                        MsgDomainType = dt,
                                                                        StreamId = 5,
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
                                                                        HasIdentifier = hasId,
                                                                        CompleteMsgEncoding = false
                                                                    };

                                                                    RequestMsg reqMsg = m_objectManager.GetOmmRequestMsg();
                                                                    EmaComplexTypeHandler.EncodeRequestMessage(msgParameters, reqMsg);

                                                                    var body = reqMsg.m_requestMsgEncoder.m_encodeIterator!.Buffer();

                                                                    RequestMsg decodeMsg = m_objectManager.GetOmmRequestMsg();
                                                                    decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                                    string encodedString = reqMsg.ToString(emaDataDictionary);
                                                                    string decodedString = decodeMsg.ToString();

                                                                    Assert.Equal(encodedString, decodedString);
                                                                    CheckRequestMsgString(decodedString, msgParameters);

                                                                    reqMsg.ClearAndReturnToPool_All();
                                                                    decodeMsg.ClearAndReturnToPool_All();
                                                                }
        }

        [Fact]
        public void UpdateMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmUpdateMsg();
            var expected = Decode(msg);
            var actualStr = msg.ToString();
            Assert.Equal(expected, actualStr);
        }

        [Fact]
        public void UpdateMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.SOURCE };
            int[] payloadTypes = { DataTypes.ELEMENT_LIST };
            int[] attribTypes = { DataTypes.FIELD_LIST };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in boolValues)
                        foreach (bool hasPermData in boolValues)
                            foreach (bool hasSeqNum in boolValues)
                                foreach (bool hasConflated in boolValues)
                                    foreach (bool doNotCache in boolValues)
                                        foreach (bool doNotConflate in boolValues)
                                            foreach (bool doNotRipple in boolValues)
                                                foreach (bool hasMsgKey in boolValues)
                                                    foreach (bool hasAttrib in boolValues)
                                                        foreach (int attribType in attribTypes)
                                                        {
                                                            MsgParameters msgParameters = new MsgParameters()
                                                            {
                                                                MsgClass = MsgClasses.UPDATE,
                                                                MsgDomainType = dt,
                                                                StreamId = 3,
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
                                                                HasPublisherId = false,
                                                                HasPartNum = false,
                                                                Solicited = false,
                                                                HasSeqNum = hasSeqNum,
                                                                DoNotConflate = doNotConflate,
                                                                DoNotRipple = doNotRipple,
                                                                HasPermData = hasPermData,
                                                                HasConfInfo = hasConflated,
                                                                CompleteMsgEncoding = false
                                                            };

                                                            UpdateMsg updateMsg = m_objectManager.GetOmmUpdateMsg();
                                                            EmaComplexTypeHandler.EncodeUpdateMessage(msgParameters, updateMsg);

                                                            var body = updateMsg.m_updateMsgEncoder.m_encodeIterator!.Buffer();

                                                            UpdateMsg decodeMsg = m_objectManager.GetOmmUpdateMsg();
                                                            decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                            string encodedString = updateMsg.ToString(emaDataDictionary);
                                                            string decodedString = decodeMsg.ToString();

                                                            Assert.Equal(encodedString, decodedString);
                                                            CheckUpdateMsgString(decodedString, msgParameters);

                                                            updateMsg.ClearAndReturnToPool_All();
                                                            decodeMsg.ClearAndReturnToPool_All();
                                                        }
        }

        [Fact]
        public void StatusMsgNoDecodedDataSetTest()
        {
            var msg = m_objectManager.GetOmmStatusMsg();
            var expected = Decode(msg);
            var actualStr = msg.ToString();
            Assert.Equal(expected, actualStr);
        }

        [Fact]
        public void StatusMsgTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            Ema.Rdm.DataDictionary emaDataDictionary = new Ema.Rdm.DataDictionary();

            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            LoadEmaEnumTypeDictionary(emaDataDictionary);
            LoadEmaFieldDictionary(emaDataDictionary);

            int[] domainTypes = { (int)DomainType.LOGIN };
            int[] payloadTypes = { DataTypes.ANSI_PAGE, DataTypes.FILTER_LIST };
            int[] attribTypes = { DataTypes.FIELD_LIST, DataTypes.MAP };

            foreach (int dt in domainTypes)
                foreach (int pt in payloadTypes)
                    foreach (bool hasExtHeader in boolValues)
                        foreach (bool hasPermData in boolValues)
                            foreach (bool clearCache in boolValues)
                                foreach (bool hasItemGroup in boolValues)
                                    foreach (bool hasState in boolValues)
                                        foreach (bool privateStream in boolValues)
                                            foreach (bool hasMsgKey in boolValues)
                                                foreach (bool hasAttrib in boolValues)
                                                    foreach (int attribType in attribTypes)
                                                    {
                                                        MsgParameters msgParameters = new MsgParameters()
                                                        {
                                                            MsgClass = MsgClasses.STATUS,
                                                            MsgDomainType = dt,
                                                            StreamId = 4,
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
                                                            HasPublisherId = false,
                                                            HasPartNum = false,
                                                            Solicited = false,
                                                            HasPermData = hasPermData,
                                                            CompleteMsgEncoding = false
                                                        };

                                                        StatusMsg statusMsg = m_objectManager.GetOmmStatusMsg();
                                                        EmaComplexTypeHandler.EncodeStatusMessage(msgParameters, statusMsg);

                                                        var body = statusMsg.m_statusMsgEncoder.m_encodeIterator!.Buffer();

                                                        StatusMsg decodeMsg = m_objectManager.GetOmmStatusMsg();
                                                        decodeMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null);

                                                        string encodedString = statusMsg.ToString(emaDataDictionary);
                                                        string decodedString = decodeMsg.ToString();

                                                        Assert.Equal(encodedString, decodedString);
                                                        CheckStatusMsgString(decodedString, msgParameters);

                                                        statusMsg.ClearAndReturnToPool_All();
                                                        decodeMsg.ClearAndReturnToPool_All();
                                                    }
        }


        [Fact]
        public void FieldList_Just_Encoded_ToString_Test()
        {
            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, MagnitudeTypes.EXPONENT_0);
            fieldList.Complete();

            Assert.Equal("\nFieldList.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                fieldList.ToString());
        }

        [Fact]
        public void ElementList_Just_Encoded_ToString_Test()
        {
            ElementList elementList = new ElementList();
            elementList.AddAscii("addAscii", "ascii value");
            elementList.AddInt("addInt", 50000000);
            elementList.AddUInt("addUInt", 1);
            elementList.Complete();

            Assert.Equal("\nElementList.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                elementList.ToString());
        }

        [Fact]
        public void Map_Just_Encoded_ToString_Test()
        {
            Map map = new Map();
            ElementList elementList = new ElementList();
            elementList.AddAscii("addAscii", "ascii value");
            map.AddKeyAscii("QATestMap", 1, elementList.Complete());
            map.Complete();

            Assert.Equal("\nMap.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                map.ToString());
        }

        [Fact]
        public void FilterList_Just_Encoded_ToString_Test()
        {
            FilterList filterList = new FilterList();
            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, MagnitudeTypes.EXPONENT_NEG_2);
            filterList.AddEntry(1, 1, fieldList.Complete(), new EmaBuffer());
            filterList.Complete();

            Assert.Equal("\nFilterList.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                filterList.ToString());
        }

        [Fact]
        public void Series_Just_Encoded_ToString_Test()
        {
            Series series = new Series();
            ElementList elementList = new ElementList();
            elementList.AddAscii("addAscii", "ascii value");
            series.AddEntry(elementList.Complete());
            series.Complete();

            Assert.Equal("\nSeries.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                series.ToString());
        }

        [Fact]
        public void Vector_Just_Encoded_ToString_Test()
        {
            Vector vector = new Vector();
            FilterList filterList = new FilterList();
            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, MagnitudeTypes.EXPONENT_NEG_2);
            filterList.AddEntry(1, 1, fieldList.Complete(), new EmaBuffer());
            filterList.Complete();
            vector.Add(1, 1, filterList, null);
            vector.Complete();

            Assert.Equal("\nVector.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                vector.ToString());
        }

        [Fact]
        public void OmmArray_Just_Encoded_ToString_Test()
        {
            OmmArray ommArray = new OmmArray();
            ommArray.AddAscii("1").AddAscii("2").AddAscii("3").Complete();

            Assert.Equal("\nOmmArray.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                ommArray.ToString());
        }

        [Fact]
        public void EMA_Msg_Just_Encoded_ToString_Test()
        {
            RefreshMsg refreshMsg = new RefreshMsg();
            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, MagnitudeTypes.EXPONENT_NEG_2);
            refreshMsg.DomainType(6).ServiceId(1).Name("IBM.N").State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
                .Solicited(true).Payload(fieldList.Complete()).Complete();

            Assert.Equal("\nRefreshMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                refreshMsg.ToString());

            GenericMsg genericMsg = new GenericMsg();
            genericMsg.DomainType(133).ServiceId(1).Name("IBM.N").Payload(fieldList).Complete();

            Assert.Equal("\nGenericMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                genericMsg.ToString());

            StatusMsg statusMsg = new StatusMsg();
            statusMsg.Name("IBM.N").ServiceId(1).DomainType(6).State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT,
                OmmState.StatusCodes.NOT_FOUND, "Item Not Found");
            Assert.Equal("\nStatusMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                statusMsg.ToString());

            UpdateMsg updateMsg = new UpdateMsg();
            updateMsg.Payload(fieldList);
            Assert.Equal("\nUpdateMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                updateMsg.ToString());

            PostMsg postMsg = new PostMsg();
            postMsg.PostId(55).ServiceId(1).Name("IBM.N").SolicitAck(true).Complete(true).Payload(updateMsg);
            Assert.Equal("\nPostMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                postMsg.ToString());

            AckMsg ackMsg = new AckMsg();
            ackMsg.SeqNum(1).Name("IBM.N").ServiceId(1).AckId(1).DomainType(6);
            Assert.Equal("\nAckMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                ackMsg.ToString());

            RequestMsg requestMsg = new RequestMsg();
            requestMsg.DomainType(133).ServiceId(1).Name("IBM.N");
            Assert.Equal("\nRequestMsg.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.",
                requestMsg.ToString());
        }

        private void CheckAckMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("AckMsg", msgString);
            Assert.Contains("AckMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}ackId=\"123\"", msgString);

            if (msgParameters.HasSeqNum) Assert.Contains($"{GetIndent(indent + 1)}seqNum=\"11\"", msgString);
            if (msgParameters.HasNackCode) Assert.Contains($"{GetIndent(indent + 1)}nackCode=\"AccessDenied\"", msgString);
            if (msgParameters.HasText) Assert.Contains($"{GetIndent(indent + 1)}text=\"Some text\"", msgString);
            if (msgParameters.HasIdentifier) Assert.Contains($"{GetIndent(indent + 1)}id=\"7\"", msgString);
            if (msgParameters.HasFilter) Assert.Contains($"{GetIndent(indent + 1)}filter=\"7\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"AckMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);

            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}nameType=\"1\"", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckGenericMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("GenericMsg", msgString);
            Assert.Contains("GenericMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);

            if (msgParameters.HasSeqNum) Assert.Contains($"{GetIndent(indent + 1)}seqNum=\"11\"", msgString);
            if (msgParameters.HasPartNum) Assert.Contains($"{GetIndent(indent + 1)}partNum=\"11\"", msgString);
            if (msgParameters.HasSecondarySeqNum) Assert.Contains($"{GetIndent(indent + 1)}secondarySeqNum=\"22\"", msgString);
            if (msgParameters.HasPermData) Assert.Contains($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString);
            if (msgParameters.HasText) Assert.Contains($"{GetIndent(indent + 1)}text=\"Some text\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"GenericMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);
            if (msgParameters.ProviderDriven) Assert.Contains($"{GetIndent(indent + 1)}ProviderDriven", msgString);
            if (msgParameters.MessageComplete) Assert.Contains($"{GetIndent(indent + 1)}MessageComplete", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckPostMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("PostMsg", msgString);
            Assert.Contains("PostMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);

            if (msgParameters.HasSeqNum) Assert.Contains($"{GetIndent(indent + 1)}seqNum=\"11\"", msgString);
            if (msgParameters.HasPostId) Assert.Contains($"{GetIndent(indent + 1)}postId=\"9\"", msgString);
            if (msgParameters.HasPartNum) Assert.Contains($"{GetIndent(indent + 1)}partNum=\"11\"", msgString);
            if (msgParameters.HasPermData) Assert.Contains($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"PostMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);
            if (msgParameters.MessageComplete) Assert.Contains($"{GetIndent(indent + 1)}MessageComplete", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckRefreshMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("RefreshMsg", msgString);
            Assert.Contains("RefreshMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}state=\"Closed, Recoverable / Suspect / None / ''\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}itemGroup=", msgString);

            if (msgParameters.HasSeqNum) Assert.Contains($"{GetIndent(indent + 1)}seqNum=\"23\"", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}seqNum=\"", msgString);
            if (msgParameters.Solicited) Assert.Contains($"{GetIndent(indent + 1)}solicited", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}solicited", msgString);
            if (msgParameters.DoNotCache) Assert.Contains($"{GetIndent(indent + 1)}doNotCache", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}doNotCache", msgString);
            if (msgParameters.ClearCache) Assert.Contains($"{GetIndent(indent + 1)}clearCache", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}clearCache", msgString);
            if (msgParameters.HasPartNum) Assert.Contains($"{GetIndent(indent + 1)}partNum=\"51\"", msgString);
            if (msgParameters.MessageComplete) Assert.Contains($"{GetIndent(indent + 1)}RefreshComplete", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}RefreshComplete", msgString);
            if (msgParameters.HasPermData) Assert.Contains($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"RefreshMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);
            if (msgParameters.PrivateStream) Assert.Contains($"{GetIndent(indent + 1)}privateStream", msgString);
            if (msgParameters.HasQos) Assert.Contains($"{GetIndent(indent + 1)}qos=\"RealTime/TickByTick\"", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckRequestMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("ReqMsg", msgString);
            Assert.Contains("ReqMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);

            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"RequestMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);
            if (msgParameters.PrivateStream) Assert.Contains($"{GetIndent(indent + 1)}privateStream", msgString);
            if (msgParameters.Streaming) Assert.Contains($"{GetIndent(indent + 1)}streaming", msgString);
            if (msgParameters.Pause) Assert.Contains($"{GetIndent(indent + 1)}pause", msgString);
            if (msgParameters.HasQos) Assert.Contains($"{GetIndent(indent + 1)}qos=\"RealTime/TickByTick\"", msgString);
            if (msgParameters.HasPriority) Assert.Contains($"{GetIndent(indent + 1)}priority class=\"1\" count=\"2\"", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckUpdateMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("UpdateMsg", msgString);
            Assert.Contains("UpdateMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}updateTypeNum=\"5\"", msgString);

            if (msgParameters.HasSeqNum) Assert.Contains($"{GetIndent(indent + 1)}seqNum=\"23\"", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}seqNum=\"", msgString);
            if (msgParameters.HasConfInfo) Assert.Contains($"{GetIndent(indent + 1)}confInfo count=\"3\" time=\"4\"", msgString);
            if (msgParameters.DoNotCache) Assert.Contains($"{GetIndent(indent + 1)}doNotCache", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}doNotCache", msgString);
            if (msgParameters.DoNotRipple) Assert.Contains($"{GetIndent(indent + 1)}doNotRipple", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}doNotRipple", msgString);
            if (msgParameters.DoNotConflate) Assert.Contains($"{GetIndent(indent + 1)}doNotConflate", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}doNotConflate", msgString);
            if (msgParameters.HasPermData) Assert.Contains($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"UpdateMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckStatusMsgString(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            Assert.Contains("StatusMsg", msgString);
            Assert.Contains("StatusMsgEnd", msgString);

            Assert.Contains($"{GetIndent(indent + 1)}streamId=\"{msgParameters.StreamId}\"", msgString);
            Assert.Contains($"{GetIndent(indent + 1)}domain=\"{Utilities.RdmDomainAsString(msgParameters.MsgDomainType)}\"", msgString);

            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}name=\"StatusMsg\"", msgString);
            if (msgParameters.HasMsgKey) Assert.Contains($"{GetIndent(indent + 1)}serviceId=\"1\"", msgString);
            if (msgParameters.PrivateStream) Assert.Contains($"{GetIndent(indent + 1)}privateStream", msgString);
            if (msgParameters.Streaming) Assert.Contains($"{GetIndent(indent + 1)}streaming", msgString);
            if (msgParameters.Pause) Assert.Contains($"{GetIndent(indent + 1)}pause", msgString);
            if (msgParameters.HasPermData) Assert.Contains($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}permissionData=\"0x70\"", msgString);
            if (msgParameters.HasState) Assert.Contains($"{GetIndent(indent + 1)}state=\"Closed / No Change / Invalid view / \'Something went wrong\'\"", msgString);
            if (msgParameters.ClearCache) Assert.Contains($"{GetIndent(indent + 1)}clearCache", msgString); else Assert.DoesNotContain($"{GetIndent(indent + 1)}clearCache", msgString);

            CheckExtendedHeader(msgString, msgParameters, indent);
            CheckAttribAndPayload(msgString, msgParameters, indent);
        }

        private void CheckExtendedHeader(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            if (msgParameters.HasExtendedHeader)
            {
                string headerStr = GetContainerTypeEntry(GetIndent(indent + 1) + "ExtendedHeader", GetIndent(indent + 1) + "ExtendedHeaderEnd", msgString);
                Assert.Contains("\n" + GetIndent(indent + 2) + "0x65 0x78 0x74 0x68 0x64 0x72", headerStr);
            }
        }

        private void CheckAttribAndPayload(string msgString, MsgParameters msgParameters, int indent = 0)
        {
            if (msgParameters.HasAttrib)
            {
                GetContainerLimits(msgParameters.AttribContainerType, out string b, out string e);
                string sum = GetContainerTypeEntry(GetIndent(indent + 1) + "Attrib ", GetIndent(indent + 1) + "AttribEnd", msgString);
                Assert.Contains($"Attrib dataType=\"{Access.DataType.AsString(msgParameters.AttribContainerType)}\"", msgString);
                string innerContainer = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, sum);
                CheckDefaultContainerString(msgParameters.AttribContainerType, innerContainer, indent + 2);
            }

            if (msgParameters.HasPayload)
            {
                GetContainerLimits(msgParameters.ContainerType, out string b, out string e);
                string sum = GetContainerTypeEntry(GetIndent(indent + 1) + "Payload ", GetIndent(indent + 1) + "PayloadEnd", msgString);
                Assert.Contains($"Payload dataType=\"{Access.DataType.AsString(msgParameters.ContainerType)}\"", msgString);
                string innerContainer = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, sum);
                CheckDefaultContainerString(msgParameters.ContainerType, innerContainer, indent + 2);
            }
        }

        private void CheckElementListString(int[] dataTypes, string elString, int indent = 0)
        {
            string indentStr = GetIndent(indent);
            string entryIndent = GetIndent(indent + 1);

            Assert.Contains(indentStr + "ElementList", elString);
            Assert.Contains("\n" + indentStr + "ElementListEnd", elString);

            string currString = elString.Substring(elString.IndexOf("\n" + entryIndent + "ElementEntry"));
            string currEntryStartString = "\n" + entryIndent + "ElementEntry";
            string currEntryEndString = "\n" + entryIndent + "ElementEntryEnd";

            for (int i = 0; i < dataTypes.Length; i++)
            {
                string name = EmaComplexTypeHandler.elDataTypeNameMap[dataTypes[i]];
                string dataType = Access.DataType.AsString(dataTypes[i]);
                string s, s2;
                if (dataTypes[i] < DataTypes.BASE_PRIMITIVE_MAX && dataTypes[i] != DataTypes.ARRAY)
                {
                    s = GetValueTypeEntry(currEntryStartString, "\n", currString);
                    Assert.Contains($"ElementEntry name=\"{name}\" dataType=\"{dataType}\" value=\"{GetPrimitiveValueString(dataTypes[i])}\"", s);
                    currString = currString.Substring(currString.IndexOf("\n") + 1);
                }
                else
                {
                    GetContainerLimits(dataTypes[i], out string b, out string e);
                    s = GetContainerTypeEntry(currEntryStartString, currEntryEndString, currString);
                    s2 = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, currString);

                    Assert.Contains($"ElementEntry name=\"{name}\" dataType=\"{dataType}\"", s);

                    CheckDefaultContainerString(dataTypes[i], s2, indent + 2);
                    currString = currString.Substring(currString.IndexOf(currEntryEndString) + currEntryEndString.Length);
                }
            }
        }

        private void CheckFieldListString(int[] dataTypes, string elString, int indent = 0)
        {
            string indentStr = GetIndent(indent);
            string entryIndent = GetIndent(indent + 1);

            Assert.Contains(indentStr + "FieldList", elString);
            Assert.Contains("\n" + indentStr + "FieldListEnd", elString);

            string currEntryStartString = "\n" + entryIndent + "FieldEntry";
            string currEntryEndString = "\n" + entryIndent + "FieldEntryEnd";

            string currString = elString.Substring(elString.IndexOf(currEntryStartString));

            for (int i = 0; i < dataTypes.Length; i++)
            {
                string fid = EmaComplexTypeHandler.elDataTypeFidMap[dataTypes[i]].ToString();
                string name = EmaComplexTypeHandler.dataTypeNameMap[dataTypes[i]];
                string dataType = Access.DataType.AsString(dataTypes[i]);
                string s, s2;
                if (dataTypes[i] < DataTypes.BASE_PRIMITIVE_MAX && dataTypes[i] != DataTypes.ARRAY)
                {
                    s = GetValueTypeEntry(currEntryStartString, "\n", currString);
                    Assert.Contains($"FieldEntry fid=\"{fid}\" name=\"{name}\" dataType=\"{dataType}\" value=\"{GetPrimitiveValueString(dataTypes[i])}\"", s);
                    currString = currString.Substring(currString.IndexOf("\n") + 1);
                }
                else
                {
                    GetContainerLimits(dataTypes[i], out string b, out string e);
                    s = GetContainerTypeEntry(currEntryStartString, currEntryEndString, currString);
                    s2 = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, s);
                    Assert.Contains($"FieldEntry fid=\"{fid}\" name=\"{name}\" dataType=\"{dataType}\"", s);
                    CheckDefaultContainerString(dataTypes[i], s2, indent + 2);
                    currString = currString.Substring(currString.IndexOf(currEntryEndString) + currEntryEndString.Length);
                }
            }
        }

        private void CheckFilterListString(bool hasCountHint, int[] filterActions,
            int[] dataTypes, bool[] permDataPresent,
            int countHint, string source, int indent = 0)
        {
            string indentStr = GetIndent(indent);
            string entryIndent = GetIndent(indent + 1);

            Assert.Contains(indentStr + "FilterList", source);
            Assert.Contains("\n" + indentStr + "FilterListEnd", source);

            if (hasCountHint)
            {
                Assert.Contains($"totalCountHint=\"{countHint}\"", source);
            }

            string currEntryStartString = "\n" + entryIndent + "FilterEntry";
            string currEntryEndString = "\n" + entryIndent + "FilterEntryEnd";

            string currString = source.Substring(source.IndexOf(currEntryStartString));

            for (int i = 0; i < dataTypes.Length; i++)
            {
                string dataType = Access.DataType.AsString(dataTypes[i]);
                string s, s2;

                GetContainerLimits(dataTypes[i], out string b, out string e);
                s = GetContainerTypeEntry(currEntryStartString, currEntryEndString, currString);

                if (filterActions[i] != (int)FilterEntryActions.CLEAR)
                {
                    if (!permDataPresent[i]) Assert.Contains($"FilterEntry action=\"{FilterAction.FilterActionToString(filterActions[i])}\" filterId=\"{i}\" dataType=\"{dataType}\"", s);
                    else Assert.Contains($"FilterEntry action=\"{FilterAction.FilterActionToString(filterActions[i])}\" filterId=\"{i}\" permissionData=\"0x70\" dataType=\"{dataType}\"", s);

                    s2 = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, s);
                    CheckDefaultContainerString(dataTypes[i], s2, indent + 2);
                }
                else
                {
                    if (!permDataPresent[i]) Assert.Contains($"FilterEntry action=\"{FilterAction.FilterActionToString(filterActions[i])}\" filterId=\"{i}\" dataType=\"NoData\"", s);
                    else Assert.Contains($"FilterEntry action=\"{FilterAction.FilterActionToString(filterActions[i])}\" filterId=\"{i}\" permissionData=\"0x70\" dataType=\"NoData\"", s);
                }

                currString = currString.Substring(currString.IndexOf(currEntryEndString) + currEntryEndString.Length);
            }
        }

        private void CheckVectorString(int dataType,
            bool hasSummary,
            bool hasTotalCountHint,
            bool supportsSorting,
            int[] actions, bool[] hasPermData,
            string containerString,
            int indent = 0)
        {
            string indentStr = GetIndent(indent);
            string entryIndent = GetIndent(indent + 1);

            Assert.Contains(indentStr + "Vector", containerString);
            Assert.Contains("\n" + indentStr + "VectorEnd", containerString);

            if (hasTotalCountHint)
            {
                Assert.Contains($"totalCountHint=\"{actions.Length}\"", containerString);
            }
            Assert.Contains($"sortable=\"{supportsSorting.ToString().ToLower()}\"", containerString);
            GetContainerLimits(dataType, out string b, out string e);
            string strDataType = Access.DataType.AsString(dataType);
            string sum;
            if (hasSummary)
            {
                sum = GetContainerTypeEntry(entryIndent + "SummaryData ", entryIndent + "SummaryDataEnd", containerString);
                Assert.Contains($"SummaryData dataType=\"{strDataType}\"", sum);
                string innerContainer = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, sum);
                CheckDefaultContainerString(dataType, innerContainer, indent + 2);
            }

            string currEntryStartString = "\n" + entryIndent + "VectorEntry";
            string currEntryEndString = "\n" + entryIndent + "VectorEntryEnd";

            string currString = containerString.Substring(containerString.IndexOf(currEntryStartString));

            for (int i = 0; i < actions.Length; i++)
            {
                string s, s2;
                s = GetContainerTypeEntry(currEntryStartString, currEntryEndString, currString);

                if (actions[i] != (int)VectorEntryActions.CLEAR && actions[i] != (int)VectorEntryActions.DELETE)
                {
                    s2 = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, s);

                    if (!hasPermData[i]) Assert.Contains($"VectorEntry action=\"{VectorAction.VectorActionToString(actions[i])}\" index=\"{i}\" dataType=\"{strDataType}\"", s);
                    else Assert.Contains($"VectorEntry action=\"{VectorAction.VectorActionToString(actions[i])}\" index=\"{i}\" permissionData=\"0x70\" dataType=\"{strDataType}\"", s);

                    CheckDefaultContainerString(dataType, s2, indent + 2);
                }
                else
                {
                    if (!hasPermData[i]) Assert.Contains($"VectorEntry action=\"{VectorAction.VectorActionToString(actions[i])}\" index=\"{i}\" dataType=\"NoData\"", s);
                    else Assert.Contains($"VectorEntry action=\"{VectorAction.VectorActionToString(actions[i])}\" index=\"{i}\" permissionData=\"0x70\" dataType=\"NoData\"", s);
                }
                currString = currString.Substring(currString.IndexOf(currEntryEndString) + currEntryEndString.Length);
            }
        }

        private void CheckMapString(int dataType,
            int[] entryActions, bool[] permDataPresent,
            int keyType, bool hasSummary, bool hasKeyFieldId,
            bool hasTotalHintCount, string containerString,
            int indent = 0)
        {
            string indentStr = GetIndent(indent);
            string entryIndent = GetIndent(indent + 1);

            Assert.Contains(indentStr + "Map", containerString);
            Assert.Contains("\n" + indentStr + "MapEnd", containerString);

            if (hasTotalHintCount)
            {
                Assert.Contains($"totalCountHint=\"{entryActions.Length}\"", containerString);
            }
            if (hasKeyFieldId)
            {
                Assert.Contains($"keyFieldId=\"{EmaComplexTypeHandler.mapKeyFieldId}\"", containerString);
            }

            GetContainerLimits(dataType, out string b, out string e);
            string strDataType = Access.DataType.AsString(dataType);
            string sum;
            if (hasSummary)
            {
                sum = GetContainerTypeEntry(entryIndent + "SummaryData ", entryIndent + "SummaryDataEnd", containerString);
                Assert.Contains($"SummaryData dataType=\"{strDataType}\"", sum);
                string innerContainer = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, sum);
                CheckDefaultContainerString(dataType, innerContainer, indent + 2);
            }

            string currEntryStartString = "\n" + entryIndent + "MapEntry";
            string currEntryEndString = "\n" + entryIndent + "MapEntryEnd";

            string currString = containerString.Substring(containerString.IndexOf(currEntryStartString));

            for (int i = 0; i < entryActions.Length; i++)
            {
                string s, s2;
                s = GetContainerTypeEntry(currEntryStartString, currEntryEndString, currString);

                if (entryActions[i] != (int)MapEntryActions.DELETE)
                {
                    s2 = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, s);

                    if (!permDataPresent[i])
                        Assert.Contains($"MapEntry action=\"{MapAction.MapActionToString(entryActions[i])}\" key dataType=\"{Access.DataType.AsString(keyType)}\" value=\"{GetPrimitiveValueString(keyType)}\" dataType=\"{strDataType}\"", s);
                    else
                        Assert.Contains($"MapEntry action=\"{MapAction.MapActionToString(entryActions[i])}\" key dataType=\"{Access.DataType.AsString(keyType)}\" value=\"{GetPrimitiveValueString(keyType)}\" permissionData=\"0x70\" dataType=\"{strDataType}\"", s);

                    CheckDefaultContainerString(dataType, s2, indent + 2);
                }
                else
                {
                    if (!permDataPresent[i])
                        Assert.Contains($"MapEntry action=\"{MapAction.MapActionToString(entryActions[i])}\" key dataType=\"{Access.DataType.AsString(keyType)}\" value=\"{GetPrimitiveValueString(keyType)}\" dataType=\"NoData\"", s);
                    else
                        Assert.Contains($"MapEntry action=\"{MapAction.MapActionToString(entryActions[i])}\" key dataType=\"{Access.DataType.AsString(keyType)}\" value=\"{GetPrimitiveValueString(keyType)}\" permissionData=\"0x70\" dataType=\"NoData\"", s);
                }
                currString = currString.Substring(currString.IndexOf(currEntryEndString) + currEntryEndString.Length);
            }
        }

        private void CheckSeriesString(int dataType, bool hasCountHint, bool hasSummary, int countHint, int length, string source, int indent = 0)
        {
            string indentStr = GetIndent(indent);
            string entryIndent = GetIndent(indent + 1);

            Assert.Contains(indentStr + "Series", source);
            Assert.Contains("\n" + indentStr + "SeriesEnd", source);

            if (hasCountHint)
            {
                Assert.Contains($"totalCountHint=\"{countHint}\"", source);
            }

            string dataTypeStr = Access.DataType.AsString(dataType);
            GetContainerLimits(dataType, out string b, out string e);

            string sum;
            if (hasSummary)
            {
                sum = GetContainerTypeEntry(entryIndent + "SummaryData ", entryIndent + "SummaryDataEnd", source);
                Assert.Contains($"SummaryData dataType=\"{dataTypeStr}\"", sum);
                string innerContainer = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, sum);
                CheckDefaultContainerString(dataType, innerContainer, indent + 2);
            }

            string currEntryStartString = "\n" + entryIndent + "SeriesEntry";
            string currEntryEndString = "\n" + entryIndent + "SeriesEntryEnd";

            string currString = source.Substring(source.IndexOf(currEntryStartString));

            for (int i = 0; i < length; i++)
            {
                string s = GetContainerTypeEntry(currEntryStartString, currEntryEndString, currString);
                string s2 = GetContainerTypeEntry("\n" + GetIndent(indent + 2) + b, "\n" + GetIndent(indent + 2) + e, s);

                Assert.Contains($"SeriesEntry dataType=\"{dataTypeStr}\"", s);
                CheckDefaultContainerString(dataType, s2, indent + 2);

                currString = currString.Substring(currString.IndexOf(currEntryEndString) + currEntryEndString.Length);
            }
        }

        private string GetIndent(int indent)
        {
            StringBuilder b = new StringBuilder();
            while (indent-- > 0) b.Append("    ");
            return b.ToString();
        }

        private void CheckDefaultContainerString(int dataType, string containerString, int indent = 0)
        {
            switch (dataType)
            {
                case DataTypes.ELEMENT_LIST:
                    CheckElementListString(EmaComplexTypeHandler.defaultElementListTypes, containerString, indent);
                    break;

                case DataTypes.FILTER_LIST:
                    CheckFilterListString(true, EmaComplexTypeHandler.defaultFilterListActions,
                        EmaComplexTypeHandler.defaultFilterListDataTypes,
                        EmaComplexTypeHandler.defaultFilterEntryHasPermData,
                        EmaComplexTypeHandler.defaultFilterListCountHint, containerString, indent);
                    break;

                case DataTypes.VECTOR:
                    CheckVectorString(EmaComplexTypeHandler.defaultVectorContainerType,
                        false, true, true,
                        EmaComplexTypeHandler.defaultVectorActions,
                        EmaComplexTypeHandler.defaultVectorEntryHasPermData, containerString, indent);
                    break;

                case DataTypes.SERIES:
                    CheckSeriesString(EmaComplexTypeHandler.defaultSeriesContainerType, true, false,
                        EmaComplexTypeHandler.defaultSeriesCountHint,
                        EmaComplexTypeHandler.defaultSeriesCountHint, containerString, indent);
                    break;

                case DataTypes.MAP:
                    CheckMapString(EmaComplexTypeHandler.defaultMapContainerType, EmaComplexTypeHandler.defaultMapAction,
                        EmaComplexTypeHandler.defaultMapEntryHasPermData, EmaComplexTypeHandler.defaultMapKeyType,
                        false, true, true, containerString, indent);
                    break;

                case DataTypes.FIELD_LIST:
                    CheckFieldListString(EmaComplexTypeHandler.defaultFieldListTypes, containerString, indent);
                    break;

                case DataTypes.ARRAY:
                    Assert.Contains("OmmArray with entries of dataType=\"Qos\"", containerString);
                    break;

                default:
                    break;
            }
        }

        private string GetValueTypeEntry(string beginString, string endString, string source)
        {
            int begin = source.IndexOf(beginString);
            int end = source.IndexOf(endString, begin + beginString.Length);

            return source.Substring(begin, end - begin);
        }

        private string GetContainerTypeEntry(string beginString, string endString, string source)
        {
            int begin = source.IndexOf(beginString);
            int end = source.IndexOf(endString);

            return source.Substring(begin, end + endString.Length - begin);
        }

        private void GetContainerLimits(int dataType, out string beginLimit, out string endLimit, bool endWithSpace = false)
        {
            beginLimit = string.Empty;
            endLimit = string.Empty;

            switch (dataType)
            {
                case DataTypes.ELEMENT_LIST:
                    beginLimit = "ElementList";
                    endLimit = "ElementListEnd";
                    break;

                case DataTypes.FILTER_LIST:
                    beginLimit = "FilterList ";
                    endLimit = "FilterListEnd";
                    break;

                case DataTypes.VECTOR:
                    beginLimit = "Vector";
                    endLimit = "VectorEnd";
                    break;

                case DataTypes.SERIES:
                    beginLimit = "Series";
                    endLimit = "SeriesEnd";
                    break;

                case DataTypes.MAP:
                    beginLimit = "Map";
                    endLimit = "MapEnd";
                    break;

                case DataTypes.FIELD_LIST:
                    beginLimit = "FieldList";
                    endLimit = "FieldListEnd";
                    break;

                case DataTypes.OPAQUE:
                    beginLimit = "Opaque";
                    endLimit = "OpaqueEnd";
                    break;

                case DataTypes.XML:
                    beginLimit = "Xml";
                    endLimit = "XmlEnd";
                    break;

                case DataTypes.ANSI_PAGE:
                    beginLimit = "AnsiPage";
                    endLimit = "AnsiPageEnd";
                    break;

                case DataTypes.ARRAY:
                    beginLimit = "OmmArray ";
                    endLimit = "OmmArrayEnd";
                    break;

                case DataTypes.NO_DATA:
                    beginLimit = "NoData";
                    endLimit = "NoDataEnd";
                    break;

                default:
                    break;
            }
        }

        private string GetPrimitiveValueString(int dataType)
        {
            switch (dataType)
            {
                case DataTypes.INT:
                    return EmaComplexTypeHandler.iv.ToString();

                case DataTypes.UINT:
                    return EmaComplexTypeHandler.uintv.ToString();

                case DataTypes.DATE:
                    return EmaComplexTypeHandler.date.ToString();

                case DataTypes.TIME:
                    return "00:00:02:000:000:000";

                case DataTypes.DATETIME:
                    return "02 NOV 2020 13:00:00:000:000:000";

                case DataTypes.ASCII_STRING:
                    return EmaComplexTypeHandler.ascii.ToString();

                case DataTypes.REAL:
                    return EmaComplexTypeHandler.real.ToString();

                case DataTypes.QOS:
                    return EmaComplexTypeHandler.qos.ToString();

                case DataTypes.STATE:
                    return "Open / Ok / None / ''";

                case DataTypes.DOUBLE:
                    return EmaComplexTypeHandler.dv.ToString();

                case DataTypes.FLOAT:
                    return EmaComplexTypeHandler.fv.ToString();

                case DataTypes.ENUM:
                    return EmaComplexTypeHandler.enumer.ToString();

                default:
                    return string.Empty;
            }
        }


        private static string Decode(Msg msg)
        {
            msg.EncodeComplete();
            var encodedBuffer = msg.Encoder!.m_encodeIterator!.Buffer();
            msg.Decode(encodedBuffer, Codec.MajorVersion(), Codec.MinorVersion(), msg.m_dataDictionary, null);
            return msg.FillString(0);
        }
    }
}