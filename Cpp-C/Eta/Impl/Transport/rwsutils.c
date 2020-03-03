/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "rtr/tr_sha_1.h"

#include "rtr/ripc_int.h"
#include "rtr/rwsutils.h"

/* Per RFC7230 & RFC6455, The WebSocket HTTP header fields and values are parsed with
 * the following definitions */
/*token = 1*tchar (tchar - token characters)
  tchar = "!" / "#" / "$" / "%" / "&" / "’" 
        / "*" / "+" / "-" / "." / "^" / "_" 
		/ "‘" / "|" / "~" / DIGIT / ALPHA   ; any VCHAR, except delimiters*/
/* An HTTP Header Field 'field-name ":" OWS field-value OWS' (OWS - Optional White Space)
 *		field-name is = token 
 *		field-value is = field-vchar [ 1*(SP/HTAB) field-vchar ] 

 *  The following are delimiters for any header field token values  
		fiels-value delimiters = ( ),/:;<=>?@[\]{} */
#define IS_TOKEN_DEL(ch) \
		((ch=='(' || ch==')' || ch==','  || ch=='/' || ch==':') ||\
		 (ch==';' || ch=='<' || ch=='='  || ch=='>' || ch=='?') ||\
		 (ch=='@' || ch=='[' || ch=='\\' || ch==']' || ch=='{') ||\
		 (ch=='}' || ch==SP  || ch==HT ) )

static const char field_tokens[256] = {
		0,    0,  0,  0,  0,  0,  0,   0, /* 0  nul soh stx etx eot enq ack bel  7 */
		0,    0,  0,  0,  0,  0,  0,   0, /* 8  bs  ht  nl  vt  np  cr  so  si  15 */
		0,    0,  0,  0,  0,  0,  0,   0, /* 16 dle dc1 dc2 dc3 dc4 nak syn etb 23 */
		0,    0,  0,  0,  0,  0,  0,   0, /* 24 can em  sub esc fs  gs  rs  us  31 */ 
		0,  '!',  0,'#','$','%','&','\'', /* 32 sp   !   "   #  $   %   &   '   39 */
		0,    0,'*','+',  0,'-','.',   0, /* 40  (   )   *   +  ,   -   .   /   47 */
		'0','1','2','3','4','5','6', '7', /* 48  0   1   2   3  4   5   6   7   55 */
		'8','9',  0,  0,  0,  0,  0,   0, /* 56  8   9   :   ;  <   =   >   ?   63 */
		0,  'a','b','c','d','e','f', 'g', /* 64  @   A   B   C  D   E   F   G   71 */
		'h','i','j','k','l','m','n', 'o', /* 72  H   I   J   K  L   M   N   O   79 */
		'p','q','r','s','t','u','v', 'w', /* 80  P   Q   R   S  T   U   V   W   87 */
		'x','y','z',  0,  0,  0,'^', '_', /* 88  X   Y   Z   [  \   ]   ^   _   95 */
		'`','a','b','c','d','e','f', 'g', /* 96  `   a   b   c  d   e   f   g  103 */
		'h','i','j','k','l','m','n', 'o', /* 104 h   i   j   k  l   m   n   o  111 */
		'p','q','r','s','t','u','v', 'w', /* 112 p   q   r   s  t   u   v   w  119 */
		'x','y','z',  0,'|',  0,'~',   0, /* 120 x   y   z   {  |   }   ~   del 127*/
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, /* 16-18 */
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, /* 19-21 */
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, /* 22-24 */
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, /* 25-27 */
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, /* 28-30 */
		0,0,0,0,0,0,0,0                                    /* 31 */
};                                                                                    

#define	CR '\r'
#define	LF '\n'
#define	SP ' '
#define	HT '\t'

#define IS_WHITESP(ch) ( (ch == SP) || (ch == HT) )

#define ISNUM_CH(ch) (ch >='0' && ch <='9')
#define ISLOWER(ch)	(ch >=97 && ch <=122)
#define ISUPPER(ch)	(ch >=65 && ch <=90)
#define ISALPHA(ch) ( (ISLOWER(ch))||(ISUPPER(ch)) )
#define TOLOWER(ch)  (ISALPHA(ch)?((unsigned char)(ch | 0x20)):ch)

#define IS_COLON(ch) (ch==':')
#define IS_LF(ch)	(ch == LF)
#define IS_CR(ch)	(ch == CR)

#define ISNUM_CH(ch) (ch >='0' && ch <='9')
#define ISLO_CH(ch)	(ch >='a' && ch <='z')
#define ISUP_CH(ch)	(ch >='A' && ch <='Z')
#define ISALNUM_CH(ch) ( (ISLO_CH(ch))||(ISUP_CH(ch))||(ISNUM_CH(ch)) )

#define IS_64KEYCHAR(ch) ((ISALNUM_CH(ch))||ch=='+'||ch=='='||ch=='/')

#define TCHAR(ch)	(field_tokens[(unsigned char)ch])

#ifdef _DEBUG_PARSE_HTTP
#define _DBG_HTTP_PRINT_LINE(l, buf, start, end) \
	{ int pos = start; int eoLn = end;\
		fprintf(stderr, "   Line %02d:", l);\
		if (buf && buf[end] == CR && buf[end+1] && buf[end+1] == LF) eoLn++;\
		if (buf && buf[pos] && pos <= eoLn) {\
			while(pos <= eoLn) {\
				if (buf[pos] == '\r') fprintf(stderr, "<CR>");\
				else if (buf[pos] == '\n') fprintf(stderr, "<LF>");\
				else fprintf(stderr, "%c", buf[pos]);\
				pos++; }\
			fprintf(stderr, "\n"); }\
	}

#define _DBG_HTTP_DEFLABLE(buf, len, lable)\
		{	int p=0; fprintf(stderr, "%s '", lable);\
			for ( ; (p < len && buf[p]!=CR && buf[p]!=LF) ; p++){\
				fprintf(stderr, "%c", buf[p]); }\
			fprintf(stderr, "' \n");\
		}
	
#define _DBG_HTTP_PROCENTRY(buf, len)\
		{	int p=0; fprintf(stderr, " Process Entry '");\
			for ( ; (p < len && buf[p]!=CR && buf[p]!=LF) ; p++){\
				fprintf(stderr, "%c", buf[p]); }\
			fprintf(stderr, "' \n");\
		}
	
#define _DBG_HTTP_FIELD(buf, len)\
		{	int p=0; fprintf(stderr, "\tField '");\
			for ( ; (p < len) ; p++){\
				fprintf(stderr, "%c", buf[p]); }\
			fprintf(stderr, "' \n");\
		}

#define _DBG_HTTP_VALUE(buf, len)\
		{	int p=0; fprintf(stderr, "\tValue '");\
			for ( ; (p < len) ; p++){\
				fprintf(stderr, "%c", buf[p]); }\
			fprintf(stderr, "'\n ");\
		}

#define _DBG_HTTP_FIELDNAME_L(buf, len)\
		{	int p=0; fprintf(stderr, " Field '");\
			for ( ; (p < len && buf[p]!=CR && buf[p]!=LF) ; p++){\
				fprintf(stderr, "%c", buf[p]); }\
			fprintf(stderr, "' \n");\
		}

#define _DBG_HTTP_FIELDVALUE_L(buf, len)\
		{	int p=0; fprintf(stderr, "\tValue '");\
			for ( ; (p < len && buf[p]!=CR && buf[p]!=LF) ; p++){\
				fprintf(stderr, "%c", buf[p]); }\
			fprintf(stderr, "'\n ");\
		}

#define _DBG_HTTP_FIELDNAME(buf, s, eoLn) \
		{	int p=s; fprintf(stderr, " Field '");\
			while(p<=eoLn && (buf[p]!=CR && buf[p]!=LF)){ \
				fprintf(stderr, "%c", buf[p]); p++; }\
			fprintf(stderr, "' \n");\
		}

#define _DBG_HTTP_FIELDVALUE(buf, s, eoLn) \
		{	int p=s; fprintf(stderr, "\tValue '");\
			while(p<=eoLn && (buf[p]!=CR && buf[p]!=LF)){ \
				fprintf(stderr, "%c", buf[p]); p++; }\
			fprintf(stderr, "'\n ");\
		}

#define _DBG_HTTP_TOKEN(buf, s, eoLn) \
		{	int p=s; fprintf(stderr, "\t\tToken '");\
			while(p<=eoLn && (field_tokens[((unsigned char)buf[p])])){ \
				fprintf(stderr, "%c", buf[p]); p++; }\
			fprintf(stderr, "'\n ");\
		}
#else
#define _DBG_HTTP_PRINT_LINE(l, buf, start, end)

#define _DBG_HTTP_DEFLABLE(buf, len, lable)
	
#define _DBG_HTTP_PROCENTRY(buf, len)
	
#define _DBG_HTTP_FIELD(buf, len)

#define _DBG_HTTP_VALUE(buf, len)

#define _DBG_HTTP_FIELDNAME_L(buf, len)

#define _DBG_HTTP_FIELDVALUE_L(buf, len)

#define _DBG_HTTP_FIELDNAME(buf, s, eoLn) 

#define _DBG_HTTP_FIELDVALUE(buf, s, eoLn) 

#define _DBG_HTTP_TOKEN(buf, s, eoLn) 

#endif // _DEBUG_PARSE_HTTP

/* This is a case sensitive validations */
#define IS_HTTP_GET(buf, p, eoLn) \
	( (p <= (eoLn - 3)) && buf[p] == 'G' && buf[p+1] == 'E' && buf[p+2] == 'T' )

/* This is a case sensitive validations */
#define IS_HTTP_HEADER(buf, p, eoLn) \
	( (p <= (eoLn - 4)) && buf[p] == 'H' && buf[p+1] == 'T' && buf[p+2] == 'T' && buf[p+3] == 'P' )

#define RWS_MOVE_TO_WHITESP(buf, p, eoLn) \
	for ( ; (p < eoLn) && buf[p] != SP && buf[p] != HT ; p++)

#define RWS_CONSUME_WHITESPACE(buf, p, eoLn) \
	for ( ; (p < eoLn) && (buf[p] == SP || buf[p] == HT) ; p++)

#define RWS_MOVE_TO_CHAR(buf, p, eoLn, ch) \
	for ( ; (p < eoLn) && buf[p] != ch ; p++)


/* RWS_GET_STATUS_REQUEST_FIELD:
 * Will consume the current field of the start line
 * up to the first white space or end of line
 * e.g    GET /WebSocket HTTP/1.1<CR> 
 *    buf-^p-^-p                 ^eoLn 
 *  OR
 *        HTTP/1.1 101 Switching Protocols<CR> 
 *    buf-^p  ->p-^                       ^eoLn */
#define RWS_GET_STATUS_REQUEST_FIELD(buf, p, eoLn) \
	for ( ; (p < eoLn) && (ISALNUM_CH(buf[p])||buf[p]=='.'||buf[p]=='/') && buf[p] != SP ; p++)

/* RWS_MOVE_TO_FIELDVALUE : 
 * If not non-whitespace, will consume the current and remaing 
 * whitespace delimeters up to the next token, seperator or 
 * end of line
 * e.g  Sec-WebSocket-Protocol:    tr_json2, tr_rwf<CR> 
 *  buf-^                    p-^->p^               ^eoLn 
 *       OR
 *      Sec-WebSocket-Protocol:   tr_json2, tr_rwf<CR> 
 *  buf-^                        p^-X no move     ^eoLn */
#define RWS_MOVE_TO_FIELDVALUE(buf, p, eoLn) \
	{ RWS_MOVE_TO_WHITESP(buf, p, eoLn); }

/* RWS_GET_FIELDVALUE:
 * Will consume the current and remaing tokens
 * up to the next token delimeter or end of line
 * e.g    Sec-WebSocket-Protocol: tr_json2, tr_rwf<CR> 
 *    buf-^p - -> - > - - > - p-^                 ^eoLn 
 *             OR
 *        Sec-WebSocket-Protocol: tr_json2, tr_rwf<CR> 
 *    buf-^p - -> - > - - > - p-^                 ^eoLn */
#define RWS_GET_FIELDVALUE(buf, p, eoLn) \
	for ( ; (p < eoLn) && (field_tokens[((unsigned char)buf[p])]) ; p++)


/* RWS_MOVE_TO_TOKEN : 
 * If not current token, will consume the current and remaing 
 * tokens delimeters up to the next token or end of line
 * e.g  Sec-WebSocket-Protocol:   tr_json2, tr_rwf<CR> 
 *  buf-^                   p-^->p^               ^eoLn 
 *       OR
 *      Sec-WebSocket-Protocol:   tr_json2, tr_rwf<CR> 
 *  buf-^                        p^-X no move     ^eoLn */
#define RWS_MOVE_TO_TOKEN(buf, p, eoLn) \
	for ( ; (p < eoLn) && (TCHAR(buf[p]) == 0) ; p++)


/* RWS_MOVE_TOKEN_END : 
 * If not at current token, will not consume any token
 * delimeters and remain in place else if on a token, will 
 * comsume tokens until at a non-token or end of line
 * e.g  Sec-WebSocket-Protocol:   tr_json2, tr_rwf<CR> 
 *  buf-^                   p-^->p^               ^eoLn 
 *       OR
 *      Sec-WebSocket-Protocol:   tr_json2, tr_rwf<CR> 
 *  buf-^                        p^-> - > ^-p     ^eoLn */
#define RWS_MOVE_TOKEN_END(buf, p, eoLn) \
	for ( ; (p < eoLn) && !(field_tokens[((unsigned char)buf[p])]) ; p++)


/* RWS_GET_ALL_FIELDVALUES :
 * Will consume the remaining line up to the end of the line 
 * e.g    Accept-Encoding:gzip, deflate<CR> 
 *    buf-^             p^- ->- - - p-^^eoLn      */
#define RWS_GET_ALL_FIELDVALUES(buf, p, eoLn) \
	for ( ; p < eoLn ; p++)

/* RWS_MOVE_TO_KEY_TOKEN : 
 * If not current Key token, will consume the current and remaing 
 * tokens delimeters up to the next valid Key token char or end of line
 * e.g  Sec-WebSocket-Key:   /naRr3GfXi9SgBfyPX7lyE2jh===<CR> 
 *  buf-^              p-^->p^                           ^eoLn 
 *      Sec-WebSocket-Key:   /naRr3GfXi9SgBfyPX7lyE2jh===<CR> 
 *  buf-^                   p^-X no move                 ^eoLn */
#define RWS_MOVE_TO_KEY_TOKEN(buf, p, eoLn) \
	for ( ; (p < eoLn) && (!IS_64KEYCHAR(buf[p])) ; p++)

/* RWS_GET_KEY_VALUE :
 * While on a valid key value, will consume the remaining 
 * line up to a non-key value or the end of the line 
 * e.g  WebSocket-Key: pSpsLZbtN3U1hvnyX0/FOQ==<CR>
 *  buf-^             p^- ->- - -> -> - - -> p-^^eoLn      */
#define RWS_GET_KEY_VALUE(buf, p, eoLn) \
	for ( ; ((p <= eoLn) && (IS_64KEYCHAR(buf[p]))) ; p++)


/* RWS_GET_FIELDNAME :
 * Will consume the current and remaing tokens
 * up to the next token delimeter or end of line
 * e.g    Sec-WebSocket-Protocol: tr_json2, tr_rwf<CR> 
 *    buf-^p - -> - > - - > - p-^                 ^eoLn */
#define RWS_GET_FIELDNAME(buf, p, eoLn) \
	for ( ; (p < eoLn) && !(IS_TOKEN_DEL(buf[p])) ; p++)
	

#define RWS_FIELD_MATCHES(buf, p, eoLn, data, length)\
	((p+length <= eoLn) && (memcmp(&buf[p], data, length) == 0) )

#define RWS_IS_MATCH(buf, p, end, data, length)\
	( ((end - p) == length) && (memcmp(&buf[p], data, length) == 0) )

#define RWS_IS_FIELDNAME(buf, bL, match)\
	((bL == match.length)&&!(memcmp(buf, match.data, match.length)))

																		/*           1         2         3         4 */
																		/* 01234567890123456789012345678901234567890 */
static const RsslBuffer rws_GUID							= { 36, (char*)"258EAFA5-E914-47DA-95CA-C5AB0DC85B11" }; 
/* Initialize WS HTTP handshake fields */
																		/*           1         2         3         4 */
																		/* 01234567890123456789012345678901234567890 */
static RsslBuffer rwsHdr_GET		= {  3, (char*)"GET" }; 
static RsslBuffer rwsHdr_HTTP		= {  8, (char*)"HTTP/1.1" }; 

static RsslBuffer rwsField_ACCEPT						= {  6, (char*)"Accept" }; 
static RsslBuffer rwsField_CONNECTION					= { 10, (char*)"Connection" }; 
static RsslBuffer rwsField_CONTENT_ENCODING			= { 16, (char*)"Content-Encoding" }; 
static RsslBuffer rwsField_CONTENT_LENGTH				= { 14, (char*)"Content-Length" }; 
static RsslBuffer rwsField_CONTENT_TYPE				= { 12, (char*)"Content-Type" }; 
static RsslBuffer rwsField_COOKIE						= {  6, (char*)"Cookie" }; 
static RsslBuffer rwsField_DATE						= {  4, (char*)"Date" }; 
static RsslBuffer rwsField_HOST						= {  4, (char*)"Host" }; 
static RsslBuffer rwsField_ORIGIN						= {  6, (char*)"Origin" }; 
static RsslBuffer rwsField_SEC_WEBSOCKET_ACCEPT		= { 20, (char*)"Sec-Websocket-Accept" }; 
static RsslBuffer rwsField_SEC_WEBSOCKET_EXTENSIONS	= { 24, (char*)"Sec-Websocket-Extensions" }; 
static RsslBuffer rwsField_SEC_WEBSOCKET_KEY			= { 17, (char*)"Sec-Websocket-Key" }; 
static RsslBuffer rwsField_SEC_WEBSOCKET_PROTOCOL		= { 22, (char*)"Sec-Websocket-Protocol" }; 
static RsslBuffer rwsField_SEC_WEBSOCKET_VERSION		= { 21, (char*)"Sec-Websocket-Version" }; 
static RsslBuffer rwsField_SET_COOKIE					= { 10, (char*)"Set-Cookie" }; 
static RsslBuffer rwsField_UPGRADE					= {  7, (char*)"Upgrade" }; 
static RsslBuffer rwsField_USER_AGENT					= { 10, (char*)"User-Agent" }; 
static RsslBuffer rwsField_WEBSOCKET					= {  9, (char*)"Websocket" }; 


														/*           1         2         3         4 */
														/* 01234567890123456789012345678901234567890 */
static RsslBuffer fv_WebSocketURI			= { 10, (char*)"/WebSocket" }; 
static RsslBuffer fv_Websocket				= {  9, (char*)"websocket" }; 
static RsslBuffer fv_PerMsgDeflate			= { 18, (char*)"permessage-deflate" };
static RsslBuffer fv_ServerNoContext		= { 26, (char*)"server_no_context_takeover" };
static RsslBuffer fv_ClientNoContext		= { 26, (char*)"client_no_context_takeover" };
static RsslBuffer fv_Upgrade				= {  7, (char*)"upgrade" };

static rwsSubProtocolList_t rwsSubProtocols[] = {
{ RWS_SP_RWF,		{ 6, (char *)"tr_rwf" },	{  8, (char *)"rssl.rwf" } },
{ RWS_SP_JSON2,	{ 8, (char *)"tr_json2" },		{ 12, (char *)"rssl.json.v2" } },
{ 0,					RSSL_INIT_BUFFER,			RSSL_INIT_BUFFER }
};

extern RsslInt32 iseof(char *, RsslInt32 , RsslInt32 );
extern RsslRet rsslWebSocketSetChannelFunctions(void);

static void _freeHttpHeader(rwsHttpHdr_t *httpHdr)
{
	int i;

	if (httpHdr && httpHdr->total > 0 && httpHdr->lines) 
	{
		for( i=(httpHdr->total-1); i >= 0; i--) { 
			if (httpHdr->lines[i].data){
				_rsslFree(httpHdr->lines[i].data);
				httpHdr->lines[i].data = 0;
			}
		}

		_rsslFree(httpHdr->lines);
		httpHdr->total = 0;
		httpHdr->lines = 0;
	}
}

static const char * _getClosedText(rwsCFStatusCodes_t code)
{
	RsslUInt16 sc = 0;

	if (code == 0 || code >= RWS_CFSC_UNKNOWN_15)
		return NULL;
		
	sc = (RsslUInt16)code;

	if (code >= RWS_CFSC_NORMAL_CLOSE)
		sc -= 1000;

	return rwsCloseFrameText[sc];
}

static void _setMaskKeyBuff(char *mask, RsslUInt32 mVal)
{
	mask[0] = (mVal >> 24) & 0xFF;
	mask[1] = (mVal >> 16) & 0xFF;
	mask[2] = (mVal >> 8) & 0xFF;
	mask[3] = mVal & 0xFF;

	return;
}

static RsslUInt32 _getMaskKey(char *mask, char *buffer)
{
	RsslUInt32 mVal = 0;

	rwfGet32(mVal, buffer);
	_setMaskKeyBuff(mask, mVal);

	return mVal;
}

static void _maskDataBlock(char *mask, char *ptrBuf, RsslUInt64 length)
{
	RsslUInt64 i;

	for (i = 0 ; i < length; i++, ptrBuf++)
		*ptrBuf = *ptrBuf ^ mask[i%4];
	
	return;
}

static int _addNewHeaderLine(rwsHttpHdr_t *httpHdr)
{
	headerLine_t *hdrLn;

	if (httpHdr == 0)
		return (-1);

	hdrLn = (headerLine_t *)realloc((void*)(httpHdr->lines),(httpHdr->total+1)*sizeof(struct headerLine));
	if (!hdrLn)
		return (-1);

	httpHdr->lines = hdrLn;
	hdrLn = &httpHdr->lines[httpHdr->total];

	httpHdr->total++;

	return httpHdr->total;
}

#ifndef SHA_DIGEST_LENGTH
	#define SHA_DIGEST_LENGTH 20
#endif

#define BASE64_LENGTH(inlen) ((((inlen) + 2) / 3) * 4)

static char *_base64Encode(unsigned char* hash, size_t len)
{
    rtrInt32    i, cc = 0;
    size_t		respLen;
    rtrUInt32   octet_1, octet_2, octet_3, triple;
    char		*resp = 0;
	static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
									'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
									'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
									'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
									'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
									'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
									'w', 'x', 'y', 'z', '0', '1', '2', '3',
									'4', '5', '6', '7', '8', '9', '+', '/'};

    respLen = BASE64_LENGTH(len);
    resp = _rsslMalloc(respLen+1);
    if (!resp)
        return 0;

	_DEBUG_TRACE_WS_CONN("Encoding %u byte buffer into %u bytes .?\n", len, (BASE64_LENGTH(len)))

    for (i= 0; i < len; )
    {
        octet_1 = i < len ? hash[i++] : 0;
        octet_2 = i < len ? hash[i++] : 0;
        octet_3 = i < len ? hash[i++] : 0;
 
        triple = (octet_1 << 0x10) + (octet_2 << 0x08) + octet_3;
 
        *(resp + cc++) = encoding_table[(triple >> 3 * 6) & 0x3F];
        *(resp + cc++) = encoding_table[(triple >> 2 * 6) & 0x3F];
        *(resp + cc++) = encoding_table[(triple >> 1 * 6) & 0x3F];
        *(resp + cc++) = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    cc -= (3 - (len % 3));

    for (i = 0; i < (3-(len % 3)); i++)
        *(resp + cc++) = '=';

    if (cc <= respLen)
        *(resp + cc) = '\0';

    return resp;
}

