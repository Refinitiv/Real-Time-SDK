/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.Example.Common
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
