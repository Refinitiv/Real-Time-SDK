/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System;

namespace LSEG.Ema.Access.Tests
{
    public class EmaComplexTypeSimpleTests : IDisposable
    {
        public void Dispose()
        {
            EtaGlobalPoolTestUtil.Clear();
            EtaGlobalPoolTestUtil.CheckEtaGlobalPoolSizes();
        }

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
        public void EncodeVectorUsingPreEncodedEntries_PassTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            Vector vector = new Vector();

            Series series = new Series();
            ElementList elementList = new ElementList();
            elementList.AddUInt("uint", 2).AddAscii("ascii", "asci_string").MarkForClear().Complete();
            
            series.TotalCountHint(2);
            series.SummaryData(elementList);
            elementList.Clear();
            elementList.AddInt("int", 4).AddTime("time", 5, 30, 25).MarkForClear().Complete();
            series.AddEntry(elementList).AddEntry(elementList).MarkForClear().Complete();

            vector.Sortable(false).Add(1, VectorAction.SET, series, null).MarkForClear().Complete();

            var buffer = vector.Encoder!.GetEncodedBuffer(false);

            Vector decodedVector = new Vector();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedVector.DecodeVector(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null));

            // Decoding vector container
            Assert.False(vector.Sortable());
            var vectorEnumerator = decodedVector.GetEnumerator();
            Assert.True(vectorEnumerator.MoveNext());
            var entry = vectorEnumerator.Current;
            Assert.Equal(DataTypes.SERIES, entry.LoadType);
            var decodedSeries = entry.Series();
            var seriesSummary = decodedSeries.SummaryData();
            Assert.Equal(DataTypes.ELEMENT_LIST, seriesSummary.DataType);
            
            foreach (var seriesEntry in decodedSeries)
            {
                Assert.Equal(DataTypes.ELEMENT_LIST, seriesEntry.LoadType);
            }

            Assert.False(vectorEnumerator.MoveNext());

