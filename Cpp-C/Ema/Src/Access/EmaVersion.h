#ifndef _EMA_VERSION_H_
#define _EMA_VERSION_H_

/*--------------------------------------------------------------------------
	 CHANGE BELOW HERE
--------------------------------------------------------------------------*/
#define Ema_Release_Major		3	
#define Ema_Release_Minor		1
#define Ema_Product_Major		2	
#define Ema_Product_Minor		1	
#define PRODNAME			"Elektron Message API EMA "
#define PRODVERNAME			"3.1.2"
#define COMPANYNAME			"Thomson Reuters, Oak Brook, IL"
#define COPYRIGHTYEAR			"2018"
#define COMPONENTNAME		"ema3.1.2.G1"
#define DDATE				"Jan 26 11:00:00 CDT 2017"
#define INTERNALVERSION			"(Internal Node: Ema 3.1.2)"
/*--------------------------------------------------------------------------
	 CHANGE ABOVE HERE
--------------------------------------------------------------------------*/

#ifdef __EMA_STATIC_BUILD__
	#define EMA_LINK_TYPE "Static"
#else
	#define EMA_LINK_TYPE "Shared Library"
#endif
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

#ifdef WIN32
#define EMA_COMPONENT_VER_PLATFORM ".win "
static char emaComponentBldtype[] = BLDTYPE;
static char emaComponentLinkType[] = EMA_LINK_TYPE;
#else
#define EMA_COMPONENT_VER_PLATFORM ".linux "
extern char emaComponentBldtype[];
extern char emaComponentLinkType[]; 
#endif

#define COMPILE_BITS_STR "64-bit "


/* ------------------------------------------------------------- */
/* The following are used in the rc files for the windows builds */
/* ------------------------------------------------------------- */
#define Ema_VersionString		COMPONENTNAME " (" BLDTYPE ")\0"   /* format "6.0.0.23 (BLDTYPE)\0" */
#define Ema_ProdName			PRODNAME "\0"
#define Ema_Version			Ema_Release_Major,Ema_Release_Minor,Ema_Product_Major,Ema_Product_Minor
#define Ema_InternalNode		INTERNALVERSION "\0"
#define Ema_CompanyName			COMPANYNAME "\0"
#define Ema_LegalCopyright		"Copyright (C) " COPYRIGHTYEAR " " Ema_CompanyName ", All Rights Reserved."
#define Ema_Comments			"Custom Elektron Message API Build"

#endif //_EMA_VERSION_H_
