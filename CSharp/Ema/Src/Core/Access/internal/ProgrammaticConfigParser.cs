/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using LSEG.Eta.Codec;
using LSEG.Eta.Transports;

using static LSEG.Ema.Access.Data;

namespace LSEG.Ema.Access
{
    internal class ProgrammaticConfigParser
    {

        // The programmatic configuration overwrites all XML file-based configuration
        internal static void ParseProgrammaticConsumerConfig(Map configMap, OmmConsumerConfigImpl consumerConfig)
        {

            CodecReturnCode ret;
            if (DataTypes.ASCII_STRING != configMap.KeyType())
            {
                throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration. KeyType must be ASCII_STRING");
            }

            var buffer = configMap!.Encoder!.m_encodeIterator!.Buffer();
            if ((ret = configMap.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null)) !=
                CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidConfigurationException("Unable to decode the Map. Error code is:" + ret.GetAsString());
            }

            foreach (MapEntry entry in configMap)
            {
                if (entry.Load == null || DataTypes.ELEMENT_LIST != entry.LoadType)
                {
                    throw new OmmInvalidConfigurationException("Invalid entry payload type for Programmatic Configuration. Top map's entries must contain ELEMENT_LIST");
                }

                switch (entry.Key.Ascii().ToString())
                {
                    case "ConsumerGroup":
                        ParseConsumerGroup((ElementList)entry.Load, consumerConfig);
                        break;
                    case "ChannelGroup":
                        ParseClientChannelGroup((ElementList)entry.Load, consumerConfig.ClientChannelConfigMap, consumerConfig.ConfigErrorLog);
                        break;
                    case "LoggerGroup":
                        ParseLoggerGroup((ElementList)entry.Load, consumerConfig.LoggerConfigMap, consumerConfig.ConfigErrorLog);
                        break;
                    case "DictionaryGroup":
                        ParseDictionaryGroup((ElementList)entry.Load, consumerConfig.DictionaryConfigMap, consumerConfig.ConfigErrorLog);
                        break;
                    default:
                        consumerConfig.ConfigErrorLog?.Add("Unknown Group element: " + entry.Key.Ascii().ToString(), LoggerLevel.ERROR);
                        break;
                }
            }
        }

        internal static void ParseProgrammaticNiProviderConfig(Map configMap, OmmNiProviderConfigImpl providerConfig)
        {

            CodecReturnCode ret;

            if (DataTypes.ASCII_STRING != configMap.KeyType())
            {
                throw new OmmInvalidConfigurationException("Invalid key type for NIProvider Programmatic Configuration. KeyType must be ASCII_STRING");
            }

            var buffer = configMap!.Encoder!.m_encodeIterator!.Buffer();
            if ((ret = configMap.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null)) !=
                CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidConfigurationException("Unable to decode the Map. Error code is:" + ret.GetAsString());
            }

