/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Example.VACommon
{

    /// <summary>
    /// Interface for Value Add application command line parsing.
    /// </summary>
    public interface ICommandLineParser
    {

        /// <summary>
        /// Parses command line arguments for application.
        /// </summary>
        ///
        /// <param name="args">array of command line arguments</param>
        ///
        /// <returns>true, if arguments parse correctly or false, otherwise</returns>
        public bool ParseArgs(string[] args);

        /// <summary>
        /// Prints usage for application command line.
        /// </summary>
        public void PrintUsage();
    }

}
