/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace Refinitiv.Eta.Example.Common
{
    public class StreamIdKey
    {
        public int StreamId { get; set; }
        public override string ToString()
        {
            return "streamId: " + StreamId + "\n";
        }

        public override int GetHashCode()
        {
            return StreamId;
        }

        public override bool Equals(object? obj)
        {
            if (obj == null)
            {
                return false;
            }
            if (!(obj is StreamIdKey))
                return false;

            StreamIdKey k = (StreamIdKey)obj;
            return k.StreamId == StreamId;
        }

        public void Clear()
        {
            StreamId = 0;
        }
    }
}
