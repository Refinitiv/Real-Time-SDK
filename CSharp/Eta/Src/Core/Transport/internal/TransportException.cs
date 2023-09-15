/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Internal
{
    internal class TransportException : Exception
    {
        public TransportException(string message) :
            base(message)
        { }

        public TransportException(string message, Exception innerException) :
            base(message, innerException)
        { }

        public override string Message =>
            (InnerException is null) ? base.Message
                                     : $"{base.Message}\n{InnerException.Message}";
    }
}
