package com.rtsdk.eta.shared;

/**
 * Consumer login states.
 */
public enum ConsumerLoginState
{
    PENDING_LOGIN, /** initial state before request is sent*/ 
    OK_SOLICITED, 
    OK_UNSOLICITED,
    CLOSED, 
    CLOSED_RECOVERABLE, 
    SUSPECT; 
}
