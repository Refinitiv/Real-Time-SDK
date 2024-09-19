/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;

namespace LSEG.Eta.Example.Common
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
