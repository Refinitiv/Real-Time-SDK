#ifndef _EMA_VERSION_H_
#define _EMA_VERSION_H_

/*--------------------------------------------------------------------------
	 CHANGE BELOW HERE
--------------------------------------------------------------------------*/
#define Ema_Release_Major		3
#define Ema_Release_Minor		0
#define Ema_Product_Major		0
#define Ema_Product_Minor		9
#define PRODNAME			"Elektron Message API EMA "
#define PRODVERNAME			"3.0.0.L1 RRG"
#define COMPANYNAME			"Thomson Reuters, Oak Brook, IL"
#define COPYRIGHTYEAR			"2015"
#define DDATE				"Wed Jul  24 11:11:42 CDT 2015"
#define INTERNALVERSION			"(Internal Node: Ema 3.0.F9)"
/*--------------------------------------------------------------------------
	 CHANGE ABOVE HERE
--------------------------------------------------------------------------*/


#ifdef WIN32
	#ifdef NDEBUG
		#ifdef _EMA_BLDTYPE_ASSERT_
			#define BLDTYPE "Release_MD_Assert"
		#else
			#define BLDTYPE "Release_MD"
		#endif
	#else
		#define BLDTYPE "Debug_MDd"
	#endif
#else
	#ifdef NDEBUG
		#ifdef _EMA_BLDTYPE_ASSERT_
			#define BLDTYPE "Optimized_Assert"
		#else
			#define BLDTYPE "Optimized"
		#endif
	#else
		#define BLDTYPE "Debug"
	#endif
#endif 


#define STR_EXPAND(str) #str
#define MKSTR(str) STR_EXPAND(str)

#define NEWVERSTRING MKSTR(Ema_Release_Major) "." MKSTR(Ema_Release_Minor) "." MKSTR(Ema_Product_Major) "." MKSTR(Ema_Product_Minor)

/* ------------------------------------------------------------- */
/* The following are used in the rc files for the windows builds */
/* ------------------------------------------------------------- */
#define Ema_VersionString		NEWVERSTRING " (" BLDTYPE ")\0"   /* format "6.0.0.23 (BLDTYPE)\0" */
#define Ema_ProdName			PRODNAME "\0"
#define Ema_Version			Ema_Release_Major,Ema_Release_Minor,Ema_Product_Major,Ema_Product_Minor
#define Ema_InternalNode		INTERNALVERSION "\0"
#define Ema_CompanyName			COMPANYNAME "\0"
#define Ema_LegalCopyright		"Copyright (C) " COPYRIGHTYEAR " " Ema_CompanyName ", All Rights Reserved."
#define Ema_Comments			"Custom Elektron Message API Build"

#endif //_EMA_VERSION_H_
