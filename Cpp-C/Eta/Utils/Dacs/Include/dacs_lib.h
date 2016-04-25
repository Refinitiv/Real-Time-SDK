/******************************************************************************
*******************************************************************************
*
*	Triarch Data Access Control System
*
*	Copyright (C) Reuters Limited 1993
*
*	DACS Library User Interface Include File
*
*       This Header is used to define all the Variables and Data Types
*       to interface with the Dacs Library.
*
*	Name:	        dacs_lib.h
*	Version:	4.0
*	Date:		August 10, 1993
*
*******************************************************************************
*
*   Notes
*   =====
*
*   Revision History
*   ================
*   4.0.0  08/10/93        TLL        Initial version created
*   4.1.0  09/09/94        TLL        Modified for 4.1
*   4.1.0  09/13/94        TLL        Modified to support C++
*   4.4.0  03/12/99        TLL        Added a new error value
*   5.3.0  11/15/06        TLL        Added Authorization Migration error
*   6.0.1  11/04/08        TLL        Added Multi-connect option
*
*******************************************************************************
******************************************************************************/

/******************************************************************************
* Do not allow the Dacs Library to be multiply included                       *
******************************************************************************/
#ifndef	__DACS_LIB__
#define	__DACS_LIB__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/******************************************************************************
* The following defines are used to signal the different types of return      *
* codes from a Dacs Library function call.                                    *
******************************************************************************/
#define DACS_SUCCESS                    0
#define DACS_DIFF                       1
#define DACS_FAILURE                   -1

/******************************************************************************
* The following defines are used to define the Type's given to the            *
* DACS_ClientStart() and DACS_ClientEnd() functions.                          *
******************************************************************************/
#define DACS_SINK                       1
#define DACS_SOURCE                     2

/******************************************************************************
* The following define codes are the types that can be given to the           *
* DACS_GetLock() function.                                                    *
******************************************************************************/
#define AND_PRODUCT_CODES              '&'
#define OR_PRODUCT_CODES               '|'

/******************************************************************************
* The following are the known types of Access locks and there Version.        *
******************************************************************************/
#ifndef	NO_COMPRESS
#define	NO_COMPRESS                     1
#endif

#ifndef	BYTE_COMPRES
#define	BYTE_COMPRESS                   2
#endif

#ifndef	DACS_STANDARD_400
#define	DACS_STANDARD_400               3
#endif

#ifndef	DACS_STANDARD_500
#define	DACS_STANDARD_500               4
#endif

/******************************************************************************
* The following values are returned from the DACS_HandleVariables() function  *
* when requesting the DACS_PERMISSION_STATE.                                  *
******************************************************************************/
#define DACS_IS_ENABLED  1
#define DACS_IS_DISABLED 2

/******************************************************************************
* The following structure is used to define the data type given to the        *
* dacsGetLock() function. The operator has changed from operator to           *
* Operator  if you use c++ to avoid the reserved word "operator" in c++.      *
******************************************************************************/
typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
	char            Operator;
#else
	char            operator;
#endif
	unsigned short  pc_listLen;
	unsigned long   pc_list[1];	/* In actual use this array is
					 * variable length defined */
}               PRODUCT_CODE_TYPE;

/*****************************************************************************
* The macro makes it possible for API developer to define a bigger array     *
* for PRODUCT_CODE_TYPE by calling malloc() with pe count, for example:	     *
* PRODUCT_CODE_TYPE *peList =											     *	
* (PRODUCT_CODE_TYPE*)malloc(PRODCUT_CODE_TYPE_SIZE(pe_count));              *
*****************************************************************************/
#define PRODUCT_CODE_TYPE_SIZE(numsrc) (sizeof(PRODUCT_CODE_TYPE) \
										- sizeof(unsigned long)	  \
										+ ((numsrc) * sizeof(unsigned long)))

/******************************************************************************
* The following Data Structure is used to define the information returned     *
* in the event of an error from a DACS Library function.                      *
******************************************************************************/
typedef struct {
	short           dacs_error;
	short           dacs_suberr;
}               DACS_ERROR_TYPE;

/******************************************************************************
* The following Data Structure is used to define the information that is      *
* passed to the DACS_CsLock() function. it is identical to that used in the   *
* UVP combine lock function.                                                  *
******************************************************************************/
#ifndef	UVP_LIB_H
typedef struct {
	int             server_type;
	char           *item_name;
	unsigned char  *access_lock;
	int             lockLen;
}               COMB_LOCK_TYPE;
#endif

