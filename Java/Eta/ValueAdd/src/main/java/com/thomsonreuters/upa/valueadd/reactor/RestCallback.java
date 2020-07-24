package com.thomsonreuters.upa.valueadd.reactor;

/**
 * The default message callback is used to communicate message events
 * to the application.
 */
interface RestCallback
{
    public int RestResponseCallback(RestResponse response, RestEvent event);
    public int RestErrorCallback(RestEvent event, String errorText);
}
