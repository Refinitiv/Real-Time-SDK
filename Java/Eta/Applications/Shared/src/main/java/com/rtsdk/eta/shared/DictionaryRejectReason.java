package com.rtsdk.eta.shared;

/**
 * Reasons a dictionary request is rejected.
 *
 */
public enum DictionaryRejectReason
{
    UNKNOWN_DICTIONARY_NAME,
    MAX_DICTIONARY_REQUESTS_REACHED,
    DICTIONARY_RDM_DECODER_FAILED;
}
