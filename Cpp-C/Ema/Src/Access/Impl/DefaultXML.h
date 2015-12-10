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
	"DefaultNiProvider",
	"DefaultSession",
	"Dictionary",
	"EnumTypeDefFileName",
	"FileName",
	"Host",
	"HsmInterface",
	"HsmMultAddress",
	"HsmPort",
	"InterfaceName",
	"Logger",
	"Name",
	"ObjectName",
	"Port",
	"RdmFieldDictionaryFileName",
	"RecvAddress",
	"RecvPort",
	"SendAddress",
	"SendPort",
	"UnicastPort",
	"XmlTraceFileName",
	"tcpControlPort",
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
	"CatchUnhandledException",
	"CompressionThreshold",
	"ConnectionPingTimeout",
	"DictionaryRequestTimeOut",
	"DirectoryRequestTimeOut",
	"DisconnectOnGap",
	"GuaranteedOutputBuffers",
	"HsmInterval",
	"IncludeDateInLoggerOutput",
	"ItemCountHint",
	"LoginRequestTimeOut",
	"MaxDispatchCountApiThread",
	"MaxDispatchCountUserThread",
	"MaxOutstandingPosts",
	"MsgKeyInUpdates",
	"NumInputBuffers",
	"ObeyOpenWindow",
	"PacketTTL",
	"PostAckTimeout",
	"RequestTimeout",
	"ServiceCountHint",
	"SysRecvBufSize",
	"SysSendBufSize",
	"TcpNodelay",
	"XmlTraceRead",
	"XmlTraceToFile",
	"XmlTraceToMultipleFiles",
	"XmlTraceToStdout",
	"XmlTraceWrite",
	"ndata",
	"nmissing",
	"nrreq",
	"pktPoolLimitHigh",
	"pktPoolLimitLow",
	"tbchold",
	"tdata",
	"tpphold",
	"trreq",
	"twait",
	"userQLimit",
};

#endif //__thomsonreuters_ema_access_DefaultXML_h
