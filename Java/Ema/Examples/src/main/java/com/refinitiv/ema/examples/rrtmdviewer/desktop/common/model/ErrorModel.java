package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model;

public class ErrorModel {
    private String header;

    private String content;

    public ErrorModel() {
    }

    public String getHeader() {
        return header;
    }

    public void setHeader(String header) {
        this.header = header;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public void clear() {
        header = "";
        content = "";
    }
}
