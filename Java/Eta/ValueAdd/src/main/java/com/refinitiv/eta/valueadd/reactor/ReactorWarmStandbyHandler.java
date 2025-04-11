/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.valueadd.common.VaNode;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;

/* WarmStandByHandlerImpl
 * Handles a list of RsslChannels that is used to handle multiple connections for the warm standby feature. 
 * This is an internal class mainly used to store all information required for the warm standby feature
 * Each warm standby handler the warm standby group impl list in the deep copied connection opts from the main reactor channel.
 * The actual functionality for the warm standby feature is in the reactor and watchlist components.
 */
class ReactorWarmStandbyHandler
{
	private VaNode reactorQueueLink;
	private List<ReactorChannel> channelList; // Keeps a list of reactor channel
	private List<ReactorWLSubmitMsgOptions> submitMsgQueue;
	private List<ReactorWLSubmitMsgOptions> freeSubmitMsgQueue;
	private ReactorChannel activeReactorChannel; // Returns the active server channel for the login based 
	private ReactorChannel nextActiveReactorChannel; // Keeps the next active channel for the login based 
	private ReactorChannel mainReactorChannelImpl; // The customer facing ReactorChannel to represent the warm standby feature. 
	private ReactorChannel startingReactorChannel; // The starting reactor channel from ReactorConnect() 
	private int mainChannelState; // The state is defined in ReactorWarmStandbyHandlerChannelState 
	private ReactorConnectOptions connectionOptions;  // The full connection options for this channel
													  // The connection options here have a fully deep copied ReactorWarmStandbyGroupImpl set,
													  // which is what the reactor and watchlist will operate on for each group.
	private int currentWarmStandbyGroupIndex; // Represents the current Warm Standby Group in the queue.
	private int previousWarmStandbyGroupIndex; // Represents the Warm Standby Group in the queue that was held when we tried to connect to the preferred Warm Standby Group
	private int warmStandbyHandlerState; // The state is defined in ReactorWarmStandbyHandlerState 
	private Reactor reactor; // The reactor component for this handler. 
	private LoginRefresh rdmLoginRefresh = (LoginRefresh)LoginMsgFactory.createMsg(); // Keeps login response of the first active server. 
	private State rdmLoginState = CodecFactory.createState(); // The current state of the login stream. 
	private boolean hasConnectionList; // This is used to indicate if a connection list is specified. 
	private ReactorChannel readMsgChannel; // keeps track which channel reads the current processing message. 
	private boolean	queuedRecoveryMessage; // This is used to indicate for queuing recovery message when moving to another warm standby group. 
	private boolean	enableSessionMgnt; // This is used to indicate whether this warm standby channel enables session management. 
	private boolean watchlistSentFirstRequests; // Used to tell if we've already sent our requests to our first connection, as they are handled by the watchlist afterwards
	private Lock warmStandByHandlerLock = new ReentrantLock();
	
	// For applying ioctl 
	private int	ioCtlCodes; // Flag set of ioctl codes for all channels. Defined in RsslReactorWSIoctlCodes 
	private int maxNumBuffers;
	private int	numGuaranteedBuffers;
	private int	highWaterMark;
	private int	systemReadBuffers;
	private int	systemWriteBuffers;
	private int	compressionThreshold;
	private int rdmFieldVersion;
	private int rdmEnumTypeVersion;
	
	ReactorWarmStandbyHandler()
	{	
		clear();
	}
	
