/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System;
using System.Collections;
using System.Reflection.Metadata;
using System.Xml;
using static LSEG.Ema.Access.Data;
using static LSEG.Ema.Access.DictionaryConfig;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    internal class ProgrammaticConfigParser
    {

        private OmmConsumerConfigImpl OmmConfig { get; set; }

        internal ProgrammaticConfigParser(OmmConsumerConfigImpl Config)
        {
            OmmConfig = Config;
        }

        // The programmatic configuration overwrites all XML file-based configuration
        internal void ParseProgrammaticConfig(Map configMap)
        {

            CodecReturnCode ret;
            if(DataTypes.ASCII_STRING != configMap.KeyType())
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

                switch(entry.Key.Ascii().ToString())
                {
                    case "ConsumerGroup":
                        ParseConsumerGroup((ElementList)entry.Load);
                        break;
                    case "ChannelGroup":
                        ParseClientChannelGroup((ElementList)entry.Load);
                        break;
                    case "LoggerGroup":
                        ParseLoggerGroup((ElementList)entry.Load);
                        break;
                    case "DictionaryGroup":
                        ParseDictionaryGroup((ElementList)entry.Load);
                        break;
                    default:
                        // TODO: Log an invalid config, this is not catastrophic
                        break;
                }
            }
        }

        void ParseConsumerGroup(ElementList consumerList)
        {
            foreach(ElementEntry groupEntry in consumerList)
            {
                switch(groupEntry.Name)
                {
                    case "DefaultConsumer":
                        if(groupEntry.Load == null || groupEntry.Load.Code == DataCode.BLANK || groupEntry.LoadType != DataTypes.ASCII_STRING)
                        {
                            throw new OmmInvalidConfigurationException("Missing or invalid DefaultConsumer. DefaultConsumer must be an ASCII_STRING and cannot be blank");
                        }
                        OmmConfig.DefaultConsumer = ((OmmAscii)groupEntry.Load).ToString();
                        break;
                    case "ConsumerList":
                        if (groupEntry.Load == null || groupEntry.LoadType != DataTypes.MAP)
                        {
                            throw new OmmInvalidConfigurationException("Invalid ConsumerList. ConsumerList must be a Map");
                        }

                        if (DataTypes.ASCII_STRING != ((Map)groupEntry.Load).KeyType())
                        {
                            throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration ConsumerList. KeyType must be ASCII_STRING");
                        }

                        foreach (MapEntry consumer in (Map)groupEntry.Load)
                        {
                            bool foundConfig = false;
                            ConsumerConfig tmpConfig;
                            if (consumer.Load == null || DataTypes.ELEMENT_LIST != consumer.LoadType)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry payload type for consumerList Map Entry. These map entries must contain ELEMENT_LIST");
                            }

                            if(consumer.Key.Data.Code == DataCode.BLANK)
                            {
                                throw new OmmInvalidConfigurationException("Invalid entry key type for consumerList Map Entry. The Key must not be blank");
                            }

                            string name = consumer.Key.Ascii().ToString();
                            
                            if (OmmConfig.ConsumerConfigMap.ContainsKey(name))
                            {
                                tmpConfig = OmmConfig.ConsumerConfigMap[name];
                                foundConfig = true;
                            }
                            else
                            {
                                tmpConfig = new ConsumerConfig();
                                tmpConfig.Name = name;
                            }

                            foreach(ElementEntry consumerEntry in (ElementList)consumer.Load)
                            {
                                bool channelSetFound = false;
                                switch(consumerEntry.Name)
                                {
                                    // CatchUnhandledException ulong
                                    case "CatchUnhandledException":
                                        if(consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element CatchUnhandledException. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }
                                        tmpConfig.CatchUnhandledException = (consumerEntry.UIntValue() != 0);
                                        break;
                                        
                                    // Channel string.  Keeping the behavior the same as XML: If ChannelSet is present, that overrides the "Channel", even if it's later in the map.
                                    case "Channel":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for consumer element Channel. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        if(channelSetFound == false)
                                        {
                                            tmpConfig.ChannelSet.Clear();
                                            tmpConfig.ChannelSet.Add(consumerEntry.OmmAsciiValue().ToString());
                                        }
                                        break;
                                    // ChannelSet string containing a comma separated list of Channel names. This will override in all cases.
                                    case "ChannelSet":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for consumer element ChannelSet. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ChannelSet.Clear();

                                        string[] channelArray = consumerEntry.OmmAsciiValue().ToString().Split(',');

                                        for (int i = 0; i < channelArray.Length; i++)
                                            tmpConfig.ChannelSet.Add(channelArray[i].Trim());

                                        channelSetFound = true;
                                        break;
                                    // Dictionary string
                                    case "Dictionary":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element Dictionary. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Dictionary = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // DictionaryRequestTimeOut ulong
                                    case "DictionaryRequestTimeOut":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element DictionaryRequestTimeOut. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.DictionaryRequestTimeOut = Utilities.Convert_ulong_long(consumerEntry.UIntValue());
                                        break;
                                    // DirectoryRequestTimeOut ulong
                                    case "DirectoryRequestTimeOut":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element DirectoryRequestTimeOut. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.DirectoryRequestTimeOut = Utilities.Convert_ulong_long(consumerEntry.UIntValue());
                                        break;
                                    // LoginRequestTimeOut ulong
                                    case "LoginRequestTimeOut":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element LoginRequestTimeOut. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.LoginRequestTimeOut = Utilities.Convert_ulong_long(consumerEntry.UIntValue());
                                        break;
                                    // DispatchTimeoutApiThread long
                                    case "DispatchTimeoutApiThread":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element DispatchTimeoutApiThread. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.DispatchTimeoutApiThread = consumerEntry.IntValue();
                                        break;
                                    // EnableRtt bool
                                    case "EnableRtt":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element EnableRtt. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.EnableRtt = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // ItemCountHint ulong
                                    case "ItemCountHint":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element ItemCountHint. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ItemCountHint = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // Logger string
                                    case "Logger":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element Logger. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.Logger = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // MaxDispatchCountApiThread ulong
                                    case "MaxDispatchCountApiThread":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element MaxDispatchCountApiThread. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxDispatchCountApiThread = Utilities.Convert_ulong_int(consumerEntry.UIntValue());
                                        break;
                                    // MaxDispatchCountApiThread ulong
                                    case "MaxDispatchCountUserThread":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element MaxDispatchCountUserThread. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxDispatchCountUserThread = Utilities.Convert_ulong_int(consumerEntry.UIntValue());
                                        break;
                                    // MaxOutstandingPosts ulong
                                    case "MaxOutstandingPosts":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element MaxOutstandingPosts. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxOutstandingPosts = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // MsgKeyInUpdates bool
                                    case "MsgKeyInUpdates":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element MsgKeyInUpdates. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.MsgKeyInUpdates = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // ObeyOpenWindow bool
                                    case "ObeyOpenWindow":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element ObeyOpenWindow. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.ObeyOpenWindow = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // PostAckTimeout uint
                                    case "PostAckTimeout":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element PostAckTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.PostAckTimeout = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // ReconnectAttemptLimit int
                                    case "ReconnectAttemptLimit":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element ReconnectAttemptLimit. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.ReconnectAttemptLimit = Utilities.Convert_long_int(consumerEntry.IntValue());
                                        break;
                                    // ReconnectMaxDelay int
                                    case "ReconnectMaxDelay":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element ReconnectMaxDelay. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.ReconnectMaxDelay = Utilities.Convert_long_int(consumerEntry.IntValue());
                                        break;
                                    // ReconnectMinDelay int
                                    case "ReconnectMinDelay":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.INT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element ReconnectMinDelay. This element entry must contain an INT and cannot be blank");
                                        }

                                        tmpConfig.ReconnectMinDelay = Utilities.Convert_long_int(consumerEntry.IntValue());
                                        break;
                                    // RequestTimeout uint
                                    case "RequestTimeout":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element RequestTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.RequestTimeout = Utilities.Convert_ulong_uint(consumerEntry.UIntValue());
                                        break;
                                    // RestEnableLog bool
                                    case "RestEnableLog":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element RestEnableLog. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.RestEnableLog = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // RestEnableLogViaCallback bool
                                    case "RestEnableLogViaCallback":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element RestEnableLogViaCallback. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.RestEnableLogViaCallback = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // RestLogFileName string
                                    case "RestLogFileName":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element RestLogFileName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.RestLogFileName = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // RestRequestTimeOut uint
                                    case "RestRequestTimeOut":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element RestRequestTimeOut. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.RestRequestTimeOut = consumerEntry.UIntValue();
                                        break;
                                    // ServiceCountHint uint
                                    case "ServiceCountHint":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element ServiceCountHint. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ServiceCountHint = Utilities.Convert_ulong_int(consumerEntry.UIntValue());
                                        break;
                                    // XmlTraceDump bool
                                    case "XmlTraceDump":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element XmlTraceDump. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.XmlTraceDump = consumerEntry.UIntValue();
                                        break;
                                    // XmlTraceToStdout bool
                                    case "XmlTraceToStdout":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element XmlTraceToStdout. This element entry must contain an INT, cannot be blank, and have a value of \"0\" or \"1\".");
                                        }

                                        tmpConfig.XmlTraceToStdout = (consumerEntry.UIntValue() != 0);
                                        break;
                                    // XmlTraceFileName string
                                    case "XmlTraceFileName":
                                        if (consumerEntry.Load == null || consumerEntry.Load.Code == DataCode.BLANK || consumerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Consumer element XmlTraceFileName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.XmlTraceFileName = consumerEntry.OmmAsciiValue().ToString();
                                        break;
                                    default:
                                        OmmConfig.ConfigErrorLog?.Add("Unknown Consumer entry element: " + consumerEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }
                            
                            // If this is a new consumer config, add it to the map.
                            if (foundConfig == false)
                            {
                                OmmConfig.ConsumerConfigMap.Add(tmpConfig.Name, tmpConfig);
                            }
                        }
                        break;
                    default:
                        OmmConfig.ConfigErrorLog?.Add("Unknown Consumer element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        void ParseClientChannelGroup(ElementList channelList)
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
                            throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration ChannelList. KeyType must be ASCII_STRING");
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
                            if (OmmConfig.ClientChannelConfigMap.ContainsKey(name))
                            {
                                tmpConfig = OmmConfig.ClientChannelConfigMap[name];
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
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK ||
                                            channelEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Channel element ChannelType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum.");
                                        }
                                        
                                        if (channelEntry.EnumValue() < 0 || channelEntry.EnumValue() > EmaConfig.ConnectionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element ChannelType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum.");
                                        }
                                        
                                        tmpConfig.ConnectInfo.ConnectOptions.ConnectionType = (Eta.Transports.ConnectionType)channelEntry.EnumValue();

                                        
                                        break;
                                    // EncryptedProtocolType enum
                                    case "EncryptedProtocolType":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK ||
                                            channelEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Channel element EncryptedProtocolType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum, excluding ConnectionTypeEnum.ENCRYPTED.");
                                        }

                                        if (channelEntry.EnumValue() < 0 || channelEntry.EnumValue() > EmaConfig.ConnectionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element ChanelType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum, excluding ConnectionTypeEnum.ENCRYPTED.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol = (Eta.Transports.ConnectionType)channelEntry.EnumValue();

                                        break;
                                    // ConnectionPingTimeout uint
                                    case "ConnectionPingTimeout":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element channelEntry. This element entry must contain an UINT and cannot be blank");
                                        }

                                        int pingTimeout = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        tmpConfig.ConnectInfo.ConnectOptions.PingTimeout = pingTimeout >= 1000 ? pingTimeout/1000 : 60;
                                        break;
                                    // EnableSessionManagement uint->bool
                                    case "EnableSessionManagement":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element EnableSessionManagement. This element entry must contain an UINT, cannot be blank, and have a value of \"0\" or \"1\"");
                                        }

                                        tmpConfig.ConnectInfo.EnableSessionManagement = (channelEntry.UIntValue() != 0);
                                        break;
                                    // GuaranteedOutputBuffers uint
                                    case "GuaranteedOutputBuffers":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element GuaranteedOutputBuffers. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers = (int)channelEntry.UIntValue();
                                        break;

                                    // HighWaterMark uint
                                    case "HighWaterMark":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element HighWaterMark. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.HighWaterMark = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // InitializationTimeout uint
                                    case "InitializationTimeout":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element InitializationTimeout. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.SetInitTimeout((int)channelEntry.UIntValue());
                                        break;
                                    // InterfaceName string
                                    case "InterfaceName":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element InterfaceName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // Location string
                                    case "Location":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element Location. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ConnectInfo.Location = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // NumInputBuffers uint
                                    case "NumInputBuffers":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element NumInputBuffers. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.NumInputBuffers = (int)channelEntry.UIntValue();
                                        break;
                                    // ServiceDiscoveryRetryCount uint
                                    case "ServiceDiscoveryRetryCount":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element ServiceDiscoveryRetryCount. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.ServiceDiscoveryRetryCount = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // SysRecvBufSize uint
                                    case "SysRecvBufSize":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element SysRecvBufSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.SysRecvBufSize = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // SysSendBufSize uint
                                    case "SysSendBufSize":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element SysSendBufSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.SysSendBufSize = Utilities.Convert_ulong_int(channelEntry.UIntValue());
                                        break;
                                    // CompressionType enum
                                    case "CompressionType":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK ||
                                            channelEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Channel element CompressionType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.CompressionType");
                                        }

                                        if (channelEntry.EnumValue() < 0 || channelEntry.EnumValue() > EmaConfig.CompressionTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element CompressionType. This must be an EMUM type, with one of the values found in LSEG.Ema.Access.EmaConfig.CompressionType.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.CompressionType = (Eta.Transports.CompressionType)channelEntry.EnumValue();
                                        
                                        break;
                                    // DirectWrite uint->bool
                                    case "DirectWrite":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element DirectWrite. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.DirectWrite = (channelEntry.UIntValue() != 0);
                                        break;
                                    // Host string
                                    case "Host":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element Host. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // Port string
                                    case "Port":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element Port. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // ProxyHost string
                                    case "ProxyHost":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element ProxyHost. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // ProxyPort string
                                    case "ProxyPort":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element ProxyPort. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = channelEntry.OmmAsciiValue().ToString();
                                        break;
                                    // TcpNodelay uint->bool
                                    case "TcpNodelay":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element TcpNodelay. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay = (channelEntry.UIntValue() != 0);
                                        break;
                                    // SecurityProtocol uint
                                    case "SecurityProtocol":
                                        if (channelEntry.Load == null || channelEntry.Load.Code == DataCode.BLANK || channelEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Channel element SecurityProtocol. This element entry must contain an UINT and cannot be blank");
                                        }

                                        if (channelEntry.UIntValue() != 0 && (channelEntry.UIntValue() & EmaConfig.EncryptedTLSProtocolFlags.TLS_ALL) == 0)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Channel element SecurityProtocol. This must be an Flag EMUM type, with the flag values found in LSEG.Ema.Access.EmaConfig.EncryptedTLSProtocolFlags.");
                                        }

                                        tmpConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags = (EncryptionProtocolFlags)channelEntry.UIntValue();
                                        break;
                                    default:
                                        OmmConfig.ConfigErrorLog?.Add("Unknown Channel entry element: " + channelEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            if (foundConfig == false)
                                OmmConfig.ClientChannelConfigMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        OmmConfig.ConfigErrorLog?.Add("Unknown Channel element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        void ParseLoggerGroup(ElementList loggerList)
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
                            if (OmmConfig.LoggerConfigMap.ContainsKey(name))
                            {
                                tmpConfig = OmmConfig.LoggerConfigMap[name];
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
                                        if (loggerEntry.Load == null || loggerEntry.Load.Code == DataCode.BLANK || loggerEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Logger element FileName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.FileName = loggerEntry.OmmAsciiValue().ToString();
                                        break;
                                    // IncludeDateInLoggerOutput uint
                                    case "IncludeDateInLoggerOutput":
                                        if (loggerEntry.Load == null || loggerEntry.Load.Code == DataCode.BLANK || loggerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Logger element IncludeDateInLoggerOutput. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.IncludeDateInLoggerOutput = loggerEntry.UIntValue();
                                        break;
                                    // NumberOfLogFiles uint
                                    case "NumberOfLogFiles":
                                        if (loggerEntry.Load == null || loggerEntry.Load.Code == DataCode.BLANK || loggerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Logger element NumberOfLogFiles. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.NumberOfLogFiles = loggerEntry.UIntValue();
                                        break;
                                    // MaxLogFileSize uint
                                    case "MaxLogFileSize":
                                        if (loggerEntry.Load == null || loggerEntry.Load.Code == DataCode.BLANK || loggerEntry.LoadType != DataTypes.UINT)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Logger element MaxLogFileSize. This element entry must contain an UINT and cannot be blank");
                                        }

                                        tmpConfig.MaxLogFileSize = loggerEntry.UIntValue();
                                        break;
                                    // LoggerSeverity enum
                                    case "LoggerSeverity":
                                        if (loggerEntry.Load == null || loggerEntry.Load.Code == DataCode.BLANK ||
                                            loggerEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Logger element LoggerSeverity. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.LoggerLevelEnum.");
                                        }

                                        if (loggerEntry.EnumValue() < 0 || loggerEntry.EnumValue() > EmaConfig.LoggerLevelEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Logger element LoggerSeverity. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.LoggerLevelEnum.");
                                        }

                                        tmpConfig.LoggerSeverity = (LoggerLevel)loggerEntry.EnumValue();
                                       
                                        break;
                                    // LoggerType enum
                                    case "LoggerType":
                                        if (loggerEntry.Load == null || loggerEntry.Load.Code == DataCode.BLANK ||
                                            loggerEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Logger element LoggerType. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.LoggerTypeEnum.");
                                        }

                                        if (loggerEntry.EnumValue() < 0 || loggerEntry.EnumValue() > EmaConfig.LoggerTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Logger element LoggerSeverity. This must be an EMUM type, with the values found in LSEG.Ema.Access.EmaConfig.LoggerTypeEnum.");
                                        }

                                        tmpConfig.LoggerType = (LoggerType)loggerEntry.EnumValue();
                                        
                                        break;
                                    default:
                                        OmmConfig.ConfigErrorLog?.Add("Unknown Logger entry element: " + loggerEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }
                            }

                            if (foundConfig == false)
                                OmmConfig.LoggerConfigMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        OmmConfig.ConfigErrorLog?.Add("Unknown Logger element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }

        void ParseDictionaryGroup(ElementList dictionaryList)
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
                            throw new OmmInvalidConfigurationException("Invalid key type for Programmatic Configuration DictionaryList. KeyType must be ASCII_STRING");
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
                            
                            if (OmmConfig.DictionaryConfigMap.ContainsKey(name))
                            {
                                tmpConfig = OmmConfig.DictionaryConfigMap[name];
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
                                        if (dictionaryEntry.Load == null || dictionaryEntry.Load.Code == DataCode.BLANK || dictionaryEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Dictionary element EnumTypeDefFileName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.EnumTypeDefFileName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;
                                    // EnumTypeDefFileName string
                                    case "EnumTypeDefItemName":
                                        if (dictionaryEntry.Load == null || dictionaryEntry.Load.Code == DataCode.BLANK || dictionaryEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Dictionary element EnumTypeDefItemName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.EnumTypeDefItemName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;
                                    // EnumTypeDefFileName string
                                    case "RdmFieldDictionaryFileName":
                                        if (dictionaryEntry.Load == null || dictionaryEntry.Load.Code == DataCode.BLANK || dictionaryEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Dictionary element RdmFieldDictionaryFileName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.RdmFieldDictionaryFileName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;
                                    // EnumTypeDefFileName string
                                    case "RdmFieldDictionaryItemName":
                                        if (dictionaryEntry.Load == null || dictionaryEntry.Load.Code == DataCode.BLANK || dictionaryEntry.LoadType != DataTypes.ASCII_STRING)
                                        {
                                            throw new OmmInvalidConfigurationException("Invalid entry payload type for Dictionary element RdmFieldDictionaryItemName. This element entry must contain an ASCII string and cannot be blank");
                                        }
                                        tmpConfig.RdmFieldDictionaryItemName = dictionaryEntry.OmmAsciiValue().ToString();
                                        break;

                                    // DictionaryType enum
                                    case "DictionaryType":
                                        if (dictionaryEntry.Load == null || dictionaryEntry.Load.Code == DataCode.BLANK ||
                                            dictionaryEntry.LoadType != DataTypes.ENUM)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid entry payload type for Dictionary element DictionaryType. This must be an EMUM type, with the values found in LSEG.Ema.Access.DictionaryConfig.DictionaryType");
                                        }

                                        if (dictionaryEntry.EnumValue() < 0 || dictionaryEntry.EnumValue() > EmaConfig.DictionaryTypeEnum.MAX_DEFINED)
                                        {
                                            throw new OmmInvalidConfigurationException(
                                                "Invalid value for Logger element DictionaryType. This must be an EMUM type, with the values found in LSEG.Ema.Access.DictionaryConfig.DictionaryType.");
                                        }

                                        tmpConfig.DictionaryType = dictionaryEntry.EnumValue();
                                        tmpConfig.IsLocalDictionary = (tmpConfig.DictionaryType == EmaConfig.DictionaryTypeEnum.FILE);

                                        break;
                                    default:
                                        OmmConfig.ConfigErrorLog?.Add("Unknown Dictionary entry element: " + dictionaryEntry.Name, LoggerLevel.ERROR);
                                        break;
                                }

                            }

                            if (foundConfig == false)
                                OmmConfig.DictionaryConfigMap.Add(tmpConfig.Name, tmpConfig);
                        }
                        break;
                    default:
                        OmmConfig.ConfigErrorLog?.Add("Unknown Dictionary element: " + groupEntry.Name, LoggerLevel.ERROR);
                        break;
                }
            }
        }
    }
}