/******************************************************************************
* The following Data Structure is used to define the information that is      *
* passed to the DACS_ItemVerify() function.                                   *
******************************************************************************/
typedef struct {
	unsigned char  *access_lock;
	int             lockLen;
}               ITEM_LOCK_TYPE;

/******************************************************************************
* The following Data Structure is returned by the DACS_GetServiceList()       *
* function, for returning the list of Service ID's attached to a parent QoS   *
* source.                                                                     *
*                                                                             *
* If there is no such Parent Service, then a NULL pointer is returned.        *
******************************************************************************/
typedef struct {
	int		server_type;
	int		priority;
}		DACS_SERVICE_ENUM;

typedef struct {
	int			iCount;
	DACS_SERVICE_ENUM	*tServiceEnum;
}		DACS_SERVICE_LIST;

/******************************************************************************
* The following Data Structure is returned by the DACS_GetQosValues()         *
* function, for returning the Name/Value pairs attached to a Service ID.      *
*                                                                             *
* If there is no such Service ID, then a NULL pointer is returned.            *
******************************************************************************/
#define	DACS_MAX_QOS_NAME	32
#define	DACS_MAX_QOS_VALUE	32

typedef struct {
	char			Name[DACS_MAX_QOS_NAME+1];
	char			Value[DACS_MAX_QOS_VALUE+1];
}		DACS_SERVICE_ATTR;

typedef struct {
	int			iCount;
	DACS_SERVICE_ATTR	*tServiceAttr;
}		DACS_SERVICE_QOS;

/******************************************************************************
* The following defines are the offsets in the callback array for the         *
* Dacs Function DACS_ClientStart().                                           *
******************************************************************************/
#define	LOGINRESULTPASS			0
#define	LOGINRESULTFAIL			1
#define	LOGOUTRESULTPASS		2
#define	FORCELOGOUT			3
#define	REVERIFYLOGIN			4
#define	RELOGINUSERPASS			5
#define	RELOGINUSERFAIL			6
#define	FORCEREPERMISSION		7
#define	ADD_FD				8
#define	DELETE_FD			9
#define	LAST_CALLBACK_FUNCTION		(DELETE_FD+1)

/******************************************************************************
* The following defines are the Log Action types that are allowed to be       *
* passed to the DACS_LogAction() function.                                    *
******************************************************************************/
#define	ITEM_STARTWATCH			1
#define	ITEM_STOPWATCH			2
#define	ITEM_ACCESS_DENIED		3
#define	LOGIN_SUCCESS			4
#define	LOGIN_DENIED			5
#define	LOGOUT_SUCCESS			6
#define	CLIENT_START			7
#define	CLIENT_STOP			8
#define	ITEM_CONTWATCH			9
#define	INSERT_STARTWATCH               10
#define	INSERT_ACCESS_DENIED            11
#define ITEM_SNAPSHOT                   12
#define ONDEMAND_EVENT                  13
#define ONDEMAND_LOGIN_EVENT            14
#define ONDEMAND_VS_EVENT		15
#define SOFTWARE_USAGE			16

/******************************************************************************
* The following Dacs Structure is used to define the information that is      *
* passed to the DACS_LogAction() function.                                    *
******************************************************************************/
typedef struct {
	unsigned short  source_type;
	char           *Item;
	unsigned long   LockLength;
	unsigned char  *LockPtr;
}               LOGDATA;

/******************************************************************************
* The following Dacs Structure is the link list extended structure used to    *
* extended item identification to usage data.                                 *
******************************************************************************/
typedef struct {
        const unsigned char   *pExtendedName;
        const void            *pExtendedData;
        unsigned int          iExtendedDataLen;
        void                  *pNextExtendedLogData;
}               LOGDATA_NEXT;


/******************************************************************************
* The following Dacs Structure is the extended usage information that is      *
* passed to the DACS_LogAction() function after the invocation of             *
* DACS_SpecialVariables(ENABLE_EXTENDED_LOGDATA) was performed.               *
******************************************************************************/
typedef struct {
	unsigned short  source_type;
	char           *Item;
	unsigned long   LockLength;
	unsigned char  *LockPtr;
        LOGDATA_NEXT   *pNextExtendedLogData;
}               LOGDATA_EX;

