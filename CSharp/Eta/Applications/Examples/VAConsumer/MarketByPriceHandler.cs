/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;


namespace LSEG.Eta.ValueAdd.Consumer
{

    /// <summary>
    /// This is the market by price handler for the ETA consumer application. It
    /// provides method decoding a field entry from a response.
    /// </summary>
    ///
    /// Methods for sending the market by price request(s) to a provider,
    /// processing the response(s) and closing streams are inherited from
    /// MarketPriceHandler.
    internal class MarketByPriceHandler : MarketPriceHandler
    {

        private Map map = new();
        private MapEntry mapEntry = new();
        private Codec.Buffer mapKey = new();
        private LocalFieldSetDefDb localFieldSetDefDb = new();


        public MarketByPriceHandler(StreamIdWatchList watchList) : base(DomainType.MARKET_BY_PRICE, watchList)
        {
        }

        protected override MarketPriceRequest CreateMarketPriceRequest()
        {
            return new MarketByPriceRequest();
        }

        public override CodecReturnCode DecodePayload(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            //level 2 market by price is a map of field lists
            CodecReturnCode ret;
            if ((ret = map.Decode(dIter)) != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Map.Decode() failed with return code: " + ret);
                return ret;
            }

            //decode set definition database
            if (map.CheckHasSetDefs())
            {
                /*
                 * decode set definition - should be field set definition
                 */
                /*
                 * this needs to be passed in when we decode each field list
                 */
                localFieldSetDefDb.Clear();
                ret = localFieldSetDefDb.Decode(dIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"LocalFieldSetDefDb.Decode() failed: <{ret}>");
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
                    Console.WriteLine($"DecodeMapEntry() failed: <{ret}>");
                    return ret;
                }

                if (mapKey.Length > 0)
                {
                    fieldValue.Append($"PRICE POINT: {mapKey}\nACTION: {GetMapActionString()}\n");
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
            return mapEntry.Action switch
            {
                MapEntryActions.UPDATE => "UPDATE",
                MapEntryActions.ADD => "ADD",
                MapEntryActions.DELETE => "DELETE",
                _ => "Unknown"
            };
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
                    Console.WriteLine($"DecodeFieldEntry() failed: <{ret}>");
                    return ret;
                }

                ret = DecodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed");
                    return ret;
                }
                fieldValue.Append('\n');
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            CodecReturnCode ret = fieldList.Decode(dIter, localFieldSetDefDb);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"DecodeFieldList() failed: <{ret}>");
                return ret;
            }

            fieldValue.Append("SUMMARY DATA\n");

            //decode each field entry in list
            while ((ret = fieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"DecodeFieldEntry() failed: <{ret}>");

                    return ret;
                }

                ret = DecodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed");
                    return ret;
                }
                fieldValue.Append('\n');
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
