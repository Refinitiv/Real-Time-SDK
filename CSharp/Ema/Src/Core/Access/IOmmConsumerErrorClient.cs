/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmConsumerErrorclient class provides callback mechanism used in place of exceptions.
    /// 
    /// <para>
    /// By default OmmConsumer class throws exceptions if a usage error occurs.<br/>
    /// Specifying OmmConsumerErrorClient on the constructor of OmmConsumer overwrites this
    /// behaviour.<br/>
    /// Instead of throwing exceptions, respective callback method on OmmConsumerErrorClient
    /// will be invoked.<br/>
    /// </para>
    /// <remark>
    /// Thread safety of all the methods in this class depends on user's implementation
    /// </remark>
    /// </summary>
    /// <seealso cref="OmmInvalidUsageException"/>
    /// <seealso cref="OmmInvalidHandleException"/>
    public interface IOmmConsumerErrorClient
    {
        /// <summary>
        /// Invoked upon receiving an invalid handle.
        /// <remark>
        /// Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
        /// </remark>
        /// </summary>
        /// <param name="handle">value of the handle that is invalid</param>
        /// <param name="text">associated error text</param>
        public void OnInvalidHandle(long handle, string text) { }

        /// <summary>
        /// Invoked in the case of invalid usage. Requires OmmConsumer constructor
        /// to have an OmmConsumerErrorClient.
        /// </summary>
        /// <param name="text">associated error text</param>
        /// <param name="errorCode">associated error code</param>
        public void OnInvalidUsage(string text, int errorCode) { }
    }
}
