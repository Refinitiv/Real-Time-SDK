package com.refinitiv.eta.valueadd.domainrep.rdm.directory;


/**
 * The RDM Directory Close.  Used by an OMM Consumer to close an open Source Directory stream.
 * 
 * @see DirectoryMsg
 */
public interface DirectoryClose extends DirectoryMsg
{
    /**
     * Performs a deep copy of {@link DirectoryClose} object.
     *
     * @param destCloseMsg Message to copy directory close object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryClose destCloseMsg);
}