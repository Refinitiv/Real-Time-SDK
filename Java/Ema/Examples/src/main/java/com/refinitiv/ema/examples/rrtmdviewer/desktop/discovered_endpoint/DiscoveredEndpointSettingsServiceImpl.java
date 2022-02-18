package com.refinitiv.ema.examples.rrtmdviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.AsyncResponseStatuses;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ChannelInformationClient;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.stream.IntStream;

public class DiscoveredEndpointSettingsServiceImpl implements DiscoveredEndpointSettingsService {

    private static final String SERVICE_DISCOVERY_ERROR_HEADER = "Error during retrieving service endpoints";

    private static final String OMM_CONSUMER_INIT_ERROR_HEADER = "Error during OMM consumer initialization";

    private ServiceEndpointDiscovery customServiceDiscovery;
    private OAuthCallback oAuthCallback = new OAuthCallback();
    private CredentialStore credentials = new CredentialStore();
    private String currServiceDiscoveryURL;
    private String currTokenURL;

    private boolean initialized;

    public DiscoveredEndpointSettingsServiceImpl() {
    }

    @Override
    public boolean isInitialized() {
        return initialized;
    }

    @Override
    public void requestServiceDiscovery(DiscoveredEndpointSettingsModel discoveredEndpointSettings, AsyncResponseModel responseListener) {
        if (customServiceDiscovery == null
                || !discoveredEndpointSettings.getDiscoveryEndpointUrl().equals(currServiceDiscoveryURL)
                || !discoveredEndpointSettings.getTokenServiceUrl().equals(currTokenURL)) {
            String tokenV1Url = discoveredEndpointSettings.useV1() ? discoveredEndpointSettings.getTokenServiceUrl() : null;
            String tokenV2Url = discoveredEndpointSettings.useV1() ? null : discoveredEndpointSettings.getTokenServiceUrl();
            customServiceDiscovery = EmaFactory.createServiceEndpointDiscovery(tokenV1Url, tokenV2Url, discoveredEndpointSettings.getDiscoveryEndpointUrl());
            currServiceDiscoveryURL = discoveredEndpointSettings.getDiscoveryEndpointUrl();
            currTokenURL = discoveredEndpointSettings.getTokenServiceUrl();
        }

        final ServiceEndpointDiscoveryOption serviceDiscoveryOpt = EmaFactory.createServiceEndpointDiscoveryOption();

        if (discoveredEndpointSettings.useV1()) {
            serviceDiscoveryOpt
                    .username(discoveredEndpointSettings.getUsername())
                    .password(discoveredEndpointSettings.getPassword())
                    .clientId(discoveredEndpointSettings.getClientId());
        } else {
            serviceDiscoveryOpt
                    .clientId(discoveredEndpointSettings.getClientId())
                    .clientSecret(discoveredEndpointSettings.getClientSecret());
        }
        serviceDiscoveryOpt.transport(discoveredEndpointSettings.getConnectionType().getTransportProtocol());
        serviceDiscoveryOpt.tokenScope("trapi.streaming.pricing.read");

        if (discoveredEndpointSettings.isProxyUsed()) {
            final ProxyDataModel proxyData = discoveredEndpointSettings.getProxyData();
            serviceDiscoveryOpt
                    .proxyHostName(proxyData.getHost())
                    .proxyPort(proxyData.getPort());
            if (proxyData.isAuthenticationUsed()) {
                final ProxyAuthenticationDataModel proxyAuthentication = proxyData.getProxyAuthentication();
                serviceDiscoveryOpt
                        .proxyUserName(proxyAuthentication.getLogin())
                        .proxyPassword(proxyAuthentication.getPassword())
                        .proxyDomain(proxyAuthentication.getDomain())
                        .proxyKRB5ConfigFile(proxyAuthentication.getKrbFilePath());
            }
        }
        try {
            customServiceDiscovery.registerClient(serviceDiscoveryOpt, this, responseListener);
        } catch (Exception e) {
            responseListener.setResponse(e.getMessage());
            responseListener.setResponseStatus(AsyncResponseStatuses.FAILED);
        }
    }

