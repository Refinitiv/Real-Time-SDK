/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace LSEG.Eta.Example.Common
{
    public class SelectElement
    {
        public Socket? Socket { get; set; }

        public SelectMode Mode { get; set; }

        public object? UserSpec { get; set; }

        public SelectElement()
        {
            Clear();
        }

        public void Clear()
        {
            Socket = null;
            Mode = SelectMode.READ;
        }
    }
}