static char *_getWebSocketAcceptKey(char * key, RsslInt32 len)
{
	char keyBuf[512];
	char *acceptKey = 0;
	unsigned char * hashRet = 0;
	sha1nfo	shInfo;

	memset(keyBuf, 0, 512);
	memcpy(keyBuf, key, len);
	memcpy(keyBuf + len, rws_GUID.data, rws_GUID.length);

	sha1_init(&shInfo);
	sha1_write(&shInfo, (const char*)&keyBuf[0], (len + rws_GUID.length));
	hashRet = sha1_result(&shInfo);

	acceptKey = (char *)_base64Encode(hashRet, SHA_DIGEST_LENGTH);

	_DEBUG_TRACE_WS_CONN("Encoding %u byte hash OR SHA_DLEN 20 into %u bytes Accept Key '%s'\n", 
						strlen(hashRet), (BASE64_LENGTH(len)), acceptKey)
	return acceptKey;
}

static size_t _getWebSocketNonce(unsigned char * nonce, size_t length)
{
    rtrInt32    i = 0;
	rtrUInt32	step = 4;
    rtrUInt64   randVal;

    if (nonce == NULL || length <= 0)
        return 0;
	
	step = sizeof(randVal);

    i = 0;
    while (i < length)
	{
        randVal = randull();
        memcpy (nonce + i, &randVal, step);
        i += step;
    }

    return length;
}

#define _WS_NONCE_LEN   16
static char * _rwsGenerateKey(void)
{
	size_t			nonceLen = _WS_NONCE_LEN;
	unsigned char	nonce[_WS_NONCE_LEN];
	char	*key = 0;
	
	if ((_getWebSocketNonce(&nonce[0], nonceLen)))
			key = (char *)_base64Encode((unsigned char *)&nonce[0], nonceLen);

	return key;
}

/*
 * For matching field-values which need to be case-sensitive matches
 */
static RsslInt32 _rwsMatchMemBuffer(char *buffer, RsslUInt32 bufferLen, char *match, RsslUInt32 matchLen)
{
	if(buffer == 0 || match == 0 || bufferLen != matchLen)
		return(0);

	return ( ((memcmp(buffer, match, matchLen) == 0) ? 1 : 0) );
}

/*
 * For matching field-values which are case-insensitive matches
 */
static RsslInt32 _rwsMatchBuffer(char *buffer, RsslInt32 bufferLen, char *match, RsslInt32 matchLen)
{
	char *pB = 0;
	char *pM = 0;
	RsslInt32 i = 0;

	if(buffer == 0 || match == 0 || bufferLen != matchLen)
		return(0);

	pB = buffer;
	pM = match;

	for( ;(i < matchLen && TOLOWER((unsigned char)*(pB+i))==TOLOWER((unsigned char)*(pM+i))) ; i++ );

	return( (i == matchLen ? 1 : 0) );
}

/*
 * For matching two RsslBuffer types which are case-insensitive matches
 */
static RsslInt32 _rwsMatchField(RsslBuffer *field, RsslBuffer *match)
{
	char *pF = 0;
	char *pM = 0;
	RsslUInt32 i = 0;

	if(field == 0 || field->length != match->length)
		return(0);

	pF = field->data;
	pM = match->data;

	for(i=0 ;(i < (RsslUInt32)match->length && TOLOWER((unsigned char)*(pF+i))==TOLOWER((unsigned char)*(pM+i))) ; i++ );

	return( (i == (RsslUInt32)match->length ? 1 : 0) );
}

/* Takes a protocol string name a looks through the list of known protocols
 * for a valid match.  If the 'deprecated argument is TRUE, then the list of old
 * protocol strings will be assumed to be invalid and ignored.
 * If a match is found, the matching enum value is returned
 */
static rwsSubProtocol_t _isValidProtocolName(char *name, RsslBool deprecate)
{
	int i = 0;
	/* This check is used for the simple case to simply validate a
	 * protocol string is valid input at initialization time
	 */

	for (i=0; i < RWS_SP_MAX; i++)
	{
		if (strncmp(name, rwsSubProtocols[i].protocolName.data, 
						  rwsSubProtocols[i].protocolName.length) == 0)
		{
			return rwsSubProtocols[i].protocol;
		}

		else if (!deprecate &&
				(strncmp(name,  rwsSubProtocols[i].oldProtocolName.data, 
								rwsSubProtocols[i].oldProtocolName.length) == 0))
		{
			return rwsSubProtocols[i].protocol;
		}
	}

	return RWS_SP_NONE;
}


/* Accepts arguments for the WebSocket Session and the RsslBuffer which
 * represents an http header fields' field-value
 *    (i.e. header field: 'Sec-WebSocket-Protocol' and field-value:  'rssl.rwf'
 * The RsslBool argument with affect whether or not deprecated values are valid
 * when checking the validity of the protocol received
 */
rwsSubProtocol_t rwsValidateSubProtocolResponse(rwsSession_t * wsSess, RsslBuffer *pValue, RsslBool deprecate, RsslError *error)
{
	char *data = pValue->data;
	RsslInt32 oldProtocolString = 0;
	RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;
	rwsSubProtocol_t subProtocol = RWS_SP_NONE;

	data = pValue->data;
	start = pos = 0, endOfLine = pValue->length;

	_DBG_HTTP_TOKEN(data, pos, endOfLine)
	/* Consume any non-token characters */
	RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
	_DBG_HTTP_TOKEN(data, pos, endOfLine)
	start = pos;
	/* Consume all consecutive token characters which define a field-value */
	RWS_GET_FIELDVALUE(data, pos, endOfLine);
	_DBG_HTTP_TOKEN(data, start, pos)
	if (pos > start)
	{
		wsSess->protocolName = (char*)_rsslMalloc((pos-start) + 1);
		if (wsSess->protocolName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
									"<%s:%d> Failed to allocate memory for protocolName.", 
									__FUNCTION__,__LINE__);
			return (RWS_SP_NONE);
		}
		else
		{
			memcpy(wsSess->protocolName, &(pValue->data[start]), (pos-start)+1);
			wsSess->protocolName[(pos-start)] = '\0';
			/* In theory, there could be a field-value response with multiple values.
			 * If this is the case, this logic is validating the first one received and if
			 * it is a valid protocol this client understands, all is well. In the case it
			 * is not valid, the no need to check the remaining since it is counter the 
			 * RFC specs to list more than one protocol within the response. This would only 
			 * become an issue if this was a response from a generic WS server which is not 
			 * supported at this time.
			 * Also, if the protocol received is invalid, do not free the protocolName since
			 * it should get freed when the channel is closed from this error */
			if ((subProtocol = _isValidProtocolName(wsSess->protocolName, deprecate)) == RWS_SP_NONE)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Unknown protocol received: '%s'", 
							__FUNCTION__,__LINE__, wsSess->protocolName);
			}
		}
	}
	return subProtocol;
}

/* Accepts arguments for the WebSocket Session,
 * The servers list of supported protocols listed by priority, left to right,
 * The RsslBuffer which represents an http header fields' field-value
 *    (i.e. header field: 'Sec-WebSocket-Protocol' and field-value:  'rssl.rwf, rssl.json'
 * The RsslBool argument with affect whether or not deprecated values are valid
 * when checking the validity of the received protocol(s)
 */
rwsSubProtocol_t rwsValidateSubProtocolRequest(rwsSession_t * wsSess, const char* srvrList, RsslBuffer *pValue, RsslBool deprecate, RsslError *error)
{
	RsslBuffer	*pSubP, *pOldSubP;
	char		*spName = 0;
	RsslInt32	 spNameLen = 0;
	RsslInt32	i, oldProtocolString = 0;
	rwsSubProtocol_t subProtocol = RWS_SP_NONE;
	char	*pProtName = 0, *pProtList = 0, *pStr = 0, *savEnd = 0;
	const char *delim = ", \t";

	if (srvrList)
		pStr = (char*)strdup(srvrList);
	else
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
			"<%s:%d>: Server sub-protocol list not defined.\n", __FUNCTION__, __LINE__);
		return 0;
	}

	/* If the client sent a list of sub-protocols, the most preferred one
	 * per the servers request should be chosen.
	 * So, starting with the most preferred server sub-protocol,
	 * iterate through the requested list 
	 */
	for (pProtName = strtok_r(pStr, delim, &savEnd); 
		 pProtName != NULL && subProtocol == RWS_SP_NONE;
		 pProtName = strtok_r(NULL, delim, &savEnd))
	{
		char *tpStr = 0;
		if (tpStr = strstr(pValue->data, pProtName))
		{
			for (i=0; i < RWS_SP_MAX; i++)
			{
				pSubP = &(rwsSubProtocols[i].protocolName); 
				pOldSubP = &(rwsSubProtocols[i].oldProtocolName); 

				if (strncmp(pProtName, pOldSubP->data, pOldSubP->length) == 0)
				{
					subProtocol = rwsSubProtocols[i].protocol;
					spName = pOldSubP->data;
					spNameLen = pOldSubP->length;
					break;
				} 
				else if (strncmp(pProtName,  pSubP->data, pSubP->length ) == 0)
				{
					subProtocol = rwsSubProtocols[i].protocol;
					spName = pSubP->data;
					spNameLen = pSubP->length;
					break;
				}
			}
		}
	}

	if (subProtocol != RWS_SP_NONE && spName != 0)
	{
		wsSess->protocolName = (char*)_rsslMalloc(spNameLen + 1);
		if (wsSess->protocolName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
									"<%s:%d> Failed to allocate memory for protocolName.", 
									__FUNCTION__,__LINE__);
		}
		else
		{
			memcpy(wsSess->protocolName, spName, spNameLen); 
			wsSess->protocolName[spNameLen] = '\0';
		}
	}
	if (pStr)
		_rsslFree(pStr);

	return subProtocol;
}


/* Parse requested list of sub-protocols, validate each one, and create a list of 
 * preferred protocols */
char * rwsSetSubProtocols(const char *protocols, RsslBool deprecate, RsslError *error)
{
	char	*pProtName = 0, *pProtList = 0, *pStr = 0;
	char	*savEnd = 0;
	const char *delim = ", \t";
	size_t	pLen = strlen(protocols);
	rwsSubProtocol_t prot = 0;

	if (protocols)
		pStr = (char*)strdup(protocols);
	else
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>: Protocol not defined.\n", __FUNCTION__, __LINE__);
		return 0;
	}

	pProtList = (char*)_rsslMalloc((sizeof(char)*pLen) + 1);
	if (pProtList == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Failed to allocate memory for protocolList.",
			__FUNCTION__, __LINE__);
	}

	for (pProtName = strtok_r(pStr, delim, &savEnd); 
		 pProtName != NULL;pProtName = strtok_r(NULL, delim, &savEnd))
	{
		if ((prot = _isValidProtocolName(pProtName, deprecate)) == RWS_SP_NONE)
		{
			_rsslFree(pStr);
			pStr = 0;
			_rsslFree(pProtList);
			pProtList = 0;
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d>: Invalid protocol found in protocol list. protocol (%d)\n",
					__FUNCTION__, __LINE__, prot);
			return 0;
		}
	}

	memcpy(pProtList, protocols, pLen); 
	pProtList[pLen] = '\0';

	if (pStr)
	{
		_rsslFree(pStr);
		pStr = 0;
	}

	return pProtList;
}

rtr_msgb_t *rwsDataBuffer(RsslSocketChannel *rsslSocketChannel, size_t size, RsslError *error)
{
	rtr_msgb_t			*retBuf = 0;
	size_t				neededSize = size;

	if (IPC_NULL_PTR(rsslSocketChannel, "", "rsslSocketChannel", error))
		return 0;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	retBuf = rwsGetPoolBuffer(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
	if (retBuf == 0)
	{
		if (ipcFlushSession(rsslSocketChannel, error) >= 0)
			retBuf = rwsGetPoolBuffer(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
	}
	
	if (retBuf != 0)
	{
		_DEBUG_TRACE_BUFFER("     After mx -= Buffer:%p buf %p prot %d ln %u mxln %u\n",
			(retBuf ? retBuf : 0),
			(retBuf ? retBuf->buffer : 0),
			(retBuf ? retBuf->protocol : 0),
			(retBuf ? retBuf->length : 0),
			(retBuf ? retBuf->maxLength : 0))
	}
	else
	{
		error->sysError = 0;
		error->rsslErrorId = RSSL_RET_BUFFER_NO_BUFFERS;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1009 rwsDataBuffer() failed, out of output buffers. The output buffer may need to be flushed.\n",
			__FILE__, __LINE__);
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retBuf);
}

rtr_msgb_t *rwsGetPoolBuffer(rtr_bufferpool_t *bufpool, size_t size)
{
	rtr_msgb_t			*retBuf = 0;
	RsslInt32			wsHdrLen = RWS_MAX_HEADER_SIZE; /* Length for WS header */

	if ((retBuf = rtr_dfltcAllocMsg(bufpool, (size + wsHdrLen))) != 0)
	{
		/* store the number of bytes reserved for the WS header  which will only be
		 * initialized if connection type is _WSOCKET */
		retBuf->protocolHdr = wsHdrLen;
		retBuf->buffer += wsHdrLen;
	}

	_DEBUG_TRACE_BUFFER("Returning WSocket Default Buffer:%p buf %p sz %u prot %d ln %u mxln %u\n", 
															(retBuf ? retBuf:0), 
															(retBuf ? retBuf->buffer:0),
															(size + wsHdrLen),
															(retBuf ? retBuf->protocolHdr:0),
															(retBuf ? retBuf->length:0),
															(retBuf ? retBuf->maxLength:0))

	return(retBuf);
}

rtr_msgb_t *rwsGetSimpleBuffer(size_t size)
{
	rtr_msgb_t			*retBuf = 0;
	RsslInt32			wsHdrLen = RWS_MAX_HEADER_SIZE; /* Length for WS header */

	if ((retBuf = ipcAllocGblMsg(size + wsHdrLen)) != 0)
	{
		/* store the number of bytes reserved for the WS header  which will only be
		 * initialized if connection type is _WSOCKET */
		retBuf->protocolHdr = wsHdrLen;
		retBuf->buffer += wsHdrLen;
	}

	_DEBUG_TRACE_BUFFER("Returning WSocket Simple Buffer:%p buf %p sz %u prot %d ln %u mxln %u\n", 
															(retBuf ? retBuf:0), 
															(retBuf ? retBuf->buffer:0),
															(size + wsHdrLen),
															(retBuf ? retBuf->protocolHdr:0),
															(retBuf ? retBuf->length:0),
															(retBuf ? retBuf->maxLength:0))

	return(retBuf);
}

// check to see if there is room for, or already is, 
// "length" bytes in input buffer after cursor
RsslInt32 checkInputBufferSpace(RsslSocketChannel *rsslSocketChannel, size_t length)
{
	size_t remaingSize = rsslSocketChannel->inputBuffer->maxLength - rsslSocketChannel->inputBuffer->length;

	if (remaingSize >= length + 4)
	{
		if (remaingSize >= rsslSocketChannel->readSize)
			return rsslSocketChannel->readSize;
		else
			return (RsslInt32)remaingSize;
	}

	return 0;
}

/* This function returns the number of free guaranteed buffers 
 * available to a user for writing. It is not very efficient.
 * Was done as a quick solution.
 */
RsslInt32 rwsIntTotalUsedOutputBuffers(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
	RsslInt32 		num = 0, largeMsgBufCnt = 0;
	rtr_msgb_t		*bufPtr = 0;

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RIPC_ERROR_TEXT,"<%s:%d> rwsIntTotalUsedOutputBuffers() failed due to session shutdown.", __FILE__,__LINE__);
		return(-1);
	}
	
	bufPtr = rsslSocketChannel->rwsLargeMsgBufferList;
	while (bufPtr != 0)
	{
		largeMsgBufCnt++;
		bufPtr = (rtr_msgb_t*)bufPtr->internal;
	}
	
	/* Return the number of guaranteed buffers used + pool buffers used */
	num = rsslSocketChannel->guarBufPool->numRegBufsUsed + rsslSocketChannel->guarBufPool->numPoolBufs + largeMsgBufCnt;
		
	return(num);
}

rtr_msgb_t *checkSizeAndRealloc(rtr_msgb_t* bufferObj, size_t newLength, size_t maxLength, RsslError *error)
{
	rtr_msgb_t* tempBufferObj;
	if (bufferObj->maxLength < newLength)
	{
		if (maxLength == 0)
		{
			size_t allocatedSize = (bufferObj->maxLength * 2) > newLength ? (bufferObj->maxLength * 2) : newLength;
			tempBufferObj = ipcAllocGblMsg(allocatedSize);
		}
		else if (bufferObj->maxLength * 2 <= maxLength)
			tempBufferObj = ipcAllocGblMsg(bufferObj->maxLength * 2);
		else if (bufferObj->maxLength < maxLength)
			tempBufferObj = ipcAllocGblMsg(maxLength);
		else
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Unsupported overall length for fragmented message",
					__FILE__,__LINE__);
			return(0);
		}

		if (tempBufferObj == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Could not allocate memory for session's reassembly/decompress buffer",
					__FILE__,__LINE__);
			return(0);
		}
	
			
		memcpy(tempBufferObj->buffer, bufferObj->buffer, bufferObj->length);
		tempBufferObj->length = bufferObj->length;
		rtr_smplcFreeMsg(bufferObj);
		bufferObj = tempBufferObj;
	}
	
	return bufferObj;
}

rtr_msgb_t *doubleSizeAndRealloc(rtr_msgb_t* bufferObj, size_t readLength, size_t maxLength, RsslError *error)
{
	rtr_msgb_t* tempBufferObj;
	if (maxLength == 0)
		tempBufferObj = ipcAllocGblMsg(bufferObj->maxLength * 2);
	else if (bufferObj->maxLength * 2 <= maxLength)
		tempBufferObj = ipcAllocGblMsg(bufferObj->maxLength * 2);
	else if (bufferObj->maxLength < maxLength)
		tempBufferObj = ipcAllocGblMsg(maxLength);
	else
		return bufferObj;

	if (tempBufferObj == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Could not allocate memory for session's reassembly/decompress buffer",
			__FILE__, __LINE__);
		return(0);
	}
		
	memcpy(tempBufferObj->buffer, bufferObj->buffer, readLength);
	tempBufferObj->length = bufferObj->length;
	rtr_smplcFreeMsg(bufferObj);
	bufferObj = tempBufferObj;
	
	return bufferObj;
}

void rwsReleaseLargeBuffer(RsslSocketChannel *rsslSocketChannel, rtr_msgb_t *msgb)
{
	rtr_msgb_t *bufPtr;
	rtr_msgb_t *prevBufPtr = 0;

	_DEBUG_TRACE_BUFFER("Internal LARGE Buffer released buffer=0x%x, \n",msgb->buffer)

	bufPtr = rsslSocketChannel->rwsLargeMsgBufferList;
	while( bufPtr )
	{
		if (bufPtr == msgb)
		{
			rsslSocketChannel->rwsLargeMsgBufferList = (rtr_msgb_t*)bufPtr->internal;
			if (prevBufPtr)
				prevBufPtr->internal = bufPtr->internal;
			else
				rsslSocketChannel->rwsLargeMsgBufferList = bufPtr->internal;  // Was first item on list
			if (bufPtr->protocolHdr)
				bufPtr->buffer -= bufPtr->protocolHdr;

			_rsslFree((void*)bufPtr->buffer);
			_rsslFree((void*)bufPtr);
			break;
		}
		prevBufPtr = bufPtr;
		bufPtr = (rtr_msgb_t*)bufPtr->internal;
	}
}


RsslInt32 rwsReadHttpHeader(char *data, RsslInt32 datalen, RsslInt32 startOffset, rwsSession_t *wsSess, rwsHttpHdr_t *httpHdr, RsslError *error)
{
    RsslInt32 endOfLine = startOffset, eoflen, lineLength=0, lineStart=0, retVal = 0;
	RsslBuffer	*pFN = 0, *pFV = 0;
	char		*pHD = 0;

    if (datalen == 0) return 0;

	_DEBUG_TRACE_WS_CONN("buffer 0x%p len %d start pos %d\n", data, datalen, startOffset)

    while(startOffset > 0 && iseof(data,startOffset -1,datalen) == 0)
        startOffset--;
    
    endOfLine = lineStart = startOffset;

    while(endOfLine < datalen)
    {
        eoflen = iseof(data,endOfLine,datalen);
        if (eoflen == 0)
        {
            lineLength++;
            endOfLine++;
            continue;
        }

        if (lineLength == 0) /* empty line designates end of http headers */
        {
			if( wsSess->headerLineNum == 0 )
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Zero length HTTP header", __FILE__, __LINE__);
				return(-400); /* Bad Request */
			}
			wsSess->headerLineNum = 0;
            return endOfLine+eoflen;
        }
		else
		{
			char *line = 0;
			RsslInt32  start = endOfLine-lineLength;
			
			if (_addNewHeaderLine(httpHdr) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to allocate memory to add HTTP a header", __FILE__, __LINE__);
				return (-1);
			}

			line = (char *) _rsslMalloc(sizeof(char)*(endOfLine - start)+1);
			if (line == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to allocate memory to parse HTTP a header", __FILE__, __LINE__);
				return (-1);
			}

			memcpy(line, &(data[start]), (endOfLine - start) +1);
			line[(endOfLine - start)] = '\0';

			httpHdr->lines[httpHdr->total-1].data = line;
			
			_AUTOTEST_DISPLAY("Line %d : %s", httpHdr->total, line)
		}

		pHD = httpHdr->lines[httpHdr->total-1].data;
		pFN = &(httpHdr->lines[httpHdr->total-1].field);
		pFV = &(httpHdr->lines[httpHdr->total-1].value);

        wsSess->headerLineNum++;
	
		_DBG_HTTP_PRINT_LINE(wsSess->headerLineNum, data, (endOfLine - lineLength), endOfLine)

		if(wsSess->headerLineNum == 1 )
		{
			RsslInt32 lineLengthSave = lineLength;
			RsslInt32  pos = endOfLine-lineLength;
			RsslInt32  start;

			lineLength = URLdecode(&data[pos], lineLength, &data[pos]);

			pFN->data = pHD;
			start = pos;
			/* Get the Field Name */
			RWS_GET_STATUS_REQUEST_FIELD(data, pos, endOfLine);
			pFN->length = pos - start;
		
			/* The first Line of an HTTP header should have a 
			 * space after the first field name */
			/* per RFC7230, Sec 3.1.1 and 3.2.1 only one space between the start line field
			 * and the first field-value argument */
			if(((pos+1) < endOfLine) && data[pos] == ' ' && data[pos+1] != ' ')
			{
				int space = pos;
				
				pos++;
				RWS_CONSUME_WHITESPACE(data, pos, endOfLine);
				if (pos < endOfLine)
				{
					/* mark the START of the field-value */
					pFV->data = pHD+(pFN->length + (pos - space));
					start = pos;
					RWS_GET_ALL_FIELDVALUES(data, pos, endOfLine);
					/* mark the END of the field-value */
					pFV->length = pos - start;
				}
			}
			else
			{
				/* bad header 1st line format */
				_DEBUG_TRACE_PARSE_HTTP("Bad HTTP status/request line %s ", pHD)
				_DBG_HTTP_TOKEN(data, pos, endOfLine)

				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
						"<%s:%d> Bad HTTP status/request line", __FUNCTION__, __LINE__);
				return(-400); /* Bad Request */
			}
			lineLength = lineLengthSave;
		} /* End of if line 1*/
		else 
		/* Interested in additional lines of http header content for _WSOCKETs */
		{
			RsslInt32 pos = endOfLine - lineLength;
			RsslInt32 start;

			pFN->data = pHD;
			start = pos;
			/* Get the Field Name */
			RWS_GET_FIELDNAME(data, pos, endOfLine);
			pFN->length = pos - start;

			/* Every field-name in the HTTP header should be followed by a ':' */
			if(data[pos] == ':')
			{
				int colon = pos;

				pos++;

				RWS_MOVE_TO_FIELDVALUE(data, pos, endOfLine);
				if (pos < endOfLine)
				{
					/* mark the START of the field-value */
					pFV->data = pHD+(pFN->length + (pos - colon));
					start = pos;
					RWS_GET_ALL_FIELDVALUES(data, pos, endOfLine);
					/* mark the END of the field-value */
					pFV->length = pos - start;
				}
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
						"<%s:%d> Improper formatted Field. Missing ':' ", __FUNCTION__, __LINE__);
				return(-400); /* Bad Request */
			}
		}

		endOfLine += eoflen;
		lineLength = 0;
	}
    /* here we are if not yet complete */
    return 0;
}

