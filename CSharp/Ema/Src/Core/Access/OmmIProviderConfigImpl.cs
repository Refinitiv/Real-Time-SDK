/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023, 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System.Collections.Generic;
using System.Net.Security;

namespace LSEG.Ema.Access
{
    internal class OmmIProviderConfigImpl
    {
        // Note: Unlike the NIProvider and Consumer configImpl classes, the IProviderConfig does not have a GenerateRole method.

        // DispatchModel config from OmmConsumerConfig methods.
        internal int DispatchModel { get; private set; } = (int)OmmIProviderConfig.OperationModelMode.API_DISPATCH;

        // Path of the Xml configuration file. If not specified when the OmmIProviderConfig
        // is created, this will default to "EmaConfig.xml"
        internal string XmlConfigPath { get; set; } = string.Empty;

        // Dictionary tables indexed by the name of the config.
        internal Dictionary<string, ServerConfig> ServerConfigMap { get; set; }
        internal Dictionary<string, LoggerConfig> LoggerConfigMap { get; set; }
        internal Dictionary<string, DictionaryConfig> DictionaryConfigMap { get; set; }
        internal Dictionary<string, IProviderConfig> IProviderConfigMap { get; set; }
        internal Dictionary<string, DirectoryConfig> DirectoryConfigMap { get; set; }

        internal ConfigErrorList? ConfigErrorLog { get; set; } = null;

        internal DirectoryCache? DirectoryCache { get; private set; } = null;

        public OmmIProviderConfig.AdminControlMode AdminControlDirectory { get; internal set; } = OmmIProviderConfig.AdminControlMode.API_CONTROL;
        public OmmIProviderConfig.AdminControlMode AdminControlDictionary { get; internal set; } = OmmIProviderConfig.AdminControlMode.API_CONTROL;

        // configured name of the IProvider
        internal string IProviderName { get; set; } = string.Empty;

        // Configured name of the server port
        internal string ServerPort { get; private set; } = string.Empty;

        // Default NI provider configured from the Xml.  If Empty, this will be either the first configured consumer, or if no consumers are configured, the default consumer.
        internal string DefaultIProvider { get; set; } = string.Empty;

        // Name of the first configured NiProvider.
        internal string FirstConfiguredIProvider { get; set; } = string.Empty;

        // On a copied OmmIProviderConfigImpl, this is the configured directory name.
        internal string DirectoryName { get; set; } = string.Empty;

        // Default Directory configured from the Xml.  If Empty, this will be either the first configured consumer, or if no consumers are configured, the default consumer.
        internal string DefaultDirectory { get; set; } = string.Empty;

        // Name of the first configured directory.
        internal string FirstConfiguredDirectory { get; set; } = string.Empty;

        internal DirectoryRefresh? AdminDirectoryRefresh { get; set; } = null;

        // Xml parser class.  This is not used with the "active" configuration in OmmBaseImpl
        internal XmlConfigParser? XmlParser { get; set; }

        internal string ServerCertPath { get; set; } = string.Empty;
        internal string ServerPrivateKeyPath { get; set; } = string.Empty;

        // Internal reference to the IProvider, dictionary, and logger configuration used as the "active" configuration, once copied to the OmmBaseImpl.
        internal IProviderConfig IProviderConfig { get; set; } = new IProviderConfig();
        internal ServerConfig ServerConfig { get; set; } = new ServerConfig();
        internal DirectoryConfig DirectoryConfig { get; set; } = new DirectoryConfig();
        internal LoggerConfig LoggerConfig { get; set; } = new LoggerConfig();

        internal uint EncryptedTLSProtocolFlags { get; set; } = 0;
        internal bool SetEncryptedProtocolFlags { get; set; } = false;

        internal List<TlsCipherSuite>? ConfigCipherSuites = null;

        private const string DefaultPort = "14002";

        public OmmIProviderConfigImpl(string? path)
        {
            IProviderConfigMap = new Dictionary<string, IProviderConfig>();
            ServerConfigMap = new Dictionary<string, ServerConfig>();
            LoggerConfigMap = new Dictionary<string, LoggerConfig>();
            DictionaryConfigMap = new Dictionary<string, DictionaryConfig>();
            DirectoryConfigMap = new Dictionary<string, DirectoryConfig>();
            ConfigErrorLog = new ConfigErrorList();

            Clear();

            XmlConfigPath = path ?? string.Empty;
            XmlParser = new XmlConfigParser(this);
        }

