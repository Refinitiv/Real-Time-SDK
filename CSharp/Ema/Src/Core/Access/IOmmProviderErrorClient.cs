/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// IOmmProviderErrorClient interface provides callback mechanism used in place of exceptions and dispatching failures.
    /// </summary>
    /// <remarks>
    /// <para>
    /// By default OmmProvider class throws exceptions if usage errors occur. Specifying IOmmProviderErrorClient
    /// on the constructor of OmmProvider overwrites this behaviour. Instead of throwing exceptions, respective
    /// callback methods on IOmmProviderErrorClient will be invoked.
    /// </para>
    /// <para>
    /// Thread safety of all the methods in this class depends on user's implementation.
    /// </para>
    /// </remarks>
    /// <seealso cref="OmmProvider"/>
    /// <seealso cref="OmmInvalidUsageException"/>
    /// <seealso cref="OmmInvalidHandleException"/>
    public interface IOmmProviderErrorClient
    {
        /// <summary>
        /// Invoked upon receiving an invalid handle.
        /// <remark>
        /// Requires OmmProvider constructor to have an IOmmProviderErrorClient.
        /// </remark>
        /// </summary>
        /// <param name="handle">value of the handle that is invalid</param>
        /// <param name="text">associated error text</param>
        public void OnInvalidHandle(long handle, string text) { }

        /// <summary>
        /// Invoked in the case of invalid usage. Requires OmmProvider constructor
        /// to have an IOmmProviderErrorClient.
        /// </summary>
        /// <param name="text">associated error text</param>
        /// <param name="errorCode">associated error code</param>
        public void OnInvalidUsage(string text, int errorCode) { }

        /// <summary>
        /// Invoked in the case of dispatching failures from Reactor component. Requires OmmProvider
        /// constructor to have an IOmmProviderErrorClient.
        /// </summary>
        /// <param name="text">associated error text</param>
        /// <param name="errorCode">associated error code defined in <see cref="DispatchErrorCode"/></param>
        public void OnDispatchError(string text, int errorCode) { }
    }
}