/******************************************************************************
* The following Dacs Structure is used by the DACS_GetLastFailure.            *
* The caller supplies the DACS_GetLastFailure() function a pointer to         *
* a DACS_FAILURE_INFO structure.                                              *
*                                                                             *
* Return:                                                                     *
* NULL     -   If the DACS_GetLastFailure() function failed.                  *
* !NULL    -   If the DACS_FAILURE_INFO contains valid information.           *
*                                                                             *
* The 24 DACS_SUB_FAILURE types are defined as follows:                       *
*                                                                             *
* 0  - User,        Exchange,    Subscribed                                   *
* 1  - Position,    Exchange,    Subscribed                                   * 
* 2  - Application, Exchange,    Subscribed                                   * 
* 3  - User,        Composite,   Subscribed                                   *
* 4  - Position,    Composite,   Subscribed                                   * 
* 5  - Application, Composite,   Subscribed                                   * 
* 6  - User,        Product,     Subscribed                                   *
* 7  - Position,    Product,     Subscribed                                   * 
* 8  - Application, Product,     Subscribed                                   * 
* 9  - User,        Specialist,  Subscribed                                   *
* 10 - Position,    Specialist,  Subscribed                                   * 
* 11 - Application, Sprcialist,  Subscribed                                   * 
* 12 - User,        Exchange,    UnSubscribed                                 *
* 13 - Position,    Exchange,    UnSubscribed                                 * 
* 14 - Application, Exchange,    UnSubscribed                                 * 
* 15 - User,        Composite,   UnSubscribed                                 *
* 16 - Position,    Composite,   UnSubscribed                                 * 
* 17 - Application, Composite,   UnSubscribed                                 * 
* 18 - User,        Product,     UnSubscribed                                 *
* 19 - Position,    Product,     UnSubscribed                                 * 
* 20 - Application, Product,     UnSubscribed                                 * 
* 21 - User,        Specialist,  UnSubscribed                                 *
* 22 - Position,    Specialist,  UnSubscribed                                 * 
* 23 - Application, Sprcialist,  UnSubscribed                                 * 
*                                                                             *
******************************************************************************/
#define	MAX_SUB_FAILURES	24
#define MAX_AIX_NAME		32

typedef struct {
	unsigned char	failed;
	char		Name[MAX_AIX_NAME+1];
} DACS_SUB_FAILURE;

typedef struct {
	DACS_SUB_FAILURE	sub_failure[MAX_SUB_FAILURES];
} DACS_FAILURE_INFO;

/******************************************************************************
* The following structure(s) are used to pass the subservice list information
* back to the caller for the DACS_UserSubservices() and DACS_PeToSubService()
* functions.
******************************************************************************/
#define DACS_SUBTYPE_PRODUCT 'P'
#define DACS_SUBTYPE_EXCHANGE 'E'
#define DACS_SUBTYPE_SPECIALIST 'S'
#define DACS_SUBTYPE_PCBE_PRODUCT 'p'
#define DACS_SUBTYPE_PCBE_EXCHANGE 'e'
#define DACS_SUBTYPE_PCBE_SPECIALIST 's'
#define DACS_SUBTYPE_UNKNOWN 'U'

typedef struct {
        char           value[MAX_AIX_NAME+1];
        char           valueType; 
	void          *pExtent;
}               DACS_SUBSERVICE_ELEMENT;

typedef struct {
        char           value[MAX_AIX_NAME+1];
}               DACS_SUBSERVICE_NAMES;

typedef struct {
        unsigned long  value;
        char           valueType;
        char           subServiceOperator;
        int            subServiceNamesCnt;
	DACS_SUBSERVICE_NAMES *subServiceNames;
	void          *pExtent;
}               DACS_PE_SUBSERVICE_ELEMENT;

/******************************************************************************
* This is the element type for each SBE/PSBE defined for the vendor service.
* This data is retrived using the 
******************************************************************************/
typedef struct {
	char	*pSubServiceName;
	char	valueType;
	char	*pSubServiceContent;
}               DACS_SBE_SUBSERVICE_ELEMENT;

/******************************************************************************
* This is the maximum length of the error string that may be returned from    *
* the DACS_ItemVerify() and LoginResultFail() DACS functions.                 *
******************************************************************************/
#define         MAX_DACS_ERROR_MESSAGE		256

/******************************************************************************
* This is the maximum length of the Variable Data buffer for the              *
* DACS_SpecialVariables() DACS functions.                                     *
******************************************************************************/
#define         MAX_DACS_VAR_LENGTH		257

