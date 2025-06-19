/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.Internal.Interfaces
{
   internal interface IRipcBuffer
   {
      TransportReturnCode Pack(out Error error);
   }
}
