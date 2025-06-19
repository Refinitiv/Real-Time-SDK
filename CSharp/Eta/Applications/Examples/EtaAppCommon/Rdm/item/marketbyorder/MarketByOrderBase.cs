/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;

namespace LSEG.Eta.Example.Common
{
    public abstract class MarketByOrderBase : MsgBase
    {
        public Buffer ItemName { get; set; } = new Buffer();
        public MarketByOrderItem MboInfo { get; set; } = new MarketByOrderItem();
        public DataDictionary Dictionary { get; set; } = new DataDictionary();

        protected Buffer tmpBuffer = new Buffer();
        protected Map map = new Map();
        protected MapEntry mapEntry = new MapEntry();
        protected FieldList fieldList = new FieldList();
        protected FieldEntry fieldEntry = new FieldEntry();
        protected Real tempReal = new Real();
        protected UInt tempUInt = new UInt();
        protected DateTime tempDateTime = new DateTime();
        protected LocalFieldSetDefDb marketByOrderSetDefDb = new LocalFieldSetDefDb();

        protected MarketByOrderBase()
        {
            SetupMarketByOrderSetDefDb();
        }

        public abstract Msg EncodeMsg();

        protected abstract CodecReturnCode EncodeMapEntries(EncodeIterator EncodeIter, DataDictionary dictionary);

        protected virtual CodecReturnCode EncodeSummaryData(EncodeIterator EncodeIter, DataDictionary dictionary, Map map)
        {
            return CodecReturnCode.SUCCESS;
        }

        protected void SetupMarketByOrderSetDefDb()
        {
            FieldSetDefEntry[] fieldSetDefEntries = new FieldSetDefEntry[3];

            // Id=ORDER_PRC type=REAL
            fieldSetDefEntries[0] = new FieldSetDefEntry();
            fieldSetDefEntries[0].DataType = DataTypes.REAL;
            fieldSetDefEntries[0].FieldId = MarketByOrderItem.ORDER_PRC_FID;

            // Id=ORDER_SIZE type=REAL
            fieldSetDefEntries[1] = new FieldSetDefEntry();
            fieldSetDefEntries[1].DataType = DataTypes.REAL;
            fieldSetDefEntries[1].FieldId = MarketByOrderItem.ORDER_SIZE_FID;

            // Id=QUOTIM MS type=UINT_4
            fieldSetDefEntries[2] = new FieldSetDefEntry();
            fieldSetDefEntries[2].DataType = DataTypes.UINT_4;
            fieldSetDefEntries[2].FieldId = MarketByOrderItem.QUOTIM_MS_FID;

            // populate FieldSetDef array inside LocalFieldSetDefDb      
            marketByOrderSetDefDb = new LocalFieldSetDefDb();
            marketByOrderSetDefDb.Clear();
            marketByOrderSetDefDb.Definitions[0].SetId = 0;
            marketByOrderSetDefDb.Definitions[0].Count = 3;
            marketByOrderSetDefDb.Definitions[0].Entries = fieldSetDefEntries;
        }

        protected virtual bool IsRefreshType()
        {
            return false;
        }

        public override void Clear()
        {
            base.Clear();
            ItemName.Clear();
        }

        public override CodecReturnCode Encode(EncodeIterator EncodeIter)
        {
            map.Clear();
            fieldList.Clear();
            fieldEntry.Clear();

            tempUInt.Clear();
            tempReal.Clear();

            Msg msg = EncodeMsg();
            CodecReturnCode ret = msg.EncodeInit(EncodeIter, 0);

            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            map.ApplyHasKeyFieldId();
            map.KeyPrimitiveType = DataTypes.BUFFER;
            map.KeyFieldId = MarketByOrderItem.ORDER_ID_FID;
            map.ContainerType = DataTypes.FIELD_LIST;
            map.ApplyHasSetDefs();

            if (IsRefreshType())
            {
                map.ApplyHasSummaryData();
            }

            ret = map.EncodeInit(EncodeIter, 0, 0);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            ret = marketByOrderSetDefDb.Encode(EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            ret = map.EncodeSetDefsComplete(EncodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            if (IsRefreshType())
            {
                ret = EncodeSummaryData(EncodeIter, Dictionary, map);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
                ret = map.EncodeSummaryDataComplete(EncodeIter, true);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }

            ret = EncodeMapEntries(EncodeIter, Dictionary);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            ret = map.EncodeComplete(EncodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return msg.EncodeComplete(EncodeIter, true);
        }

        public StringBuilder PrepareStringBuffer()
        {
            StringBuilder stringBuf = base.PrepareStringBuilder();

            stringBuf.Append(tab);
            stringBuf.Append("itemName: ");
            stringBuf.Append(ItemName);
            stringBuf.AppendLine();

            stringBuf.Append(tab);
            stringBuf.Append("item info: ");
            stringBuf.Append(MboInfo.ToString());
            stringBuf.AppendLine();

            return stringBuf;
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            return 0;
        }

    }
}
