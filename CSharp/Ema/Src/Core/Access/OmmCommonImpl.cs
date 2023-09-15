/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Eta.Common;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// This interface defines base contract methods for consumer and provider application types.
    /// </summary>
    internal interface IOmmCommonImpl
    {
        public enum ImpleType
        {
            CONSUMER,
            NIPROVIDER,
            IPROVIDER
        };

        EmaObjectManager GetEmaObjManager();

        string InstanceName { get; set; }

        StringBuilder GetStrBuilder();

        void EventReceived();

        Locker GetUserLocker();
        void ChannelInformation(ChannelInformation channelInformation);
    }
}