/* The server side call for reading the initial opening handshake request sent from a new
 * client requesting a WebSocket connection  */
RsslInt32 rwsReadOpeningHandshake(char *data, RsslInt32 datalen, RsslInt32 startOffset, RsslSocketChannel * rsslSocketChannel, RsslError *error)
{
    RsslInt32 lineLength=0, lineStart=0;
	RsslInt32 retVal = 0;
	rwsHttpHdr_t *openHdrs = 0;
	headerLine_t *hdrLine = 0;
	rwsSession_t *wsSess = 0;
	rwsServer_t *wsServer = 0;

    if (datalen == 0) return 0;

	_DEBUG_TRACE_WS_CONN("fd "SOCKET_PRINT_TYPE" \n", rsslSocketChannel->stream)


	if (rsslSocketChannel->rwsSession == 0)
	{
		rsslSocketChannel->rwsSession = (void *)rwsNewSession();
		if (rsslSocketChannel->rwsSession == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Failed to allocate memory for rwsSession struct.", __FUNCTION__,__LINE__);
			return(-1);
		}
		_DEBUG_TRACE_INIT("rwsSession 0x%p *0x%p\n", 
								rsslSocketChannel->rwsSession, 
								*((rwsSession_t *)rsslSocketChannel->rwsSession))
	}
	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;
	if (!wsSess->server)
		wsSess->server = (rwsServer_t *)rsslSocketChannel->server->rwsServer;
			
	openHdrs = &(wsSess->hsReceived);

	retVal = rwsReadHttpHeader(data, datalen, startOffset, wsSess, openHdrs, error);
	if (retVal < 0) /* Fails to parse the received headers */
	{
		return (retVal);
	}
	else if (retVal > 0)
	{	
		RsslInt32 lnNum = 1, pos;
		RsslInt32 fldLen, valLen;
		RsslBuffer *pField, *pValue;
		
		hdrLine = openHdrs->lines;

		// Check the first line is a GET request and the field-value field
		// has a URL for /WebSocket
		// e.g. value = '/WebSocket HTTP/1.1' fv_WebSocketURI = '/WebSocket'
		if (hdrLine && 
			(hdrLine[0].field.length == rwsHdr_GET.length) &&
			(memcmp(hdrLine[0].field.data, rwsHdr_GET.data, rwsHdr_GET.length)==0) &&
			(hdrLine[0].value.length >= fv_WebSocketURI.length) && 
			(memcmp(hdrLine[0].value.data, fv_WebSocketURI.data, fv_WebSocketURI.length)==0))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			_DEBUG_TRACE_PARSE_HTTP("Received GET request '%s' ", hdrLine[0].data)
			wsSess->recvGetReq = 1;
			rsslSocketChannel->connType = RSSL_CONN_TYPE_WEBSOCKET;
		}
		else
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Invalid GET request received ", __FUNCTION__,__LINE__);
			return(-1);
		}

		for(lnNum = 1; lnNum < openHdrs->total ; lnNum++)
		{
			pos = 0;
			pField = &(hdrLine[lnNum].field);
			fldLen = hdrLine[lnNum].field.length;

			pValue = &(hdrLine[lnNum].value);
			valLen = hdrLine[lnNum].value.length;

			_DEBUG_TRACE_PARSE_HTTP("Checking http header line %d : %s\n", 
												lnNum, hdrLine[lnNum].data)
			_DBG_HTTP_FIELD(pField->data, pField->length)
			_DBG_HTTP_VALUE(pValue->data, pValue->length)
			/* Process the HTTP Header fields related to the Opening handshake received*/

			/* Checking for HTTP Header field 
			 * Upgrade: */
			if (_rwsMatchField(pField, &rwsField_UPGRADE)) {
				//_DBG_HTTP_PROCENTRY(pField->data, pField->length)
				if (_rwsMatchField(pValue, &fv_Websocket))
				{
					//_DBG_HTTP_PROCENTRY(pValue->data, pValue->length)
					wsSess->upgrade = 1;
				}
			}

			/* Checking for HTTP Header field 
			 * Connection: */
			else if (_rwsMatchField(pField, &rwsField_CONNECTION))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				while (pos < endOfLine)
				{
					RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
					start = pos;
					RWS_GET_FIELDVALUE(data, pos, endOfLine);
					_DBG_HTTP_TOKEN(data, start, pos);

					if(_rwsMatchBuffer(data+start, (pos - start), fv_Upgrade.data, fv_Upgrade.length))
						wsSess->connUpgrade = 1;
				}
			}

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Key: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_KEY))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_KEY_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_KEY_VALUE(data, pos, endOfLine);
				_DBG_HTTP_FIELDVALUE(data, start, pos);
				
				if (wsSess->keyRecv.data == 0)
				{
					wsSess->keyRecv.data = (char*)_rsslMalloc((pos-start) + 1);
					if (wsSess->keyRecv.data == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Failed to allocate memory for Sec-Websocket-Key value.", 
								__FUNCTION__,__LINE__);
						return(-1);
					}
					wsSess->keyRecv.length = pos-start;
					memcpy(wsSess->keyRecv.data, &(pValue->data[start]), wsSess->keyRecv.length);
					wsSess->keyRecv.data[wsSess->keyRecv.length] = '\0';
					_DEBUG_TRACE_WS_CONN("Key ->Recv '%s' \n", wsSess->keyRecv.data)
				}
				else
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Invalid HTTP header, duplicate fields received, Sec-Websocket-Key", 
							__FUNCTION__,__LINE__);
					return(-1);
				}
			}

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Version: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_VERSION))
			{
				char *data = pValue->data;
				RsslInt32 WS_version = 0;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_FIELDVALUE(data, pos, endOfLine);
				_DBG_HTTP_TOKEN(data, start, pos);
				if ((wsSess->versionRecv = getIntValue( &start, pos, data)) < 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Invalid WebSocket Version ", __FUNCTION__,__LINE__);
					return(-1);
				}
			} 

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Protocol: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_PROTOCOL)) 
			{
				/* If the handshake sent an invalid sub protocol, 0 will be returned
				 * and will be rejected as an invalid protocol. If no protocol was sent
				 * the default protocol will be used */
				wsSess->protocol = rwsValidateSubProtocolRequest(wsSess, wsSess->server->protocolList, pValue, 0, error);
				if (wsSess->protocol == RWS_SP_NONE)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Unable to read sub-protocol Request ", __FUNCTION__,__LINE__);
					return (-1);
				}
				_DEBUG_TRACE_PARSE_HTTP("SubProtocol set '%s' (%d)\n", 
										wsSess->protocolName, wsSess->protocol);
			}

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Extensions: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_EXTENSIONS))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;
				RsslInt32 deflateEnd = 0;

				while (pos<endOfLine)
				{
					_DBG_HTTP_TOKEN(data, pos, endOfLine)
					RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
					start = pos;
					RWS_GET_FIELDVALUE(data, pos, endOfLine);
					_DBG_HTTP_TOKEN(data, start, pos);

					/* Check for permessage-deflate */
					if(_rwsMatchBuffer(data+start, (pos - start), 
										fv_PerMsgDeflate.data, fv_PerMsgDeflate.length))
					{
						wsSess->deflate = 1;
						deflateEnd = pos - start;
					}

					if(_rwsMatchBuffer(data+start, (pos - start), 
										fv_ServerNoContext.data, fv_ServerNoContext.length))
						wsSess->comp.flags |= RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT;

					if(_rwsMatchBuffer(data+start, (pos - start), 
										fv_ClientNoContext.data, fv_ClientNoContext.length))
						wsSess->comp.flags |= RWS_COMPF_DEFLATE_NO_INBOUND_CONTEXT;
				}

				if ((!wsSess->deflate) || (wsSess->deflate && 
					!((wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)||
					(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_INBOUND_CONTEXT))))
				{
					/* a field-value entry with a ';' following permessage-deflate ;
					 * and no context definitions, should generate an error */
					if (wsSess->deflate)
					{
						int st = deflateEnd;
						RWS_MOVE_TO_CHAR(data, st, endOfLine, ';');
						if (data[st] != ';')
							continue;
					}
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Invalid Sec-WebSocket-Extensions parameter list format", 
								__FUNCTION__,__LINE__);
					return(-1);
				}
			}

			/* Checking for HTTP Header field 
			 * Host: */
			else if (_rwsMatchField(pField, &rwsField_HOST)) {
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_FIELDVALUE(data, pos, endOfLine);
				_DBG_HTTP_TOKEN(data, start, pos);

				if (!(wsSess->host = (char*)_rsslMalloc((endOfLine - start) + 1)))
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Failed to allocate memory for Host token value.", 
								__FUNCTION__,__LINE__);
					return(-1);
				}
				memcpy(wsSess->host, &(data[start]), (endOfLine - start));
				wsSess->host[(endOfLine - start)] = '\0';

				if (!(wsSess->hostname = (char*)_rsslMalloc((pos - start) + 1)))
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Failed to allocate memory for hostname token value.", 
								__FUNCTION__,__LINE__);
					return(-1);
				}
				memcpy(wsSess->hostname, &(data[start]), pos - start);
				wsSess->hostname[pos - start] = '\0';

				if (data[pos] == ':')
				{
					pos++;
					start = pos;
					RWS_GET_FIELDVALUE(data, pos, endOfLine);
					_DBG_HTTP_TOKEN(data, start, pos);

					if (!(wsSess->port = (char*)_rsslMalloc((pos - start) + 1)))
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Failed to allocate memory for port token value.", 
									__FUNCTION__,__LINE__);
						return(-1);
					}
					memcpy(wsSess->port, &(data[start]), pos - start);
					wsSess->port[pos - start] = '\0';
				}
			}

			/* Checking for HTTP Header field 
			 * Origin: */
			else if (_rwsMatchField(pField, &rwsField_ORIGIN))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_FIELDVALUE(data, pos, endOfLine);
				_DBG_HTTP_TOKEN(data, start, pos);

				if (!(wsSess->origin = (char*)_rsslMalloc((endOfLine - start) + 1)))
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Failed to allocate memory for Origin value.",
						__FUNCTION__, __LINE__);
					return(-1);
				}
				memcpy(wsSess->origin, &(data[start]), (endOfLine - start));
				wsSess->origin[(endOfLine - start)] = '\0';
			}

			/* Checking for HTTP Header field 
			 * Cookie: */
			else if ((_rwsMatchField(pField, &rwsField_COOKIE)) &&
					  (wsSess->server->cookies.authToken.length || 
					   wsSess->server->cookies.position.length || 
					   wsSess->server->cookies.applicationId.length))
			{
				RsslInt32 * cookieLenPtr = 0; 
				char ** cookieValuePtr = 0;
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				while (pos < endOfLine)
				{
					RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
					start = pos;
					RWS_GET_FIELDVALUE(data, pos, endOfLine);
					_DBG_HTTP_TOKEN(data, start, pos);
					if(_rwsMatchMemBuffer((data+start), pos - start, 
										 wsSess->server->cookies.authToken.data, 
										 wsSess->server->cookies.authToken.length))
					{
						_DEBUG_TRACE_PARSE_HTTP("Cookie authToken")
						/* Set the pointers to the found field-value to check below */
						cookieValuePtr = &(wsSess->cookies.authToken.data);
						cookieLenPtr = &(wsSess->cookies.authToken.length);
					}
					else if(_rwsMatchMemBuffer((data+start), pos - start, 
											  wsSess->server->cookies.applicationId.data, 
											  wsSess->server->cookies.applicationId.length))
					{
						/* Set the pointers to the found field-value to check below */
						cookieValuePtr = &(wsSess->cookies.applicationId.data);
						cookieLenPtr = &(wsSess->cookies.applicationId.length);
						_DEBUG_TRACE_PARSE_HTTP("Cookie applicationId")
					}
					else if(_rwsMatchMemBuffer((data+start), pos - start, 
											  wsSess->server->cookies.position.data, 
											  wsSess->server->cookies.position.length))
					{
						/* Set the pointers to the found field-value to check below */
						cookieValuePtr = &(wsSess->cookies.position.data);
						cookieLenPtr = &(wsSess->cookies.position.length);
						_DEBUG_TRACE_PARSE_HTTP("Cookie position")
					}

					if (cookieValuePtr)
					{
						RWS_MOVE_TO_CHAR(data, pos, endOfLine, '=');
						if (data[pos] == '=')
						{
							RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
							start = pos;
							RWS_GET_FIELDVALUE(data, pos, endOfLine);
							_DBG_HTTP_TOKEN(data, start, pos);
							
							if (pos > start)
							{
								if (!(*cookieValuePtr = (char*)_rsslMalloc(pos - start)))
								{
									_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
									snprintf(error->text, MAX_RSSL_ERROR_TEXT,
											"<%s:%d> Failed to allocate memory for authentication token.", 
											__FUNCTION__,__LINE__);
									return(-1);
								}
								memcpy(*cookieValuePtr, &(data[pos]), (pos - start));
								*cookieLenPtr = pos - start;
							}
						}
					}
					cookieValuePtr = 0;
					cookieLenPtr = 0;
				}
			}

			/* Checking for HTTP Header field 
			 * User-Agent: */
			else if (_rwsMatchField(pField, &rwsField_USER_AGENT)) 
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_FIELDVALUE(data, pos, endOfLine);
				_DBG_HTTP_TOKEN(data, start, pos);

				// _DBG_HTTP_PROCENTRY(pField->data, pField->length)
				/* Call function to handle this hdr field */
				if (!(wsSess->userAgent = (char*)_rsslMalloc((pos - start) + 1)))
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Failed to allocate memory for User-Agent string.", 
							__FUNCTION__,__LINE__);
					return(-1);
				}
				memcpy(wsSess->userAgent, &(data[start]), (pos - start));
				wsSess->userAgent[(pos - start)] = '\0';
			}
		}
	}

	return retVal;
}

RsslInt32 rwsReadResponseHandshake(char *data, RsslInt32 datalen, RsslInt32 startOffset, rwsSession_t * wsSess, RsslError *error)
{
	RsslInt32 lineLength=0, lineStart=0;
	RsslInt32 retVal = 0;
	rwsHttpHdr_t *respHdrs = 0;
	headerLine_t *hdrLine = 0;

    if (datalen == 0) return 0;

	respHdrs = &(wsSess->hsReceived);

	retVal = rwsReadHttpHeader(data, datalen, startOffset, wsSess, respHdrs, error);
	if (retVal < 0)
	{
		return (retVal);
	}
	else if (retVal > 0)
	{	
		RsslInt32 lnNum = 1, pos;
		RsslInt32 fldLen, valLen;
		RsslBuffer *pField, *pValue;
		
		hdrLine = respHdrs->lines;

		// Check the first line is a 'HTTP ' request and the field-value field
		// has a URL for /WebSocket
		// e.g. value = '/WebSocket HTTP/1.1' fv_WebSocketURI = '/WebSocket'
		if (hdrLine && (hdrLine[0].field.length == rwsHdr_HTTP.length) &&
			(memcmp(hdrLine[0].field.data, rwsHdr_HTTP.data, rwsHdr_HTTP.length)==0))
		{
			char *data = 0;
			RsslInt32 start = 0, pos = 0, endOfLine = 0;

			_DEBUG_TRACE_PARSE_HTTP("Received HTTP response header '%s' ", hdrLine[0].data)

			pValue = &(hdrLine[0].value);
			valLen = hdrLine[0].value.length;
			data = pValue->data;
			endOfLine = pValue->length;

			start = pos;
			RWS_GET_FIELDVALUE(data, pos, endOfLine);

			_DBG_HTTP_TOKEN(data, start, pos);

			if ((wsSess->statusCode = getIntValue( &start, pos, data)) != 101)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Invalid HTTP response, status code %d ", 
							__FUNCTION__,__LINE__, wsSess->statusCode);
				return((wsSess->statusCode >= 400 ? (-wsSess->statusCode) : -1));
			}

			_DEBUG_TRACE_PARSE_HTTP("Received HTTP status code '%s'", pValue->data)
		}
		else
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Invalid HTTP response",
				__FUNCTION__, __LINE__);
			return(-400);
		}

		for(lnNum = 1; lnNum < respHdrs->total ; lnNum++)
		{
			pos = 0;
			pField = &(hdrLine[lnNum].field);
			fldLen = hdrLine[lnNum].field.length;

			pValue = &(hdrLine[lnNum].value);
			valLen = hdrLine[lnNum].value.length;

			_DEBUG_TRACE_PARSE_HTTP("Checking http header line %d : %s\n", lnNum, hdrLine[lnNum].data)
			_DBG_HTTP_FIELD(pField->data, pField->length)
			_DBG_HTTP_VALUE(pValue->data, pValue->length)
			/* Processing HTTP Header fields related to the Handshake Response */

			/* Checking for HTTP Header field 
			 * Upgrade: */
			if (_rwsMatchField(pField, &rwsField_UPGRADE)) {
				// _DBG_HTTP_PROCENTRY(pField->data, pField->length)
				if (_rwsMatchField(pValue, &fv_Websocket))
				{
					// _DBG_HTTP_PROCENTRY(pValue->data, pValue->length)
					wsSess->upgrade = 1;
				}
			}

			/* Checking for HTTP Header field 
			 * Connection: */
			else if (_rwsMatchField(pField, &rwsField_CONNECTION))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				while (pos < endOfLine)
				{
					RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
					start = pos;
					RWS_GET_FIELDVALUE(data, pos, endOfLine);
					_DBG_HTTP_TOKEN(data, start, pos);

					if(_rwsMatchBuffer(data+start, (pos - start), fv_Upgrade.data, fv_Upgrade.length))
						wsSess->connUpgrade = 1;
				}
			}

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Accept: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_ACCEPT))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_KEY_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_KEY_VALUE(data, pos, endOfLine);
				_DBG_HTTP_FIELDVALUE(data, start, pos);
				
				if (wsSess->keyRecv.data == 0)
				{
					wsSess->keyRecv.data = (char*)_rsslMalloc((pos-start) + 1);
					if (wsSess->keyRecv.data == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Failed to allocate memory for Sec-Websocket-Key value.", 
							__FUNCTION__,__LINE__);
						return(-1);
					}
					wsSess->keyRecv.length = pos-start;
					memcpy(wsSess->keyRecv.data, &(pValue->data[start]), wsSess->keyRecv.length);
					wsSess->keyRecv.data[wsSess->keyRecv.length] = '\0';
					if (strncmp(wsSess->keyRecv.data, wsSess->keyAccept.data, 
													  wsSess->keyAccept.length) != 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Key received '%s' is not key expected '%s'", 
							__FUNCTION__,__LINE__, wsSess->keyRecv.data, wsSess->keyAccept.data);
						return(-400);
					}
					_DEBUG_TRACE_WS_CONN("Keys matched? ->Recv '%s' Expected '%s'\n", 
													wsSess->keyRecv.data, wsSess->keyAccept.data)
				}
				else
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Invalid HTTP header, duplicate fields received, Sec-Websocket-Key", 
						__FUNCTION__,__LINE__);
					return(-400);
				}
			}

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Version: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_VERSION))
			{
				char *data = pValue->data;
				RsslInt32 WS_version = 0;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;

				RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
				start = pos;
				RWS_GET_FIELDVALUE(data, pos, endOfLine);
				_DBG_HTTP_TOKEN(data, start, pos);
				if ((wsSess->versionRecv = getIntValue( &start, pos, data)) < 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Invalid WebSocket Version ", __FUNCTION__,__LINE__);
					return(-400);
				}
			} 

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Protocol: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_PROTOCOL)) 
			{
				/* If the handshake Response sent an invalid sub protocol, 0 will be returned
				 * and will be rejected as an invalid protocol. If no protocol was sent
				 * the default protocol will be used */
				wsSess->protocol = rwsValidateSubProtocolResponse(wsSess, pValue, 0, error);
				if (wsSess->protocol == RWS_SP_NONE)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error with protocol response ", __FUNCTION__,__LINE__);
					return(-400);
				}
			}

			/* Checking for HTTP Header field 
			 * Sec-WebSocket-Extensions: */
			else if (_rwsMatchField(pField, &rwsField_SEC_WEBSOCKET_EXTENSIONS))
			{
				char *data = pValue->data;
				RsslInt32 start = 0, pos = 0, endOfLine = pValue->length;
				RsslInt32 deflateEnd = 0;

				while (pos<endOfLine)
				{
					_DBG_HTTP_TOKEN(data, pos, endOfLine)
					RWS_MOVE_TO_TOKEN(data, pos, endOfLine);
					start = pos;
					RWS_GET_FIELDVALUE(data, pos, endOfLine);
					_DBG_HTTP_TOKEN(data, start, pos);

					if(_rwsMatchBuffer(data+start, (pos - start), 
										fv_PerMsgDeflate.data, fv_PerMsgDeflate.length))
					{
						wsSess->deflate = 1;
						deflateEnd = pos - start;
					}
					if(_rwsMatchBuffer(data+start, (pos - start), 
										fv_ServerNoContext.data, fv_ServerNoContext.length))
						wsSess->comp.flags |= RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT;

					if(_rwsMatchBuffer(data+start, (pos - start), 
										fv_ClientNoContext.data, fv_ClientNoContext.length))
						wsSess->comp.flags |= RWS_COMPF_DEFLATE_NO_INBOUND_CONTEXT;
				}

				if ((!wsSess->deflate) || (wsSess->deflate && 
					!((wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)||
					(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_INBOUND_CONTEXT))))
				{
					/* a field-value entry with a ';' following permessage-deflate ;
					 * and no context definitions, should generate an error */
					if (wsSess->deflate)
					{
						int st = deflateEnd;
						RWS_MOVE_TO_CHAR(data, st, endOfLine, ';');
						if (data[st] != ';')
							continue;
					}
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Invalid Sec-WebSocket-Extensions parameter list format", 
								__FUNCTION__,__LINE__);
					return(-400);
				}
			}
		}
	}

    return retVal;
}