/******************************************************************************
* The following defines are the Valid Variables that can be retrieved from    *
* a valid Handle using the DACS_HandleVariables() function.                   *
******************************************************************************/
#define     DACS_USER_STATE                 1
#define     DACS_USER_HANDLE                2
#define     DACS_PENDING_MESSAGE            3
#define     DACS_USER_NAME                  4
#define     DACS_POSITION_NAME              5
#define     DACS_APPLICATION_NAME           6
#define     DACS_USER_ID                    7
#define     DACS_USER_PROFILE               8
#define     DACS_POSITION_PROFILE           9
#define     DACS_APPLICATION_PROFILE        10
#define     DACS_DISPLAY_NAME               11
#define     DACS_PERMISSION_STATE           12
#define     DACS_CLIENT_HANDLE              13

/******************************************************************************
* The following defines are the Valid Types of Variables returned from        *
* the DACS_HandleVariables() function.                                        *
******************************************************************************/
#define		DACS_SIGNED_BYTE                1
#define		DACS_UNSIGNED_BYTE              2
#define		DACS_SIGNED_WORD                3
#define		DACS_UNSIGNED_WORD              4
#define		DACS_SIGNED_LONG                5
#define		DACS_UNSIGNED_LONG		6
#define		DACS_STRING			7
#define		DACS_BYTE_ARRAY			8

/******************************************************************************
* This is the sub-error list that is returned within the DACS_ERROR_TYPE      *
* Data Structure. This parameter is useful for determining the type of error  *
* rather then specific error.                                                 *
******************************************************************************/
#define         DACS_NO_ERROR                   0
#define         DACS_RESOURCES                  1
#define         DACS_FAULT                      2
#define         DACS_INVALID_PARAMETER          3

/******************************************************************************
* This is the error list for the types of errors that can be returned within  *
* the DACS_ERROR_TYPE Data Structure.                                         *
******************************************************************************/
#ifndef		DACS_NO_ERROR
#define		DACS_NO_ERROR			0
#endif
#define		DACS_MALLOC_GETLOCK		1
#define		DACS_INVALID_ERROR		2
#define		DACS_BAD_OPERATOR		3
#define		DACS_NOT_SORTED			4
#define		DACS_SMALL_BUFFER		5
#define		DACS_NO_LOCKS			6
#define		DACS_MALLOC_COMBLOCK	7
#define		DACS_MALLOC_INENT		8
#define		DACS_MALLOC_PCENT		9
#define		DACS_BAD_INPUT_LOCK		10
#define		DACS_CS_E_NET_DOWN		11
#define		DACS_CS_E_TIMEOUT		12
#define		DACS_E_ROOM				13
#define		DACS_E_BAD_USER			14
#define		DACS_E_SYS_ERR			15
#define		DACS_E_NO_DATA			16
#define		DACS_E_OVERFLOW			17
#define		DACS_E_IOERR			18
#define		DACS_E_INTR				19
#define		DACS_E_NO_SESSION		20
#define		DACS_E_ARG				21
#define		DACS_E_NO_RESOURCE		22
#define		DACS_E_BUF_SIZE			23
#define		DACS_E_CLI_EX			24
#define		DACS_E_SVR_EX			25
#define		DACS_E_SVR_THERE		26
#define		DACS_E_TOO_MNY_CLI		27
#define		DACS_E_NORMAL			28
#define		DACS_E_INVAL			29
#define		DACS_E_ISCONN			30
#define		DACS_E_SVR_UNKNOWN		31
#define		DACS_MALLOC_UINFO		32
#define		DACS_CLIENT_DOWN		33
#define		DACS_CLIENT_PENDING		34
#define		DACS_CLIENT_SEND_FAILURE	35
#define		DACS_NO_PROFILE_INFO		36
#define		DACS_MALLOC_USER_PROFILE	37
#define		DACS_MALLOC_POSITION_PROFILE	38
#define		DACS_MALLOC_APPLICATION_PROFILE	39
#define		DACS_FORCE_LOGOUT		40
#define		DACS_NO_SERVICE_TABLE		41
#define		DACS_NO_MAPPING_TABLE		42
#define		DACS_DENIED_USER_ACCESS		43
#define		DACS_DENIED_POSITION_ACCESS	44
#define		DACS_DENIED_APPLICATION_ACCESS	45
#define		DACS_DENIED_UNKNOWN_ACCESS	46
#define		DACS_DENIED_USER_ACCESS_FOR_SERVICE	47
#define		DACS_DENIED_POSITION_ACCESS_FOR_SERVICE	48
#define		DACS_DENIED_APPLICATION_ACCESS_FOR_SERVICE	49
#define		DACS_DENIED_ACCESS_FOR_SERVICE	50
#define		DACS_DENIED_ACCESS_FOR_MAP	51
#define		DACS_UNKNOWN_HANDLE		52
#define		DACS_NOT_A_DACS_LOCK		53
#define         DACS_NOT_ACCESS_LOCK            54
#define		DACS_MALLOC_ACC_KEY             55
#define         DACS_UNKNOWN_TYPE               56
#define         DACS_NO_PROCESSING              57
#define         DACS_NO_NM                      58
#define         DACS_CHANNEL_NOT_OPENED         59
#define         DACS_NOT_OWNER                  60
#define         DACS_NO_INIT_PROC               61
#define         DACS_CHANNEL_NOT_SRC_SIDE       62
#define         DACS_HISTORY_BUFFER_OVERFLOW    63
#define         DACS_INVALID_CHANNEL            64
#define         DACS_INVALID_LOCK               65
#define         DACS_MALLOC_LOCK_FAIL           66
#define         DACS_NOT_ENOUGH_LOCK_MEM        67
#define         DACS_MALLOC_NEWLOCKLIST         68
#define         DACS_CANNOT_GET_MUTEX           69
#define         DACS_INVALID_ARGUMENT           70
#define         DACS_NOT_ENABLED                71
#define         DACS_V4_COMBINE_NOT_ALLOWED     72
#define         DACS_AUTHORIZATION_MIGRATION    73

