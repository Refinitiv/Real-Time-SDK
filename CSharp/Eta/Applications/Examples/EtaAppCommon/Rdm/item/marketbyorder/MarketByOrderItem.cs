/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Collections.Generic;
using System.Text;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.Example.Common
{
    public class MarketByOrderItem
    {
        public const int ORDER_ID_FID = 3426;
        public const int ORDER_PRC_FID = 3427;
        public const int ORDER_SIDE_FID = 3428;
        public const int ORDER_SIZE_FID = 3429;
        public const int MKT_MKR_ID_FID = 212;
        public const int QUOTIM_MS_FID = 3855;
        public const int CURRENCY_FID = 15;
        public const int MKT_STATUS_IND_FID = 133;
        public const int ACTIVE_DATE_FID = 17;
        public const int MKOASK_VOL_FID = 3839;
        public const int MKOBID_VOL_FID = 3840;

        public const int ORDER_BID = 1;
        public const int ORDER_ASK = 2;
        public const int MAX_ORDERS = 3;

        public static Enum USD_ENUM { get; } = new Enum();
        public static Enum BBO_ENUM { get; } = new Enum();

        public string? ItemName { get; set; }
        public bool IsInUse { get; set; }
        public List<OrderInfo> OrderInfoList { get; internal set; }

        public bool IncludePrivateStream { get; set; } = false;

        private StringBuilder stringBuf = new StringBuilder();
        private const string eol = "\n";
        private const string tab = "\t";

        static MarketByOrderItem()
        {
            USD_ENUM.Value(840);
            BBO_ENUM.Value(20);
        }

        public MarketByOrderItem()
        {
            OrderInfoList = new List<OrderInfo>(MAX_ORDERS);
            for (int i = 0; i < MAX_ORDERS; i++)
            {
                OrderInfoList.Add(new OrderInfo());
            }
        }

        public void Clear()
        {
            IsInUse = false;
            foreach (OrderInfo orderInfo in OrderInfoList)
            {
                orderInfo.ORDER_ID = null;
                orderInfo.MKT_MKR_ID = null;
                orderInfo.ORDER_PRC.Clear();
                orderInfo.ORDER_SIZE.Clear();
                orderInfo.ORDER_SIDE.Clear();
                orderInfo.QUOTIM_MS = 0;
                orderInfo.MKOASK_VOL.Clear();
                orderInfo.MKOBID_VOL.Clear();
                orderInfo.Lifetime = 0;
            }
        }

        public void InitFields()
        {
            IsInUse = true;
            int idx = -1;
            foreach (OrderInfo orderInfo in OrderInfoList)
            {
                ++idx;
                orderInfo.ORDER_ID = (idx + 100).ToString();
                orderInfo.MKT_MKR_ID = "MarketMaker" + (idx + 1);
                orderInfo.ORDER_PRC.Value(3459 + 100 * idx, RealHints.EXPONENT_2);
                orderInfo.ORDER_SIZE.Value(5 + idx, RealHints.EXPONENT0);
                orderInfo.ORDER_SIDE.Value((idx >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID);
                orderInfo.QUOTIM_MS = 500 * idx;
                orderInfo.MKOASK_VOL.Value(2 + idx, RealHints.EXPONENT0);
                orderInfo.MKOBID_VOL.Value(3 + idx, RealHints.EXPONENT0);
                orderInfo.Lifetime = 5 + idx;
            }
        }

        public void UpdateFields()
        {
            int idx = -1;

            foreach (OrderInfo orderInfo in OrderInfoList)
            {
                ++idx;
                orderInfo.ORDER_PRC.Value(orderInfo.ORDER_PRC.ToLong() + 1, orderInfo.ORDER_PRC.Hint);
                orderInfo.QUOTIM_MS += 1000;

                orderInfo.MKOASK_VOL.Value(orderInfo.MKOASK_VOL.ToLong() + 1,
                                           orderInfo.ORDER_PRC.Hint);
                orderInfo.MKOBID_VOL.Value(orderInfo.MKOASK_VOL.ToLong() + 1,
                                           orderInfo.ORDER_PRC.Hint);

                if (orderInfo.Lifetime == 0)
                    orderInfo.Lifetime = 5 + idx;
                else
                    --orderInfo.Lifetime;
            }
        }

        public override string ToString()
        {
            stringBuf.Clear();
            foreach (OrderInfo orderInfo in OrderInfoList)
            {
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER ID: ");
                stringBuf.Append(orderInfo.ORDER_ID);
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("MRKT MAKER ID: ");
                stringBuf.Append(orderInfo.MKT_MKR_ID);
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER PRICE: ");
                stringBuf.Append(orderInfo.ORDER_PRC.ToDouble());
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER SIZE: ");
                stringBuf.Append(orderInfo.ORDER_SIZE.ToDouble());
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER SIDE: ");
                stringBuf.Append((orderInfo.ORDER_SIDE.ToInt() == ORDER_BID) ? "BID" : "ASK");
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("QUOTIM MS: ");
                stringBuf.Append(orderInfo.QUOTIM_MS);
                stringBuf.Append(eol);
                if (IncludePrivateStream)
                {
                    stringBuf.Append(tab);
                    stringBuf.Append(tab);
                    stringBuf.Append("MKT ORDER ASK VOL: ");
                    stringBuf.Append(orderInfo.MKOASK_VOL);
                    stringBuf.Append(eol);
                    stringBuf.Append(tab);
                    stringBuf.Append(tab);
                    stringBuf.Append("MKT ORDER BID VOL: ");
                    stringBuf.Append(orderInfo.MKOBID_VOL);
                    stringBuf.Append(eol);
                    stringBuf.Append(tab);
                    stringBuf.Append(tab);
                    stringBuf.Append("Lifetime: ");
                    stringBuf.Append(orderInfo.Lifetime);
                    stringBuf.Append(eol);
                }
            }
            return stringBuf.ToString();
        }
    }
}
