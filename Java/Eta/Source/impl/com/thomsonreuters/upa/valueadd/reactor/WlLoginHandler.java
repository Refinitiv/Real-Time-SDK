package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;

/* The login stream handler for the watchlist. */
class WlLoginHandler implements WlHandler 
{
	Watchlist _watchlist;
	WlStream _stream;
	int _loginStreamId;
	EncodeIterator _eIter = CodecFactory.createEncodeIterator();
	DecodeIterator _dIter = CodecFactory.createDecodeIterator();
	LoginRequest _loginRequest;
	LoginRequest _tempLoginRequest;
	ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
	ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();
	Msg _tempMsg = CodecFactory.createMsg();
	LoginRefresh _loginRefresh;
	LoginStatus _loginStatus;
	StatusMsg _statusMsg;
	Buffer _tempBuffer;
	boolean _awaitingResumeAll;
	boolean _userloginStreamOpen;
	int _requestCount; // tracks pending requests so re-issues aren't sent until refresh is received

	WlInteger _tempWlInteger = ReactorFactory.createWlInteger();

	WlLoginHandler(Watchlist watchlist)
	{
		_watchlist = watchlist;
		_loginRefresh = (LoginRefresh) LoginMsgFactory.createMsg();
		_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		_loginStatus = (LoginStatus) LoginMsgFactory.createMsg();
		_loginStatus.rdmMsgType(LoginMsgType.STATUS);
		_statusMsg = (StatusMsg) CodecFactory.createMsg();
		_statusMsg.msgClass(MsgClasses.STATUS);
		_statusMsg.domainType(DomainTypes.LOGIN);
		_tempBuffer = CodecFactory.createBuffer();
		_tempBuffer.data("");
		_tempLoginRequest = (LoginRequest) LoginMsgFactory.createMsg();
		_tempLoginRequest.rdmMsgType(LoginMsgType.REQUEST);
		_userloginStreamOpen = true;

		// get next id for login stream from watchlist
		_loginStreamId = _watchlist.nextStreamId();
	}

