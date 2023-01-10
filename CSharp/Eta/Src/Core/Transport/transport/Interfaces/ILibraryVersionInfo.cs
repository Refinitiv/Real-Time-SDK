/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Library Version Information to be populated with <see cref="Transport"/> library version info
    /// </summary>
    /// <seealso cref="Transport"/>
    public interface ILibraryVersionInfo
    {
        /// <summary>
        /// Gets product release and load information.
        /// </summary>
        /// <value>The product version</value>
        string ProductVersion();

        /// <summary>
        /// Gets date library was produced for product release.
        /// </summary>
        /// <value>The product date</value>
        string ProductDate();
    }
}