    @Override
    public void createOmmConsumer(DiscoveredEndpointSettingsModel discoveredEndpointSettingsModel,
                                  ServiceEndpointDataModel serviceEndpointDataModel,
                                  OMMViewerError viewerError) {
        try {
            final OmmConsumerConfig config;
            final EmaConfigModel emaConfig = discoveredEndpointSettingsModel.getEmaConfigModel();
            if (emaConfig.isUseEmaConfig()) {
                config = EmaFactory.createOmmConsumerConfig(emaConfig.getEmaConfigFilePath())
                        .consumerName(emaConfig.getConsumerName());
            } else {
                final Map ommConsumerConfig = createProgrammaticConfig(serviceEndpointDataModel,
                        Objects.equals(discoveredEndpointSettingsModel.getConnectionType(), DiscoveredEndpointConnectionTypes.ENCRYPTED_WEBSOCKET));
                config = EmaFactory.createOmmConsumerConfig()
                        .config(ommConsumerConfig)
                        .consumerName("Consumer_1");
            }
            if (discoveredEndpointSettingsModel.useV1()) {
                config.username(discoveredEndpointSettingsModel.getUsername())
                        .password(discoveredEndpointSettingsModel.getPassword())
                        .clientId(discoveredEndpointSettingsModel.getClientId());
            } else {
                config.clientId(discoveredEndpointSettingsModel.getClientId())
                        .clientSecret(discoveredEndpointSettingsModel.getClientSecret());
            }


            if (discoveredEndpointSettingsModel.isEncrypted()) {
                config.tunnelingKeyStoreFile(discoveredEndpointSettingsModel.getEncryptionData().getKeyFilePath())
                        .tunnelingKeyStorePasswd(discoveredEndpointSettingsModel.getEncryptionData().getKeyPassword());
            }
            if (discoveredEndpointSettingsModel.isProxyUsed()) {
                config.tunnelingProxyHostName(discoveredEndpointSettingsModel.getProxyData().getHost())
                        .tunnelingProxyPort(discoveredEndpointSettingsModel.getProxyData().getPort());
                if (discoveredEndpointSettingsModel.getProxyData().isAuthenticationUsed()) {
                    config.tunnelingCredentialUserName(discoveredEndpointSettingsModel.getProxyData().getProxyAuthentication().getLogin())
                            .tunnelingCredentialPasswd(discoveredEndpointSettingsModel.getProxyData().getProxyAuthentication().getPassword())
                            .tunnelingCredentialDomain(discoveredEndpointSettingsModel.getProxyData().getProxyAuthentication().getDomain())
                            .tunnelingCredentialKRB5ConfigFile(discoveredEndpointSettingsModel.getProxyData().getProxyAuthentication().getKrbFilePath());
                }
            }

            ChannelInformationClient channelInformationClient;
            config.serviceDiscoveryUrl(discoveredEndpointSettingsModel.getDiscoveryEndpointUrl());

            if(ApplicationSingletonContainer.containsBean(ChannelInformationClient.class) == false) {
                channelInformationClient = new ChannelInformationClient();
                ApplicationSingletonContainer.addBean(ChannelInformationClient.class, channelInformationClient);
            }
            else {
                channelInformationClient = ApplicationSingletonContainer.getBean(ChannelInformationClient.class);
                channelInformationClient.clear();
            }

            channelInformationClient.setConfigScenario(ChannelInformationClient.ConfigScenario.DISCOVERY_ENDPOINT);

            final OmmConsumer consumer;
            if (discoveredEndpointSettingsModel.useV1()) {
                config.tokenServiceUrl(discoveredEndpointSettingsModel.getTokenServiceUrl());
                consumer = EmaFactory.createOmmConsumer(config, channelInformationClient);
            } else {
                credentials.clientId = discoveredEndpointSettingsModel.getClientId();
                credentials.clientSecret = discoveredEndpointSettingsModel.getClientSecret();
                consumer = EmaFactory.createOmmConsumer(config.tokenServiceUrlV2(discoveredEndpointSettingsModel.getTokenServiceUrl()), channelInformationClient, oAuthCallback, credentials);
            }
            ApplicationSingletonContainer.addBean(OmmConsumer.class, consumer);
        } catch (OmmException e) {
            viewerError.clear();
            viewerError.setFailed(true);
            viewerError.appendErrorText(OMM_CONSUMER_INIT_ERROR_HEADER);
            viewerError.appendErrorText(e.getMessage());
        }
    }

    @Override
    public void initialize() {
        initialized = true;
    }

