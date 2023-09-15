/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// Domain Type enumeration values, see RDM Usage Guide for domain model
    /// definitions (DMT = DomainType). This will be extended as new message types
    /// are defined and implemented.
    /// </summary>
    public enum DomainType : byte
    {
        /// <summary>
        /// Login Message </summary>
        LOGIN = 1,
        /// <summary>
        /// Source Message </summary>
        SOURCE = 4,
        /// <summary>
        /// Dictionary Message </summary>
        DICTIONARY = 5,
        /// <summary>
        /// Market Price Message </summary>
        MARKET_PRICE = 6,
        /// <summary>
        /// Market by Order/Order Book Message </summary>
        MARKET_BY_ORDER = 7,
        /// <summary>
        /// Market by Price/Market Depth Message </summary>
        MARKET_BY_PRICE = 8,
        /// <summary>
        /// Market Maker Message </summary>
        MARKET_MAKER = 9,
        /// <summary>
        /// Symbol List Messages </summary>
        SYMBOL_LIST = 10,
        /// <summary>
        /// Service Provider Status </summary>
        SERVICE_PROVIDER_STATUS = 11,
        /// <summary>
        /// History Message </summary>
        HISTORY = 12,
        /// <summary>
        /// Headline Message </summary>
        HEADLINE = 13,
        /// <summary>
        /// Story Message </summary>
        STORY = 14,
        /// <summary>
        /// Replay Headline Message </summary>
        REPLAYHEADLINE = 15,
        /// <summary>
        /// Replay Story Message </summary>
        REPLAYSTORY = 16,
        /// <summary>
        /// Transaction Message </summary>
        TRANSACTION = 17,
        /// <summary>
        /// Yield Curve </summary>
        YIELD_CURVE = 22,
        /// <summary>
        /// Contribution </summary>
        CONTRIBUTION = 27,
        /* 28 reserved for now */
        /// <summary>
        /// Provider Message </summary>
        PROVIDER_ADMIN = 29,
        /// <summary>
        /// Analytics </summary>
        ANALYTICS = 30,
        /// <summary>
        /// Reference </summary>
        REFERENCE = 31,
        /// <summary>
        /// News Text Analytics </summary>
        NEWS_TEXT_ANALYTICS = 33,
        /// <summary>
        /// Economic Indicator domain </summary>
        ECONOMIC_INDICATOR = 34,
        /// <summary>
        /// Poll domain </summary>
        POLL = 35,
        /// <summary>
        /// Forecast domain </summary>
        FORECAST = 36,
        /// <summary>
        /// Market By Time domain </summary>
        MARKET_BY_TIME = 37,
        /// <summary>
        /// System domain for use with domain neutral content (e.g. tunnel stream creation) </summary>
        SYSTEM = 127,
        /// <summary> Maximum reserved message type value </summary>
        MAX_RESERVED = 127,
        /// <summary> Maximum value for a message type </summary>
        MAX_VALUE = 255
    }

    /// <summary>
    /// Domain Type enumeration values, see RDM Usage Guide for domain model
    /// definitions(DMT = DomainType). This will be extended as new message types
    /// are defined and implemented.
    /// </summary>
    public sealed partial class DomainTypes
    {
        /// <summary>
        /// String representation of a domain type name.
        /// </summary>
        /// <param name="domainType"> domain type
        /// </param>
        /// <returns> the string representation of a domain type name
        /// </returns>
        public static string ToString(int domainType) => ToString((DomainType)domainType);

        /// <summary>
        /// String representation of a domain type name.
        /// </summary>
        /// <param name="domainType"> domain type
        /// </param>
        /// <returns> the string representation of a domain type name
        /// </returns>
        public static string ToString(DomainType domainType) => $"{domainType}";

        /// <summary>
        /// Returns domainType value from domain type string.
        /// </summary>
        /// <param name="domainTypeString"> domain type string representation
        /// </param>
        /// <returns> the domainType value
        /// </returns>
        public static int DomainTypeFromString(string domainTypeString)
        {
            int domainType = (int)Enum.Parse(typeof(DomainType), domainTypeString);
            return domainType;
        }
    }

}