/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.specified_endpoint;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ChannelInformationClient;
import com.refinitiv.ema.examples.rtviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.DictionaryDataModel;
import com.refinitiv.eta.codec.CodecReturnCodes;

public class SpecifiedEndpointSettingsServiceImpl implements SpecifiedEndpointSettingsService {

    private static final String OMM_CONSUMER_INIT_ERROR_HEADER = "Error during OMM consumer initialization";

    @Override
    public int connect(SpecifiedEndpointSettingsModel settings, OMMViewerError error) {
        error.clear();

        OmmConsumerConfig config;

        /* End setting Dictionary group */
        try {

        if (settings.getEmaConfigModel().isUseEmaConfig()) {
            config = EmaFactory
                    .createOmmConsumerConfig(settings.getEmaConfigModel().getEmaConfigFilePath())
                    .consumerName(settings.getEmaConfigModel().getConsumerName());
        } else {
            config = EmaFactory.createOmmConsumerConfig().config(loadProgrammaticConfigMap(settings));
            if (settings.getConnectionDataModel().isEncrypted() && settings.hasCustomEncrOptions()) {
                if (settings.getEncryptionSettings().getKeyFilePath() != null && !settings.getEncryptionSettings().getKeyFilePath().equals("")) {
                    config.tunnelingKeyStoreFile(settings.getEncryptionSettings().getKeyFilePath());
                    config.tunnelingSecurityProtocol("TLS");
                }
                if (settings.getEncryptionSettings().getKeyPassword() != null && !settings.getEncryptionSettings().getKeyPassword().equals("")) {
                    config.tunnelingKeyStorePasswd(settings.getEncryptionSettings().getKeyPassword());
                }
                
                boolean tlsv12Enable = settings.isTLSv12Enabled();
                boolean tlsv13Enable = settings.isTLSv13Enabled();
                config.tunnelingSecurityProtocol("TLS");
                
                if (tlsv12Enable && tlsv13Enable)
                	config.tunnelingSecurityProtocolVersions(new String[]{"1.2", "1.3"});
                else if (tlsv12Enable)
                	config.tunnelingSecurityProtocolVersions(new String[]{"1.2"});
                else if (tlsv13Enable)
                	config.tunnelingSecurityProtocolVersions(new String[]{"1.3"});
		else
                	config.tunnelingSecurityProtocolVersions(new String[]{});

            }
        }

        if(!settings.getApplicationId().isEmpty()) {
            config.applicationId(settings.getApplicationId());
        }

        if(!settings.getPosition().isEmpty()) {
            config.position(settings.getPosition());
        }

        if(!settings.getUsername().isEmpty()) {
            config.username(settings.getUsername());
        }

            ChannelInformationClient channelInformationClient;

            if(ApplicationSingletonContainer.containsBean(ChannelInformationClient.class) == false) {
                channelInformationClient = new ChannelInformationClient();
                ApplicationSingletonContainer.addBean(ChannelInformationClient.class, channelInformationClient);
            }
            else {
                channelInformationClient = ApplicationSingletonContainer.getBean(ChannelInformationClient.class);
                channelInformationClient.clear();
            }

            channelInformationClient.setConfigScenario(ChannelInformationClient.ConfigScenario.SPECIFY_ENDPOINT);

            OmmConsumer consumer = EmaFactory.createOmmConsumer(config, channelInformationClient);
            ApplicationSingletonContainer.addBean(OmmConsumer.class, consumer);
        } catch (Exception ex) {
            error.appendErrorText(OMM_CONSUMER_INIT_ERROR_HEADER);
            error.appendErrorText(ex.getMessage());
            error.setFailed(true);
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private Map loadProgrammaticConfigMap(SpecifiedEndpointSettingsModel settings) {
        Map innerMap = EmaFactory.createMap();
        Map configMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();

        /* Consumer group */
        elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1"));

        if (settings.getHost().size() == 1) {
            innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", "Channel_1"));
        } else {
            innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", "Channel_1, Channel_2"));
        }

        innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
        innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
        innerElementList.add(EmaFactory.createElementEntry().uintValue("EnableRtt", 1));
        innerMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map("ConsumerList", innerMap));
        innerMap.clear();
        configMap.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, elementList));
        elementList.clear();
        /* End setting Consumer group */

        for (int i = 0; i < settings.getHost().size(); i++) {

            /* Channel group */
            if (settings.getConnectionDataModel().isEncrypted()) {
                innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
                innerElementList.add(EmaFactory.createElementEntry().ascii("EncryptedProtocolType", "EncryptedProtocolType::" + getProtocolType(settings.getConnectionDataModel().getConnectionType())));
            } else {
                innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::" + getProtocolType(settings.getConnectionDataModel().getConnectionType())));
            }

            innerElementList.add(EmaFactory.createElementEntry().ascii("WsProtocols", settings.getConnectionDataModel().getProtocolList()));

            innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 5000));
            innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 50000));
            innerElementList.add(EmaFactory.createElementEntry().ascii("Host", settings.getHost().get(i)));
            innerElementList.add(EmaFactory.createElementEntry().ascii("Port", settings.getPort().get(i)));

            String channelName = "Channel_" + (i + 1);

            innerMap.add(EmaFactory.createMapEntry().keyAscii(channelName, MapEntry.MapAction.ADD, innerElementList));

            innerElementList.clear();
        }

        elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList));
        elementList.clear();
        /* End setting Channel Group */

        /* Dictionary group */
        final DictionaryDataModel dictionaryData = settings.getDictionarySettings();
        if (dictionaryData.isDownloadFromNetwork()) {
            innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
        } else {
            innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
            innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", dictionaryData.getFieldDictionaryPath()));
            innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", dictionaryData.getEnumDictionaryPath()));
        }
        innerMap.add(EmaFactory.createMapEntry().keyAscii("Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map("DictionaryList", innerMap));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii("DictionaryGroup", MapEntry.MapAction.ADD, elementList));
        elementList.clear();

        return configMap;
    }

    private String getProtocolType(SpecifiedEndpointConnectionTypes connType) {
        switch (connType) {
            case SOCKET:
            case ENCRYPTED_SOCKET:
                return "RSSL_SOCKET";
            case ENCRYPTED_WEBSOCKET:
            case WEBSOCKET:
                return "RSSL_WEBSOCKET";
            default:
                return null;
        }
    }

}
