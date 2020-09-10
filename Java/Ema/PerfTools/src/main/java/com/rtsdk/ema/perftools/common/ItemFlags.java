package com.rtsdk.ema.perftools.common;

/** Item flags. */
public class ItemFlags
{
	/** Provider should send updates */
	public static final int IS_STREAMING_REQ	= 0x04;
	/** Item was requested(not published) */
	public static final int IS_SOLICITED		= 0x10;
	/** Consumer should send posts */
	public static final int IS_POST				= 0x20;
	/** Consumer should send generic msgs */
	public static final int IS_GEN_MSG			= 0x40;
	/** Consumer should request private stream */
	public static final int IS_PRIVATE			= 0x80;
}
