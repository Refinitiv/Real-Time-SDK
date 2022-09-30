/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;
using System;

namespace Refinitiv.Eta.Example.Common
{
    public class LoginRttInfo
    {
        public LoginRTT LoginRtt { get; private set; } = new LoginRTT();
        public IChannel? Channel { get; set; }
        public bool IsInUse { get; set; }
        public long RttLastSendNanoTime { get; set; }

        public LoginRttInfo()
        {
            Clear();
            RttLastSendNanoTime = DateTime.Now.Ticks * 100;
        }

        public void Clear()
        {
            LoginRtt.Clear();
            Channel = null;
            IsInUse = false;
        }
    }
}
