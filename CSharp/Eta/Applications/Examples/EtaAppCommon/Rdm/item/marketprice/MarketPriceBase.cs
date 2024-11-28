/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System.Diagnostics;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    public abstract class MarketPriceBase : MsgBase
    {
        private Buffer _itemName;
        private MarketPriceItem? _mpInfo;
        private DataDictionary? _dictionary;
        private int _serviceId;

        protected FieldList fieldList = new FieldList();
        protected FieldEntry fieldEntry = new FieldEntry();
        protected Real tempReal = new Real();
        protected UInt tempUInt = new UInt();

        public DataDictionary? DataDictionary { get => _dictionary; set { Debug.Assert(value != null); _dictionary = value; } }    
        public Buffer ItemName { get => _itemName; set { _itemName = value; } }
        public int ServiceId { get => _serviceId; set { _serviceId = value; } }
        public MarketPriceItem? MarketPriceItem { get => _mpInfo; set { _mpInfo = value; } }

        public ItemDomainCommonFlags Flags { get; set; }

        public bool PrivateStream
        {
            get => (Flags & ItemDomainCommonFlags.PRIVATE_STREAM) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.PRIVATE_STREAM;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.PRIVATE_STREAM;
            }
        }
        public bool HasServiceId
        {
            get => (Flags & ItemDomainCommonFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.HAS_SERVICE_ID;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.HAS_SERVICE_ID;
            }
        }

        protected MarketPriceBase()
        {
            _itemName = new Buffer();
        }

        public override void Clear()
        {
            base.Clear();
            _serviceId = 0;
            Flags = 0;
            _itemName.Clear();
        }

        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            fieldList.Clear();
            fieldEntry.Clear();

            tempUInt.Clear();
            tempReal.Clear();

            MarketPriceItem mpItem = MarketPriceItem!;

            Msg msg = EncodeMsg();

            CodecReturnCode ret = msg.EncodeInit(encodeIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            fieldList.ApplyHasStandardData();
            ret = fieldList.EncodeInit(encodeIter, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            ret = EncodeRefreshFields(encodeIter, _dictionary!);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            // TRDPRC_1
            fieldEntry.Clear();
            IDictionaryEntry dictionaryEntry = _dictionary!.Entry(MarketPriceItem.TRDPRC_1_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(mpItem.TRDPRC_1, RealHints.EXPONENT_2);
                ret = fieldEntry.Encode(encodeIter, tempReal);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // BID
            fieldEntry.Clear();
            dictionaryEntry = _dictionary.Entry(MarketPriceItem.BID_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.BID_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(mpItem.BID, RealHints.EXPONENT_2);
                ret = fieldEntry.Encode(encodeIter, tempReal);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            // ASK
            fieldEntry.Clear();
            dictionaryEntry = _dictionary.Entry(MarketPriceItem.ASK_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.ASK_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(mpItem.ASK, RealHints.EXPONENT_2);
                ret = fieldEntry.Encode(encodeIter, tempReal);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            // ACVOL_1
            fieldEntry.Clear();
            dictionaryEntry = _dictionary.Entry(MarketPriceItem.ACVOL_1_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.ACVOL_1_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(mpItem.ACVOL_1, RealHints.EXPONENT_2);
                ret = fieldEntry.Encode(encodeIter, tempReal);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            // NETCHNG_1
            fieldEntry.Clear();
            dictionaryEntry = _dictionary.Entry(MarketPriceItem.NETCHNG_1_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.NETCHNG_1_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(mpItem.NETCHNG_1, RealHints.EXPONENT_2);
                ret = fieldEntry.Encode(encodeIter, tempReal);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // ASK_TIME
            fieldEntry.Clear();
            dictionaryEntry = _dictionary.Entry(MarketPriceItem.ASK_TIME_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.ASK_TIME_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                ret = fieldEntry.Encode(encodeIter, mpItem.ASK_TIME.Time());
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            if (PrivateStream)
            {
                // PERATIO
                fieldEntry.Clear();
                dictionaryEntry = _dictionary.Entry(MarketPriceItem.PERATIO_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketPriceItem.PERATIO_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    tempReal.Clear();
                    tempReal.Value(mpItem.PERATIO, RealHints.EXPONENT_2);
                    ret = fieldEntry.Encode(encodeIter, tempReal);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                // SALTIME
                fieldEntry.Clear();
                dictionaryEntry = _dictionary.Entry(MarketPriceItem.SALTIME_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketPriceItem.SALTIME_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    ret = fieldEntry.Encode(encodeIter, mpItem.SALTIME.Time());
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            ret = fieldList.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return msg.EncodeComplete(encodeIter, true);
        }

        public abstract Msg EncodeMsg();

        protected virtual CodecReturnCode EncodeRefreshFields(EncodeIterator encodeIter, DataDictionary dictionary)
        {
            return CodecReturnCode.SUCCESS;
        }
        
        protected override StringBuilder PrepareStringBuilder()
        {
            StringBuilder stringBuf = base.PrepareStringBuilder();

            stringBuf.Append(tab);
            stringBuf.Append("itemName: ");
            stringBuf.Append(_itemName);
            stringBuf.AppendLine();

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.AppendLine();
            }
            stringBuf.Append(tab);
            stringBuf.Append("item info: ");
            stringBuf.AppendLine();
            stringBuf.Append(_mpInfo?.ToString());
            stringBuf.AppendLine();
            return stringBuf;
        }
    
        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            return 0;
        }
    }
}
