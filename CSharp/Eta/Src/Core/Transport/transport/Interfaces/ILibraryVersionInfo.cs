/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
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
        /// <returns>The product version</returns>
        string ProductVersion();

        /// <summary>
        /// Gets product internal version, useful for raising questions or reporting issues.
        /// </summary>
        /// <returns>The internal product version</returns>
        string ProductInternalVersion();

        /// <summary>
        /// Gets date library was produced for product release.
        /// </summary>
        /// <returns>The product date</returns>
        string ProductDate();
    }
}
