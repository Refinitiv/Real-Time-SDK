/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class AccessTokenInformation
    {
        public int expires_in { get; set; }

        public string? access_token { get; set; }

        public string? token_type { get; set; }

        public AccessTokenInformation()
        {
            Clear();
        }

        public void Clear()
        {
            expires_in = -1;
            token_type = string.Empty;
            access_token = string.Empty;
        }

        public override string ToString()
        {
            return "token_type = " + token_type + ", access_token = " +
                access_token + ", expires_in = " + expires_in;
        }
    }
}
