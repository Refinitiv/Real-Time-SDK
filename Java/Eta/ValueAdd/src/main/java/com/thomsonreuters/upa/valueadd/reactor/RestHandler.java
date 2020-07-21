package com.thomsonreuters.upa.valueadd.reactor;


import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.concurrent.FutureCallback;


class RestHandler implements FutureCallback<HttpResponse> {
	
    private RestResultClosure _resultClosure;
    private RestReactor _restReactor;
    private RestEvent _event;
    private RestResponse _response;
    
    RestHandler(RestReactor restReactor, RestResultClosure resultClosure)
	{
         _resultClosure = resultClosure;
         _restReactor = restReactor;
         _event = new RestEvent(RestEventTypes.COMPLETED, resultClosure);
	}

	@Override
	public void completed(HttpResponse response)
	{
		_event.clear();
		_event.eventType(RestEventTypes.COMPLETED);
		_event.resultClosure(_resultClosure);
		_response = new RestResponse();
		if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
		{			
			RestReactor.convertResponse(_restReactor, response, _response, _event.errorInfo());
			_event.eventType(RestEventTypes.FAILED);
		}
		else
		{
			RestReactor.convertResponse(_restReactor, response, _response, _event.errorInfo());
		}
			
		RestReactor.processResponse(_restReactor, _response, _event);

	}

	@Override
	public void failed(Exception ex)
	{
		_event.clear();
		_event.eventType(RestEventTypes.FAILED);
		_resultClosure.restCallback().RestErrorCallback(_event, ex.getLocalizedMessage());
	}

	@Override
	public void cancelled()
	{
		_event.clear();
		_event.eventType(RestEventTypes.CANCELLED);
		_event.resultClosure(_resultClosure);
		RestReactor.populateErrorInfo(_event.errorInfo(),
                ReactorReturnCodes.FAILURE,
                "RestHandler.cancelled", "Cancelled REST request.");
		
		_resultClosure.restCallback().RestErrorCallback(_event,  _event.errorInfo().error().text());
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
                + (_resultClosure == null ? "_resultClosure null" : _resultClosure)
                + ", "
                + (_event == null ? "_event null" : _event.toString())
                + ", "
                + (_response == null ? "_response null" : _response.toString());
    }
}