	@Override
	public int submitRequest(WlRequest wlRequest, RequestMsg requestMsg,
			boolean isReissue, ReactorSubmitOptions submitOptions,
			ReactorErrorInfo errorInfo) 
	{
		int ret;

		// check for different login stream id
		// user is allowed to open a different stream if login stream is closed
		if (_loginRequest != null
				&& requestMsg.streamId() != _loginRequest.streamId() && _stream != null
				&& _stream.state().streamState() != StreamStates.CLOSED) 
		{
			// cannot have more than one login stream
			return _watchlist
					.reactor()
					.populateErrorInfo(errorInfo,
							ReactorReturnCodes.INVALID_USAGE,
							"WlLoginHandler.submitRequest",
							"Cannot have more than one login stream with watchlist enabled.");
		}

		WlInteger wlInteger = ReactorFactory.createWlInteger();
		wlInteger.value(requestMsg.streamId());

		if (_stream == null) 
		{
			if (!_watchlist.streamIdtoWlStreamTable().containsKey(wlInteger)) 
			{
				// create stream
				_stream = ReactorFactory.createWlStream();
				_stream.handler(this);
				_stream.watchlist(_watchlist);
				_stream.streamId(requestMsg.streamId());
				_stream.domainType(requestMsg.domainType());
			}
			else // stream already exists with this id
			{
				return _watchlist.reactor().populateErrorInfo(
						errorInfo,
						ReactorReturnCodes.INVALID_USAGE,
						"WlLoginHandler.submitRequest",
						"Stream already exists with id of "
								+ requestMsg.streamId() + ".");
			}
		}

		if (isReissue) // subsequent login request
		{
			// convert to rdm login request
			_tempLoginRequest.clear();
			_watchlist.convertCodecToRDMMsg(requestMsg, _tempLoginRequest);
			// handle reissue
			if ((ret = handleReissue(_tempLoginRequest, errorInfo)) < ReactorReturnCodes.SUCCESS) {
				return ret;
			}
			// copy to official login request
			_loginRequest.clear();
			_loginRequest.rdmMsgType(LoginMsgType.REQUEST);
			
			_tempLoginRequest.copy(_loginRequest);
			
			// if pause not supported, remove flag from requestMsg
			if (!(_loginRefresh.checkHasFeatures() && _loginRefresh.features().checkHasSupportOptimizedPauseResume()
					&& _loginRefresh.features().supportOptimizedPauseResume() == 1) && requestMsg.checkPause())
				{
					requestMsg.flags(requestMsg.flags() & ~RequestMsgFlags.PAUSE);
				}
		} 
		else // first login request
		{
			// convert to rdm login request and save
			if (_loginRequest == null) {
				_loginRequest = (LoginRequest) LoginMsgFactory.createMsg();
			} else {
				_loginRequest.clear();
			}

			_loginRequest.rdmMsgType(LoginMsgType.REQUEST);
			_watchlist.convertCodecToRDMMsg(requestMsg, _loginRequest);
			
			// if pause is set, remove pause as we do not send it in login request
			if (requestMsg.checkPause())
			{
				requestMsg.flags(requestMsg.flags() & ~RequestMsgFlags.PAUSE);
			}
		}

		// send message if request not pending
		if (_requestCount == 0)
		{
			if(_stream.isChannelUp())
			{
				if ((ret = _stream.sendMsg(requestMsg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS) 
				{
					if (!isReissue) {
						_loginRequest = null;
						_stream.returnToPool();
						_stream = null;
					}
					return ret;
				}
			}
		}
		if (!requestMsg.checkNoRefresh() && _stream.isChannelUp())
		{
			_requestCount++;
		}

		// save stream info
		if (!isReissue)
		{
			// only save service name when not a reissue
			wlRequest.streamInfo().serviceName(submitOptions.serviceName());

			// add stream to watchlist table
			_stream.tableKey(wlInteger);
			_watchlist.streamIdtoWlStreamTable().put(wlInteger, _stream);
		} 
		else
		{
			wlInteger.returnToPool();
		}
		wlRequest.streamInfo().userSpecObject(
				submitOptions.requestMsgOptions().userSpecObj());

		return ReactorReturnCodes.SUCCESS;
	}

	@Override
	public int submitMsg(WlRequest wlRequest, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo) 
	{
		int ret;

		switch (msg.msgClass()) 
		{
		case MsgClasses.CLOSE:
			// notify other handlers that login stream is closed
			int ret1 = _watchlist.directoryHandler().loginStreamClosed();
			int ret2 = _watchlist.itemHandler().loginStreamClosed(msg);

			if (ret1 < ReactorReturnCodes.SUCCESS) 
			{
				return ret1;
			} else if (ret2 < ReactorReturnCodes.SUCCESS) 
			{
				return ret2;
			}

			// send message
			if ((ret = _stream.sendMsg(msg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS) {
				return ret;
			}

			// close watchlist request
			_watchlist.closeWlRequest(wlRequest);

			// close stream
			_stream.close();
			_stream = null;
			break;
		case MsgClasses.POST:
			if (_loginRefresh.features().checkHasSupportPost())
			{
				if (_stream.state().streamState() == StreamStates.OPEN)
				{
	                boolean resetServiceId = false;
	                
					// validate post submit
					if ((ret = _stream.validatePostSubmit((PostMsg) msg, errorInfo)) != ReactorReturnCodes.SUCCESS)
					{
						return ret;
					}

					// replace service id if message submitted with service name
					if (submitOptions.serviceName() != null)
					{
					    if (!((PostMsg) msg).checkHasMsgKey())
					    {
                            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                          ReactorReturnCodes.INVALID_USAGE,
                                                                          "WlLoginHandler.submitMsg",
                                                                          "Post message submitted with service name but no message key.");
					        
					    }
					    
					    if ((ret = _watchlist.changeServiceNameToID(((PostMsg) msg).msgKey(), submitOptions.serviceName(), errorInfo)) < ReactorReturnCodes.SUCCESS)
					    {
					        return ret;
					    }
	                    
	                    // set resetServiceId flag
	                    resetServiceId = true;
					}

					// send message
	                ret = _stream.sendMsg(msg, submitOptions, errorInfo);
	                
	                // reset service id if necessary
	                if (resetServiceId)
	                {
	                    ((PostMsg) msg).msgKey().flags(((PostMsg) msg).msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
	                    ((PostMsg) msg).msgKey().serviceId(0);
	                }              
	                
	                // return if send message not successful
	                if (ret < ReactorReturnCodes.SUCCESS)
	                {
	                    return ret;
	                }
				} 
				else
				{
					// cannot submit post when stream is not open
					return _watchlist
							.reactor()
							.populateErrorInfo(errorInfo,
									ReactorReturnCodes.INVALID_USAGE,
									"WlLoginHandler.submitMsg",
									"Cannot submit PostMsg when stream not in open state.");
				}
			} 
			else
			{
				return _watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.INVALID_USAGE,
						"WlLoginHandler.submitMsg",
						"Posting not supported by provider");
			}
			break;
		case MsgClasses.GENERIC:
			if (_stream.state().streamState() == StreamStates.OPEN)
			{
			    boolean resetServiceId = false;
			    
				// replace service id if message submitted with service name
				if (submitOptions.serviceName() != null)
				{
                    if (!((GenericMsg) msg).checkHasMsgKey())
                    {
                        return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                      ReactorReturnCodes.INVALID_USAGE,
                                                                      "WlLoginHandler.submitMsg",
                                                                      "Generic message submitted with service name but no message key.");
                        
                    }
                    
                    if ((ret = _watchlist.changeServiceNameToID(((GenericMsg) msg).msgKey(), submitOptions.serviceName(), errorInfo)) < ReactorReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                    
                    // set resetServiceId flag
                    resetServiceId = true;
				}
				
				// send message
				ret = _stream.sendMsg(msg, submitOptions, errorInfo);
				
                // reset service id if necessary
                if (resetServiceId)
                {
                    ((GenericMsg) msg).msgKey().flags(((GenericMsg) msg).msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
                    ((GenericMsg) msg).msgKey().serviceId(0);
                }              
                
                // return if send message not successful
				if (ret < ReactorReturnCodes.SUCCESS)
				{
					return ret;
				}
			}
			else
			{
				// cannot submit generic message when stream is not open
				return _watchlist
						.reactor()
						.populateErrorInfo(errorInfo,
								ReactorReturnCodes.INVALID_USAGE,
								"WlLoginHandler.submitMsg",
								"Cannot submit GenericMsg when stream not in open state.");
			}
			break;
		default:
			return _watchlist.reactor().populateErrorInfo(
					errorInfo,
					ReactorReturnCodes.FAILURE,
					"WlLoginHandler.submitMsg",
					"Invalid message class (" + msg.msgClass()
							+ ") submitted to Watchlist login handler");
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/* Handles a login request reissue. */
	int handleReissue(LoginRequest loginRequest, ReactorErrorInfo errorInfo) 
	{
		int ret;
		// validate and handle login credentials update
		
		if ((ret = validateReissue(loginRequest, errorInfo)) != ReactorReturnCodes.SUCCESS) 
		{
			return ret;
		}
		// handle pause
		if (loginRequest.checkPause()) 
		{
			if (_loginRefresh.checkHasFeatures() && _loginRefresh.features().checkHasSupportOptimizedPauseResume()
					&& _loginRefresh.features().supportOptimizedPauseResume() == 1)
			{
				_awaitingResumeAll = true;	
		
				// notify item handler to pause all
				ret = _watchlist.itemHandler().pauseAll();
				if (ret < ReactorReturnCodes.SUCCESS) 
					return ret;
							
				loginRequest.flags(loginRequest.flags() | LoginRequestFlags.PAUSE_ALL);
			}
		} 
		else // not pause
		{
			// handle resume
			if (_awaitingResumeAll && !isTokenChange(loginRequest))
			{
				_awaitingResumeAll = false;

				// notify item handler to resume all
				ret = _watchlist.itemHandler().resumeAll();
				if (ret < ReactorReturnCodes.SUCCESS) return ret;				
			}
		}
		return ReactorReturnCodes.SUCCESS;
	}

	/* Validates a login request reissue. */
	int validateReissue(LoginRequest loginRequest, ReactorErrorInfo errorInfo)
	{
		if (loginRequest.checkHasRole() && loginRequest.role() != Login.RoleTypes.CONS) 
		{
			return _watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.INVALID_USAGE,
					"WlLoginHandler.validateReissue",
					"Login role is not consumer");
		}

		if (_loginRequest.checkHasUserNameType() == loginRequest.checkHasUserNameType())
		{
			if (_loginRequest.checkHasUserNameType()
					&& (loginRequest.userNameType() != _loginRequest.userNameType() 
					|| (_loginRequest.userNameType() != Login.UserIdTypes.TOKEN  
					&& _loginRequest.userNameType() != Login.UserIdTypes.AUTHN_TOKEN
					    && !_loginRequest.userName().equals(loginRequest.userName()))))
			{
				return _watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.INVALID_USAGE,
						"WlLoginHandler.validateReissue",
						"Login userNameType does not match existing request");
			}
		} 
		else 
		{
			return _watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
					"WlLoginHandler.validateReissue", "Login userNameType does not match existing request");
		}

		if (_loginRequest.checkHasDownloadConnectionConfig() == loginRequest.checkHasDownloadConnectionConfig())
		{
			if (_loginRequest.checkHasDownloadConnectionConfig()
					&& _loginRequest.downloadConnectionConfig() != loginRequest.downloadConnectionConfig())
			{
				return _watchlist.reactor().populateErrorInfo(errorInfo,
								ReactorReturnCodes.INVALID_USAGE, "WlLoginHandler.validateReissue",
								"Login downloadConnectionConfig does not match existing request");
			}
		}
		else
		{
			return _watchlist
					.reactor()
					.populateErrorInfo(errorInfo,
							ReactorReturnCodes.INVALID_USAGE,
							"WlLoginHandler.validateReissue",
							"Login downloadConnectionConfig does not match existing request");
		}

		if (_loginRequest.checkHasInstanceId() == loginRequest.checkHasInstanceId())
		{
			if (_loginRequest.checkHasInstanceId()
					&& !_loginRequest.instanceId().equals(loginRequest.instanceId()))
			{
				return _watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.INVALID_USAGE,
						"WlLoginHandler.validateReissue",
						"Login instanceId does not match existing request");
			}
		}
		else
		{
			return _watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.INVALID_USAGE,
					"WlLoginHandler.validateReissue",
					"Login instanceId does not match existing request");
		}

		if (_loginRequest.checkHasPassword() == loginRequest.checkHasPassword())
		{
			if (_loginRequest.checkHasPassword()
			        && !_loginRequest.password().equals(loginRequest.password()))
			{
				return _watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.INVALID_USAGE,
						"WlLoginHandler.validateReissue",
						"Login password does not match existing request");
			}
		}
		else
		{
			return _watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.INVALID_USAGE,
					"WlLoginHandler.validateReissue",
					"Login password does not match existing request");
		}

