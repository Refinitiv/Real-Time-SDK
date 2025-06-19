/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.PerfTools.Common
{
    public class ItemRequest
    {
        public int RequestState;
        public string? ItemName;
        public ItemInfo ItemInfo = new ItemInfo();

        public void Clear()
        {
            ItemInfo.Clear();
        }
    }
}