/******************************************************************************
*******************************************************************************
* The following function prototypes are for the DACS Library.                 *
*******************************************************************************
******************************************************************************/

#if defined(_WINDOWS)
#if !defined(__STDC__)
#define	UNDO_STDC
#define	__STDC__
#endif
#endif

/******************************************************************************
* Options that can be passed to the DACS-API in the multiConnect mode.
******************************************************************************/
#define MAX_DACS_OPTION_NAME 32
#define MAX_DACS_OPTION_VALUE 1024
typedef struct {
	char optionName[MAX_DACS_OPTION_NAME+1];
	char optionValue[MAX_DACS_OPTION_VALUE+1];
	void *pNext; /* Set to NULL to terminate list */
} DACS_OPTIONS;

/******************************************************************************
* Sink DACS Library functions                                                 *
******************************************************************************/
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
/* Make a connection to the DACS Daemon process */
extern int DACS_ClientStart(char *, int , int (*[LAST_CALLBACK_FUNCTION])(), DACS_ERROR_TYPE *);
extern int DACS_ClientStartMC(const char *ClientName, int ClientType, int (*[LAST_CALLBACK_FUNCTION])(), unsigned long *clientHandle, const DACS_OPTIONS *pOptions, DACS_ERROR_TYPE *);
/* Disconnect from the DACS daemon process */
extern int DACS_ClientExit(char *, int, DACS_ERROR_TYPE *);
extern int DACS_ClientExitMC(char *, int, DACS_ERROR_TYPE *, unsigned long clientHandle);
/* Get the version of DACS-API */
extern int DACS_FindVersion(unsigned char *, unsigned long, DACS_ERROR_TYPE *);
/* Begin the user login sequence to the DACS System */
extern int DACS_UserLogin(char *, char *, char *, unsigned long, unsigned long *, DACS_ERROR_TYPE *);
extern int DACS_UserLoginMC(char *, char *, char *, unsigned long, unsigned long *, DACS_ERROR_TYPE *, unsigned long clientHandle);
/* Logout a previously successful user login */
extern int DACS_UserLogout(unsigned long userHandle);
extern int DACS_UserLogoutMC(unsigned long userHandle, unsigned long clientHandle);
/* Relogin a user, this asks the DACS Server for a refreshed user profile */
extern int DACS_ReloginUser(unsigned long userHandle, DACS_ERROR_TYPE *);
extern int DACS_ItemVerify(unsigned long userHandle, ITEM_LOCK_TYPE *, DACS_ERROR_TYPE *, char *);
extern int DACS_ItemVerifyOd(unsigned long userHandle, ITEM_LOCK_TYPE *, int eConsiderOd, DACS_ERROR_TYPE *, char *);
extern int DACS_InsertVerify(unsigned long userHandle, int serviceId, char *item, DACS_ERROR_TYPE *errorCode, char *errorString);
extern int DACS_InsertVerifyLock(unsigned long userHandle, ITEM_LOCK_TYPE *, DACS_ERROR_TYPE *errorCode, char *errorString);
extern int DACS_SubscribeVerify(unsigned long userHandle, int, char *, DACS_ERROR_TYPE *, char *);
extern int DACS_CreateKey(unsigned long userHandle, int, unsigned char **, int *, DACS_ERROR_TYPE *);

