/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Net;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class RestResponse
    {
        public HttpStatusCode StatusCode { get; set; }

        public string? StatusText { get;set; }

        public Version? Version { get;set; }

        public string? Content { get;set; }
    }
}
