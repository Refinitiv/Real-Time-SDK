/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#ifndef _cnv_tab_h_
#define _cnv_tab_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _M_I86
#define FAR __far
#else
#define FAR
#endif
typedef unsigned short FAR rssl_uni_map_entry;

/* Conversion tables */
extern rssl_uni_map_entry rsslcnvtab_rbcs2[];
extern rssl_uni_map_entry rsslcnvtab_rbcs2_br[];
extern rssl_uni_map_entry rsslcnvtab_romaji[];
extern rssl_uni_map_entry rsslcnvtab_katakana[];
extern rssl_uni_map_entry rsslcnvtab_208a[];
extern rssl_uni_map_entry rsslcnvtab_208b[];
extern rssl_uni_map_entry rsslcnvtab_cns1a[];
extern rssl_uni_map_entry rsslcnvtab_cns1b[];
extern rssl_uni_map_entry rsslcnvtab_cns2[];

#ifdef __cplusplus
}	// extern "C"
#endif

#endif

