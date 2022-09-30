/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using Refinitiv.Eta.Rdm;

namespace Refinitiv.Eta.Example.VACommon
{
    /// <summary>
    /// Item argument class for the Value Add consumer and non-interactive provider applications.
    /// </summary>
    public class ItemArg
    {
        /// <summary>
        /// Domain of an item
        /// </summary>
        public DomainType Domain;

        /// <summary>
        /// Name of an item
        /// </summary>
        public string? ItemName;

        /// <summary>
        /// enable private stream for this item
        /// </summary>
        public bool EnablePrivateStream;

        /// <summary>
        /// enable symbollist datastream
        /// </summary>
        public bool SymbolListData;

        public ItemArg()
        { }

        public ItemArg(DomainType domain, string itemName, bool enablePrivateStream)
        {
            Domain = domain;
            ItemName = itemName;
            EnablePrivateStream = enablePrivateStream;
            SymbolListData = false;
        }
    }
}