	void clear()
	{
		if (reactorQueueLink == null)
		{
			reactorQueueLink = new VaNode();
		}
		
		if (channelList == null)
			channelList = new ArrayList<ReactorChannel>();
		
		channelList.clear();
		
		if (submitMsgQueue == null)
			submitMsgQueue = new LinkedList<ReactorWLSubmitMsgOptions>();
		submitMsgQueue.clear();
		
		if (freeSubmitMsgQueue == null)
			freeSubmitMsgQueue = new LinkedList<ReactorWLSubmitMsgOptions>();
		
		freeSubmitMsgQueue.clear();
		mainChannelState(ReactorWarmStandbyHandlerChannelStateImpl.INITIALIZING);
		warmStandbyHandlerState(ReactorWarmStandbyHandlerState.INITIALIZING);
		rdmLoginRefresh().clear();
		rdmLoginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		
		connectionOptions = null;
		
		currentWarmStandbyGroupIndex = -1;
		previousWarmStandbyGroupIndex = -1;
		reactor(null);
		rdmLoginState().clear();
		if (readMsgChannel() != null)
			readMsgChannel = null;
		queuedRecoveryMessage(false);
		enableSessionMgnt(false);
		
		ioCtlCodes(0);
		maxNumBuffers = 0;
		highWaterMark = 0;
		systemReadBuffers = 0;
		systemWriteBuffers = 0;
		compressionThreshold = 0;
		hasConnectionList = false;
	}

	VaNode reactorQueueLink() 
	{
		return reactorQueueLink;
	}

	List<ReactorChannel> channelList() 
	{
		return channelList;
	}

	List<ReactorWLSubmitMsgOptions> submitMsgQueue() 
	{
		return submitMsgQueue;
	}

	List<ReactorWLSubmitMsgOptions> freeSubmitMsgQueue() 
	{
		return freeSubmitMsgQueue;
	}

	ReactorChannel activeReactorChannel() 
	{
		return activeReactorChannel;
	}

	void activeReactorChannel(ReactorChannel activeReactorChannel) 
	{
		this.activeReactorChannel = activeReactorChannel;
	}

	ReactorChannel nextActiveReactorChannel()
	{
		return nextActiveReactorChannel;
	}

	void nextActiveReactorChannel(ReactorChannel nextActiveReactorChannel) 
	{
		this.nextActiveReactorChannel = nextActiveReactorChannel;
	}

	ReactorChannel mainReactorChannelImpl()
	{
		return mainReactorChannelImpl;
	}

	void mainReactorChannelImpl(ReactorChannel mainReactorChannelImpl) 
	{
		this.mainReactorChannelImpl = mainReactorChannelImpl;
	}

	ReactorChannel startingReactorChannel() 
	{
		return startingReactorChannel;
	}

	void startingReactorChannel(ReactorChannel startingReactorChannel) 
	{
		this.startingReactorChannel = startingReactorChannel;
	}

	int mainChannelState() 
	{
		return mainChannelState;
	}

	void mainChannelState(int mainChannelState) 
	{
		this.mainChannelState = mainChannelState;
	}
	
	void connectionOptions(ReactorConnectOptions opts ) 
	{
		this.connectionOptions = opts;
	}
	
	ReactorConnectOptions getConnectionOptions()
	{
		return connectionOptions;
	}
	
	ReactorWarmStandbyGroupImpl currentWarmStandbyGroupImpl()
	{
		if(currentWarmStandbyGroupIndex != -1)
			return (ReactorWarmStandbyGroupImpl)connectionOptions._reactorWarmStandyGroupList.get(currentWarmStandbyGroupIndex);
		else
			return null;
	}
	
	ReactorWarmStandbyGroupImpl previousWarmStandbyGroupImpl()
	{
		if(previousWarmStandbyGroupIndex != -1)
			return (ReactorWarmStandbyGroupImpl)connectionOptions._reactorWarmStandyGroupList.get(previousWarmStandbyGroupIndex);
		else
			return null;
	}
	
	List<ReactorWarmStandbyGroup> warmStandbyGroupList()
	{
		return connectionOptions._reactorWarmStandyGroupList;
	}

	int currentWarmStandbyGroupIndex()
	{
		return currentWarmStandbyGroupIndex;
	}
	
	int previousWarmStandbyGroupIndex()
	{
		return previousWarmStandbyGroupIndex;
	}
	
	void currentWarmStandbyGroupIndex(int currentWarmStandbyGroupIndex)
	{
		this.currentWarmStandbyGroupIndex = currentWarmStandbyGroupIndex;
	}
	