            foreach (MapEntry entry in configMap)
            {
                if (entry.Load == null || DataTypes.ELEMENT_LIST != entry.LoadType)
                {
                    throw new OmmInvalidConfigurationException("Invalid entry payload type for NIProvider programmatic configuration. Top map's entries must contain ELEMENT_LIST");
                }

                switch (entry.Key.Ascii().ToString())
                {
                    case "NiProviderGroup":
                        ParseNiProviderGroup((ElementList)entry.Load, providerConfig);
                        break;
                    case "ChannelGroup":
                        ParseClientChannelGroup((ElementList)entry.Load, providerConfig.ClientChannelConfigMap, providerConfig.ConfigErrorLog);
                        break;
                    case "LoggerGroup":
                        ParseLoggerGroup((ElementList)entry.Load, providerConfig.LoggerConfigMap, providerConfig.ConfigErrorLog);
                        break;
                    case "DictionaryGroup":
                        ParseDictionaryGroup((ElementList)entry.Load, providerConfig.DictionaryConfigMap, providerConfig.ConfigErrorLog);
                        break;
                    case "DirectoryGroup":
                        string defaultDirectoryName;
                        ParseDirectoryGroup((ElementList)entry.Load, out defaultDirectoryName, providerConfig.DirectoryConfigMap, true, providerConfig.ConfigErrorLog);
                        if(string.IsNullOrEmpty(defaultDirectoryName))
                        {
                            providerConfig.DefaultDirectory = defaultDirectoryName;
                        }
                        break;
                    default:
                        providerConfig.ConfigErrorLog?.Add("Unknown Group element: " + entry.Key.Ascii().ToString(), LoggerLevel.ERROR);
                        break;
                }
            }
        }

        internal static void ParseProgrammaticIProviderConfig(Map configMap, OmmIProviderConfigImpl providerConfig)
        {

            CodecReturnCode ret;

            if (DataTypes.ASCII_STRING != configMap.KeyType())
            {
                throw new OmmInvalidConfigurationException("Invalid key type for IProvider Programmatic Configuration. KeyType must be ASCII_STRING");
            }

            var buffer = configMap!.Encoder!.m_encodeIterator!.Buffer();
            if ((ret = configMap.Decode(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null)) !=
                CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidConfigurationException("Unable to decode the IProvider Map. Error code is:" + ret.GetAsString());
            }

            foreach (MapEntry entry in configMap)
            {
                if (entry.Load == null || DataTypes.ELEMENT_LIST != entry.LoadType)
                {
                    throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider Programmatic Configuration. Top map's entries must contain ELEMENT_LIST");
                }

                switch (entry.Key.Ascii().ToString())
                {
                    case "IProviderGroup":
                        ParseIProviderGroup((ElementList)entry.Load, providerConfig);
                        break;
                    case "ServerGroup":
                        ParseServerGroup((ElementList)entry.Load, providerConfig.ServerConfigMap, providerConfig.ConfigErrorLog);
                        break;
                    case "LoggerGroup":
                        ParseLoggerGroup((ElementList)entry.Load, providerConfig.LoggerConfigMap, providerConfig.ConfigErrorLog);
                        break;
                    case "DictionaryGroup":
                        ParseDictionaryGroup((ElementList)entry.Load, providerConfig.DictionaryConfigMap, providerConfig.ConfigErrorLog);
                        break;
                    case "DirectoryGroup":
                        string defaultDirectoryName;
                        ParseDirectoryGroup((ElementList)entry.Load, out defaultDirectoryName, providerConfig.DirectoryConfigMap, false, providerConfig.ConfigErrorLog);
                        if (string.IsNullOrEmpty(defaultDirectoryName))
                        {
                            providerConfig.DefaultDirectory = defaultDirectoryName;
                        }
                        break;
                    default:
                        providerConfig.ConfigErrorLog?.Add("Unknown Group element: " + entry.Key.Ascii().ToString(), LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseConsumerGroup(ElementList consumerList, OmmConsumerConfigImpl ommConfig)
        {
            foreach (ElementEntry groupEntry in consumerList)
            {
                switch (groupEntry.Name)
                {
                    case "DefaultConsumer":
                        if (groupEntry.Load == null || groupEntry.Load.Code == DataCode.BLANK || groupEntry.LoadType != DataTypes.ASCII_STRING)
                        {
                            throw new OmmInvalidConfigurationException("Missing or invalid DefaultConsumer. DefaultConsumer must be an ASCII_STRING and cannot be blank");
                        }
                        ommConfig.DefaultConsumer = ((OmmAscii)groupEntry.Load).ToString();
                        break;
                    case "ConsumerList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid ConsumerList. ConsumerList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for ConsumerList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry consumer in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            ConsumerConfig tmpConfig;
                            if (consumer.Load == null || DataTypes.ELEMENT_LIST != consumer.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for ConsumerList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (consumer.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for ConsumerList Map Entry. The Key must not be blank");
                            }

                            string name = consumer.Key.Ascii().ToString();

                            if (ommConfig.ConsumerConfigMap.ContainsKey(name))
                            {
                                tmpConfig = ommConfig.ConsumerConfigMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new ConsumerConfig();
                                tmpConfig.Name = name;
                            }

                            if (string.IsNullOrEmpty(ommConfig.FirstConfiguredConsumerName))
                            {
                                ommConfig.FirstConfiguredConsumerName = tmpConfig.Name;
                            }

                            foreach (ElementEntry consumerEntry in (ElementList)consumer.Load)
                            {
                                bool channelSetFound = false;
                                switch (consumerEntry.Name)
                                {
                                    // Channel string.  Keeping the behavior the same as XML: If ChannelSet is present, that overrides the "Channel", even if it's later in the map.
                                    case "Channel":
                                        CheckElementEntry("consumer", "Channel", DataTypes.ASCII_STRING, consumerEntry);
                                        if(channelSetFound == false)
                                        {
                                            tmpConfig.ChannelSet.Clear();
                                            tmpConfig.ChannelSet.Add(consumerEntry.OmmAsciiValue().ToString());
                                        }
                                        break;
                                    // ChannelSet string containing a comma separated list of Channel names. This will override in all cases.
                                    case "ChannelSet":
                                        CheckElementEntry("consumer", "ChannelSet", DataTypes.ASCII_STRING, consumerEntry);
                                        tmpConfig.ChannelSet.Clear();

                                        string[] channelArray = consumerEntry.OmmAsciiValue().ToString().Split(',');

                                        for (int i = 0; i < channelArray.Length; i++)
                                            tmpConfig.ChannelSet.Add(channelArray[i].Trim());

                                        channelSetFound = true;
                                        break;
                                    // Dictionary string
                                    case "Dictionary":
                                        CheckElementEntry("Consumer", "Dictionary", DataTypes.ASCII_STRING, consumerEntry);
                                        tmpConfig.Dictionary = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // DictionaryRequestTimeOut ulong
                                    case "DictionaryRequestTimeOut":
                                        CheckElementEntry("Consumer", "DictionaryRequestTimeOut", DataTypes.UINT, consumerEntry);

                                        tmpConfig.DictionaryRequestTimeOut = Utilities.Convert_ulong_long(consumerEntry.UIntValue());
                                        break;
                                    // DirectoryRequestTimeOut ulong
                                    case "DirectoryRequestTimeOut":
                                        CheckElementEntry("Consumer", "DirectoryRequestTimeOut", DataTypes.UINT, consumerEntry);

                                        tmpConfig.DirectoryRequestTimeOut = Utilities.Convert_ulong_long(consumerEntry.UIntValue());
                                        break;
                                    // LoginRequestTimeOut ulong
                                    case "LoginRequestTimeOut":
                                        CheckElementEntry("Consumer", "LoginRequestTimeOut", DataTypes.UINT, consumerEntry);

                                        tmpConfig.LoginRequestTimeOut = Utilities.Convert_ulong_long(consumerEntry.UIntValue());
                                        break;
                                    // DispatchTimeoutApiThread long
                                    case "DispatchTimeoutApiThread":
                                        CheckElementEntry("Consumer", "DispatchTimeoutApiThread", DataTypes.INT, consumerEntry);

                                        tmpConfig.DispatchTimeoutApiThread = consumerEntry.IntValue();
                                        break;
                                    // EnableRtt bool
                                    case "EnableRtt":
                                        CheckElementEntry("Consumer", "EnableRtt", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.EnableRtt = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // ItemCountHint ulong
                                    case "ItemCountHint":
                                        CheckElementEntry("Consumer", "ItemCountHint", DataTypes.UINT, consumerEntry);

                                        tmpConfig.ItemCountHint = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());

                                        if(tmpConfig.ItemCountHint == 0)
                                        {
                                            tmpConfig.ItemCountHint = 1024;
                                        }

                                        break;
                                    // Logger string
                                    case "Logger":
                                        CheckElementEntry("Consumer", "Logger", DataTypes.ASCII_STRING, consumerEntry);
                                        tmpConfig.Logger = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // MaxDispatchCountApiThread ulong
                                    case "MaxDispatchCountApiThread":
                                        CheckElementEntry("Consumer", "MaxDispatchCountApiThread", DataTypes.UINT, consumerEntry);

                                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_ulong_int(consumerEntry.UIntValue());
                                        break;
                                    // MaxDispatchCountUserThread ulong
                                    case "MaxDispatchCountUserThread":
                                        CheckElementEntry("Consumer", "MaxDispatchCountUserThread", DataTypes.UINT, consumerEntry);

                                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_ulong_int(consumerEntry.UIntValue());
                                        break;
                                    // MaxOutstandingPosts ulong
                                    case "MaxOutstandingPosts":
                                        CheckElementEntry("Consumer", "MaxOutstandingPosts", DataTypes.UINT, consumerEntry);

                                        tmpConfig.MaxOutstandingPosts = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // MsgKeyInUpdates bool
                                    case "MsgKeyInUpdates":
                                        CheckElementEntry("Consumer", "MsgKeyInUpdates", DataTypes.UINT, consumerEntry);

                                        tmpConfig.MsgKeyInUpdates = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // ObeyOpenWindow bool
                                    case "ObeyOpenWindow":
                                        CheckElementEntry("Consumer", "ObeyOpenWindow", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.ObeyOpenWindow = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // PostAckTimeout uint
                                    case "PostAckTimeout":
                                        CheckElementEntry("Consumer", "PostAckTimeout", DataTypes.UINT, consumerEntry);

                                        tmpConfig.PostAckTimeout = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // ReconnectAttemptLimit int
                                    case "ReconnectAttemptLimit":
                                        CheckElementEntry("Consumer", "ReconnectAttemptLimit", DataTypes.INT, consumerEntry);

                                        tmpConfig.ReconnectAttemptLimit = Utilities.Convert_long_int(consumerEntry.IntValue());
                                        break;
                                    // ReconnectMaxDelay int
                                    case "ReconnectMaxDelay":
                                        {
                                            CheckElementEntry("Consumer", "ReconnectMaxDelay", DataTypes.INT, consumerEntry);

                                            int value = Utilities.Convert_long_int(consumerEntry.IntValue());

                                            if (value > 0)
                                                tmpConfig.ReconnectMaxDelay = value;
                                        }
                                        break;
                                    // ReconnectMinDelay int
                                    case "ReconnectMinDelay":
                                        {
                                            CheckElementEntry("Consumer", "ReconnectMinDelay", DataTypes.INT, consumerEntry);

                                            int value = Utilities.Convert_long_int(consumerEntry.IntValue());

                                            if (value > 0)
                                                tmpConfig.ReconnectMinDelay = value;
                                        }
                                        break;
                                    // RequestTimeout uint
                                    case "RequestTimeout":
                                        CheckElementEntry("Consumer", "RequestTimeout", DataTypes.UINT, consumerEntry);

                                        tmpConfig.RequestTimeout = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // RestEnableLog bool
                                    case "RestEnableLog":
                                        CheckElementEntry("Consumer", "RestEnableLog", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.RestEnableLog = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // RestEnableLogViaCallback bool
                                    case "RestEnableLogViaCallback":
                                        CheckElementEntry("Consumer", "RestEnableLogViaCallback", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.RestEnableLogViaCallback = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // RestLogFileName string
                                    case "RestLogFileName":
                                        CheckElementEntry("Consumer", "RestLogFileName", DataTypes.ASCII_STRING, consumerEntry);
                                        tmpConfig.RestLogFileName = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // RestRequestTimeOut uint
                                    case "RestRequestTimeOut":
                                        CheckElementEntry("Consumer", "RestRequestTimeOut", DataTypes.UINT, consumerEntry);

                                        tmpConfig.RestRequestTimeOut = consumerEntry.UIntValue();
                                        break;
                                    // RestProxyHostName string
                                    case "RestProxyHostName":
                                        CheckElementEntry("Consumer", "RestProxyHostName", DataTypes.ASCII_STRING, consumerEntry);

                                        tmpConfig.RestProxyHostName = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // RestProxyPort string
                                    case "RestProxyPort":
                                        CheckElementEntry("Consumer", "RestProxyPort", DataTypes.ASCII_STRING, consumerEntry);

                                        tmpConfig.RestProxyPort = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // ServiceCountHint uint
                                    case "ServiceCountHint":
                                        CheckElementEntry("Consumer", "ServiceCountHint", DataTypes.UINT, consumerEntry);

                                        tmpConfig.ServiceCountHint = Utilities.Convert_ulong_int(consumerEntry.UIntValue());

                                        if(tmpConfig.ServiceCountHint == 0)
                                        {
                                            tmpConfig.ServiceCountHint = 513;
                                        }

                                        break;

                                    // XmlTraceToStdout bool
                                    case "XmlTraceToStdout":
                                        CheckElementEntry("Consumer", "XmlTraceToStdout", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToStdout = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceToFile bool
                                    case "XmlTraceToFile":
                                        CheckElementEntry("Consumer", "XmlTraceToFile", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToFile = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceToFile string
                                    case "XmlTraceFileName":
                                        CheckElementEntry("Consumer", "XmlTraceFileName", DataTypes.ASCII_STRING, consumerEntry);

                                        tmpConfig.XmlTraceFileName = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // XmlTraceMaxFileSize ulong
                                    case "XmlTraceMaxFileSize":
                                        CheckElementEntry("Consumer", "XmlTraceMaxFileSize", DataTypes.UINT, consumerEntry);

                                        tmpConfig.XmlTraceMaxFileSize = consumerEntry.UIntValue();
                                        break;
                                    // XmlTraceToMultipleFiles bool
                                    case "XmlTraceToMultipleFiles":
                                        CheckElementEntry("Consumer", "XmlTraceToMultipleFiles", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToMultipleFiles = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceWrite bool
                                    case "XmlTraceWrite":
                                        CheckElementEntry("Consumer", "XmlTraceWrite", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceWrite = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceRead bool
                                    case "XmlTraceRead":
                                        CheckElementEntry("Consumer", "XmlTraceRead", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceRead = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // XmlTracePing bool
                                    case "XmlTracePing":
                                        CheckElementEntry("Consumer", "XmlTracePing", DataTypes.UINT, consumerEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTracePing = (consumerEntry.UIntValue() != 0);
                                        break;

                                    default:
                                        ommConfig.ConfigErrorLog?.Add("Unknown Consumer entry element: " + consumerEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            // If this is a new consumer config, add it to the map.
                            if (foundConfig == false)
                            {
                                ommConfig.ConsumerConfigMap.Add(tmpConfig.Name, tmpConfig);
                            }
                        }
                        break;
                    default:
                        ommConfig.ConfigErrorLog?.Add("Unknown Consumer element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseNiProviderGroup(ElementList niProviderList, OmmNiProviderConfigImpl ommConfig)
        {
            foreach (ElementEntry groupEntry in niProviderList)
            {
                switch (groupEntry.Name)
                {
                    case "DefaultNiProvider":
                        if (groupEntry.Load == null || groupEntry.Load.Code == DataCode.BLANK || groupEntry.LoadType != DataTypes.ASCII_STRING)
                        {
                            throw new OmmInvalidConfigurationException("Missing or invalid DefaultNiProvider. DefaultNiProvider must be an ASCII_STRING and cannot be blank");
                        }
                        ommConfig.DefaultNiProvider = ((OmmAscii)groupEntry.Load).ToString();
                        break;
                    case "NiProviderList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid NiProviderList. NiProviderList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for NiProviderList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry niProvider in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            NiProviderConfig tmpConfig;
                            bool channelSetFound = false;
                            if (niProvider.Load == null || DataTypes.ELEMENT_LIST != niProvider.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProviderList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (niProvider.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for NiProviderList Map Entry. The Key must not be blank");
                            }

                            string name = niProvider.Key.Ascii().ToString();

                            if (ommConfig.NiProviderConfigMap.ContainsKey(name))
                            {
                                tmpConfig = ommConfig.NiProviderConfigMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new NiProviderConfig();
                                tmpConfig.Name = name;
                            }

                            if (string.IsNullOrEmpty(ommConfig.FirstConfiguredNiProviderName))
                            {
                                ommConfig.FirstConfiguredNiProviderName = tmpConfig.Name;
                            }

                            foreach (ElementEntry niProviderEntry in (ElementList)niProvider.Load)
                            {
                                switch (niProviderEntry.Name)
                                {
                                    // Channel string.  Keeping the behavior the same as XML: If ChannelSet is present, that overrides the "Channel", even if it's later in the map.
                                    case "Channel":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element Channel. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        if (channelSetFound == false)
                                        {
                                            tmpConfig.ChannelSet.Clear();
                                            tmpConfig.ChannelSet.Add(niProviderEntry.OmmAsciiValue().ToString());
                                        }
                                        break;
                                    // ChannelSet string containing a comma separated list of Channel names. This will override in all cases.
                                    case "ChannelSet":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element ChannelSet. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ChannelSet.Clear();

                                        string[] channelArray = niProviderEntry.OmmAsciiValue().ToString().Split(',');

                                        for (int i = 0; i < channelArray.Length; i++)
                                            tmpConfig.ChannelSet.Add(channelArray[i].Trim());

                                        channelSetFound = true;
                                        break;
                                    // Dictionary string
                                    case "Directory":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element Directory. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Directory = niProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // DispatchTimeoutApiThread long
                                    case "DispatchTimeoutApiThread":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element DispatchTimeoutApiThread. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.DispatchTimeoutApiThread = niProviderEntry.IntValue();
                                        break;
                                    // ItemCountHint ulong
                                    case "ItemCountHint":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element ItemCountHint. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ItemCountHint = Utilities.Convert_ulong_uint(niProviderEntry.UIntValue());

                                        if(tmpConfig.ItemCountHint == 0)
                                        {
                                            tmpConfig.ItemCountHint = 1024;
                                        }

                                        break;
                                    // Logger string
                                    case "Logger":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element Logger. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Logger = niProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // LoginRequestTimeOut ulong
                                    case "LoginRequestTimeOut":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element LoginRequestTimeOut. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.LoginRequestTimeOut = Utilities.Convert_ulong_long(niProviderEntry.UIntValue());
                                        break;
                                    // MaxDispatchCountApiThread ulong
                                    case "MaxDispatchCountApiThread":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element MaxDispatchCountApiThread. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_ulong_int(niProviderEntry.UIntValue());
                                        break;
                                    // MaxDispatchCountUserThread ulong
                                    case "MaxDispatchCountUserThread":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element MaxDispatchCountUserThread. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_ulong_int(niProviderEntry.UIntValue());
                                        break;
                                    // MaxEventsInPool ulong
                                    case "MaxEventsInPool":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element MaxEventsInPool. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxEventsInPool = Utilities.Convert_ulong_int(niProviderEntry.UIntValue());
                                        break;
                                    // MergeSourceDirectoryStreams uint->bool
                                    case "MergeSourceDirectoryStreams":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element MergeSourceDirectoryStreams. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.MergeSourceDirectoryStreams = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // ReconnectAttemptLimit int
                                    case "ReconnectAttemptLimit":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element ReconnectAttemptLimit. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.ReconnectAttemptLimit = Utilities.Convert_long_int(niProviderEntry.IntValue());
                                        break;
                                    // ReconnectMaxDelay int
                                    case "ReconnectMaxDelay":
                                        {
                                            if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.INT)
                                            {
                                                throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element ReconnectMaxDelay. This element entry must contain an INT and cannot be blank");
                                            }

                                            int value = Utilities.Convert_long_int(niProviderEntry.IntValue());

                                            if(value > 0)
                                                tmpConfig.ReconnectMaxDelay = value;
                                        }
                                        break;
                                    // ReconnectMinDelay int
                                    case "ReconnectMinDelay":
                                        {
                                            if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.INT)
                                            {
                                                throw new OmmInvalidConfigurationException("Invalid entry payload type for NIProvider element ReconnectMinDelay. This element entry must contain an INT and cannot be blank");
                                            }

                                            int value = Utilities.Convert_long_int(niProviderEntry.IntValue());

                                            if(value > 0)
                                                tmpConfig.ReconnectMinDelay = value;
                                        }
                                        break;
                                    // RecoverUserSubmitSourceDirectory uint->bool
                                    case "RecoverUserSubmitSourceDirectory":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element RecoverUserSubmitSourceDirectory. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.RecoverUserSubmitSourceDirectory = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // RefreshFirstRequired uint->bool
                                    case "RefreshFirstRequired":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element RefreshFirstRequired. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.RefreshFirstRequired = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // RemoveItemsOnDisconnect uint->bool
                                    case "RemoveItemsOnDisconnect":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element RemoveItemsOnDisconnect. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.RemoveItemsOnDisconnect = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // RequestTimeout uint
                                    case "RequestTimeout":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element RequestTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.RequestTimeout = Utilities.Convert_ulong_uint(niProviderEntry.UIntValue());
                                        break;
                                    // ServiceCountHint uint
                                    case "ServiceCountHint":
                                        if (niProviderEntry.Load == null || niProviderEntry.Load.Code == DataCode.BLANK || niProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for NiProvider element ServiceCountHint. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ServiceCountHint = Utilities.Convert_ulong_int(niProviderEntry.UIntValue());

                                        if (tmpConfig.ServiceCountHint == 0)
                                        {
                                            tmpConfig.ServiceCountHint = 513;
                                        }

                                        break;

                                    // XmlTraceToStdout bool
                                    case "XmlTraceToStdout":
                                        CheckElementEntry("NiProvider", "XmlTraceToStdout", DataTypes.UINT, niProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToStdout = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceToFile bool
                                    case "XmlTraceToFile":
                                        CheckElementEntry("NiProvider", "XmlTraceToFile", DataTypes.UINT, niProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToFile = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceToFile string
                                    case "XmlTraceFileName":
                                        CheckElementEntry("NiProvider", "XmlTraceFileName", DataTypes.ASCII_STRING, niProviderEntry);

                                        tmpConfig.XmlTraceFileName = niProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // XmlTraceMaxFileSize ulong
                                    case "XmlTraceMaxFileSize":
                                        CheckElementEntry("NiProvider", "XmlTraceMaxFileSize", DataTypes.UINT, niProviderEntry);

                                        tmpConfig.XmlTraceMaxFileSize = niProviderEntry.UIntValue();
                                        break;
                                    // XmlTraceToMultipleFiles bool
                                    case "XmlTraceToMultipleFiles":
                                        CheckElementEntry("NiProvider", "XmlTraceToMultipleFiles", DataTypes.UINT, niProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToMultipleFiles = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceWrite bool
                                    case "XmlTraceWrite":
                                        CheckElementEntry("NiProvider", "XmlTraceWrite", DataTypes.UINT, niProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceWrite = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceRead bool
                                    case "XmlTraceRead":
                                        CheckElementEntry("NiProvider", "XmlTraceRead", DataTypes.UINT, niProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceRead = (niProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTracePing bool
                                    case "XmlTracePing":
                                        CheckElementEntry("NiProvider", "XmlTracePing", DataTypes.UINT, niProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTracePing = (niProviderEntry.UIntValue() != 0);
                                        break;

                                    default:
                                        ommConfig.ConfigErrorLog?.Add("Unknown NiProvider entry element: " + niProviderEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }
                            // If this is a new consumer config, add it to the map.
                            if (foundConfig == false)
                            {
                                ommConfig.NiProviderConfigMap.Add(tmpConfig.Name, tmpConfig);
                            }
                        }
                        break;
                    default:
                        ommConfig.ConfigErrorLog?.Add("Unknown NiProvider element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseIProviderGroup(ElementList ConsumerList, OmmIProviderConfigImpl ommConfig)
        {
            foreach (ElementEntry groupEntry in ConsumerList)
            {
                switch (groupEntry.Name)
                {
                    case "DefaultIProvider":
                        if (groupEntry.Load == null || groupEntry.Load.Code == DataCode.BLANK || groupEntry.LoadType != DataTypes.ASCII_STRING)
                        {
                            throw new OmmInvalidConfigurationException("Missing or invalid DefaultIProvider. DefaultIProvider must be an ASCII_STRING and cannot be blank");
                        }
                        ommConfig.DefaultIProvider = ((OmmAscii)groupEntry.Load).ToString();
                        break;
                    case "IProviderList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid IProviderList. IProviderList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for IProviderList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry iProvider in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            IProviderConfig tmpConfig;
                            if (iProvider.Load == null || DataTypes.ELEMENT_LIST != iProvider.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type fo IProviderList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (iProvider.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for IProviderList Map Entry. The Key must not be blank");
                            }

                            string name = iProvider.Key.Ascii().ToString();

                            if (ommConfig.IProviderConfigMap.ContainsKey(name))
                            {
                                tmpConfig = ommConfig.IProviderConfigMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new IProviderConfig();
                                tmpConfig.Name = name;
                            }

                            if (string.IsNullOrEmpty(ommConfig.FirstConfiguredIProvider))
                            {
                                ommConfig.FirstConfiguredIProvider = tmpConfig.Name;
                            }

                            foreach (ElementEntry iProviderEntry in (ElementList)iProvider.Load)
                            {
                                switch (iProviderEntry.Name)
                                {
                                    // Server string
                                    case "Server":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element Server. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Server = iProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // Dictionary string
                                    case "Directory":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element Directory. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Directory = iProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // DispatchTimeoutApiThread long
                                    case "DispatchTimeoutApiThread":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element DispatchTimeoutApiThread. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.DispatchTimeoutApiThread = iProviderEntry.IntValue();
                                        break;
                                    // ItemCountHint ulong
                                    case "ItemCountHint":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element ItemCountHint. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ItemCountHint = Utilities.Convert_ulong_uint(iProviderEntry.UIntValue());

                                        if(tmpConfig.ItemCountHint == 0)
                                        {
                                            tmpConfig.ItemCountHint = 1024;
                                        }

                                        break;
                                    // Logger string
                                    case "Logger":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element Logger. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Logger = iProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // MaxDispatchCountApiThread ulong
                                    case "MaxDispatchCountApiThread":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element MaxDispatchCountApiThread. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_ulong_int(iProviderEntry.UIntValue());
                                        break;
                                    // MaxDispatchCountUserThread ulong
                                    case "MaxDispatchCountUserThread":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element MaxDispatchCountUserThread. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_ulong_int(iProviderEntry.UIntValue());
                                        break;
                                    // MaxEventsInPool ulong
                                    case "MaxEventsInPool":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element MaxEventsInPool. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxEventsInPool = Utilities.Convert_ulong_int(iProviderEntry.UIntValue());
                                        break;
                                    // RefreshFirstRequired uint->bool
                                    case "RefreshFirstRequired":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element RefreshFirstRequired. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.RefreshFirstRequired = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // RequestTimeout uint
                                    case "RequestTimeout":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element RequestTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.RequestTimeout = Utilities.Convert_ulong_uint(iProviderEntry.UIntValue());
                                        break;
                                    // ServiceCountHint uint
                                    case "ServiceCountHint":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element ServiceCountHint. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ServiceCountHint = Utilities.Convert_ulong_int(iProviderEntry.UIntValue());

                                        if (tmpConfig.ServiceCountHint == 0)
                                        {
                                            tmpConfig.ServiceCountHint = 513;
                                        }

                                        break;

                                    // XmlTraceToStdout bool
                                    case "XmlTraceToStdout":
                                        CheckElementEntry("IProvider", "XmlTraceToStdout", DataTypes.UINT, iProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToStdout = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceToFile bool
                                    case "XmlTraceToFile":
                                        CheckElementEntry("IProvider", "XmlTraceToFile", DataTypes.UINT, iProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToFile = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceToFile string
                                    case "XmlTraceFileName":
                                        CheckElementEntry("IProvider", "XmlTraceFileName", DataTypes.ASCII_STRING, iProviderEntry);

                                        tmpConfig.XmlTraceFileName = iProviderEntry.OmmAsciiValue().ToString();
                                        break;
                                    // XmlTraceMaxFileSize ulong
                                    case "XmlTraceMaxFileSize":
                                        CheckElementEntry("IProvider", "XmlTraceMaxFileSize", DataTypes.UINT, iProviderEntry);

                                        tmpConfig.XmlTraceMaxFileSize = iProviderEntry.UIntValue();
                                        break;
                                    // XmlTraceToMultipleFiles bool
                                    case "XmlTraceToMultipleFiles":
                                        CheckElementEntry("IProvider", "XmlTraceToMultipleFiles", DataTypes.UINT, iProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceToMultipleFiles = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceWrite bool
                                    case "XmlTraceWrite":
                                        CheckElementEntry("IProvider", "XmlTraceWrite", DataTypes.UINT, iProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceWrite = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceRead bool
                                    case "XmlTraceRead":
                                        CheckElementEntry("IProvider", "XmlTraceRead", DataTypes.UINT, iProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTraceRead = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // XmlTracePing bool
                                    case "XmlTracePing":
                                        CheckElementEntry("IProvider", "XmlTracePing", DataTypes.UINT, iProviderEntry, "and have a value of \"0\" or \"1\".");

                                        tmpConfig.XmlTracePing = (iProviderEntry.UIntValue() != 0);
                                        break;

                                    // AcceptDirMessageWithoutMinFilters uint->bool
                                    case "AcceptDirMessageWithoutMinFilters":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element AcceptDirMessageWithoutMinFilters. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.AcceptDirMessageWithoutMinFilters = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // AcceptMessageSameKeyButDiffStream uint->bool
                                    case "AcceptMessageSameKeyButDiffStream":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element AcceptMessageSameKeyButDiffStream. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.AcceptMessageSameKeyButDiffStream = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // AcceptMessageThatChangesService uint->bool
                                    case "AcceptMessageThatChangesService":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element AcceptMessageThatChangesService. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.AcceptMessageThatChangesService = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // AcceptMessageWithoutAcceptingRequests uint->bool
                                    case "AcceptMessageWithoutAcceptingRequests":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element AcceptMessageWithoutAcceptingRequests. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.AcceptMessageWithoutAcceptingRequests = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // AcceptMessageWithoutBeingLogin uint->bool
                                    case "AcceptMessageWithoutBeingLogin":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element AcceptMessageWithoutBeingLogin. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.AcceptMessageWithoutBeingLogin = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // AcceptMessageWithoutQosInRange uint->bool
                                    case "AcceptMessageWithoutQosInRange":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element AcceptMessageWithoutQosInRange. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.AcceptMessageWithoutQosInRange = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // EnforceAckIDValidation uint->bool
                                    case "EnforceAckIDValidation":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element EnforceAckIDValidation. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.EnforceAckIDValidation = (iProviderEntry.UIntValue() != 0);
                                        break;
                                    // EnumTypeFragmentSize uint
                                    case "EnumTypeFragmentSize":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element EnumTypeFragmentSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.EnumTypeFragmentSize = Utilities.Convert_ulong_int(iProviderEntry.UIntValue());
                                        break;
                                    // FieldDictionaryFragmentSize uint
                                    case "FieldDictionaryFragmentSize":
                                        if (iProviderEntry.Load == null || iProviderEntry.Load.Code == DataCode.BLANK || iProviderEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for IProvider element FieldDictionaryFragmentSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.FieldDictionaryFragmentSize = Utilities.Convert_ulong_int(iProviderEntry.UIntValue());
                                        break;
                                    default:
                                        ommConfig.ConfigErrorLog?.Add("Unknown IProvider entry element: " + iProviderEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            // If this is a new consumer config, add it to the map.
                            if (foundConfig == false)
                            {
                                ommConfig.IProviderConfigMap.Add(tmpConfig.Name, tmpConfig);
                            }
                        }
                        break;
                    default:
                        ommConfig.ConfigErrorLog?.Add("Unknown IProvider element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseClientChannelGroup(ElementList channelList, Dictionary<string, ClientChannelConfig> channelMap, ConfigErrorList? errorList)
        {
            foreach (ElementEntry groupEntry in channelList)
            {
                switch (groupEntry.Name)
                {
                    case "ChannelList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid ChannelList. ChannelList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for ChannelList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry channel in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            ClientChannelConfig tmpConfig;
                            if (channel.Load == null || DataTypes.ELEMENT_LIST != channel.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for ChannelList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (channel.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for ChannelList Map Entry. The Key must not be blank");
                            }

                            string name = channel.Key.Ascii().ToString();
                            if (channelMap.ContainsKey(name))
                            {
                                tmpConfig = channelMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new ClientChannelConfig();
                                tmpConfig.Name = name;
                            }

                            foreach (ElementEntry channelEntry in (ElementList)channel.Load)
                            {
                                switch (channelEntry.Name)
                                {
                                    // ChannelType enum
                                    case "ChannelType":
                                        CheckElementEntry("Channel", "ChannelType", DataTypes.ENUM, channelEntry, "with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum.");

                                        if (channelEntry.EnumValue() < 0 || channelEntry.EnumValue() > EmaConfig.ConnectionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element ChannelType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.ConnectionType = (Eta.Transports.ConnectionType)channelEntry.EnumValue();


                                        break;
                                    // EncryptedProtocolType enum
                                    case "EncryptedProtocolType":
                                        CheckElementEntry("Channel", "EncryptedProtocolType", DataTypes.ENUM, channelEntry, "with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum, excluding ConnectionTypeEnum.ENCRYPTED.");

                                        if (channelEntry.EnumValue() < 0 || channelEntry.EnumValue() > EmaConfig.ConnectionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element ChanelType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum, excluding ConnectionTypeEnum.ENCRYPTED.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol = (Eta.Transports.ConnectionType)channelEntry.EnumValue();

                                        break;
                                    // ConnectionPingTimeout uint
                                    case "ConnectionPingTimeout":
                                        CheckElementEntry("Channel", "ConnectionPingTimeout", DataTypes.UINT, channelEntry);

                                        int pingTimeout = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        tmpConfig.ConnectInfo.ConnectOptions.PingTimeout = pingTimeout >= 1000 ? pingTimeout / 1000 : 60;
                                        break;
                                    // EnableSessionManagement uint->bool
                                    case "EnableSessionManagement":
                                        CheckElementEntry("Channel", "EnableSessionManagement", DataTypes.UINT, channelEntry, "and have a value of \"0\" or \"1\"");

                                        tmpConfig.ConnectInfo.EnableSessionManagement = (channelEntry.UIntValue() != 0);
                                        break;
                                    // GuaranteedOutputBuffers uint
                                    case "GuaranteedOutputBuffers":
                                        CheckElementEntry("Channel", "GuaranteedOutputBuffers", DataTypes.UINT, channelEntry);

                                        tmpConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers = (int)channelEntry.UIntValue();
                                        break;

                                    // HighWaterMark uint
                                    case "HighWaterMark":
                                        CheckElementEntry("Channel", "HighWaterMark", DataTypes.UINT, channelEntry);

                                        tmpConfig.HighWaterMark = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // InitializationTimeout uint
                                    case "InitializationTimeout":
                                        CheckElementEntry("Channel", "InitializationTimeout", DataTypes.UINT, channelEntry);

                                        tmpConfig.ConnectInfo.SetInitTimeout((int)channelEntry.UIntValue());
                                        break;
                                    // AuthenticationTimeout uint
                                    case "AuthenticationTimeout":
                                        CheckElementEntry("Channel", "AuthenticationTimeout", DataTypes.UINT, channelEntry);

                                        int temp = Utilities.Convert_ulong_int(channelEntry.UIntValue());

                                        if (temp > 0)
                                        {
                                            tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout = temp;
                                        }
                                        break;
                                    // InterfaceName string
                                    case "InterfaceName":
                                        CheckElementEntry("Channel", "InterfaceName", DataTypes.ASCII_STRING, channelEntry);
                                        tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // Location string
                                    case "Location":
                                        CheckElementEntry("Channel", "Location", DataTypes.ASCII_STRING, channelEntry);
                                        tmpConfig.ConnectInfo.Location = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // NumInputBuffers uint
                                    case "NumInputBuffers":
                                        CheckElementEntry("Channel", "NumInputBuffers", DataTypes.UINT, channelEntry);

                                        tmpConfig.ConnectInfo.ConnectOptions.NumInputBuffers = (int)channelEntry.UIntValue();
                                        break;
                                    // ServiceDiscoveryRetryCount uint
                                    case "ServiceDiscoveryRetryCount":
                                        CheckElementEntry("Channel", "ServiceDiscoveryRetryCount", DataTypes.UINT, channelEntry);

                                        tmpConfig.ConnectInfo.ServiceDiscoveryRetryCount = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // SysRecvBufSize uint
                                    case "SysRecvBufSize":
                                        CheckElementEntry("Channel", "SysRecvBufSize", DataTypes.UINT, channelEntry);

                                        tmpConfig.ConnectInfo.ConnectOptions.SysRecvBufSize = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // SysSendBufSize uint
                                    case "SysSendBufSize":
                                        CheckElementEntry("Channel", "SysSendBufSize", DataTypes.UINT, channelEntry);

                                        tmpConfig.ConnectInfo.ConnectOptions.SysSendBufSize = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // CompressionType enum
                                    case "CompressionType":
                                        CheckElementEntry("Channel", "CompressionType", DataTypes.ENUM, channelEntry, "with one of the values found in LSEG.Ema.Access.EmaConfig.CompressionType");

                                        if (channelEntry.EnumValue() < 0 || channelEntry.EnumValue() > EmaConfig.CompressionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element CompressionType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.CompressionType.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.CompressionType = (Eta.Transports.CompressionType)channelEntry.EnumValue();

                                        break;
                                    // CompressionThreshold uint
                                    case "CompressionThreshold":
                                        CheckElementEntry("Channel", "CompressionThreshold", DataTypes.UINT, channelEntry);

                                        tmpConfig.CompressionThreshold = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        tmpConfig.CompressionThresholdSet = true;
                                        break;
                                    // DirectWrite uint->bool
                                    case "DirectWrite":
                                        CheckElementEntry("Channel", "DirectWrite", DataTypes.UINT, channelEntry, "and have a value of \"0\" or \"1\"");

                                        tmpConfig.DirectWrite = (channelEntry.UIntValue() != 0);
                                        break;
                                    // Host string
                                    case "Host":
                                        CheckElementEntry("Channel", "Host", DataTypes.ASCII_STRING, channelEntry);
                                        
                                        tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // Port string
                                    case "Port":
                                       CheckElementEntry("Channel", "Port", DataTypes.ASCII_STRING, channelEntry);
                                       
                                        tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // ProxyHost string
                                    case "ProxyHost":
                                        CheckElementEntry("Channel", "ProxyHost", DataTypes.ASCII_STRING, channelEntry);
                                        
                                        tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // ProxyPort string
                                    case "ProxyPort":
                                        CheckElementEntry("Channel", "ProxyPort", DataTypes.ASCII_STRING, channelEntry);
                                        
                                        tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // TcpNodelay uint->bool
                                    case "TcpNodelay":
                                        CheckElementEntry("Channel", "TcpNodelay", DataTypes.UINT, channelEntry, "and have a value of \"0\" or \"1\"");

                                        tmpConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay = (channelEntry.UIntValue() != 0);
                                        break;
                                    // SecurityProtocol uint
                                    case "SecurityProtocol":
                                        CheckElementEntry("Channel", "SecurityProtocol", DataTypes.UINT, channelEntry);

                                        if (channelEntry.UIntValue() != 0 && (channelEntry.UIntValue() & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element SecurityProtocol. This must be an Flag EMUM type, with the flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags = (EncryptionProtocolFlags)channelEntry.UIntValue();
                                        break;
                                    default:
                                        errorList?.Add("Unknown Channel entry element: " + channelEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            if (foundConfig == false)
                                channelMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        errorList?.Add("Unknown Channel element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseServerGroup(ElementList serverList, Dictionary<string, ServerConfig> serverMap, ConfigErrorList? errorList)
        {
            foreach (ElementEntry groupEntry in serverList)
            {
                switch (groupEntry.Name)
                {
                    case "ServerList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid ServerList. ServerList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for ServerList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry server in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            ServerConfig tmpConfig;
                            if (server.Load == null || DataTypes.ELEMENT_LIST != server.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for ServerList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (server.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for ServerList Map Entry. The Key must not be blank");
                            }

                            string name = server.Key.Ascii().ToString();
                            if (serverMap.ContainsKey(name))
                            {
                                tmpConfig = serverMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new ServerConfig();
                                tmpConfig.Name = name;
                            }

                            foreach (ElementEntry serverEntry in (ElementList)server.Load)
                            {
                                switch (serverEntry.Name)
                                {
                                    // ServerType enum
                                    case "ServerType":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK ||
                                            serverEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Server element ServerType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum.");
                                        }

                                        if (serverEntry.EnumValue() < 0 || serverEntry.EnumValue() > EmaConfig.ConnectionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Server element ServerType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum.");
                                        }

                                        tmpConfig.BindOptions.ConnectionType = (Eta.Transports.ConnectionType)serverEntry.EnumValue();
                                        break;
                                    // ConnectionMinPingTimeout uint
                                    case "ConnectionMinPingTimeout":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element ConnectionMinPingTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        int minPingTimeout = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        tmpConfig.BindOptions.MinPingTimeout = minPingTimeout >= 1000 ? minPingTimeout / 1000 : 60;
                                        break;
                                    // ConnectionPingTimeout uint
                                    case "ConnectionPingTimeout":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element ConnectionPingTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        int pingTimeout = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        tmpConfig.BindOptions.PingTimeout = pingTimeout >= 1000 ? pingTimeout / 1000 : 60;
                                        break;
                                    // CompressionThreshold uint
                                    case "CompressionThreshold":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element CompressionThreshold. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.CompressionThresholdSet = true;
                                        tmpConfig.CompressionThreshold = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // CompressionType enum
                                    case "CompressionType":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK ||
                                            serverEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Server element CompressionType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.CompressionTypeEnum.");
                                        }

                                        if (serverEntry.EnumValue() < 0 || serverEntry.EnumValue() > EmaConfig.CompressionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Server element CompressionType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.CompressionType.");
                                        }

                                        tmpConfig.BindOptions.CompressionType = (Eta.Transports.CompressionType)serverEntry.EnumValue();
                                        break;
                                    // DirectWrite uint->bool
                                    case "DirectWrite":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element DirectWrite. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.DirectWrite = (serverEntry.UIntValue() != 0);
                                        break;
                                    // GuaranteedOutputBuffers uint
                                    case "GuaranteedOutputBuffers":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element GuaranteedOutputBuffers. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.BindOptions.GuaranteedOutputBuffers = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // HighWaterMark uint
                                    case "HighWaterMark":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element HighWaterMark. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.HighWaterMark = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // InitializationTimeout uint
                                    case "InitializationTimeout":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element InitializationTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.InitializationTimeout = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // InterfaceName string
                                    case "InterfaceName":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element InterfaceName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.BindOptions.InterfaceName = serverEntry.OmmAsciiValue().ToString();
                                        break;
                                    // MaxFragmentSize uint
                                    case "MaxFragmentSize":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element MaxFragmentSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.BindOptions.MaxFragmentSize = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // NumInputBuffers uint
                                    case "NumInputBuffers":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element NumInputBuffers. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.BindOptions.NumInputBuffers = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // Port string
                                    case "Port":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element Port. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.BindOptions.ServiceName = serverEntry.OmmAsciiValue().ToString();
                                        break;
                                    // SysRecvBufSize uint
                                    case "SysRecvBufSize":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element SysRecvBufSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.BindOptions.SysRecvBufSize = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // SysSendBufSize uint
                                    case "SysSendBufSize":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element SysSendBufSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.BindOptions.SysSendBufSize = Utilities.Convert_ulong_int(serverEntry.UIntValue());
                                        break;
                                    // TcpNodelay uint->bool
                                    case "TcpNodelay":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element TcpNodelay. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.BindOptions.TcpOpts.TcpNoDelay = (serverEntry.UIntValue() != 0);
                                        break;
                                    // ServerCert string
                                    case "ServerCert":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element ServerCert. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.BindOptions.BindEncryptionOpts.ServerCertificate = serverEntry.OmmAsciiValue().ToString();
                                        break;
                                    // ServerPrivateKey string
                                    case "ServerPrivateKey":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element ServerPrivateKey. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey = serverEntry.OmmAsciiValue().ToString();
                                        break;
                                    // SecurityProtocol uint
                                    case "SecurityProtocol":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element SecurityProtocol. This element entry must contain an UINT and cannot be blank");
                                        }

                                        if (serverEntry.UIntValue() != 0 && (serverEntry.UIntValue() & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Server element SecurityProtocol. This must be an Flag EMUM type, with the flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags.");
                                        }

                                        tmpConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags = (EncryptionProtocolFlags)serverEntry.UIntValue();
                                        break;
                                    // CipherSuite string
                                    case "CipherSuite":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element CipherSuite. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites = ServerConfig.StringToCipherList(serverEntry.OmmAsciiValue().ToString(), errorList!);
                                        break;
                                    // AuthenticationTimeout uint
                                    case "AuthenticationTimeout":
                                        if (serverEntry.Load == null || serverEntry.Load.Code == DataCode.BLANK || serverEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Server element AuthenticationTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        int temp = Utilities.Convert_ulong_int(serverEntry.UIntValue());

                                        if (temp > 0)
                                        {
                                            tmpConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout = temp;
                                        }
                                        break;
                                    default:
                                        errorList?.Add("Unknown Server entry element: " + serverEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            if (foundConfig == false)
                                serverMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        errorList?.Add("Unknown Server element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }
        static void ParseLoggerGroup(ElementList loggerList, Dictionary<string, LoggerConfig> loggerMap, ConfigErrorList? errorList)
        {
            foreach (ElementEntry groupEntry in loggerList)
            {
                switch (groupEntry.Name)
                {
                    case "LoggerList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid LoggerList. LoggerList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration LoggerList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry logger in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            LoggerConfig tmpConfig;
                            if (logger.Load == null || DataTypes.ELEMENT_LIST != logger.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for LoggerList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (logger.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for LoggerList Map Entry. The Key must not be blank");
                            }

                            string name = logger.Key.Ascii().ToString();
                            if (loggerMap.ContainsKey(name))
                            {
                                tmpConfig = loggerMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new LoggerConfig();
                                tmpConfig.Name = name;
                            }

                            foreach (ElementEntry loggerEntry in (ElementList)logger.Load)
                            {
                                switch (loggerEntry.Name)
                                {
                                    // InterfaceName string
                                    case "FileName":
                                        CheckElementEntry("Logger", "FileName", DataTypes.ASCII_STRING, loggerEntry);
                                        
                                        tmpConfig.FileName = loggerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // IncludeDateInLoggerOutput uint
                                    case "IncludeDateInLoggerOutput":
                                        CheckElementEntry("Logger", "IncludeDateInLoggerOutput", DataTypes.UINT, loggerEntry);

                                        tmpConfig.IncludeDateInLoggerOutput = loggerEntry.UIntValue();
                                        break;
                                    // NumberOfLogFiles uint
                                    case "NumberOfLogFiles":
                                        CheckElementEntry("Logger", "NumberOfLogFiles", DataTypes.UINT, loggerEntry);

                                        tmpConfig.NumberOfLogFiles = loggerEntry.UIntValue();
                                        break;
                                    // MaxLogFileSize uint
                                    case "MaxLogFileSize":
                                        CheckElementEntry("Logger", "MaxLogFileSize", DataTypes.UINT, loggerEntry);

                                        tmpConfig.MaxLogFileSize = loggerEntry.UIntValue();
                                        break;
                                    // LoggerSeverity enum
                                    case "LoggerSeverity":
                                        CheckElementEntry("Logger", "LoggerSeverity", DataTypes.ENUM, loggerEntry, "with the values found in LSEG.Ema.Access.EmaConfig.LoggerLevelEnum.");

                                        if (loggerEntry.EnumValue() < 0 || loggerEntry.EnumValue() > EmaConfig.LoggerLevelEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Logger element LoggerSeverity. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.LoggerLevelEnum.");
                                        }

                                        tmpConfig.LoggerSeverity = (LoggerLevel)loggerEntry.EnumValue();

                                        break;
                                    // LoggerType enum
                                    case "LoggerType":
                                        CheckElementEntry("Logger", "LoggerType", DataTypes.ENUM, loggerEntry, "with the values found in LSEG.Ema.Access.EmaConfig.LoggerTypeEnum.");

                                        if (loggerEntry.EnumValue() < 0 || loggerEntry.EnumValue() > EmaConfig.LoggerTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Logger element LoggerSeverity. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.LoggerTypeEnum.");
                                        }

                                        tmpConfig.LoggerType = (LoggerType)loggerEntry.EnumValue();

                                        break;
                                    default:
                                        errorList?.Add("Unknown Logger entry element: " + loggerEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            if (foundConfig == false)
                                loggerMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        errorList?.Add("Unknown Logger element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseDictionaryGroup(ElementList dictionaryList, Dictionary<string, DictionaryConfig> dictionaryMap, ConfigErrorList? errorList)
        {
            foreach (ElementEntry groupEntry in dictionaryList)
            {
                switch (groupEntry.Name)
                {
                    case "DictionaryList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid DictionaryList. DictionaryList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for DictionaryList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry dictionary in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            DictionaryConfig tmpConfig;
                            if (dictionary.Load == null || DataTypes.ELEMENT_LIST != dictionary.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for DictionaryList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if (dictionary.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for DictionaryList Map Entry. The Key must not be blank");
                            }

                            string name = dictionary.Key.Ascii().ToString();

                            if (dictionaryMap.ContainsKey(name))
                            {
                                tmpConfig = dictionaryMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new DictionaryConfig();
                                tmpConfig.Name = name;
                            }

                            foreach (ElementEntry dictionaryEntry in (ElementList)dictionary.Load)
                            {
                                switch (dictionaryEntry.Name)
                                {
                                    // EnumTypeDefFileName string
                                    case "EnumTypeDefFileName":
                                        CheckElementEntry("Dictionary", "EnumTypeDefFileName", DataTypes.ASCII_STRING, dictionaryEntry);
                                        
                                        tmpConfig.EnumTypeDefFileName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;
                                    // EnumTypeDefFileName string
                                    case "EnumTypeDefItemName":
                                        CheckElementEntry("Dictionary", "EnumTypeDefItemName", DataTypes.ASCII_STRING, dictionaryEntry);
                                        
                                        tmpConfig.EnumTypeDefItemName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;

                                    // RdmFieldDictionaryFileName string
                                    case "RdmFieldDictionaryFileName":
                                        CheckElementEntry("Dictionary", "RdmFieldDictionaryFileName", DataTypes.ASCII_STRING, dictionaryEntry);
                                        
                                        tmpConfig.RdmFieldDictionaryFileName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;

                                    // RdmFieldDictionaryItemName string
                                    case "RdmFieldDictionaryItemName":
                                        CheckElementEntry("Dictionary", "RdmFieldDictionaryItemName", DataTypes.ASCII_STRING, dictionaryEntry);
                                        
                                        tmpConfig.RdmFieldDictionaryItemName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;

                                    // DictionaryType enum
                                    case "DictionaryType":
                                        CheckElementEntry("Dictionary", "DictionaryType", DataTypes.ENUM, dictionaryEntry, "with the values found in LSEG.Ema.Access.EmaConfig.DictionaryTypeEnum");

                                        if (dictionaryEntry.EnumValue() < 0 || dictionaryEntry.EnumValue() > EmaConfig.DictionaryTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Logger element DictionaryType. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.DictionaryTypeEnum.");
                                        }

                                        tmpConfig.DictionaryType = dictionaryEntry.EnumValue();
                                        tmpConfig.IsLocalDictionary = (tmpConfig.DictionaryType == EmaConfig.DictionaryTypeEnum.FILE);

                                        break;
                                    default:
                                        errorList?.Add("Unknown Dictionary entry element: " + dictionaryEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }

                            }

                            if (foundConfig == false)
                                dictionaryMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        errorList?.Add("Unknown Dictionary element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void ParseDirectoryGroup(ElementList directoryList, out string defaultDirectory, Dictionary<string, DirectoryConfig> directoryMap, bool niProvider, ConfigErrorList? errorList)
        {
            defaultDirectory = string.Empty;
            foreach (ElementEntry groupEntry in directoryList)
            {
                switch (groupEntry.Name)
                {
                    case "DefaultDirectory":
                        if (groupEntry.Load == null || groupEntry.Load.Code == DataCode.BLANK || groupEntry.LoadType != DataTypes.ASCII_STRING)
                        {
                            throw new OmmInvalidConfigurationException("Missing or invalid DefaultDirectory. DefaultDirectory must be an ASCII_STRING and cannot be blank");
                        }
                        defaultDirectory = ((OmmAscii)groupEntry.Load).ToString();
                        break;

                    case "DirectoryList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid DirectoryList. DirectoryList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration DirectoryList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry directory in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            DirectoryConfig tmpConfig;
                            if (directory.Load == null || DataTypes.MAP != directory.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for DirectoryList Map Entry. These map entries must contain a Map");
                            }

                            if (directory.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for DirectoryList Map Entry. The Key must not be blank");
                            }

                            string name = directory.Key.Ascii().ToString();

                            if (directoryMap.ContainsKey(name))
                            {
                                tmpConfig = directoryMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new DirectoryConfig();
                                tmpConfig.Name = name;
                            }

                            if (DataTypes.ASCII_STRING != ((Map)directory.Load).KeyType())
                            {
                                throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration DirectoryList Map KeyType. KeyType must be ASCII_STRING");
                            }

                            foreach (MapEntry service in (Map)directory.Load)
                            {
                                bool foundService = false;
                                EmaServiceConfig tmpServiceConfig;
                                if (service.Load == null || DataTypes.ELEMENT_LIST != service.LoadType)
                                {
                                    throw new OmmInvalidConfigurationException("Invalid entry payload type for DictionaryList Map Entry. These map entries must contain a Map");
                                }

                                if (service.Key.Data.Code == DataCode.BLANK)
                                {
                                    throw new OmmInvalidConfigurationException("Invalid entry key type for DictionaryList Map Entry. The Key must not be blank");
                                }

                                string serviceName = service.Key.Ascii().ToString();

                                if (tmpConfig.ServiceMap.ContainsKey(serviceName))
                                {
                                    tmpServiceConfig = tmpConfig.ServiceMap[serviceName];
                                    foundService = true;
                                }
                                else
                                {
                                    tmpServiceConfig = new EmaServiceConfig(niProvider, false);
                                    tmpServiceConfig.Service.Info.ServiceName.Data(serviceName);
                                }


                                foreach (ElementEntry directoryEntry in (ElementList)service.Load)
                                {
                                    switch (directoryEntry.Name)
                                    {
                                        // EnumTypeDefFileName string
                                        case "InfoFilter":
                                            if (directoryEntry.Load == null || directoryEntry.Load.Code == DataCode.BLANK || directoryEntry.LoadType != DataTypes.ELEMENT_LIST)
                                            {
                                                throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory element InfoFilter. This element entry must contain an Element List and cannot be blank");
                                            }

                                            foreach (ElementEntry infoEntry in (ElementList)directoryEntry.Load)
                                            {
                                                switch (infoEntry.Name)
                                                {
                                                    // ServiceId ulong->int
                                                    case "ServiceId":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element ServiceId. This element entry must contain an UINT and cannot be blank");
                                                        }
                                                        tmpServiceConfig.Service.ServiceId = Utilities.Convert_ulong_int(infoEntry.UIntValue());

                                                        if (tmpServiceConfig.Service.ServiceId > ushort.MaxValue)
                                                        {
                                                            throw new OmmInvalidConfigurationException($"service[{tmpServiceConfig.Service.Info.ServiceName}] specifies out of range ServiceId ({tmpServiceConfig.Service.ServiceId}).");
                                                        }

                                                        break;
                                                    // Vendor string
                                                    case "Vendor":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.ASCII_STRING)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element Vendor. This element entry must contain an ASCII string and cannot be blank");
                                                        }
                                                        tmpServiceConfig.Service.Info.HasVendor = true;
                                                        tmpServiceConfig.Service.Info.Vendor.Data(infoEntry.OmmAsciiValue().ToString());
                                                        break;
                                                    // IsSource long-> 1 or 0
                                                    case "IsSource":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element IsSource. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\".");
                                                        }

                                                        tmpServiceConfig.Service.Info.HasIsSource = true;
                                                        tmpServiceConfig.Service.Info.IsSource = (infoEntry.UIntValue() != 0 ? 1 : 0);
                                                        break;
                                                    // Capabilities ulong->int
                                                    case "Capabilities":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.ARRAY)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element Capabilities. This element entry must contain an ARRAY and cannot be blank");
                                                        }
                                                        long tmpCapability;

                                                        tmpServiceConfig.Service.Info.CapabilitiesList.Clear();

                                                        foreach (var capability in (OmmArray)infoEntry.Load)
                                                        {
                                                            switch (capability.LoadType)
                                                            {
                                                                case DataTypes.ASCII_STRING:
                                                                    if (capability.Load == null || capability.Load.Code == DataCode.BLANK)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid array entry payload for Source Directory InfoFilter element Capabilities array. The array entries must contain either an UINT or ASCII string, and cannot be greater than 65535.");
                                                                    }

                                                                    tmpCapability = DirectoryConfig.StringToCapability(capability.OmmAsciiValue().ToString());
                                                                    break;
                                                                case DataTypes.UINT:
                                                                    if (capability.Load == null || capability.Load.Code == DataCode.BLANK || capability.OmmUIntValue().Value > 0xFFFF)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid array entry payload for Source Directory InfoFilter element Capabilities array. The array entries must contain either an UINT or ASCII string, and cannot be greater than 65535.");
                                                                    }

                                                                    tmpCapability = Utilities.Convert_ulong_long(capability.OmmUIntValue().Value);
                                                                    break;
                                                                default:
                                                                    throw new OmmInvalidConfigurationException("Invalid array entry payload type for Source Directory InfoFilter element Capabilities array. The array entries must contain either an UINT or ASCII string, and cannot be greater than 65535.");
                                                            }

                                                            if (!tmpServiceConfig.Service.Info.CapabilitiesList.Contains(tmpCapability))
                                                            {
                                                                tmpServiceConfig.Service.Info.CapabilitiesList.Add(tmpCapability);
                                                            }
                                                        }
                                                        break;
                                                    // AcceptingConsumerStatus ulong
                                                    case "AcceptingConsumerStatus":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element AcceptingConsumerStatus. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\".");
                                                        }

                                                        tmpServiceConfig.Service.Info.HasAcceptingConsStatus = true;
                                                        tmpServiceConfig.Service.Info.AcceptConsumerStatus = (infoEntry.UIntValue() != 0 ? 1 : 0);
                                                        break;
                                                    // Vendor string
                                                    case "ItemList":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.ASCII_STRING)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element ItemList. This element entry must contain an ASCII string and cannot be blank");
                                                        }
                                                        tmpServiceConfig.Service.Info.HasItemList = true;
                                                        tmpServiceConfig.Service.Info.ItemList.Data(infoEntry.OmmAsciiValue().ToString());
                                                        break;
                                                    // DictionariesProvided array of ascii strings
                                                    case "DictionariesProvided":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.ARRAY)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element DictionariesProvided. This element entry must contain an ARRAY and cannot be blank");
                                                        }

                                                        tmpServiceConfig.DictionariesProvidedList.Clear();

                                                        foreach (var dictionaryProvided in (OmmArray)infoEntry.Load)
                                                        {
                                                            switch (dictionaryProvided.LoadType)
                                                            {
                                                                case DataTypes.ASCII_STRING:
                                                                    if (dictionaryProvided.Load == null || dictionaryProvided.Load.Code == DataCode.BLANK)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid array entry payload for Source Directory InfoFilter element DictionariesProvided array. The array entries must contain an ASCII string.");
                                                                    }

                                                                    if (!tmpServiceConfig.DictionariesProvidedList.Contains(dictionaryProvided.OmmAsciiValue().ToString()))
                                                                    {
                                                                        tmpServiceConfig.DictionariesProvidedList.Add(dictionaryProvided.OmmAsciiValue().ToString());
                                                                    }
                                                                    break;
                                                                default:
                                                                    throw new OmmInvalidConfigurationException("Invalid array entry payload type for Source Directory InfoFilter element DictionariesProvided array. The array entries must contain an ASCII string.");
                                                            }
                                                        }
                                                        break;
                                                    // DictionariesUsed array of ascii strings
                                                    case "DictionariesUsed":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.ARRAY)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element DictionariesUsed. This element entry must contain an ARRAY and cannot be blank");
                                                        }

                                                        tmpServiceConfig.DictionariesUsedList.Clear();

                                                        foreach (var dictionaryUsed in (OmmArray)infoEntry.Load)
                                                        {
                                                            switch (dictionaryUsed.LoadType)
                                                            {
                                                                case DataTypes.ASCII_STRING:
                                                                    if (dictionaryUsed.Load == null || dictionaryUsed.Load.Code == DataCode.BLANK)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid array entry payload for Source Directory InfoFilter element DictionariesUsed array. The array entries must contain an ASCII string.");
                                                                    }

                                                                    if (!tmpServiceConfig.DictionariesUsedList.Contains(dictionaryUsed.OmmAsciiValue().ToString()))
                                                                    {
                                                                        tmpServiceConfig.DictionariesUsedList.Add(dictionaryUsed.OmmAsciiValue().ToString());
                                                                    }
                                                                    break;
                                                                default:
                                                                    throw new OmmInvalidConfigurationException("Invalid array entry payload type for Source Directory InfoFilter element DictionariesUsed array. The array entries must contain an ASCII string.");
                                                            }
                                                        }
                                                        break;
                                                    // QoS: Series of ElementLists 
                                                    case "QoS":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.SERIES)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element QoS. This element entry must contain a Series and cannot be blank");
                                                        }

                                                        tmpServiceConfig.Service.Info.QosList.Clear();

                                                        foreach (SeriesEntry qosEntry in (Series)infoEntry.Load)
                                                        {
                                                            if (qosEntry.Load == null || qosEntry.Load.Code == DataCode.BLANK || qosEntry.LoadType != DataTypes.ELEMENT_LIST)
                                                            {
                                                                throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter QoS series element. This series entry must contain an Element List and cannot be blank");
                                                            }
                                                            uint rate = 0;
                                                            uint timeliness = 0;
                                                            foreach (ElementEntry qosElement in (ElementList)qosEntry.Load)
                                                            {
                                                                switch (qosElement.Name)
                                                                {
                                                                    // Rate ASCII string or UINT
                                                                    case "Rate":
                                                                        if (qosElement.Load == null || qosElement.Load.Code == DataCode.BLANK || (qosElement.LoadType != DataTypes.ASCII_STRING && qosElement.LoadType != DataTypes.UINT))
                                                                        {
                                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter QoS Rate element. This element entry must contain either an ASCII string or unsigned integer and cannot be blank");
                                                                        }

                                                                        if (qosElement.LoadType == DataTypes.ASCII_STRING)
                                                                        {
                                                                            string[] rateArray = qosElement.OmmAsciiValue().ToString().Split("::");

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
                                                                        else
                                                                        {
                                                                            // Already verified the type earlier
                                                                            rate = Utilities.Convert_ulong_uint(qosElement.UIntValue());
                                                                        }
                                                                        break;
                                                                    // Timeliness ASCII string or UINT
                                                                    case "Timeliness":
                                                                        if (qosElement.Load == null || qosElement.Load.Code == DataCode.BLANK || (qosElement.LoadType != DataTypes.ASCII_STRING && qosElement.LoadType != DataTypes.UINT))
                                                                        {
                                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter QoS Timeliness element. This element entry must contain either an ASCII string or unsigned integer and cannot be blank");
                                                                        }

                                                                        if (qosElement.LoadType == DataTypes.ASCII_STRING)
                                                                        {
                                                                            string[] timelinessArray = qosElement.OmmAsciiValue().ToString().Split("::");

                                                                            if (timelinessArray.Length != 2)
                                                                            {
                                                                                throw new OmmInvalidConfigurationException(
                                                                                    "Invalid Qos Timeliness string format. Correct format is  \"Timeliness::<RealTime or InexactDelayed>\".");
                                                                            }

                                                                            if (timelinessArray[0] != "Timeliness")
                                                                            {
                                                                                throw new OmmInvalidConfigurationException(
                                                                                    "Invalid QoS Timeliness string format. Correct format is  \"Timeliness::<RealTime or InexactDelayed>\".");
                                                                            }
                                                                            // This indicates that the entry is not an alphanumeric number, so it should be parsed as a string.
                                                                            timeliness = DirectoryConfig.StringToTimeliness(timelinessArray[1]);
                                                                        }
                                                                        else
                                                                        {
                                                                            // Already verified the type earlier
                                                                            timeliness = Utilities.Convert_ulong_uint(qosElement.UIntValue());
                                                                        }
                                                                        break;
                                                                    default:
                                                                        errorList?.Add("Unknown QoS element: " + qosElement.Name, LoggerLevel.ERROR);
                                                                        break;
                                                                }
                                                            }

                                                            Qos tmpQos = new Qos();

                                                            Utilities.ToRsslQos(rate, timeliness, tmpQos);

                                                            tmpServiceConfig.Service.Info.QosList.Add(tmpQos);
                                                        }
                                                        break;
                                                    // SupportsQoSRange ulong
                                                    case "SupportsQoSRange":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element SupportsQoSRange. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\".");
                                                        }

                                                        tmpServiceConfig.Service.Info.HasSupportQosRange = true;
                                                        tmpServiceConfig.Service.Info.SupportsQosRange = (infoEntry.UIntValue() != 0 ? 1 : 0);
                                                        break;
                                                    // SupportsOutOfBandSnapshots ulong
                                                    case "SupportsOutOfBandSnapshots":
                                                        if (infoEntry.Load == null || infoEntry.Load.Code == DataCode.BLANK || infoEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory InfoFilter element SupportsOutOfBandSnapshots. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\".");
                                                        }

                                                        tmpServiceConfig.Service.Info.HasSupportOOBSnapshots = true;
                                                        tmpServiceConfig.Service.Info.SupportsOOBSnapshots = (infoEntry.UIntValue() != 0 ? 1 : 0);
                                                        break;
                                                    default:
                                                        errorList?.Add("Unknown Directory service info filter element: " + infoEntry.Name, LoggerLevel.ERROR);
                                                        break;
                                                }
                                            }
                                            break;
                                        case "StateFilter":
                                            if (directoryEntry.Load == null || directoryEntry.Load.Code == DataCode.BLANK || directoryEntry.LoadType != DataTypes.ELEMENT_LIST)
                                            {
                                                throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory element StateFilter. This element entry must contain an Element List and cannot be blank");
                                            }

                                            foreach (ElementEntry stateEntry in (ElementList)directoryEntry.Load)
                                            {
                                                switch (stateEntry.Name)
                                                {
                                                    // ServiceState ulong
                                                    case "ServiceState":
                                                        if (stateEntry.Load == null || stateEntry.Load.Code == DataCode.BLANK || stateEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter element ServiceState. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\".");
                                                        }

                                                        tmpServiceConfig.Service.State.ServiceStateVal = (stateEntry.UIntValue() != 0 ? 1 : 0);
                                                        break;
                                                    // AcceptingRequests ulong
                                                    case "AcceptingRequests":
                                                        if (stateEntry.Load == null || stateEntry.Load.Code == DataCode.BLANK || stateEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter element AcceptingRequests. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                                        }

                                                        tmpServiceConfig.Service.State.HasAcceptingRequests = true;
                                                        tmpServiceConfig.Service.State.AcceptingRequests = (stateEntry.UIntValue() != 0 ? 1 : 0);
                                                        break;
                                                    // Status Element list structure.
                                                    case "Status":
                                                        if (stateEntry.Load == null || stateEntry.Load.Code == DataCode.BLANK || stateEntry.LoadType != DataTypes.ELEMENT_LIST)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter element Status. This element entry must contain an Element List.");
                                                        }

                                                        foreach(ElementEntry statusEntry in (ElementList)stateEntry.Load)
                                                        {
                                                            switch(statusEntry.Name)
                                                            {
                                                                case "StreamState":
                                                                    if (statusEntry.Load == null || statusEntry.Load.Code == DataCode.BLANK || statusEntry.LoadType != DataTypes.ASCII_STRING)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter Status element StreamState. This element entry must contain an ASCII string.");
                                                                    }
                                                                    string[] streamStateArray = statusEntry.OmmAsciiValue().ToString().Split("::");

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
                                                                    break;
                                                                case "DataState":
                                                                    if (statusEntry.Load == null || statusEntry.Load.Code == DataCode.BLANK || statusEntry.LoadType != DataTypes.ASCII_STRING)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter Status element DataState. This element entry must contain an ASCII string.");
                                                                    }
                                                                    string[] dataStateArray = statusEntry.OmmAsciiValue().ToString().Split("::");

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
                                                                    break;
                                                                case "StatusCode":
                                                                    if (statusEntry.Load == null || statusEntry.Load.Code == DataCode.BLANK || statusEntry.LoadType != DataTypes.ASCII_STRING)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter Status element StatusCode. This element entry must contain an ASCII string.");
                                                                    }
                                                                    string[] statusCodeArray = statusEntry.OmmAsciiValue().ToString().Split("::");

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
                                                                    break;
                                                                case "StatusText":
                                                                    if (statusEntry.Load == null || statusEntry.Load.Code == DataCode.BLANK || statusEntry.LoadType != DataTypes.ASCII_STRING)
                                                                    {
                                                                        throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory StateFilter Status element StatusText. This element entry must contain an ASCII string.");
                                                                    }

                                                                    tmpServiceConfig.Service.State.Status.Text().Data(statusEntry.OmmAsciiValue().ToString());
                                                                    break;
                                                                default:
                                                                    errorList?.Add("Unknown Directory service status element: " + statusEntry.Name, LoggerLevel.ERROR);
                                                                    break;
                                                            }
                                                        }
                                                        break;
                                                    default:
                                                        errorList?.Add("Unknown Directory service info filter element: " + stateEntry.Name, LoggerLevel.ERROR);
                                                        break;
                                                }
                                            }
                                            break;
                                        case "LoadFilter":
                                            if (directoryEntry.Load == null || directoryEntry.Load.Code == DataCode.BLANK || directoryEntry.LoadType != DataTypes.ELEMENT_LIST)
                                            {
                                                throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory element LoadFilter. This element entry must contain an Element List and cannot be blank");
                                            }

                                            foreach (ElementEntry loadEntry in (ElementList)directoryEntry.Load)
                                            {
                                                switch (loadEntry.Name)
                                                {
                                                    // OpenLimit ulong->long
                                                    case "OpenLimit":
                                                        if (loadEntry.Load == null || loadEntry.Load.Code == DataCode.BLANK || loadEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory LoadFilter element OpenLimit. This element entry must contain an UINT and cannot be blank");
                                                        }
                                                        tmpServiceConfig.Service.Load.HasOpenLimit = true;
                                                        tmpServiceConfig.Service.Load.OpenLimit = Utilities.Convert_ulong_long(loadEntry.UIntValue());
                                                        break;
                                                    // OpenWindow ulong->long
                                                    case "OpenWindow":
                                                        if (loadEntry.Load == null || loadEntry.Load.Code == DataCode.BLANK || loadEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory LoadFilter element OpenWindow. This element entry must contain an UINT and cannot be blank");
                                                        }
                                                        tmpServiceConfig.Service.Load.HasOpenWindow = true;
                                                        tmpServiceConfig.Service.Load.OpenWindow = Utilities.Convert_ulong_long(loadEntry.UIntValue());
                                                        break;
                                                    // OpenWindow ulong->long
                                                    case "LoadFactor":
                                                        if (loadEntry.Load == null || loadEntry.Load.Code == DataCode.BLANK || loadEntry.LoadType != DataTypes.UINT)
                                                        {
                                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Source Directory LoadFilter element LoadFactor. This element entry must contain an UINT and cannot be blank");
                                                        }
                                                        tmpServiceConfig.Service.Load.HasLoadFactor = true;
                                                        tmpServiceConfig.Service.Load.LoadFactor = Utilities.Convert_ulong_long(loadEntry.UIntValue());
                                                        break;
                                                    default:
                                                        errorList?.Add("Unknown Directory LoadFilter element: " + loadEntry.Name, LoggerLevel.ERROR);
                                                        break;
                                                }
                                            }
                                            break;
                                        default:
                                            errorList?.Add("Unknown Directory entry element: " + directoryEntry.Name, LoggerLevel.ERROR);
                                            break;
                                    }
                                }
                                if (foundService == false)
                                    tmpConfig.ServiceMap.Add(tmpServiceConfig.Service.Info.ServiceName.ToString(), tmpServiceConfig);
                            }

                            if (foundConfig == false)
                                directoryMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        errorList?.Add("Unknown Directory element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        static void CheckElementEntry(string groupName, string elementName, int dataType, ElementEntry elementEntry, string? errorMessageAddition = null)
        {
            if (elementEntry.Load == null || elementEntry.Load.Code == DataCode.BLANK || elementEntry.LoadType != dataType)
            {
                throw new OmmInvalidConfigurationException($"Invalid entry payload type for {groupName} element {elementName}. This element entry must contain {DataTypes.ToString(dataType)}"
                    + (string.IsNullOrEmpty(errorMessageAddition)
                        ? " and cannot be blank"
                        : ", cannot be blank, " + errorMessageAddition));
            }
        }
    }
}

    