extern int DACS_HandleVariables(unsigned long userHandle, unsigned long, unsigned long *, void **);
/* Log the usage */
extern int DACS_LogAction(unsigned long eventType, unsigned long userHandle, LOGDATA *);
extern int DACS_LogActionMC(unsigned long eventType, unsigned long userHandle, LOGDATA *, unsigned long clientHandle);
/* Log the usage using the extended interface */
extern int DACS_LogActionEx(unsigned long type, const char *userName, const char *positionName, const char *applicationName, LOGDATA *LogData);
extern int DACS_LogActionExMC(unsigned long eventType, const char *userName, const char *positionName, const char *applicationName, LOGDATA *LogData, unsigned long clientHandle);
/* Flush the cached log information to disk or to the DACS Daemon depending on configuration */
extern int DACS_LogFlush(void);
extern int DACS_LogFlushMC(unsigned long clientHandle);

extern int DACS_LogInit(unsigned long, char *);

extern int DACS_SpecialVariables(unsigned long userHandle, char *, char *);
extern int DACS_SpecialVariablesMC(unsigned long userHandle, char *, char *, unsigned long clientHandle);

extern DACS_SERVICE_LIST *DACS_GetServiceList(char *);
extern DACS_SERVICE_LIST *DACS_GetServiceListMC(char *, unsigned long clientHandle);

extern DACS_SERVICE_QOS  *DACS_GetQoSValues(int);
extern DACS_SERVICE_QOS  *DACS_GetQoSValuesMC(int, unsigned long clientHandle);

extern int DACS_GetPEList(unsigned long userHandle, int, const char *, int *, unsigned long **, DACS_ERROR_TYPE *, char *);
extern int DACS_GetPEODList(unsigned long userHandle, int, const char *, int *, unsigned long **, DACS_ERROR_TYPE *, char *);
extern int DACS_GetPEODListEx(unsigned long userHandle, int, const char *, int *, unsigned long **, unsigned short **, DACS_ERROR_TYPE *, char *);
extern int DACS_GetInsertPEList(unsigned long userHandle, int, const char *, int *, unsigned long **, DACS_ERROR_TYPE *, char *);
extern int DACS_GetSubscribeList(unsigned long userHandle, int, char **, DACS_ERROR_TYPE *, char *);
extern int DACS_GetInsertList(unsigned long userHandle, int, char **, DACS_ERROR_TYPE *, char *);
extern int DACS_GetUserSubServices(unsigned long userHandle, int, DACS_SUBSERVICE_ELEMENT **, int *, DACS_ERROR_TYPE *, char *);
extern int DACS_GetUserSubServicesOd(unsigned long userHandle, int, DACS_SUBSERVICE_ELEMENT **, int *, DACS_ERROR_TYPE *, char *);
/* Get a list of PE's -> Subservice's */
extern int DACS_PeToSubService(int service, DACS_PE_SUBSERVICE_ELEMENT **ppPe2SubserviceList, int *pPe2SubserviceListCnt, DACS_ERROR_TYPE *Dacs_Error, char *ErrorString);
extern int DACS_PeToSubServiceMC(int service, DACS_PE_SUBSERVICE_ELEMENT **ppPe2SubserviceList, int *pPe2SubserviceListCnt, DACS_ERROR_TYPE *Dacs_Error, char *ErrorString, unsigned long clientHandle);
/* Get a list of SBE's -> Subservices */
extern int DACS_SubServiceToSbe(int service, DACS_SBE_SUBSERVICE_ELEMENT **ppSubservice2SbeList, int *pSubservice2SbeListCnt, DACS_ERROR_TYPE *Dacs_Error, char *ErrorString);
extern int DACS_SubServiceToSbeMC(int service, DACS_SBE_SUBSERVICE_ELEMENT **ppSubservice2SbeList, int *pSubservice2SbeListCnt, DACS_ERROR_TYPE *Dacs_Error, char *ErrorString, unsigned long clientHandle);

