package com.refinitiv.ema.perftools.common;


import com.refinitiv.ema.access.FieldEntry;

public class MarketField {
    private final int fieldId;                // The market field id.
    private final int loadType;               // The market field load type.
    private final String value;               // The market field init value.
    private final QosWrapper qosWrapper;
    private final StateWrapper stateWrapper;
    private final boolean blank;
    private FieldEntry fieldEntry;

    /**
     * Instantiates a new market field.
     */
    public MarketField(int fieldId, int loadType, String value, QosWrapper qosWrapper, StateWrapper stateWrapper) {
        this.fieldId = fieldId;
        this.loadType = loadType;
        this.value = value;
        this.qosWrapper = qosWrapper;
        this.stateWrapper = stateWrapper;
        this.blank = (value == null || value.isEmpty());
    }

    public int fieldId() {
        return fieldId;
    }

    public int loadType() {
        return loadType;
    }

    /**
     * The market field value.
     *
     * @return the object
     */
    public String value() {
        return value;
    }

    public QosWrapper qos() {
        return qosWrapper;
    }

    public StateWrapper state() {
        return stateWrapper;
    }

    public FieldEntry fieldEntry() {
        return fieldEntry;
    }

    public void setFieldEntry(FieldEntry fieldEntry) {
        this.fieldEntry = fieldEntry;
    }

    public boolean isBlank() {
        return blank;
    }
}