	void previousWarmStandbyGroupIndex(int previousWarmStandbyGroupIndex)
	{
		this.previousWarmStandbyGroupIndex = previousWarmStandbyGroupIndex;
	}
	
	void incrementWarmStandbyGroupIndex()
	{
		currentWarmStandbyGroupIndex++;
	}

	int warmStandbyHandlerState()
	{
		return warmStandbyHandlerState;
	}

	void warmStandbyHandlerState(int warmStandbyHandlerState) 
	{
		this.warmStandbyHandlerState = warmStandbyHandlerState;
	}
	
	void setPrimaryLoginResponseState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_LOGIN_RESPONSE;
	}
	
	void setPrimaryDirectoryResponseState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_DIRECTORY_RESPONSE;
	}
	
	void setSecondaryDirectoryResponseState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.RECEIVED_SECONDARY_DIRECTORY_RESPONSE;
	}
	
	void setConnectingToStartingServerState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.CONNECTING_TO_A_STARTING_SERVER;
	}
	
	void setMoveToChannelListState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.MOVE_TO_CHANNEL_LIST;
	}
	
	void setClosingStandbyChannelsState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.CLOSING_STANDBY_CHANNELS;
	}	
	
	void setMoveToNextWSBGroupState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.MOVE_TO_NEXT_WSB_GROUP;
	}	
	
	void setReceivedPrimaryFieldDictionaryState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_FIELD_DICTIONARY_RESPONSE;
	}	
	
	void setReceivedPrimaryEnumDictionaryState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_ENUM_DICTIONARY_RESPONSE;
	}	
	
	void setClosingState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.CLOSING;
	}	
	
	void setInactiveState()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.INACTIVE;
	}	
	
	void setMovedToChannelList()
	{
		this.warmStandbyHandlerState |= ReactorWarmStandbyHandlerState.MOVED_TO_CHANNEL_LIST;
	}	
	
	Reactor reactor()
	{
		return reactor;
	}

	void reactor(Reactor reactor)
	{
		this.reactor = reactor;
	}

	LoginRefresh rdmLoginRefresh() 
	{
		return rdmLoginRefresh;
	}

	void rdmLoginRefresh(LoginRefresh rdmLoginRefresh)
	{
		rdmLoginRefresh.copy(this.rdmLoginRefresh);
	}

	State rdmLoginState() 
	{
		return rdmLoginState;
	}

	void rdmLoginState(State rdmLoginState) 
	{
		this.rdmLoginState = rdmLoginState;
	}

	int rdmFieldVersion() 
	{
		return rdmFieldVersion;
	}

	void rdmFieldVersion(int rdmFieldVersion)
	{
		this.rdmFieldVersion = rdmFieldVersion;
	}

	int rdmEnumTypeVersion()
	{
		return rdmEnumTypeVersion;
	}

	void rdmEnumTypeVersion(int rdmEnumTypeVersion)
	{
		this.rdmEnumTypeVersion = rdmEnumTypeVersion;
	}

	ReactorChannel readMsgChannel()
	{
		return readMsgChannel;
	}

	void readMsgChannel(ReactorChannel readMsgChannel) 
	{
		this.readMsgChannel = readMsgChannel;
	}

	boolean queuedRecoveryMessage()
	{
		return queuedRecoveryMessage;
	}

	void queuedRecoveryMessage(boolean queuedRecoveryMessage) 
	{
		this.queuedRecoveryMessage = queuedRecoveryMessage;
	}

	boolean enableSessionMgnt() 
	{
		return enableSessionMgnt;
	}

	void enableSessionMgnt(boolean enableSessionMgnt) 
	{
		this.enableSessionMgnt = enableSessionMgnt;
	}

	int ioCtlCodes() 
	{
		return ioCtlCodes;
	}

	void ioCtlCodes(int ioCtlCodes) 
	{
		this.ioCtlCodes = ioCtlCodes;
	}

	int maxNumBuffers() 
	{
		return maxNumBuffers;
	}

	void maxNumBuffers(int maxNumBuffers) 
	{
		ioCtlCodes |= ReactorWsbIoctlCodes.MAX_NUM_BUFFERS;
		this.maxNumBuffers = maxNumBuffers;
	}

	int numGuaranteedBuffers() 
	{
		return numGuaranteedBuffers;
	}

	void numGuaranteedBuffers(int numGuaranteedBuffers) 
	{
		ioCtlCodes |= ReactorWsbIoctlCodes.NUM_GUARANTEED_BUFFERS;
		this.numGuaranteedBuffers = numGuaranteedBuffers;
	}

	int highWaterMark() 
	{
		return highWaterMark;
	}

	void highWaterMark(int highWaterMark) 
	{
		ioCtlCodes |= ReactorWsbIoctlCodes.HIGH_WATER_MARK;
		this.highWaterMark = highWaterMark;
	}

	int systemReadBuffers() 
	{
		return systemReadBuffers;
	}

	void systemReadBuffers(int systemReadBuffers) 
	{
		ioCtlCodes |= ReactorWsbIoctlCodes.SYSTEM_READ_BUFFERS;
		this.systemReadBuffers = systemReadBuffers;
	}

	int systemWriteBuffers() 
	{
		return systemWriteBuffers;
	}

	void systemWriteBuffers(int systemWriteBuffers) 
	{
		ioCtlCodes |= ReactorWsbIoctlCodes.SYSTEM_WRITE_BUFFERS;
		this.systemWriteBuffers = systemWriteBuffers;
	}

	int compressionThreshold() 
	{
		return compressionThreshold;
	}

	void compressionThreshold(int compressionThreshold) 
	{
		ioCtlCodes |= ReactorWsbIoctlCodes.COMPRESSION_THRESHOLD;
		this.compressionThreshold = compressionThreshold;
	}
	
	boolean hasConnectionList() 
	{
		return hasConnectionList;
	}

	void hasConnectionList(boolean hasConnectionList) 
	{
		this.hasConnectionList = hasConnectionList;
	}

	boolean watchlistSentFirstRequests() 
	{
		return watchlistSentFirstRequests;
	}
	
	void watchlistSentFirstRequests(boolean watchlistSentFirstRequests) 
	{
		this.watchlistSentFirstRequests = watchlistSentFirstRequests;
	}
	
	Lock warmStandByHandlerLock()
	{
		return warmStandByHandlerLock;
	}
	
}

