/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           	   Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ETA Reactor Channel IOCtl codes for <see cref="ReactorChannel.IOCtl(ReactorChannelIOCtlCode, object, out ReactorErrorInfo?)"/>
    /// </summary>
    public enum ReactorChannelIOCtlCode
    {
        /// <summary>
        /// This option is used to fallback to a preferred host.
        /// </summary>
        PREFERRED_HOST_OPTIONS = 201
    }
}
