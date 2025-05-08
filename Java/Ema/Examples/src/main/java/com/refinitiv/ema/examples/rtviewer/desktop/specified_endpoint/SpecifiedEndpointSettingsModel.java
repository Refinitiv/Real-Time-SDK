/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.specified_endpoint;

import com.refinitiv.ema.examples.rtviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.ConnectionDataModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.DictionaryDataModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EmaConfigModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EncryptionDataModel;
import com.refinitiv.eta.codec.CodecReturnCodes;

import java.util.ArrayList;
import java.util.List;

public class SpecifiedEndpointSettingsModel {

    private List<String> port;
    private List<String> host;

    private boolean customEncryptionOptions;

    private ConnectionDataModel connectionSettings;
    private EncryptionDataModel encryptionSettings;
    private DictionaryDataModel dictionaryData;

    private String applicationId;
    private String position;
    private String username;

    private EmaConfigModel emaConfigModel;
    
    private boolean isTLSv12Enabled;
    private boolean isTLSv13Enabled;

    private SpecifiedEndpointSettingsModel(List<String> port, List<String> host, boolean customEncr, ConnectionDataModel connectionSettings,
                                           EncryptionDataModel encryptionSettings, String appId, String position, String username,
                                           DictionaryDataModel dictionaryData, EmaConfigModel emaConfigModel, boolean isTLSv12Enabled, boolean isTLSv13Enabled) {
        this.port = port;
        this.host = host;
        this.customEncryptionOptions = customEncr;
        this.connectionSettings = connectionSettings;
        this.encryptionSettings = encryptionSettings;
        this.applicationId = appId;
        this.position = position;
        this.username = username;
        this.dictionaryData = dictionaryData;
        this.emaConfigModel = emaConfigModel;
        this.isTLSv12Enabled = isTLSv12Enabled;
        this.isTLSv13Enabled = isTLSv13Enabled;
    }

    public List<String> getHost() { return host; }

    public List<String> getPort() { return port; }

    public boolean hasCustomEncrOptions() { return customEncryptionOptions; }

    public ConnectionDataModel getConnectionDataModel() { return connectionSettings; }

    public EncryptionDataModel getEncryptionSettings() { return encryptionSettings; }

    public DictionaryDataModel getDictionarySettings() {
        return dictionaryData;
    }

    public String getApplicationId() { return applicationId; }

    public String getPosition() { return position; }

    public String getUsername() { return username; }

    public EmaConfigModel getEmaConfigModel() {
        return emaConfigModel;
    }
    
    public boolean isTLSv12Enabled() { return isTLSv12Enabled; }
    
    public boolean isTLSv13Enabled() { return isTLSv13Enabled; }

    public int validate(OMMViewerError error) {
        error.clear();
        int ret = CodecReturnCodes.SUCCESS;

        if( host.size() == 0 ) {
            error.appendErrorText("Error: Host and Port fields cannot be empty.");
            ret = CodecReturnCodes.FAILURE;
        }

        if (connectionSettings.getProtocolList().trim().isEmpty()) {
            error.appendErrorText("At least one protocol must be chosen.");
            ret = CodecReturnCodes.FAILURE;
        }

        error.setFailed(ret == CodecReturnCodes.FAILURE);

        return ret;
    }

    public static SpecifiedEndpointSettingsModelBuilder  builder() {
        return new SpecifiedEndpointSettingsModelBuilder();
    }

    public static class SpecifiedEndpointSettingsModelBuilder {
        private List<String> portList = new ArrayList<>();
        private List<String> hostList = new ArrayList<>();

        private boolean hasCustomEncrOptions;

        private ConnectionDataModel connectionSettings;
        private EncryptionDataModel encryptionSettings;
        private DictionaryDataModel dictionaryData;

        private String applicationId;
        private String position;
        private String username;

        private EmaConfigModel emaConfigModel;
        
        private boolean isTLSv12Enabled;
        private boolean isTLSv13Enabled;

        public SpecifiedEndpointSettingsModelBuilder addServer(String host, String port) {
            hostList.add(host);
            portList.add(port);
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder hasCustomEncrOptions(boolean value) {
            this.hasCustomEncrOptions = value;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder applicationId(String appId) {
            this.applicationId = appId;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder position(String position) {
            this.position = position;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder username(String username) {
            this.username = username;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder dictionaryData(DictionaryDataModel dictionaryModel) {
            this.dictionaryData = dictionaryModel;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder encryptionSettings(EncryptionDataModel settings) {
            this.encryptionSettings = settings;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder connectionSettings(ConnectionDataModel settings) {
            this.connectionSettings = settings;
            return this;
        }

        public SpecifiedEndpointSettingsModelBuilder useEmaConfigFile(EmaConfigModel emaConfigModel) {
            this.emaConfigModel = emaConfigModel;
            return this;
        }
        
        public SpecifiedEndpointSettingsModelBuilder isTLSv12Enabled(boolean isTLSv12Enabled) {
        	this.isTLSv12Enabled = isTLSv12Enabled;
        	return this;
        }
        
        public SpecifiedEndpointSettingsModelBuilder isTLSv13Enabled(boolean isTLSv13Enabled) {
        	this.isTLSv13Enabled = isTLSv13Enabled;
        	return this;
        }

        public SpecifiedEndpointSettingsModel build() {
            return new SpecifiedEndpointSettingsModel(portList, hostList, hasCustomEncrOptions, connectionSettings, encryptionSettings,
                    applicationId, position, username, dictionaryData, emaConfigModel, isTLSv12Enabled, isTLSv13Enabled);
        }
    }


}
