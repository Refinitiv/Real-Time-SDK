/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorAcceptOptions to be used in the <see cref="Reactor.Accept(IServer, 
    /// ReactorAcceptOptions, ReactorRole, out ReactorErrorInfo?)"/> call.
    /// </summary>
    sealed public class ReactorAcceptOptions
    {
        private const int DEFAULT_TIME = 60;
        int m_Timeout = DEFAULT_TIME;

        /// <summary>
        /// Gets ETA AcceptOptions.
        /// </summary>
        public AcceptOptions AcceptOptions { get; private set; }

        /// <summary>
        /// Create <see cref="ReactorAcceptOptions"/>
        /// </summary>
        public ReactorAcceptOptions()
        {
            AcceptOptions = new AcceptOptions();
        }

        /// <summary>
        /// Sets the amount of time (in seconds) to wait for the successful connection
        /// establishment of a <see cref="ReactorChannel"/>. If a timeout occurs,
        /// an event is dispatched to the application to indicate that the ReactorChannel
        /// is down. Timeout must be greater than zero. Default is 60 seconds.
        /// </summary>
        /// <param name="timeout">the initialization timeout in seconds</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if the timeout is valid,
        /// otherwise <see cref="ReactorReturnCode.PARAMETER_OUT_OF_RANGE"/> if 
        /// the timeout is out of range</returns>
        public ReactorReturnCode SetInitTimeout(int timeout)
        {
            if (timeout < 1)
                return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;

            m_Timeout = timeout;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Clears current ReactorAcceptOptions instance
        /// </summary>
        public void Clear()
        {
            if (AcceptOptions != null)
            {
                AcceptOptions.Clear();
            }
        }

        internal int GetInitTimeout()
        {
            return m_Timeout;
        }
    }
}
