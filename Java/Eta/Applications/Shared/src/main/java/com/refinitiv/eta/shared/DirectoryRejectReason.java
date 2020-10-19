package com.refinitiv.eta.shared;

/** Reasons a directory request is rejected */
public enum DirectoryRejectReason
{
    MAX_SRCDIR_REQUESTS_REACHED,
    INCORRECT_FILTER_FLAGS,
    DIRECTORY_RDM_DECODER_FAILED;
}
