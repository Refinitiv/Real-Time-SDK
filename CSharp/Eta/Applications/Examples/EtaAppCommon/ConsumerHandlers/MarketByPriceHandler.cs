/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the market by price handler for the ETA consumer application. 
    /// It provides method decoding a field entry from a response. 
    /// </summary>
    public class MarketByPriceHandler : MarketDomainHandlerBase
    {
        private Map map = new Map();
        private MapEntry mapEntry = new MapEntry();
        private Buffer mapKey = new Buffer();
        private LocalFieldSetDefDb localFieldSetDefDb = new LocalFieldSetDefDb();

        public MarketByPriceHandler(StreamIdWatchList watchList) : base((int)DomainType.MARKET_BY_PRICE, watchList) { }

        protected override MarketRequest CreateRequest()
        {
            return new MarketByPriceRequest();
        }

        protected override CodecReturnCode Decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            StringBuilder fieldValue = new StringBuilder("\n");
            GetItemName(msg, fieldValue);
            if (msg.MsgClass == MsgClasses.REFRESH)
                fieldValue.Append(((IRefreshMsg)msg).State.ToString() + "\n");

            CodecReturnCode ret = map.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeMap() failed with return code: " + ret);
                return ret;
            }

            if (map.CheckHasSetDefs())
            {
                localFieldSetDefDb.Clear();
                ret = localFieldSetDefDb.Decode(dIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeLocalFieldSetDefDb() failed");
                    return ret;
                }
            }
            if (map.CheckHasSummaryData())
            {
                ret = DecodeSummaryData(dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            ret = DecodeMap(dIter, dictionary, fieldValue);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            Console.WriteLine(fieldValue.ToString());
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeMap(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            CodecReturnCode ret;
            while ((ret = mapEntry.Decode(dIter, mapKey)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeMapEntry() failed: <" + ret + ">");
                    return ret;
                }

                if (mapKey.Length > 0)
                {
                    fieldValue.Append("PRICE POINT: " + mapKey.ToString() + "\nACTION: "
                                + GetMapActionString() + "\n");
                }

                if (mapEntry.Action != MapEntryActions.DELETE)
                {
                    ret = DecodeUpdateFieldList(dIter, dictionary, fieldValue);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }
            return CodecReturnCode.SUCCESS;
        }

        private string GetMapActionString()
        {
            String actionString;
            switch (mapEntry.Action)
            {
                case MapEntryActions.UPDATE:
                    return "UPDATE";
                case MapEntryActions.ADD:
                    actionString = "ADD";
                    break;
                case MapEntryActions.DELETE:
                    actionString = "DELETE";
                    break;
                default:
                    actionString = "Unknown";
                    break;

            }
            return actionString;
        }

        private CodecReturnCode DecodeUpdateFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            //decode field list
            CodecReturnCode ret = fieldList.Decode(dIter, localFieldSetDefDb);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeFieldList() failed with return code: " + ret);
                return ret;
            }

            //decode each field entry in list
            while ((ret = fieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed: <" + ret + ">");
                    return ret;
                }

                ret = DecodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("decodeFieldEntry() failed");
                    return ret;
                }
                fieldValue.Append("\n");
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            CodecReturnCode ret = fieldList.Decode(dIter, localFieldSetDefDb);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("decodeFieldList() failed: <" + ret + ">");
                return ret;
            }

            fieldValue.Append("SUMMARY DATA\n");

            //decode each field entry in list
            while ((ret = fieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("decodeFieldEntry() failed: <" + ret + ">");

                    return ret;
                }

                ret = DecodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("decodeFieldEntry() failed");
                    return ret;
                }
                fieldValue.Append("\n");
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
