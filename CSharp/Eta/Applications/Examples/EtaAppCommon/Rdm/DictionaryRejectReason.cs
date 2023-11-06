/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Reasons a dictionary request is rejected.
    /// </summary>
    public enum DictionaryRejectReason
    {
        UNKNOWN_DICTIONARY_NAME,
        MAX_DICTIONARY_REQUESTS_REACHED,
        DICTIONARY_RDM_DECODER_FAILED
    }
}