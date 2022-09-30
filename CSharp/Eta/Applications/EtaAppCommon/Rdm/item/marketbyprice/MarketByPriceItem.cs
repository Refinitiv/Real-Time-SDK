/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System;
using System.Collections.Generic;
using System.Text;
using Enum = Refinitiv.Eta.Codec.Enum;

namespace Refinitiv.Eta.Example.Common
{
    public class MarketByPriceItem
    {
        public const int ORDER_PRC_FID = 3427; /* Order Price */
        public const int ORDER_SIDE_FID = 3428; /* Order Side */
        public const int ORDER_SIZE_FID = 3429; /* Order Size */
        public const int QUOTIM_MS_FID = 3855; /* Quote Time */
        public const int NO_ORD_FID = 3430; /* Number of Orders */
        public const int MKOASK_VOL_FID = 3839;
        public const int MKOBID_VOL_FID = 3840;
        public const int CURRENCY_FID = 15;

        public const int ORDER_BID = 1;
        public const int ORDER_ASK = 2;
        public const int MAX_ORDERS = 3;

        public static Enum USD_ENUM = new Enum();
        public static Enum BBO_ENUM = new Enum();

        public string? ItemName { get; set; }
        public bool IsInUse { get; set; }

        /* market by order item data */
        public List<PriceInfo> PriceInfoList = new List<PriceInfo>(MAX_ORDERS);

        public bool includePrivateStream = false;

        private StringBuilder stringBuf = new StringBuilder();
        private const char eol = '\n';
        private const char tab = '\t';

        static MarketByPriceItem()
        {
            USD_ENUM.Value(840);
            BBO_ENUM.Value(20);
        }

        public MarketByPriceItem()
        {
            for (int i = 0; i < MAX_ORDERS; i++)
            {
                PriceInfoList.Add(new PriceInfo());
            }
        }

        public void Clear()
        {
            IsInUse = false;
            foreach (PriceInfo priceInfo in PriceInfoList)
            {
                priceInfo.MKT_MKR_ID = null;
                priceInfo.ORDER_PRC.Clear();
                priceInfo.ORDER_SIZE.Clear();
                priceInfo.ORDER_SIDE.Clear();
                priceInfo.QUOTIM_MS = 0;
                priceInfo.PRICE_POINT = null;
                priceInfo.NO_ORD = 0;
                priceInfo.MKOASK_VOL.Clear();
                priceInfo.MKOBID_VOL.Clear();
                priceInfo.Lifetime = 0;
            }
        }

        public void InitFields()
        {
            IsInUse = true;
            int idx = -1;
            foreach (PriceInfo priceInfo in PriceInfoList)
            {
                ++idx;
                priceInfo.MKT_MKR_ID = "MarketMaker" + (idx + 1);
                priceInfo.ORDER_PRC.Value(3459 + 100 * idx, RealHints.EXPONENT_2);
                priceInfo.ORDER_SIZE.Value(5 + idx, RealHints.EXPONENT0);
                priceInfo.ORDER_SIDE.Value((idx >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID);
                priceInfo.QUOTIM_MS = 500 * idx;
                priceInfo.PRICE_POINT = priceInfo.ORDER_PRC + ((idx >= MAX_ORDERS / 2) ? "a" : "b");
                priceInfo.NO_ORD = 1 + idx;
                priceInfo.MKOASK_VOL.Value(2 + idx, RealHints.EXPONENT0);
                priceInfo.MKOBID_VOL.Value(3 + idx, RealHints.EXPONENT0);
                priceInfo.Lifetime = 5 + idx;
            }
        }

        public override string ToString()
        {
            stringBuf.Clear();
            foreach (PriceInfo priceInfo in PriceInfoList)
            {
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("MRKT MAKER ID: ");
                stringBuf.Append(priceInfo.MKT_MKR_ID);
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER PRICE: ");
                stringBuf.Append(priceInfo.ORDER_PRC.ToDouble());
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER SIZE: ");
                stringBuf.Append(priceInfo.ORDER_SIZE.ToDouble());
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("ORDER SIDE: ");
                stringBuf.Append((priceInfo.ORDER_SIDE.ToInt() == ORDER_BID) ? "BID" : "ASK");
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("QUOTIM MS: ");
                stringBuf.Append(priceInfo.QUOTIM_MS);
                stringBuf.Append(eol);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("No ORDERS: ");
                stringBuf.Append(priceInfo.NO_ORD);
                stringBuf.Append(eol);
                if (includePrivateStream)
                {
                    stringBuf.Append(tab);
                    stringBuf.Append(tab);
                    stringBuf.Append("MKT ORDER ASK VOL: ");
                    stringBuf.Append(priceInfo.MKOASK_VOL);
                    stringBuf.Append(eol);
                    stringBuf.Append(tab);
                    stringBuf.Append(tab);
                    stringBuf.Append("MKT ORDER BID VOL: ");
                    stringBuf.Append(priceInfo.MKOBID_VOL);
                    stringBuf.Append(eol);
                    stringBuf.Append(tab);
                    stringBuf.Append(tab);
                    stringBuf.Append("Lifetime: ");
                    stringBuf.Append(priceInfo.Lifetime);
                    stringBuf.Append(eol);
                }
            }

            return stringBuf.ToString();
        }

        public void UpdateFields()
        {
            int idx = -1;

            foreach (PriceInfo priceInfo in PriceInfoList)
            {
                ++idx;
                priceInfo.ORDER_PRC.Value(priceInfo.ORDER_PRC.ToLong() + 1, priceInfo.ORDER_PRC.Hint);
                priceInfo.QUOTIM_MS += 1000;

                priceInfo.MKOASK_VOL.Value(priceInfo.MKOASK_VOL.ToLong() + 1,
                                           priceInfo.ORDER_PRC.Hint);
                priceInfo.MKOBID_VOL.Value(priceInfo.MKOASK_VOL.ToLong() + 1,
                                           priceInfo.ORDER_PRC.Hint);

                if (priceInfo.Lifetime == 0)
                    priceInfo.Lifetime = 5 + idx;
                else
                    --priceInfo.Lifetime;
            }
        }

    }
}
