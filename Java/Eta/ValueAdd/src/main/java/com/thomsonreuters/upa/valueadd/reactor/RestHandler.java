package com.thomsonreuters.upa.valueadd.reactor;


import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.concurrent.FutureCallback;


class RestHandler implements FutureCallback<HttpResponse> {
	
    private ReactorChannel _userSpecObj;
    private RestReactor _restReactor;
    private RestEvent _event;
    private RestResponse _response;
    
    RestHandler(RestReactor restReactor, ReactorChannel userSpecObj)
	{
         _userSpecObj = userSpecObj;
         _restReactor = restReactor;
         _event = new RestEvent(RestEventTypes.COMPLETED, userSpecObj);
	}

	@Override
	public void completed(HttpResponse response)
	{
		_event.clear();
		_event.eventType(RestEventTypes.COMPLETED);
		_event.userSpecObj(_userSpecObj);
		_response = new RestResponse();
		if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
		{			
			RestReactor.convertResponse(_restReactor, response, _response, _event);
			_event.eventType(RestEventTypes.FAILED);
		}
		else
		{
			RestReactor.convertResponse(_restReactor, response, _response, _event);
		}
			
		RestReactor.processResponse(_restReactor, _response, _event);

	}

	@Override
	public void failed(Exception ex)
	{
		_event.clear();
		_event.eventType(RestEventTypes.FAILED);
		_event.userSpecObj(_userSpecObj);
		
		if(_userSpecObj.sessionMgntState() == ReactorChannel.SessionMgntState.QUERYING_SERVICE_DISCOVERY)
		{
			RestReactor.populateErrorInfo(_event.errorInfo(),
	                ReactorReturnCodes.FAILURE,
	                "RestHandler.failed", "Failed REST request for the service discovery. Text: " + ex.getLocalizedMessage());
		}
		else
		{
			RestReactor.populateErrorInfo(_event.errorInfo(),
	                ReactorReturnCodes.FAILURE,
	                "RestHandler.failed", "Failed REST request for the token service. Text: " + ex.getLocalizedMessage());
		}
		
		if (_restReactor.reactorOptions().defaultRespCallback() != null)
		{
			_restReactor.reactorOptions().defaultRespCallback().RestErrorCallback(_event);
		}
	}

	@Override
	public void cancelled()
	{
		_event.clear();
		_event.eventType(RestEventTypes.CANCELLED);
		_event.userSpecObj(_userSpecObj);
		RestReactor.populateErrorInfo(_event.errorInfo(),
                ReactorReturnCodes.FAILURE,
                "RestHandler.cancelled", "Cancelled REST request.");
		
		if (_restReactor.reactorOptions().defaultRespCallback() != null)
		{
			_restReactor.reactorOptions().defaultRespCallback().RestErrorCallback(_event);
		}
	}
	
    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return (_restReactor == null ? "_RestReactor null" : _restReactor)
                + ", "
                + (_userSpecObj == null ? "_userSpecObj null" : _userSpecObj)
                + ", "
                + (_event == null ? "_event null" : _event.toString())
                + ", "
                + (_response == null ? "_response null" : _response.toString());
    }
}
