package com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary;


/**
 * The RDM Dictionary Close. Used by an OMM Consumer to close a Dictionary
 * stream.
 * 
 * @see DictionaryMsg
 */
public interface DictionaryClose extends DictionaryMsg
{
    /**
     * Performs a deep copy of {@link DictionaryClose} object.
     * 
     * @param destCloseMsg Message to copy dictionary close object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DictionaryClose destCloseMsg);
}