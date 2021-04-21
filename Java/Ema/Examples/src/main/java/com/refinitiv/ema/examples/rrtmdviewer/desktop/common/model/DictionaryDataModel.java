package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model;

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
