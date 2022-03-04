/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model;

public class EncryptionDataModel {

    private String keyFilePath;

    private String keyPassword;

    public EncryptionDataModel(String keyFilePath, String keyPassword) {
        this.keyFilePath = keyFilePath;
        this.keyPassword = keyPassword;
    }

    public String getKeyFilePath() {
        return keyFilePath;
    }

    public String getKeyPassword() {
        return keyPassword;
    }

    public static EncryptionDataModelBuilder builder() {
        return new EncryptionDataModelBuilder();
    }

    public static class EncryptionDataModelBuilder {
        private String keyFilePath;
        private String keyPassword;

        public EncryptionDataModelBuilder keyFilePath(String keyFilePath) {
            this.keyFilePath = keyFilePath;
            return this;
        }

        public EncryptionDataModelBuilder keyPassword(String keyPassword) {
            this.keyPassword = keyPassword;
            return this;
        }

        public EncryptionDataModel build() {
            return new EncryptionDataModel(keyFilePath, keyPassword);
        }
    }
}
