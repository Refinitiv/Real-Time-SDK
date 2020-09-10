package com.rtsdk.eta.shared;

/** reasons a login request is rejected */
public enum LoginRejectReason
{
    MAX_LOGIN_REQUESTS_REACHED,
    NO_USER_NAME_IN_REQUEST,
    LOGIN_RDM_DECODER_FAILED;
}