		if (_loginRequest.checkHasAttrib() == loginRequest.checkHasAttrib()) 
		{
			if (_loginRequest.checkHasAttrib()) {
				if (_loginRequest.attrib().checkHasApplicationId() == loginRequest
						.attrib().checkHasApplicationId()) {
					if (_loginRequest.attrib().checkHasApplicationId()
							&& !_loginRequest
									.attrib()
									.applicationId()
									.equals(loginRequest.attrib()
											.applicationId())) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login applicationId does not match existing request");
					}
				} else {
					return _watchlist
							.reactor()
							.populateErrorInfo(errorInfo,
									ReactorReturnCodes.INVALID_USAGE,
									"WlLoginHandler.validateReissue",
									"Login applicationId does not match existing request");
				}

				if (_loginRequest.attrib().checkHasApplicationName() == loginRequest
						.attrib().checkHasApplicationName()) {
					if (_loginRequest.attrib().checkHasApplicationName()
							&& !_loginRequest
									.attrib()
									.applicationName()
									.equals(loginRequest.attrib()
											.applicationName())) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login applicationName does not match existing request");
					}
				} else {
					return _watchlist
							.reactor()
							.populateErrorInfo(errorInfo,
									ReactorReturnCodes.INVALID_USAGE,
									"WlLoginHandler.validateReissue",
									"Login applicationName does not match existing request");
				}

