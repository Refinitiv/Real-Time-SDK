﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Tests;
using System;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access.Tests
{
    public class EmaContainersTest
    {
        private EmaObjectManager m_objectManager = new EmaObjectManager();
        int[] containerTypes = { DataType.DataTypes.FILTER_LIST,
                DataType.DataTypes.ELEMENT_LIST,
                DataType.DataTypes.FIELD_LIST,
                DataType.DataTypes.MAP,
                DataType.DataTypes.VECTOR,
                DataType.DataTypes.SERIES,
                DataType.DataTypes.OPAQUE,
                DataType.DataTypes.XML,
                DataType.DataTypes.ANSI_PAGE
        };
        bool[] boolValues = { true, false };
        

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
        public void ElementListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);
            
            int[][] dataTypesArray = {
                new int[] { DataType.DataTypes.INT, DataType.DataTypes.QOS, DataType.DataTypes.STATE, DataType.DataTypes.ENUM, DataType.DataTypes.DATE, DataType.DataTypes.DATETIME },
                new int[] { DataType.DataTypes.UINT, DataType.DataTypes.ASCII, DataType.DataTypes.FIELD_LIST, DataType.DataTypes.FIELD_LIST },
                new int[] { DataType.DataTypes.DOUBLE, DataType.DataTypes.REAL, DataType.DataTypes.VECTOR },
                new int[] { DataType.DataTypes.XML, DataType.DataTypes.OPAQUE, DataType.DataTypes.ANSI_PAGE }
            };
         
            foreach (var dataTypes in dataTypesArray)
                foreach (var preencodeEntry in boolValues)
                {
                    ElementList elementList = m_objectManager.GetOmmElementList();
                    EmaComplexTypeHandler.EncodeElementList(elementList, dataTypes, preencodeEntry);

                    var buffer = elementList.Encoder!.m_encodeIterator!.Buffer();
                    elementList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                    EmaComplexTypeHandler.DecodeAndCheckElementList(elementList, dataTypes);
                    elementList.ReturnToPool();

                    CheckEmaObjectManagerPoolSizes(m_objectManager);
                }
        }

        [Fact]
        public void ElementListNonRWFTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            var dataTypes = new int[] { DataType.DataTypes.OPAQUE, DataType.DataTypes.ANSI_PAGE, DataType.DataTypes.XML, DataType.DataTypes.ENUM, DataType.DataTypes.DATE, DataType.DataTypes.DATETIME };
            ElementList elementList = m_objectManager.GetOmmElementList();
            EmaComplexTypeHandler.EncodeElementList(elementList, dataTypes);

            var buffer = elementList.Encoder!.m_encodeIterator!.Buffer();
            elementList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

            EmaComplexTypeHandler.DecodeAndCheckElementList(elementList, dataTypes);
            elementList.Encoder.Clear();
            m_objectManager.ReturnToPool(elementList);

            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void SeriesTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            int[] lengths = { 3, 8, 12, 1 };
            int[] countHints = { 4, 5, 10, 2 };

            foreach (var dataType in containerTypes)
                foreach (var countHintPresent in boolValues) 
                    foreach (var summaryPresent in boolValues)
                        foreach (var countHint in countHints)
                            foreach (var length in lengths)
                                foreach (var preencodeEntry in boolValues)
                                {
                                    Series series = m_objectManager.GetOmmSeries();
                                    EmaComplexTypeHandler.EncodeSeries(series, dataType, countHintPresent, true, countHint, length, preencodeEntry);

                                    var buffer = series.Encoder!.m_encodeIterator!.Buffer();
                                    Series decodedSeries = m_objectManager.GetOmmSeries();
                                    decodedSeries.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                    EmaComplexTypeHandler.DecodeAndCheckSeries(decodedSeries, dataType, countHintPresent, summaryPresent, countHint, length);
                                    series.ReturnToPool();
                                    decodedSeries.ReturnToPool();

                                    CheckEmaObjectManagerPoolSizes(m_objectManager);
                                }
        }

        [Fact]
        public void MapTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            int[] mapContainerTypes = {
                DataType.DataTypes.FILTER_LIST,
                DataType.DataTypes.ELEMENT_LIST,
                DataType.DataTypes.FIELD_LIST,
                DataType.DataTypes.MAP,
                DataType.DataTypes.VECTOR };

            int[][] mapActionsArray = { 
                new int[]{ MapAction.ADD, MapAction.ADD, MapAction.UPDATE },
                new int[]{ MapAction.DELETE, MapAction.ADD, MapAction.DELETE },
                new int[]{ MapAction.UPDATE, MapAction.DELETE, MapAction.UPDATE },
                new int[]{ MapAction.DELETE, MapAction.DELETE, MapAction.DELETE },
                new int[]{ MapAction.ADD, MapAction.ADD, MapAction.ADD },
                new int[]{ MapAction.ADD, MapAction.UPDATE, MapAction.DELETE }
            };

            int[] keyTypes = {
                DataType.DataTypes.INT,
                DataType.DataTypes.UINT,
                DataType.DataTypes.FLOAT,
                DataType.DataTypes.DOUBLE,
                DataType.DataTypes.TIME,
                DataType.DataTypes.DATETIME,
                DataType.DataTypes.REAL,
                DataType.DataTypes.QOS,
                DataType.DataTypes.STATE
            };


            bool[][] hasPermDataArray = {
                new bool[] { false, false, false },
                new bool[] { false, false, true },
                new bool[] { true, true, true },
                new bool[] { true, false, true },
            };

            foreach (var dataType in mapContainerTypes)
                foreach (var mapActions in mapActionsArray)
                    foreach (var keyType in keyTypes)
                        foreach (var summaryPresent in boolValues)
                            foreach (var keyFieldIdPresent in boolValues)
                                foreach (var countHintPresent in boolValues)
                                    foreach (var permDataPresent in hasPermDataArray)
                                        foreach (var preencodeEntry in boolValues)
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
                                                preencodeEntry);

                                            var buffer = map.Encoder!.m_encodeIterator!.Buffer();
                                            
                                            Map decodedMap = m_objectManager.GetOmmMap();
                                            decodedMap.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                            EmaComplexTypeHandler.DecodeAndCheckMap(decodedMap,
                                                dataType,
                                                mapActions,
                                                permDataPresent,
                                                keyType,
                                                true,
                                                keyFieldIdPresent,
                                                countHintPresent);

                                            map.ReturnToPool();
                                            decodedMap.ReturnToPool();

                                            CheckEmaObjectManagerPoolSizes(m_objectManager);
                                        }

            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void VectorTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            int[][] vectorActionsArray = new int[][]
            {
                new int[] { VectorAction.CLEAR, VectorAction.UPDATE, VectorAction.DELETE, VectorAction.INSERT, VectorAction.SET },
                new int[] { VectorAction.CLEAR, VectorAction.CLEAR, VectorAction.CLEAR, VectorAction.CLEAR, VectorAction.CLEAR },
                new int[] { VectorAction.DELETE, VectorAction.DELETE, VectorAction.DELETE, VectorAction.DELETE, VectorAction.DELETE },
                new int[] { VectorAction.UPDATE, VectorAction.DELETE, VectorAction.INSERT, VectorAction.INSERT, VectorAction.CLEAR },
                new int[] { VectorAction.SET, VectorAction.SET, VectorAction.SET, VectorAction.SET, VectorAction.SET },
                new int[] { VectorAction.UPDATE, VectorAction.DELETE, VectorAction.UPDATE, VectorAction.DELETE, VectorAction.UPDATE },
                new int[] { VectorAction.DELETE, VectorAction.CLEAR, VectorAction.DELETE, VectorAction.CLEAR, VectorAction.DELETE },
                new int[] { VectorAction.INSERT, VectorAction.INSERT, VectorAction.UPDATE, VectorAction.INSERT, VectorAction.SET }
            };

            bool[][] hasPermDataArray = {
                new bool[] { false, false, false, false, true },
                new bool[] { false, false, false, false, false },
                new bool[] { true, true, true, true, true },
                new bool[] { true, false, true, false, true },
            };

            foreach (var dataType in containerTypes)
                foreach (var vectorActions in vectorActionsArray)
                    foreach (var hasPermData in hasPermDataArray)
                        foreach (var summaryPresent in boolValues)
                            foreach (var supportsSorting in boolValues) 
                                foreach (var hasTotalCountHint in boolValues)
                                    foreach (var preencodeEntry in boolValues)
                                        foreach (var noDataEntries in boolValues)
                                        {
                                            Vector vector = m_objectManager.GetOmmVector();
                                            EmaComplexTypeHandler.EncodeVector(vector,
                                                dataType,
                                                summaryPresent,
                                                hasTotalCountHint,
                                                supportsSorting,
                                                vectorActions,
                                                hasPermData,
                                                preencodeEntry,
                                                noDataEntries);

                                            var buffer = vector.Encoder!.m_encodeIterator!.Buffer();
                                            vector.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                            EmaComplexTypeHandler.DecodeAndCheckVector(vector,
                                                dataType,
                                                summaryPresent,
                                                hasTotalCountHint,
                                                supportsSorting,
                                                vectorActions,
                                                hasPermData,
                                                noDataEntries);

                                            vector.ReturnToPool();

                                            CheckEmaObjectManagerPoolSizes(m_objectManager);
                                        }
            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void FilterListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            int[][] filterActionsArray = new int[][]
            {
                new int[] { FilterAction.CLEAR, FilterAction.CLEAR, FilterAction.CLEAR, FilterAction.CLEAR },
                new int[] { FilterAction.UPDATE, FilterAction.CLEAR, FilterAction.SET, FilterAction.SET }, //this
                new int[] { FilterAction.SET, FilterAction.UPDATE, FilterAction.SET, FilterAction.UPDATE },
                new int[] { FilterAction.CLEAR, FilterAction.UPDATE, FilterAction.SET, FilterAction.CLEAR }
            };

            bool[][] hasPermDataArray = {
                new bool[] { false, false, false, false },
                new bool[] { false, false, false, true },
                new bool[] { true, true, true, true },
                new bool[] { true, false, true, false },
            };

            int[][] dataTypesArray = new int[][]
            {
                new int[] { DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.MAP, DataType.DataTypes.ELEMENT_LIST },
                new int[] { DataType.DataTypes.VECTOR, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.VECTOR, DataType.DataTypes.VECTOR },
                new int[] { DataType.DataTypes.FIELD_LIST, DataType.DataTypes.VECTOR, DataType.DataTypes.FIELD_LIST, DataType.DataTypes.FIELD_LIST },
                new int[] { DataType.DataTypes.MAP, DataType.DataTypes.MAP, DataType.DataTypes.FIELD_LIST, DataType.DataTypes.MAP },
                new int[] { DataType.DataTypes.XML, DataType.DataTypes.MAP, DataType.DataTypes.MAP, DataType.DataTypes.MAP },
                new int[] { DataType.DataTypes.OPAQUE, DataType.DataTypes.OPAQUE, DataType.DataTypes.OPAQUE, DataType.DataTypes.ANSI_PAGE }
            };

            int[] countHintArray = new int[] { 3, 4, 5, 6 };

            bool[][] noDataEntriesArray =
            {
                new bool[] { false, false, false, false },
                new bool[] { false, true, false, false }
            };

            foreach (var hasCountHint in boolValues)
                foreach (var filterActions in filterActionsArray)
                    foreach (var dataTypes in dataTypesArray)
                        foreach (var hasPermData in hasPermDataArray)
                            foreach (var countHint in countHintArray)
                                foreach (var preencodeEntry in boolValues)
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
                                        preencodeEntry,
                                        noDataEntries);

                                    var buffer = filterList.Encoder!.m_encodeIterator!.Buffer();
                                    filterList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

                                    EmaComplexTypeHandler.DecodeAndCheckFilterList(filterList,
                                        DataType.DataTypes.NO_DATA, // variable not necessary
                                        hasCountHint,
                                        filterActions,
                                        dataTypes,
                                        hasPermData,
                                        countHint,
                                        noDataEntries);

                                    filterList.ReturnToPool();
                                    

                                    CheckEmaObjectManagerPoolSizes(m_objectManager);
                                }
        }

        [Fact]
        public void SingleFilterListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            FilterList filterList = m_objectManager.GetOmmFilterList();
            EmaComplexTypeHandler.EncodeFilterList(filterList,
                DataType.DataTypes.NO_DATA, // variable not necessary
                true,
                new int[] { FilterAction.UPDATE, FilterAction.CLEAR, FilterAction.SET, FilterAction.SET },
                new int[] { DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.MAP, DataType.DataTypes.ELEMENT_LIST },
                new bool[] { false, false, false, false },
                3,
                false);

            var buffer = filterList.Encoder!.m_encodeIterator!.Buffer();
            filterList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

            EmaComplexTypeHandler.DecodeAndCheckFilterList(filterList,
                DataType.DataTypes.NO_DATA, // variable not necessary
                true,
                new int[] { FilterAction.UPDATE, FilterAction.CLEAR, FilterAction.SET, FilterAction.SET },
                new int[] { DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.MAP, DataType.DataTypes.ELEMENT_LIST },
                new bool[] { false, false, false, false },
                3);

            filterList.ReturnToPool();

            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void FieldListTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            int[][] dataTypesArray = new int[][]
            {
                new int[] { DataType.DataTypes.REAL, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.DATE, DataType.DataTypes.TIME },
                new int[] { DataType.DataTypes.MAP, DataType.DataTypes.REAL, DataType.DataTypes.VECTOR, DataType.DataTypes.UINT },
                new int[] { DataType.DataTypes.ARRAY, DataType.DataTypes.TIME, DataType.DataTypes.VECTOR, DataType.DataTypes.ARRAY }
            };

            foreach (var dataTypes in dataTypesArray)
                foreach (var preencodeEntry in boolValues)
                {
                    FieldList fieldList = m_objectManager.GetOmmFieldList();
                    EmaComplexTypeHandler.EncodeFieldList(fieldList, dataTypes, preencodeEntry);

                    var buffer = fieldList.Encoder!.m_encodeIterator!.Buffer();
                    fieldList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);
                    EmaComplexTypeHandler.DecodeAndCheckFieldList(fieldList, dataTypes);

                    fieldList.ReturnToPool();

                    CheckEmaObjectManagerPoolSizes(m_objectManager);
                }
        }

        [Fact]
        public void FieldListSingleTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            int[] dataTypes = new int[] { DataType.DataTypes.ARRAY, DataType.DataTypes.TIME, DataType.DataTypes.VECTOR, DataType.DataTypes.ARRAY };

            FieldList fieldList = m_objectManager.GetOmmFieldList();
            EmaComplexTypeHandler.EncodeFieldList(fieldList, dataTypes);

            var buffer = fieldList.Encoder!.m_encodeIterator!.Buffer();
            fieldList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);
            EmaComplexTypeHandler.DecodeAndCheckFieldList(fieldList, dataTypes);

            fieldList.ReturnToPool();

            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void ArrayTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            OmmArray array = m_objectManager.GetOmmArray();
            EmaComplexTypeHandler.EncodeArray(DataType.DataTypes.QOS, array);

            var buffer = array.Encoder!.m_encodeIterator!.Buffer();
            OmmArray arr = new OmmArray();
            arr.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buffer);
            
            EmaComplexTypeHandler.DecodeAndCheckArray(arr, DataType.DataTypes.QOS);

            array.ReturnToPool();

            CheckEmaObjectManagerPoolSizes(m_objectManager);
        }

        [Fact]
        public void EncodeBlankEntryToElementListTest()
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
                DataType.DataTypes.BUFFER
            };

            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            ElementList elementList = m_objectManager.GetOmmElementList();
            EmaComplexTypeHandler.EncodeElementListWithCodeValues(elementList, dataTypes);

            var buffer = elementList.Encoder!.m_encodeIterator!.Buffer();
            elementList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null);

            EmaComplexTypeHandler.DecodeAndCheckElementListWithCodeValues(elementList);
            elementList.ReturnToPool();
        }

        [Fact]
        public void EncodeDecodedContainerTest()
        {
            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleElementList(encodeIterator, new int[] { DataTypes.INT, DataTypes.UINT, DataTypes.UINT });

            ElementList elementList = m_objectManager.GetOmmElementList();
            elementList.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null);
            Assert.Equal(Access.DataType.DataTypes.ELEMENT_LIST, elementList.DataType);

            Map map = m_objectManager.GetOmmMap();
            map.KeyType(DataTypes.INT);
            map.AddKeyInt(5, MapAction.ADD, elementList);
            map.Complete();

            var mapBuffer = map!.Encoder!.m_encodeIterator!.Buffer();
            map.Decode(Codec.MajorVersion(), Codec.MinorVersion(), mapBuffer, null, null);

            var mapEnumerator = map.GetEnumerator();
            
            Assert.True(mapEnumerator.MoveNext());
            var el = mapEnumerator.Current;

            var enumerator = el.ElementList().GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.INT, element.LoadType);
            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.UINT, element.LoadType);
            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.UINT, element.LoadType);

            Assert.False(enumerator.MoveNext());
            Assert.False(mapEnumerator.MoveNext());
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
        }

    }
}
