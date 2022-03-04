/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;



import java.nio.ByteBuffer;



import com.refinitiv.eta.codec.Buffer;

import com.refinitiv.eta.codec.CodecFactory;

import com.refinitiv.eta.codec.DataStates;

import com.refinitiv.eta.codec.Msg;

import com.refinitiv.eta.codec.State;

import com.refinitiv.eta.codec.StreamStates;

import com.refinitiv.eta.transport.Error;	

import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;



/*

 * The TunnelStreamManager represents one or more TunnelStream structures associated

 * with a particular connection. The TunnelManager provides a single

 * point for driving events associated with the associated streams.

 */

class TunnelStreamManager 

{

	long _nextDispatchTime;



	VaDoubleLinkList<TunnelStream> _tunnelStreamList;
	VaDoubleLinkList<TunnelStream> _tunnelStreamDispatchList;
	VaDoubleLinkList<TunnelStream> _tunnelStreamTimeoutList;
    Buffer _tunnelStreamTempBuffer = CodecFactory.createBuffer();

    ByteBuffer _tunnelStreamTempByteBuffer = ByteBuffer.allocateDirect(8192);

	

	ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();

    private com.refinitiv.eta.codec.State _tmpState = CodecFactory.createState();



	TunnelStreamManager()

	{

		_tunnelStreamList = new VaDoubleLinkList<TunnelStream>();
		_tunnelStreamDispatchList = new VaDoubleLinkList<TunnelStream>();
		_tunnelStreamTimeoutList = new VaDoubleLinkList<TunnelStream>();
	}



    void clear()

    {

        _nextDispatchTime = 0;

        _tunnelStreamList.clear();

        _tunnelStreamDispatchList.clear();

        _tunnelStreamTimeoutList.clear();

    }

	

	int dispatch(Error error)

	{

		int ret = ReactorReturnCodes.SUCCESS;

		long nextMsgTimeoutNsec = 0;



		TunnelStream tunnelStream;



		for(tunnelStream = _tunnelStreamDispatchList.start(TunnelStream.DISPATCH_LINK); 

				tunnelStream != null;

				tunnelStream = _tunnelStreamDispatchList.forth(TunnelStream.DISPATCH_LINK))

		{

			ret = tunnelStream.dispatch(error);

			if (ret != ReactorReturnCodes.SUCCESS)

			{

                if (ret == ReactorReturnCodes.FAILURE)

                {

                    // send close message for TunnelStream

                    tunnelStream.sendCloseMsg(error);

                    

                    // return SUCCESS here, application can reopen closed TunnelStream

                    ret = ReactorReturnCodes.SUCCESS;

                }


                // Send TunnelStream status close/recover event via Reactor
                sendTunnelStreamStatusCloseRecover(tunnelStream, error);


			    break;

			}

		}



		switch(ret)

		{

			case ReactorReturnCodes.SUCCESS:

				break;



            case ReactorReturnCodes.NO_BUFFERS:

                // return SUCCESS for NO_BUFFERS

                // user will call again and eventually get a buffer

                ret = ReactorReturnCodes.SUCCESS;

                break;

                

            default:

				return ret;

		}



		if ((tunnelStream = _tunnelStreamTimeoutList.start(TunnelStream.TIMEOUT_LINK)) != null)

		{

			long currentTimeNsec = System.nanoTime();



			for (; tunnelStream != null; 

					tunnelStream = _tunnelStreamTimeoutList.forth(TunnelStream.TIMEOUT_LINK))

			{

				assert(tunnelStream.hasNextTimeout());



				ret = tunnelStream.handleTimer(currentTimeNsec, error);

				if (ret == ReactorReturnCodes.SUCCESS &&

				    tunnelStream.hasNextTimeout() &&

				    tunnelStream.nextTimeoutNsec() - nextMsgTimeoutNsec < 0)

				{

					_nextDispatchTime = tunnelStream.nextTimeoutNsec();

				}

			}

		}



		return (ret == ReactorReturnCodes.SUCCESS) ?  _tunnelStreamDispatchList.count() : ret;

	}



    

