/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Text;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// This is the market price handler for the ETA consumer application. 
    /// It provides methods for sending the market price request(s) to a provider 
    /// and processing the response(s). Methods for decoding 
    /// a field entry from aresponse, and closing market price streams are also provided.
    /// </summary>
    public class MarketPriceHandler : MarketDomainHandlerBase
    {        
        public MarketPriceHandler(StreamIdWatchList watchList) : base((int)DomainType.MARKET_PRICE, watchList) {}

        protected override MarketRequest CreateRequest()
        {
            return new MarketPriceRequest();
        }
        protected override CodecReturnCode Decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            CodecReturnCode ret = fieldList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeFieldList() failed with return code: " + ret);
                return ret;
            }

            StringBuilder fieldValue = new StringBuilder();
            GetItemName(msg, fieldValue);
            if (msg.MsgClass == MsgClasses.REFRESH)
                fieldValue.Append((((IRefreshMsg)msg).State).ToString() + "\n");

            // Decode each field entry in list
            while ((ret = fieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed with return code: " + ret);
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
            Console.WriteLine(fieldValue.ToString());

            return CodecReturnCode.SUCCESS;
        }

    }
}