ripcSessInit rwsSendOpeningHandshake(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inProgress, RsslError *error)
{
	ripcRWFlags		rwflags = RIPC_RW_WAITALL;
  	int cc = 0, i = 0;
	RsslInt32 hsLen = 0;
	char	hsBuffer[RWS_MAX_HTTP_HEADER_SIZE];
	rwsSession_t *wsSess = 0;
	RsslUInt32 keyLen = 0;
	char *newKey = 0;

	_DEBUG_TRACE_WS_CONN("fd "SOCKET_PRINT_TYPE" \n", rsslSocketChannel->stream)

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	RIPC_ASSERT(rsslSocketChannel->intState == RIPC_INT_ST_WS_SEND_OPENING_HANDSHAKE);

	//Create a Client RWS_Handle and populate the struct.
	//         The number of ws related info is large and growing
	if (rsslSocketChannel->rwsSession == 0)
	{
		rsslSocketChannel->rwsSession = (void *)rwsNewSession();
		if (rsslSocketChannel->rwsSession == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Failed to allocate memory for rwsSession struct.", __FUNCTION__,__LINE__);
			return(-1);
		}
		_DEBUG_TRACE_INIT("rwsSession 0x%p *0x%p\n", 
								rsslSocketChannel->rwsSession, 
								*((rwsSession_t *)rsslSocketChannel->rwsSession))
	}
	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	wsSess->isClient = RSSL_TRUE;
	if (rsslSocketChannel->compression == RSSL_COMP_ZLIB)
	{
		wsSess->comp.type = rsslSocketChannel->compression;
		wsSess->comp.zlibLevel = rsslSocketChannel->zlibCompLevel;
	}

	/*
	 * GET /WebSocket HTTP/1.1<CR><LF>
	 * Upgrade: websocket<CR><LF>
	 * Connection: Upgrade<CR><LF>
	 * Host: localhost:15000<CR><LF>
	 * Origin: http://localhost:15000<CR><LF>
	 * Sec-WebSocket-Key: yB0kpwchuxb9yF1Jbm3OTA==<CR><LF>
	 * Sec-WebSocket-Version: 13<CR><LF><LF>
	 * Sec-WebSocket-Protocol: tr_json2<CR><LF>
	 * User-Agent: TBD<CR><LF>
	 */
	hsLen = snprintf(hsBuffer, RWS_MAX_HTTP_HEADER_SIZE,		  "GET /WebSocket HTTP/1.1\r\n");
	hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "Upgrade: websocket\r\n");
	hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "Connection: keep-alive, Upgrade\r\n");

	if (rsslSocketChannel->hostName)
	{
		hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "Host: %s:%s\r\n",
					rsslSocketChannel->hostName, rsslSocketChannel->serverName);
	}
	/* If Supply Origin:*/
	if (0)
		hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "Origin: null\r\n");
	
	wsSess->keyNonce.data = _rwsGenerateKey();
	if (wsSess->keyNonce.data == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> Error allocating memory for client key token, Sec-Websocket-Key token value.", __FUNCTION__,__LINE__);
		return(-1);
	}
	wsSess->keyNonce.length = (RsslUInt32)strlen(wsSess->keyNonce.data);

	/* Once the Client nonce is generated for the 'Sec-WebSocket-Key' token field, construct 
	 * the key expected within a hanshake response */
	wsSess->keyAccept.data = (char *)_getWebSocketAcceptKey(wsSess->keyNonce.data, wsSess->keyNonce.length);
	if (wsSess->keyAccept.data == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> Error allocating memory expected client token in Sec-Websocket-Accept field.", __FUNCTION__,__LINE__);
		return(-1);
	}
	wsSess->keyAccept.length = (RsslUInt32)strlen(wsSess->keyAccept.data);

	hsLen += snprintf(hsBuffer + hsLen, RWS_MAX_HTTP_HEADER_SIZE,
								"Sec-WebSocket-Key: %s\r\n", wsSess->keyNonce.data);

	hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "Sec-WebSocket-Version: 13\r\n");
	hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "Sec-WebSocket-Protocol: ");

	
	// Parse list of protocols from user options
	if (wsSess->protocolList)
		hsLen += snprintf(hsBuffer + hsLen, RWS_MAX_HTTP_HEADER_SIZE, "%s\r\n", wsSess->protocolList);	

	if(wsSess->comp.type == RSSL_COMP_ZLIB)
		hsLen += sprintf(hsBuffer + hsLen, "%s%s%s", "Sec-WebSocket-Extensions: permessage-deflate", 
					";server_no_context_takeover", "\r\n");

	hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "User-Agent: Mozilla/5.0\r\n");

	hsLen += snprintf(hsBuffer +hsLen , RWS_MAX_HTTP_HEADER_SIZE, "\r\n");

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, hsBuffer, hsLen ,rwflags, error);		

	IPC_MUTEX_LOCK(rsslSocketChannel);

	_DEBUG_TRACE_WS_WRITE("handshake sent, fd "SOCKET_PRINT_TYPE" cc %d err %d\n", 
					rsslSocketChannel->stream, cc, errno)
	
	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 rwsClientSendHandshake() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	if (cc < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
			"<%s:%d> Error: 1002 rwsClientSendHandshake() Client WS handshake. System errno: (%d)\n",
							__FILE__, __LINE__, errno);
	
		return RSSL_RET_FAILURE;
	}

	rsslSocketChannel->intState = RIPC_INT_ST_WS_WAIT_HANDSHAKE_RESPONSE;

	return(RIPC_CONN_IN_PROGRESS);
}

ripcSessInit rwsWaitResponseHandshake(RsslSocketChannel * rsslSocketChannel, ripcSessInProg *inProgress, RsslError *error)
{
	char			buffer[RWS_MAX_HTTP_HEADER_SIZE];
	char			*ptBuff = 0;
	RsslInt32		cc = 0, httpHeaderLen = 0;
	RsslInt32		idOffset = 0; /* will always be 0 if not tunneling */
	RsslInt32		opCode = 0;
	RsslUInt16		msgLen = 0;
	RsslUInt32		versionNumber = 0;
	RsslUInt8 		flags = 0x0;
	RsslInt32		i = 0;
	ripcRWFlags		rwflags = RIPC_RW_NONE;
	rwsSession_t *wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	_DEBUG_TRACE_WS_CONN("fd "SOCKET_PRINT_TYPE" \n", rsslSocketChannel->stream)

	if (IPC_NULL_PTR(wsSess, "rwsWaitResponseHandshake", "wsSess", error))
		return(RIPC_CONN_ERROR);

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	cc = (*(rsslSocketChannel->transportFuncs->readTransport))(rsslSocketChannel->transportInfo,
			buffer, RWS_MAX_HTTP_HEADER_SIZE, rwflags, error);

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s():%d> Error: 1003 failed due to channel shutting down.\n",
			__FUNCTION__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	_DEBUG_TRACE_WS_READ("fd "SOCKET_PRINT_TYPE" cc %d err %d\n", rsslSocketChannel->stream, cc, ((cc <= 0) ? errno : 0))

	if (cc < 0)
	{
		/* Try to re-connect with an older header */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not read IPC Mount Ack.  Connection attempt has failed. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			return(RIPC_CONN_ERROR);
	}
	else if (cc == 0)
	{
		/* This will happen when there is no data at all to be read,
		and the read fails with error _IPC_WOULD_BLOCK or EINTR
		*/
		return (RIPC_CONN_IN_PROGRESS);
	}

	httpHeaderLen = rwsReadResponseHandshake(buffer, cc, 0, wsSess, error);

	/* TODO
	if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
	{
		(*(ripcDumpInFunc))(__FUNCTION__, buf, cc, rsslSocketChannel->stream);
	}
	*/
	_DEBUG_TRACE_WS_CONN("WS Session active httpHeaderLen %d sub-protocol %d\n", 
								httpHeaderLen, wsSess->protocol)

	if (httpHeaderLen < 0)
	{
		return(RIPC_CONN_ERROR);
	}
	else if (cc > httpHeaderLen)
	{
		/*
		* If we read more bytes than in the header, put them
		* into the read buffer. This can happen in quick applications
		* that send data as soon as the channel becomes active.
		*/
		char* pBuf = &buffer[0];
		RsslInt32 putback = cc - httpHeaderLen;
		MemCopyByInt(rsslSocketChannel->inputBuffer->buffer, (pBuf + httpHeaderLen), putback);
		rsslSocketChannel->inputBuffer->length += putback;
	}
	else if (httpHeaderLen == 0)
	{
		/* This will happen when there is no data at all to be read,
		and the read fails with error _IPC_WOULD_BLOCK or EINTR
		*/
		return (RIPC_CONN_IN_PROGRESS);
	}

	/* If subProtocol is RSSL_RWF then move to regular
	 * client connecting state */
	if (wsSess->protocol == RWS_SP_NONE)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Unsupported Websocket protocol type received(%d) ", 
				__FUNCTION__,__LINE__, wsSess->protocol);
		return(RIPC_CONN_ERROR);
	}
	if (wsSess->keyRecv.length == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> No Sec-Websocket-Accept: key received, key expected '%s'. ", 
			__FUNCTION__,__LINE__, wsSess->keyAccept.data);

		return(RIPC_CONN_ERROR);
	}

	rsslSocketChannel->protocolType = (RsslUInt8)wsSess->protocol;
	if (wsSess->protocol == RWS_SP_RWF)
	{
		rwsInitializeProtocolFuncs();
		ipcSetSocketChannelProtocolHdrFuncs(rsslSocketChannel, RSSL_CONN_TYPE_WEBSOCKET);
		rsslSocketChannel->intState = RIPC_INT_ST_CONNECTING;

		return ipcConnecting(rsslSocketChannel, inProgress, error);
	}
	else
	{

		if (rsslWebSocketSetChannelFunctions() == RSSL_RET_FAILURE)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Failed to rsslWebSocketSetChannelFunctions() for connection.", 
				__FUNCTION__,__LINE__);
			return(RIPC_CONN_ERROR);
		}

		if (wsSess->deflate)
		{
			wsSess->comp.inDecompress = RSSL_COMP_ZLIB;
			wsSess->comp.outCompression = RSSL_COMP_ZLIB;

			rsslSocketChannel->outCompression = wsSess->comp.outCompression;

			wsSess->comp.inDecompFuncs = ipcGetCompFunc(wsSess->comp.outCompression);
			wsSess->comp.outCompFuncs = ipcGetCompFunc(wsSess->comp.outCompression);

			if ((wsSess->comp.inDecompFuncs->decompressInit == 0) ||
				(wsSess->comp.inDecompFuncs->decompressEnd == 0) ||
				(wsSess->comp.inDecompFuncs->decompress == 0) ||
				(wsSess->comp.outCompFuncs->compress == 0) ||
				(wsSess->comp.outCompFuncs->compressEnd == 0) ||
				(wsSess->comp.outCompFuncs->compressInit == 0))
			{
				wsSess->comp.inDecompFuncs = 0;
				wsSess->comp.outCompFuncs = 0;

				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1007 Server has specified an unknown compression type (%d)\n",
					__FILE__, __LINE__, RSSL_COMP_ZLIB);
				return(RIPC_CONN_ERROR);
			}

			if (wsSess->comp.inDecompFuncs)
			{
				wsSess->comp.c_stream_in = (*(wsSess->comp.inDecompFuncs->decompressInit))(RSSL_TRUE,error);
				if (wsSess->comp.c_stream_in == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

					return(RIPC_CONN_ERROR);
				}
			}
			if (wsSess->comp.outCompFuncs)
			{
				wsSess->comp.c_stream_out = (*(wsSess->comp.outCompFuncs->compressInit))(wsSess->comp.zlibLevel, RSSL_TRUE,error);
				if (wsSess->comp.c_stream_out == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

					return(RIPC_CONN_ERROR);
				}
			}
		}

		/* Determine what the proper maxMsgSize will be */
		rsslSocketChannel->maxUserMsgSize = (RsslInt32)wsSess->maxMsgSize;
		// Refine this calculation or dont worry about adding the
		//          WebSocket header here
		rsslSocketChannel->maxMsgSize = (RsslUInt32)(wsSess->maxMsgSize + RWS_MAX_HEADER_SIZE);

		/* Initialize the input buffer */
		rsslSocketChannel->inputBuffer = ipcAllocGblMsg((rsslSocketChannel->maxMsgSize * rsslSocketChannel->readSize));

		wsSess->reassemblyBuffer = ipcAllocGblMsg(rsslSocketChannel->maxMsgSize * 10);

		if (wsSess->reassemblyBuffer == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Could not allocate memory for session's reassembly buffer",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}

		if (wsSess->comp.inDecompress)
		{
			wsSess->comp.decompressBuf = ipcAllocGblMsg(rsslSocketChannel->maxMsgSize);
			if (wsSess->comp.decompressBuf == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Could not compression allocate buffer memory.\n",
					__FILE__, __LINE__);

				return(RIPC_CONN_ERROR);
			}
			rsslSocketChannel->curInputBuf = ipcDupGblMsg(wsSess->comp.decompressBuf);
		}
		else
		{
			rsslSocketChannel->curInputBuf = ipcDupGblMsg(rsslSocketChannel->inputBuffer);
}

		if ((rsslSocketChannel->inputBuffer == 0) || (rsslSocketChannel->curInputBuf == 0))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Could not allocate compression buffer memory.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}
		rsslSocketChannel->readSize = (RsslInt32)rsslSocketChannel->inputBuffer->maxLength / 2;

		if ((rsslSocketChannel->guarBufPool->sharedPool) &&
			(rsslSocketChannel->guarBufPool->sharedPool->initialized == 0))
		{
			if (rtrBufferPoolFinishInit(rsslSocketChannel->guarBufPool->sharedPool, rsslSocketChannel->maxMsgSize) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Could not initialize shared buffer pool.\n",
					__FILE__, __LINE__);

				return(RIPC_CONN_ERROR);
			}
		}

		if (rtr_dfltcSetBufSize(rsslSocketChannel->guarBufPool, rsslSocketChannel->maxMsgSize) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Could not allocate buffer memory\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}

		/* priority write stuff */
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++);
		{
			rsslInitQueue(&rsslSocketChannel->priorityQueues[i].priorityQueue);
			rsslSocketChannel->priorityQueues[i].queueLength = 0;
		}

		/* this may change if I let them pass it in off bind/connect */
		rsslSocketChannel->flushStrategy[0] = 0;
		rsslSocketChannel->flushStrategy[1] = 1;
		rsslSocketChannel->flushStrategy[2] = 0;
		rsslSocketChannel->flushStrategy[3] = 2;
		rsslSocketChannel->flushStrategy[4] = 0;
		rsslSocketChannel->flushStrategy[5] = 1;
		rsslSocketChannel->flushStrategy[6] = -1;

		rsslSocketChannel->currentOutList = 0;

		rsslSocketChannel->compressQueue = -1;
		rsslSocketChannel->nextOutBuf = -1;
	}

	rsslSocketChannel->state = RSSL_CH_STATE_ACTIVE;
	rsslSocketChannel->intState = RIPC_INT_ST_ACTIVE;

	return(RIPC_CONN_ACTIVE);
}

RsslInt32 rwsSendResponseHandshake(RsslSocketChannel *rsslSocketChannel, rwsSession_t *wsSess, RsslError *error)
{
	RsslInt32			cc = 0, respLen = 0;
	char				resp[RWS_MAX_HTTP_HEADER_SIZE];
	// TODO Validate this using max key length per RFC standard 16(?) bytes
	ripcRWFlags			rwflags = 0;

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	_DEBUG_TRACE_WS_CONN("fd "SOCKET_PRINT_TYPE" protocol %s(%d)\n", 
			rsslSocketChannel->stream, wsSess->protocolName, wsSess->protocol)

	respLen = snprintf(resp, RWS_MAX_HTTP_HEADER_SIZE, "HTTP/1.1 101 Switching Protocols\r\n");
	respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, "Upgrade: websocket\r\n");
	respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, "Connection: Upgrade\r\n");
	respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, "Sec-WebSocket-Protocol: ");

	respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, "%s\r\n", wsSess->protocolName);

	if (wsSess->server->compressionSupported == RSSL_COMP_ZLIB && wsSess->deflate)
	{
		respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, 
								"Sec-WebSocket-Extensions: permessage-deflate");

		//TODO When they become configurable
		if (wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)
			respLen += snprintf(resp + respLen, RWS_MAX_HTTP_HEADER_SIZE, ";server_no_context_takeover");

		//TODO When they become configurable
		if (wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_INBOUND_CONTEXT)
			respLen += snprintf(resp + respLen, RWS_MAX_HTTP_HEADER_SIZE, ";client_no_context_takeover");

		respLen += snprintf (resp + respLen, RWS_MAX_HTTP_HEADER_SIZE, "\r\n");
	}

	wsSess->keyAccept.data = _getWebSocketAcceptKey(wsSess->keyRecv.data, wsSess->keyRecv.length);
	if (wsSess->keyAccept.data == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> Error allocating memory for response field, Sec-Websocket-Accept value.", __FUNCTION__,__LINE__);
		return(-1);
	}

	wsSess->keyAccept.length = (RsslInt32)strlen(wsSess->keyRecv.data);
	respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, "Sec-WebSocket-Accept: %s\r\n", wsSess->keyAccept.data);

	respLen += snprintf(resp +respLen, RWS_MAX_HTTP_HEADER_SIZE, "\r\n");

	cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, resp, respLen,rwflags, error);

	_DEBUG_TRACE_WS_WRITE("Response sent, fd "SOCKET_PRINT_TYPE" cc %d err %d\n", 
					rsslSocketChannel->stream, cc, errno)
	
	return(cc);
}

ripcSessInit rwsAcceptWebSocket(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
	rwsSession_t *wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rwsAcceptWebSocket", "wsSess", error))
		return(RIPC_CONN_ERROR);

	if ((rwsSendResponseHandshake(rsslSocketChannel, wsSess, error)) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
							"<%s:%d> Error: 1002 rwsClientSendHandshake() Client WS handshake. System errno: (%d)\n",
							__FILE__, __LINE__, errno);
	
		return(RIPC_CONN_ERROR);
	}
	
	if (rsslSocketChannel->protocolType == RWS_SP_RWF)
	{
		// only set the appropriate funcs at this point
		rwsInitializeProtocolFuncs();
		ipcSetSocketChannelProtocolHdrFuncs(rsslSocketChannel, RSSL_CONN_TYPE_WEBSOCKET);
		rsslSocketChannel->intState = RIPC_INT_ST_READ_HDR;

		return (RIPC_CONN_IN_PROGRESS);
	}
	else
	{
		wsSess->reassemblyBuffer = ipcAllocGblMsg(rsslSocketChannel->maxMsgSize * 10);
		if (wsSess->reassemblyBuffer == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Could not allocate memory for session's reassembly buffer",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}

		rsslSocketChannel->curInputBuf = ipcDupGblMsg(rsslSocketChannel->inputBuffer);
		rsslSocketChannel->guarBufPool = rtr_dfltcAllocatePool(
											rsslSocketChannel->server->maxGuarMsgs, 
											rsslSocketChannel->server->maxGuarMsgs, 10, 
											rsslSocketChannel->maxMsgSize, 
											rsslSocketChannel->server->sharedBufPool,
											(rsslSocketChannel->server->maxNumMsgs - rsslSocketChannel->server->maxGuarMsgs),
											0);
		if (rsslSocketChannel->guarBufPool == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text,MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Could not allocate memory for session's output buffer",
						__FILE__,__LINE__);
			
			return(RIPC_CONN_ERROR);
		}

		//  compression/decompression Initialization
		wsSess->comp.type = wsSess->server->compressionSupported;
		if (wsSess->comp.type == RSSL_COMP_ZLIB && wsSess->deflate)
		{
			wsSess->comp.zlibLevel = wsSess->server->zlibCompLevel;
			wsSess->comp.outCompression = wsSess->comp.type;

			rsslSocketChannel->outCompression = wsSess->comp.outCompression;

			wsSess->comp.outCompFuncs = ipcGetCompFunc(wsSess->comp.outCompression);
			wsSess->comp.c_stream_out = (*(wsSess->comp.outCompFuncs->compressInit))(wsSess->comp.zlibLevel,RSSL_TRUE,error);

			// Must setup for decompression of inbound messages
			// browsers normally compress all messages.
			wsSess->comp.inDecompress = wsSess->comp.type;
			
			wsSess->comp.inDecompFuncs = ipcGetCompFunc(wsSess->comp.inDecompress);
			wsSess->comp.c_stream_in = (*(wsSess->comp.inDecompFuncs->decompressInit))(RSSL_TRUE,error);
		}

	}

	if (rsslWebSocketSetChannelFunctions() == RSSL_RET_FAILURE)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Failed to rsslWebSocketSetChannelFunctions() for new client connection.", 
			__FUNCTION__,__LINE__);
		return(RIPC_CONN_ERROR);
	}

	rsslSocketChannel->state = RSSL_CH_STATE_ACTIVE;
	rsslSocketChannel->intState = RIPC_INT_ST_ACTIVE;

	return(RIPC_CONN_ACTIVE);
}

ripcSessInit rwsValidateWebSocketRequest(RsslSocketChannel *rsslSocketChannel, char * hdrStart, RsslInt32 cc, RsslError *error)
{
	RsslInt32	httpHeaderLen = 0;
	rwsSession_t *wsSess = 0;
				
	if (rsslSocketChannel->server->rwsServer == 0)
		return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_CONN_ERROR, error));

	httpHeaderLen = rwsReadOpeningHandshake(hdrStart, cc, 0, rsslSocketChannel, error);

	if (httpHeaderLen < 0)
	{
		if(httpHeaderLen == -400) /* Bad HTTP request */
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_NO_RESRC, error));
		else
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_CONN_ERROR, error));
	}	
	else if (httpHeaderLen == 0)
	{
		/* Not done with header yet, unlikely */
		rsslSocketChannel->inputBufCursor += cc;

		if (rsslSocketChannel->inputBufCursor >= rsslSocketChannel->inputBuffer->maxLength)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> HTTP request is too large.", 
														__FILE__,__LINE__);
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_REQUEST_TOO_LARGE, error));
		}
		else
			return RIPC_CONN_IN_PROGRESS;
	} 
	else 
	{
		wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

		_DEBUG_TRACE_WS_CONN("HTTP WS Header fd "SOCKET_PRINT_TYPE" inBC %d inBL %d hdrL %d\n",
									rsslSocketChannel->stream,
									rsslSocketChannel->inputBufCursor,
									rsslSocketChannel->inputBuffer->length,
									httpHeaderLen)

		rsslSocketChannel->inputBufCursor += httpHeaderLen;

		if ( rsslSocketChannel->inputBuffer->length == rsslSocketChannel->inputBufCursor )
		{
			/* Only the Header was in the buffer */
			rsslSocketChannel->inputBuffer->length = 0;
			rsslSocketChannel->inputBufCursor = 0;
		}
		
		// Validate upgrade request
		if (!wsSess->upgrade && !wsSess->connUpgrade)
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_NO_RESRC, error));

		if(rsslSocketChannel->mountNak == 1)
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_NO_RESRC, error));

		wsSess->version = wsSess->server->version;
		if (wsSess->versionRecv < wsSess->version)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text,MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Invalid WebSocket Version (%d), version supported (%d)",
				__FILE__,__LINE__, wsSess->versionRecv, wsSess->version);
			
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_UNSUPPORTED_VERSION, error));
		}

		/* Check for unknown protocol. */
		if (wsSess->protocol == RWS_SP_NONE)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Unsupported Websocket protocol type(%d) ", __FILE__,__LINE__, 
					wsSess->protocol);
			return(rwsRejectSession(rsslSocketChannel, RSSL_WS_REJECT_UNSUPPORTED_SUB_PROTOCOL, error));
		}
		rsslSocketChannel->protocolType = (RsslUInt8)wsSess->protocol;

		return (rwsAcceptWebSocket(rsslSocketChannel, error));
	}
}


