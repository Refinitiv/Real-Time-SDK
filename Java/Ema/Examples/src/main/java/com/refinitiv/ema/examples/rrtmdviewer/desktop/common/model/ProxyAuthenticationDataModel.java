package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model;

public class ProxyAuthenticationDataModel {

    private String login;

    private String password;

    private String domain;

    private String krbFilePath;

    ProxyAuthenticationDataModel(String login, String password, String domain, String krbFilePath) {
        this.login = login;
        this.password = password;
        this.domain = domain;
        this.krbFilePath = krbFilePath;
    }

    public static ProxyAuthenticationDataModelBuilder builder() {
        return new ProxyAuthenticationDataModelBuilder();
    }

    public String getLogin() {
        return login;
    }

    public String getPassword() {
        return password;
    }

    public String getDomain() {
        return domain;
    }

    public String getKrbFilePath() {
        return krbFilePath;
    }

    public static class ProxyAuthenticationDataModelBuilder {
        private String login;
        private String password;
        private String domain;
        private String krbFilePath;

        public ProxyAuthenticationDataModelBuilder login(String login) {
            this.login = login;
            return this;
        }

        public ProxyAuthenticationDataModelBuilder password(String password) {
            this.password = password;
            return this;
        }

        public ProxyAuthenticationDataModelBuilder domain(String domain) {
            this.domain = domain;
            return this;
        }

        public ProxyAuthenticationDataModelBuilder krbFilePath(String krbFilePath) {
            this.krbFilePath = krbFilePath;
            return this;
        }

        public ProxyAuthenticationDataModel build() {
            return new ProxyAuthenticationDataModel(login, password, domain, krbFilePath);
        }
    }
}
