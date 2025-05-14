/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.Schema;

using LSEG.Eta.Transports;
using LSEG.Eta.Codec;
using System.IO;
using System.Linq.Expressions;
using System.Reflection;
using System.Diagnostics.CodeAnalysis;

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

        private XmlDocument ConfigXml { get; set; }

        private const string CorrectBooleanFormatMessage = "Correct values are: \"0\" or \"1\".";
        private const string CorrectUnsignedNumericMessage = "Correct format is an unsigned numeric string.";

        // Reusable XmlNode references
        private XmlNode? CurrNode;
        private XmlElement? ValueNode;
        private XmlNodeList? CurrNodeList;
        private XmlNodeList? InnerNodeList;

        private XmlNode? CurrNode2;
        private XmlAttribute? XmlAttribute;

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

            GroupNode = ConfigNode.SelectSingleNode("SessionChannelGroup");
            if (GroupNode != null)
            {
                // Parse the SessionChannel group here
                ParseSessionChannelGroup(GroupNode, Config.SessionChannelInfoMap, Config.ConfigErrorLog!);
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
                // Parse the Provider Group here
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
        }

        private static string IncorrectFormatMessage(string attribut)
            => $"The value attribute in the {attribut} element is incorrectly formatted.";

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
                NodeParser consumerNodeParser = new("Consumer", consumerListNode);
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

                HandleSingleListNode(consumerListNode, "Consumer", "ChannelSet", tmpConfig.ChannelSet);

                consumerNodeParser
                    .Parse(() => tmpConfig.Dictionary)
                    .Parse(() => tmpConfig.DictionaryRequestTimeOut)
                    .Parse(() => tmpConfig.DirectoryRequestTimeOut)
                    .Parse(() => tmpConfig.LoginRequestTimeOut)
                    .Parse(() => tmpConfig.DispatchTimeoutApiThread)
                    .Parse(() => tmpConfig.EnableRtt)
                    .Parse(() => tmpConfig.ItemCountHint, hint => hint == 0 ? 1024 : hint)
                    .Parse(() => tmpConfig.Logger)
                    .Parse(() => tmpConfig.MaxDispatchCountApiThread)
                    .Parse(() => tmpConfig.MaxDispatchCountUserThread)
                    .Parse(() => tmpConfig.MaxOutstandingPosts)
                    .Parse(() => tmpConfig.MsgKeyInUpdates)
                    .Parse(() => tmpConfig.ObeyOpenWindow)
                    .Parse(() => tmpConfig.PostAckTimeout)
                    .Parse(() => tmpConfig.ReconnectAttemptLimit)
                    .Parse(() => tmpConfig.ReconnectMaxDelay, v => v > 0 ? v : tmpConfig.ReconnectMaxDelay)
                    .Parse(() => tmpConfig.ReconnectMinDelay, v => v > 0 ? v : tmpConfig.ReconnectMinDelay)
                    .Parse(() => tmpConfig.RequestTimeout, v => v > 0 ? v : tmpConfig.RequestTimeout)
                    .Parse(() => tmpConfig.ServiceCountHint, v => v == 0 ? 513 : v)
                    .Parse(() => tmpConfig.RestLogFileName)
                    .Parse(() => tmpConfig.RestRequestTimeOut)
                    .Parse(() => tmpConfig.RestProxyHostName)
                    .Parse(() => tmpConfig.RestProxyPort)
                    .Parse(() => tmpConfig.RestEnableLog)
                    .Parse(() => tmpConfig.RestEnableLogViaCallback)
                    .Parse(() => tmpConfig.SessionEnhancedItemRecovery);

                ParseXmlTraceConfigNodes(consumerNodeParser, tmpConfig);
                if (foundConfig == false)
                    Config.ConsumerConfigMap.Add(tmpConfig.Name, tmpConfig);

                // SessionChannelSet string containing a comma separated list of SessionChannel names
                HandleSingleListNode(consumerListNode, "Consumer", "SessionChannelSet", tmpConfig.SessionChannelSet);
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
                NodeParser niProviderParser = new("NiProvider", niProviderListNode);
                if (CurrNode == null)
                    return;

                // Name string, this is required
                CurrNode2 = niProviderListNode.SelectSingleNode("Name");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    var XmlAttribute = ValueNode.GetAttributeNode("value");

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
                    var XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Missing value attribute in the NiProvider Channel element");
                    }

                    tmpConfig.ChannelSet.Add(XmlAttribute.Value);
                }

                HandleSingleListNode(niProviderListNode, "NiProvider", "ChannelSet", tmpConfig.ChannelSet);

                niProviderParser.Parse(() => tmpConfig.Directory)
                    .Parse(() => tmpConfig.DispatchTimeoutApiThread)
                    .Parse(() => tmpConfig.ItemCountHint, v => v == 0 ? 1024 : v)
                    .Parse(() => tmpConfig.Logger)
                    .Parse(() => tmpConfig.LoginRequestTimeOut)
                    .Parse(() => tmpConfig.MaxDispatchCountApiThread)
                    .Parse(() => tmpConfig.LoginRequestTimeOut)
                    .Parse(() => tmpConfig.MaxDispatchCountUserThread)
                    .Parse(() => tmpConfig.MergeSourceDirectoryStreams)
                    .Parse(() => tmpConfig.ReconnectAttemptLimit)
                    .Parse(() => tmpConfig.ReconnectMaxDelay, v => v > 0 ? v : tmpConfig.ReconnectMaxDelay)
                    .Parse(() => tmpConfig.ReconnectMinDelay, v => v > 0 ? v : tmpConfig.ReconnectMinDelay)
                    .Parse(() => tmpConfig.RecoverUserSubmitSourceDirectory)
                    .Parse(() => tmpConfig.RefreshFirstRequired)
                    .Parse(() => tmpConfig.RemoveItemsOnDisconnect)
                    .Parse(() => tmpConfig.RequestTimeout)
                    .Parse(() => tmpConfig.ServiceCountHint, v => v == 0 ? 513 : v);

                ParseXmlTraceConfigNodes(niProviderParser, tmpConfig);

                if (foundConfig == false)
                    Config.NiProviderConfigMap.Add(tmpConfig.Name, tmpConfig);
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
                NodeParser iProviderParser = new ("IProvider", iProviderListNode);
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


                iProviderParser
                    .Parse(() => tmpConfig.Directory)
                    .Parse(() => tmpConfig.DispatchTimeoutApiThread)
                    .Parse(() => tmpConfig.ItemCountHint, v => v == 0 ? 1024 : v)
                    .Parse(() => tmpConfig.Logger)
                    .Parse(() => tmpConfig.MaxDispatchCountApiThread)
                    .Parse(() => tmpConfig.MaxDispatchCountUserThread)
                    .Parse(() => tmpConfig.RefreshFirstRequired)
                    .Parse(() => tmpConfig.RequestTimeout)
                    .Parse(() => tmpConfig.ServiceCountHint, v => v == 0 ? 513 : v)
                    .Parse(() => tmpConfig.Server)
                    .Parse(() => tmpConfig.AcceptDirMessageWithoutMinFilters)
                    .Parse(() => tmpConfig.AcceptMessageSameKeyButDiffStream)
                    .Parse(() => tmpConfig.AcceptMessageThatChangesService)
                    .Parse(() => tmpConfig.AcceptMessageWithoutAcceptingRequests)
                    .Parse(() => tmpConfig.AcceptMessageWithoutBeingLogin)
                    .Parse(() => tmpConfig.AcceptMessageWithoutQosInRange)
                    .Parse(() => tmpConfig.EnforceAckIDValidation)
                    .Parse(() => tmpConfig.EnumTypeFragmentSize)
                    .Parse(() => tmpConfig.FieldDictionaryFragmentSize);

                ParseXmlTraceConfigNodes(iProviderParser, tmpConfig);
                if (foundConfig == false)
                    Config.IProviderConfigMap.Add(tmpConfig.Name, tmpConfig);
            }
        }

        private static TryParseDelegate<T> CreatePrefixedValueParser<T>(string type, Func<string, T> parseValue) => 
            (string str, out T value) =>
        {
            var array = str.Split("::");
            if (array.Length == 2 && array[0] == type)
            {
                value = parseValue(array[1]);
                return true;
            }
            value = default!;
            return false;
        };

        private static readonly TryParseDelegate<EncryptionProtocolFlags> TryParseEncryptionProtocolFlags = CombineTryParseWithTryConvert(uint.TryParse, (uint tmpInt, out EncryptionProtocolFlags flags) =>
        {
            flags = default;
            if (tmpInt != 0 && (tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
            {
                return false;
            }
            flags = (EncryptionProtocolFlags)(tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL);
            return true;
        });

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
                NodeParser channelParser = new ("Channel", channelListNode);
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

                channelParser
                    // ChannelType string: This will remove the "ChannelType::" prepend and call StringToConnectionType
                    .Parse("ChannelType", () => tmpConfig.ConnectInfo.ConnectOptions.ConnectionType,
                        CreatePrefixedValueParser("ChannelType", ClientChannelConfig.StringToConnectionType), 
                        "Invalid ConnectionType string format. Correct format is \"ChannelType::<RSSL_SOCKET or RSSL_ENCRYPTED>\".")
                    .Parse("ConnectionPingTimeout", () => tmpConfig.ConnectInfo.ConnectOptions.PingTimeout, pingTimeout => pingTimeout >= 1000 ? pingTimeout / 1000 : 60)
                    .Parse(() => tmpConfig.ConnectInfo.EnableSessionManagement)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers)
                    .Parse(() => tmpConfig.HighWaterMark)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName)
                    .Parse(() => tmpConfig.ConnectInfo.Location)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.NumInputBuffers)
                    .Parse(() => tmpConfig.ConnectInfo.ServiceDiscoveryRetryCount)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.SysRecvBufSize)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.SysSendBufSize)
                    .Parse("CompressionThreshold",
                        (uint v) =>
                        {
                            tmpConfig.CompressionThreshold = Utilities.Convert_uint_int(v);
                            tmpConfig.CompressionThresholdSet = true;
                        }, uint.TryParse, CorrectUnsignedNumericMessage)
                    .Parse("Host", () => tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address)
                    .Parse("Port", () => tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                    .Parse("ProxyHost", () => tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort)
                    .Parse("TcpNodelay", () => tmpConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay)
                    .Parse(() => tmpConfig.DirectWrite)
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout,
                            v => v > 0 ? v : tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout)

                    // InitializationTimeout uint, parsed as int because of the ConnectInfo method
                    .Parse("InitializationTimeout", t => tmpConfig.ConnectInfo.SetInitTimeout(t))
                    .Parse(() => tmpConfig.ConnectInfo.ConnectOptions.CompressionType,
                        CreatePrefixedValueParser("CompressionType", ClientChannelConfig.StringToCompressionType), 
                        "Invalid CompressionType string format. Correct format is \"CompressionType::<None/ZLib/LZ4>\".")

                    // EncryptedProtocolType string: This will remove the "EncryptedProtocolType::" prepend and call StringToConnectionType
                    .Parse("EncryptedProtocolType", () => tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol,
                        CreatePrefixedValueParser("EncryptedProtocolType", ClientChannelConfig.StringToConnectionType),  
                        "Invalid EncryptedProtocolType string format. Correct format is \"EncryptedProtocolType::<RSSL_SOCKET>\".")

                    // SecurityProtocol enum
                    .Parse("SecurityProtocol", () => tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags,
                        TryParseEncryptionProtocolFlags , $"Invalid value for Channel element SecurityProtocol. This must be an int type, with the flag values found in {typeof(EmaConfig.EncryptedTLSProtocolFlags).FullName}.");

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);
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
                NodeParser serverNodeParser = new("Server", serverListNode);
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

                serverNodeParser
                    .Parse("ConnectionMinPingTimeout", () => tmpConfig.BindOptions.MinPingTimeout, pingTimeout => pingTimeout >= 1000 ? pingTimeout / 1000 : 60)
                    .Parse("ConnectionPingTimeout", () => tmpConfig.BindOptions.PingTimeout, pingTimeout => pingTimeout >= 1000 ? pingTimeout / 1000 : 60)
                    .Parse("CompressionThreshold",
                        (uint v) =>
                        {
                            tmpConfig.CompressionThreshold = Utilities.Convert_uint_int(v);
                            tmpConfig.CompressionThresholdSet = true;
                        }, uint.TryParse, CorrectUnsignedNumericMessage)
                    // CompressionType enumeration
                    .Parse(() => tmpConfig.BindOptions.CompressionType,
                        CreatePrefixedValueParser("CompressionType", ClientChannelConfig.StringToCompressionType),
                        "Invalid CompressionType string format. Correct format is \"CompressionType::<None/ZLib/LZ4>\".")
                    .Parse(() => tmpConfig.DirectWrite)
                    .Parse(() => tmpConfig.BindOptions.GuaranteedOutputBuffers)
                    .Parse(() => tmpConfig.HighWaterMark)
                    .Parse(() => tmpConfig.InitializationTimeout)
                    .Parse(() => tmpConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout)
                    .Parse(() => tmpConfig.BindOptions.InterfaceName)
                    .Parse(() => tmpConfig.BindOptions.MaxFragmentSize)
                    .Parse(() => tmpConfig.BindOptions.NumInputBuffers)
                    .Parse("Port", () => tmpConfig.BindOptions.ServiceName)

                    // ServerType string: This will remove the "ServerType::" prepend and call StringToConnectionType
                    .Parse("ServerType", () => tmpConfig.BindOptions.ConnectionType,
                        CreatePrefixedValueParser("ServerType", ClientChannelConfig.StringToConnectionType),
                        "Invalid ServerType string format. Correct format is \"ServerType::<RSSL_SOCKET or RSSL_ENCRYPTED>\".")
                    .Parse(() => tmpConfig.BindOptions.SysRecvBufSize)
                    .Parse(() => tmpConfig.BindOptions.SysSendBufSize)
                    .Parse("TcpNodelay", () => tmpConfig.BindOptions.TcpOpts.TcpNoDelay)
                    .Parse("ServerCert", () => tmpConfig.BindOptions.BindEncryptionOpts.ServerCertificate)
                    .Parse(() => tmpConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey)

                    // SecurityProtocol enum
                    .Parse("SecurityProtocol", () => tmpConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags, 
                        TryParseEncryptionProtocolFlags, 
                        $"Invalid value for Channel element SecurityProtocol. This must be an int type, with the flag values found in {typeof(EmaConfig.EncryptedTLSProtocolFlags).FullName}.")

                    // CipherSuite This is a comma separated string with either the names or integer values of ciphers specified in System.Net.Security.TlsCipherSuite
                    .Parse("CipherSuite", str => tmpConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites = ServerConfig.StringToCipherList(str, configError));

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);
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
                NodeParser loggerParser = new NodeParser("Logger", loggerListNode);
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

                loggerParser.
                    Parse(() => tmpConfig.FileName)
                    .Parse(() => tmpConfig.IncludeDateInLoggerOutput)
                    .Parse(() => tmpConfig.NumberOfLogFiles)
                    .Parse(() => tmpConfig.MaxLogFileSize)
                    .Parse(() => tmpConfig.LoggerSeverity,
                            CreatePrefixedValueParser("LoggerSeverity",
                                    LoggerConfig.StringToLoggerLevel),
                                    "Invalid LoggerSeverity string format. " +
                                    "Correct format is \"LoggerSeverity::<Trace/Debug/Info " +
                                    "or Success/Warning/Error or Verbose/NoLogMsg>\".")
                    .Parse(() => tmpConfig.LoggerType,
                            CreatePrefixedValueParser("LoggerType",
                                    LoggerConfig.StringToLoggerType),
                                    "Invalid LoggerType string format. " +
                                    "Correct format is \"LoggerType::<File/Stdout>\".");

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);
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
                NodeParser dictionaryNodeParser = new NodeParser("Dictionary", dictionaryListNode);
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

                dictionaryNodeParser
                    .Parse(() => tmpConfig.EnumTypeDefFileName)
                    .Parse(() => tmpConfig.EnumTypeDefItemName)
                    .Parse(() => tmpConfig.RdmFieldDictionaryFileName)
                    .Parse(() => tmpConfig.RdmFieldDictionaryItemName)
                    .Parse("DictionaryType", 
                        (int dictionaryType) => 
                        {
                            tmpConfig.DictionaryType = dictionaryType;
                            tmpConfig.IsLocalDictionary = dictionaryType == EmaConfig.DictionaryTypeEnum.FILE;
                        }, CreatePrefixedValueParser(
                            "DictionaryType",
                            DictionaryConfig.StringToDictionaryMode),
                        "Invalid DictionaryType string format. Correct format is \"DictionaryType::<FileDictionary/ChannelDictionary>\".");

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);
            }
        }

        private void ParseSourceDirectoryGroup(XmlNode DirectoryNode, Dictionary<string, DirectoryConfig> configMap, bool niProvider,
            ConfigErrorList configError, out String defaultDirectoryName, out string firstDirectoryName)
        {
            CurrNode = DirectoryNode.SelectSingleNode("DirectoryList");
            defaultDirectoryName = string.Empty;
            firstDirectoryName = string.Empty;
            XmlNode? serviceNode;
            HashSet<int> serviceIds = new();
            int generateServiceID;
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

                                    if (tmpServiceConfig.Service.ServiceId > ushort.MaxValue)
                                    {
                                        throw new OmmInvalidConfigurationException(
                                        $"service[{tmpServiceConfig.Service.Info.ServiceName}] specifies out of range ServiceId ({tmpServiceConfig.Service.ServiceId}).");
                                    }

                                    if (serviceIds.Contains(tmpServiceConfig.Service.ServiceId))
                                    {
                                        throw new OmmInvalidConfigurationException(
                                       $"service[{tmpServiceConfig.Service.Info.ServiceName}] specifies the same ServiceId (value of {tmpServiceConfig.Service.ServiceId}) as already specified by another service.");
                                    }

                                    setServiceId = true;
                                    serviceIds.Add(tmpServiceConfig.Service.ServiceId);
                                }
                                catch (SystemException)
                                {
                                    throw new OmmInvalidConfigurationException(                                        IncorrectFormatMessage("Directory Service ServiceId") + CorrectUnsignedNumericMessage);
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
                        }

                        if (foundServiceConfig == false)
                        {
                            if (!setServiceId)
                            {
                                while (serviceIds.Contains(generateServiceID))
                                {
                                    ++generateServiceID;
                                }

                                if (generateServiceID > ushort.MaxValue)
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
                    }
                }

                if (foundConfig == false)
                    configMap.Add(tmpConfig.Name, tmpConfig);
            }
        }

        private static void ParseXmlTraceConfigNodes(NodeParser nodeParser, XmlTraceConfigurable configImpl)
        {
            // XmlTraceToStdout bool
            nodeParser.Parse<bool>("XmlTraceToStdout", v => configImpl.XmlTraceToStdout = v, TryParseBoolnumeric, CorrectBooleanFormatMessage).
            // XmlTraceToFile bool
            Parse<bool>("XmlTraceToFile", v => configImpl.XmlTraceToFile = v, TryParseBoolnumeric, CorrectBooleanFormatMessage).
            // XmlTraceMaxFileSize ulong
            Parse<ulong>("XmlTraceMaxFileSize", v => configImpl.XmlTraceMaxFileSize = v, ulong.TryParse, "Correct format is an unsigned numeric string").
            // XmlTraceFileName string
            Parse("XmlTraceFileName", v => configImpl.XmlTraceFileName = v).
            // XmlTraceToMultipleFiles bool
            Parse<bool>("XmlTraceToMultipleFiles", v => configImpl.XmlTraceToMultipleFiles = v, TryParseBoolnumeric, CorrectBooleanFormatMessage).
            // XmlTraceWrite bool
            Parse<bool>("XmlTraceWrite", v => configImpl.XmlTraceWrite = v, TryParseBoolnumeric, CorrectBooleanFormatMessage).
            // XmlTraceRead bool
            Parse<bool>("XmlTraceRead", v => configImpl.XmlTraceRead = v, TryParseBoolnumeric, CorrectBooleanFormatMessage).
            // XmlTracePing bool
            Parse<bool>("XmlTracePing", v => configImpl.XmlTracePing = v, TryParseBoolnumeric, CorrectBooleanFormatMessage);
        }

        // Load and validate XML configuration document
        private static XmlDocument LoadXmlConfig(string? configFilePath)
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
            try
            {
                XmlSchema ? schema = XmlSchema.Read(new StringReader(Properties.Resources.EmaConfigSchema), null);

                if (schema != null)
                {
                    ConfigXml.Schemas.Add(schema);
                    ConfigXml.Validate(null);
                }
            }
            catch (XmlException ex)
            {
                throw new OmmInvalidConfigurationException(
                    $"XML Configuration validation failed: {ex.Message}");
            }
            catch (XmlSchemaValidationException ex)
            {
                throw new OmmInvalidConfigurationException(
                    $"Error validating XML configuration: {ex.Message}");
            }
            catch (XmlSchemaException ex)
            {
                throw new OmmInvalidConfigurationException(
                    $"XML Schema is not valid: {ex.Message} at line {ex.LineNumber}");
            }

            return ConfigXml;
        }

        private readonly static TryParseDelegate<bool> TryParseBoolnumeric = CombineTryParseWithConvert<ulong, bool>(ulong.TryParse, v => v != 0);

        delegate bool TryParseDelegate<TValue>(string s, out TValue value);
        delegate bool TryConvertDelegate<T, TResult>(T v, out TResult value);

        private static TryParseDelegate<T> CombineTryParseWithTransform<T>(TryParseDelegate<T> tryParse, Func<T, T>? transform) =>
                CombineTryParseWithConvert(tryParse, transform ?? ((T _) => _));

        private static TryParseDelegate<TResult> CombineTryParseWithConvert<T, TResult>(TryParseDelegate<T> tryParse, Func<T, TResult> convert) =>
         CombineTryParseWithTryConvert(tryParse, (T v, out TResult value) =>
         {
             value = convert(v);
             return true;
         });

        private static TryParseDelegate<TResult> CombineTryParseWithTryConvert<T, TResult>(TryParseDelegate<T> tryParse, TryConvertDelegate<T, TResult> tryConvert) =>
        (string s, out TResult value) =>
        {
            value = default!;
            if (!tryParse(s, out var intermediateVal))
                return false;

            if (!tryConvert(intermediateVal, out value))
                return false;
            return true;
        };

        private class NodeParser
        {
            private readonly string prefixName;
            private readonly XmlNode parentNode;

            public NodeParser(string prefixName, XmlNode parentNode)
            {
                this.prefixName = prefixName;
                this.parentNode = parentNode;
            }

            public NodeParser Parse(Expression<Func<string>> property)
            {
                Parse(ExtractPropertyName(property), ExtractSetter(property));
                return this;
            }

            public NodeParser Parse(string nodeName, Expression<Func<string>> property)
            {
                Parse(nodeName, ExtractSetter(property));
                return this;
            }

            public NodeParser Parse<T>(Expression<Func<T>> propertyExpression, TryParseDelegate<T> tryParse, string? correctValueFormatMessage)
            {
                Parse(ExtractPropertyName(propertyExpression), propertyExpression, tryParse, correctValueFormatMessage);
                return this;
            }

            public NodeParser Parse<T>(string nodeName, Expression<Func<T>> propertyExpression, TryParseDelegate<T> tryParse, string? correctValueFormatMessage)
            {
                Parse(nodeName, ExtractSetter(propertyExpression), tryParse, correctValueFormatMessage);
                return this;
            }

            public NodeParser Parse(Expression<Func<bool>> propertyExpression)
            {
                Parse(ExtractPropertyName(propertyExpression), propertyExpression);
                return this;
            }

            public NodeParser Parse(string nodeName, Expression<Func<bool>> propertyExpression)
            {
                Parse(nodeName, propertyExpression, TryParseBoolnumeric, IncorrectFormatMessage($"{prefixName} {ExtractPropertyName(propertyExpression)}") + CorrectBooleanFormatMessage);
                return this;
            }

            public NodeParser Parse(Expression<Func<uint>> propertyExpression, Func<uint, uint>? transform = null)
            {
                Parse(ExtractPropertyName(propertyExpression), propertyExpression, transform);
                return this;
            }

            public NodeParser Parse(Expression<Func<int>> propertyExpression, Func<int, int>? transform = null)
            {
                Parse(ExtractPropertyName(propertyExpression), propertyExpression, transform);
                return this;
            }

            public NodeParser Parse(Expression<Func<long>> propertyExpression, Func<long, long>? transform = null)
            {
                Parse(ExtractPropertyName(propertyExpression), propertyExpression, transform);
                return this;
            }

            public NodeParser Parse(Expression<Func<ulong>> propertyExpression, Func<ulong, ulong>? transform = null)
            {
                Parse(ExtractPropertyName(propertyExpression), propertyExpression, transform);
                return this;
            }

            public NodeParser Parse(string nodeName, Expression<Func<uint>> propertyExpression, Func<uint, uint>? transform = null)
            {
                ParseNumeric(nodeName, ExtractSetter(propertyExpression), 
                    CombineTryParseWithTransform(CombineTryParseWithConvert<ulong, uint>(ulong.TryParse, Utilities.Convert_ulong_uint), transform));
                return this;
            }

            public NodeParser Parse(string nodeName, Expression<Func<int>> propertyExpression, Func<int, int>? transform = null)
            {
                ParseNumeric(nodeName, ExtractSetter(propertyExpression), 
                    CombineTryParseWithTransform(int.TryParse, transform));
                return this;
            }

            public NodeParser Parse(string nodeName, Action<int> assign, Func<int, int>? transform = null)
            {
                ParseNumeric(nodeName, assign, 
                    CombineTryParseWithTransform(int.TryParse, transform));
                return this;
            }

            public NodeParser Parse(string nodeName, Expression<Func<long>> propertyExpression, Func<long, long>? transform = null)
            {
                ParseNumeric(nodeName, ExtractSetter(propertyExpression),
                    CombineTryParseWithTransform(long.TryParse, transform));
                return this;
            }

            public NodeParser Parse(string nodeName, Expression<Func<ulong>> propertyExpression, Func<ulong, ulong>? transform = null)
            {
                ParseNumeric(nodeName, ExtractSetter(propertyExpression), 
                    CombineTryParseWithTransform(ulong.TryParse, transform));
                return this;
            }

            private void ParseNumeric<T>(string nodeName, Action<T> assign, TryParseDelegate<T> tryParse) =>
                 Parse(nodeName, assign, tryParse, IncorrectFormatMessage($"{prefixName} {nodeName}") + CorrectUnsignedNumericMessage);

            public NodeParser Parse(string nodeName, Action<string> assign)
            {
                Parse(
                    nodeName,
                    assign,
                    (string s, out string value) =>
                    {
                        value = s;
                        return true;
                    },
                    null);
                return this;
            }

            public NodeParser Parse<TValue>(
                string nodeName,
                Action<TValue> assign,
                TryParseDelegate<TValue> tryParse,
                string? correctValueFormatMessage)
            {
                var currNode = parentNode.SelectSingleNode(nodeName);
                if (currNode == null)
                    return this;

                var valueAttr = currNode.Attributes!["value"];

                if (valueAttr == null)
                    throw new OmmInvalidConfigurationException($"Missing value attribute in the {prefixName} {nodeName} element");

                if (!tryParse(valueAttr.Value, out var parsedValue))
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {prefixName} {nodeName}  element is incorrectly formatted." +
                        (!string.IsNullOrEmpty(correctValueFormatMessage)
                            ? " " + correctValueFormatMessage
                            : string.Empty));

                assign(parsedValue);
                return this;
            }

            private static Action<T> ExtractSetter<T>(Expression<Func<T>> propertyExpression)
            {
                if (propertyExpression.Body is MemberExpression memberExpression)
                {
                    if (memberExpression.Member is PropertyInfo propertyInfo)
                    {
                        var objectExpression = memberExpression.Expression;
                        if (objectExpression is not null)
                        {
                            var compiledLambda = Expression.Lambda(objectExpression).Compile();
                            var obj = compiledLambda.DynamicInvoke();
                            return v => propertyInfo.SetValue(obj, v);
                        }
                    }
                }
                throw new ArgumentException($"Invalid expression: {propertyExpression.Body}. It needs to be a property.");
            }

            private static string ExtractPropertyName<T>(Expression<Func<T>> propertyExpression)
            {
                if (propertyExpression.Body is MemberExpression memberExpression)
                {
                    if (memberExpression.Member is PropertyInfo propertyInfo)
                    {
                        return propertyInfo.Name;
                    }
                }
                throw new ArgumentException($"Invalid expression: {propertyExpression.Body}. It needs to be a property.");
            }
        }

        private void ParseSessionChannelGroup(XmlNode DictionaryNode, Dictionary<string, SessionChannelConfig> configMap, ConfigErrorList configError)
        {
            CurrNode = DictionaryNode.SelectSingleNode("SessionChannelList");

            if (CurrNode == null)
            {
                throw new OmmInvalidConfigurationException("Missing SessionChannelList element");

            }

            CurrNodeList = CurrNode.SelectNodes("SessionChannelInfo");

            if (CurrNodeList == null)
            {
                throw new OmmInvalidConfigurationException("Missing SessionChannelInfo element");
            }

            foreach (XmlNode sessionChannelInfoNode in CurrNodeList)
            {
                bool foundConfig = false;
                SessionChannelConfig tmpConfig;

                NodeParser sessionChannelInfoNodeParser = new NodeParser("SessionChannelInfo", sessionChannelInfoNode);

                // Name string, this is required
                if (HandleSingleStringNode(sessionChannelInfoNode, "SessionChannelInfo", "Name", out string? sessionChannelName))
                {
                    if (configMap.ContainsKey(sessionChannelName))
                    {
                        tmpConfig = configMap[sessionChannelName];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new SessionChannelConfig();
                        tmpConfig.Name = sessionChannelName;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing SessionChannelInfo Name element");
                }

                /* Added checking to ensure that these parameters are set as Consumer's parameters can be used instead. */
                if (sessionChannelInfoNode.SelectSingleNode("ReconnectAttemptLimit") != null)
                {
                    sessionChannelInfoNodeParser.Parse(() => tmpConfig.ReconnectAttemptLimit);
                    tmpConfig.ReconnectAttemptLimitSet = true;
                }

                if (sessionChannelInfoNode.SelectSingleNode("ReconnectMaxDelay") != null)
                {
                    sessionChannelInfoNodeParser.Parse(() => tmpConfig.ReconnectMaxDelay);
                    tmpConfig.ReconnectMaxDelaySet = true;
                }

                if (sessionChannelInfoNode.SelectSingleNode("ReconnectMinDelay") != null)
                {
                    sessionChannelInfoNodeParser.Parse(() => tmpConfig.ReconnectMinDelay);
                    tmpConfig.ReconnectMinDelaySet = true;
                }

                HandleSingleListNode(sessionChannelInfoNode, "SessionChannelInfo", "ChannelSet",  tmpConfig.ChannelSet);

                if (!foundConfig)
                {
                    configMap.Add(tmpConfig.Name, tmpConfig);
                }
            }
        }

        #region Helper methods

        internal bool HandleSingleStringNode(XmlNode groupNode, string groupName, string nodeName, [NotNullWhen(true)]out string? nodeValue)
        {
            CurrNode2 = groupNode.SelectSingleNode(nodeName);
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} {nodeName} element");
                }

                nodeValue = XmlAttribute.Value;
                return true;
            }
            nodeValue = null;
            return false;
        }

        internal bool HandleSingleULongNode(XmlNode groupNode, string groupName, string nodeName, out ulong nodeValue)
        {
            CurrNode2 = groupNode.SelectSingleNode(nodeName);
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} {nodeName} element");
                }

                if (ulong.TryParse(XmlAttribute.Value, out nodeValue))
                    return true;
                else
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} {nodeName} element is incorrectly formatted. Correct format is an unsigned numeric string.");
            }
            nodeValue = default;
            return false;
        }

        internal bool HandleSingleLongNode(XmlNode groupNode, string groupName, string nodeName, out long nodeValue)
        {
            CurrNode2 = groupNode.SelectSingleNode(nodeName);
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} {nodeName} element");
                }

                if (long.TryParse(XmlAttribute.Value, out nodeValue))
                    return true;
                else
                    throw new OmmInvalidConfigurationException(
                        $"The value attribute in the {groupName} {nodeName} element is incorrectly formatted. Correct format is a numeric string.");
            }
            nodeValue = default;
            return false;
        }

        internal bool HandleSingleListNode(XmlNode groupNode, string groupName, string nodeName, List<string> nodeValue)
        {
            CurrNode2 = groupNode.SelectSingleNode(nodeName);
            if (CurrNode2 != null)
            {
                ValueNode = (XmlElement)CurrNode2;
                XmlAttribute = ValueNode.GetAttributeNode("value");

                if (XmlAttribute == null)
                {
                    throw new OmmInvalidConfigurationException(
                        $"Missing value attribute in the {groupName} {nodeName} element");
                }

                nodeValue.Clear();

                string[] valuesArray = XmlAttribute.Value.Split(',');

                foreach (string strValue in valuesArray)
                    nodeValue.Add(strValue.Trim());

                return true;
            }

            return false;
        }

        #endregion
    }
}