extern DACS_FAILURE_INFO    *DACS_GetLastFailure(DACS_FAILURE_INFO *);
extern DACS_FAILURE_INFO    *DACS_GetLastFailureMC(DACS_FAILURE_INFO *, unsigned long clientHandle);

#else
extern int DACS_ClientStart();
extern int DACS_ClientStartMC();

extern int DACS_ClientExit();
extern int DACS_ClientExitMC();

extern int DACS_FindVersion();

extern int DACS_UserLogin();
extern int DACS_UserLoginMC();

extern int DACS_UserLogout();
extern int DACS_UserLogoutMC();

extern int DACS_ReloginUser();
extern int DACS_ItemVerify();
extern int DACS_ItemVerifyOd();
extern int DACS_InsertVerify();
extern int DACS_InsertVerifyLock();
extern int DACS_SubscribeVerify();
extern int DACS_CreateKey();
extern int DACS_HandleVariables();

extern int DACS_LogAction();
extern int DACS_LogActionMC();
extern int DACS_LogActionEx();
extern int DACS_LogActionExMC();

extern int DACS_LogFlush();
extern int DACS_LogFlushMC();

extern int DACS_LogInit();

extern int DACS_SpecialVariables();
extern int DACS_SpecialVariablesMC();

extern DACS_SERVICE_LIST *DACS_GetServiceList();
extern DACS_SERVICE_LIST *DACS_GetServiceListMC();

extern DACS_SERVICE_QOS  *DACS_GetQoSValues();
extern DACS_SERVICE_QOS  *DACS_GetQoSValuesMC();

extern int DACS_GetPEList();
extern int DACS_GetPEODList();
extern int DACS_GetPEODListEx();

extern int DACS_GetInsertPEList();
extern int DACS_GetSubscribeList();
extern int DACS_GetInsertList();
extern int DACS_GetUserSubServices();
extern int DACS_GetUserSubServicesOd();

extern int DACS_PeToSubService();
extern int DACS_PeToSubServiceMC();

/* Get a list of SBE's -> Subservices */
extern int DACS_SubServiceToSbe();
extern int DACS_SubServiceToSbeMC();

extern DACS_FAILURE_INFO    *DACS_GetLastFailure();
extern DACS_FAILURE_INFO    *DACS_GetLastFailureMC();
#endif

/******************************************************************************
* Sink DACS Library Callback functions                                        *
******************************************************************************/
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
extern int LoginResultPass(unsigned long);
extern int LoginResultFail(unsigned long, char *);
extern int LogoutResultPass(unsigned long);
extern int ForceLogout(unsigned long);
extern int ReverifyLogin(unsigned long);
extern int ReloginUserPass(unsigned long);
extern int ReloginUserFail(unsigned long, char *);
extern int ForceRepermission(unsigned long);
extern int DACS_AddFD(int, int (*)(void), int (*)(void));
extern int DACS_DeleteFD(int);
#else
extern int LoginResultPass();
extern int LoginResultFail();
extern int LogoutResultPass();
extern int ForceLogout();
extern int ReverifyLogin();
extern int ReloginUserPass();
extern int ReloginUserFail();
extern int ForceRepermission();
extern int DACS_AddFD();
extern int DACS_DeleteFD();
#endif

/******************************************************************************
* Source DACS Library functions                                               *
******************************************************************************/
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
extern int DACS_CsLock(int, char *, unsigned char **, int *, COMB_LOCK_TYPE *, DACS_ERROR_TYPE *);
extern int DACS_GetLock(int, PRODUCT_CODE_TYPE *, unsigned char **, int *, DACS_ERROR_TYPE *);
extern int DACS_GetLockV4(int, const char *, unsigned char **, int *, DACS_ERROR_TYPE *);
extern int DACS_CmpLock(unsigned char *, int, unsigned char *, int, DACS_ERROR_TYPE *);
#else
extern int DACS_CsLock();
extern int DACS_GetLock();
extern int DACS_GetLockV4();
extern int DACS_CmpLock();
#endif

/******************************************************************************
* Common DACS Library functions                                               *
******************************************************************************/
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
extern int DACS_perror(unsigned char *, int, unsigned char *, DACS_ERROR_TYPE *);
#else
extern int DACS_perror();
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* C++ definition */

#if defined(UNDO_STDC)
#undef __STDC__
#endif
#endif
