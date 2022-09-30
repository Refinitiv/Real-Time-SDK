/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Rdm;


namespace Refinitiv.Eta.ValueAdd.Consumer
{

    /// <summary>
    /// This is the market by order handler for the ETA Value Add consumer
    /// application. It provides methods for decoding a map containing field
    /// list contents, decoding field entries from a response.
    /// </summary>
    ///
    /// Methods for sending the market by order request(s) to a provider,
    /// processing the response(s) and closing streams are inherited from
    /// MarketPriceHandler.
    internal class MarketByOrderHandler : MarketPriceHandler
    {
        private Map map = new();
        private MapEntry mapEntry = new();
        private Codec.Buffer mapKey = new();
        private LocalFieldSetDefDb localFieldSetDefDb = new();

        public MarketByOrderHandler(StreamIdWatchList watchList) : base(DomainType.MARKET_BY_ORDER, watchList)
        {
        }


        protected override MarketPriceRequest CreateMarketPriceRequest()
        {
            return new MarketByOrderRequest();
        }


        public CodecReturnCode DecodePayload(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
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
                    Console.WriteLine($"DecodeLocalFieldSetDefDb() failed: <{ret}>");
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
            string actionString;
            CodecReturnCode ret;
            /* decode the map */
            while ((ret = mapEntry.Decode(dIter, mapKey)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeMapEntry() failed with return code: " + ret);
                    return ret;
                }

                //convert the action to a string for display purposes
                actionString = mapEntry.Action switch
                {
                    MapEntryActions.UPDATE => "UPDATE",
                    MapEntryActions.ADD => "ADD",
                    MapEntryActions.DELETE => "DELETE",
                    _ => "Unknown"
                };

                //print out the key
                if (mapKey.Length > 0)
                {
                    fieldValue.Append($"ORDER ID: {mapKey}\nACTION: {actionString}\n");
                }

                //there is not any payload in delete actions
                if (mapEntry.Action != MapEntryActions.DELETE)
                {
                    ret = DecodeFieldList(dIter, dictionary, fieldValue);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }
            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode DecodeFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            //decode field list
            CodecReturnCode ret = fieldList.Decode(dIter, localFieldSetDefDb);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"DecodeFieldList() failed: <{ret}>");
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
                fieldValue.Append("\n");
            }

            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode DecodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            CodecReturnCode ret = fieldList.Decode(dIter, localFieldSetDefDb);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"DecodeFieldList failed: <{ret}>");
                return ret;
            }

            fieldValue.Append("SUMMARY DATA\n");
            //decode each field entry in list
            while ((ret = fieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"DecodeFieldEntry failed: <{ret}>");
                    return ret;
                }

                ret = DecodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed");
                    return ret;
                }
                fieldValue.Append("\n");
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
