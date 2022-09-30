/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Rdm;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Xml;

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// Builds a list of items from an XML file. 
    /// This may be used by applications to determine what items to open 
    /// when starting a session.
    /// </summary>
    public class XmlItemInfoList
    {
        private int m_CurrentItemCount;
        private const string SECTION_NAME = "itemList";
        private const string ITEM_NAME = "item";
        private const string DOMAIN_TAG = "domain";
        private const string NAME_TAG = "name";
        private const string POST_TAG = "post";
        private const string GEN_TAG = "generic";
        private const string SNAP_TAG = "snapshot";

        /// <summary>
        /// The total number of items in the list
        /// </summary>
        public int ItemInfoCount { get; internal set; }

        /// <summary>
        /// Number of items in list for posting
        /// </summary>
        public int PostMsgItemCount { get; internal set; }

        /// <summary>
        /// Number of items in list for sending generic msgs
        /// </summary>
        public int GenMsgItemCount { get; internal set; }

        /// <summary>
        /// The list of items
        /// </summary>
        public XmlItemInfo[] ItemInfoList { get; internal set; }

        /// <summary>
        /// Instantiates a new xml item info list.
        /// </summary>
        /// <param name="itemCount">the item count</param>
        public XmlItemInfoList(int itemCount)
        {
            m_CurrentItemCount = 0;
            ItemInfoCount = itemCount;
            ItemInfoList = new XmlItemInfo[ItemInfoCount];
        }

        /// <summary>
        /// Parses xml message data file
        /// </summary>
        /// <param name="file">the path to the file</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value indicating the status of the operation</returns>
        public PerfToolsReturnCode ParseFile(string file)
        {
            XmlDocument xmlDocument = new XmlDocument();
            try
            {
                xmlDocument.Load(file);
                XmlNodeList elementsByTagName = xmlDocument.GetElementsByTagName(SECTION_NAME);
                XmlNode? xmlNode;
                if (elementsByTagName.Count == 0 || (xmlNode = elementsByTagName.Item(0)) == null)
                {
                    Console.WriteLine($"Item file address: {file}. Item file contains no data...");
                    return PerfToolsReturnCode.FAILURE;
                }
                foreach (XmlNode childNode in xmlNode.ChildNodes)
                {
                    if (m_CurrentItemCount >= ItemInfoCount)
                    {
                        break;
                    }
                    Debug.Assert(childNode.Name.Equals(ITEM_NAME));
                    if (childNode.Attributes != null && childNode.Attributes.Count != 0)
                    {
                        XmlItemInfo xmlItemInfo = new XmlItemInfo();
                        ItemInfoList[m_CurrentItemCount++] = xmlItemInfo;
                        XmlAttribute? domainAttr = childNode.Attributes[DOMAIN_TAG];
                        xmlItemInfo.DomainType = domainAttr != null ? DomainTypeValue(domainAttr.Value) : 0;
                        XmlAttribute? nameAttr = childNode.Attributes[NAME_TAG];
                        xmlItemInfo.Name = nameAttr != null ? nameAttr.Value : string.Empty;
                        XmlAttribute? snapshotAttr = childNode.Attributes[SNAP_TAG];
                        xmlItemInfo.IsSnapshot = snapshotAttr != null && snapshotAttr.Value.ToLower().Equals("true");
                        XmlAttribute? genericAttr = childNode.Attributes[GEN_TAG];
                        xmlItemInfo.IsGenMsg = genericAttr != null && genericAttr.Value.ToLower().Equals("true");
                        if (xmlItemInfo.IsGenMsg)
                            ++GenMsgItemCount;
                        XmlAttribute? postAttr = childNode.Attributes[POST_TAG];
                        xmlItemInfo.IsPost = postAttr != null && postAttr.Value.ToLower().Equals("true");
                        if (xmlItemInfo.IsPost)
                            ++PostMsgItemCount;
                    }
                    else
                        break;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error parsing xml item list: {ex.Message}");
            }
            if (m_CurrentItemCount >= ItemInfoCount)
                return PerfToolsReturnCode.SUCCESS;
            Console.WriteLine($"Error: Item file contained {m_CurrentItemCount} items, but consumer wants {ItemInfoCount} items.");
            return PerfToolsReturnCode.FAILURE;
        }

        /// <summary>
        /// Converts domain type string to domain type value
        /// </summary>
        /// <param name="domainTypeString">the domain type string to be converted</param>
        /// <returns>the integer representing given domain type</returns>
        private int DomainTypeValue(string domainTypeString)
        {
            int type = 0;
            if (domainTypeString.Equals("MarketPrice"))
                type = (int) DomainType.MARKET_PRICE;
            else if (domainTypeString.Equals("MarketByOrder"))
                type = (int)DomainType.MARKET_BY_ORDER;
            else if (domainTypeString.Equals("MarketByPrice"))
                type = (int)DomainType.MARKET_BY_PRICE;
            return type;
        }
    }
}
