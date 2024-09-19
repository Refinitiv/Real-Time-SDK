/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Transports.proxy
{
    internal class ProxyAuthenticationException : Exception
    {
        public ProxyAuthenticationException(string message) : base(message)
        {
        }

        public ProxyAuthenticationException(string message, Exception innerException) :
            base(message, innerException)
        {
        }
    }
}
