package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.FieldEntry;

import java.io.IOException;
import java.util.Objects;

public class XmlMsgData {
    class XmlMsgDataReader extends XMLReader {

        private final ItemEncoder itemEncoder = new ItemEncoder();

        /**
         * XML Pull Parser beginSectionRead().
         */
        protected void beginSectionRead() throws IOException {
            String tag = XPP.getName();

            if (tag.equals("marketPriceMsgList")) {
                // get msg counts (updates, posts, and generic msgs)
                getFileMessageCounts();

                marketPriceUpdateMsgs = new MarketPriceMsg[fileUpdateCount];
                marketPricePostMsgs = new MarketPriceMsg[filePostCount];
                marketPriceGenMsgs = new MarketPriceMsg[fileGenMsgCount];
                parsingMPMsgs = true;

                hasMarketPrice = true;
            }

            if (parsingMPMsgs) {
                if (tag.equals("refreshMsg")) {
                    parsingRefreshMsg = true;
                } else if (tag.equals("updateMsg") && XPP.getDepth() == 3) {
                    parsingUpdateMsg = true;
                } else if (tag.equals("postMsg")) {
                    parsingPostMsg = true;
                } else if (tag.equals("genMsg")) {
                    parsingGenMsg = true;
                } else if (tag.equals("fieldList")) {
                    // get field entry count
                    getFieldEntryCount();

                    if (parsingRefreshMsg) {
                        marketPriceRefreshMsg = new MarketPriceMsg(fileFieldEntryCount);
                    } else if (parsingUpdateMsg) {
                        updateMsg = new MarketPriceMsg(fileFieldEntryCount);
                    } else if (parsingPostMsg) {
                        postMsg = new MarketPriceMsg(fileFieldEntryCount);
                    } else if (parsingGenMsg) {
                        genMsg = new MarketPriceMsg(fileFieldEntryCount);
                    }
                } else if (tag.equals("fieldEntry")) {
                    if (getAttributeCount() == 0)
                        return;

                    // get field entry information
                    MarketField marketField = getFieldEntryInfo();

                    // add market field to appropriate message
                    if (parsingRefreshMsg) {
                        if (fieldCount < marketPriceRefreshMsg.fieldEntries().length) {
                            marketPriceRefreshMsg.fieldEntries()[fieldCount++] = marketField;
                        }
                    } else if (parsingUpdateMsg) {
                        if (fieldCount < updateMsg.fieldEntries().length) {
                            updateMsg.fieldEntries()[fieldCount++] = marketField;
                        }
                    } else if (parsingPostMsg) {
                        if (fieldCount < postMsg.fieldEntries().length) {
                            postMsg.fieldEntries()[fieldCount++] = marketField;
                        }
                    } else if (parsingGenMsg) {
                        if (fieldCount < genMsg.fieldEntries().length) {
                            genMsg.fieldEntries()[fieldCount++] = marketField;
                        }
                    }
                }
            }
        }

        /**
         * XML Pull Parser endSectionRead().
         */
        void endSectionRead() throws IOException {
            String tag = XPP.getName();

            if (tag.equals("marketPriceMsgList")) {
                parsingMPMsgs = false;
            }

            if (parsingMPMsgs) {
                if (tag.equals("refreshMsg")) {
                    marketPriceRefreshMsg.fieldEntryCount(fieldCount);
                    fieldCount = 0;
                    parsingRefreshMsg = false;
                } else if (tag.equals("updateMsg") && XPP.getDepth() == 3) {
                    updateMsg.fieldEntryCount(fieldCount);
                    if (updateCount < marketPriceUpdateMsgs.length) {
                        marketPriceUpdateMsgs[updateCount++] = updateMsg;
                    }
                    fieldCount = 0;
                    parsingUpdateMsg = false;
                } else if (tag.equals("postMsg")) {
                    postMsg.fieldEntryCount(fieldCount);
                    if (postCount < marketPricePostMsgs.length) {
                        marketPricePostMsgs[postCount++] = postMsg;
                    }
                    fieldCount = 0;
                    parsingPostMsg = false;
                } else if (tag.equals("genMsg")) {
                    genMsg.fieldEntryCount(fieldCount);
                    if (genMsgCount < marketPriceGenMsgs.length) {
                        marketPriceGenMsgs[genMsgCount++] = genMsg;
                    }
                    fieldCount = 0;
                    parsingGenMsg = false;
                }
            }
        }

