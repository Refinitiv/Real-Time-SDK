/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal interface IWlLoginHandler : IWlHandler
    {
        bool SupportAllowSuspectData { get; }
        bool SupportSingleOpen { get; }

        bool SupportPost { get;  }

        bool SupportOptimizedPauseResume { get; }

        WlStream? Stream { get; internal set; }
        LoginRequest? LoginRequestForEDP { get; }
        bool IsRttEnabled { get; set; }
    }
}