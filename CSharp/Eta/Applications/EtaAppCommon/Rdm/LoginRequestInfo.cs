/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;

namespace Refinitiv.Eta.Example.Common
{
    public class LoginRequestInfo
    {
        public LoginRequest LoginRequest { get; private set; } = new LoginRequest();
        public IChannel? Channel { get; internal set; }

        public bool IsInUse { get; internal set; }

        public LoginRequestInfo()
        {
            Clear();
        }

        public void Clear()
        {
            LoginRequest.Clear();
            Channel = null;
            IsInUse = false;
        }
    }
}
