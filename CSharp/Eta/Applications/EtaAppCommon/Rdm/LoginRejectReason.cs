/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// reasons a login request is rejected
    /// </summary>
    public enum LoginRejectReason
    {
        MAX_LOGIN_REQUESTS_REACHED,
        NO_USER_NAME_IN_REQUEST,
        LOGIN_RDM_DECODER_FAILED
    }
}
