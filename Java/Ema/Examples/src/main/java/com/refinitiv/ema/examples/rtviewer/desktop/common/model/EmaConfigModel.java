/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common.model;

public class EmaConfigModel {

    private final boolean useEmaConfig;

    private final String emaConfigFilePath;

    private final String consumerName;

    public EmaConfigModel(boolean useEmaConfig, String emaConfigFilePath, String consumerName) {
        this.useEmaConfig = useEmaConfig;
        this.emaConfigFilePath = emaConfigFilePath;
        this.consumerName = consumerName;
    }

    public boolean isUseEmaConfig() {
        return useEmaConfig;
    }

    public String getEmaConfigFilePath() {
        return emaConfigFilePath;
    }

    public String getConsumerName() {
        return consumerName;
    }

    public static EmaConfigModelBuilder builder() {
        return new EmaConfigModelBuilder();
    }

    public static class EmaConfigModelBuilder {
        private boolean useEmaConfig;
        private String emaConfigFilePath;
        private String consumerName;

        public EmaConfigModelBuilder useEmaConfig(boolean useEmaConfig) {
            this.useEmaConfig = useEmaConfig;
            return this;
        }

        public EmaConfigModelBuilder emaConfigFilePath(String emaConfigFilePath) {
            this.emaConfigFilePath = emaConfigFilePath;
            return this;
        }

        public EmaConfigModelBuilder consumerName(String consumerName) {
            this.consumerName = consumerName;
            return this;
        }

        public EmaConfigModel build() {
            return new EmaConfigModel(useEmaConfig, emaConfigFilePath, consumerName);
        }
    }
}