RsslInt32 rwsRejectSession(RsslSocketChannel *rsslSocketChannel, RsslRejectCodeType code, RsslError *error)
{
	char				resp[RWS_MAX_HTTP_HEADER_SIZE];
	RsslInt32			cc;
	RsslInt32			retVal = RIPC_CONN_ERROR;
	ripcRWFlags			rwflags = RIPC_RW_WAITALL; /* Waits until writing bytes */

	_DEBUG_TRACE_WS_CONN("Rejecting WS Session fd "SOCKET_PRINT_TYPE"\n",rsslSocketChannel->stream)

	switch(code)
	{
	/* Since RWS_MAX_HTTP_HEADER_SIZE is at least 1024, there isnt any concern with keeping
	 * track of remaining space in the response buffer */
		case RSSL_WS_REJECT_CONN_ERROR:
			cc = snprintf(resp, RWS_MAX_HTTP_HEADER_SIZE,      "HTTP/1.1 400 Bad Request\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Content-Type: text/html; charset=UTF-8\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Connection: close\r\n");
			break;
		case RSSL_WS_REJECT_NO_SESS:
		case RSSL_WS_REJECT_NO_RESRC:
		case RSSL_WS_REJECT_UNSUPPORTED_VERSION:
		case RSSL_WS_REJECT_UNSUPPORTED_SUB_PROTOCOL:
			cc = snprintf(resp, RWS_MAX_HTTP_HEADER_SIZE,      "HTTP/1.1 400 Bad Request\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Content-Type: text/html; charset=UTF-8\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Cache-Control: no-cache, private, no-store\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Transfer-Encoding: chunked\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Connection: close\r\n");
			break;
		case RSSL_WS_REJECT_AUTH_FAIL:
			cc = snprintf(resp, RWS_MAX_HTTP_HEADER_SIZE,      "HTTP/1.1 401 Unauthorized\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Content-Type: text/html; charset=UTF-8\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Cache-Control: no-cache, private, no-store\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Transfer-Encoding: chunked\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Connection: close\r\n");
			break;
		case RSSL_WS_REJECT_REQUEST_TOO_LARGE:
			cc = snprintf(resp, RWS_MAX_HTTP_HEADER_SIZE,       "HTTP/1.1 413 Payload Too Large\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Content-Type: text/html; charset=UTF-8\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Cache-Control: no-cache, private, no-store\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Transfer-Encoding: chunked\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Connection: close\r\n");
			break;
		default:
			cc = snprintf(resp, RWS_MAX_HTTP_HEADER_SIZE,      "HTTP/1.1 400 Bad Request\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Content-Type: text/html; charset=UTF-8\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Cache-Control: no-cache, private, no-store\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Transfer-Encoding: chunked\r\n");
			cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "Connection: close\r\n");
			break;
	}

	cc += snprintf(resp +cc , RWS_MAX_HTTP_HEADER_SIZE, "\r\n");

	cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo,  resp, cc ,rwflags, error);
	
	return(retVal);
}


#define _WEBSOCKET_FRAME_HEADER_LENGTH(__buf)\
	(	_WS_CONTROL_HEADER_LEN +\
		( (rwfGetBit(__buf[1], _WS_BIT_POS_MASKKEY)) ? _WS_MASK_KEY_FIELD_LEN : 0) + \
		((__buf[1] & _WS_PAYLOAD_LEN_MASK)==_WS_FLAG_2BYTE_EXT_PAYLOAD ? _WS_2BYTE_EXT_PAYLOAD :\
		 ((__buf[1] & _WS_PAYLOAD_LEN_MASK)==_WS_FLAG_8BYTE_EXT_PAYLOAD ? _WS_8BYTE_EXT_PAYLOAD : 0))\
	)

static RsslInt32 _resetWSFrame(rwsFrameHdr_t *frame, char * buffer)
{
	frame->pCtlHdr = buffer;
	/* DO NOT reset the frame fields:
	 *		->fragment
	 *		->dataType
	 *		->compressed
	 *		->finset
	 *		->opcode
	 *	These fields keep the state for any fragmented frames being 
	 *	reassembled along with possible control frames intermixed
	 */
	frame->pExtHdr = 0;
	frame->hdrLen = 0;
	frame->maskVal = 0;
	frame->payload = 0;
	frame->payloadLen = 0;
	frame->advancedInputCursor = RSSL_FALSE;

	return 1;
}

static RsslBool _decodeWSFrame(rwsFrameHdr_t *frame, char * buffer, size_t bufLen)
{
	/* Set the location of the WS control header if not set */
	if (frame->pCtlHdr == 0 && bufLen > 0)
		frame->pCtlHdr = buffer;
	/* If there is no 2 byte control header to read, return after
	 * flagging this as a partial header */

	if (bufLen < 2)
	{
		frame->partial = RSSL_TRUE;
		return frame->partial;
	}
	/*  0                   1                   2                   3
	 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 * +-+-+-+-+-------+-+-------------+-------------------------------+
	 * |F|R|R|R| opcode|M| Payload len |    Extended payload length    |...
	 * |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |...
	 * |N|V|V|V|       |S|             |   (if payload len==126/127)   |...
	 * | |1|2|3|       |K|             |                               |...
	 * +-+-+-+-+-------+-+-------------+-------------------------------+ */
	/* Calculate the frame header length from the 2 byte control header portion */
	frame->hdrLen = _WEBSOCKET_FRAME_HEADER_LENGTH(frame->pCtlHdr);

	//_DEBUG_TRACE_WS_FRAME(frame->pCtlHdr)
	/* finSet bit for complete/fragmented frame */
	frame->finSet = rwfGetBit(*(frame->pCtlHdr), _WS_BIT_POS_FIN);
	/* RSV1 bit for any handshake negotiated extensions */
	frame->rsv1Set = rwfGetBit(*(frame->pCtlHdr), _WS_BIT_POS_RSV1);
	/* Frame OpCode */
	frame->opcode = (*(frame->pCtlHdr) & _WS_OPCODE_MASK);

	/* Frame fragments should not be updated if a Control Frame is received in the
	 * middle of collecting/assembling frames */
	if (!(frame->control = RWS_IS_CONTROL_FRAME(frame->opcode)))
	{
		frame->fragment = (!frame->finSet ? 1 : 0);
		if (frame->opcode != _WS_OPC_CONT)
		{
			/* Set the compressed flag for the payload for only fin or complete frames */
			/* The Compression bit (RSV1) is onlt set on the first frame and not set on the
			 * continuation frame if fragmented */
			frame->dataType = frame->opcode;
			frame->compressed = frame->rsv1Set;
		}
	}

	/* Flag to identify if ayload is masked and key suffix to the frame header */
	frame->maskSet = rwfGetBit(*(frame->pCtlHdr + 1), _WS_BIT_POS_MASKKEY);

	/* PayloadLength or flag for 2 or 8 byte payload length fields */
	frame->payloadLen = (*(frame->pCtlHdr + 1 ) & _WS_PAYLOAD_LEN_MASK);

	if (frame->payloadLen == _WS_FLAG_2BYTE_EXT_PAYLOAD)
		frame->extHdrLen = _WS_2BYTE_EXT_PAYLOAD;
	else if (frame->payloadLen == _WS_FLAG_8BYTE_EXT_PAYLOAD)
		frame->extHdrLen = _WS_8BYTE_EXT_PAYLOAD;
	else
		frame->extHdrLen = 0;

	/* Get the values if the WS frame header has been read */
	if (bufLen >= frame->hdrLen)
	{
		frame->pExtHdr = frame->pCtlHdr + _WS_CONTROL_HEADER_LEN;
		if (frame->extHdrLen)
		{
			if(frame->extHdrLen == _WS_2BYTE_EXT_PAYLOAD)
			{
				RsslUInt16 pl = 0;
				frame->pExtHdr += rwfGet16(pl, frame->pExtHdr);
				frame->payloadLen = pl;
			}
			else
			{
				RsslUInt64 pl = 0;
				frame->pExtHdr += rwfGet64(pl, frame->pExtHdr);
				frame->payloadLen = pl;
			}
		}
		/* get the value of the mask key if it is set and not already set on a previous pass */
		if (frame->maskSet && frame->maskVal == 0)
			frame->maskVal = _getMaskKey((char*)(frame->mask), frame->pExtHdr);
		
		/* mark the beginning of the payload segment when enough bytes have been read*/
		if (frame->payload == 0 && frame->payloadLen && bufLen >= (frame->hdrLen + 1))
			frame->payload = frame->pCtlHdr + frame->hdrLen;

		frame->partial = (bufLen < (frame->hdrLen + frame->payloadLen));
	}
	else
	{
		frame->partial = RSSL_TRUE;
		/* If this is a partial Frame Header, no reason to calculate the 
		 * extended payload length until the complete header has been read */
		if (frame->payloadLen == _WS_FLAG_2BYTE_EXT_PAYLOAD || 
			frame->payloadLen == _WS_FLAG_8BYTE_EXT_PAYLOAD)
			frame->payloadLen = 0;
	}

	return frame->partial;
}

RsslRet rwsProcessWsOpCodes(RsslSocketChannel	*rsslSocketChannel, RsslError *error)
{
	rwsSession_t		*wsSess = 0;
	rwsFrameHdr_t		*frame = 0;

	if (IPC_NULL_PTR(rsslSocketChannel, "", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;
	frame = &(wsSess->frameHdr);

	switch (frame->opcode)
	{
		case RWS_OPC_CONT:
		case RWS_OPC_TEXT:
		case RWS_OPC_BINARY:
		case RWS_OPC_CLOSE:
		case RWS_OPC_PING:
		case RWS_OPC_PONG:
		{
			/* Any WS frames sent client to the server, should be masked per RFC6455 */
			if (!frame->maskSet && !rsslSocketChannel->clientSession)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf((error->text), MAX_RSSL_ERROR_TEXT,
					"<%s():%d> Unmasked client frame", __FUNCTION__, __LINE__);
				return(RSSL_RET_FAILURE);
			}
			else if (frame->maskSet)
			{
				_DEBUG_TRACE_WS_READ("Unmasking Key %u mv cur %d bytes hdrLen %d payLen %d\n",
					frame->maskVal,
					frame->cursor,
					frame->hdrLen,
					frame->payloadLen)
					_maskDataBlock(frame->mask, frame->payload, frame->payloadLen);
			}
			_DEBUG_TRACE_WS_FRAME(((char*)frame->pCtlHdr))

			if (frame->opcode == RWS_OPC_CLOSE)
			{
				RsslUInt16 closeCode = 0;

				wsSess->recvClose = RSSL_TRUE;
				/* Move inputBuffer->length and inputBufCursor past the WebSocket
					* protocol message */
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> received WS_CLOSE frame ", __FUNCTION__, __LINE__);
				if (frame->payload && frame->payloadLen >= 2)
				{
					RsslUInt16 sc = 0;
					rwfGet16(sc, frame->payload);
					if (sc < RWS_CFSC_UNKNOWN_15)
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"WS Code %d %s", sc, _getClosedText((rwsCFStatusCodes_t)sc));
				}

				/*  The peer is closing the session */
				_DEBUG_TRACE_WS_READ("Rcvd WS_CLOSE fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)
					if (!wsSess->sentClose)
					{
						/* Must unlock first as the rwsSendWsClose() always acquires the lock */
						IPC_MUTEX_UNLOCK(rsslSocketChannel);

						if (rwsSendWsClose(rsslSocketChannel, RWS_CFSC_ENDPOINT_GONE, error) < 0)
						{
							snprintf((error->text), MAX_RSSL_ERROR_TEXT,
								"<%s():%d> Failed sending WS Close Frame ", __FUNCTION__, __LINE__);
						}

						IPC_MUTEX_UNLOCK(rsslSocketChannel);
					}

				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				return(RSSL_RET_FAILURE);
			}
			else if (frame->opcode == RWS_OPC_PING)
			{
				/* Move inputBuffer->length and inputBufCursor past the WebSocket
					* protocol message */
				_DEBUG_TRACE_WS_READ("Rcvd WS_PONG fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)
					if (rwsSendWsPong(rsslSocketChannel, NULL, error) < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s():%d> Failed sending WS Pong Frame ", __FUNCTION__, __LINE__);
						return(RSSL_RET_FAILURE);
					}

				return (RSSL_RET_SUCCESS);
			}
			else if (frame->opcode == RWS_OPC_PONG)
			{
				/* Move inputBuffer->length and inputBufCursor past the WebSocket
					* protocol message */
				_DEBUG_TRACE_WS_READ("Rcvd WS_PONG fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)
					snprintf((error->text), MAX_RSSL_ERROR_TEXT,
						"<%s:%d> received WS_PONG frame", __FUNCTION__, __LINE__);

				return (RSSL_RET_SUCCESS);
			}
		}
	}

	return (RSSL_RET_SUCCESS);
}

void handleWebSocketMessages(RsslSocketChannel *rsslSocketChannel, RsslRet *readret, rwsSession_t *wsSess, rwsFrameHdr_t *frame, RsslInt32 bytesRead, RsslInt32 *uncompBytesRead,RsslError *error)
{
	RsslInt32		hdrLen = 0;
	RsslUInt16		frameLen = 2;
	unsigned char	overRun[4];
	unsigned char*	mask = 0;
	ripcCompBuffer	compBuf;
	RsslInt32		uncompRead = 0;
	int				retVal;
	static unsigned char compressEnd[] = { 0x00, 0x00, 0xFF, 0xFF };

	*readret = RSSL_RET_SUCCESS;

	do
	{
		if (!frame->finSet)
		{
			// if opcode is CONT (0), this isn't the first fragment
			if (frame->opcode == RWS_OPC_CONT)
			{
				if (frame->compressed)
				{
					compBuf.next_in = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;
					compBuf.avail_in = (RsslUInt32)frame->payloadLen;
					compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
					compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);

					if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
						(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)
						? 1 : 0, error)) < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Decompress failed for WS frame ", __FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;
					_DEBUG_TRACE_WS_READ("reasemb->len %d b_out_used %d", wsSess->reassemblyBuffer->length, compBuf.bytes_out_used)
					if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0)
					{
						// Not enough room in buffer to decompress too. Double size of buffer and continue reading
						do
						{
							if ((wsSess->reassemblyBuffer = doubleSizeAndRealloc(wsSess->reassemblyBuffer, wsSess->reassemblyBuffer->length, wsSess->maxPayload, error)) == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf((error->text), MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Resizing the reassembly buffer failed for WS frame in reassembly ",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}
							compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
							compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);
							if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
								0, /* In the middle of a message; don't reset even if there's no context-takeover */
								error)) < 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
								snprintf((error->text), MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Decompress failed for WS frame in reassembly ",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}

							wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;

						} while (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength < wsSess->maxPayload);

						if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength == wsSess->maxPayload)
						{
							// More to read but we are already at max size, disconnect
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Unsupported overall length for fragmented message",
								__FUNCTION__, __LINE__);
							*readret = RSSL_RET_FAILURE;
							return;
						}
					}

					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					_DEBUG_TRACE_WS_READ("reasemb->len %d b_out_used %d", wsSess->reassemblyBuffer->length, compBuf.bytes_out_used)
				}
				else
				{
					if ((wsSess->reassemblyBuffer = checkSizeAndRealloc(wsSess->reassemblyBuffer, wsSess->reassemblyBuffer->length + frame->payloadLen, wsSess->maxPayload, error)) == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Failed to resize reassemblyBuffer while processing WS_CONT frame",
							__FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					memcpy(wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length, rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor, frame->payloadLen);
					wsSess->reassemblyBuffer->length += frame->payloadLen;
					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					*uncompBytesRead = bytesRead;
				}

				rsslSocketChannel->inputBufCursor += (RsslUInt32)frame->payloadLen;
			}
			else 
			{	// This is the first fragment
				if (!wsSess->reassemblyUnfinished)
					wsSess->reassemblyBuffer->length = 0;

				if (frame->compressed)
				{
					compBuf.next_in = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;
					compBuf.avail_in = (RsslUInt32)frame->payloadLen;
					compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
					compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);

					if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
						(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)
						? 1 : 0, error)) < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Decompress failed for WS frame ", __FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;
					_DEBUG_TRACE_WS_READ("reasemb->len %d b_out_used %d", wsSess->reassemblyBuffer->length, compBuf.bytes_out_used)
					if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0)
					{
						// Not enough room in buffer to decompress too. Double size of buffer and continue reading
						do
						{
							if ((wsSess->reassemblyBuffer = doubleSizeAndRealloc(wsSess->reassemblyBuffer, wsSess->reassemblyBuffer->length, wsSess->maxPayload, error)) == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf((error->text), MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Resizing the reassembly buffer failed for WS frame in reassembly ",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}
							compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
							compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);
							if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
								0, /* In the middle of a message; don't reset even if there's no context-takeover */
								error)) < 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
								snprintf((error->text), MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Decompress failed for WS frame in reassembly ",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}

							wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;

						} while (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength < wsSess->maxPayload);

						if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength == wsSess->maxPayload)
						{
							// More to read but we are already at max size, disconnect
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Unsupported overall length for fragmented message",
								__FUNCTION__, __LINE__);
							*readret = RSSL_RET_FAILURE;
							return;
						}
					}

					wsSess->reassemblyUnfinished = 1;
					// Remember that we are compressed. The compression flag will not be set in the subsequent fragments.
					wsSess->reassemblyCompressed = 1;

					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					_DEBUG_TRACE_WS_READ("reasemb->len %d b_out_used %d", wsSess->reassemblyBuffer->length, compBuf.bytes_out_used)
				}
				else
				{
					if ((wsSess->reassemblyBuffer = checkSizeAndRealloc(wsSess->reassemblyBuffer, frame->payloadLen, wsSess->maxPayload, error)) == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf((error->text),
							MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Failed to resize reassemblyBuffer while processing the first fragmentation",
							__FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					memcpy(wsSess->reassemblyBuffer->buffer, rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor, frame->payloadLen);
					wsSess->reassemblyBuffer->length = frame->payloadLen;
					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					wsSess->reassemblyUnfinished = 1;
				}

				rsslSocketChannel->inputBufCursor += (RsslUInt32)frame->payloadLen;
			}
		}
		else
		{	/* Fin bit is set */
			if (frame->opcode == RWS_OPC_CONT) /* This is the last fragmented message */
			{
				if (frame->compressed)
				{
					compBuf.next_in = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;
					compBuf.avail_in = (RsslUInt32)frame->payloadLen;
					compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
					compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);

					if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
						(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)
						? 1 : 0, error)) < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Decompress failed for WS frame ", __FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;
					_DEBUG_TRACE_WS_READ("reasemb->len %d b_out_used %d", wsSess->reassemblyBuffer->length, compBuf.bytes_out_used)
					if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0)
					{
						// Not enough room in buffer to decompress too. Double size of buffer and continue reading
						do
						{
							if ((wsSess->reassemblyBuffer = doubleSizeAndRealloc(wsSess->reassemblyBuffer, wsSess->reassemblyBuffer->length, wsSess->maxPayload, error)) == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf((error->text), MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Resizing the reassembly buffer failed for WS frame in reassembly ",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}
							compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
							compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);
							if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
								0, /* In the middle of a message; don't reset even if there's no context-takeover */
								error)) < 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
								snprintf((error->text), MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Decompress failed for WS frame in reassembly ",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}

							wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;

						} while (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength < wsSess->maxPayload);

						if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength == wsSess->maxPayload)
						{
							// More to read but we are already at max size, disconnect
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Unsupported overall length for fragmented message",
								__FUNCTION__, __LINE__);
							*readret = RSSL_RET_FAILURE;
							return;
						}
					}

					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					wsSess->reassemblyUnfinished = 0;
					wsSess->reassemblyCompressed = 0;
				}
				else
				{
					if ((wsSess->reassemblyBuffer = checkSizeAndRealloc(wsSess->reassemblyBuffer, wsSess->reassemblyBuffer->length + frame->payloadLen, wsSess->maxPayload, error)) == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Failed to resize for reassemblyBuffer while processing the last WS_CONT frame",
							__FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					memcpy(wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length, rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor, frame->payloadLen);
					wsSess->reassemblyBuffer->length += frame->payloadLen;
					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					wsSess->reassemblyUnfinished = 0;
				}

				rsslSocketChannel->inputBufCursor += (RsslUInt32)frame->payloadLen;

				if (!wsSess->reassemblyUnfinished)
					*uncompBytesRead = (RsslInt32)(wsSess->reassemblyBuffer->length + rsslSocketChannel->inBufProtOffset);

				break;
			}
			else
			{
				if (!wsSess->reassemblyUnfinished)
					wsSess->reassemblyBuffer->length = 0;

				/* This is for single frame message case only */
				if (frame->compressed)
				{
					compBuf.next_in = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;
					compBuf.avail_in = (RsslUInt32)frame->payloadLen;
					compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
					compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);

					// If at the end of the full message, add the compressEnd
					// Save the four bytes currently there; we are overwriting them, so we'll need to restore them
					// after decompressing.
					if (frame->finSet)
					{
						memcpy(overRun, rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor + frame->payloadLen, 4);
						memcpy(rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor + frame->payloadLen, compressEnd, 4);
						compBuf.avail_in += 4;
					}

					if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
						(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT)
						? 1 : 0, error)) < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Decompress failed for WS frame ", __FUNCTION__, __LINE__);
						*readret = RSSL_RET_FAILURE;
						return;
					}

					wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;
					_DEBUG_TRACE_WS_READ("reasemb->len %d b_out_used %d", wsSess->reassemblyBuffer->length, compBuf.bytes_out_used)
						if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0)
						{
							// Not enough room in buffer to decompress too. Double size of buffer and continue reading
							do
							{
								if ((wsSess->reassemblyBuffer = doubleSizeAndRealloc(wsSess->reassemblyBuffer, wsSess->reassemblyBuffer->length, wsSess->maxPayload, error)) == 0)
								{
									_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
									snprintf((error->text), MAX_RSSL_ERROR_TEXT,
										"<%s:%d> Resizing the reassembly buffer failed for WS frame in reassembly ",
										__FUNCTION__, __LINE__);
									*readret = RSSL_RET_FAILURE;
									return;
								}
								compBuf.next_out = wsSess->reassemblyBuffer->buffer + wsSess->reassemblyBuffer->length;
								compBuf.avail_out = (unsigned long)(wsSess->reassemblyBuffer->maxLength - wsSess->reassemblyBuffer->length);
								if ((retVal = (*(wsSess->comp.inDecompFuncs->decompress)) (wsSess->comp.c_stream_in, &compBuf,
									0, /* In the middle of a message; don't reset even if there's no context-takeover */
									error)) < 0)
								{
									_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
									snprintf((error->text), MAX_RSSL_ERROR_TEXT,
										"<%s:%d> Decompress failed for WS frame in reassembly ",
										__FUNCTION__, __LINE__);
									*readret = RSSL_RET_FAILURE;
									return;
								}

								wsSess->reassemblyBuffer->length += compBuf.bytes_out_used;

							} while (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength < wsSess->maxPayload);

							if (retVal > 0 && compBuf.avail_in > 0 && compBuf.avail_out == 0 && wsSess->reassemblyBuffer->maxLength == wsSess->maxPayload)
							{
								// More to read but we are already at max size, disconnect
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Unsupported overall length for fragmented message",
									__FUNCTION__, __LINE__);
								*readret = RSSL_RET_FAILURE;
								return;
							}
						}
					// Restore the original four bytes that we overwrote with the compressEnd bytes
					if (frame->finSet)
						memcpy(rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor + frame->payloadLen, overRun, 4);

					wsSess->reassemblyUnfinished = 0;
					wsSess->reassemblyCompressed = 0;

					*uncompBytesRead = (RsslInt32)(wsSess->reassemblyBuffer->length + rsslSocketChannel->inBufProtOffset);

					rsslSocketChannel->curInputBuf->buffer = wsSess->reassemblyBuffer->buffer;
					rsslSocketChannel->curInputBuf->length = wsSess->reassemblyBuffer->length;
					rsslSocketChannel->inputBufCursor += (RsslUInt32)frame->payloadLen;

					break;
				}
				else
				{
					*uncompBytesRead = bytesRead;

					// Just point to the message in the input buffer
					rsslSocketChannel->curInputBuf->buffer = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;
					rsslSocketChannel->curInputBuf->length = frame->payloadLen;
					rsslSocketChannel->inputBufCursor += (RsslUInt32)frame->payloadLen;

					break;
				}
			}
		}

		/* Parse more Websocket frame if there is more data */
		if (rsslSocketChannel->inputBufCursor < rsslSocketChannel->inputBuffer->length)
		{
			bytesRead -= (RsslInt32)(frame->hdrLen + frame->payloadLen);
			_resetWSFrame(frame, (rsslSocketChannel->inputBuffer->buffer + wsSess->inputReadCursor));
			_decodeWSFrame(frame, rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor, bytesRead);

			if (frame->partial)
			{
				_DEBUG_TRACE_WS_READ("Read PARTIAL %d of %d msg:\n", frame->cursor, frame->hdrLen + frame->payloadLen);
				*readret = RSSL_RET_READ_WOULD_BLOCK;
				return;
			}

			/* move ->inputBufCursor past the WS frame header */
			if (bytesRead >= frame->hdrLen && (frame->advancedInputCursor == RSSL_FALSE))
			{
				rsslSocketChannel->inputBufCursor += frame->hdrLen;
				rsslSocketChannel->inBufProtOffset += frame->hdrLen;
				frame->advancedInputCursor = RSSL_TRUE;
			}

			*readret = rwsProcessWsOpCodes(rsslSocketChannel, error);

			if (*readret != RSSL_RET_SUCCESS)
			{
				return;
			}

			wsSess->inputReadCursor += frame->hdrLen + frame->payloadLen;

			if (wsSess->inputReadCursor == wsSess->actualInBuffLen)
			{
				wsSess->inputReadCursor = 0;
				wsSess->actualInBuffLen = 0;
			}
		}
		else
		{
			if (rsslSocketChannel->inputBufCursor == rsslSocketChannel->inputBuffer->length)
			{
				rsslSocketChannel->inputBufCursor = 0;
				rsslSocketChannel->inputBuffer->length = 0;
			}

			break; /* No more data */
		}

	} while (1);
}

