/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports
{
    class LibraryVersionInfoImpl : ILibraryVersionInfo
    {
        internal string m_ProductVersion;
        internal string m_ProductDate;
        internal string m_ProductInternalVersion;

        /// <summary>
        /// Gets date library was produced for product release.
        /// </summary>
        /// <returns>The product date</returns>
        public string ProductDate()
        {
            return m_ProductDate;
        }

        /// <summary>
        /// Gets product release and load information.
        /// </summary>
        /// <returns>The product version</returns>
        public string ProductVersion()
        {
            return m_ProductVersion;
        }

        /// <summary>
        /// Gets Internalinformation, useful for raising questions or reporting issues.
        /// </summary>
        /// <returns>The product internal version</returns>
        public string ProductInternalVersion()
        {
            return m_ProductInternalVersion;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return "Enterprise Transport API (ETA), C# Edition, LibraryVersionInfo" + "\n" +
               "\tproductVersion: " + m_ProductVersion + "\n" +
               "\tproductInternalVersion: " + m_ProductInternalVersion + "\n" +
               "\tproductDate: " + m_ProductDate;
        }
    }
}