        internal void Clear()
        {
            IProviderName = string.Empty;
            ServerPort = string.Empty;
            DefaultIProvider = string.Empty;
            FirstConfiguredIProvider = string.Empty;
            DirectoryName = string.Empty;
            DefaultDirectory = string.Empty;
            FirstConfiguredDirectory = string.Empty;
            DispatchModel = (int)OmmIProviderConfig.OperationModelMode.API_DISPATCH;
            IProviderConfigMap.Clear();
            ServerConfigMap.Clear();
            LoggerConfigMap.Clear();
            DictionaryConfigMap.Clear();
            DirectoryConfigMap.Clear();
            IProviderConfig.Clear();
            LoggerConfig.Clear();
            AdminControlDirectory = OmmIProviderConfig.AdminControlMode.API_CONTROL;
            AdminControlDictionary = OmmIProviderConfig.AdminControlMode.API_CONTROL;
            EncryptedTLSProtocolFlags = 0;
            SetEncryptedProtocolFlags = false;
            ServerCertPath = string.Empty;
            ServerPrivateKeyPath = string.Empty;
        }

        internal void ProviderName(string providerName)
        {
            IProviderName = providerName;
        }

        internal void Port(string serverPort)
        {
            ServerPort = serverPort;
        }

        internal void OperationModel(int operationModel)
        {
            DispatchModel = operationModel;
        }

        internal void Config(Map config)
        {
            ProgrammaticConfigParser.ParseProgrammaticIProviderConfig(config, this);
        }

        internal void EncryptedProtocolFlags(uint protocolFlags)
        {
            EncryptedTLSProtocolFlags = protocolFlags;
            SetEncryptedProtocolFlags = true;
        }

        internal void TlsCipherSuites(IEnumerable<TlsCipherSuite> cipherSuites)
        {
            if (ConfigCipherSuites == null)
            {
                ConfigCipherSuites = new List<TlsCipherSuite>(cipherSuites);
            }
            else
            {
                ConfigCipherSuites.Clear();
                ConfigCipherSuites.AddRange(cipherSuites);
            }
        }

        internal void ServerCertificate(string serverCert)
        {
            ServerCertPath = serverCert;
        }

        internal void ServerPrivateKey(string privateKey)
        {
            ServerPrivateKeyPath = privateKey;
        }

        internal void AddAdminMsg(RefreshMsg refreshMsg)
        {
            if (refreshMsg.m_refreshMsgEncoder.m_rsslMsg.DomainType == (int)DomainType.SOURCE)
            {
                DecodeIterator decodeIter;
                IRefreshMsg msg;
                DirectoryRefresh tmpDirectoryRefresh = new();

                refreshMsg.EncodeComplete();

                decodeIter = new DecodeIterator();

                decodeIter.SetBufferAndRWFVersion(refreshMsg.m_refreshMsgEncoder?.m_encodeIterator?.Buffer(), Codec.MajorVersion(), Codec.MinorVersion());

                msg = new Eta.Codec.Msg();

                if (msg.Decode(decodeIter) != CodecReturnCode.SUCCESS)
                {
                    ConfigErrorLog?.Add("Unable to decode the provided Source Directory message, leaving request at default values.", LoggerLevel.ERROR);
                    return;
                }

                if (tmpDirectoryRefresh.Decode(decodeIter, (Eta.Codec.Msg)msg) != CodecReturnCode.SUCCESS)
                {
                    ConfigErrorLog?.Add("Unable to decode the provided Source Directory message, leaving request at default values", LoggerLevel.ERROR);
                    return;
                }

                // Set the AdminDirectoryRefresh reference to the new tmpDirectoryRefresh
                AdminDirectoryRefresh = tmpDirectoryRefresh;
            }
            else
            {
                throw new OmmInvalidConfigurationException("Non-supported domain in request message passed to AddAdminMsg. Supported request message domains are: LSEG.Eta.Rdm.DomainType.SOURCE.");
            }
        }


