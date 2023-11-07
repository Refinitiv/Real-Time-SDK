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
    public class OrderInfo
    {
        public Real ORDER_PRC = new Real();
        public string? MKT_MKR_ID;
        public Real ORDER_SIZE = new Real();
        public string? ORDER_ID;
        public Enum ORDER_SIDE = new Enum();
        public long QUOTIM_MS;
        public Real MKOASK_VOL = new Real();
        public Real MKOBID_VOL = new Real();

        public int Lifetime;
    }
}