        /* Get msg counts (updates, posts, and generic msgs). */
        private void getFileMessageCounts() {
            for (int i = 0; i < xmlReader.getAttributeCount(); i++) {
                String ntag = XPP.getAttributeName(i);
                String nvalue = XPP.getAttributeValue(i);

                if (ntag.equals("updateMsgCount")) {
                    fileUpdateCount = Integer.parseInt(nvalue);
                } else if (ntag.equals("postMsgCount")) {
                    filePostCount = Integer.parseInt(nvalue);
                } else if (ntag.equals("genMsgCount")) {
                    fileGenMsgCount = Integer.parseInt(nvalue);
                }
            }
        }

        /* Get field entry count. */
        private void getFieldEntryCount() {
            for (int i = 0; i < getAttributeCount(); i++) {
                String ntag = XPP.getAttributeName(i);
                String nvalue = XPP.getAttributeValue(i);

                if (ntag.equals("entryCount")) {
                    fileFieldEntryCount = Integer.parseInt(nvalue);
                }
            }
        }

        /* Get field entry information. */
        private MarketField getFieldEntryInfo() {
            int fieldId = 0, dataTypeValue = 0;
            String value = null;
            QosWrapper qosWrapper = null;
            StateWrapper stateWrapper = null;
            for (int i = 0; i < getAttributeCount(); i++) {
                String ntag = XPP.getAttributeName(i);
                String nvalue = XPP.getAttributeValue(i);

                if (ntag.equals("fieldId")) {
                    fieldId = Integer.parseInt(nvalue);
                } else if (ntag.equals("dataType")) {
                    dataTypeValue = dataTypeValue(nvalue);
                    if (dataTypeValue == DataType.DataTypes.QOS) {
                        qosWrapper = new QosWrapper();
                    } else if (dataTypeValue == DataType.DataTypes.STATE) {
                        stateWrapper = new StateWrapper();
                    }
                } else if (ntag.equals("data")) {
                    value = nvalue;
                } else if (Objects.nonNull(qosWrapper)) {
                    if (ntag.equals("qosRate") && Objects.nonNull(qosWrapper)) {
                        qosWrapper.rate(nvalue);
                    } else if (ntag.equals("qosRateInfo") && Objects.nonNull(qosWrapper)) {
                        qosWrapper.rateInfo(nvalue);
                    } else if (ntag.equals("qosTimeliness") && Objects.nonNull(qosWrapper)) {
                        qosWrapper.timeliness(nvalue);
                    } else if (ntag.equals("qosTimeInfo") && Objects.nonNull(qosWrapper)) {
                        qosWrapper.timeInfo(nvalue);
                    } else if (ntag.equals("qosDynamic") && Objects.nonNull(qosWrapper)) {
                        qosWrapper.dynamic(nvalue);
                    }
                } else if (Objects.nonNull(stateWrapper)) {
                    if (ntag.equals("streamState")) {
                        stateWrapper.streamState(nvalue);
                    } else if (ntag.equals("dataState")) {
                        stateWrapper.dataState(nvalue);
                    } else if (ntag.equals("code")) {
                        stateWrapper.statusCode(nvalue);
                    } else if (ntag.equals("text")) {
                        stateWrapper.statusText(nvalue);
                    }
                }
            }

            MarketField marketField = new MarketField(
                    fieldId, dataTypeValue, value, qosWrapper, stateWrapper
            );
            FieldEntry fieldEntry = itemEncoder.loadPrimitive(marketField);
            marketField.setFieldEntry(fieldEntry);
            return marketField;
        }

        /* Converts data type string to data type value. */
        private int dataTypeValue(String dataTypeString) {
            int retVal = 0;

            if (dataTypeString.equals("RSSL_DT_INT")) {
                retVal = DataType.DataTypes.INT;
            } else if (dataTypeString.equals("RSSL_DT_UINT")) {
                retVal = DataType.DataTypes.UINT;
            } else if (dataTypeString.equals("RSSL_DT_FLOAT")) {
                retVal = DataType.DataTypes.FLOAT;
            } else if (dataTypeString.equals("RSSL_DT_DOUBLE")) {
                retVal = DataType.DataTypes.DOUBLE;
            } else if (dataTypeString.equals("RSSL_DT_REAL")) {
                retVal = DataType.DataTypes.REAL;
            } else if (dataTypeString.equals("RSSL_DT_DATE")) {
                retVal = DataType.DataTypes.DATE;
            } else if (dataTypeString.equals("RSSL_DT_TIME")) {
                retVal = DataType.DataTypes.TIME;
            } else if (dataTypeString.equals("RSSL_DT_DATETIME")) {
                retVal = DataType.DataTypes.DATETIME;
            } else if (dataTypeString.equals("RSSL_DT_QOS")) {
                retVal = DataType.DataTypes.QOS;
            } else if (dataTypeString.equals("RSSL_DT_STATE")) {
                retVal = DataType.DataTypes.STATE;
            } else if (dataTypeString.equals("RSSL_DT_ENUM")) {
                retVal = DataType.DataTypes.ENUM;
            } else if (dataTypeString.equals("RSSL_DT_BUFFER")) {
                retVal = DataType.DataTypes.BUFFER;
            } else if (dataTypeString.equals("RSSL_DT_ASCII_STRING")) {
                retVal = DataType.DataTypes.ASCII;
            } else if (dataTypeString.equals("RSSL_DT_UTF8_STRING")) {
                retVal = DataType.DataTypes.UTF8;
            } else if (dataTypeString.equals("RSSL_DT_RMTES_STRING")) {
                retVal = DataType.DataTypes.RMTES;
            }

            return retVal;
        }
    }

