/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.Example.Common
{
    public class PriceInfo
    {
        public Real ORDER_PRC = new Real();
        public string? MKT_MKR_ID;
        public Real ORDER_SIZE = new Real();
        public string? PRICE_POINT;
        public Enum ORDER_SIDE = new Enum();
        public long QUOTIM_MS;
        public long NO_ORD;
        public Real MKOASK_VOL = new Real();
        public Real MKOBID_VOL = new Real();

        // To demonstrate the container actions "Add," "Update," and "Delete,"
        // Each order follows a cycle where it is first added, then updated some
        // number of times, then deleted, then added again. The 'lifetime'
        // member is used to control this cycle.
        public int Lifetime { get; set; }

    }
}
