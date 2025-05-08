/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common.model;

public class DictionaryDataModel {

    private boolean downloadFromNetwork;

    private String fieldDictionaryPath;

    private String enumDictionaryPath;

    public DictionaryDataModel(String fieldDictionaryPath, String enumDictionaryPath, boolean downloadFromNetwork) {
        this.fieldDictionaryPath = fieldDictionaryPath;
        this.enumDictionaryPath = enumDictionaryPath;
        this.downloadFromNetwork = downloadFromNetwork;
    }

    public String getFieldDictionaryPath() {
        return fieldDictionaryPath;
    }

    public String getEnumDictionaryPath() {
        return enumDictionaryPath;
    }

    public boolean isDownloadFromNetwork() {
        return downloadFromNetwork;
    }

    public static DictionaryDataModelBuilder builder() {
        return new DictionaryDataModelBuilder();
    }

    public static class DictionaryDataModelBuilder {
        private String fieldDictionaryPath;
        private String enumDictionaryPath;
        private boolean downloadFromNetwork;

        public DictionaryDataModelBuilder fieldDictionaryPath(String dictionaryPath) {
            this.fieldDictionaryPath = dictionaryPath;
            return this;
        }

        public DictionaryDataModelBuilder enumDictionaryPath(String enumPath) {
            this.enumDictionaryPath = enumPath;
            return this;
        }

        public DictionaryDataModelBuilder downloadFromNetwork(boolean value) {
            this.downloadFromNetwork = value;
            return this;
        }

        public DictionaryDataModel build() {
            return new DictionaryDataModel(fieldDictionaryPath, enumDictionaryPath, downloadFromNetwork);
        }
    }
}
