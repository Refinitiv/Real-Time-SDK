/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.Linq;

namespace LSEG.Ema.Access
{
    // This class will, upon creation from OmmConsumerConfigImpl/OmmProviderImpl/OmmNiProviderImpl, parse the either the default Xml file, or the given file from the OmmConfig's XmlConfigPath.
    // When created from the copy constructor in OmmBaseImpl, this class will represent the full config for the configured singular consumer that EMA will use to create an OmmConsumer, OmmProvider, or OmmNiProvider.
    internal class XmlConfigParser
    {

        private XmlDocument ConfigXml { get; set; }
        private OmmConsumerConfigImpl OmmConfig { get; set; }

        // Reusable XmlNode references
        private XmlNode? CurrNode;
        private XmlElement? ValueNode;
        private XmlNodeList? CurrNodeList;
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
            "CatchUnhandledException",
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
            "XmlTraceDump",
            "XmlTraceToStdout",
            "XmlTraceFileName",
            // Channel related strings
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
            "DictionaryType"
        };
        // Xml Configuration parser for an OmmConsumerConfig.
        // This method looks for the following configuration groups and parses them:
        // ConsumerGroup
        // LoggerGroup
        // ChannelGroup
        // LoggerGroup
        // Error conditions from this method: configured file not found, Xml parsing errors.
        internal XmlConfigParser(OmmConsumerConfigImpl Config)
        {
            XmlNode? ConfigNode;
            OmmConfig = Config;
            ConfigXml = new XmlDocument();
            ConfigXml.PreserveWhitespace = true;

            try
            {
                if (string.IsNullOrEmpty(OmmConfig.XmlConfigPath) == true)
                {
                    ConfigXml.Load("EmaConfig.xml");
                }
                else
                {
                    ConfigXml.Load(OmmConfig.XmlConfigPath);
                }
            }
            catch (System.IO.FileNotFoundException excp)
            {
                // If the path is set to null, the application could be using the default config or a programmatic config, so just return without adding any info to the Config.
                if (string.IsNullOrEmpty(OmmConfig.XmlConfigPath) == true)
                    return;
                else
                {
                    throw new OmmInvalidConfigurationException("Could not load the configured XML file. FileNotFoundException text: " + excp.Message);
                }
            }
            catch (System.Xml.XmlException excp)
            {
                throw new OmmInvalidConfigurationException("Error parsing XML file. XmlException text: " + excp.Message);
            }

            if (ConfigXml.DocumentElement == null)
            {
                throw new OmmInvalidConfigurationException("XML Parsing failed.");
            }

            if (ConfigXml.DocumentElement.Name != "EmaConfig")
            {
                throw new OmmInvalidConfigurationException("Error parsing XML file. Top severity node is not set to \"EmaConfig\"");
            }

            ConfigNode = ConfigXml.DocumentElement;

            XmlNode? GroupNode = ConfigNode.SelectSingleNode("ConsumerGroup");
            if (GroupNode != null)
            {
                // Parse the Consumer Group here
                ParseConsumerGroup(GroupNode);
            }

            GroupNode = ConfigNode.SelectSingleNode("ChannelGroup");
            if (GroupNode != null)
            {
                // Parse the channel group here
                ParseClientChannelGroup(GroupNode);
            }

            GroupNode = ConfigNode.SelectSingleNode("LoggerGroup");
            if (GroupNode != null)
            {
                // Parse the Logger group here
                ParseLoggerGroup(GroupNode);
            }

            GroupNode = ConfigNode.SelectSingleNode("DictionaryGroup");
            if (GroupNode != null)
            {
                // Parse the Dictionary group here
                ParseDictionaryGroup(GroupNode);
            }

            // Now iterate through the entirety of the XML and find any elements that aren't available 
            foreach (XmlNode node in ConfigNode)
            {
                if (node.NodeType == XmlNodeType.Element && !xmlNodeNames.Contains(node.Name))
                {
                    OmmConfig.ConfigErrorLog?.Add("Unknown XML element: " + node.Name, LoggerLevel.ERROR);
                }
            }
        }

