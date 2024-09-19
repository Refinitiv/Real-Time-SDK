/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.PerfTools.Common
{
    public class ItemAttributes
    {
        public int DomainType;
        public string? ServiceName;
        public int ServiceId;
        public int NameType;
        public string? Name;
        public bool HasNameType;

        /// <summary>
        /// Clears current <see cref="ItemAttributes"/> instance.
        /// </summary>
        public void Clear()
        {
            DomainType = 0;
            ServiceId = 0;
            NameType = 1;
            ServiceName = null;
            Name = null;
            HasNameType = false;
        }
    }
}
