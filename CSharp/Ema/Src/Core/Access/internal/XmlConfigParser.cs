/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.Schema;

using LSEG.Eta.Transports;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Access
{
    // This class will, upon creation from
    // OmmConsumerConfigImpl/OmmProviderImpl/OmmNiProviderImpl, parse the either the
    // default Xml file, or the given file from the OmmConfig's XmlConfigPath.

    // When created from the copy constructor in OmmBaseImpl, this class will represent
    // the full config for the configured singular consumer that EMA will use to create an
    // OmmConsumer, OmmProvider, or OmmNiProvider.
    internal class XmlConfigParser
    {

        private const string DEFAULT_CONFIG_FILE = "EmaConfig.xml";
        // let unit-tests modify this value for testing purposes
        internal static string DEFAULT_SCHEMA_FILE = "EmaConfig.xsd";

        private XmlDocument ConfigXml { get; set; }

        // Reusable XmlNode references
        private XmlNode? CurrNode;
        private XmlElement? ValueNode;
        private XmlNodeList? CurrNodeList;
        private XmlNodeList? InnerNodeList;

        private XmlNode? CurrNode2;
        private XmlAttribute? XmlAttribute;

        private static HashSet<string> xmlNodeNames { get; } = new HashSet<string>
        {
            "EmaConfig",
            // Generic strings used across structures
            "Name",
            "Dictionary",
            "Channel",
            "Logger",
            // Consumer related strings
            "ConsumerGroup",
            "ConsumerList",
            "DefaultConsumer",
            "Consumer",
            "ChannelSet",
            "DictionaryRequestTimeOut",
            "DirectoryRequestTimeOut",
            "LoginRequestTimeOut",
            "DispatchTimeoutApiThread",
            "EnableRtt",
            "ItemCountHint",
            "MaxDispatchCountApiThread",
            "MaxDispatchCountUserThread",
            "MaxOutstandingPosts",
            "MsgKeyInUpdates",
            "ObeyOpenWindow",
            "PostAckTimeout",
            "ReconnectAttemptLimit",
            "ReconnectMaxDelay",
            "ReconnectMinDelay",
            "RequestTimeout",
            "RestEnableLog",
            "RestEnableLogViaCallback",
            "RestLogFileName",
            "RestRequestTimeOut",
            "ServiceCountHint",

            // Trace received/sent messages to file (write them in XML format)
            "XmlTraceToStdout",
            "XmlTraceToFile",
            "XmlTraceMaxFileSize",
            "XmlTraceFileName",
            "XmlTraceToMultipleFiles",
            "XmlTraceWrite",
            "XmlTraceRead",
            "XmlTracePing",

            // Provider related strings
            "MergeSourceDirectoryStreams",
            "RecoverUserSubmitSourceDirectory",
            "RefreshFirstRequired",
            "RemoveItemsOnDisconnect",
            "RestProxyHostName",
            "RestProxyPort",
            // Channel related strings
            "AuthenticationTimeout", // This is used only for the encrypted connection type.
            "ChannelGroup",
            "ChannelList",
            "ChannelType",
            "EncryptedProtocolType",
            "ConnectionPingTimeout",
            "EnableSessionManagement",
            "GuaranteedOutputBuffers",
            "HighWaterMark",
            "InitializationTimeout",
            "InterfaceName",
            "Location",
            "NumInputBuffers",
            "ServiceDiscoveryRetryCount",
            "SysRecvBufSize",
            "SysSendBufSize",
            "CompressionType",
            "CompressionThreshold",
            "DirectWrite",
            "Host",
            "Port",
            "ProxyHost",
            "ProxyPort",
            "TcpNodelay",
            "SecurityProtocol",
            // Logger related strings
            "LoggerGroup",
            "LoggerList",
            "FileName",
            "IncludeDateInLoggerOutput",
            "NumberOfLogFiles",
            "MaxLogFileSize",
            "LoggerSeverity",
            "LoggerType",
            // Dictionary related strings
            "DictionaryGroup",
            "DictionaryList",
            "EnumTypeDefFileName",
            "EnumTypeDefItemName",
            "RdmFieldDictionaryFileName",
            "RdmFieldDictionaryItemName",
            "DictionaryType",
            // NiProvider related strings
            "Directory",
            "Service",
            "NiProviderGroup",
            "DirectoryGroup",
            // IProvider related strings
            "IProviderGroup",
            "IProviderList",
            "IProvider",
            "ServerGroup",
            "ServerList",
            "Server",
            "AcceptDirMessageWithoutMinFilters",
            "AcceptMessageSameKeyButDiffStream",
            "AcceptMessageThatChangesService",
            "AcceptMessageWithoutAcceptingRequests",
            "AcceptMessageWithoutBeingLogin",
            "AcceptMessageWithoutQosInRange",
            "EnforceAckIDValidation",
            "EnumTypeFragmentSize",
            "FieldDictionaryFragmentSize",
            // Server related strings
            "ConnectionMinPingTimeout",
            "ServerType",
            "ServerCert",
            "ServerPrivateKey",
            "CipherSuite",
            "MaxFragmentSize",
            // Directory related strings
            "DirectoryList",
            "InfoFilter",
            "StateFilter",
            "LoadFilter",
            "CapabilitiesEntry",
            "Capabilities",
            "QoS",
            "QoSEntry",
            "Timeliness",
            "Rate",
            "Status",
            "StreamState",
            "DataState",
            "ServiceState",
            "StatusCode",
            "StatusText",
            "AcceptingRequests",
            "LoadFactor",
            "OpenWindow",
            "OpenLimit",
        };

        // Xml Configuration parser for an OmmConsumerConfig.
        // This method looks for the following configuration groups and parses them:
        // ConsumerGroup
        // ChannelGroup
        // LoggerGroup
        // DictionaryGroup
        // Error conditions from this method: configured file not found, Xml parsing errors.
        // NOTE: The ErrorLog will always be created in the Config object
        internal XmlConfigParser(OmmConsumerConfigImpl Config)
        {
            ConfigXml = LoadXmlConfig(Config.XmlConfigPath);

            XmlNode? ConfigNode = ConfigXml.DocumentElement;

            if (ConfigNode == null)
                return;

            XmlNode? GroupNode = ConfigNode.SelectSingleNode("ConsumerGroup");
            if (GroupNode != null)
            {
                // Parse the Consumer Group here
                ParseConsumerGroup(GroupNode, Config);
            }

            GroupNode = ConfigNode.SelectSingleNode("ChannelGroup");
            if (GroupNode != null)
            {
                // Parse the channel group here
                ParseClientChannelGroup(GroupNode, Config.ClientChannelConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("LoggerGroup");
            if (GroupNode != null)
            {
                // Parse the Logger group here
                ParseLoggerGroup(GroupNode, Config.LoggerConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("DictionaryGroup");
            if (GroupNode != null)
            {
                // Parse the Dictionary group here
                ParseDictionaryGroup(GroupNode, Config.DictionaryConfigMap, Config.ConfigErrorLog!);
            }

            // Now iterate through the entirety of the XML and find any elements that aren't available 
            foreach (XmlNode node in ConfigNode)
            {
                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                {
                    Config.ConfigErrorLog?.Add("Unknown XML element: " + node.Name, LoggerLevel.ERROR);
                }
            }
        }

        // Xml Configuration parser for an OmmNiProviderConfigImpl.
        // This method looks for the following configuration groups and parses them:
        // ChannelGroup
        // LoggerGroup
        // DictionaryGroup
        // DirectoryGroup
        // Error conditions from this method: configured file not found, Xml parsing errors.
        // NOTE: The ErrorLog will always be created in the Config object
        internal XmlConfigParser(OmmNiProviderConfigImpl Config)
        {
            ConfigXml = LoadXmlConfig(Config.XmlConfigPath);

            XmlNode? ConfigNode = ConfigXml.DocumentElement;

            if (ConfigNode == null)
                return;

            XmlNode? GroupNode = ConfigNode.SelectSingleNode("NiProviderGroup");
            if (GroupNode != null)
            {
                // Parse the Consumer Group here
                ParseNiProviderGroup(GroupNode, Config);
            }

            GroupNode = ConfigNode.SelectSingleNode("ChannelGroup");
            if (GroupNode != null)
            {
                // Parse the channel group here
                ParseClientChannelGroup(GroupNode, Config.ClientChannelConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("LoggerGroup");
            if (GroupNode != null)
            {
                // Parse the Logger group here
                ParseLoggerGroup(GroupNode, Config.LoggerConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("DictionaryGroup");
            if (GroupNode != null)
            {
                // Parse the Dictionary group here
                ParseDictionaryGroup(GroupNode, Config.DictionaryConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("DirectoryGroup");
            if (GroupNode != null)
            {
                string tmpDefaultName = string.Empty;
                string tmpFirstName = string.Empty;
                // Parse the Dictionary group here
                ParseSourceDirectoryGroup(GroupNode, Config.DirectoryConfigMap, true, Config.ConfigErrorLog!, out tmpDefaultName, out tmpFirstName);

                if (!string.IsNullOrEmpty(tmpDefaultName))
                {
                    Config.DefaultDirectory = tmpDefaultName;
                }

                if (!string.IsNullOrEmpty(tmpFirstName))
                {
                    Config.FirstConfiguredDirectory = tmpFirstName;
                }
            }

            // Now iterate through the entirety of the XML and find any elements that aren't available 
            foreach (XmlNode node in ConfigNode)
            {
                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                {
                    Config.ConfigErrorLog?.Add("Unknown XML element: " + node.Name, LoggerLevel.ERROR);
                }
            }
        }

        // Xml Configuration parser for an OmmNiProviderConfigImpl.
        // This method looks for the following configuration groups and parses them:
        // ChannelGroup
        // LoggerGroup
        // DictionaryGroup
        // DirectoryGroup
        // Error conditions from this method: configured file not found, Xml parsing errors.
        // NOTE: The ErrorLog will always be created in the Config object
        internal XmlConfigParser(OmmIProviderConfigImpl Config)
        {
            ConfigXml = LoadXmlConfig(Config.XmlConfigPath);

            XmlNode? ConfigNode = ConfigXml.DocumentElement;

            if (ConfigNode == null)
                return;

            XmlNode? GroupNode = ConfigNode.SelectSingleNode("IProviderGroup");
            if (GroupNode != null)
            {
                // Parse the Consumer Group here
                ParseIProviderGroup(GroupNode, Config);
            }

            GroupNode = ConfigNode.SelectSingleNode("ServerGroup");
            if (GroupNode != null)
            {
                // Parse the channel group here
                ParseServerGroup(GroupNode, Config.ServerConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("LoggerGroup");
            if (GroupNode != null)
            {
                // Parse the Logger group here
                ParseLoggerGroup(GroupNode, Config.LoggerConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("DictionaryGroup");
            if (GroupNode != null)
            {
                // Parse the Dictionary group here
                ParseDictionaryGroup(GroupNode, Config.DictionaryConfigMap, Config.ConfigErrorLog!);
            }

            GroupNode = ConfigNode.SelectSingleNode("DirectoryGroup");
            if (GroupNode != null)
            {
                string tmpDefaultName = string.Empty;
                string tmpFirstName = string.Empty;
                // Parse the Dictionary group here
                ParseSourceDirectoryGroup(GroupNode, Config.DirectoryConfigMap, false, Config.ConfigErrorLog!, out tmpDefaultName, out tmpFirstName);

                if (!string.IsNullOrEmpty(tmpDefaultName))
                {
                    Config.DefaultDirectory = tmpDefaultName;
                }

                if (!string.IsNullOrEmpty(tmpFirstName))
                {
                    Config.FirstConfiguredDirectory = tmpFirstName;
                }
            }

            // Now iterate through the entirety of the XML and find any elements that aren't available 
            foreach (XmlNode node in ConfigNode)
            {
                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                {
                    Config.ConfigErrorLog?.Add("Unknown XML element: " + node.Name, LoggerLevel.ERROR);
                }
            }
        }

        private void ParseConsumerGroup(XmlNode ConsumerNode, OmmConsumerConfigImpl Config)
        {

            CurrNode = ConsumerNode.SelectSingleNode("DefaultConsumer");
            if (CurrNode != null)
            {
                // Get the Value from the DefaultConsumer, if present
                ValueNode = (XmlElement)CurrNode;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException("Missing Default Consumer Value attribute");
                }

                Config.DefaultConsumer = XmlAttribute.Value;
            }

            CurrNode = ConsumerNode.SelectSingleNode("ConsumerList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing ConsumerList node");
            }

            CurrNodeList = CurrNode.SelectNodes("Consumer");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing Consumers in the ConsumerList");
            }

            foreach (XmlNode consumerListNode in CurrNodeList)
            {
                bool foundConfig = false;
                ConsumerConfig tmpConfig;
                if (CurrNode == null)
                    return;

                // Name string, this is required
                CurrNode2 = consumerListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Consumer Name element");
                    }

                    if (Config.ConsumerConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = Config.ConsumerConfigMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new ConsumerConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }

                    if (string.IsNullOrEmpty(Config.FirstConfiguredConsumerName))
                    {
                        Config.FirstConfiguredConsumerName = tmpConfig.Name;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Name element in the Consumer Name");
                }

                // Channel string
                CurrNode2 = consumerListNode.SelectSingleNode("Channel");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer Channel element");
                    }

                    tmpConfig.ChannelSet.Add(XmlAttribute.Value);
                }

                // ChannelSet string containing a comma separated list of Channel names
                CurrNode2 = consumerListNode.SelectSingleNode("ChannelSet");
                if (CurrNode2 != null)
                {
                    if (CurrNode2 != null)
                    {
                        ValueNode = (XmlElement)CurrNode2;
                        XmlAttribute = ValueNode.GetAttributeNode("value");

                        if (XmlAttribute == null)
                        {
                            throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer ChannelSet element");
                        }

                        tmpConfig.ChannelSet.Clear();

                        string[] channelArray = XmlAttribute.Value.Split(',');

                       foreach(string channelName in channelArray)
                            tmpConfig.ChannelSet.Add(channelName.Trim());
                    }
                }

                // Dictionary string
                CurrNode2 = consumerListNode.SelectSingleNode("Dictionary");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer Dictionary element");
                    }

                    tmpConfig.Dictionary = XmlAttribute.Value;
                }

                // DictionaryRequestTimeOut ulong
                CurrNode2 = consumerListNode.SelectSingleNode("DictionaryRequestTimeOut");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer DictionaryRequestTimeOut element");
                    }
                    try
                    {
                        tmpConfig.DictionaryRequestTimeOut = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer DictionaryRequestTimeOut element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }

                }

                // DirectoryRequestTimeOut ulong
                CurrNode2 = consumerListNode.SelectSingleNode("DirectoryRequestTimeOut");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer DirectoryRequestTimeOut element");
                    }

                    try
                    {
                        tmpConfig.DirectoryRequestTimeOut = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer DirectoryRequestTimeOut element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // LoginRequestTimeOut ulong
                CurrNode2 = consumerListNode.SelectSingleNode("LoginRequestTimeOut");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer LoginRequestTimeOut element");
                    }

                    try
                    {
                        tmpConfig.LoginRequestTimeOut = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer LoginRequestTimeOut element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // DispatchTimeoutApiThread long
                CurrNode2 = consumerListNode.SelectSingleNode("DispatchTimeoutApiThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the DispatchTimeoutApiThread element");
                    }

                    try
                    {
                        tmpConfig.DispatchTimeoutApiThread = (long.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer DispatchTimeoutApiThread element is incorrectly formatted. Correct format is a signed numeric string.");
                    }
                }

                // EnableRtt bool
                CurrNode2 = consumerListNode.SelectSingleNode("EnableRtt");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer EnableRtt element");
                    }

                    try
                    {
                        tmpConfig.EnableRtt = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer DispatchTimeoutApiThread element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // ItemCountHint ulong
                CurrNode2 = consumerListNode.SelectSingleNode("ItemCountHint");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer ItemCountHint element");
                    }

                    try
                    {
                        tmpConfig.ItemCountHint = uint.Parse(XmlAttribute.Value);

                        if(tmpConfig.ItemCountHint == 0)
                        {
                            tmpConfig.ItemCountHint = 1024;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer DispatchTimeoutApiThread element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // Logger string
                CurrNode2 = consumerListNode.SelectSingleNode("Logger");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer Logger element");
                    }

                    tmpConfig.Logger = XmlAttribute.Value;
                }

                // MaxDispatchCountApiThread ulong
                CurrNode2 = consumerListNode.SelectSingleNode("MaxDispatchCountApiThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer MaxDispatchCountApiThread element");
                    }

                    try
                    {
                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer MaxDispatchCountApiThread element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MaxDispatchCountUserThread uint
                CurrNode2 = consumerListNode.SelectSingleNode("MaxDispatchCountUserThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer MaxDispatchCountUserThread element");
                    }

                    try
                    {
                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));


                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer MaxDispatchCountUserThread element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MaxOutstandingPosts uint
                CurrNode2 = consumerListNode.SelectSingleNode("MaxOutstandingPosts");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer MaxOutstandingPosts element");
                    }

                    try
                    {
                        tmpConfig.MaxOutstandingPosts = Utilities.Convert_ulong_uint(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer MaxOutstandingPosts element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MsgKeyInUpdates bool
                CurrNode2 = consumerListNode.SelectSingleNode("MsgKeyInUpdates");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer MsgKeyInUpdates element");
                    }

                    try
                    {
                        tmpConfig.MsgKeyInUpdates = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer MsgKeyInUpdates element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // ObeyOpenWindow ulong->bool
                CurrNode2 = consumerListNode.SelectSingleNode("ObeyOpenWindow");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer ObeyOpenWindow element");
                    }

                    try
                    {
                        tmpConfig.ObeyOpenWindow = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ObeyOpenWindow element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // PostAckTimeout uint
                CurrNode2 = consumerListNode.SelectSingleNode("PostAckTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer PostAckTimeout element");
                    }

                    try
                    {
                        tmpConfig.PostAckTimeout = Utilities.Convert_ulong_uint(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer PostAckTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ReconnectAttemptLimit int
                CurrNode2 = consumerListNode.SelectSingleNode("ReconnectAttemptLimit");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer PostAckTimeout element");
                    }

                    try
                    {
                        tmpConfig.ReconnectAttemptLimit = int.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ReconnectAttemptLimit element is incorrectly formatted. Correct format is a signed numeric string.");
                    }
                }

                // ReconnectMaxDelay int
                CurrNode2 = consumerListNode.SelectSingleNode("ReconnectMaxDelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer ReconnectMaxDelay element");
                    }

                    try
                    {
                        int value = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if (value > 0)
                        {
                            tmpConfig.ReconnectMaxDelay = value;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ReconnectMaxDelay element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ReconnectMinDelay int
                CurrNode2 = consumerListNode.SelectSingleNode("ReconnectMinDelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer ReconnectMinDelay element");
                    }

                    try
                    {
                        int value = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if (value > 0)
                        {
                            tmpConfig.ReconnectMinDelay = value;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ReconnectMinDelay element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // RequestTimeout uint
                CurrNode2 = consumerListNode.SelectSingleNode("RequestTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer RequestTimeout element");
                    }

                    try
                    {
                        tmpConfig.RequestTimeout = Utilities.Convert_ulong_uint(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer RequestTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // RestEnableLog bool
                CurrNode2 = consumerListNode.SelectSingleNode("RestEnableLog");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer RestEnableLog element");
                    }

                    try
                    {
                        tmpConfig.RestEnableLog = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer RestEnableLog element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // RestEnableLogViaCallback ulong->bool
                CurrNode2 = consumerListNode.SelectSingleNode("RestEnableLogViaCallback");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer RestEnableLogViaCallback element");
                    }

                    try
                    {
                        tmpConfig.RestEnableLogViaCallback = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer RestEnableLogViaCallback element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // RestLogFileName string
                CurrNode2 = consumerListNode.SelectSingleNode("RestLogFileName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer RestLogFileName element");
                    }
                    tmpConfig.RestLogFileName = XmlAttribute.Value;
                }

                // RestRequestTimeOut ulong
                CurrNode2 = consumerListNode.SelectSingleNode("RestRequestTimeOut");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer RestRequestTimeOut element");
                    }

                    try
                    {
                        tmpConfig.RestRequestTimeOut = ulong.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer RestRequestTimeOut element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ServiceCountHint uint
                CurrNode2 = consumerListNode.SelectSingleNode("ServiceCountHint");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer ServiceCountHint element");
                    }

                    try
                    {
                        tmpConfig.ServiceCountHint = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if(tmpConfig.ServiceCountHint == 0)
                        {
                            tmpConfig.ServiceCountHint = 513;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ServiceCountHint element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                ParseXmlTraceConfigNodes("Consumer", tmpConfig, consumerListNode);

                if (foundConfig == false)
                    Config.ConsumerConfigMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a Consumer group
                foreach (XmlNode node in consumerListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        Config.ConfigErrorLog?.Add($"Unknown Consumer entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseNiProviderGroup(XmlNode ConsumerNode, OmmNiProviderConfigImpl Config)
        {

            CurrNode = ConsumerNode.SelectSingleNode("DefaultNiProvider");
            if (CurrNode != null)
            {
                // Get the Value from the DefaultConsumer, if present
                ValueNode = (XmlElement)CurrNode;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException("Missing Default NiProvider Value attribute");
                }

                Config.DefaultNiProvider = XmlAttribute.Value;
            }

            CurrNode = ConsumerNode.SelectSingleNode("NiProviderList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing NiProviderList node");
            }

            CurrNodeList = CurrNode.SelectNodes("NiProvider");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing NiProviders in the NiProviderList");
            }

            foreach (XmlNode niProviderListNode in CurrNodeList)
            {
                bool foundConfig = false;
                NiProviderConfig tmpConfig;
                if (CurrNode == null)
                    return;

                // Name string, this is required
                CurrNode2 = niProviderListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider Name element");
                    }

                    if (Config.NiProviderConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = Config.NiProviderConfigMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new NiProviderConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }

                    if (string.IsNullOrEmpty(Config.FirstConfiguredNiProviderName))
                    {
                        Config.FirstConfiguredNiProviderName = tmpConfig.Name;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Name element in the NiProvider Name");
                }

                // Channel string
                CurrNode2 = niProviderListNode.SelectSingleNode("Channel");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider Channel element");
                    }

                    tmpConfig.ChannelSet.Add(XmlAttribute.Value);
                }

                // ChannelSet string containing a comma separated list of Channel names
                CurrNode2 = niProviderListNode.SelectSingleNode("ChannelSet");
                if (CurrNode2 != null)
                {
                    if (CurrNode2 != null)
                    {
                        ValueNode = (XmlElement)CurrNode2;
                        XmlAttribute = ValueNode.GetAttributeNode("value");

                        if (XmlAttribute == null)
                        {
                            throw new OmmInvalidConfigurationException(
                                "Missing value attribute in the NiProvider ChannelSet element");
                        }

                        tmpConfig.ChannelSet.Clear();

                        string[] channelArray = XmlAttribute.Value.Split(',');

                        foreach (string channelName in channelArray)
                            tmpConfig.ChannelSet.Add(channelName.Trim());
                    }
                }

                // Directory string
                CurrNode2 = niProviderListNode.SelectSingleNode("Directory");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider Directory element");
                    }

                    tmpConfig.Directory = XmlAttribute.Value;
                }

                // DispatchTimeoutApiThread long
                CurrNode2 = niProviderListNode.SelectSingleNode("DispatchTimeoutApiThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider DispatchTimeoutApiThread element");
                    }

                    try
                    {
                        tmpConfig.DispatchTimeoutApiThread = (long.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider DispatchTimeoutApiThread element is incorrectly formatted. Correct format is a signed numeric string.");
                    }
                }

                // ItemCountHint ulong
                CurrNode2 = niProviderListNode.SelectSingleNode("ItemCountHint");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider ItemCountHint element");
                    }

                    try
                    {
                        tmpConfig.ItemCountHint = Utilities.Convert_ulong_uint(ulong.Parse(XmlAttribute.Value));

                        if(tmpConfig.ItemCountHint == 0)
                        {
                            tmpConfig.ItemCountHint = 1024;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider ItemCountHint element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // Logger string
                CurrNode2 = niProviderListNode.SelectSingleNode("Logger");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider Logger element");
                    }

                    tmpConfig.Logger = XmlAttribute.Value;
                }

                // LoginRequestTimeOut ulong
                CurrNode2 = niProviderListNode.SelectSingleNode("LoginRequestTimeOut");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider LoginRequestTimeOut element");
                    }

                    try
                    {
                        tmpConfig.LoginRequestTimeOut = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider LoginRequestTimeOut element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MaxDispatchCountApiThread ulong
                CurrNode2 = niProviderListNode.SelectSingleNode("MaxDispatchCountApiThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider MaxDispatchCountApiThread element");
                    }

                    try
                    {
                        tmpConfig.MaxDispatchCountApiThread =
                            Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider MaxDispatchCountApiThread element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MaxDispatchCountUserThread uint
                CurrNode2 = niProviderListNode.SelectSingleNode("MaxDispatchCountUserThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider MaxDispatchCountUserThread element");
                    }

                    try
                    {
                        tmpConfig.MaxDispatchCountUserThread =
                            Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider MaxDispatchCountUserThread element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MergeSourceDirectoryStreams ulong->bool
                CurrNode2 = niProviderListNode.SelectSingleNode("MergeSourceDirectoryStreams");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider MergeSourceDirectoryStreams element");
                    }

                    try
                    {
                        tmpConfig.MergeSourceDirectoryStreams = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider MergeSourceDirectoryStreams element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // ReconnectAttemptLimit int
                CurrNode2 = niProviderListNode.SelectSingleNode("ReconnectAttemptLimit");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider PostAckTimeout element");
                    }

                    try
                    {
                        tmpConfig.ReconnectAttemptLimit = Utilities.Convert_long_int(long.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider ReconnectAttemptLimit element is incorrectly formatted. Correct format is a signed numeric string.");
                    }
                }

                // ReconnectMaxDelay int
                CurrNode2 = niProviderListNode.SelectSingleNode("ReconnectMaxDelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider ReconnectMaxDelay element");
                    }

                    try
                    {
                        int value = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if (value > 0)
                        {
                            tmpConfig.ReconnectMaxDelay = value;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider ReconnectMaxDelay element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ReconnectMinDelay int
                CurrNode2 = niProviderListNode.SelectSingleNode("ReconnectMinDelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider ReconnectMinDelay element");
                    }

                    try
                    {
                        int value = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if (value > 0)
                        {
                            tmpConfig.ReconnectMinDelay = value;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider ReconnectMinDelay element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // RecoverUserSubmitSourceDirectory bool
                CurrNode2 = niProviderListNode.SelectSingleNode("RecoverUserSubmitSourceDirectory");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider RecoverUserSubmitSourceDirectory element");
                    }

                    try
                    {
                        tmpConfig.RecoverUserSubmitSourceDirectory = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider RecoverUserSubmitSourceDirectory element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // RefreshFirstRequired bool
                CurrNode2 = niProviderListNode.SelectSingleNode("RefreshFirstRequired");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider RefreshFirstRequired element");
                    }

                    try
                    {
                        tmpConfig.RefreshFirstRequired = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider RefreshFirstRequired element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // RemoveItemsOnDisconnect bool
                CurrNode2 = niProviderListNode.SelectSingleNode("RemoveItemsOnDisconnect");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider RemoveItemsOnDisconnect element");
                    }

                    try
                    {
                        tmpConfig.RemoveItemsOnDisconnect = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider RemoveItemsOnDisconnect element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // RequestTimeout uint
                CurrNode2 = niProviderListNode.SelectSingleNode("RequestTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider RequestTimeout element");
                    }

                    try
                    {
                        tmpConfig.RequestTimeout = Utilities.Convert_ulong_uint(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider RequestTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ServiceCountHint uint
                CurrNode2 = niProviderListNode.SelectSingleNode("ServiceCountHint");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider ServiceCountHint element");
                    }

                    try
                    {
                        tmpConfig.ServiceCountHint = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if(tmpConfig.ServiceCountHint == 0)
                        {
                            tmpConfig.ServiceCountHint = 513;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the NiProvider ServiceCountHint element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                ParseXmlTraceConfigNodes("NiProvider", tmpConfig, niProviderListNode);

                if (foundConfig == false)
                    Config.NiProviderConfigMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a NiProvider group
                foreach (XmlNode node in niProviderListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        Config.ConfigErrorLog?.Add($"Unknown NiProvider entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseIProviderGroup(XmlNode ConsumerNode, OmmIProviderConfigImpl Config)
        {

            CurrNode = ConsumerNode.SelectSingleNode("DefaultIProvider");
            if (CurrNode != null)
            {
                // Get the Value from the DefaultConsumer, if present
                ValueNode = (XmlElement)CurrNode;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException("Missing Default IProvider Value attribute");
                }

                Config.DefaultIProvider = XmlAttribute.Value;
            }

            CurrNode = ConsumerNode.SelectSingleNode("IProviderList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing IProviderList node");
            }

            CurrNodeList = CurrNode.SelectNodes("IProvider");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing IProviders in the IProviderList");
            }

            foreach (XmlNode iProviderListNode in CurrNodeList)
            {
                bool foundConfig = false;
                IProviderConfig tmpConfig;
                if (CurrNode == null)
                    return;

                // Name string, this is required
                CurrNode2 = iProviderListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider Name element");
                    }

                    if (Config.IProviderConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = Config.IProviderConfigMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new IProviderConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }

                    if (string.IsNullOrEmpty(Config.FirstConfiguredIProvider))
                    {
                        Config.FirstConfiguredIProvider = tmpConfig.Name;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Name element in the IProvider Name");
                }

                // Directory string
                CurrNode2 = iProviderListNode.SelectSingleNode("Directory");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider Directory element");
                    }

                    tmpConfig.Directory = XmlAttribute.Value;
                }

                // DispatchTimeoutApiThread long
                CurrNode2 = iProviderListNode.SelectSingleNode("DispatchTimeoutApiThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider DispatchTimeoutApiThread element");
                    }

                    try
                    {
                        tmpConfig.DispatchTimeoutApiThread = long.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider DispatchTimeoutApiThread element is incorrectly formatted. Correct value is a signed numeric string.");
                    }
                }

                // ItemCountHint ulong
                CurrNode2 = iProviderListNode.SelectSingleNode("ItemCountHint");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider ItemCountHint element");
                    }

                    try
                    {
                        tmpConfig.ItemCountHint = ulong.Parse(XmlAttribute.Value);

                        if(tmpConfig.ItemCountHint == 0)
                        {
                            tmpConfig.ItemCountHint = 1024;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider ItemCountHint element is incorrectly formatted. Correct value is a unsigned numeric string.");
                    }
                }

                // Logger string
                CurrNode2 = iProviderListNode.SelectSingleNode("Logger");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider Logger element");
                    }

                    tmpConfig.Logger = XmlAttribute.Value;
                }

                // MaxDispatchCountApiThread int
                CurrNode2 = iProviderListNode.SelectSingleNode("MaxDispatchCountApiThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider MaxDispatchCountApiThread element");
                    }

                    try
                    {
                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider MaxDispatchCountApiThread element is incorrectly formatted. Correct value is a signed numeric string.");
                    }
                }

                // MaxDispatchCountUserThread uint
                CurrNode2 = iProviderListNode.SelectSingleNode("MaxDispatchCountUserThread");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider MaxDispatchCountUserThread element");
                    }

                    try
                    {
                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider MaxDispatchCountUserThread element is incorrectly formatted. Correct value is a signed numeric string.");
                    }
                }

                // RefreshFirstRequired ulong
                CurrNode2 = iProviderListNode.SelectSingleNode("RefreshFirstRequired");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider RefreshFirstRequired element");
                    }

                    try
                    {
                        tmpConfig.RefreshFirstRequired = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider RefreshFirstRequired element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // RequestTimeout ulong
                CurrNode2 = iProviderListNode.SelectSingleNode("RequestTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider RequestTimeout element");
                    }

                    try
                    {
                        tmpConfig.RequestTimeout = ulong.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider RequestTimeout element is incorrectly formatted. Correct value is a unsigned numeric string.");
                    }
                }

                // ServiceCountHint int
                CurrNode2 = iProviderListNode.SelectSingleNode("ServiceCountHint");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider ServiceCountHint element");
                    }

                    try
                    {
                        tmpConfig.ServiceCountHint = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                        if(tmpConfig.ServiceCountHint == 0)
                        {
                            tmpConfig.ServiceCountHint = 513;
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider ServiceCountHint element is incorrectly formatted. Correct value is a signed numeric string.");
                    }
                }

                // Server string
                CurrNode2 = iProviderListNode.SelectSingleNode("Server");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider Server element");
                    }

                    tmpConfig.Server = XmlAttribute.Value;
                }

                // AcceptDirMessageWithoutMinFilters ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("AcceptDirMessageWithoutMinFilters");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider AcceptDirMessageWithoutMinFilters element");
                    }

                    try
                    {
                        tmpConfig.AcceptDirMessageWithoutMinFilters = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider AcceptDirMessageWithoutMinFilters element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // AcceptMessageSameKeyButDiffStream ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("AcceptMessageSameKeyButDiffStream");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider AcceptMessageSameKeyButDiffStream element");
                    }

                    try
                    {
                        tmpConfig.AcceptMessageSameKeyButDiffStream = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider AcceptMessageSameKeyButDiffStream element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // AcceptMessageThatChangesService ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("AcceptMessageThatChangesService");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider AcceptMessageThatChangesService element");
                    }

                    try
                    {
                        tmpConfig.AcceptMessageThatChangesService = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider AcceptMessageThatChangesService element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // AcceptMessageWithoutAcceptingRequests ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("AcceptMessageWithoutAcceptingRequests");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider AcceptMessageWithoutAcceptingRequests element");
                    }

                    try
                    {
                        tmpConfig.AcceptMessageWithoutAcceptingRequests = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider AcceptMessageWithoutAcceptingRequests element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // AcceptMessageWithoutBeingLogin ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("AcceptMessageWithoutBeingLogin");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider AcceptMessageWithoutBeingLogin element");
                    }

                    try
                    {
                        tmpConfig.AcceptMessageWithoutBeingLogin = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider AcceptMessageWithoutBeingLogin element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // AcceptMessageWithoutQosInRange ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("AcceptMessageWithoutQosInRange");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider AcceptMessageWithoutQosInRange element");
                    }

                    try
                    {
                        tmpConfig.AcceptMessageWithoutQosInRange = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider AcceptMessageWithoutQosInRange element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // EnforceAckIDValidation ulong->bool
                CurrNode2 = iProviderListNode.SelectSingleNode("EnforceAckIDValidation");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider EnforceAckIDValidation element");
                    }

                    try
                    {
                        tmpConfig.EnforceAckIDValidation = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider EnforceAckIDValidation element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // EnumTypeFragmentSize uint
                CurrNode2 = iProviderListNode.SelectSingleNode("EnumTypeFragmentSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider EnumTypeFragmentSize element");
                    }

                    try
                    {
                        tmpConfig.EnumTypeFragmentSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider EnumTypeFragmentSize element is incorrectly formatted. Correct value is a unsigned numeric string.");
                    }
                }

                // FieldDictionaryFragmentSize uint
                CurrNode2 = iProviderListNode.SelectSingleNode("FieldDictionaryFragmentSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the IProvider FieldDictionaryFragmentSize element");
                    }

                    try
                    {
                        tmpConfig.FieldDictionaryFragmentSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException(
                            "The value attribute in the IProvider FieldDictionaryFragmentSize element is incorrectly formatted. Correct value is a unsigned numeric string.");
                    }
                }

                ParseXmlTraceConfigNodes("IProvider", tmpConfig, iProviderListNode);

                if (foundConfig == false)
                    Config.IProviderConfigMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a IProvider group
                foreach (XmlNode node in iProviderListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        Config.ConfigErrorLog?.Add($"Unknown IProvider entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseClientChannelGroup(XmlNode ChannelNode, Dictionary<string, ClientChannelConfig> configMap,
            ConfigErrorList configError)
        {
            CurrNode = ChannelNode.SelectSingleNode("ChannelList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing ChannelList element.");
            }

            CurrNodeList = CurrNode.SelectNodes("Channel");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing Channel element(s).");
            }

            foreach (XmlNode channelListNode in CurrNodeList)
            {
                bool foundConfig = false;
                ClientChannelConfig tmpConfig;

                // Name string, this is required
                CurrNode2 = channelListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Channel Name element");
                    }

                    if (configMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = configMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new ClientChannelConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Channel Name element");
                }

                // ChannelType string: This will remove the "ChannelType::" prepend and call StringToConnectionType
                CurrNode2 = channelListNode.SelectSingleNode("ChannelType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel ConnectionType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid ConnectionType string format. Correct format is \"ChannelType::<RSSL_SOCKET or RSSL_ENCRYPTED>\".");
                    }

                    if (channelArray[0] != "ChannelType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid ConnectionType string format. Correct format is \"ChannelType::<RSSL_SOCKET or RSSL_ENCRYPTED>\".");
                    }

                    tmpConfig.ConnectInfo.ConnectOptions.ConnectionType = ClientChannelConfig.StringToConnectionType(channelArray[1]);
                }

                // EncryptedProtocolType string: This will remove the "EncryptedProtocolType::" prepend and call StringToConnectionType
                CurrNode2 = channelListNode.SelectSingleNode("EncryptedProtocolType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel EncryptedProtocolType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid EncryptedProtocolType string format. Correct format is \"EncryptedProtocolType::<RSSL_SOCKET>\".");
                    }

                    if (channelArray[0] != "EncryptedProtocolType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid EncryptedProtocolType string format. Correct format is \"EncryptedProtocolType::<RSSL_SOCKET>\".");
                    }

                    tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol = ClientChannelConfig.StringToConnectionType(channelArray[1]);
                }

                // ConnectionPingTimeout int
                CurrNode2 = channelListNode.SelectSingleNode("ConnectionPingTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel ConnectionPingTimeout element");
                    }

                    try
                    {
                        int pingTimeout = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
                        tmpConfig.ConnectInfo.ConnectOptions.PingTimeout = pingTimeout >= 1000 ? pingTimeout / 1000 : 60;
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel ConnectionPingTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // EnableSessionManagement ulong
                CurrNode2 = channelListNode.SelectSingleNode("EnableSessionManagement");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel EnableSessionManagement element");
                    }


                    try
                    {
                        tmpConfig.ConnectInfo.EnableSessionManagement = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel EnableSessionManagement element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // GuaranteedOutputBuffers uint
                CurrNode2 = channelListNode.SelectSingleNode("GuaranteedOutputBuffers");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel GuaranteedOutputBuffers element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel GuaranteedOutputBuffers element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // HighWaterMark uint
                CurrNode2 = channelListNode.SelectSingleNode("HighWaterMark");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel HighWaterMark element");
                    }

                    try
                    {
                        tmpConfig.HighWaterMark = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel HighWaterMark element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // InitializationTimeout uint, parsed as int because of the ConnectInfo method
                CurrNode2 = channelListNode.SelectSingleNode("InitializationTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel InitializationTimeout element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.SetInitTimeout(Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value)));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel InitializationTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // InterfaceName string
                CurrNode2 = channelListNode.SelectSingleNode("InterfaceName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel InterfaceName element");
                    }
                    tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName = XmlAttribute.Value;
                }

                // Location string
                CurrNode2 = channelListNode.SelectSingleNode("Location");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel Location element");
                    }
                    tmpConfig.ConnectInfo.Location = XmlAttribute.Value;
                }

                // NumInputBuffers uint, parsed as int.
                CurrNode2 = channelListNode.SelectSingleNode("NumInputBuffers");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel NumInputBuffers element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.ConnectOptions.NumInputBuffers = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel NumInputBuffers element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ServiceDiscoveryRetryCount uint
                CurrNode2 = channelListNode.SelectSingleNode("ServiceDiscoveryRetryCount");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel ServiceDiscoveryRetryCount element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.ServiceDiscoveryRetryCount = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel ServiceDiscoveryRetryCount element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // SysRecvBufSize uint parsed as int
                CurrNode2 = channelListNode.SelectSingleNode("SysRecvBufSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel SysRecvBufSize element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.ConnectOptions.SysRecvBufSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel SysRecvBufSize element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // SysSendBufSize uint parsed as int
                CurrNode2 = channelListNode.SelectSingleNode("SysSendBufSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel SysSendBufSize element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.ConnectOptions.SysSendBufSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel SysSendBufSize element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // CompressionType enumeration
                CurrNode2 = channelListNode.SelectSingleNode("CompressionType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel CompressionType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid CompressionType string format. Correct format is \"CompressionType::<None/ZLib/LZ4>\".");

                    }

                    if (channelArray[0] != "CompressionType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid CompressionType string format. Correct format is \"CompressionType::<None/ZLib/LZ4>\".");

                    }

                    tmpConfig.ConnectInfo.ConnectOptions.CompressionType = ClientChannelConfig.StringToCompressionType(channelArray[1]);

                }

                // CompressionThreshold
                CurrNode2 = channelListNode.SelectSingleNode("CompressionThreshold");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel CompressionThreshold element");
                    }

                    try
                    {
                        tmpConfig.CompressionThreshold = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                        tmpConfig.CompressionThresholdSet = true;
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel CompressionThreshold element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // DirectWrite uint->bool
                CurrNode2 = channelListNode.SelectSingleNode("DirectWrite");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel DirectWrite element");
                    }

                    try
                    {
                        tmpConfig.DirectWrite = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel DirectWrite element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // Host string
                CurrNode2 = channelListNode.SelectSingleNode("Host");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel Host element");
                    }
                    tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = XmlAttribute.Value;
                }

                // Port string
                CurrNode2 = channelListNode.SelectSingleNode("Port");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel Port element");
                    }
                    tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = XmlAttribute.Value;
                }

                // ProxyHost string
                CurrNode2 = channelListNode.SelectSingleNode("ProxyHost");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel ProxyHost element");
                    }
                    tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = XmlAttribute.Value;
                }

                // ProxyPort string
                CurrNode2 = channelListNode.SelectSingleNode("ProxyPort");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel ProxyPort element");
                    }
                    tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = XmlAttribute.Value;
                }

                // TcpNodelay uint->bool
                CurrNode2 = channelListNode.SelectSingleNode("TcpNodelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel TcpNodelay element");
                    }

                    try
                    {
                        tmpConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay = (uint.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel TcpNodelay element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // SecurityProtocol enum
                CurrNode2 = channelListNode.SelectSingleNode("SecurityProtocol");
                if (CurrNode2 != null)
                {
                    uint tmpInt;

                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel SecurityProtocol element");
                    }
                    try
                    {
                        tmpInt = uint.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel SecurityProtocol element is incorrectly formatted. Correct values are combinations of flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags");
                    }

                    if (tmpInt != 0 && (tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid value for Channel element SecurityProtocol. This must be an int type, with the flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags.");
                    }

                    tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags = (EncryptionProtocolFlags)(tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL);
                }

                // AuthenticationTimeout uint
                CurrNode2 = channelListNode.SelectSingleNode("AuthenticationTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel AuthenticationTimeout element");
                    }

                    try
                    {
                        uint temp = uint.Parse(XmlAttribute.Value);

                        if (temp > 0)
                        {
                            tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout = Utilities.Convert_uint_int(temp);
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel AuthenticationTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a Channel group
                foreach (XmlNode node in channelListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        configError.Add($"Unknown Channel entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseServerGroup(XmlNode ChannelNode, Dictionary<string, ServerConfig> configMap,
            ConfigErrorList configError)
        {
            CurrNode = ChannelNode.SelectSingleNode("ServerList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing ServerList element.");
            }

            CurrNodeList = CurrNode.SelectNodes("Server");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing Server element(s).");
            }

            foreach (XmlNode serverListNode in CurrNodeList)
            {
                bool foundConfig = false;
                ServerConfig tmpConfig;

                // Name string, this is required
                CurrNode2 = serverListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Server Name element");
                    }

                    if (configMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = configMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new ServerConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Server Name element");
                }

                // ConnectionMinPingTimeout int
                CurrNode2 = serverListNode.SelectSingleNode("ConnectionMinPingTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server ConnectionMinPingTimeout element");
                    }

                    try
                    {
                        int pingTimeout = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
                        tmpConfig.BindOptions.MinPingTimeout = pingTimeout >= 1000 ? pingTimeout / 1000 : 60;
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server ConnectionMinPingTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ConnectionPingTimeout int
                CurrNode2 = serverListNode.SelectSingleNode("ConnectionPingTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server ConnectionPingTimeout element");
                    }

                    try
                    {
                        int pingTimeout = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
                        tmpConfig.BindOptions.PingTimeout = pingTimeout >= 1000 ? pingTimeout / 1000 : 60;
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server ConnectionPingTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // CompressionThreshold uint
                CurrNode2 = serverListNode.SelectSingleNode("CompressionThreshold");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server CompressionThreshold element");
                    }

                    try
                    {
                        tmpConfig.CompressionThreshold = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                        tmpConfig.CompressionThresholdSet = true;
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server CompressionThreshold element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // CompressionType enumeration
                CurrNode2 = serverListNode.SelectSingleNode("CompressionType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server CompressionType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid Server CompressionType string format. Correct format is \"CompressionType::<None/ZLib/LZ4>\".");

                    }

                    if (channelArray[0] != "CompressionType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid Server CompressionType string format. Correct format is \"CompressionType::<None/ZLib/LZ4>\".");

                    }

                    tmpConfig.BindOptions.CompressionType = ClientChannelConfig.StringToCompressionType(channelArray[1]);

                }

                // DirectWrite uint->bool
                CurrNode2 = serverListNode.SelectSingleNode("DirectWrite");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server DirectWrite element");
                    }

                    try
                    {
                        tmpConfig.DirectWrite = (uint.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server DirectWrite element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // GuaranteedOutputBuffers uint
                CurrNode2 = serverListNode.SelectSingleNode("GuaranteedOutputBuffers");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server GuaranteedOutputBuffers element");
                    }

                    try
                    {
                        tmpConfig.BindOptions.GuaranteedOutputBuffers = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server GuaranteedOutputBuffers element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // HighWaterMark uint
                CurrNode2 = serverListNode.SelectSingleNode("HighWaterMark");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server HighWaterMark element");
                    }

                    try
                    {
                        tmpConfig.HighWaterMark = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server HighWaterMark element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // InitializationTimeout uint
                CurrNode2 = serverListNode.SelectSingleNode("InitializationTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server InitializationTimeout element");
                    }

                    try
                    {
                        tmpConfig.InitializationTimeout = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server InitializationTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // AuthenticationTimeout uint
                CurrNode2 = serverListNode.SelectSingleNode("AuthenticationTimeout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server AuthenticationTimeout element");
                    }

                    try
                    {
                        uint temp = uint.Parse(XmlAttribute.Value);

                        if (temp > 0)
                        {
                            tmpConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout = Utilities.Convert_uint_int(temp);
                        }
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server AuthenticationTimeout element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // InterfaceName string
                CurrNode2 = serverListNode.SelectSingleNode("InterfaceName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server InterfaceName element");
                    }
                    tmpConfig.BindOptions.InterfaceName = XmlAttribute.Value;
                }

                // MaxFragmentSize uint
                CurrNode2 = serverListNode.SelectSingleNode("MaxFragmentSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server MaxFragmentSize element");
                    }

                    try
                    {
                        tmpConfig.BindOptions.MaxFragmentSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server MaxFragmentSize element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // NumInputBuffers uint
                CurrNode2 = serverListNode.SelectSingleNode("NumInputBuffers");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server NumInputBuffers element");
                    }

                    try
                    {
                        tmpConfig.BindOptions.NumInputBuffers = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server NumInputBuffers element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // Port string
                CurrNode2 = serverListNode.SelectSingleNode("Port");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server Port element");
                    }
                    tmpConfig.BindOptions.ServiceName = XmlAttribute.Value;
                }

                // ServerType string: This will remove the "ServerType::" prepend and call StringToConnectionType
                CurrNode2 = serverListNode.SelectSingleNode("ServerType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel ServerType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid ServerType string format. Correct format is \"ServerType::<RSSL_SOCKET or RSSL_ENCRYPTED>\".");
                    }

                    if (channelArray[0] != "ServerType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid ServerType string format. Correct format is \"ChannelType::<RSSL_SOCKET or RSSL_ENCRYPTED>\".");
                    }

                    tmpConfig.BindOptions.ConnectionType = ClientChannelConfig.StringToConnectionType(channelArray[1]);
                }

                // SysRecvBufSize uint
                CurrNode2 = serverListNode.SelectSingleNode("SysRecvBufSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server SysRecvBufSize element");
                    }

                    try
                    {
                        tmpConfig.BindOptions.SysRecvBufSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server SysRecvBufSize element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // SysSendBufSize uint
                CurrNode2 = serverListNode.SelectSingleNode("SysSendBufSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server SysSendBufSize element");
                    }

                    try
                    {
                        tmpConfig.BindOptions.SysSendBufSize = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server SysSendBufSize element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // TcpNodelay uint->bool
                CurrNode2 = serverListNode.SelectSingleNode("TcpNodelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server TcpNodelay element");
                    }

                    try
                    {
                        tmpConfig.BindOptions.TcpOpts.TcpNoDelay = (uint.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Server TcpNodelay element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // ServerCert string
                CurrNode2 = serverListNode.SelectSingleNode("ServerCert");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server ServerCert element");
                    }
                    tmpConfig.BindOptions.BindEncryptionOpts.ServerCertificate = XmlAttribute.Value;
                }

                // ServerPrivateKey string
                CurrNode2 = serverListNode.SelectSingleNode("ServerPrivateKey");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server ServerPrivateKey element");
                    }
                    tmpConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey = XmlAttribute.Value;
                }

                // SecurityProtocol enum
                CurrNode2 = serverListNode.SelectSingleNode("SecurityProtocol");
                if (CurrNode2 != null)
                {
                    uint tmpInt;

                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server SecurityProtocol element");
                    }
                    try
                    {
                        tmpInt = uint.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel SecurityProtocol element is incorrectly formatted. Correct values are combinations of flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags");
                    }

                    if (tmpInt != 0 && (tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid value for Channel element SecurityProtocol. This must be an int type, with the flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags.");
                    }

                    tmpConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags = (EncryptionProtocolFlags)(tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL);
                }

                // CipherSuite This is a comma separated string with either the names or integer values of ciphers specified in System.Net.Security.TlsCipherSuite
                CurrNode2 = serverListNode.SelectSingleNode("CipherSuite");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Server CipherSuite element");
                    }
                    tmpConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites = ServerConfig.StringToCipherList(XmlAttribute.Value, configError);
                }

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a Channel group
                foreach (XmlNode node in serverListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        configError.Add($"Unknown Server entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseLoggerGroup(XmlNode LoggerNode, Dictionary<string, LoggerConfig> configMap, ConfigErrorList configError)
        {
            CurrNode = LoggerNode.SelectSingleNode("LoggerList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing LoggerList element");
            }

            CurrNodeList = CurrNode.SelectNodes("Logger");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing Logger element");
            }

            foreach (XmlNode loggerListNode in CurrNodeList)
            {
                bool foundConfig = false;
                LoggerConfig tmpConfig;

                // Name string, this is required
                CurrNode2 = loggerListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger Name element");
                    }

                    if (configMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = configMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new LoggerConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Logger Name element");
                }

                // FileName string
                CurrNode2 = loggerListNode.SelectSingleNode("FileName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger FileName element");
                    }

                    tmpConfig.FileName = XmlAttribute.Value;
                }

                // IncludeDateInLoggerOutput ulong
                CurrNode2 = loggerListNode.SelectSingleNode("IncludeDateInLoggerOutput");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger IncludeDateInLoggerOutput element");
                    }

                    tmpConfig.IncludeDateInLoggerOutput = ulong.Parse(XmlAttribute.Value);
                }

                // NumberOfLogFiles ulong
                CurrNode2 = loggerListNode.SelectSingleNode("NumberOfLogFiles");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger NumberOfLogFiles element");
                    }

                    tmpConfig.NumberOfLogFiles = ulong.Parse(XmlAttribute.Value);
                }

                // MaxLogFileSize ulong
                CurrNode2 = loggerListNode.SelectSingleNode("MaxLogFileSize");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger MaxLogFileSize element");
                    }

                    tmpConfig.MaxLogFileSize = ulong.Parse(XmlAttribute.Value);
                }

                // LoggerSeverity enumeration
                CurrNode2 = loggerListNode.SelectSingleNode("LoggerSeverity");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger LoggerSeverity element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid LoggerSeverity string format. Correct format is \"LoggerSeverity::<Trace/Debug/Info or Success/Warning/Error or Verbose/NoLogMsg>\".");

                    }

                    if (channelArray[0] != "LoggerSeverity")
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid LoggerSeverity string format. Correct format is \"LoggerSeverity::<Trace/Debug/Info or Success/Warning/Error or Verbose/NoLogMsg>\".");
                    }

                    tmpConfig.LoggerSeverity = LoggerConfig.StringToLoggerLevel(channelArray[1]);

                }

                // LoggerType enumeration
                CurrNode2 = loggerListNode.SelectSingleNode("LoggerType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Logger LoggerType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid LoggerType string format. Correct format is \"LoggerType::<File/Stdout>\".");
                    }

                    if (channelArray[0] != "LoggerType")
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid LoggerType string format. Correct format is \"LoggerType::<File/Stdout>\".");
                    }

                    tmpConfig.LoggerType = LoggerConfig.StringToLoggerType(channelArray[1]);

                }

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a Logger group
                foreach (XmlNode node in loggerListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        configError.Add($"Unknown Logger entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseDictionaryGroup(XmlNode DictionaryNode, Dictionary<string, DictionaryConfig> configMap, ConfigErrorList configError)
        {
            CurrNode = DictionaryNode.SelectSingleNode("DictionaryList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing DictionaryList element");

            }

            CurrNodeList = CurrNode.SelectNodes("Dictionary");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing Dictionary element");
            }

            foreach (XmlNode dictionaryListNode in CurrNodeList)
            {
                bool foundConfig = false;
                DictionaryConfig tmpConfig;

                // Name string, this is required
                CurrNode2 = dictionaryListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary Name element");
                    }

                    if (configMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = configMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new DictionaryConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Dictionary Name element");
                }

                // EnumTypeDefFileName string
                CurrNode2 = dictionaryListNode.SelectSingleNode("EnumTypeDefFileName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary EnumTypeDefFileName element");
                    }

                    tmpConfig.EnumTypeDefFileName = XmlAttribute.Value;
                }

                // EnumTypeDefItemName string
                CurrNode2 = dictionaryListNode.SelectSingleNode("EnumTypeDefItemName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary EnumTypeDefItemName element");
                    }

                    tmpConfig.EnumTypeDefItemName = XmlAttribute.Value;
                }

                // RdmFieldDictionaryFileName string
                CurrNode2 = dictionaryListNode.SelectSingleNode("RdmFieldDictionaryFileName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary RdmFieldDictionaryFileName element");
                    }

                    tmpConfig.RdmFieldDictionaryFileName = XmlAttribute.Value;
                }

                // RdmFieldDictionaryItemName string
                CurrNode2 = dictionaryListNode.SelectSingleNode("RdmFieldDictionaryItemName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary RdmFieldDictionaryItemName element");
                    }

                    tmpConfig.RdmFieldDictionaryItemName = XmlAttribute.Value;
                }

                // DictionaryType enumeration
                CurrNode2 = dictionaryListNode.SelectSingleNode("DictionaryType");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary DictionaryType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid DictionaryType string format. Correct format is \"DictionaryType::<FileDictionary/ChannelDictionary>\".");
                    }

                    if (channelArray[0] != "DictionaryType")
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid DictionaryType string format. Correct format is \"DictionaryType::<FileDictionary/ChannelDictionary>\".");
                    }

                    tmpConfig.DictionaryType = DictionaryConfig.StringToDictionaryMode(channelArray[1]);
                    tmpConfig.IsLocalDictionary = (tmpConfig.DictionaryType == EmaConfig.DictionaryTypeEnum.FILE);
                }

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported element in a Dictionary group
                foreach (XmlNode node in dictionaryListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        configError.Add($"Unknown Dictionary entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseSourceDirectoryGroup(XmlNode DirectoryNode, Dictionary<string, DirectoryConfig> configMap, bool niProvider,
            ConfigErrorList configError, out String defaultDirectoryName, out string firstDirectoryName)
        {
            CurrNode = DirectoryNode.SelectSingleNode("DirectoryList");
            defaultDirectoryName = string.Empty;
            firstDirectoryName = string.Empty;
            XmlNode? serviceNode;
            HashSet<int> serviceIds = new HashSet<int>();
            ushort generateServiceID;
            bool setServiceId;

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing DirectoryList element");
            }

            CurrNode2 = CurrNode.SelectSingleNode("DefaultDirectory");
            if (CurrNode2 != null)
            {
                // Get the Value from the DefaultConsumer, if present
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException("Missing Default Directory Value attribute");
                }

                defaultDirectoryName = XmlAttribute.Value;
            }


            CurrNodeList = CurrNode.SelectNodes("Directory");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing Dictionary element");
            }

            foreach (XmlNode directoryListNode in CurrNodeList)
            {
                bool foundConfig = false;
                DirectoryConfig tmpConfig;

                // Name string, this is required
                CurrNode2 = directoryListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the Dictionary Name element");
                    }

                    if (configMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = configMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new DirectoryConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }

                    if (string.IsNullOrEmpty(firstDirectoryName))
                    {
                        firstDirectoryName = XmlAttribute.Value;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Dictionary Name element");
                }

                serviceIds.Clear(); // Clears the list of service Ids per directory
                generateServiceID = 0; // Clears the generated service ID per directory

                InnerNodeList = directoryListNode.SelectNodes("Service");

                if (InnerNodeList != null)
                {
                    foreach (XmlNode serviceListNode in InnerNodeList)
                    {
                        bool foundServiceConfig = false;
                        EmaServiceConfig tmpServiceConfig;
                        XmlNodeList? serviceXmlNodeList;
                        XmlNode? serviceXmlNode;
                        setServiceId = false;

                        // Name string, this is required
                        CurrNode2 = serviceListNode.SelectSingleNode("Name");

                        if (CurrNode2 != null)
                        {
                            ValueNode = (XmlElement)CurrNode2;
                            XmlAttribute = ValueNode.GetAttributeNode("value");

                            if (XmlAttribute == null)
                            {
                                throw new OmmInvalidConfigurationException(
                                    "Missing value attribute in the Service Name element");
                            }

                            if (tmpConfig.ServiceMap.ContainsKey(XmlAttribute.Value))
                            {
                                tmpServiceConfig = tmpConfig.ServiceMap[XmlAttribute.Value];
                                foundServiceConfig = true;
                            }
                            else
                            {
                                tmpServiceConfig = new EmaServiceConfig(niProvider, false);
                                tmpServiceConfig.Service.Info.ServiceName.Data(XmlAttribute.Value);
                            }
                        }
                        else
                        {
                            throw new OmmInvalidConfigurationException("Missing Service Name element");
                        }

                        serviceNode = serviceListNode.SelectSingleNode("InfoFilter");

                        if (serviceNode != null)
                        {
                            // ServiceId uint
                            CurrNode2 = serviceNode.SelectSingleNode("ServiceId");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the ServiceId element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.ServiceId = Utilities.Convert_uint_int(uint.Parse(XmlAttribute.Value));

                                    if(tmpServiceConfig.Service.ServiceId > ushort.MaxValue)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                        $"service[{tmpServiceConfig.Service.Info.ServiceName}] specifies out of range ServiceId ({tmpServiceConfig.Service.ServiceId}).");
                                    }

                                    if(serviceIds.Contains(tmpServiceConfig.Service.ServiceId))
                                    {
                                        throw new OmmInvalidConfigurationException(
                                       $"service[{tmpServiceConfig.Service.Info.ServiceName}] specifies the same ServiceId (value of {tmpServiceConfig.Service.ServiceId}) as already specified by another service.");
                                    }

                                    setServiceId = true;
                                    serviceIds.Add(tmpServiceConfig.Service.ServiceId);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service ServiceId element is incorrectly formatted. Correct format is an unsigned numeric string.");
                                }
                            }

                            // Vendor string
                            CurrNode2 = serviceNode.SelectSingleNode("Vendor");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory Service Vendor element");
                                }

                                tmpServiceConfig.Service.Info.HasVendor = true;
                                tmpServiceConfig.Service.Info.Vendor.Data(XmlAttribute.Value);
                            }

                            // IsSource ulong->bool
                            CurrNode2 = serviceNode.SelectSingleNode("IsSource");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Service IsSource element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Info.HasIsSource = true;
                                    tmpServiceConfig.Service.Info.IsSource = long.Parse(XmlAttribute.Value);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service IsSource element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                                }
                            }

                            // Capabilities list and Capabilities Entry parsing
                            CurrNode2 = serviceNode.SelectSingleNode("Capabilities");
                            if (CurrNode2 != null)
                            {
                                serviceXmlNodeList = CurrNode2.SelectNodes("CapabilitiesEntry");

                                if (serviceXmlNodeList != null)
                                {
                                    foreach (XmlNode capabilitiesNode in serviceXmlNodeList)
                                    {
                                        ValueNode = (XmlElement)capabilitiesNode;
                                        XmlAttribute = ValueNode.GetAttributeNode("value");
                                        long tmpCapability;

                                        if (XmlAttribute == null)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Missing value attribute in the Service CapabilitiesEntry element");
                                        }

                                        try
                                        {
                                            tmpCapability = long.Parse(XmlAttribute.Value);
                                            if (tmpCapability < 0 ||
                                                tmpCapability > EmaConfig.CapabilitiesEnum.MMT_MAX_VALUE)
                                            {
                                                throw new OmmInvalidConfigurationException(
                                                    "Invalid Service CapabilitiesEntry element value. Correct numeric values are 0-255.");
                                            }

                                            if (!tmpServiceConfig.Service.Info.CapabilitiesList.Contains(tmpCapability))
                                            {
                                                tmpServiceConfig.Service.Info.CapabilitiesList.Add(tmpCapability);
                                            }
                                        }
                                        catch (SystemException)
                                        {
                                            // This indicates that the entry is not an alphanumeric number, so it should be parsed as a string.
                                            tmpCapability = DirectoryConfig.StringToCapability(XmlAttribute.Value);
                                            if (!tmpServiceConfig.Service.Info.CapabilitiesList.Contains(tmpCapability))
                                            {
                                                tmpServiceConfig.Service.Info.CapabilitiesList.Add(tmpCapability);
                                            }
                                        }

                                        // Checks for unsupported elements
                                        foreach (XmlNode node in capabilitiesNode.ChildNodes)
                                        {
                                            if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                            {
                                                configError.Add($"Unknown Service InfoFilter CapabilitiesEntry element: {node.Name}", LoggerLevel.ERROR);
                                            }
                                        }
                                    }
                                }

                                // Checks for unsupported elements
                                foreach (XmlNode node in CurrNode2.ChildNodes)
                                {
                                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                    {
                                        configError.Add($"Unknown Service InfoFilter Capabilities element: {node.Name}", LoggerLevel.ERROR);
                                    }
                                }
                            }

                            // AcceptingConsumerStatus ulong->bool
                            CurrNode2 = serviceNode.SelectSingleNode("AcceptingConsumerStatus");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Service AcceptingConsumerStatus element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Info.HasAcceptingConsStatus = true;
                                    tmpServiceConfig.Service.Info.AcceptConsumerStatus = long.Parse(XmlAttribute.Value);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service AcceptingConsumerStatus element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                                }
                            }

                            // ItemList string
                            CurrNode2 = serviceNode.SelectSingleNode("ItemList");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory Service ItemList element");
                                }

                                tmpServiceConfig.Service.Info.HasItemList = true;
                                tmpServiceConfig.Service.Info.ItemList.Data(XmlAttribute.Value);
                            }

                            // DictionariesProvided list and DictionariesProvided Entry parsing
                            CurrNode2 = serviceNode.SelectSingleNode("DictionariesProvided");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                serviceXmlNodeList = CurrNode2.SelectNodes("DictionariesProvidedEntry");

                                if (serviceXmlNodeList != null)
                                {
                                    foreach (XmlNode capabilitiesNode in serviceXmlNodeList)
                                    {
                                        ValueNode = (XmlElement)capabilitiesNode;
                                        XmlAttribute = ValueNode.GetAttributeNode("value");

                                        if (XmlAttribute == null)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Missing value attribute in the Service DictionariesProvidedEntry element");
                                        }

                                        if (!tmpServiceConfig.DictionariesProvidedList.Contains(XmlAttribute.Value))
                                        {
                                            tmpServiceConfig.DictionariesProvidedList.Add(XmlAttribute.Value);
                                        }
                                    }
                                }
                            }

                            // DictionariesUsed list and DictionariesUsed Entry parsing
                            CurrNode2 = serviceNode.SelectSingleNode("DictionariesUsed");
                            if (CurrNode2 != null)
                            {
                                serviceXmlNodeList = CurrNode2.SelectNodes("DictionariesUsedEntry");

                                if (serviceXmlNodeList != null)
                                {
                                    foreach (XmlNode capabilitiesNode in serviceXmlNodeList)
                                    {
                                        ValueNode = (XmlElement)capabilitiesNode;
                                        XmlAttribute = ValueNode.GetAttributeNode("value");

                                        if (XmlAttribute == null)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Missing value attribute in the Service DictionariesUsedEntry element");
                                        }

                                        if (!tmpServiceConfig.DictionariesUsedList.Contains(XmlAttribute.Value))
                                        {
                                            tmpServiceConfig.DictionariesUsedList.Add(XmlAttribute.Value);
                                        }
                                    }
                                }
                            }

                            // QoS list and QoS Entry parsing
                            CurrNode2 = serviceNode.SelectSingleNode("QoS");
                            if (CurrNode2 != null)
                            {
                                serviceXmlNodeList = CurrNode2.SelectNodes("QoSEntry");

                                if (serviceXmlNodeList != null)
                                {
                                    foreach (XmlNode qosNode in serviceXmlNodeList)
                                    {
                                        uint rate = 0;
                                        uint timeliness = 0;
                                        serviceXmlNode = qosNode.SelectSingleNode("Timeliness");

                                        if (serviceXmlNode != null)
                                        {
                                            ValueNode = (XmlElement)serviceXmlNode;
                                            XmlAttribute = ValueNode.GetAttributeNode("value");

                                            if (XmlAttribute == null)
                                            {
                                                throw new OmmInvalidConfigurationException(
                                                    "Missing value attribute in the Service DictionariesUsedEntry element");
                                            }

                                            try
                                            {
                                                timeliness = uint.Parse(XmlAttribute.Value);
                                            }
                                            catch (SystemException)
                                            {
                                                // This indicates that the entry is not an alphanumeric number, so it should be parsed as a string.
                                                string[] timelinessArray = XmlAttribute.Value.Split("::");

                                                if (timelinessArray.Length != 2)
                                                {
                                                    throw new OmmInvalidConfigurationException(
                                                        "Invalid QoS Timeliness string format. Correct format is \"Timeliness::<RealTime or InexactDelayed>\".");
                                                }

                                                if (timelinessArray[0] != "Timeliness")
                                                {
                                                    throw new OmmInvalidConfigurationException(
                                                        "Invalid QoS Timeliness string format. Correct format is \"Timeliness::<RealTime or InexactDelayed>\".");
                                                }

                                                timeliness = DirectoryConfig.StringToTimeliness(timelinessArray[1]);
                                            }
                                        }

                                        serviceXmlNode = qosNode.SelectSingleNode("Rate");

                                        if (serviceXmlNode != null)
                                        {
                                            ValueNode = (XmlElement)serviceXmlNode;
                                            XmlAttribute = ValueNode.GetAttributeNode("value");

                                            if (XmlAttribute == null)
                                            {
                                                throw new OmmInvalidConfigurationException(
                                                    "Missing value attribute in the Service DictionariesUsedEntry element");
                                            }

                                            try
                                            {
                                                rate = uint.Parse(XmlAttribute.Value);
                                            }
                                            catch (SystemException)
                                            {
                                                string[] rateArray = XmlAttribute.Value.Split("::");

                                                if (rateArray.Length != 2)
                                                {
                                                    throw new OmmInvalidConfigurationException(
                                                        "Invalid QoS Rate string format. Correct format is \"Rate::<TickByTick or JustInTimeConflated>\".");
                                                }

                                                if (rateArray[0] != "Rate")
                                                {
                                                    throw new OmmInvalidConfigurationException(
                                                        "Invalid QoS Rate string format. Correct format is \"Rate::<TickByTick or JustInTimeConflated>\".");
                                                }
                                                // This indicates that the entry is not an alphanumeric number, so it should be parsed as a string.
                                                rate = DirectoryConfig.StringToRate(rateArray[1]);
                                            }
                                        }

                                        Qos tmpQos = new Qos();

                                        Utilities.ToRsslQos(rate, timeliness, tmpQos);

                                        tmpServiceConfig.Service.Info.QosList.Add(tmpQos);
                                        
                                        // Checks for unsupported elements
                                        foreach (XmlNode node in qosNode.ChildNodes)
                                        {
                                            if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                            {
                                                configError.Add($"Unknown Service InfoFilter QoSEntry element: {node.Name}", LoggerLevel.ERROR);
                                            }
                                        }
                                    }
                                }

                                // Checks for unsupported elements
                                foreach (XmlNode node in CurrNode2.ChildNodes)
                                {
                                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                    {
                                        configError.Add($"Unknown Service InfoFilter QoS element: {node.Name}", LoggerLevel.ERROR);
                                    }
                                }
                            }

                            // SupportsQoSRange ulong->bool
                            CurrNode2 = serviceNode.SelectSingleNode("SupportsQoSRange");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Service SupportsQoSRange element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Info.HasSupportQosRange = true;
                                    tmpServiceConfig.Service.Info.SupportsQosRange = long.Parse(XmlAttribute.Value);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service SupportsQoSRange element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                                }
                            }

                            // SupportsOutOfBandSnapshots ulong->bool
                            CurrNode2 = serviceNode.SelectSingleNode("SupportsOutOfBandSnapshots");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Service SupportsOutOfBandSnapshots element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Info.HasSupportOOBSnapshots = true;
                                    tmpServiceConfig.Service.Info.SupportsOOBSnapshots = long.Parse(XmlAttribute.Value);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service SupportsOutOfBandSnapshots element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                                }
                            }

                            // Checks for unsupported elements
                            foreach (XmlNode node in serviceListNode.ChildNodes)
                            {
                                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                {
                                    configError.Add($"Unknown Service InfoFilter element: {node.Name}", LoggerLevel.ERROR);
                                }
                            }
                        }

                        // State Filter group
                        serviceNode = serviceListNode.SelectSingleNode("StateFilter");

                        if (serviceNode != null)
                        {
                            // ServiceState ulong->int
                            CurrNode2 = serviceNode.SelectSingleNode("ServiceState");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory ServiceState element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.State.ServiceStateVal = uint.Parse(XmlAttribute.Value);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service ServiceState element is incorrectly formatted.  Correct values are: \"0\" or \"1\".");
                                }
                            }

                            // AcceptingRequests ulong->int
                            CurrNode2 = serviceNode.SelectSingleNode("AcceptingRequests");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory ServiceState element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.State.HasAcceptingRequests = true;
                                    tmpServiceConfig.Service.State.AcceptingRequests = uint.Parse(XmlAttribute.Value);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service ServiceState element is incorrectly formatted.  Correct values are: \"0\" or \"1\".");
                                }
                            }

                            // Status parsing
                            CurrNode2 = serviceNode.SelectSingleNode("Status");

                            if (CurrNode2 != null)
                            {
                                XmlNode? statusNode = CurrNode2.SelectSingleNode("StreamState");
                                if (statusNode != null)
                                {
                                    // StreamState string: This will remove the "StreamState::" prepend and call StringToStreamState

                                    ValueNode = (XmlElement)statusNode;
                                    XmlAttribute = ValueNode.GetAttributeNode("value");

                                    if (XmlAttribute == null)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Missing value attribute in the Directory Service StreamState element");
                                    }

                                    string[] streamStateArray = XmlAttribute.Value.Split("::");

                                    if (streamStateArray.Length != 2)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Invalid StreamState string format. Correct format is \"StreamState::<Open, NonStreaming, Closed, ClosedRecover, or ClosedRedirected>\".");
                                    }

                                    if (streamStateArray[0] != "StreamState")
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Invalid StreamState string format. Correct format is \"StreamState::<Open, NonStreaming, Closed, ClosedRecover, or ClosedRedirected>\".");
                                    }

                                    tmpServiceConfig.Service.State.Status.StreamState(DirectoryConfig.StringToStreamState(streamStateArray[1]));
                                }


                                // DataState string: This will remove the "DataState::" prepend and call StringToDataState
                                statusNode = CurrNode2.SelectSingleNode("DataState");
                                if (statusNode != null)
                                {
                                    ValueNode = (XmlElement)statusNode;
                                    XmlAttribute = ValueNode.GetAttributeNode("value");

                                    if (XmlAttribute == null)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Missing value attribute in the Directory Service DataState element");
                                    }

                                    string[] dataStateArray = XmlAttribute.Value.Split("::");

                                    if (dataStateArray.Length != 2)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Invalid DataState string format. Correct format is \"DataState::<NoChange, Ok, Suspect>\".");
                                    }

                                    if (dataStateArray[0] != "DataState")
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Invalid DataState string format. Correct format is \"DataState::<NoChange, Ok, Suspect>\".");
                                    }

                                    tmpServiceConfig.Service.State.Status.DataState(DirectoryConfig.StringToDataState(dataStateArray[1]));
                                }

                                // StatusCode string: This will remove the "StatusCode::" prepend and call StringToStatusCode
                                statusNode = CurrNode2.SelectSingleNode("StatusCode");
                                if (statusNode != null)
                                {
                                    ValueNode = (XmlElement)statusNode;
                                    XmlAttribute = ValueNode.GetAttributeNode("value");

                                    if (XmlAttribute == null)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Missing value attribute in the Directory Service StatusCode element");
                                    }

                                    string[] statusCodeArray = XmlAttribute.Value.Split("::");

                                    if (statusCodeArray.Length != 2)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Invalid StatusCode string format. Correct format is \"StatusCode::<Strings listed in OmmState.cs>\".");
                                    }

                                    if (statusCodeArray[0] != "StatusCode")
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Invalid StatusCode string format. Correct format is \"StatusCode::<Strings listed in OmmState.cs>\".");
                                    }

                                    tmpServiceConfig.Service.State.Status.Code(DirectoryConfig.StringToStatusCode(statusCodeArray[1]));
                                }

                                // StatusText string
                                statusNode = CurrNode2.SelectSingleNode("StatusText");
                                if (statusNode != null)
                                {
                                    ValueNode = (XmlElement)statusNode;
                                    XmlAttribute = ValueNode.GetAttributeNode("value");

                                    if (XmlAttribute == null)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                            "Missing value attribute in the Directory Service StatusText element");
                                    }

                                    tmpServiceConfig.Service.State.Status.Text().Data(XmlAttribute.Value);
                                }

                                // Checks for unsupported elements
                                foreach (XmlNode node in CurrNode2.ChildNodes)
                                {
                                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                    {
                                        configError.Add($"Unknown Service StateFilter Status element: {node.Name}", LoggerLevel.ERROR);
                                    }
                                }
                            }

                            // Checks for unsupported elements
                            foreach (XmlNode node in serviceNode.ChildNodes)
                            {
                                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                {
                                    configError.Add($"Unknown Service StateFilter element: {node.Name}", LoggerLevel.ERROR);
                                }
                            }
                        }

                        // Load Filter group
                        serviceNode = serviceListNode.SelectSingleNode("LoadFilter");

                        if (serviceNode != null)
                        {
                            // OpenLimit ulong->long
                            CurrNode2 = serviceNode.SelectSingleNode("OpenLimit");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory OpenLimit element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Load.HasOpenLimit = true;
                                    tmpServiceConfig.Service.Load.OpenLimit = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service OpenLimit element is incorrectly formatted. Correct format is an unsigned numeric string");
                                }
                            }

                            // OpenWindow ulong->long
                            CurrNode2 = serviceNode.SelectSingleNode("OpenWindow");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory OpenWindow element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Load.HasOpenWindow = true;
                                    tmpServiceConfig.Service.Load.OpenWindow = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service OpenWindow element is incorrectly formatted. Correct format is an unsigned numeric string");
                                }
                            }

                            // LoadFactor ulong->long
                            CurrNode2 = serviceNode.SelectSingleNode("LoadFactor");
                            if (CurrNode2 != null)
                            {
                                ValueNode = (XmlElement)CurrNode2;
                                XmlAttribute = ValueNode.GetAttributeNode("value");

                                if (XmlAttribute == null)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "Missing value attribute in the Directory LoadFactor element");
                                }

                                try
                                {
                                    tmpServiceConfig.Service.Load.HasLoadFactor = true;
                                    tmpServiceConfig.Service.Load.LoadFactor = Utilities.Convert_ulong_long(ulong.Parse(XmlAttribute.Value));
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        "The value attribute in the Directory Service LoadFactor element is incorrectly formatted. Correct format is an unsigned numeric string");
                                }
                            }

                            foreach (XmlNode node in serviceNode.ChildNodes)
                            {
                                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                                {
                                    configError.Add($"Unknown Service LoadInfo entry element: {node.Name}", LoggerLevel.ERROR);
                                }
                            }
                        }

                        if (foundServiceConfig == false)
                        {
                            if(!setServiceId)
                            {
                                while (serviceIds.Contains(generateServiceID))
                                {
                                    ++generateServiceID;
                                }

                                if(generateServiceID > ushort.MaxValue)
                                {
                                    throw new OmmInvalidConfigurationException(
                                        $"EMA ran out of assignable service ids.");
                                }

                                tmpServiceConfig.Service.ServiceId = generateServiceID;
                                serviceIds.Add(generateServiceID);
                                ++generateServiceID;
                            }

                            tmpConfig.ServiceMap.Add(tmpServiceConfig.Service.Info.ServiceName.ToString(), tmpServiceConfig);
                        }

                        // Checks for unsupported elements
                        foreach (XmlNode node in serviceListNode.ChildNodes)
                        {
                            if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                            {
                                configError.Add($"Unknown Service entry element: {node.Name}", LoggerLevel.ERROR);
                            }
                        }
                    }
                }

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);

                // Checks for unsupported elements
                foreach (XmlNode node in directoryListNode.ChildNodes)
                {
                    if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                    {
                        configError.Add($"Unknown Directory entry element: {node.Name}", LoggerLevel.ERROR);
                    }
                }
            }
        }

        private void ParseXmlTraceConfigNodes(string groupName, XmlTraceConfigurable configImpl, XmlNode configListNode)
        {
            XmlNode? CurrNode2;

            // XmlTraceToStdout bool
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceToStdout");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceToStdout element");
                }

                try
                {
                    configImpl.XmlTraceToStdout = (ulong.Parse(XmlAttribute.Value) != 0);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTraceToStdout element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

            // XmlTraceToFile bool
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceToFile");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceToFile element");
                }

                try
                {
                    configImpl.XmlTraceToFile = (ulong.Parse(XmlAttribute.Value) != 0);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTraceToFile element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

            // XmlTraceMaxFileSize ulong
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceMaxFileSize");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceMaxFileSize element");
                }

                try
                {
                    configImpl.XmlTraceMaxFileSize = ulong.Parse(XmlAttribute.Value);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTraceMaxFileSize element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

            // XmlTraceFileName string
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceFileName");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceFileName element");
                }
                configImpl.XmlTraceFileName = XmlAttribute.Value;
            }


            // XmlTraceToMultipleFiles bool
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceToMultipleFiles");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceToMultipleFiles element");
                }

                try
                {
                    configImpl.XmlTraceToMultipleFiles = (ulong.Parse(XmlAttribute.Value) != 0);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTraceToMultipleFiles element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

            // XmlTraceWrite bool
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceWrite");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceWrite element");
                }

                try
                {
                    configImpl.XmlTraceWrite = (ulong.Parse(XmlAttribute.Value) != 0);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTraceWrite element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

            // XmlTraceRead bool
            CurrNode2 = configListNode.SelectSingleNode("XmlTraceRead");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTraceRead element");
                }

                try
                {
                    configImpl.XmlTraceRead = (ulong.Parse(XmlAttribute.Value) != 0);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTraceRead element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

            // XmlTracePing bool
            CurrNode2 = configListNode.SelectSingleNode("XmlTracePing");
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} XmlTracePing element");
                }

                try
                {
                    configImpl.XmlTracePing = (ulong.Parse(XmlAttribute.Value) != 0);
                }
                catch (SystemException)
                {
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} XmlTracePing element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                }
            }

        }

        // Load XML configuration document, and if XML schema definition file is detected,
        // validate it
        private XmlDocument LoadXmlConfig(string? configFilePath)
        {
            XmlDocument ConfigXml = new XmlDocument();
            ConfigXml.PreserveWhitespace = true;

            // Load the XML Configuration document from file
            try
            {
                if (string.IsNullOrEmpty(configFilePath))
                {
                    ConfigXml.Load(DEFAULT_CONFIG_FILE);
                }
                else
                {
                    ConfigXml.Load(configFilePath);
                }
            }
            catch (System.IO.FileNotFoundException excp)
            {
                // If the path is set to null, the application could be using the default
                // config or a programmatic config, so just return without adding any info
                // to the Config.
                if (string.IsNullOrEmpty(configFilePath))
                    return ConfigXml;
                else
                {
                    throw new OmmInvalidConfigurationException(
                        $"Could not load the configured XML file. FileNotFoundException text: {excp.Message}");
                }
            }
            catch (XmlException excp)
            {
                throw new OmmInvalidConfigurationException(
                    $"Error parsing XML file. XmlException text: {excp.Message}");
            }
            catch (Exception excp)
            {
                throw new OmmInvalidConfigurationException(
                    $"Error loading XML file. Exception text: {excp.Message}");
            }

            // happens only when the XML file is present and is read, but is empty
            if (ConfigXml.DocumentElement is null)
            {
                throw new OmmInvalidConfigurationException(
                    "XML Parsing failed.");
            }

            if (!"EmaConfig".Equals(ConfigXml.DocumentElement.Name))
            {
                throw new OmmInvalidConfigurationException(
                    "Error parsing XML file. Root element is not \"EmaConfig\"");
            }

            // XML Document is loaded, now validate it (but only when XML Schema is present)
            string schemaFileName = DEFAULT_SCHEMA_FILE;

            if (System.IO.File.Exists(schemaFileName))
            {
                try
                {
                    ConfigXml.Schemas.Add("", schemaFileName);

                    ConfigXml.Validate((object? sender, ValidationEventArgs e) =>
                    {
                        if (e.Severity == XmlSeverityType.Error)
                        {
                            throw new OmmInvalidConfigurationException(
                                $"Error validating XML configuration: {e.Message}");
                        }
                    });
                }
                catch (XmlException ex)
                {
                    throw new OmmInvalidConfigurationException(
                        $"XML Configuration validation failed: {ex.Message}");
                }
                catch (XmlSchemaException ex)
                {
                    throw new OmmInvalidConfigurationException(
                        $"XML Schema is not valid: {ex.Message} at line {ex.LineNumber}");
                }
            }

            return ConfigXml;
        }
    }
}