rtr_msgb_t *rwsReadWebSocket(RsslSocketChannel *rsslSocketChannel, RsslRet *readret, int *moreData, RsslInt32* bytesRead, RsslInt32* uncompBytesRead, RsslInt32 *packing, RsslError *error)
{
	RsslInt32		cc = 0, hdrLen = 0;
	RsslUInt16		frameLen = 2;
	ripcRWFlags		rwflags = 0;
	unsigned char*	mask = 0;
	rwsSession_t	*wsSess;
	rwsFrameHdr_t	*frame;
	size_t inputBufferLength = 0;

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);
	*moreData = 0;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;
	frame = &(wsSess->frameHdr);

	inputBufferLength = rsslSocketChannel->inputBuffer->length;

	if (rsslSocketChannel->inputBuffer->length == 0 || frame->partial)
	{
		RsslInt32 readSize;
		// Make sure we either have 2 bytes already read or room to read at least 2 bytes
		// to determine the header length and opcode.

		if ( (readSize = checkInputBufferSpace(rsslSocketChannel, 2)) == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT,
				"<%s:%d> rwsReadWebSocket() internal error, Unable to read WS header",
				__FILE__, __LINE__);

			*readret = RSSL_RET_FAILURE;
			return(0);
		}

		_DEBUG_TRACE_WS_READ("(before read): inBC %d inBL %d\n maxLen %d rwFlgs 0x%0x err %d",
			rsslSocketChannel->inputBufCursor,
			rsslSocketChannel->inputBuffer->length,
			rsslSocketChannel->readSize,
			rwflags, errno)

			cc = rwsReadTransportMsg(rsslSocketChannel,
				rsslSocketChannel->inputBuffer->buffer,
				readSize,
				rwflags, error);
		_DEBUG_TRACE_WS_READ("(after read): cc %d inBC %d inBL %d\n maxLen %d err %d",
			cc, rsslSocketChannel->inputBufCursor,
			rsslSocketChannel->inputBuffer->length,
			rsslSocketChannel->readSize, errno)
	}
	else
	{
		/* Parse more Websocket frame if there is more data */
		if (rsslSocketChannel->inputBufCursor < rsslSocketChannel->inputBuffer->length)
		{
			RsslInt32 bytesRemaining = (RsslInt32)(wsSess->actualInBuffLen - wsSess->inputReadCursor);
			_resetWSFrame(frame, (rsslSocketChannel->inputBuffer->buffer + wsSess->inputReadCursor));
			_decodeWSFrame(frame, rsslSocketChannel->inputBuffer->buffer + wsSess->inputReadCursor, bytesRemaining);

			if (frame->partial)
			{
				*moreData = 1;
				*readret = RSSL_RET_SUCCESS;
				return NULL;
			}

			/* move ->inputBufCursor past the WS frame header */
			if ((wsSess->actualInBuffLen - wsSess->inputReadCursor) >= frame->hdrLen && (frame->advancedInputCursor == RSSL_FALSE))
			{
				rsslSocketChannel->inputBufCursor += frame->hdrLen;
				frame->advancedInputCursor = RSSL_TRUE;
			}

			*readret = rwsProcessWsOpCodes(rsslSocketChannel, error);

			if (*readret != RSSL_RET_SUCCESS)
			{
				return 0;
			}

			wsSess->inputReadCursor += frame->hdrLen + frame->payloadLen;

			if (wsSess->inputReadCursor == wsSess->actualInBuffLen)
			{
				wsSess->inputReadCursor = 0;
				wsSess->actualInBuffLen = 0;
			}

			/* Has more data to process */
			handleWebSocketMessages(rsslSocketChannel, readret, wsSess, frame, bytesRemaining, uncompBytesRead, error);

			if (*readret == RSSL_RET_FAILURE)
			{
				return(0);
			}
		}

		if (rsslSocketChannel->inputBufCursor == rsslSocketChannel->inputBuffer->length)
		{
			rsslSocketChannel->inputBufCursor = 0;
			rsslSocketChannel->inputBuffer->length = 0;
		}
		else
		{
			*moreData = 1;
		}

		if (rsslSocketChannel->curInputBuf->length == 0 ||
			(frame->opcode == RWS_OPC_PING) || (frame->opcode == RWS_OPC_PONG))
			*readret = RSSL_RET_READ_PING;
		else
			*readret = RSSL_RET_READ_WOULD_BLOCK;

		if (frame->finSet && !frame->partial)
		{
			rsslSocketChannel->inBufProtOffset = 0;
			return(rsslSocketChannel->curInputBuf);
		}
		else
			return NULL; /* Unfinished fragment; don't return the buffer until we've completed reassembly. */
	}

	if (frame->opcode == RWS_OPC_CLOSE)
	{
		/* Returns RSSL_RET_FAILURE for applications to close the channel */
		*readret = RSSL_RET_FAILURE;

		return(0);
	}
	else if (cc == RSSL_RET_READ_WOULD_BLOCK)
	{
		*bytesRead = (RsslInt32)(rsslSocketChannel->inputBuffer->length - inputBufferLength);
		*readret = RSSL_RET_READ_WOULD_BLOCK;

		return(0);
	}
	else if (cc < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
				"<%s:%d> Error: 1002 Error Reading WS Header. System errno: (%d)\n",
				__FILE__, __LINE__, errno);
		*readret = RSSL_RET_FAILURE;

		return(0);
	} 
	else
	{
		RsslInt32		uncompRead = 0;
		RsslUInt64		wsFrameLen = frame->payloadLen + frame->hdrLen;

_DEBUG_TRACE_WS_READ("(after read): cc %d inBC %d inBL %d\n", 
						cc, rsslSocketChannel->inputBufCursor, rsslSocketChannel->inputBuffer->length)
	
		*bytesRead = cc;
		rsslSocketChannel->inputBuffer->length += cc;
_DEBUG_TRACE_WS_READ("Update inBL %d ->compressed %d reassemComp %d\n", 
														rsslSocketChannel->inputBuffer->length,
														frame->compressed, 
														wsSess->reassemblyCompressed)

		if ( wsFrameLen > rsslSocketChannel->inputBuffer->maxLength)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Invalid Message Size. Message size is: (%llu). Max Message size is(%zu)\n",
				__FILE__, __LINE__, wsFrameLen, rsslSocketChannel->inputBuffer->maxLength);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		/* Checks to ensure that the session is enabled for the permessage-deflate option as well. */
		if (frame->compressed && !wsSess->deflate)
		{
			// Packet was compressed but session does not indicate it should be
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Compression settings mismatch, internal error", __FUNCTION__, __LINE__);
			*readret = RSSL_RET_FAILURE;
			return(0);
		}

		/* Read more data from network when reading partial web-socket message */
		if (frame->partial)
		{
			*moreData = 1;
			*readret = RSSL_RET_SUCCESS;
			return NULL;
		}

		handleWebSocketMessages(rsslSocketChannel, readret, wsSess, frame, *bytesRead, uncompBytesRead, error);

		if (*readret == RSSL_RET_FAILURE)
		{
			return(0);
		}

		_DEBUG_TRACE_WS_READ("Checking moreData condition cur %u hdrl %u pll %u pl %p inBL %u inBC %u\n", 
														frame->cursor,
														frame->hdrLen,
														frame->payloadLen,
														(void*)(frame->payload),
														rsslSocketChannel->inputBuffer->length, 
														rsslSocketChannel->inputBufCursor)

		if (rsslSocketChannel->inputBufCursor == rsslSocketChannel->inputBuffer->length)
		{
			rsslSocketChannel->inputBufCursor = 0;
			rsslSocketChannel->inputBuffer->length = 0;
		}
		else
		{
			*moreData = 1;
		}

		if (rsslSocketChannel->curInputBuf->length == 0 || 
			(frame->opcode == RWS_OPC_PING) || (frame->opcode == RWS_OPC_PONG) )
			*readret = RSSL_RET_READ_PING;
		else
			*readret = RSSL_RET_READ_WOULD_BLOCK;

		if (frame->finSet && !frame->partial)
		{
			rsslSocketChannel->inBufProtOffset = 0;
			return(rsslSocketChannel->curInputBuf);
		}
		else
			return NULL; /* Unfinished fragment; don't return the buffer until we've completed reassembly. */

	}

	*readret = RSSL_RET_READ_WOULD_BLOCK;
	return 0;
}

/* This will consume the WS header, and make some assumptions which will simplify the number of required checks when reading a WS frame: 
 * This is intended to be called for relatively small payload messages which are normally seen during a 
 * connecting negotiation.  This use case will have little to zero chance the WS header lengths will be
 * more than 2-8 bytes and will not read more than the WS hdr length + the payload length 
 * Assuming: The calling function has passed a pointer to a memory location which is significantly 
 * larger than the length argument (Yes, it is ugly but pretty reasonable considering this is from an 
 * abstracted interface, this code base is controls the calls to it.
 * 1) Read 2 bytes to identify WS header length
 * 2) Read additional bytes for remaining WS header if needed and the WS frame Payload passed via the 
 *    length argument, but no more than the WS header length past
 * 3) If a mask key is present, unmask the payload
 * 4) Update the inputBufferCursor += WS header length, this should enable the calling function 
 *    to begin processing it message using the inputBufferCursor offset.
 * 5) Return ( number of bytes read - WS header length )
 * */
RsslInt32 rwsReadWsConnMsg(void *transport, char *buf, int bufLen, ripcRWFlags rwflags, RsslError *error)
{
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel *)transport;
	rwsSession_t	*wsSess = 0;
	rwsFrameHdr_t	*frame = 0;
	char *pfBuff = 0;
	RsslInt32 bytesRead = 0;
	RsslInt32 bytesNeeded = 0;

	if (IPC_NULL_PTR(rsslSocketChannel, "", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;
	
	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (wsSess) frame = &(wsSess->frameHdr);

	if (!frame->partial)
	{
		clearFrameHeader(frame);
		pfBuff = &(frame->buffer[0]);
	}
	/* First only read the WS 2byte Control Header and it may be the only bytes needed.
	 *  Once the control header is read, any additional header bytes needed will be known */
	bytesNeeded = 2;
	bytesRead = (*(rsslSocketChannel->transportFuncs->readTransport))(rsslSocketChannel->transportInfo, pfBuff, bytesNeeded, rwflags, error);

	//_DEBUG_TRACE_WS_CONN(" called read %d needed %d ibCur %d fhdl %d", bytesRead, bytesNeeded, frame->cursor, frame->hdrLen)
	if (bytesRead < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
								"<%s():%d> Error Reading WS control hdr errno(%d)", __FUNCTION__,__LINE__, errno);
		return -1;
	}

	if (bytesRead == bytesNeeded)
		_decodeWSFrame(frame, pfBuff, bytesRead);

	/* Either the WS frame length is known and > bytesRead or
	 * For some unknown reason there are not the bytesNeeded */
	bytesNeeded = 0;
	/* Worst case bytesRead is 0 or 1, where the remaining length will be read 
	 * to complete the control header length * Otherwise, the remaining extended 
	 * header will be read (frame->hdrLen - frame->cursor) */
	if (bytesRead == 1)
		/* This will never happen .... almost */
		bytesNeeded = 1;
	else if (frame->hdrLen > bytesRead)
		bytesNeeded = (frame->hdrLen - bytesRead);

	if (bytesNeeded)
	{
		bytesRead = (*(rsslSocketChannel->transportFuncs->readTransport))(rsslSocketChannel->transportInfo, pfBuff + bytesRead, bytesNeeded, rwflags, error);

	//_DEBUG_TRACE_WS_CONN(" called read %d needed %d ibCur %d fhdl %d", bytesRead, bytesNeeded, frame->cursor, frame->hdrLen)
		if (bytesRead <= 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
									"<%s():%d> Error Reading WS Extnd Hdr errno(%d)", __FUNCTION__,__LINE__, errno);
			return -1;
		}
		_decodeWSFrame(frame, pfBuff, bytesRead);
	}

	//_DEBUG_TRACE_WS_FRAME_STRUCT(frame)

	if (bytesRead > 0)
	{
		if (frame->control)
			// TODO Should probably do something but I doubt there is anything we can do except 
			// response to a PING or CLOSE.  The questions to ask are, How likely is this since 
			// the client would be an ESDK WS client( not likely) and do we read for the Frame 
			// data message after handling the control message??  So, return and let the 
			// calling function make the decision if it should call this function again
			return 0;

		if (frame->partial && frame->payloadLen == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
						"<%s():%d> Error Reading partial WS frame and 0 length payload errno(%d)", 
						__FUNCTION__,__LINE__, errno);
				return -1;
		}
		/* Only read up to payloadLen or available space */
		bytesNeeded = (RsslInt32)frame->payloadLen;
		bytesRead = (*(rsslSocketChannel->transportFuncs->readTransport))(rsslSocketChannel->transportInfo, buf, bytesNeeded, rwflags, error);

		_DEBUG_TRACE_WS_READ(" called read %d needed %d ibCur %d fhdl %d, pl %d\n", bytesRead, bytesNeeded, frame->cursor, frame->hdrLen, frame->payloadLen)
		if (bytesRead <= 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
						"<%s():%d> Error Reading WS payload errno(%d)", 
						__FUNCTION__,__LINE__, errno);
			return -1;
		}

		_decodeWSFrame(frame, pfBuff, bytesRead);

		if (frame->maskSet && bytesRead >= frame->payloadLen)
		{
			_DEBUG_TRACE_WS_READ(" Unmasking %d byte payload of %d bytes read\n", frame->payloadLen, bytesRead)
			_maskDataBlock(frame->mask, buf, frame->payloadLen);
		}
	}
	return bytesRead;
}

/* This will consume the WS header, populate the rws session struct and set the inputBuffer 
 * pointer to the  beginning of its payload.
 * This function returns RSSL_RET_SUCCESS for successful operation. Otherwise, returns RSSL_RET_FAILURE or RSSL_RET_READ_WOULD_BLOCK
 */
RsslInt32 rwsReadTransportMsg(void *transport, char * buffer, int bufferLen, ripcRWFlags rwflags, RsslError *error)
{
	RsslInt32			bytesRead = 0, cc = 0, totalCC = 0;
	RsslBool			haveData = RSSL_FALSE;
	RsslSocketChannel	*rsslSocketChannel = (RsslSocketChannel *)transport;
	rwsSession_t		*wsSess = 0;
	rwsFrameHdr_t		*frame = 0;
	RsslUInt8			canRead = 1;

	if (IPC_NULL_PTR(rsslSocketChannel, "", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;
	frame = &(wsSess->frameHdr);
	
	/* Using the state when the inputBuffer length == Cursor to reset the WS frame
	 * which  will decode the WS header prepended to the incoming message */
	_DEBUG_TRACE_WS_READ(" Check for reset WS frame cur %u hdrl %u pll %u pl %p inBL %u inBC %u actInBL %u\n",
														frame->cursor,
														frame->hdrLen,
														frame->payloadLen,
														(void*)(frame->payload),
														rsslSocketChannel->inputBuffer->length, 
														rsslSocketChannel->inputBufCursor,
														rsslSocketChannel->inBufProtOffset)
	/* reset the protocol header bytes read counter if not processing a message
	 * split into websocket frame fragments */
	if (!frame->fragment)
		rsslSocketChannel->inBufProtOffset = 0;

	/* This should be when there is an empty inputBuffer with
	 * inputBufCursor reset as well */
	if (wsSess->actualInBuffLen == 0)
	{
		_resetWSFrame(frame, buffer);
		wsSess->inputReadCursor	 = 0;
	}
	/* This should be when there was either a previous call resulting in a partial buffer/frame
	 * or after parsing a complete WS frame. 
	 * The flag frame->partial will identify if an attempt was previously made to read the
	 * WS frame header and validate the bytes received complete the payload */
	else if ((wsSess->actualInBuffLen - wsSess->inputReadCursor) >= 2 && !frame->partial)
	{

		/* This means there was not a previous call with a partial read of a WS frame.
		 * So call to reset the WS frame struct to the end of the previous read and expecting to be
		 * the start of the next WS frame */
		if (!frame->partial)
		{
			_resetWSFrame(frame, (rsslSocketChannel->inputBuffer->buffer + wsSess->inputReadCursor));
			_decodeWSFrame(frame, 
							(rsslSocketChannel->inputBuffer->buffer + wsSess->inputReadCursor), 
							(wsSess->actualInBuffLen - wsSess->inputReadCursor));
			if (!frame->partial)
			{
				haveData = RSSL_TRUE; /* indicates having the entire payload */
			}
		}
	}

	if (!haveData)
	{
		/* Figures out additional read for entire message */
		int neededSize = 0;
		int maxRead = 0;

		if (wsSess->actualInBuffLen != 0)
			neededSize = (int)((frame->hdrLen + frame->payloadLen) - (wsSess->actualInBuffLen - wsSess->inputReadCursor));

		maxRead = (neededSize > 0 ) && (bufferLen >= neededSize) ? neededSize : bufferLen;

		while (cc == 0 || (totalCC < neededSize) )
		{
			if (canRead)
			{
				IPC_MUTEX_UNLOCK(rsslSocketChannel);

				cc = (*(rsslSocketChannel->transportFuncs->readTransport))(rsslSocketChannel->transportInfo,
					(buffer + wsSess->actualInBuffLen),
					maxRead,
					rwflags, error);

				IPC_MUTEX_LOCK(rsslSocketChannel);
				_DEBUG_TRACE_WS_READ(" called read %d inBL %d inBC %d\n", cc,
					rsslSocketChannel->inputBuffer->length,
					rsslSocketChannel->inputBufCursor)
					_DEBUG_TRACE_WS_READ("fd "SOCKET_PRINT_TYPE" cc %d err %d\n", rsslSocketChannel->stream, cc, ((cc <= 0) ? errno : 0))

				if (cc < 0)
				{
					_DEBUG_TRACE_WS_READ("Read Failed: cc = %d\n", cc)

					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT, "<%s:%d> Error:1002 ipcRead() failure. System errno: (%d)",
						__FUNCTION__, __LINE__, errno);
					return(RSSL_RET_FAILURE);
				}

				if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Session Shutdown (%d)", __FILE__, __LINE__, errno);

					return(RSSL_RET_FAILURE);
				}

				if (rsslSocketChannel->blocking == 0)
					canRead = 0;
				else
				{
					/* This is blocking read until it receives the needed data size.*/
					maxRead -= cc;

					if (maxRead <= 0)
					{
						break;
					}
				}
			}
			else
			{
				/* Updates the length for partial read if any. */
				rsslSocketChannel->inputBuffer->length += totalCC;
				return(RSSL_RET_READ_WOULD_BLOCK);
			}

			totalCC += cc;
			wsSess->actualInBuffLen += cc;
		}
	}

	bytesRead = (RsslInt32)(wsSess->actualInBuffLen - wsSess->inputReadCursor);
	// See if we have the 2 bytes so we can calculate the length of the WS Frame Header
	if (bytesRead >= 2)
	{
		_decodeWSFrame(frame, buffer, bytesRead);

		/* move ->inputBufCursor past the WS frame header */
		if (bytesRead >= frame->hdrLen && (frame->advancedInputCursor == RSSL_FALSE))
		{
			rsslSocketChannel->inputBufCursor += frame->hdrLen;
			rsslSocketChannel->inBufProtOffset += frame->hdrLen;
			frame->advancedInputCursor = RSSL_TRUE;
		}

		if (frame->partial)
		{
			_DEBUG_TRACE_WS_READ("Read PARTIAL %d of %d msg:\n", frame->cursor, frame->hdrLen+ frame->payloadLen)

			return totalCC;
		}

		switch(frame->opcode)
		{
		case RWS_OPC_CONT:
		case RWS_OPC_TEXT:
		case RWS_OPC_BINARY:
		case RWS_OPC_CLOSE:
		case RWS_OPC_PING:
		case RWS_OPC_PONG:
		{
			/* Any WS frames sent client to the server, should be masked per RFC6455 */
			if (!frame->maskSet && !rsslSocketChannel->clientSession)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
										"<%s():%d> Unmasked client frame", __FUNCTION__,__LINE__);
				return(RSSL_RET_FAILURE);
			}
			else if (frame->maskSet)
			{

			_DEBUG_TRACE_WS_READ("Unmasking Key %u mv cur %d bytes hdrLen %d payLen %d\n", 
																		frame->maskVal, 
																		frame->cursor,
																		frame->hdrLen,
																		frame->payloadLen)
				_maskDataBlock(frame->mask, frame->payload, frame->payloadLen);
			}
			_DEBUG_TRACE_WS_FRAME(((char*)frame->pCtlHdr))

			if (frame->opcode == RWS_OPC_CLOSE)
			{
				RsslUInt16 closeCode = 0;

				wsSess->recvClose = RSSL_TRUE;
				/* Move inputBuffer->length and inputBufCursor past the WebSocket
				 * protocol message */
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> received WS_CLOSE frame ", __FUNCTION__,__LINE__);
				if (frame->payload && frame->payloadLen >= 2)
				{
					RsslUInt16 sc = 0;
					rwfGet16(sc, frame->payload);
					if (sc < RWS_CFSC_UNKNOWN_15)
						snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
								"WS Code %d %s", sc, _getClosedText((rwsCFStatusCodes_t)sc));
				}

				/*  The peer is closing the session */
				_DEBUG_TRACE_WS_READ("Rcvd WS_CLOSE fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)
				if (!wsSess->sentClose)
				{
					/* Must unlock first as the rwsSendWsClose() always acquires the lock */
					IPC_MUTEX_UNLOCK(rsslSocketChannel);	
						
					if (rwsSendWsClose(rsslSocketChannel, RWS_CFSC_ENDPOINT_GONE, error) < 0)
					{
						snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
												"<%s():%d> Failed sending WS Close Frame ", __FUNCTION__,__LINE__);
					}

					IPC_MUTEX_UNLOCK(rsslSocketChannel);	
				}

				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				return(RSSL_RET_FAILURE);
			}
			else if (frame->opcode == RWS_OPC_PING)
			{
				/* Move inputBuffer->length and inputBufCursor past the WebSocket
				 * protocol message */
				_DEBUG_TRACE_WS_READ("Rcvd WS_PONG fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)
				if (rwsSendWsPong(rsslSocketChannel, NULL, error) < 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
											"<%s():%d> Failed sending WS Pong Frame ", __FUNCTION__,__LINE__);
					return(RSSL_RET_FAILURE);
				}
				
				return(totalCC);
			}
			else if (frame->opcode == RWS_OPC_PONG)
			{
				/* Move inputBuffer->length and inputBufCursor past the WebSocket
				 * protocol message */
				_DEBUG_TRACE_WS_READ("Rcvd WS_PONG fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)
				snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
							"<%s:%d> received WS_PONG frame", __FUNCTION__,__LINE__);

				return(totalCC);
			}
			
			_DEBUG_TRACE_WS_READ("Read WS Msg fd "SOCKET_PRINT_TYPE" cur %u hdrl %u pll %u pl %p br %u inBL %u inBC %u\n", 
												rsslSocketChannel->stream,
												frame->cursor,
												frame->hdrLen,
												frame->payloadLen,
												frame->payload,
												bytesRead,
												rsslSocketChannel->inputBuffer->length,
												rsslSocketChannel->inputBufCursor
												)

			/* Since this is an abstraction for the SOCKET readTransport, the calling function
			 * will update ->inputBuffer->length per the number of bytes returned.  Here we
			 * will only account for consuming the WS frame header size and also reflect this
			 * within ->inputBufCursor and bytesReas as well. To enable the calling layer to
			 * determine the correct number of bytes read and the correct byte offset, the total
			 * bytes read for WebSocket headers in the inputBuffer is held in he variable,
			 * rsslSocketChannel->inputBufProtOffset */

			/*
			 * x----x----------------------------------------x
			 *                                           -->--^
			 * move ->inputReadCursor past the WS frame header and payload in case
			 * there are more frames in inputBuffer  */
			wsSess->inputReadCursor += frame->hdrLen + frame->payloadLen;

			/* if there isn't any mode bytes past ->actualBytesRead then
			 * reset for the next time this is called  */
			if (wsSess->inputReadCursor == wsSess->actualInBuffLen)
			{
				wsSess->inputReadCursor = 0;
				wsSess->actualInBuffLen = 0;
			}
		
			bytesRead = (RsslInt32)(frame->hdrLen + frame->payloadLen);

			_DEBUG_TRACE_WS_READ("UPDATED  fd "SOCKET_PRINT_TYPE" cur %u hdrl %u pll %u pl %p br %u inBL %u inBC %u\n", 
												rsslSocketChannel->stream,
												frame->cursor,
												frame->hdrLen,
												frame->payloadLen,
												frame->payload,
												bytesRead,
												rsslSocketChannel->inputBuffer->length,
												rsslSocketChannel->inputBufCursor
												)


			return(totalCC);

			break;
		}

		default:
			snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
						"<%s:%d> Unexpected WebSocket OpCode: (%d)", __FILE__,__LINE__,frame->opcode);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			return(RSSL_RET_FAILURE);
			break;
		}
	}
	else
	{
		/* Read more data to parse the WS header properly */
		return(RSSL_RET_READ_WOULD_BLOCK);
	}

	return totalCC;
}