    @Override
    public void onSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent event) {
        final List<DiscoveredEndpointInfoModel> seds = mapToServiceDiscoveryInfoModel(serviceEndpointResp.serviceEndpointInfoList());
        final AsyncResponseModel responseProperty = (AsyncResponseModel) event.closure();
        responseProperty.setResponse(new ServiceEndpointResponseModel(seds));
        responseProperty.setResponseStatus(AsyncResponseStatuses.SUCCESS);
    }

    @Override
    public void onError(String errorText, ServiceEndpointDiscoveryEvent event) {
        final AsyncResponseModel responseProperty = (AsyncResponseModel) event.closure();
        responseProperty.setResponse(errorText);
        responseProperty.setResponseStatus(AsyncResponseStatuses.FAILED);
    }

    @Override
    public void uninitialize() {
        if (customServiceDiscovery != null) {
            customServiceDiscovery.uninitialize();
            customServiceDiscovery = null;
        }
        currTokenURL = null;
        currServiceDiscoveryURL = null;

        initialized = false;
    }

    private Map createProgrammaticConfig(ServiceEndpointDataModel sed, boolean isWebSocket) {
        Map configDb = EmaFactory.createMap();
        Map elementMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", getChannelSetValue(sed.getEndpoints().size())));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));

        //For support regular tracing it should be also enabled.
        innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));

        //For support RTT monitoring it should be enabled
        innerElementList.add(EmaFactory.createElementEntry().uintValue("EnableRtt", 1));

        elementMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map("ConsumerList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, elementList));
        elementList.clear();

        for (int i = 0; i < sed.getEndpoints().size(); i++) {
            innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));

            if (isWebSocket) {
                innerElementList.add(EmaFactory.createElementEntry().ascii("EncryptedProtocolType", "EncryptedProtocolType::RSSL_WEBSOCKET"));
                innerElementList.add(EmaFactory.createElementEntry().ascii("WsProtocols", "tr_json2, rssl.json.v2"));
            }

            innerElementList.add(EmaFactory.createElementEntry().ascii("Host", sed.getEndpoints().get(i).getEndpoint()));
            innerElementList.add(EmaFactory.createElementEntry().ascii("Port", sed.getEndpoints().get(i).getPort()));
            innerElementList.add(EmaFactory.createElementEntry().intValue("EnableSessionManagement", 1));

            elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_" + (i + 1), MapEntry.MapAction.ADD, innerElementList));
            innerElementList.clear();
        }

        elementList.add(EmaFactory.createElementEntry().map("ChannelList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList));
        elementList.clear();

        /* Dictionary group */
        final DictionaryDataModel dictionaryData = sed.getDictionaryData();
        if (dictionaryData.isDownloadFromNetwork()) {
            innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
        } else {
            innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
            innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", dictionaryData.getFieldDictionaryPath()));
            innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", dictionaryData.getEnumDictionaryPath()));
        }
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map("DictionaryList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("DictionaryGroup", MapEntry.MapAction.ADD, elementList));
        elementList.clear();
        return configDb;
    }

    private String getChannelSetValue(int size) {
        final StringBuilder channelBuilder = new StringBuilder();
        IntStream.range(1, size)
                .forEach(i -> channelBuilder.append("Channel_").append(i).append(", "));
        channelBuilder.append("Channel_").append(size);
        return channelBuilder.toString();
    }

    private List<DiscoveredEndpointInfoModel> mapToServiceDiscoveryInfoModel(List<ServiceEndpointDiscoveryInfo> serviceEndpointDiscoveries) {
        final List<DiscoveredEndpointInfoModel> seds = new ArrayList<>();
        for (ServiceEndpointDiscoveryInfo serviceEndpointDiscoveryInfo : serviceEndpointDiscoveries) {
            final DiscoveredEndpointInfoModel discoveredEndpointInfoModel = new DiscoveredEndpointInfoModel();
            discoveredEndpointInfoModel.setLocations(serviceEndpointDiscoveryInfo.locationList());
            discoveredEndpointInfoModel.setEndpoint(serviceEndpointDiscoveryInfo.endpoint());
            discoveredEndpointInfoModel.setPort(serviceEndpointDiscoveryInfo.port());
            discoveredEndpointInfoModel.composeLabel();
            seds.add(discoveredEndpointInfoModel);
        }
        return seds;
    }

    class OAuthCallback implements OmmOAuth2ConsumerClient
    {
        public void onOAuth2CredentialRenewal(OmmConsumerEvent event)
        {
            CredentialStore credentials = (CredentialStore)event.closure();

            OAuth2CredentialRenewal renewal = EmaFactory.createOAuth2CredentialRenewal();

            renewal.clientId(credentials.clientId);
            renewal.clientSecret(credentials.clientSecret);

            credentials.consumer.renewOAuthCredentials(renewal);
        }
    }

    /* This is for example purposes, For best security, please use a proper credential store. */
    class CredentialStore
    {
        public String clientSecret;
        public String clientId;
        public OmmConsumer consumer;
    }
}