    int readMsg(TunnelStream tunnelStream, Msg deliveredMsg, Error error)

    {

        int ret;

        

        if ((ret = tunnelStream.readMsg(deliveredMsg, error)) == ReactorReturnCodes.FAILURE)

        {

            // send close message for TunnelStream

            tunnelStream.sendCloseMsg(error);

            

            // Send TunnelStream status close/recover event via Reactor

            sendTunnelStreamStatusCloseRecover(tunnelStream, error);



            // return SUCCESS here, application can reopen closed TunnelStream

            ret = ReactorReturnCodes.SUCCESS;

        }



        return ret;

    }   

	

	boolean hasNextDispatchTime()

	{

		return _tunnelStreamTimeoutList.count() > 0;

	}



	

	long nextDispatchTime()

	{

		return _nextDispatchTime;

	}



	void addTunnelStreamToDispatchList(TunnelStream tunnelStream)

	{

		if (tunnelStream.notifying() == false)

		{		

			_tunnelStreamDispatchList.push(tunnelStream, TunnelStream.DISPATCH_LINK);

			tunnelStream.notifying(true);

		}

	}

    

	void removeTunnelStreamFromDispatchList(TunnelStream tunnelStream)

	{

		if (tunnelStream.notifying())			

		{

			_tunnelStreamDispatchList.remove(tunnelStream, TunnelStream.DISPATCH_LINK);

			tunnelStream.notifying(false);

		}

	}

	

	void addTunnelStreamToTimeoutList(TunnelStream tunnelStream, long nextDispatchTime)

	{

		boolean hasNextDispatchTime = _tunnelStreamTimeoutList.count() > 0;



		if (!tunnelStream.hasNextTimeout())

		{

			tunnelStream.hasNextTimeout(true);

			_tunnelStreamTimeoutList.push(tunnelStream, TunnelStream.TIMEOUT_LINK);

		}



		/* Set next dispatch time, if we don't already have one or

		 * this one is sooner. */

		if (!hasNextDispatchTime || tunnelStream.nextTimeoutNsec() < _nextDispatchTime)

			_nextDispatchTime = tunnelStream.nextTimeoutNsec();



	}



    

	void removeTunnelStreamFromTimeoutList(TunnelStream tunnelStream)

	{

		if (tunnelStream.hasNextTimeout())

		{

			tunnelStream.hasNextTimeout(false);

			_tunnelStreamTimeoutList.remove(tunnelStream, TunnelStream.TIMEOUT_LINK);

		}

	}



	/* TunnelStream attributes. */

	ReactorChannel _reactorChannel;

	boolean _needsFlush;



    

    int setChannel(ReactorChannel reactorChannel, Error error)

    {

        _reactorChannel = reactorChannel;



        for(TunnelStream tunnelStream = _tunnelStreamList.start(TunnelStream.MANAGER_LINK); tunnelStream != null; 

                tunnelStream = _tunnelStreamList.forth(TunnelStream.MANAGER_LINK))

            addTunnelStreamToDispatchList(tunnelStream);

        

        return ReactorReturnCodes.SUCCESS;

    }



	/* Opens a consumer-side TunnelStream and associates it with this TunnelManager. */

	TunnelStream createTunnelStream(TunnelStreamOpenOptions options)

	{

		TunnelStream tunnelStream = new TunnelStream(_reactorChannel, options);

		_tunnelStreamList.push(tunnelStream, TunnelStream.MANAGER_LINK);

		return tunnelStream;

	}



	

	/* Opens a provider-side TunnelStream and associates it with this TunnelManager. */

	TunnelStream createTunnelStream(TunnelStreamRequestEvent event, TunnelStreamAcceptOptions options)

	{

		TunnelStream tunnelStream = new TunnelStream(_reactorChannel, event, options);

		_tunnelStreamList.push(tunnelStream, TunnelStream.MANAGER_LINK);

		return tunnelStream;

	}



