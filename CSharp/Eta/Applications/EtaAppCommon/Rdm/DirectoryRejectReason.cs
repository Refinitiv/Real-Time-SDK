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
    /// Reasons a directory request is rejected
    /// </summary>
    public enum DirectoryRejectReason
    {
        MAX_SRCDIR_REQUESTS_REACHED,
        INCORRECT_FILTER_FLAGS,
        DIRECTORY_RDM_DECODER_FAILED
    }
}
