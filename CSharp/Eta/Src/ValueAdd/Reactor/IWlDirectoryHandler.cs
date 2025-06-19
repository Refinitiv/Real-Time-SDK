/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal interface IWlDirectoryHandler : IWlHandler
    {
        public int ServiceId(string serviceName);
        public WlServiceCache ServiceCache { get; set; }
        public void DeleteAllServices(bool isChannelDown, out ReactorErrorInfo? errorInfo);
        public ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo? errorInfo);
        public ReactorReturnCode LoginStreamClosed();
        public void Init();
    }
}