package com.refinitiv.ema.perftools.emajconsperf;

/** Item request state. */
public class ItemRequestState
{
	/** Item request has not been set. */
	public static final int NOT_REQUESTED = 0;
	/** Item is waiting for its solicited refresh. */
	public static final int WAITING_FOR_REFRESH = 1;
	/** Item has received its solicited refresh. */
	public static final int HAS_REFRESH = 2;
}