/* This will consume the WS header, populate the rws session struct and set the inputBuffer
 * pointer to the  beginning of its payload */
RsslInt32 rwsReadPrependTransportHdr(void* transport, char* buffer, int bufferLen, ripcRWFlags rwflags, int* ret, RsslError* error)
{
	int cc;
	RsslSocketChannel	*rsslSocketChannel = (RsslSocketChannel *)transport;

	rwflags |= RIPC_RW_WAITALL;

	cc = rwsReadTransportMsg((void*)rsslSocketChannel, buffer,
		bufferLen, rwflags, error);

	if (cc >= 0)
	{
		rsslSocketChannel->inputBuffer->length += cc;

		*ret = RSSL_RET_SUCCESS;

		return cc;
	}
	else 
	{
		*ret = cc; /* Expects either RSSL_RET_FAILURE(-1) or RSSL_RET_READ_WOULD_BLOCK(-11)*/

		return RSSL_RET_FAILURE;
	}
}

RsslInt32 rwsAdditionalHeaderLength()
{
	return RWS_MAX_HEADER_SIZE;
}

/*********************************************
 *	The following defines the functions need
 *	for writing ipc messages
 *********************************************/

/*********************************************
 *  This use of values for writing.
 *  startWrt - A buffer's current write position - next position
 *            to be given out for writing for buffer.
 *  buffer - Pointer to real memory beginning.
 *  outputBufLength - Total number of bytes to be written out.
 *********************************************/

RsslRet rwsWriteWebSocket(RsslSocketChannel *rsslSocketChannel, rsslBufferImpl *rsslBufImpl, 
						RsslInt32 wFlags, RsslInt32 *bytesWritten, RsslInt32 *uncompBytesWritten, 
						RsslInt32 forceFlush, RsslError *error)
{
	RsslRet			retval = RSSL_RET_SUCCESS;
	rwsSession_t *wsSess;
	int	i = 0;
	int uncompBytes = 0;
	int retVal = 0;
	rtr_msgb_t		*ripcBuffer = NULL;
	rtr_msgb_t		*msgb = NULL;
	RsslQueueLink	*pLink = 0;
	int	hdrlen = 0;
	rwsOpCodes_t opCode = RWS_OPC_NONE;

	if (IPC_NULL_PTR(rsslSocketChannel, "rwsWriteWebSocket", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	_DEBUG_TRACE_WS_WRITE("fd "SOCKET_PRINT_TYPE" bImp len %d pOfset %d totLn %u\n",
							rsslSocketChannel->stream, rsslBufImpl->buffer.length, 
							rsslBufImpl->packingOffset, rsslBufImpl->totalLength)

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rwsWriteWebSocket", "wsSess", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	rsslSocketChannel->bytesOutLastMsg = 0;

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s():%d> failed due to session shutdown.", __FUNCTION__,__LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	if (rsslBufImpl->bufferInfo == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 failed due the buffer has been released.\n",
				__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	/* Set fin bit according to the fragmentation flag */
	if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_NONE)
	{
		wsSess->finBit = RSSL_TRUE;
	}
	else if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER)
	{
		wsSess->finBit = RSSL_FALSE;
	}
	else if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_SUBSEQ_FRAG_HEADER)
	{
		wsSess->finBit = RSSL_FALSE;
		opCode = RWS_OPC_CONT;
	}
	else if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_LAST_FRAG_HEADER)
	{
		wsSess->finBit = RSSL_TRUE;
		opCode = RWS_OPC_CONT;
	}

	msgb = ((rtr_msgb_t*)rsslBufImpl->bufferInfo);
	rsslSocketChannel->bytesOutLastMsg = 0;

	if(msgb->length > 0)
	{
		if (wsSess->comp.outCompression == RSSL_COMP_ZLIB && wsSess->deflate)
		{
			ripcCompBuffer	compBuf;
			rtr_msgb_t    *compressedmb1;

			IPC_MUTEX_UNLOCK(rsslSocketChannel);

			compressedmb1 = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, msgb->length+RWS_MAX_HEADER_SIZE,  error);

			IPC_MUTEX_LOCK(rsslSocketChannel);

			if ( compressedmb1 == 0 )
			{
				/* Getting a buffer failed, override text and return error */
				snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
									"<%s:%d> Failed to get temp buffer for outbound WS frame buffer",
									__FUNCTION__,__LINE__);
				
				IPC_MUTEX_UNLOCK(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
			/* Don't need the extra bytes reserved for message lengths
			 * as it's already included in the payload
			 */

			compBuf.next_in = msgb->buffer;
			compBuf.avail_in = (RsslUInt32)msgb->length;
			compBuf.next_out = compressedmb1->buffer;
			compBuf.avail_out = (unsigned long)(compressedmb1->maxLength - compressedmb1->protocolHdr);
			  
			if ( (*(wsSess->comp.outCompFuncs->compress)) (wsSess->comp.c_stream_out,&compBuf, 
						(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT) ? 1 : 0,
						error) < 0)
			{
				/* Release the compressedbuffer here */
				compressedmb1->protocolHdr = 0;
				rtr_dfltcFreeMsg(compressedmb1);

				IPC_MUTEX_UNLOCK(rsslSocketChannel);
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
								"<%s:%d> Compression failed for outbound WS frame packet ", 
								__FUNCTION__,__LINE__);

				return RSSL_RET_FAILURE;
			}

			compressedmb1->length = compBuf.bytes_out_used;

			if (compBuf.avail_out == 0)
			{
				rtr_msgb_t    *compressedmb2 = 0;
				uncompBytes = compBuf.bytes_in_used;

				IPC_MUTEX_UNLOCK(rsslSocketChannel);
				compressedmb2 = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, rsslSocketChannel->maxUserMsgSize, error);
				IPC_MUTEX_LOCK(rsslSocketChannel);

				if ( compressedmb2 == 0 )
				{
					/* Getting a buffer failed, override text and return error */
					snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
								"<%s:%d> Failed to get additional temp buffer for compressed outbound WS frame",
								__FUNCTION__,__LINE__);
					
					IPC_MUTEX_UNLOCK(rsslSocketChannel);

					return RSSL_RET_FAILURE;
				}
				/* Don't need to reserve the space for the header or lengths
				*/
				/* This is for the data length header, but now 
					* ->protocolHdr is the entire WS header fix */
				compressedmb2->buffer -= compressedmb2->protocolHdr;
				compressedmb2->protocolHdr = 0;

				compBuf.next_out = compressedmb2->buffer;
				compBuf.avail_out = (unsigned long)compressedmb2->maxLength;

				if ( (*(wsSess->comp.outCompFuncs->compress)) (wsSess->comp.c_stream_out,&compBuf, 
							0, /* This buffer is for holding the remainder of the compressed message. Don't reset context. */
							error) < 0 ||
						compBuf.avail_out == 0)
				{
					/* Release the compressedbuffer here */
						
					rtr_dfltcFreeMsg(compressedmb2);

					IPC_MUTEX_UNLOCK(rsslSocketChannel);

					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT, 
								"<%s:%d> Compression failed for outbound WS frame", __FUNCTION__,__LINE__);
					return RSSL_RET_FAILURE;
				}

				// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec 
				compressedmb2->length = compBuf.bytes_out_used - 4;
					
				// Add the header as this is included if not compressed
				uncompBytes +=  compBuf.bytes_in_used;

				hdrlen = rwsWriteWsHdr(compressedmb1, compressedmb2, wsSess, 1, opCode);
				_DEBUG_TRACE_WS_WRITE("uncompB %d hdrLen %d", uncompBytes, hdrlen)
				*uncompBytesWritten = uncompBytes + hdrlen;

				retval = rwsWriteAndFlush(rsslSocketChannel, compressedmb1, &forceFlush, error);
				retval = rwsWriteAndFlush(rsslSocketChannel, compressedmb2, &forceFlush, error);
			}
			else
			{
				// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec
				compressedmb1->length -= 4;
				// Add the header as this is included if not compressed
				hdrlen = rwsWriteWsHdr(compressedmb1, 0, wsSess, 1, opCode);
				_DEBUG_TRACE_WS_WRITE("b_in_used %d hdrLen %d", compBuf.bytes_in_used, hdrlen)
				*uncompBytesWritten = compBuf.bytes_in_used + hdrlen;

				retval = rwsWriteAndFlush(rsslSocketChannel, compressedmb1, &forceFlush, error);
			}

			/* Release the original message */
			rtr_dfltcFreeMsg(msgb);
			msgb->buffer = 0;
			msgb->length = 0;
		}
		else
		{
			rwsWriteWsHdr(msgb, 0, wsSess, 0, opCode);
			*uncompBytesWritten = (RsslInt32)msgb->length;
			retval = rwsWriteAndFlush(rsslSocketChannel, msgb, &forceFlush, error);
		}

	} /* if(buf->length > 0) */
	else
	{
		if (msgb->maxLength > rsslSocketChannel->maxMsgSize)
			rwsReleaseLargeBuffer(rsslSocketChannel, msgb);
		else
		{
			rtr_dfltcFreeMsg(msgb);
			msgb->buffer = 0;
			msgb->length = 0;
		}
	}

	*bytesWritten = rsslSocketChannel->bytesOutLastMsg;

	if (retval != RSSL_RET_FAILURE)
	{
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
			retval += rsslSocketChannel->priorityQueues[i].queueLength;

		if (retval > (RsslInt32)rsslSocketChannel->high_water_mark)
		{
			retval = ipcFlushSession(rsslSocketChannel, error);
		}
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return (retval);
}

RsslInt32 rwsWriteAndFlush(RsslSocketChannel *rsslSocketChannel, rtr_msgb_t *msgb, int *forceFlush, RsslError *error)
{
	rtr_msgb_t		*lastmb;

	/* Enforces with the highest priority */
	msgb->priority = 0;

	msgb->local = msgb->buffer;
	if (*forceFlush == 1)
   	{
   		int cc = 0;
   		int i = 0;
   		int queuedBytesToWrite = 0;
		ripcRWFlags			rwflags = 0;

   		rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

   		msgb->local = msgb->buffer;

   		/* the user wants to force a flush - check if the output buffers have any data */
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		{
			if ((queuedBytesToWrite = rsslSocketChannel->priorityQueues[i].queueLength))
				break;
		}
		if (!queuedBytesToWrite)
   		{
			_DEBUG_TRACE_WS_WRITE("Writing msgb %p ->buffer %p ->local %p ->length %d\n", 
				msgb, msgb->buffer, msgb->local, msgb->length, forceFlush)
   			/* pass the buffer directly to write and free the buffer */
   			cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo,msgb->local,(RsslInt32)msgb->length,rwflags, error);

   			/* normal processing */
   			if (cc == msgb->length)
   			{
   				/* if write succeeds */
   				rsslSocketChannel->bytesOutLastMsg += (RsslInt32)msgb->length;
	
   				/* entire thing was written */
				if (msgb->maxLength > rsslSocketChannel->maxMsgSize)
					rwsReleaseLargeBuffer(rsslSocketChannel, msgb);
				else
				{
					if (msgb->pool == rsslSocketChannel->gblInputBufs)
					{
						rtr_smplcFreeMsg(msgb);
					}
					else
					{
						rtr_dfltcFreeMsg(msgb);
					}

					msgb->buffer = 0;
					msgb->length = 0;
				}
			}
   			else if (cc < 0)
   			{
   				/* if write fails, we need to queue the buffer up like normal, 
   				 * but try to call flush when we finish write */
   				*forceFlush = 2;
   			}
   			else
   			{
   			  	/* cc is somewhere between nothing and our size
   				 * update the local pointer, and put buffer in queue */
   				msgb->local = (caddr_t)msgb->local + cc;
				rsslSocketChannel->bytesOutLastMsg += cc;

   				*forceFlush = 2;
   			}
   		}
   		else
   		{
   			/* there is something to flush out of the queue */
			*forceFlush = 2;
   		}
   	}

   	if ((*forceFlush == 0) || (*forceFlush == 2))
   	{
		RsslQueueLink	*pLink = 0;

		/* figure out which priority list to put this in */
		rsslSocketChannel->priorityQueues[msgb->priority].queueLength += (RsslInt32)msgb->length;

		pLink = rsslQueuePeekBack(&rsslSocketChannel->priorityQueues[msgb->priority].priorityQueue);

		/* since there are dblks - actual memory block, and mblks - pointers
		into dblks, it is possible that there are several mblks sharing
		the same dblk.  If this is the case, there is possibly a pointer
		to earlier in this dblk in this output queue.  This checks for that.
		If it is the case, we update the length of the block (to avoid adding
		another pointer to the same block).  If its not the
		case, we put the block in the list */
		if (pLink && (lastmb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink)) && ((lastmb->buffer + lastmb->length) == msgb->buffer))
		{
			lastmb->length += msgb->length;
			lastmb->maxLength += msgb->length;

			rsslSocketChannel->bytesOutLastMsg += (RsslUInt32)(msgb->length);

			if (msgb->maxLength > rsslSocketChannel->maxMsgSize)
				rwsReleaseLargeBuffer(rsslSocketChannel, msgb);
			else
			{
				if (msgb->pool == rsslSocketChannel->gblInputBufs)
				{
					rtr_smplcFreeMsg(msgb);
				}
				else
				{
					rtr_dfltcFreeMsg(msgb);
				}

				msgb->buffer = 0;
				msgb->length = 0;
			}
		}
		else
		{
			/* figure out which priority list to put this in */
			_DEBUG_TRACE_WS_WRITE("#2 Queuing %u bytes (forceFlush=%u, queueLength=%u)\n", msgb->length, *forceFlush, rsslSocketChannel->priorityQueues[msgb->priority].queueLength)

			rsslQueueAddLinkToBack(&(rsslSocketChannel->priorityQueues[msgb->priority].priorityQueue), &(msgb->link));
			rsslSocketChannel->bytesOutLastMsg += (RsslUInt32)(msgb->length);
		}
   	}

	return rsslSocketChannel->priorityQueues[0].queueLength;
}

RsslUInt8 rwsGetWsHdrSize(RsslUInt64 dataLen, RsslInt32 clientSession)
{
	RsslInt32 hdrLen = (clientSession ? _WS_MASK_KEY_FIELD_LEN : 0); /* + [ 0 | 4 ] */

	if ( dataLen < 126)
		hdrLen += _WS_MIN_HEADER_LEN; /* 2 */
	else if ( dataLen <= 65535 )
		hdrLen += _WS_126_HEADER_LEN; /* 2 + 2 */
	else
		hdrLen += _WS_127_HEADER_LEN; /* 2 + 8 */
		
	return hdrLen; 
}

RsslUInt8 rwsWriteWsHdrBuffer(char * buffer, RsslUInt64 dataLen, rwsSession_t *wsSess, RsslBool finBit, RsslInt32 compressed, rwsOpCodes_t opcode)
{
	char *ptrHdr;
	RsslInt32 hdrLen = 0;
	RsslUInt16 u16val = 0;
	RsslUInt8 plHdrLen = 0;
	RsslInt32 maskLen = 0;

	maskLen = (wsSess->isClient ? _WS_MASK_KEY_FIELD_LEN : 0);

	if ( dataLen < 126)
		plHdrLen = _WS_MIN_HEADER_LEN; /* 2 */
	else if ( dataLen <= 65535 )
		plHdrLen = _WS_126_HEADER_LEN; /* 2 + 2 */
	else
		plHdrLen = _WS_127_HEADER_LEN; /* 2 + 8 */
		
	hdrLen = plHdrLen + maskLen; /* + [ 0 | 4 ] */

	ptrHdr = (char*)&(buffer[0]);

	memset(ptrHdr, 0, hdrLen);

	/*  0                   1                   2                   3
	 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 * +-+-+-+-+-------+-+-------------+-------------------------------+
	 * |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	 * |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	 * |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	 * | |1|2|3|       |K|             |                               |
	 * +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - + */
	/* ALso setting FIN bit to indicate end of fragment. */

	/* Set FIN */
	if (finBit)
		rwfSetBit(ptrHdr, 7);

	/* Set RSV1 */
	if (compressed)
		rwfSetBit(ptrHdr, 6);

	if (opcode != RWS_OPC_NONE)
		/* set opcode for arg opcode frame */
		ptrHdr[0] |= (opcode & 0x0F);
	else if (wsSess->protocol == RWS_SP_JSON2)
		/* set opcode for text frame */
		ptrHdr[0] |= (RWS_OPC_TEXT & 0x0F);
	else
		/* set opcode for binary frame */
		ptrHdr[0] |= (RWS_OPC_BINARY & 0x0F);

	/* Populate the WS payload length field or 
	 * the Extended payload length(126|127) */
	switch (plHdrLen)
	{
	case _WS_126_HEADER_LEN:
	{
		*(ptrHdr + _WS_PAYLOAD_LEN_OFFSET) = 126;
		u16val = (RsslUInt16)dataLen;
		rwfPut16((ptrHdr + _WS_EXTENDED_PAYLOAD_OFFSET), u16val);
		break;
	}
	case _WS_127_HEADER_LEN:
	{
		*(ptrHdr + _WS_PAYLOAD_LEN_OFFSET) = 127;
		rwfPut64((ptrHdr + _WS_EXTENDED_PAYLOAD_OFFSET), dataLen);
		break;
	}
	// if the case _WS_MIN_HEADER_LEN:
	default:
	{
		*(ptrHdr + _WS_PAYLOAD_LEN_OFFSET) = (RsslUInt8)dataLen;
		break;
	}
	}

	/* The payload data for all Client to Server frames are masked */
	if (wsSess->isClient)
	{
		char mask[4];
		RsslUInt32 maskVal = randu32();
		RsslUInt16 maskOffset = (hdrLen - maskLen);

		rwfSetBit((ptrHdr+1), 7);
		rwfPut32((ptrHdr + maskOffset), maskVal);

		_DEBUG_TRACE_WS_MASK((ptrHdr + maskOffset))

		memset (mask, 0, 4);
		_setMaskKeyBuff(mask, maskVal);
		_maskDataBlock(mask, (ptrHdr + hdrLen), dataLen); 	
	}
	_DEBUG_TRACE_WS_FRAME(ptrHdr)

	return hdrLen;
}

/* This will write WS header within _MAX_HEADER suffix reserved at the beginning of the buffer.
 * So, first set the pointer from a right justify start from the payload data buffer pointer */
RsslUInt8 rwsWriteWsHdr(rtr_msgb_t *msgb, rtr_msgb_t *msgb2, rwsSession_t *wsSess, RsslInt32 compressed, rwsOpCodes_t opCode)
{
	RsslUInt8 hdrLen = 0;
	RsslUInt16 u16val = 0;
	RsslUInt8 plHdrLen = 0;
	RsslInt32 maskLen = 0;
	RsslUInt64 	dataLen;

	/* Need to calculate the WS Frame Header length in order to move
	 * the ->buffer pointer back enough to populate the WS header fields */
	maskLen = (wsSess->isClient ? _WS_MASK_KEY_FIELD_LEN : 0);

	dataLen = (msgb2 ? msgb->length + msgb2->length : msgb->length);

	if ( dataLen < 126)
		plHdrLen = _WS_MIN_HEADER_LEN; /* 2 */
	else if ( dataLen <= 65535 )
		plHdrLen = _WS_126_HEADER_LEN; /* 2 + 2 */
	else
		plHdrLen = _WS_127_HEADER_LEN; /* 2 + 8 */
		
	hdrLen = plHdrLen + maskLen; /* + [ 0 | 4 ] */

	msgb->buffer -= hdrLen;
	memset(msgb->buffer, 0, hdrLen);

	hdrLen = rwsWriteWsHdrBuffer(msgb->buffer, msgb->length, wsSess, wsSess->finBit, compressed, opCode);

	msgb->protocolHdrLength = hdrLen;
	msgb->protocolHdr -= msgb->protocolHdrLength;
	msgb->length += msgb->protocolHdrLength;

	_DEBUG_TRACE_BUFFER("Populated WS Buffer msgb %p ->buf %p ->prot %d ->ln %u ->mxln %u\n", 
															(msgb ? msgb:0), 
															(msgb ? msgb->buffer:0),
															(msgb ? msgb->protocolHdr:0),
															(msgb ? msgb->length:0),
															(msgb ? msgb->maxLength:0))
#ifdef _DISABLE_DEBUG_WRITE
	{
		int	i = 0;
		unsigned char *ptr;

		ptr = msgb->buffer;
		for (i = 0;i < msgb->length ; i++, ptr++)
		{
		 	printf("0x%02x ", *(unsigned char*)ptr);
		}
		printf("\n");
	}
#endif

	return hdrLen;
}

