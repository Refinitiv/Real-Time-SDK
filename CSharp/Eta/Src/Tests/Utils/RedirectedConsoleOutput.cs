/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;

namespace LSEG.Eta.Tests.Utils
{
    /// <summary>
    /// Temporary redirects console output to string and disables it after <see cref="Dispose"/>.
    /// Console output can be retrieved via <see cref="ToString"/> method.
    /// </summary>
    public class RedirectedConsoleOutput : IDisposable
    {
        private readonly StringWriter m_Writer;
        private readonly TextWriter m_OriginalWriter;

        public RedirectedConsoleOutput()
        {
            m_OriginalWriter = Console.Out;
            m_Writer = new();
            Console.SetOut(m_Writer);
        }

        public void Dispose()
        {
            Console.SetOut(m_OriginalWriter);
            m_Writer.Dispose();
        }

        /// <summary>
        /// Retrieves redirected console output.
        /// </summary>
        /// <returns>Returns console output.</returns>
        public override string ToString() => m_Writer.ToString();
    }
}