        // Iterates through the consumer config and determines everything is correct:
        // The Default IProvider is present(if config'd)
        // All Servers referenced by the IProviders are present
        // All Loggers referenced by the IProviders are present
        // All Directories referenced by the IProviders are present
        // The Default Directory is present
        // All Dictionaries referenced by all of the Directory services are present
        internal void VerifyConfiguration()
        {
            // First, if there's a default consumer, verify that it exists in the consumerConfigMap
            if (!string.IsNullOrEmpty(IProviderName))
            {
                if (!IProviderConfigMap.ContainsKey(IProviderName))
                {
                    throw new OmmInvalidConfigurationException("IProvider " + IProviderName + " set by OmmIProviderConfig.ProviderName is not defined in this OmmIProviderConfig");
                }
            }

            if (!string.IsNullOrEmpty(DefaultIProvider))
            {
                if (!IProviderConfigMap.ContainsKey(DefaultIProvider))
                {
                    throw new OmmInvalidConfigurationException("Default IProvider " + DefaultIProvider + " is not defined in this OmmIProviderConfig");
                }
            }

            // Now iterate through all of the consumers and verify that the Channel, Logger, and Dictionary configs are present in the config database
            foreach (IProviderConfig iProvider in IProviderConfigMap.Values)
            {
                if (string.IsNullOrEmpty(iProvider.Server))
                {
                    throw new OmmInvalidConfigurationException("IProvider " + iProvider.Name + " does not have any configured server. Use Server configure the channels.");
                }

                if (!ServerConfigMap.ContainsKey(iProvider.Server))
                {
                    throw new OmmInvalidConfigurationException("Server " + iProvider.Server + " in IProvider " + iProvider.Name + " is not defined in this OmmIProviderConfig");
                }

                if (!string.IsNullOrEmpty(iProvider.Logger))
                {
                    if (!LoggerConfigMap.ContainsKey(iProvider.Logger))
                    {
                        throw new OmmInvalidConfigurationException("Logger " + iProvider.Logger + " in IProvider " + iProvider.Name + " is not defined in this OmmIProviderConfig");
                    }
                }

                if (!string.IsNullOrEmpty(iProvider.Directory))
                {
                    if (!DirectoryConfigMap.ContainsKey(iProvider.Directory))
                    {
                        throw new OmmInvalidConfigurationException("Directory " + iProvider.Directory + " in IProvider " + iProvider.Name + " is not defined in this OmmIProviderConfig");
                    }
                }
            }

            if (!string.IsNullOrEmpty(DefaultDirectory))
            {
                if (!DirectoryConfigMap.ContainsKey(DefaultDirectory))
                {
                    throw new OmmInvalidConfigurationException("Default Directory " + DefaultDirectory + " is not defined in this OmmIProviderConfig");
                }
            }

            // Now iterate through all of the Directory definitions and make sure that the dictionaries they reference are all defined.
            foreach (DirectoryConfig directory in DirectoryConfigMap.Values)
            {
                foreach (EmaServiceConfig service in directory.ServiceMap.Values)
                {
                    // Check to make sure that the service Id isn't already set somewhere else in the serviceMap
                    foreach (EmaServiceConfig otherService in directory.ServiceMap.Values)
                    {
                        if (!ReferenceEquals(service, otherService))
                        {
                            if (service.Service.ServiceId == otherService.Service.ServiceId)
                            {
                                throw new OmmInvalidConfigurationException("Multiple services with the same service Id " + service.Service.ServiceId + " are present in Directory " + directory.Name);
                            }
                        }
                    }

                    foreach (string providedEntry in service.DictionariesProvidedList)
                    {
                        if (!DictionaryConfigMap.ContainsKey(providedEntry))
                        {
                            throw new OmmInvalidConfigurationException("Dictionary provided " + providedEntry + " in Service " + service.Service.Info.ServiceName + " is not defined in this OmmIProviderConfig");
                        }
                    }

                    foreach (string usedEntry in service.DictionariesUsedList)
                    {
                        if (!DictionaryConfigMap.ContainsKey(usedEntry))
                        {
                            throw new OmmInvalidConfigurationException("Dictionary " + usedEntry + " in Service " + service.Service.Info.ServiceName + " is not defined in this OmmIProviderConfig");
                        }
                    }
                }
            }
        }