    private final int MAX_UPDATE_MSGS = 10;
    private final int MAX_POST_MSGS = 10;
    private final int MAX_GEN_MSGS = 10;
    private final int MAX_FIELD_ENTRIES = 100;

    public XmlMsgDataReader xmlReader = new XmlMsgDataReader();
    private MarketPriceMsg marketPriceRefreshMsg; /* market price refresh message */
    private MarketPriceMsg updateMsg; /* market price update message */
    private MarketPriceMsg postMsg; /* market price post message */
    private MarketPriceMsg genMsg; /* market price generic message */
    private MarketPriceMsg[] marketPriceUpdateMsgs; /* array of market price update messages */
    private MarketPriceMsg[] marketPricePostMsgs; /* array of market price post messages */
    private MarketPriceMsg[] marketPriceGenMsgs; /* array of market price generic messages */

    private int updateCount; /* current update message count */
    private int postCount; /* current post message count */
    private int genMsgCount; /* current generic message count */
    private int fieldCount; /* current field entry count */
    private int fileUpdateCount; /* update message count from file */
    private int filePostCount; /* post message count from file */
    private int fileGenMsgCount; /* generic message count from file */
    private int fileFieldEntryCount; /* field entry count from file */

    private boolean parsingMPMsgs; /* flag to indicate currently parsing market price messages */
    private boolean parsingRefreshMsg; /* flag to indicate currently parsing a refresh message */
    private boolean parsingUpdateMsg; /* flag to indicate currently parsing an update message */
    private boolean parsingPostMsg; /* flag to indicate currently parsing a post message */
    private boolean parsingGenMsg; /* flag to indicate currently parsing a generic message */

    private boolean hasMarketPrice;

    /**
     * Initialize the list.
     */
    public XmlMsgData() {
        fileUpdateCount = MAX_UPDATE_MSGS;
        filePostCount = MAX_POST_MSGS;
        fileGenMsgCount = MAX_GEN_MSGS;
        fileFieldEntryCount = MAX_FIELD_ENTRIES;
    }

    /**
     * Parses xml message data file.
     *
     * @param filename the filename
     * @return {@link PerfToolsReturnCodes}
     */
    public int parseFile(String filename) {
        return xmlReader.parseFile(filename);
    }

    /**
     * Market price refresh message.
     *
     * @return the market price msg
     */
    public MarketPriceMsg marketPriceRefreshMsg() {
        return marketPriceRefreshMsg;
    }

    /**
     * Array of market price update messages.
     *
     * @return the market price msg[]
     */
    public MarketPriceMsg[] marketPriceUpdateMsgs() {
        return marketPriceUpdateMsgs;
    }

    /**
     * Array of market price post messages.
     *
     * @return the market price msg[]
     */
    public MarketPriceMsg[] marketPricePostMsgs() {
        return marketPricePostMsgs;
    }

    /**
     * Array of market price generic messages.
     *
     * @return the market price msg[]
     */
    public MarketPriceMsg[] marketPriceGenMsgs() {
        return marketPriceGenMsgs;
    }

    /**
     * Market price update message count.
     *
     * @return the int
     */
    public int marketPriceUpdateMsgCount() {
        return updateCount;
    }

    /**
     * Market price post message count.
     *
     * @return the int
     */
    public int marketPricePostMsgCount() {
        return postCount;
    }

    /**
     * Market price generic message count.
     *
     * @return the int
     */
    public int marketPriceGenMsgCount() {
        return genMsgCount;
    }

    /**
     * Checks for market price.
     *
     * @return true, if there is market data in the xml.
     */
    public boolean hasMarketPrice() {
        return hasMarketPrice;
    }
}
