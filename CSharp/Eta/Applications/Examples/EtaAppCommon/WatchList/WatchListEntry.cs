/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    public class WatchListEntry
    {
        public bool IsPrivateStream { get; set; }
        public string? ItemName { get; set; }
        public State? ItemState { get; set; }
        public int DomainType { get; set; }
        public override string ToString()
        {
            return "isPrivateStream: " + IsPrivateStream +
                    ", itemName: " + ItemName + ", itemState: " + ItemState +
                    ", domainType:" + DomainType + "\n";
        }
    }
}