				if (_loginRequest.attrib().checkHasPosition() == loginRequest
						.attrib().checkHasPosition()) {
					if (_loginRequest.attrib().checkHasPosition()
							&& !_loginRequest.attrib().position()
									.equals(loginRequest.attrib().position())) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login position does not match existing request");
					}
				} else {
					return _watchlist.reactor().populateErrorInfo(errorInfo,
							ReactorReturnCodes.INVALID_USAGE,
							"WlLoginHandler.validateReissue",
							"Login position does not match existing request");
				}

				if (_loginRequest.attrib().checkHasSingleOpen() == loginRequest
						.attrib().checkHasSingleOpen()) {
					if (_loginRequest.attrib().checkHasSingleOpen()
							&& _loginRequest.attrib().singleOpen() != loginRequest
									.attrib().singleOpen()) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login singleOpen does not match existing request");
					}
				} else {
					return _watchlist.reactor().populateErrorInfo(errorInfo,
							ReactorReturnCodes.INVALID_USAGE,
							"WlLoginHandler.validateReissue",
							"Login singleOpen does not match existing request");
				}

				if (_loginRequest.attrib().checkHasAllowSuspectData() == loginRequest
						.attrib().checkHasAllowSuspectData()) {
					if (_loginRequest.attrib().checkHasAllowSuspectData()
							&& _loginRequest.attrib().allowSuspectData() != loginRequest
									.attrib().allowSuspectData()) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login allowSuspectData does not match existing request");
					}
				} else {
					return _watchlist
							.reactor()
							.populateErrorInfo(errorInfo,
									ReactorReturnCodes.INVALID_USAGE,
									"WlLoginHandler.validateReissue",
									"Login allowSuspectData does not match existing request");
				}

				if (_loginRequest.attrib()
						.checkHasProvidePermissionExpressions() == loginRequest
						.attrib().checkHasProvidePermissionExpressions()) {
					if (_loginRequest.attrib()
							.checkHasProvidePermissionExpressions()
							&& _loginRequest.attrib()
									.providePermissionExpressions() != loginRequest
									.attrib().providePermissionExpressions()) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login providePermissionExpressions does not match existing request");
					}
				} else {
					return _watchlist
							.reactor()
							.populateErrorInfo(errorInfo,
									ReactorReturnCodes.INVALID_USAGE,
									"WlLoginHandler.validateReissue",
									"Login providePermissionExpressions does not match existing request");
				}

				if (_loginRequest.attrib().checkHasProvidePermissionProfile() == loginRequest
						.attrib().checkHasProvidePermissionProfile()) {
					if (_loginRequest.attrib()
							.checkHasProvidePermissionProfile()
							&& _loginRequest.attrib()
									.providePermissionProfile() != loginRequest
									.attrib().providePermissionProfile()) {
						return _watchlist
								.reactor()
								.populateErrorInfo(errorInfo,
										ReactorReturnCodes.INVALID_USAGE,
										"WlLoginHandler.validateReissue",
										"Login providePermissionProfile does not match existing request");
					}
				} else {
					return _watchlist
							.reactor()
							.populateErrorInfo(errorInfo,
									ReactorReturnCodes.INVALID_USAGE,
									"WlLoginHandler.validateReissue",
									"Login providePermissionProfile does not match existing request");
				}
			}
		} else {
			return _watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.INVALID_USAGE,
					"WlLoginHandler.validateReissue",
					"Login attrib does not match existing request");
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/* Determines if there is a token change. */
    boolean isTokenChange(LoginRequest loginRequest)
    {
        return (loginRequest.userNameType() == Login.UserIdTypes.TOKEN || loginRequest.userNameType() == Login.UserIdTypes.AUTHN_TOKEN) &&
                (!loginRequest.userName().equals(_loginRequest.userName()) || !loginRequest.authenticationExtended().equals(_loginRequest.authenticationExtended()));
    }

    @Override
	public int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
	{
		assert (_stream == wlStream);
		assert (msg.streamId() == _loginRequest.streamId());

		int ret = ReactorReturnCodes.SUCCESS;
		int ret1, ret2;

		switch (msg.msgClass()) 
		{
		case MsgClasses.REFRESH:
			ret = readRefreshMsg(wlStream, dIter, msg, errorInfo);
			break;
		case MsgClasses.STATUS:
			ret = readStatusMsg(wlStream, dIter, msg, errorInfo);
			break;
		case MsgClasses.GENERIC:
			ret = readGenericMsg(wlStream, dIter, msg, errorInfo);
			break;
		case MsgClasses.ACK:
			ret = readAckMsg(wlStream, dIter, msg, errorInfo);
			break;
		default:
			ret = _watchlist.reactor().populateErrorInfo(
					errorInfo,
					ReactorReturnCodes.FAILURE,
					"WlLoginHandler.readMsg",
					"Invalid message class (" + msg.msgClass()
							+ ") received by Watchlist login handler");
			break;
		}

        /* If application closed the login stream while in callback, do not process further. */
        if (_stream == null)
            return ret;

		// handle any state transition
		if (ret == ReactorReturnCodes.SUCCESS) 
		{
			switch (wlStream.state().streamState()) 
			{
			case StreamStates.CLOSED:
			case StreamStates.CLOSED_RECOVER:
			case StreamStates.REDIRECTED:
				// notify other handlers that login stream is closed
				ret1 = _watchlist.directoryHandler().loginStreamClosed();
				ret2 = _watchlist.itemHandler().loginStreamClosed(msg);

				if (ret1 < ReactorReturnCodes.SUCCESS) 
				{
					return ret1;
				} else if (ret2 < ReactorReturnCodes.SUCCESS) 
				{
					return ret2;
				}

				if (wlStream.state().streamState() == StreamStates.CLOSED
						|| wlStream.state().streamState() == StreamStates.REDIRECTED) 
				{
					// close watchlist request
					_tempWlInteger.value(msg.streamId());
					WlRequest wlRequest = _watchlist.streamIdtoWlRequestTable()
							.get(_tempWlInteger);
					_watchlist.closeWlRequest(wlRequest);

					// close stream if state is closed
					_stream.close();
					_stream = null;
				}
				break;
			case StreamStates.OPEN:
				if (wlStream.state().dataState() == DataStates.OK) 
				{
                    // Connection is established. Reset reconnect timeout.
                    _watchlist.reactorChannel().resetReconnectTimers();

					// notify other handlers that login stream is open
					ret1 = _watchlist.directoryHandler().loginStreamOpen(
							errorInfo);
					ret2 = _watchlist.itemHandler().loginStreamOpen(errorInfo);

					if (ret1 < ReactorReturnCodes.SUCCESS) 
					{
						return ret1;
					} 
					else if (ret2 < ReactorReturnCodes.SUCCESS) 
					{
						return ret2;
					}
				}
				break;
			default:
				break;
			}
		}

		return ret;
	}

	/* Reads a refresh message. */
	int readRefreshMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
	{
		int ret;
		
		// make sure refresh complete flag is set
		// login handler doesn't handle multi-part login refreshes
		if (!((RefreshMsg) msg).checkRefreshComplete()) 
		{
			return _watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE,
					"WlLoginHandler.readRefreshMsg",
					"Watchlist doesn't handle multi-part login refresh.");
		}

		// notify stream that response received if solicited
		if (((RefreshMsg) msg).checkSolicited()) {
			wlStream.responseReceived();
		}

		// convert to rdm login refresh and save
		_loginRefresh.decode(dIter, msg);

		// alter login refresh for user
		// SingleOpen and AllowSuspectData must match the user's request,
		// regardless of provider support.
		// SupportOptimizedPauseResume must be passed on from the provider, but
		// must NOT pass on SupportPauseResume.
		// SupportBatchRequests is supported regardless of provider support.
		// Enhanced symbol list data streams are always supported.
		_loginRefresh.attrib().applyHasSingleOpen();
		_loginRefresh.attrib().singleOpen(supportSingleOpen() ? 1 : 0);
		_loginRefresh.attrib().applyHasAllowSuspectData();
		_loginRefresh.attrib().allowSuspectData(
				supportAllowSuspectData() ? 1 : 0);
		_loginRefresh.features().applyHasSupportBatchRequests();
		_loginRefresh.features().supportBatchRequests(
				Login.BatchSupportFlags.SUPPORT_REQUESTS);
		_loginRefresh.features().applyHasSupportEnhancedSymbolList();
		_loginRefresh.features().supportEnhancedSymbolList(
				Login.EnhancedSymbolListSupportFlags.DATA_STREAMS);

		// set state from login refresh
		_loginRefresh.state().copy(wlStream.state());

		if (_loginRefresh.state().streamState() == StreamStates.CLOSED_RECOVER) {
			_loginRefresh.state().streamState(StreamStates.OPEN);
			_loginRefresh.state().dataState(DataStates.SUSPECT);
            return callbackUserAndDisconnect("WlLoginHandler.readRefreshMsg", msg, _loginRefresh, errorInfo);
		}

		// call back user
        _tempWlInteger.value(msg.streamId());
		ret = callbackUser("WlLoginHandler.readRefreshMsg", msg,
				_loginRefresh, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo);
		
		// send pending request if necessary
		if (_requestCount > 0)
		{
			_requestCount--;
		}
		if (_requestCount > 0 && ret != ReactorCallbackReturnCodes.FAILURE)
		{
			_tempMsg.clear();
			_watchlist.convertRDMToCodecMsg(_loginRequest, _tempMsg);
			if ((ret = wlStream.sendMsg(_tempMsg, _submitOptions, errorInfo)) == ReactorReturnCodes.SUCCESS)
			{
				if (!_loginRequest.checkNoRefresh())
				{
					_requestCount = 1;
				}
				else
				{
					_requestCount = 0;
				}
			}
		}
		
		return ret;
	}

	/* Reads a status message. */
	int readStatusMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
	{
		// convert to rdm login status and save
		_loginStatus.decode(dIter, msg);

		// notify stream that response received
		wlStream.responseReceived();

		if (_loginStatus.checkHasState()) 
		{
			_loginStatus.state().copy(wlStream.state());

			// if CLOSED_RECOVER, change state to OPEN/SUSPECT for call back
			if (_loginStatus.state().streamState() == StreamStates.CLOSED_RECOVER) 
			{
				_loginStatus.state().streamState(StreamStates.OPEN);
				_loginStatus.state().dataState(DataStates.SUSPECT);
                return callbackUserAndDisconnect("WlLoginHandler.readStatusMsg", msg, _loginStatus, errorInfo);
			}
		}

		// call back user
        _tempWlInteger.value(msg.streamId());
		return callbackUser("WlLoginHandler.readStatusMsg", msg, _loginStatus, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo);
	}
    
    /* Used when login Closed/Recoverable state is received.
     * Notifies application, then disconnects channel. */
    int callbackUserAndDisconnect(String location, Msg msg, LoginMsg loginMsg, ReactorErrorInfo errorInfo)
    {
        int ret;
        
        _tempWlInteger.value(msg.streamId());
        if ((ret = callbackUser(location, msg, loginMsg, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) != ReactorReturnCodes.SUCCESS)
            return ret;
        
        _watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, 
                "Received login response with Closed/Recover stream state. Disconnecting.");
        return _watchlist._reactor.disconnect( _watchlist.reactorChannel(), location, errorInfo);
    }

	/* Reads a generic message. */
	int readGenericMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo) 
	{
		WlRequest wlRequest = _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger);

		// call back user
		return _watchlist.reactor().sendAndHandleDefaultMsgCallback(
				"WlLoginHandler.readGenericMsg", _watchlist.reactorChannel(),
				null, msg, (wlRequest != null ? wlRequest.streamInfo() : null),
				errorInfo);
	}

	/* Reads an Ack message. */
	int readAckMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo) 
	{
		int ret = ReactorCallbackReturnCodes.SUCCESS;

		// handle the post Ack
		if (wlStream.handlePostAck(msg)) {
			// call back user if ACK was processed
	        _tempWlInteger.value(msg.streamId());
			if (msg.domainType() != DomainTypes.LOGIN) {
				ret = _watchlist.itemHandler().callbackUser(
						"WlLoginHandler.readAckMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo);
			} else {
				ret = callbackUser("WlLoginHandler.readAckMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger),
						errorInfo);
			}
		}

		return ret;
	}

	@Override
	public int callbackUser(String location, Msg msg, MsgBase rdmMsg, WlRequest wlRequest, ReactorErrorInfo errorInfo) 
	{
		int ret = ReactorReturnCodes.SUCCESS;

		ret = _watchlist.reactor().sendAndHandleLoginMsgCallback(location,
				_watchlist.reactorChannel(), null, msg, (LoginMsg) rdmMsg,
				(wlRequest != null ? wlRequest.streamInfo() : null), errorInfo);

		if (ret == ReactorCallbackReturnCodes.RAISE) {
			ret = _watchlist.reactor().sendAndHandleDefaultMsgCallback(
					location, _watchlist.reactorChannel(), null, msg,
					(wlRequest != null ? wlRequest.streamInfo() : null),
					errorInfo);
		}

		return ret;
	}

	/* Dispatch all streams for the handler. */
	int dispatch(ReactorErrorInfo errorInfo) 
	{
		if (_stream != null) 
		{
			return _stream.dispatch(errorInfo);
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/* Returns whether or not watchlist login stream supports single open. */
	boolean supportSingleOpen()
	{
		if (_loginRequest != null && _loginRequest.checkHasAttrib()
				&& _loginRequest.attrib().checkHasSingleOpen()) 
		{
			return (_loginRequest.attrib().singleOpen() > 0 ? true : false);
		} 
		else
		{
			if (_watchlist._role != null
					&& _watchlist._role._loginRequest != null
					&& _watchlist._role._loginRequest.attrib() != null)
				return (_watchlist._role._loginRequest.attrib().singleOpen() > 0 ? true
						: false);
			else
				return false;
		}
	}

	/*
	 * Returns whether or not watchlist login stream supports allow suspect
	 * data.
	 */
	boolean supportAllowSuspectData()
	{
		if (_loginRequest != null && _loginRequest.checkHasAttrib()
				&& _loginRequest.attrib().checkHasAllowSuspectData()) {
			return (_loginRequest.attrib().allowSuspectData() > 0 ? true
					: false);
		} else {
			if (_watchlist._role != null
					&& _watchlist._role._loginRequest != null
					&& _watchlist._role._loginRequest.attrib() != null)
				return (_watchlist._role._loginRequest.attrib()
						.allowSuspectData() > 0 ? true : false);
			else
				return false;
		}
	}

	/* Returns whether or not login info is provided by the application. */
	boolean userLoginStreamProvided() 
	{
		if (_userloginStreamOpen == false)
			return false;

		if ((_loginRequest != null || _watchlist._role._loginRequest != null))
			return true;
		else
			return false;
	}

	/* Returns whether or not watchlist login stream supports batch request. */
	boolean supportBatchRequests() 
	{
		// always support batch requests
		return true;
	}

	/*
	 * Returns whether or not watchlist login stream supports enhanced symbol
	 * list.
	 */
	boolean supportEnhancedSymbolList() 
	{
		// always support enhanced symbol list
		return true;
	}

	/*
	 * Returns whether or not watchlist login stream supports optimized pause
	 * and resume.
	 */
	boolean supportOptimizedPauseResume() 
	{
		if (_loginRefresh != null
				&& _loginRefresh.features()
						.checkHasSupportOptimizedPauseResume()) {
			return (_loginRefresh.features().supportOptimizedPauseResume() > 0 ? true
					: false);
		} else {
			return false;
		}
	}

	/* Returns whether or not watchlist login stream supports view requests. */
	boolean supportViewRequests() 
	{
		if (_loginRefresh != null
				&& _loginRefresh.features().checkHasSupportViewRequests()) {
			return (_loginRefresh.features().supportViewRequests() > 0 ? true
					: false);
		} else {
			return false;
		}
	}

	/* Returns whether or not provider supports posting. */
	boolean supportPost() 
	{
		if (_loginRefresh != null
				&& _loginRefresh.features().checkHasSupportPost()) {
			return (_loginRefresh.features().supportOMMPost() > 0 ? true
					: false);
		} else {
			return false;
		}
	}

	/* Returns the login stream. */
	WlStream wlStream() 
	{
		return _stream;
	}

	/* Handles channel down event. */
	void channelDown() 
	{
		int streamId = (_loginRequest != null ? _loginRequest.streamId() : 0);

		if (_stream != null) {
			_stream.channelDown();

			// set state to closed recover if current state isn't closed
			if (_stream.state().streamState() == StreamStates.OPEN) {
				_stream.state().clear();
				_stream.state().streamState(StreamStates.CLOSED_RECOVER);
				_stream.state().dataState(DataStates.SUSPECT);

				// call back user with login status of OPEN/SUSPECT
				_statusMsg.streamId(streamId);
				_statusMsg.applyHasState();
				_statusMsg.state().streamState(StreamStates.OPEN);
				_statusMsg.state().dataState(DataStates.SUSPECT);
				_statusMsg.state().code(StateCodes.NONE);
				_statusMsg.state().text(_tempBuffer);

				_loginStatus.streamId(streamId);
				_loginStatus.applyHasState();
				_loginStatus.state().streamState(StreamStates.OPEN);
				_loginStatus.state().dataState(DataStates.SUSPECT);
				_loginStatus.state().code(StateCodes.NONE);
				_loginStatus.state().text(_tempBuffer);

                _userloginStreamOpen = false;
                _tempWlInteger.value(_statusMsg.streamId());
                callbackUser("WlLoginHandler.channelDown", _statusMsg,
                        _loginStatus, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo);
			}
		}
	}

	/* Handles channel up event. */
	void channelUp(ReactorErrorInfo errorInfo) 
	{
		boolean newStream = false;
		boolean newRequest = false;

		_userloginStreamOpen = true;

		// notify login stream that channel is up
		if (_stream != null) 
		{
			_stream.channelUp();
		}

		// create login request if not created yet and role has one
		if (_loginRequest == null
				&& _watchlist.role().rdmLoginRequest() != null) 
		{
			newRequest = true;

			_loginRequest = (LoginRequest) LoginMsgFactory.createMsg();
			_loginRequest.rdmMsgType(LoginMsgType.REQUEST);
			_watchlist.role().rdmLoginRequest().copy(_loginRequest);

			// create login stream if not created yet
			if (_stream == null) 
			{
				newStream = true;

				_loginRequest.streamId(_loginStreamId);

				// create stream
				_stream = ReactorFactory.createWlStream();
				_stream.handler(this);
				_stream.watchlist(_watchlist);
				_stream.streamId(_loginRequest.streamId());
				_stream.domainType(_loginRequest.domainType());
			}

		}
		// send login request via stream
		if (_loginRequest != null && _stream != null) 
		{
			if (_loginRequest.checkPause())
			{
				_loginRequest.flags(_loginRequest.flags() & ~LoginRequestFlags.PAUSE_ALL);
			}
			if (_loginRequest.checkNoRefresh())
			{
				_loginRequest.flags(_loginRequest.flags() & ~LoginRequestFlags.NO_REFRESH);
			}
			_tempMsg.clear();
			_watchlist.convertRDMToCodecMsg(_loginRequest, _tempMsg);

			if (_stream.sendMsg(_tempMsg, _submitOptions, errorInfo) >= ReactorReturnCodes.SUCCESS)
			{
				// if successful update tables
				if (newRequest) {
					// add to watchlist request table
					WlRequest wlRequest = ReactorFactory.createWlRequest();
					_tempMsg.clear();
					_watchlist.convertRDMToCodecMsg(_loginRequest, _tempMsg);
					wlRequest.requestMsg().clear();
					_tempMsg.copy(wlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS);
					wlRequest.handler(this);
					WlInteger wlInteger = ReactorFactory.createWlInteger();
					wlInteger.value(_loginRequest.streamId());
					wlRequest.tableKey(wlInteger);
					_watchlist.streamIdtoWlRequestTable().put(wlInteger, wlRequest);
				}

				if (newStream) 
				{
					// add stream to watchlist table
					WlInteger wlInteger = ReactorFactory.createWlInteger();
					wlInteger.value(_loginRequest.streamId());
					_stream.tableKey(wlInteger);
					_watchlist.streamIdtoWlStreamTable()
							.put(wlInteger, _stream);
				}

				_requestCount = 1;
			} 
			else // sendMsg failed
			{
				// if new request, set loginRequest to null
				if (newRequest) 
				{
					_loginRequest = null;
				}

				// if new stream, return stream to pool
				if (newStream) {
					_stream.returnToPool();
					_stream = null;
				}
			}
		}
	}

	@Override
	public int requestTimeout(WlStream wlStream, ReactorErrorInfo errorInfo) 
	{
		int streamId = (_loginRequest != null ? _loginRequest.streamId() : 0);

		// call back user with login status of OPEN/SUSPECT
		_statusMsg.streamId(streamId);
		_statusMsg.applyHasState();
		_statusMsg.state().streamState(StreamStates.OPEN);
		_statusMsg.state().dataState(DataStates.SUSPECT);
		_statusMsg.state().code(StateCodes.NONE);
		_statusMsg.state().text(_tempBuffer);

		_loginStatus.streamId(streamId);
		_loginStatus.applyHasState();
		_loginStatus.state().streamState(StreamStates.OPEN);
		_loginStatus.state().dataState(DataStates.SUSPECT);
		_loginStatus.state().code(StateCodes.NONE);
		_loginStatus.state().text(_tempBuffer);

        _tempWlInteger.value(_statusMsg.streamId());
		callbackUser("WlLoginHandler.channelDown", _statusMsg, _loginStatus, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), 
				_errorInfo);

		// re-send login request
		_tempMsg.clear();
		_watchlist.convertRDMToCodecMsg(_loginRequest, _tempMsg);
		return wlStream.sendMsg(_tempMsg, _submitOptions, errorInfo);
	}

	/* Clear state of watchlist login handler for re-use. */
	void clear() 
	{
		// this handler is still associated with same watchlist so don't set
		// watchlist to null
		_stream = null;
		_loginRequest = null;
		_tempLoginRequest.clear();
		_tempLoginRequest.rdmMsgType(LoginMsgType.REQUEST);
		_errorInfo.clear();
		_eIter.clear();
		_dIter.clear();
		_loginRefresh.clear();
		_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		_loginStatus.clear();
		_loginStatus.rdmMsgType(LoginMsgType.STATUS);
		_statusMsg.clear();
		_statusMsg.msgClass(MsgClasses.STATUS);
		_statusMsg.domainType(DomainTypes.LOGIN);
		_tempBuffer.clear();
		_tempBuffer.data("");
		_tempMsg.clear();
		_errorInfo.clear();
		_awaitingResumeAll = false;
		_requestCount = 0;
	}
}
