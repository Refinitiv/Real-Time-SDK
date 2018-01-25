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
	"CapabilitiesEntry",
	"Channel",
	"ChannelSet",
	"DefaultConsumer",
	"DefaultDictionary",
	"DefaultDirectory",
	"DefaultIProvider",
	"DefaultNiProvider",
	"DictionariesProvidedEntry",
	"DictionariesUsedEntry",
	"Dictionary",
	"Directory",
	"EnumTypeDefFileName",
	"EnumTypeDefItemName",
	"FileName",
	"Host",
	"HsmInterface",
	"HsmMultAddress",
	"HsmPort",
	"InterfaceName",
	"ItemList",
	"Logger",
	"Name",
	"ObjectName",
	"Port",
	"ProxyHost",
	"ProxyPort",
	"Rate",
	"RdmFieldDictionaryFileName",
	"RdmFieldDictionaryItemName",
	"RecvAddress",
	"RecvPort",
	"SendAddress",
	"SendPort",
	"Server",
	"StatusText",
	"tcpControlPort",
	"Timeliness",
	"UnicastPort",
	"Vendor",
	"XmlTraceFileName",
};

thomsonreuters::ema::access::EmaString EnumeratedValues[] = {
	"ChannelType",
	"CompressionType",
	"DataState",
	"DictionaryType",
	"LoggerSeverity",
	"LoggerType",
	"ServerType",
	"StatusCode",
	"StreamState",
};

thomsonreuters::ema::access::EmaString Int64Values[] = {
	"DictionaryID",
	"DispatchTimeoutApiThread",
	"PipePort",
	"ReconnectAttemptLimit",
	"ReconnectMaxDelay",
	"ReconnectMinDelay",
	"XmlTraceMaxFileSize",
};

thomsonreuters::ema::access::EmaString UInt64Values[] = {
	"AcceptDirMessageWithoutMinFilters",
	"AcceptingConsumerStatus",
	"AcceptingRequests",
	"AcceptMessageSameKeyButDiffStream",
	"AcceptMessageThatChangesService",
	"AcceptMessageWithoutAcceptingRequests",
	"AcceptMessageWithoutBeingLogin",
	"AcceptMessageWithoutQosInRange",
	"CatchUnhandledException",
	"CompressionThreshold",
	"ConnectionMinPingTimeout",
	"ConnectionPingTimeout",
	"DictionaryRequestTimeOut",
	"DirectoryRequestTimeOut",
	"DisconnectOnGap",
	"EnumTypeFragmentSize",
	"FieldDictionaryFragmentSize",
	"GuaranteedOutputBuffers",
	"HighWaterMark",
	"HsmInterval",
	"IncludeDateInLoggerOutput",
	"ItemCountHint",
	"IsSource",
	"LoginRequestTimeOut",
	"MaxDispatchCountApiThread",
	"MaxDispatchCountUserThread",
	"MaxOutstandingPosts",
	"MergeSourceDirectoryStreams",
	"MsgKeyInUpdates",
	"NumInputBuffers",
	"ObeyOpenWindow",
	"PacketTTL",
	"PostAckTimeout",
	"RecoverUserSubmitSourceDirectory",
	"RefreshFirstRequired",
	"RemoveItemsOnDisconnect",
	"RequestTimeout",
	"ServiceCountHint",
	"ServiceId",
	"ServiceState",
	"SupportsOutOfBandSnapshots",
	"SupportsQoSRange",
	"SysRecvBufSize",
	"SysSendBufSize",
	"TcpNodelay",
	"XmlTraceHex",
	"XmlTracePing",
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

thomsonreuters::ema::access::EmaString NodesThatRequireName[] = {
	"Channel",
	"Consumer",
	"Dictionary",
	"Directory",
	"IProvider"
	"Logger"
	"NiProvider",
	"Server",
	"Service",
};

#endif //__thomsonreuters_ema_access_DefaultXML_h
