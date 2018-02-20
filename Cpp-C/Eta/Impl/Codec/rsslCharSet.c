/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslCharSet.h"

const int SHAPE_94 = 94;
const int SHAPE_96 = 96;

const RsslRmtesCharSet _rsslReuterBasic1 = {0, 0, 94, 94, 0, 1, 0};
const RsslRmtesCharSet _rsslReuterBasic2 = {rsslcnvtab_rbcs2, rsslcnvtab_rbcs2_br, 94, 94, 63, 1, 0};
const RsslRmtesCharSet _rsslJapaneseKatakana = {rsslcnvtab_katakana, 0, 94, 63, 0, 1, 0};
const RsslRmtesCharSet _rsslJapaneseLatin = {rsslcnvtab_romaji, 0, 94, 94, 0, 1, 0};
const RsslRmtesCharSet _rsslJapaneseKanji = {rsslcnvtab_208a, rsslcnvtab_208b, 94, ((0x28 - 0x21) * 94 + (0x40 - 0x21) + 1), ((0x30 - 0x21) * 94), ((0x74 - 0x30) * 94 + (0x26 - 0x21) + 1), 2};
const RsslRmtesCharSet _rsslChinese1 = {rsslcnvtab_cns1a, rsslcnvtab_cns1b, 94, ((0x26 - 0x21) * 94 + (0x3E - 0x21) + 1), ((0x42 - 0x21) * 94), ((0x7D - 0x42) * 94 + (0x4B - 0x21) + 1), 2};
const RsslRmtesCharSet _rsslChinese2 = {rsslcnvtab_cns2, 0, 94, ((0x72 - 0x21) * 94 + (0x44 - 0x21) + 1), 0, 0, 2};

#ifdef __cplusplus
}
#endif
