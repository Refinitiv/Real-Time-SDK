/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Eta.Example.Common
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

        //APIQA

        private int qosRate = 0;
        private int qosTimeliness = 0;
        private int worstQosRate = 0;
        private int worstQosTimeliness = 0;
        private bool isReissueChangeServiceName = false;
        private int changeServiceId = 0;
        private bool privateStreamReq = false;
        private bool sendReqSameKeyWithDiffStreamId = false;
        private int specificServiceId = 0;
        public void SetReissueQos(int _qosRate, int _qosTimeliness)
        {
            qosRate = _qosRate;
            qosTimeliness = _qosTimeliness;
        }
        public void SetReissueQosAndWorstQos(int _qosRate, int _qosTimeliness, int _worstQosRate, int _worstQosTimeliness) 
        {
            qosRate = _qosRate;
            qosTimeliness = _qosTimeliness;
            worstQosRate = _worstQosRate;
            worstQosTimeliness = _worstQosTimeliness;
        }

        public void SetReissueChangeServiceName(int newServiceId) 
        {
            isReissueChangeServiceName = true;
            changeServiceId = newServiceId;
        }
        public void SetPriviateStream() 
        {
            privateStreamReq = true;
        }
        public void SetReqSameKeyWithDiffStreamId()
        {
            sendReqSameKeyWithDiffStreamId = true;
        }
        public void SetSpecificServiceId(int _specificServiceId)
        {
            specificServiceId = _specificServiceId;
        }

        //END APIQA
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