    /* Removes a tunnel stream from all lists (overall, dispatch needed, timeout). */

	void removeTunnelStream(TunnelStream tunnelStream)

	{

	    _tunnelStreamList.remove(tunnelStream, TunnelStream.MANAGER_LINK);

        removeTunnelStreamFromDispatchList(tunnelStream);        

        removeTunnelStreamFromTimeoutList(tunnelStream);

        tunnelStream.streamClosed(_errorInfo.error());

	}



	

	boolean needsFlush()

	{

		boolean needsFlush = _needsFlush;

		_needsFlush = false;

		return needsFlush;

	}



	

	boolean needsDispatchNow()

	{ 

		return _tunnelStreamDispatchList.count() > 0;

	}

	

	ReactorChannel reactorChannel()

	{

		return _reactorChannel;

	}



	void setNeedsFlush()

	{

		_needsFlush = true;

	}



	

	void close()

	{

        /* Remove all associated tunnel streams. */

        for (TunnelStream tunnelStream = _tunnelStreamList.start(TunnelStream.MANAGER_LINK);

                tunnelStream != null;

                tunnelStream = _tunnelStreamList.forth(TunnelStream.MANAGER_LINK))

            removeTunnelStream(tunnelStream);

	}

	

	void sendTunnelStreamStatusCloseRecover(TunnelStream tunnelStream, Error error)

	{

        _tmpState.clear();

        _tmpState.streamState(StreamStates.CLOSED_RECOVER);

        _tmpState.dataState(DataStates.SUSPECT);

        _tmpState.text().data(error.text());

        Reactor reactor = tunnelStream.reactorChannel().reactor();

        reactor.sendTunnelStreamStatusEventCallback(tunnelStream.reactorChannel(),

                                                    tunnelStream,

                                                    null,

                                                    null,

                                                    _tmpState,

                                                    null,

                                                    _errorInfo);	    

	}

	

	void sendTunnelStreamStatusClose(TunnelStream tunnelStream, Error error)

	{

        _tmpState.clear();

        

        _tmpState.streamState(StreamStates.CLOSED);



        _tmpState.dataState(DataStates.SUSPECT);

              

        _tmpState.text().data(error.text());

        TunnelStream reactorTunnelStream = tunnelStream;

        Reactor reactor = reactorTunnelStream.reactorChannel().reactor();

        reactor.sendTunnelStreamStatusEventCallback(reactorTunnelStream.reactorChannel(),

                                                    reactorTunnelStream,

                                                    null,

                                                    null,

                                                    _tmpState,

                                                    null,

                                                    _errorInfo);	    

	}	

	

	

	void sendTunnelStreamStatusPendingClose(TunnelStream tunnelStream, Error error)

	{

        _tmpState.clear();

       

        _tmpState.streamState(StreamStates.OPEN);

        

        _tmpState.dataState(DataStates.SUSPECT);

              

        _tmpState.text().data(error.text());

        Reactor reactor = tunnelStream.reactorChannel().reactor();

        reactor.sendTunnelStreamStatusEventCallback(tunnelStream.reactorChannel(),

                                                    tunnelStream,

                                                    null,

                                                    null,

                                                    _tmpState,

                                                    null,

                                                    _errorInfo);	    

	}	

	

    

	void sendTunnelStreamStatus(TunnelStream tunnelStream, State state, Msg msg, LoginMsg loginMsg)

    {

        _tmpState.clear();

        state.copy(_tmpState);

        if (state != null)

        {

            state.copy(tunnelStream.state());

        }

        Reactor reactor = tunnelStream.reactorChannel().reactor();

        reactor.sendTunnelStreamStatusEventCallback(tunnelStream.reactorChannel(),

                                                    tunnelStream,

                                                    null,

                                                    msg,

                                                    _tmpState,

                                                    loginMsg,

                                                    _errorInfo);        

    }

}

