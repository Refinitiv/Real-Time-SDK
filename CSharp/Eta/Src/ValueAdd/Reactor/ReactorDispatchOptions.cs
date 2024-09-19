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
    /// ReactorDispatchOptions is used in the <see cref="Reactor.Dispatch(ReactorDispatchOptions, out ReactorErrorInfo?)"/> call.
    /// </summary>
    sealed public class ReactorDispatchOptions
    {
        private ReadArgs m_ReadArgs = new ReadArgs();
        const int DEFAULT_MAX_MESSAGE = 100;
        int m_MaxMessage = DEFAULT_MAX_MESSAGE;

        /// <summary>
        /// Gets the <see cref="ReadArgs"/>
        /// </summary>
        public ReadArgs ReadArgs
        {
            get { return m_ReadArgs; }
        }

        /// <summary>
        /// Controls the maximum number of events or messages processed in this call.
        /// </summary>
        /// <param name="maxMessages">the max messages per dispatch</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if maxMessages is valid,
        /// otherwise <see cref="ReactorReturnCode.PARAMETER_OUT_OF_RANGE"/> if the
        /// maxMessages is out of range.
        /// </returns>
        public ReactorReturnCode SetMaxMessages(int maxMessages)
        {
            if (maxMessages < 1)
                return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;

            m_MaxMessage = maxMessages;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns the maxMessages value.
        /// </summary>
        /// <returns>the maxMessages value</returns>
        public int GetMaxMessages()
        {
            return m_MaxMessage;
        }

        /// <summary>
        /// If set, dispatch events for this channel only.
        /// </summary>
        public ReactorChannel? ReactorChannel { get; set; }

        /// <summary>
        /// Clears this object for reuse.
        /// </summary>
        public void Clear()
        {
            m_ReadArgs.Clear();
            m_MaxMessage = DEFAULT_MAX_MESSAGE;
            ReactorChannel = null;
        }
    }
}
