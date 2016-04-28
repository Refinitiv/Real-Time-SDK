/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_DefaultXML_h
#define __thomsonreuters_ema_access_DefaultXML_h

#include "EmaString.h"

thomsonreuters::ema::access::EmaString AsciiValues[] = {
	"Channel",
	"ChannelSet",
	"ConsumerName",
	"DefaultConsumer",
	"DefaultSession",
	"Dictionary",
	"EnumTypeDefFileName",
	"FileName",
	"Host",
	"InterfaceName",
	"Logger",
	"Name",
	"Port",
	"RecvAddress",
	"RecvPort",
	"SendAddress",
	"SendPort",
	"UnicastPort",
	"HsmInterface",
	"HsmMultAddress",
	"HsmPort",
	"tcpControlPort",
	"ObjectName",
	"RdmFieldDictionaryFileName",
	"XmlTraceFileName",
};

thomsonreuters::ema::access::EmaString EnumeratedValues[] = {
	"ChannelType",
	"CompressionType",
	"DictionaryType",
	"LoggerSeverity",
	"LoggerType",
};

thomsonreuters::ema::access::EmaString Int64Values[] = {
	"DispatchTimeoutApiThread",
	"PipePort",
	"ReconnectAttemptLimit",
	"ReconnectMaxDelay",
	"ReconnectMinDelay",
	"XmlTraceMaxFileSize",
};

thomsonreuters::ema::access::EmaString UInt64Values[] = {
	"ConnectionPingTimeout",
	"CompressionThreshold",
	"DictionaryRequestTimeOut",
	"DirectoryRequestTimeOut",
	"GuaranteedOutputBuffers",
	"CatchUnhandledException",
	"IncludeDateInLoggerOutput",
	"ItemCountHint",
	"LoginRequestTimeOut",
	"MaxDispatchCountApiThread",
	"MaxDispatchCountUserThread",
	"MaxOutstandingPosts",
	"MsgKeyInUpdates",
	"NumInputBuffers",
	"ObeyOpenWindow",
	"PostAckTimeout",
	"RequestTimeout",
	"ServiceCountHint",
	"SysRecvBufSize",
	"SysSendBufSize",
	"TcpNodelay",
	"PacketTTL",
	"DisconnectOnGap",
	"ndata",
	"nmissing",
	"nrreq",
	"tdata",
	"trreq",
	"pktPoolLimitHigh",
	"pktPoolLimitLow",
	"twait",
	"tbchold",
	"tpphold",
	"userQLimit",
	"HsmInterval",
	"XmlTracePing",
	"XmlTraceHex",
	"XmlTraceRead",
	"XmlTraceToFile",
	"XmlTraceToMultipleFiles",
	"XmlTraceToStdout",
	"XmlTraceWrite",
};

#endif //__thomsonreuters_ema_access_DefaultXML_h
