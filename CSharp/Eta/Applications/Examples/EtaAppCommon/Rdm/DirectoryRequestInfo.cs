/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.Example.Common
{
    public class DirectoryRequestInfo
    {
        public IChannel? Channel { get; internal set; }

        public bool IsInUse { get; internal set; }

        public DirectoryRequest DirectoryRequest { get; private set; } = new DirectoryRequest();

        public DirectoryRequestInfo()
        {
            Clear();
        }

        public void Clear()
        {
            Channel = null;
            IsInUse = false;
            DirectoryRequest.Clear();
        }
    }
}