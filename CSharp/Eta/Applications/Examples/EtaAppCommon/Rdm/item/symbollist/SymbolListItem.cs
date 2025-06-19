/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using LSEG.Eta.ValueAdd.Rdm;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    public class SymbolListItem
    {
        private Buffer itemName;
        public bool IsInUse { get; set; }
        public int InterestCount { get; set; }
        public Buffer ItemName { get => itemName; set { Debug.Assert(value != null); BufferHelper.CopyBuffer(value, itemName); } }

        public SymbolListItem()
        {
            IsInUse = false;
            InterestCount = 0;
            itemName = new Buffer();
        }

        public void Clear()
        {
            IsInUse = false;
            InterestCount = 0;
            itemName.Clear();
        }
    }
}