            vectorEnumerator.Dispose();
            vector.Clear();
            decodedVector.Clear();
            series.Clear();
            elementList.Clear();
        }

        [Fact]
        public void EncodeSeriesUsingPostEncodedEntries_PassTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            Series series = new Series();
            ElementList entry = new ElementList();

            series.TotalCountHint(3).AddEntry(entry);
            entry.AddAscii("awesome ascii", "some interesting stuff").AddInt("secret value", 10).MarkForClear().Complete();
            entry.Clear();
            series.AddEntry(entry);
            entry.AddAscii("awesome ascii 2", "even more interesting stuff").AddInt("public value", 25).MarkForClear().Complete();
            series.MarkForClear().Complete();

            Series decodedSeries = new Series();
            var buffer = series.Encoder!.GetEncodedBuffer(false);

            Assert.Equal(CodecReturnCode.SUCCESS, decodedSeries.DecodeSeries(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null));

            series.Clear();
            decodedSeries.Clear();
        }

        [Fact]
        public void EncodeMapUsingMixedEncodedEntries_PassTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            Map map = new Map();
            ElementList entry = new ElementList();
            map.KeyFieldId(2);
            map.AddKeyAscii("key 1", MapAction.ADD, entry);
            entry.AddAscii("name 1", "ascii 1").AddAscii("name 2", "ascii 2").MarkForClear().Complete();
            entry.Clear();
            entry.AddAscii("name 3", "ascii 3").AddAscii("name 4", "ascii 4").MarkForClear().Complete();
            map.AddKeyAscii("key 2", MapAction.UPDATE, entry).MarkForClear().Complete();

            var buffer = map.Encoder!.GetEncodedBuffer(false);

            Map decodedMap = new Map();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedMap.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), buffer, dataDictionary, null));

            Assert.Equal(DataTypes.ASCII_STRING, decodedMap.KeyType());
            Assert.Equal(DataTypes.NO_DATA, decodedMap.SummaryData().DataType);

            var mapEnumerator = decodedMap.GetEnumerator();
            
            Assert.True(mapEnumerator.MoveNext());
            var mapEntry = mapEnumerator.Current;
            Assert.Equal(DataTypes.ASCII_STRING, mapEntry.Key.DataType);
            Assert.Equal("key 1", mapEntry.Key.Ascii().Value);
            Assert.Equal(DataTypes.ELEMENT_LIST, mapEntry.LoadType);
            
            var load = mapEntry.ElementList();
            var loadEnumerator = load.GetEnumerator();
            Assert.True(loadEnumerator.MoveNext());
            var loadEntry = loadEnumerator.Current;
            Assert.Equal(DataTypes.ASCII_STRING, loadEntry.LoadType);
            Assert.Equal("ascii 1", loadEntry.OmmAsciiValue().Value);
            Assert.True(loadEnumerator.MoveNext());
            loadEntry = loadEnumerator.Current;
            Assert.Equal(DataTypes.ASCII_STRING, loadEntry.LoadType);
            Assert.Equal("ascii 2", loadEntry.OmmAsciiValue().Value);
            loadEnumerator.Dispose();

            Assert.True(mapEnumerator.MoveNext());
            mapEntry = mapEnumerator.Current;
            Assert.Equal(DataTypes.ASCII_STRING, mapEntry.Key.DataType);
            Assert.Equal("key 2", mapEntry.Key.Ascii().Value);
            Assert.Equal(DataTypes.ELEMENT_LIST, mapEntry.LoadType);
            
            load = mapEntry.ElementList();
            loadEnumerator = load.GetEnumerator();
            Assert.True(loadEnumerator.MoveNext());
            loadEntry = loadEnumerator.Current;
            Assert.Equal(DataTypes.ASCII_STRING, loadEntry.LoadType);
            Assert.Equal("ascii 3", loadEntry.OmmAsciiValue().Value);
            Assert.True(loadEnumerator.MoveNext());
            loadEntry = loadEnumerator.Current;
            Assert.Equal(DataTypes.ASCII_STRING, loadEntry.LoadType);
            Assert.Equal("ascii 4", loadEntry.OmmAsciiValue().Value);
            loadEnumerator.Dispose();

            map.Clear();
            decodedMap.Clear();
            entry.Clear();
        }

        [Fact]
        public void EncodeMsgWithPreEncodedPayloadAndAttribs_PassTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            RequestMsg requestMsg = new RequestMsg();
            
            Series seriesPayload = new Series();       
            ElementList elementList = new ElementList();
            elementList.AddUInt("uint", 2).AddAscii("ascii", "asci_string").MarkForClear().Complete();

            seriesPayload.TotalCountHint(2);
            seriesPayload.SummaryData(elementList);
            elementList.Clear();
            elementList.AddInt("int", 4).AddTime("time", 5, 30, 25).MarkForClear().Complete();
            seriesPayload.AddEntry(elementList).AddEntry(elementList).MarkForClear().Complete();

            ElementList elementListAttrib = new ElementList();
            elementListAttrib.AddUInt("uint", 55).AddInt("int", 44).MarkForClear().Complete();

            requestMsg.StreamId(1)
                .DomainType((int)DomainType.LOGIN)
                .InterestAfterRefresh(true)
                .MarkForClear().Payload(seriesPayload)
                .Attrib(elementListAttrib);

            requestMsg.MarkForClear().EncodeComplete(); // called by API

            var body = requestMsg.Encoder!.GetEncodedBuffer(false);

            RequestMsg decodedRequestMsg = new RequestMsg();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedRequestMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));

            var decodedSeriesPayload = decodedRequestMsg.MarkForClear().Payload().Series();
            var seriesSummary = decodedSeriesPayload.SummaryData();
            Assert.Equal(DataTypes.ELEMENT_LIST, seriesSummary.DataType);
            foreach (var seriesEntry in decodedSeriesPayload)
            {
                Assert.Equal(DataTypes.ELEMENT_LIST, seriesEntry.LoadType);
            }

            var decodedElementListAttribs = decodedRequestMsg.Attrib().ElementList();

            requestMsg.Clear();
            decodedRequestMsg.Clear();
            seriesPayload.Clear();
            decodedSeriesPayload.Clear();
            elementList.Clear();
            decodedElementListAttribs.Clear();
            elementListAttrib.Clear();
        }

        [Fact]
        public void EncodeMsgWithPostEncodedPayloadAndAttribs_PassTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            RequestMsg requestMsg = new RequestMsg();

            Series seriesPayload = new Series();

            requestMsg.StreamId(1)
                .DomainType((int)DomainType.LOGIN)
                .InterestAfterRefresh(true);

            ElementList elementListAttrib = new ElementList();

            requestMsg.Attrib(elementListAttrib);

            elementListAttrib.AddUInt("uint", 55).AddInt("int", 44).MarkForClear().Complete(); // complete encoding already added Attributes

            ElementList elementList = new ElementList();
            elementList.AddUInt("uint", 2).AddAscii("ascii", "asci_string").MarkForClear().Complete();

            seriesPayload.TotalCountHint(2);
            seriesPayload.SummaryData(elementList);
            elementList.Clear();
            elementList.AddInt("int", 4).AddTime("time", 5, 30, 25).MarkForClear().Complete();

            requestMsg.MarkForClear().Payload(seriesPayload);
            seriesPayload.AddEntry(elementList).AddEntry(elementList).MarkForClear().Complete(); // complete encoding Payload that is already added to message

            requestMsg.MarkForClear().EncodeComplete(); // called by API

            var body = requestMsg.Encoder!.GetEncodedBuffer(false);

            RequestMsg decodedRequestMsg = new RequestMsg();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedRequestMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));

            var decodedSeriesPayload = decodedRequestMsg.MarkForClear().Payload().Series();
            var seriesSummary = decodedSeriesPayload.SummaryData();
            Assert.Equal(DataTypes.ELEMENT_LIST, seriesSummary.DataType);
            foreach (var seriesEntry in decodedSeriesPayload)
            {
                Assert.Equal(DataTypes.ELEMENT_LIST, seriesEntry.LoadType);
            }

            var decodedElementListAttribs = decodedRequestMsg.Attrib().ElementList();

            requestMsg.Clear();
            decodedRequestMsg.Clear();
            seriesPayload.Clear();
            decodedSeriesPayload.Clear();
            elementList.Clear();
            decodedElementListAttribs.Clear();
            elementListAttrib.Clear();
        }

        [Fact]
        public void EncodeMsgWithPostEncodedPayloadAndAttribs_FailTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            RequestMsg requestMsg = new RequestMsg();

            Series seriesPayload = new Series();

            requestMsg.StreamId(1)
                .DomainType((int)DomainType.LOGIN)
                .InterestAfterRefresh(true)
                .MarkForClear().Payload(seriesPayload);

            ElementList elementList = new ElementList();
            elementList.AddUInt("uint", 2).AddAscii("ascii", "asci_string").MarkForClear().Complete();

            seriesPayload.TotalCountHint(2);
            seriesPayload.SummaryData(elementList);
            elementList.Clear();
            elementList.AddInt("int", 4).AddTime("time", 5, 30, 25).MarkForClear().Complete();

            seriesPayload.AddEntry(elementList).AddEntry(elementList).MarkForClear().Complete(); // complete encoding Payload that is already added to message

            ElementList elementListAttrib = new ElementList();

            try
            {
                Assert.Throws<OmmInvalidUsageException>(() => requestMsg.Attrib(elementListAttrib)); // when post-encoded containers are used, attributes have to be encoded before payload
            }
            finally
            {
                requestMsg.Clear();
                seriesPayload.Clear();
                elementList.Clear();
                elementListAttrib.Clear();
            }
        }

        [Fact]
        public void EncodeMsgWithPostEncodedPayloadPreEncodedAttribs_FailTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            RequestMsg requestMsg = new RequestMsg();

            Series seriesPayload = new Series();

            requestMsg.StreamId(1)
                .DomainType((int)DomainType.LOGIN)
                .InterestAfterRefresh(true);

            ElementList elementListAttrib = new ElementList();
            elementListAttrib.AddUInt("uint", 55).AddInt("int", 44).MarkForClear().Complete(); // complete encoding preencoded Attributes

            requestMsg.Attrib(elementListAttrib);         

            ElementList elementList = new ElementList();
            elementList.AddUInt("uint", 2).AddAscii("ascii", "asci_string").MarkForClear().Complete();

            seriesPayload.TotalCountHint(2);
            seriesPayload.SummaryData(elementList);
            elementList.Clear();
            elementList.AddInt("int", 4).AddTime("time", 5, 30, 25).MarkForClear().Complete();

            try
            {
                Assert.Throws<OmmInvalidUsageException>(() => requestMsg.MarkForClear().Payload(seriesPayload)); // cannot add pre-encoded payload if post-encoded attributes were already added
            }
            finally
            {
                requestMsg.Clear();
                seriesPayload.Clear();
                elementList.Clear();
                elementListAttrib.Clear();
            }        
        }

        [Fact]
        public void EncodeMsgWithPreEncodedPayloadPostEncodedAttribs_FailTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            RequestMsg requestMsg = new RequestMsg();

            Series seriesPayload = new Series();
            ElementList elementList = new ElementList();
            elementList.AddUInt("uint", 2).AddAscii("ascii", "asci_string").MarkForClear().Complete();

            seriesPayload.TotalCountHint(2);
            seriesPayload.SummaryData(elementList);
            elementList.Clear();
            elementList.AddInt("int", 4).AddTime("time", 5, 30, 25).MarkForClear().Complete();
            seriesPayload.MarkForClear().Complete();

            requestMsg.StreamId(1)
                .DomainType((int)DomainType.LOGIN)
                .InterestAfterRefresh(true).MarkForClear().Payload(seriesPayload);

            ElementList elementListAttrib = new ElementList();

            Assert.Throws<OmmInvalidUsageException>(() => requestMsg.Attrib(elementListAttrib)); // cannot add post-encoded attributes if pre-encoded payload has already been added     

            requestMsg.Clear();
            seriesPayload.Clear();
            elementList.Clear();
            elementListAttrib.Clear();
        }

        [Fact(Skip = "Obsolete")]
        public void EncodeSeriesWithNonPreEncodedMsgEntry_FailTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            Series series = new Series();
            StatusMsg msg = new StatusMsg();

            // Containers should not allow adding empty messages that contain no preencoded data
            Assert.Throws<OmmInvalidUsageException>(() => series.AddEntry(msg)); // message encoding has not started - containers do not allow post-encoded messages

            series.Clear();
            msg.Clear();
        }

        [Fact(Skip = "Obsolete")]
        public void EncodeMsgWithNonPreEncodedMsgPayload_FailTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            PostMsg post = new PostMsg();
            StatusMsg msg = new StatusMsg();

            post.StreamId(5).DomainType((int)DomainType.MARKET_PRICE);

            try
            {
                // Messages should not allow adding empty messages that contain no preencoded data as Payload
                Assert.Throws<OmmInvalidUsageException>(() => post.MarkForClear().Payload(msg)); // message encoding has not started - messages do not allow post-encoded messages as payload
            }
            finally
            {
                post.Clear();
                msg.Clear();
            }        
        }

        [Fact(Skip = "Obsolete")]
        public void EncodeMsgWithNonPreEncodedMsgAttributes_FailTest()
        {
            DataDictionary dictionary = new DataDictionary();

            LoadFieldDictionary(dictionary);
            LoadEnumTypeDictionary(dictionary);

            PostMsg post = new PostMsg();
            StatusMsg msg = new StatusMsg();

            post.StreamId(5).DomainType((int)DomainType.MARKET_PRICE);

            try
            {
                // Messages should not allow adding empty messages that contain no preencoded data as Attributes
                Assert.Throws<OmmInvalidUsageException>(() => post.Attrib(msg)); // message encoding has not started - messages do not allow post-encoded messages as attributes
            }
            finally
            {
                post.Clear();
                msg.Clear();
            }
        }

        [Fact]
        public void EncodeMsgWithMsgPreEncodedPayload_PassTest()
        {
            DataDictionary dataDictionary = new DataDictionary();
            LoadEnumTypeDictionary(dataDictionary);
            LoadFieldDictionary(dataDictionary);

            PostMsg postMsg = new PostMsg();

            UpdateMsg updateMsg = new UpdateMsg();
            ElementList elemList = new ElementList();
            elemList.AddInt("int", 2).MarkForClear().Complete();

            updateMsg.StreamId(7).MarkForClear().Payload(elemList);

            postMsg.StreamId(1)
                .DomainType((int)DomainType.LOGIN)
                .MarkForClear().Payload(updateMsg).MarkForClear().Complete(true);

            postMsg.MarkForClear().EncodeComplete(); // called by API

            var body = postMsg.Encoder!.GetEncodedBuffer(false);

            PostMsg decodedPostMsg = new PostMsg();
            Assert.Equal(CodecReturnCode.SUCCESS, decodedPostMsg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));

            var decodedMsgPayload = decodedPostMsg.MarkForClear().Payload().UpdateMsg();
            var decodedElemList = decodedMsgPayload.MarkForClear().Payload().ElementList();

            var enumer = decodedElemList.GetEnumerator();
            Assert.True(enumer.MoveNext());
            var curr = enumer.Current;
            Assert.Equal(DataTypes.INT, curr.LoadType);
            Assert.Equal("int", curr.Name);
            Assert.Equal(2, curr.IntValue());

            postMsg.Clear();
            decodedPostMsg.Clear();
            decodedMsgPayload.Clear();
            decodedElemList.Clear();
            elemList.Clear();
            updateMsg.Clear();
        }
    }
}
