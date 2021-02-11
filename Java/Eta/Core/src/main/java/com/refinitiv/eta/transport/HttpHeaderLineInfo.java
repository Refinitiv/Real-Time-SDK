package com.refinitiv.eta.transport;

class HttpHeaderLineInfo {

    private static final char DEFAULT_VALUE_DELIMITER = ',';

    private int lineNumber;

    private int dataLength;

    private int startOffset;

    private String headerValue;

    private char valueDelimiter;

    private HttpHeaderLineInfo(String headerValue) {
        this.headerValue = headerValue;
    }

    private HttpHeaderLineInfo(String headerValue, int lineNumber, int dataLength, int startOffset, char valueDelimiter) {
        this.headerValue = headerValue;
        this.lineNumber = lineNumber;
        this.dataLength = dataLength;
        this.startOffset = startOffset;
        this.valueDelimiter = valueDelimiter;
    }

    public HttpHeaderLineInfo(HttpHeaderLineInfo headerLineInfo) {
        this.lineNumber = headerLineInfo.lineNumber;
        this.dataLength = headerLineInfo.dataLength;
        this.startOffset = headerLineInfo.startOffset;
        this.headerValue = headerLineInfo.headerValue;
        this.valueDelimiter = headerLineInfo.valueDelimiter;
    }

    public int getLineNumber() {
        return lineNumber;
    }

    public int getDataLength() {
        return dataLength;
    }

    public int getStartOffset() {
        return startOffset;
    }

    public String getHeaderValue() {
        return headerValue;
    }

    public char getValueDelimiter() {
        return valueDelimiter;
    }

    public void setValueDelimiter(char valueDelimiter) {
        this.valueDelimiter = valueDelimiter;
    }

    public static HttpHeaderLineInfoBuilder builder() {
        return new HttpHeaderLineInfoBuilder();
    }

    public static HttpHeaderLineInfo valueOf(Object value) {
        assert value != null;
        return new HttpHeaderLineInfo(value.toString());
    }

    static class HttpHeaderLineInfoBuilder {
        private int lineNumber;

        private int dataLength;

        private int startOffset;

        private String headerValue;

        private char valueDelimiter = DEFAULT_VALUE_DELIMITER;

        public HttpHeaderLineInfoBuilder lineNumber(int lineNumber) {
            this.lineNumber = lineNumber;
            return this;
        }

        public HttpHeaderLineInfoBuilder dataLength(int dataLength) {
            this.dataLength = dataLength;
            return this;
        }

        public HttpHeaderLineInfoBuilder startOffset(int startOffset) {
            this.startOffset = startOffset;
            return this;
        }

        public HttpHeaderLineInfoBuilder headerValue(String headerValue) {
            this.headerValue = headerValue;
            return this;
        }

        public HttpHeaderLineInfoBuilder valueDelimiter(char valueDelimiter) {
            this.valueDelimiter = valueDelimiter;
            return this;
        }

        public HttpHeaderLineInfo build() {
            return new HttpHeaderLineInfo(headerValue, lineNumber, dataLength, startOffset, valueDelimiter);
        }
    }
}