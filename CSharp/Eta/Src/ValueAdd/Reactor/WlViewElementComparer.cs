/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class WlViewIntElementComparer : IComparer<WlViewElement<int>>
    {
        public int Compare(WlViewElement<int>? first, WlViewElement<int>? second)
        {
            if (first!.Value < second!.Value)
            {
                return -1;
            }
            else if (first!.Value == second!.Value)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }

    internal class WlViewStringElementComparer : IComparer<WlViewElement<String>>
    {
        public int Compare(WlViewElement<String>? first, WlViewElement<String>? second)
        {
            return first!.Value!.CompareTo(second!.Value);
        }
    }
}