        private void ParseConsumerGroup(XmlNode ConsumerNode)
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
                OmmConfig.DefaultConsumer = XmlAttribute.Value;
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer Name element");
                    }

                    if (OmmConfig.ConsumerConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = OmmConfig.ConsumerConfigMap[XmlAttribute.Value];
                        foundConfig = true;
                    }
                    else
                    {
                        tmpConfig = new ConsumerConfig();
                        tmpConfig.Name = XmlAttribute.Value;
                    }
                }
                else
                {
                    throw new OmmInvalidConfigurationException("Missing Name element in the Consumer Name");
                }

                // CatchUnhandledException ulong
                CurrNode2 = consumerListNode.SelectSingleNode("CatchUnhandledException");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer CatchUnhandledException element");
                    }

                    try
                    {
                        tmpConfig.CatchUnhandledException = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch(SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer CatchUnhandledException element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
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
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer CatchUnhandledException element is incorrectly formatted. Correct format is an unsigned numeric string.");
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
                        tmpConfig.ItemCountHint = Utilities.Convert_ulong_uint(ulong.Parse(XmlAttribute.Value));
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
                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer MaxDispatchCountApiThread element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // MaxDispatchCountUserThread ulong
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
                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
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
                        tmpConfig.ReconnectAttemptLimit = Utilities.Convert_long_int(long.Parse(XmlAttribute.Value));
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
                        tmpConfig.ReconnectMaxDelay = Utilities.Convert_long_int(long.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ReconnectMaxDelay element is incorrectly formatted. Correct format is a signed numeric string.");
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
                        tmpConfig.ReconnectMinDelay = Utilities.Convert_long_int(long.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ReconnectMinDelay element is incorrectly formatted. Correct format is a signed numeric string.");
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
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer RestRequestTimeOut element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
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
                        tmpConfig.ServiceCountHint = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer ServiceCountHint element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // XmlTraceDump ulong
                CurrNode2 = consumerListNode.SelectSingleNode("XmlTraceDump");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer XmlTraceDump element");
                    }
                    
                    try
                    {
                        tmpConfig.XmlTraceDump = ulong.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer XmlTraceDump element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // XmlTraceToStdout bool
                CurrNode2 = consumerListNode.SelectSingleNode("XmlTraceToStdout");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer XmlTraceToStdout element");
                    }

                    try
                    {
                        tmpConfig.XmlTraceToStdout = (ulong.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Consumer XmlTraceToStdout element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
                    }
                }

                // XmlTraceFileName string
                CurrNode2 = consumerListNode.SelectSingleNode("XmlTraceFileName");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Consumer XmlTraceFileName element");
                    }
                    tmpConfig.XmlTraceFileName = XmlAttribute.Value;
                }

                if (foundConfig == false)
                    OmmConfig.ConsumerConfigMap.Add(tmpConfig.Name, tmpConfig);
            }
        }

        private void ParseClientChannelGroup(XmlNode ChannelNode)
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel Name element");
                    }

                    if (OmmConfig.ClientChannelConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = OmmConfig.ClientChannelConfigMap[XmlAttribute.Value];
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

                    tmpConfig.ConnectInfo.ConnectOptions.ConnectionType = ChannelInformation.EmaToEtaConnectionType(ClientChannelConfig.StringToConnectionType(channelArray[1]));
                }

                // ChannelType string: This will remove the "ChannelType::" prepend and call StringToConnectionType
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

                    tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol = ChannelInformation.EmaToEtaConnectionType(ClientChannelConfig.StringToConnectionType(channelArray[1]));
                }

                // XmlTraceDump int
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
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel ConnectionPingTimeout element is incorrectly formatted. Correct format is a signed numeric string.");
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
                        tmpConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
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
                        tmpConfig.HighWaterMark = Utilities.Convert_long_int(long.Parse(XmlAttribute.Value));
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
                        tmpConfig.ConnectInfo.SetInitTimeout(int.Parse(XmlAttribute.Value));
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
                        tmpConfig.ConnectInfo.ConnectOptions.NumInputBuffers = int.Parse(XmlAttribute.Value);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel NumInputBuffers element is incorrectly formatted. Correct format is an unsigned numeric string.");
                    }
                }

                // ServiceDiscoveryRetryCount uint parsed as int
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
                        tmpConfig.ConnectInfo.ServiceDiscoveryRetryCount = int.Parse(XmlAttribute.Value);
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
                        tmpConfig.ConnectInfo.ConnectOptions.SysRecvBufSize = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
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
                        tmpConfig.ConnectInfo.ConnectOptions.SysSendBufSize = Utilities.Convert_ulong_int(ulong.Parse(XmlAttribute.Value));
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

                    tmpConfig.ConnectInfo.ConnectOptions.CompressionType = ChannelInformation.EmaToEtaCompressionType(ClientChannelConfig.StringToCompressionType(channelArray[1]));

                }

                // DirectWrite uint
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
                        tmpConfig.DirectWrite = (uint.Parse(XmlAttribute.Value) != 0);
                    }
                    catch (SystemException)
                    {
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel SysSendBufSize element is incorrectly formatted. Correct values are: \"0\" or \"1\".");
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

                // TcpNodelay int->bool
                CurrNode2 = channelListNode.SelectSingleNode("TcpNodelay");
                if (CurrNode2 != null)
                {
                    ValueNode = (XmlElement)CurrNode2;
                    XmlAttribute = ValueNode.GetAttributeNode("value");

                    if (XmlAttribute == null)
                    {
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Channel TcpNodelay element");
                    }

                    tmpConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay = (int.Parse(XmlAttribute.Value) == 1);
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
                        throw new OmmInvalidConfigurationException("The value attribute in the Channel SysSendBufSize element is incorrectly formatted. Correct values are combinations of flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags");
                    }

                    if (tmpInt != 0 && (tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
                    {
                        throw new OmmInvalidConfigurationException(
                            "Invalid value for Channel element SecurityProtocol. This must be an int type, with the flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags.");
                    }

                    tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags = (EncryptionProtocolFlags)(tmpInt & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL);
                }

                if (foundConfig == false)
                    OmmConfig.ClientChannelConfigMap.Add(tmpConfig.Name, tmpConfig);
            }
        }

        private void ParseLoggerGroup(XmlNode LoggerNode)
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger Name element");
                    }

                    if (OmmConfig.LoggerConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = OmmConfig.LoggerConfigMap[XmlAttribute.Value];
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger FileName element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger IncludeDateInLoggerOutput element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger NumberOfLogFiles element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger MaxLogFileSize element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger LoggerSeverity element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid LoggerSeverity string format. Correct format is \"LoggerSeverity::<Trace/Debug/Info or Success/Warning/Error or Verbose/NoLogMsg>\".");

                    }

                    if (channelArray[0] != "LoggerSeverity")
                    {
                        throw new OmmInvalidConfigurationException("Invalid LoggerSeverity string format. Correct format is \"LoggerSeverity::<Trace/Debug/Info or Success/Warning/Error or Verbose/NoLogMsg>\".");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Logger LoggerType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid LoggerType string format. Correct format is \"LoggerType::<File/Stdout>\".");
                    }

                    if (channelArray[0] != "LoggerType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid LoggerType string format. Correct format is \"LoggerType::<File/Stdout>\".");
                    }

                    tmpConfig.LoggerType = LoggerConfig.StringToLoggerType(channelArray[1]);

                }

                if (foundConfig == false)
                    OmmConfig.LoggerConfigMap.Add(tmpConfig.Name, tmpConfig);


            }
        }

        private void ParseDictionaryGroup(XmlNode DictionaryNode)
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Dictionary Name element");
                    }

                    if (OmmConfig.DictionaryConfigMap.ContainsKey(XmlAttribute.Value))
                    {
                        tmpConfig = OmmConfig.DictionaryConfigMap[XmlAttribute.Value];
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Dictionary EnumTypeDefFileName element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Dictionary EnumTypeDefItemName element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Dictionary RdmFieldDictionaryFileName element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Dictionary RdmFieldDictionaryItemName element");
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
                        throw new OmmInvalidConfigurationException("Missing value attribute in the Dictionary DictionaryType element");
                    }

                    string[] channelArray = XmlAttribute.Value.Split("::");

                    if (channelArray.Length != 2)
                    {
                        throw new OmmInvalidConfigurationException("Invalid DictionaryType string format. Correct format is \"DictionaryType::<FileDictionary/ChannelDictionary>\".");
                    }

                    if (channelArray[0] != "DictionaryType")
                    {
                        throw new OmmInvalidConfigurationException("Invalid DictionaryType string format. Correct format is \"DictionaryType::<FileDictionary/ChannelDictionary>\".");
                    }

                    tmpConfig.DictionaryType = DictionaryConfig.StringToDictionaryMode(channelArray[1]);
                    tmpConfig.IsLocalDictionary = (tmpConfig.DictionaryType == EmaConfig.DictionaryTypeEnum.FILE);
                }

                if (foundConfig == false)
                    OmmConfig.DictionaryConfigMap.Add(tmpConfig.Name, tmpConfig);
            }
        }
    }
}