/* rwsPrependWsHdr has the assumption for the argument, rtr_msgb_t, was retrieved via a pool which
 * preallocated a prefix the size of buf->protocolHdr = _WS_MAX_HEADER_LEN, 
 * buf->buffer points to the start of the * payload data, and 
 * buf->length is the length of the current payload data. */
RsslInt32 rwsPrependWsHdr(void *transport, rtr_msgb_t *msgb, RsslError *error)
{
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel *)transport;
	rwsSession_t *wsSess;

	if (IPC_NULL_PTR(rsslSocketChannel, "", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	_DEBUG_TRACE_WS_WRITE("fd "SOCKET_PRINT_TYPE" msgb  protocol %d length %d \n", 
					rsslSocketChannel->stream, msgb->protocolHdr, msgb->length)

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	wsSess->finBit = RSSL_TRUE;

	rwsWriteWsHdr(msgb, 0, wsSess, 0, RWS_OPC_NONE);

	return ((RsslInt32)msgb->length);
}

RsslInt32 rwsSendPingData(RsslSocketChannel* rsslSocketChannel, RsslBuffer *pingData, RsslError *error)
{
  	RsslInt32	forceFlush = 1;
	rtr_msgb_t  *msgb;
	rwsSession_t *wsSess;

	if (IPC_NULL_PTR(rsslSocketChannel, "rwsSendPingData", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rwsSendPingData", "wsSess", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	wsSess->finBit = RSSL_TRUE;

	if (wsSess->comp.outCompression == RSSL_COMP_ZLIB && wsSess->deflate)
	{
		ripcCompBuffer	compBuf;
		rtr_msgb_t    *compressedmb1;

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		compressedmb1 = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, pingData->length + RWS_MAX_HEADER_SIZE, error);

		IPC_MUTEX_LOCK(rsslSocketChannel);

		if (compressedmb1 == 0)
		{
			/* Getting a buffer failed, override text and return error */
			snprintf((error->text), MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Failed to get temp buffer for outbound WS frame buffer",
				__FUNCTION__, __LINE__);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		/* Don't need the extra bytes reserved for message lengths
		 * as it's already included in the payload
		 */

		compBuf.next_in = pingData->data;
		compBuf.avail_in = (RsslUInt32)pingData->length;
		compBuf.next_out = compressedmb1->buffer;
		compBuf.avail_out = (unsigned long)(compressedmb1->maxLength - compressedmb1->protocolHdr);

		if ((*(wsSess->comp.outCompFuncs->compress)) (wsSess->comp.c_stream_out, &compBuf,
			(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT ? 1 : 0),
			error) < 0)
		{
			/* Release the compressedbuffer here */
			compressedmb1->protocolHdr = 0;
			rtr_dfltcFreeMsg(compressedmb1);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Compression failed for outbound WS frame packet ",
				__FUNCTION__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		compressedmb1->length = compBuf.bytes_out_used;

		// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec
		compressedmb1->length -= 4;
		// Add the header as this is included if not compressed
		rwsWriteWsHdr(compressedmb1, 0, wsSess, 1, RWS_OPC_NONE);
		rwsWriteAndFlush(rsslSocketChannel, compressedmb1, &forceFlush, error);
	}
	else
	{
		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		msgb = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, pingData->length, error);

		IPC_MUTEX_LOCK(rsslSocketChannel);

		if (!msgb)
		{
			/* Getting a buffer failed, override text and return error */
			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			sprintf(error->text,
				"<%s:%d> failed, allocating buffer for rssl WS Ping data.  Output buffer may need to be flushed",
				__FUNCTION__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		if (pingData->length)
			memcpy(msgb->buffer, pingData->data, pingData->length);

		msgb->length = pingData->length;

		rwsWriteWsHdr(msgb, 0, wsSess, wsSess->deflate, RWS_OPC_NONE);

		rwsWriteAndFlush(rsslSocketChannel, msgb, &forceFlush, error);
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return rsslSocketChannel->priorityQueues[0].queueLength;
}

RsslInt32 rwsSendWsPing(RsslSocketChannel* rsslSocketChannel, RsslBuffer *pingData, RsslError *error)
{
  	RsslInt32 retVal = 0, i = 0, payloadLen = 0, bufLen = 0;
  	RsslInt32 forceFlush = 1;
	rtr_msgb_t  	*msgb;
	rwsSession_t *wsSess;

	if (IPC_NULL_PTR(rsslSocketChannel, "rwsSendWsPing", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rwsSendWsPing", "wsSess", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	if (pingData)
		payloadLen += (pingData->length < _WS_FLAG_2BYTE_EXT_PAYLOAD ? 
										  pingData->length: 
										  _WS_FLAG_2BYTE_EXT_PAYLOAD-1);

	bufLen = payloadLen;

	msgb = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, bufLen, error);
	
	if (!msgb)
	{
		/* Getting a buffer failed, override text and return error */
		sprintf(error->text, 
		"<%s:%d> failed, allocating buffer for WS ping.  Output buffer may need to be flushed", 
		__FUNCTION__, __LINE__);
		return -1;

	}
	IPC_MUTEX_LOCK(rsslSocketChannel);

	wsSess->finBit = RSSL_TRUE;

	memset(msgb->buffer, 0, bufLen);
	if (payloadLen)
		memcpy(msgb->buffer, pingData->data, payloadLen);

	msgb->length = bufLen;

	rwsWriteWsHdr(msgb, 0, wsSess, 0, RWS_OPC_PING);

	rwsWriteAndFlush(rsslSocketChannel, msgb, &forceFlush, error); 	

	return rsslSocketChannel->priorityQueues[0].queueLength;
}

RsslInt32 rwsSendWsPong(RsslSocketChannel* rsslSocketChannel, RsslBuffer *pongData, RsslError *error)
{
  	RsslInt32	retVal = 0, i = 0, payloadLen = 0, bufLen = 0;
  	RsslInt32 forceFlush = 1;
	rtr_msgb_t  *msgb;
	rwsSession_t *wsSess;

	if (IPC_NULL_PTR(rsslSocketChannel, "rwsSendWsPong", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rwsSendWsPong", "wsSess", error))
		return RSSL_RET_FAILURE;


	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	if (pongData)
		payloadLen += (pongData->length < _WS_FLAG_2BYTE_EXT_PAYLOAD ? 
										  pongData->length: 
										  _WS_FLAG_2BYTE_EXT_PAYLOAD-1);

	bufLen = payloadLen;
	msgb = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, bufLen, error);
	
	if (!msgb)
	{
		/* Getting a buffer failed, override text and return error */
		sprintf(error->text, 
			"<%s:%d> failed, allocating buffer for WS pong.  Output buffer may need to be flushed", 
			__FUNCTION__, __LINE__);
		return -1;

	}
	IPC_MUTEX_LOCK(rsslSocketChannel);

	wsSess->finBit = RSSL_TRUE;

	memset(msgb->buffer, 0, bufLen);
	if (payloadLen)
		memcpy(msgb->buffer, pongData->data, payloadLen);

	msgb->length = bufLen;

	rwsWriteWsHdr(msgb, 0, wsSess, 0, RWS_OPC_PONG);

	rwsWriteAndFlush(rsslSocketChannel, msgb, &forceFlush, error); 	

	return rsslSocketChannel->priorityQueues[0].queueLength;
}

RsslInt32 rwsSendWsClose(RsslSocketChannel* rsslSocketChannel, rwsCFStatusCodes_t code, RsslError *error)
{
  	RsslInt32	retVal = 0, i = 0, payloadLen = 0, bufLen = 0;
  	RsslInt32 forceFlush = 1;
	RsslUInt16 statusCode = (RsslUInt16)code;
	const char *closeText = 0;
	RsslInt32	closeTextLen = 0;
	rtr_msgb_t  *msgb;
	rwsSession_t *wsSess;

	if (IPC_NULL_PTR(rsslSocketChannel, "rwsSendWsClose", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rwsSendWsClose", "wsSess", error))
		return RSSL_RET_FAILURE;

	closeText = _getClosedText(code);
	if(closeText)
		closeTextLen = (RsslInt32)strlen(closeText);

	/* per RFC6455 the close frame payload must start with a 2 byte, unsigned status code
	 * followed by optional payload no more than 125 bytes in length */
	if (closeText && closeTextLen > 0)
		payloadLen += (closeTextLen < _WS_MAX_CONTROL_FRAME - 2 ? closeTextLen : _WS_MAX_CONTROL_FRAME - 2);
	else
		payloadLen = 2;

	bufLen = 2 + payloadLen; /* 2 the size of close status */

	msgb = (rtr_msgb_t*)rwsDataBuffer(rsslSocketChannel, bufLen, error);

	if (!msgb)
	{
		/* Getting a buffer failed, override text and return error */
		sprintf(error->text, 
			"<%s:%d> failed, allocating buffer for WS close frame.  Output buffer may need to be flushed", 
			__FUNCTION__, __LINE__);
		return -1;
	}

	IPC_MUTEX_LOCK(rsslSocketChannel);

	wsSess->finBit = RSSL_TRUE;

	memset(msgb->buffer, 0, bufLen);

	rwfPut16((msgb->buffer), statusCode);
	if (closeText && closeTextLen > 0)
		memcpy((msgb->buffer + 2), closeText, payloadLen);

	msgb->length = bufLen;

	rwsWriteWsHdr(msgb, 0, wsSess, 0, RWS_OPC_CLOSE);

	rwsWriteAndFlush(rsslSocketChannel, msgb, &forceFlush, error);

	wsSess->sentClose = RSSL_TRUE;

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return rsslSocketChannel->priorityQueues[0].queueLength;
}

RsslInt32 URLdecode(char *encoded, RsslInt32 length, char *pDecode)
{
	char *pDecBeg;
	char *pEncode;
	char *pEnd=encoded+length;
	RsslInt32 Hi;
	RsslInt32 Lo;
	RsslInt32 Result;

	pDecBeg=pDecode;
	pEncode=encoded;

	while (pEncode < pEnd)
	{
		if (*pEncode == '+')
		{
			*pDecode++ = ' ';
			pEncode++;
		}
		else if (*pEncode == '%')
		{
			pEncode++;
			if (isxdigit(pEncode[0]) && (isxdigit(pEncode[1])))
			{
				Hi = pEncode[0];
				if ('0' <= Hi && Hi <= '9')
					Hi -= '0';
				else if ('a' <= Hi && Hi <= 'f')
					Hi -= ('a'-10);
				else if ('A' <= Hi && Hi <= 'F')
					Hi -= ('A'-10);

				Lo = pEncode[1];
				if ('0' <= Lo && Lo <= '9')
					Lo -= '0';
				else if ('a' <= Lo && Lo <= 'f')
					Lo -= ('a'-10);
				else if ('A' <= Lo && Lo <= 'F')
					Lo -= ('A'-10);

				Result = Lo + (16 * Hi);
				*pDecode++ = (char)Result;
				pEncode += 2;
			}
			else
				return(-1);
		}
		else
		{
			*pDecode++ = *pEncode;
			pEncode++;
		}
	}
	return (RsslInt32)(pDecode - pDecBeg);
}

RsslInt32 getIntValue(RsslInt32 *pos, RsslInt32 endOfLine, char *data)
{
	char val[50];
	RsslInt32 cnt = 0;
	val[0] = '\0';

	while ( ((*pos) <= endOfLine) && (cnt < 50) && isdigit(data[(*pos)]) )
	{
		val[cnt] = data[(*pos)];
		++cnt;++(*pos);
	}

	if (cnt == 0) 
	  return -1;

	val[cnt] = '\0';

	return atoi(val);
}

void clearFrameHeader(rwsFrameHdr_t * frame)
{
	memset(frame->buffer, 0, RWS_MAX_HEADER_SIZE);
	frame->pCtlHdr = 0;
	frame->pExtHdr = 0;
	frame->cursor = 0;
	frame->hdrLen = 0;
	frame->extHdrLen = 0;
	frame->partial = 0;
	frame->finSet = 0;
	frame->rsv1Set = 0;
	frame->rsv2Set = 0;
	frame->rsv3Set = 0;
	frame->opcode = 0;
	frame->dataType = 0;
	frame->control = 0;
	frame->fragment = 0;
	frame->compressed = 0;
	frame->maskSet = 0;
	memset(frame->mask, 0, RWS_MASK_KEY_LEN);
	frame->maskVal = 0;
	frame->payloadLen = 0;
	frame->payload = 0;
}

void rwsClearCompression(rwsComp_t *wsComp)
{
	wsComp->type = 0;
	wsComp->flags = 0;
	wsComp->compressFuncs = 0;
	wsComp->inDecompress = 0;
	wsComp->outCompression = 0;
	wsComp->inDecompFuncs = 0;
	wsComp->outCompFuncs = 0;
	wsComp->c_stream_in = 0;
	wsComp->c_stream_out = 0;
	wsComp->zlibLevel = 6;
	wsComp->decompressBuf = 0;
}

void rwsRelCompression(rwsComp_t *wsComp)
{
	if (wsComp)
	{
		if (wsComp->decompressBuf)
		{
			rtr_smplcFreeMsg(wsComp->decompressBuf);
			wsComp->decompressBuf = 0;
		}

		if (wsComp->c_stream_out && wsComp->outCompFuncs)
			(*(wsComp->outCompFuncs->compressEnd))(wsComp->c_stream_out);

		if (wsComp->c_stream_in && wsComp->inDecompFuncs)
			(*(wsComp->inDecompFuncs->decompressEnd))(wsComp->c_stream_in);

	}
}

void rwsClearCookies(rwsCookies_t *cookies)
{
	cookies->authToken.data = 0;
	cookies->authToken.length = 0;
	cookies->position.data = 0;
	cookies->position.length = 0;
	cookies->applicationId.data = 0;
	cookies->applicationId.length = 0;
}

void rwsRelCookies(rwsCookies_t *cookies)
{
	if (cookies)
	{
		if (cookies->authToken.data)
		{
			_rsslFree(cookies->authToken.data);
			cookies->authToken.data = 0;
			cookies->authToken.length = 0;
		}
		if (cookies->position.data)
		{
			_rsslFree(cookies->position.data);
			cookies->position.data = 0;
			cookies->position.length = 0;
		}
		if (cookies->applicationId.data)
		{
			_rsslFree(cookies->applicationId.data);
			cookies->applicationId.data = 0;
			cookies->applicationId.length = 0;
		}
	}
}

void rwsClearSession(rwsSession_t *wsSess)
{
	wsSess->server = 0;
	wsSess->actualInBuffLen = 0;
	wsSess->inputReadCursor = 0;
	wsSess->hsReceived.total = 0;
	wsSess->hsReceived.lines = 0;
	wsSess->headerLineNum = 0;
	wsSess->statusCode = 0;
	wsSess->upgrade = 0;
	wsSess->connUpgrade = 0;
	wsSess->deflate = 0;
	wsSess->compressed = 0;
	wsSess->protocol = 0;
	wsSess->url = 0;
	wsSess->host = 0;
	wsSess->port = 0;
	wsSess->origin = 0;
	wsSess->userAgent = 0;
	wsSess->hostname = 0;
	wsSess->peerIP = 0;
	wsSess->protocolName = 0;
	wsSess->protocolList = 0;
	rwsClearCookies(&(wsSess->cookies)); 
	wsSess->keyRecv.data = 0;
	wsSess->keyRecv.length = 0;
	wsSess->keyAccept.data = 0;
	wsSess->keyAccept.length = 0;
	wsSess->keyNonce.data = 0;
	wsSess->keyNonce.length = 0;
	wsSess->versionRecv = 0;
	wsSess->version = RWS_PROTOCOL_VERSION;
	// end hs
	clearFrameHeader(&(wsSess->frameHdr));
	wsSess->mask = 0;
	memset(wsSess->maskFld, 0, _WS_MASK_KEY_FIELD_LEN);
	rwsClearCompression(&(wsSess->comp));
	wsSess->maxPayload = 0;
	wsSess->reassemblyBuffer = 0;
	wsSess->reassemblyUnfinished = 0;
	wsSess->reassemblyCompressed = 0;
	wsSess->recvGetReq = 0;
	wsSess->recvClose = 0;
	wsSess->sentClose = 0;
	wsSess->pingRecvd = 0;
	wsSess->sendPong = 0;
	wsSess->isClient = 0;
	wsSess->maxMsgSize = RSSL_MAX_JSON_MSG_SIZE;
}

rwsSession_t *rwsNewSession()
{
	rwsSession_t *session = 0;
	
	session = (rwsSession_t *)_rsslMalloc(sizeof(struct rwsSession));
	if (session)
		rwsClearSession(session);

    _DEBUG_TRACE_REF("rwsSess 0x%p *wsS 0x%p\n", session, *session)

	return session;
}

void rwsReleaseSession(rwsSession_t *wsSess)
{

    _DEBUG_TRACE_REF("rwsSess 0x%p *wsS 0x%p\n", wsSess, *wsSess)

	if (wsSess)
	{
		if (wsSess->hsReceived.total)
			_freeHttpHeader(&(wsSess->hsReceived));

		if (wsSess->reassemblyBuffer)
		{
			rtr_smplcFreeMsg(wsSess->reassemblyBuffer);
			wsSess->reassemblyBuffer = 0;
		}

		if (wsSess->url)
		{
			_rsslFree(wsSess->url);
			wsSess->url = 0;
		}
		if (wsSess->host)
		{
			_rsslFree(wsSess->host);
			wsSess->host = 0;
		}
		if (wsSess->origin)
		{
			_rsslFree(wsSess->origin);
			wsSess->origin = 0;
		}
		if (wsSess->userAgent)
		{
			_rsslFree(wsSess->userAgent);
			wsSess->userAgent = 0;
		}
		if (wsSess->hostname)
		{
			_rsslFree(wsSess->hostname);
			wsSess->hostname = 0;
		}
		if (wsSess->peerIP)
		{
			_rsslFree(wsSess->peerIP);
			wsSess->peerIP = 0;
		}
		if (wsSess->protocolName)
		{
			_rsslFree(wsSess->protocolName);
			wsSess->protocolName = 0;
		}
		if (wsSess->protocolList)
		{
			_rsslFree(wsSess->protocolList);
			wsSess->protocolList = 0;
		}

		rwsRelCompression(&(wsSess->comp));

		rwsRelCookies(&(wsSess->cookies));

		if (wsSess->keyRecv.data)
		{
			_rsslFree(wsSess->keyRecv.data);
			wsSess->keyRecv.data = 0;
			wsSess->keyRecv.length = 0;
		}
		if (wsSess->keyAccept.data)
		{
			_rsslFree(wsSess->keyAccept.data);
			wsSess->keyAccept.data = 0;
			wsSess->keyAccept.length = 0;
		}
		if (wsSess->keyNonce.data)
		{
			_rsslFree(wsSess->keyNonce.data);
			wsSess->keyNonce.data = 0;
			wsSess->keyNonce.length = 0;
		}
	}
}

void rwsClearServer(rwsServer_t *wsSrvr)
{
	wsSrvr->protocolList = 0;
	wsSrvr->zlibCompLevel = 6;
	wsSrvr->compressionSupported = 0;
	rwsClearCookies(&(wsSrvr->cookies)); 
	wsSrvr->version = RWS_PROTOCOL_VERSION;
}

rwsServer_t *rwsNewServer()
{
	rwsServer_t *server = 0;
	
	server = (rwsServer_t *)_rsslMalloc(sizeof(struct rwsServer));
	if (server)
		rwsClearServer(server);

    _DEBUG_TRACE_REF("rwsServer 0x%p *wsS 0x%p\n", server, *server)

	return server;
}


void rwsReleaseServer(rwsServer_t *wsServ)
{
    _DEBUG_TRACE_REF("rwsServer 0x%p *wsS 0x%p\n", wsServ, *wsServ)
	if (wsServ)
	{
		if (wsServ->protocolList)
		{
			_rsslFree(wsServ->protocolList);
			wsServ->protocolList = 0;
		}
		rwsRelCookies(&(wsServ->cookies));
	}
}

RsslRet rwsInitSessionOptions(RsslSocketChannel *rsslSocketChannel, RsslWSocketOpts *wsOpts, RsslError *error)
{
	rwsSession_t *wsSess;

	if (rsslSocketChannel->rwsSession == 0)
	{
		rsslSocketChannel->rwsSession = (void *)rwsNewSession();
		if (rsslSocketChannel->rwsSession == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Failed to allocate memory for rwsSession struct.", __FUNCTION__,__LINE__);
			return RSSL_RET_FAILURE;
		}
		_DEBUG_TRACE_INIT("rwsSession 0x%p *0x%p\n", 
								rsslSocketChannel->rwsSession, 
								*((rwsSession_t *)rsslSocketChannel->rwsSession))
	}

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	wsSess->maxMsgSize = wsOpts->maxMsgSize == 0 ? RSSL_MAX_JSON_MSG_SIZE : wsOpts->maxMsgSize;

	/* This will initialize the list of sub-protocol strings passed within the options struct 
	 * and be assigned to the WS session protocolList */
	if (wsOpts->protocols)
	{
		RsslBool depracateOldProtocols =  RSSL_FALSE;
		wsSess->protocolList = rwsSetSubProtocols((const char*)(wsOpts->protocols ?
																wsOpts->protocols :
																RWS_DEFAULT_SUBPROTOCOL), 
													depracateOldProtocols, error);

		_DEBUG_TRACE_INIT(" wsOpts->protocols : '%s'\nwsSess->protocolList : '%s'\n", wsOpts->protocols, wsSess->protocolList)
		if (wsSess->protocolList == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d>: Failed sub-protocols list init\n", __FUNCTION__, __LINE__);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rwsInitServerOptions(RsslServerSocketChannel *rsslServerSocketChannel, RsslWSocketOpts *wsOpts, RsslError *error)
{
	rwsServer_t *wsServer;

	if (rsslServerSocketChannel->rwsServer == 0)
	{
		rsslServerSocketChannel->rwsServer = (void *)rwsNewServer();
		if (rsslServerSocketChannel->rwsServer == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Failed to allocate memory for rwsServer struct.", __FUNCTION__,__LINE__);
			return RSSL_RET_FAILURE;
		}
		_DEBUG_TRACE_INIT("rwsServer 0x%p *0x%p\n", 
							rsslServerSocketChannel->rwsServer, 
							*((rwsServer_t *)rsslServerSocketChannel->rwsServer))
	}

	wsServer = (rwsServer_t*)rsslServerSocketChannel->rwsServer;

	/* This will initialize the list of sub-protocol strings passed within the options struct 
	 * and will use to determine what will be assigned to the WS session protocolList */
	{
		RsslBool depracateOldProtocols =  RSSL_FALSE;
		wsServer->protocolList = rwsSetSubProtocols((const char*)(wsOpts->protocols ?
																  wsOpts->protocols :
																  RWS_DEFAULT_SUBPROTOCOL_LIST), 
													depracateOldProtocols, error);
		if (wsServer->protocolList == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT,
					"<%s:%d>: Failed sub-protocols list init\n", __FUNCTION__, __LINE__);
			return RSSL_RET_FAILURE;
		}
	}

	if (rsslServerSocketChannel->compressionSupported)
	{
		if (rsslServerSocketChannel->compressionSupported != RSSL_COMP_ZLIB)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf((error->text), MAX_RSSL_ERROR_TEXT,
					"<%s:%d>: Error Invalid compression type configured for WebSocket connections\n", 
					__FUNCTION__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		wsServer->compressionSupported = rsslServerSocketChannel->compressionSupported;
		wsServer->zlibCompLevel = rsslServerSocketChannel->zlibCompressionLevel;
	}

	return RSSL_RET_SUCCESS;
}