        // Copy Constructor that will be used in OmmBaseImpl.  This will contain only the information needed by EMA to generate
        // ReactorConnectOptions and the ReactorRole.  It will not copy everything, just:
        // the configured required Consumer(in order: IProviderName, IProvider, the first IProvider in the consumer list, the default IProvider)
        // any channels directly referenced by the consumer(or 
        // the dictionary referenced by the consumer(or default)
        // the 
        // 
        // PREREQUSITES: OldConfigImpl has been verified with VerifyConfiguration()
        internal OmmIProviderConfigImpl(OmmIProviderConfigImpl OldConfigImpl)
        {
            IProviderConfigMap = new Dictionary<string, IProviderConfig>();
            ServerConfigMap = new Dictionary<string, ServerConfig>();
            LoggerConfigMap = new Dictionary<string, LoggerConfig>();
            DictionaryConfigMap = new Dictionary<string, DictionaryConfig>();
            DirectoryConfigMap = new Dictionary<string, DirectoryConfig>();

            Clear();

            ServerPort = OldConfigImpl.ServerPort;
            IProviderName = OldConfigImpl.IProviderName;
            DispatchModel = OldConfigImpl.DispatchModel;
            DefaultIProvider = OldConfigImpl.DefaultIProvider;
            DefaultDirectory = OldConfigImpl.DefaultDirectory;
            AdminControlDirectory = OldConfigImpl.AdminControlDirectory;
            AdminControlDictionary = OldConfigImpl.AdminControlDictionary;
            ServerCertPath = OldConfigImpl.ServerCertPath;
            ServerPrivateKeyPath = OldConfigImpl.ServerPrivateKeyPath;

            if(OldConfigImpl.ConfigCipherSuites != null)
            {
                ConfigCipherSuites = new List<TlsCipherSuite>();

                ConfigCipherSuites.AddRange(OldConfigImpl.ConfigCipherSuites);
            }

            IProviderConfig.Clear();
            LoggerConfig.Clear();
            IProviderConfigMap.Clear();
            ServerConfigMap.Clear();
            LoggerConfigMap.Clear();
            DictionaryConfigMap.Clear();
            DirectoryConfigMap.Clear();

            if (OldConfigImpl.IProviderConfigMap.Count > 0)
            {
                if (string.IsNullOrEmpty(OldConfigImpl.IProviderName))
                {
                    if (!string.IsNullOrEmpty(OldConfigImpl.DefaultIProvider))
                    {
                        IProviderName = OldConfigImpl.DefaultIProvider;
                    }
                    else
                    {
                        IProviderName = OldConfigImpl.FirstConfiguredIProvider;
                    }
                }
                else
                {
                    IProviderName = OldConfigImpl.IProviderName;
                }

                OldConfigImpl.IProviderConfigMap[IProviderName].Copy(IProviderConfig);
            }
            else
            {
                // Default consumer case where either nothing's configured or the host and port have been specified, we will override as necessary later on.
                // ConsumerConfig was cleared in the earlier Clear() call.
                IProviderConfig.Name = "DefaultEmaIProvider";
                IProviderName = IProviderConfig.Name;
            }

            IProviderConfigMap.Add(IProviderConfig.Name, IProviderConfig);

            // If the channelset is empty we're in the default connection state or the Port has been specified, so create a default config, and set hostName and port if it's been configured.
            if (string.IsNullOrEmpty(IProviderConfig.Server))
            {
                // Server config has been initialized with the defaults, so we just need to set the overrides.
                ServerConfig.Name = "DefaultEmaServer";
                // Both of these will be set as long as the application calls OmmConsumerConfig.Host().
                if (!string.IsNullOrEmpty(ServerPort))
                {
                    ServerConfig.BindOptions.ServiceName = ServerPort;
                }
                else
                {
                    // Set this to the same default port as the client channel.
                    ServerConfig.BindOptions.ServiceName = DefaultPort;
                }
            }
            else
            {
                // There's a configured server config, so copy it over.
                OldConfigImpl.ServerConfigMap[IProviderConfig.Server].Copy(ServerConfig);
                
                // Override the configured port if it was set via method
                if (!string.IsNullOrEmpty(ServerPort))
                {
                    ServerConfig.BindOptions.ServiceName = ServerPort;
                }
            }

            if(!string.IsNullOrEmpty(ServerCertPath))
            {
                ServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate = ServerCertPath;
            }

            if (!string.IsNullOrEmpty(ServerPrivateKeyPath))
            {
                ServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey = ServerPrivateKeyPath;
            }

            if(ConfigCipherSuites != null)
            {
                ServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites = ConfigCipherSuites;
            }

            // Add the Server to the config map.
            ServerConfigMap.Add(ServerConfig.Name, ServerConfig);
            IProviderConfig.Server = ServerConfig.Name;

            if (!string.IsNullOrEmpty(IProviderConfig.Logger))
            {
                // There's a configured logger config, so copy it over.
                OldConfigImpl.LoggerConfigMap[IProviderConfig.Logger].Copy(LoggerConfig);
            }
            else
            {
                // LoggerConfig has alredy been initialized to defaults in the Clear() method.
                LoggerConfig.Name = "DefaultEmaLogger";
                IProviderConfig.Logger = LoggerConfig.Name;
            }

            LoggerConfigMap.Add(LoggerConfig.Name, LoggerConfig);

            DirectoryCache = new DirectoryCache();

            // If the directory mode is API_DRIVEN, setup the DirectoryCache with the new info
            if (AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            {
                long filters = 0;
                DirectoryConfig tmpDirectoryConfig;
                EmaServiceConfig tmpEmaService;
                bool addDefaultDictionary = false;

                // if directory refresh was specified via addAdminMsg, that overrides everything else, so take the decoded refresh and add all services to the service cache.
                if (OldConfigImpl.AdminDirectoryRefresh != null)
                {
                    tmpDirectoryConfig = new DirectoryConfig();
                    OldConfigImpl.AdminDirectoryRefresh.Copy(DirectoryCache.DirectoryRefresh);
                    AdminDirectoryRefresh = DirectoryCache.DirectoryRefresh;

                    tmpDirectoryConfig.Name = "AddAdminDirectory";

                    foreach (Service service in AdminDirectoryRefresh.ServiceList)
                    {
                        tmpEmaService = new EmaServiceConfig(true, service);

                        tmpEmaService.DictionariesProvidedList.Add("DefaultEmaDictionary");
                        tmpEmaService.DictionariesUsedList.Add("DefaultEmaDictionary");
                        addDefaultDictionary = true;

                        tmpDirectoryConfig.ServiceMap.Add(tmpEmaService.Service.Info.ServiceName.ToString(), tmpEmaService);
                    }
                }
                else if (OldConfigImpl.DirectoryConfigMap.Count > 0)
                {
                    if (string.IsNullOrEmpty(IProviderConfig.Directory))
                    {
                        if (string.IsNullOrEmpty(OldConfigImpl.DefaultDirectory))
                        {
                            DirectoryName = OldConfigImpl.FirstConfiguredDirectory;
                            IProviderConfig.Directory = DirectoryName;
                        }
                        else
                        {
                            DirectoryName = OldConfigImpl.DefaultDirectory;
                            IProviderConfig.Directory = DirectoryName;
                        }
                    }
                    else
                    {
                        DirectoryName = IProviderConfig.Directory;
                    }

                    DirectoryCache.DirectoryName = DirectoryName;
                    tmpDirectoryConfig = new DirectoryConfig(OldConfigImpl.DirectoryConfigMap[DirectoryName]);

                    foreach (EmaServiceConfig emaService in tmpDirectoryConfig.ServiceMap.Values)
                    {
                        // If there was no configured QoS for this service, add it.
                        if (emaService.Service.Info.QosList.Count == 0)
                        {
                            Qos qos = new();
                            qos.Rate(QosRates.TICK_BY_TICK);
                            qos.Timeliness(QosTimeliness.REALTIME);
                            emaService.Service.Info.QosList.Add(qos);
                        }
                        DictionaryConfig tmpDictionary;
                        // Add the dictionaries proivded and used into the DictionaryConfigMap, and also add them to the service list.
                        foreach (string dictionaryName in emaService.DictionariesProvidedList)
                        {
                            if (DictionaryConfigMap.ContainsKey(dictionaryName))
                                tmpDictionary = DictionaryConfigMap[dictionaryName];
                            else
                            {
                                tmpDictionary = new DictionaryConfig();
                                OldConfigImpl.DictionaryConfigMap[dictionaryName].Copy(tmpDictionary);
                            }

                            if (!string.IsNullOrEmpty(tmpDictionary.RdmFieldDictionaryItemName))
                            {
                                emaService.Service.Info.DictionariesProvidedList.Add(tmpDictionary.RdmFieldDictionaryItemName);
                                emaService.Service.Info.HasDictionariesProvided = true;
                            }

                            if (!string.IsNullOrEmpty(tmpDictionary.EnumTypeDefItemName))
                            {
                                emaService.Service.Info.DictionariesProvidedList.Add(tmpDictionary.EnumTypeDefItemName);
                                emaService.Service.Info.HasDictionariesProvided = true;
                            }

                            if (!DictionaryConfigMap.ContainsKey(dictionaryName))
                                DictionaryConfigMap.Add(tmpDictionary.Name, tmpDictionary);
                        }

                        foreach (string dictionaryName in emaService.DictionariesUsedList)
                        {
                            if (DictionaryConfigMap.ContainsKey(dictionaryName))
                                tmpDictionary = DictionaryConfigMap[dictionaryName];
                            else
                            {
                                tmpDictionary = new DictionaryConfig();
                                OldConfigImpl.DictionaryConfigMap[dictionaryName].Copy(tmpDictionary);
                            }

                            OldConfigImpl.DictionaryConfigMap[dictionaryName].Copy(tmpDictionary);

                            if (!string.IsNullOrEmpty(tmpDictionary.RdmFieldDictionaryItemName))
                            {
                                emaService.Service.Info.DictionariesUsedList.Add(tmpDictionary.RdmFieldDictionaryItemName);
                                emaService.Service.Info.HasDictionariesUsed = true;
                            }

                            if (!string.IsNullOrEmpty(tmpDictionary.EnumTypeDefItemName))
                            {
                                emaService.Service.Info.DictionariesUsedList.Add(tmpDictionary.EnumTypeDefItemName);
                                emaService.Service.Info.HasDictionariesUsed = true;
                            }

                            if (!DictionaryConfigMap.ContainsKey(dictionaryName))
                                DictionaryConfigMap.Add(tmpDictionary.Name, tmpDictionary);
                        }

                        if (emaService.Service.HasInfo)
                            filters |= Directory.ServiceFilterFlags.INFO;
                        if (emaService.Service.HasState)
                            filters |= Directory.ServiceFilterFlags.STATE;
                        if (emaService.Service.HasData)
                            filters |= Directory.ServiceFilterFlags.DATA;
                        if (emaService.Service.HasLink)
                            filters |= Directory.ServiceFilterFlags.LINK;

                        DirectoryCache.AddService(emaService.Service);
                    }
                    // Set AdminDirectoryRefresh to the generated DirectoryRefresh refresh object
                    AdminDirectoryRefresh = DirectoryCache.DirectoryRefresh;
                    AdminDirectoryRefresh.Filter = filters;
                }
                else
                {
                    tmpDirectoryConfig = new DirectoryConfig();
                    // Set AdminDirectoryRefresh to the generated DirectoryRefresh refresh object
                    AdminDirectoryRefresh = DirectoryCache.DirectoryRefresh;
                    tmpDirectoryConfig.Name = "DefaultEmaDirectory";
                    // Setup a default ema service config
                    tmpEmaService = new EmaServiceConfig(false, true);
                    tmpEmaService.DictionariesProvidedList.Add("DefaultEmaDictionary");
                    tmpEmaService.DictionariesUsedList.Add("DefaultEmaDictionary");
                    addDefaultDictionary = true;
                    AdminDirectoryRefresh.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE;
                    // Add the new generated service to the directory cache and the directory config.
                    DirectoryCache.AddService(tmpEmaService.Service);
                    tmpDirectoryConfig.ServiceMap.Add(tmpEmaService.Service.Info.ServiceName.ToString(), tmpEmaService);

                }
                // Add the directory to the config map.
                DirectoryConfigMap.Add(tmpDirectoryConfig.Name, tmpDirectoryConfig);
                DirectoryConfig = tmpDirectoryConfig;

                if(addDefaultDictionary)
                {
                    DictionaryConfig defaultDictionaryConfig = new DictionaryConfig();

                    defaultDictionaryConfig.Name = "DefaultEmaDictionary";

                    DictionaryConfigMap.Add(defaultDictionaryConfig.Name, defaultDictionaryConfig);
                }
            }

        }

        // Helper to get the configured bind options from the IProviderConfig.
        // Prerequsites: The user-supplied OmmIProviderConfigImpl has been verified with OmmIProviderConfigImpl.VerifyConfig and this is run on the "active" configuration copy
        public BindOptions GenerateBindOptions()
        {
            return ServerConfig.BindOptions;
        }

    }
}
   