class ReactorWSRecoveryMsgInfo
{
	int _domainType;
	int _containerType;
	int _streamId;
	MsgKey _msgKey = CodecFactory.createMsgKey();
	State _msgState = CodecFactory.createState();
	int _flags;
	Object _userSpecObject = null;
	Buffer _serviceName = CodecFactory.createBuffer();
	
	ReactorWSRecoveryMsgInfo()
	{
		clear();
	}
	
	void clear()
	{
		_domainType = 0;
		_containerType = DataTypes.UNKNOWN;
		_streamId = 0;
		_msgKey.clear();
		_msgState.clear();
		_flags = 0;
		_userSpecObject = null;
		_serviceName.clear();
	}
	
}

class ReactorWLSubmitMsgOptions
{
	ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	Msg msg = CodecFactory.createMsg();
	long submitTime;
	
	ReactorWLSubmitMsgOptions()
	{
		clear();
	}
	
	void clear()
	{
		submitOptions.clear();
		msg.clear();
		submitTime = 0;
	}
}

class ReactorWsbIoctlCodes
{
	public static final int MAX_NUM_BUFFERS = 0x01;
	public static final int NUM_GUARANTEED_BUFFERS = 0x02;
	public static final int HIGH_WATER_MARK = 0x04;
	public static final int SYSTEM_WRITE_BUFFERS = 0x08;
	public static final int SYSTEM_READ_BUFFERS = 0x10;
	public static final int COMPRESSION_THRESHOLD = 0x20;
}
