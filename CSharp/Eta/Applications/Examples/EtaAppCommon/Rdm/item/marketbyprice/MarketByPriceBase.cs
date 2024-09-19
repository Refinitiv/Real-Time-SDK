/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;

namespace LSEG.Eta.Example.Common
{
    public abstract class MarketByPriceBase : MsgBase
    {
        public Buffer ItemName { get; set; } = new Buffer();
        public MarketByPriceItem MbpInfo { get; set; } = new MarketByPriceItem();
        public DataDictionary Dictionary { get; set; } = new DataDictionary();
        public int PartNo { get; set; }

        protected const string eolChar = "\n";
        protected const string tabChar = "\t";

        protected const int ENCODED_SET_DEF_SIZE = 60;
        protected Buffer tmpBuffer = new Buffer();
        protected Map map = new Map();
        protected MapEntry mapEntry = new MapEntry();
        protected FieldList fieldList = new FieldList();
        protected FieldEntry fieldEntry = new FieldEntry();
        protected Real tempReal = new Real();
        protected UInt tempUInt = new UInt();
        protected DateTime tempDateTime = new DateTime();
        protected LocalFieldSetDefDb? marketByPriceSetDefDb;

        protected MarketByPriceBase()
        {
            EncodeIterator encIter = new EncodeIterator();
            encIter.Clear();
            SetupMarketByPriceSetDefDb(encIter);
            PartNo = -1;
        }

        private void SetupMarketByPriceSetDefDb(EncodeIterator encIter)
        {
            FieldSetDefEntry[] fieldSetDefEntries = new FieldSetDefEntry[4];

            // Id=ORDER_PRC type=REAL
            fieldSetDefEntries[0] = new FieldSetDefEntry();
            fieldSetDefEntries[0].DataType = DataTypes.REAL;
            fieldSetDefEntries[0].FieldId = MarketByPriceItem.ORDER_PRC_FID;

            // Id=ORDER_SIZE type=REAL
            fieldSetDefEntries[1] = new FieldSetDefEntry();
            fieldSetDefEntries[1].DataType = DataTypes.REAL;
            fieldSetDefEntries[1].FieldId = MarketByPriceItem.ORDER_SIZE_FID;

            // Id=QUOTIM MS type=UINT_4
            fieldSetDefEntries[2] = new FieldSetDefEntry();
            fieldSetDefEntries[2].DataType = DataTypes.UINT_4;
            fieldSetDefEntries[2].FieldId = MarketByPriceItem.QUOTIM_MS_FID;

            // Number of Orders
            fieldSetDefEntries[3] = new FieldSetDefEntry();
            fieldSetDefEntries[3].DataType = DataTypes.UINT_4;
            fieldSetDefEntries[3].FieldId = MarketByPriceItem.NO_ORD_FID;

            // Populate FieldSetDef array inside LocalFieldSetDefDb
            marketByPriceSetDefDb = new LocalFieldSetDefDb();
            marketByPriceSetDefDb.Clear();

            // index and ID must match
            marketByPriceSetDefDb.Definitions[0].SetId = 0;
            marketByPriceSetDefDb.Definitions[0].Count = 0;
            marketByPriceSetDefDb.Definitions[0].Entries = fieldSetDefEntries;
        }

        public override void Clear()
        {
            base.Clear();
            ItemName.Clear();
            PartNo = -1;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            map.Clear();
            fieldList.Clear();
            fieldEntry.Clear();

            tempUInt.Clear();
            tempReal.Clear();

            Msg msg = EncodeMsg();

            CodecReturnCode ret = msg.EncodeInit(encIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            map.ApplyHasKeyFieldId();
            map.KeyPrimitiveType = DataTypes.BUFFER;
            map.KeyFieldId = MarketByPriceItem.ORDER_PRC_FID;
            map.ContainerType = DataTypes.FIELD_LIST;
            map.ApplyHasSetDefs();

            if (PartNo == 0)
            {
                map.ApplyHasSummaryData();
            }

            ret = map.EncodeInit(encIter, 0, 0);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            ret = marketByPriceSetDefDb!.Encode(encIter);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            ret = map.EncodeSetDefsComplete(encIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            if (PartNo == 0)
            {
                ret = EncodeSummaryData(encIter, Dictionary, map);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
                ret = map.EncodeSummaryDataComplete(encIter, true);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }

            ret = EncodeMapEntries(encIter, Dictionary);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            ret = map.EncodeComplete(encIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return msg.EncodeComplete(encIter, true);
        }

        public abstract Msg EncodeMsg();

        protected virtual CodecReturnCode EncodeSummaryData(EncodeIterator encodeIter, DataDictionary dictionary, Map map)
        {
            return CodecReturnCode.SUCCESS;
        }

        protected abstract CodecReturnCode EncodeMapEntries(EncodeIterator encodeIter, DataDictionary dictionary);

        protected override StringBuilder PrepareStringBuilder()
        {
            StringBuilder stringBuf = base.PrepareStringBuilder();

            stringBuf.Append(tabChar);
            stringBuf.Append("itemName: ");
            stringBuf.Append(ItemName);
            stringBuf.Append(eolChar);

            stringBuf.Append(tabChar);
            stringBuf.Append("item info: ");
            stringBuf.Append(MbpInfo.ToString());
            stringBuf.Append(eolChar);

            return stringBuf;
        }

        public override CodecReturnCode Decode(DecodeIterator encIter, Msg msg)
        {
            throw new NotImplementedException();
        }
    }
}
