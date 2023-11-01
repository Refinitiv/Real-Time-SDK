/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Item request state
    /// </summary>
    public enum ItemRequestState : int
    {
        // Item request has not been set.
        NOT_REQUESTED = 0,
        // Item is waiting for its solicited refresh.
        WAITING_FOR_REFRESH = 1,
        // Item has received its solicited refresh.
        HAS_REFRESH = 2
    }
}
