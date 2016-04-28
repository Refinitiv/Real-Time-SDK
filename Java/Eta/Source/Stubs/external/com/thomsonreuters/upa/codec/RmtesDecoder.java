package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * Used to decode RMTES data from a Buffer using RmtesBuffer and
 * RmtesCacheBuffer.
 * 
 * @see RmtesBuffer
 * @see RmtesCacheBuffer
 */

public interface RmtesDecoder
{

    /**
     * Converts the given cache to UCS2 Unicode
     * 
     * Typical use:<BR>
     * 1. Allocate memory for the cache buffer.<BR>
     * 2. After decoding the payload buffer, call RMTESApplyToCache to copy the
     * data to the RmtesCacheBuffer.<BR>
     * 3. Allocate memory for the unicode string.<BR>
     * 3. Call RMTESToUCS2 to convert the RMTES data for display or parsing.<BR>
     * 
     * @param rmtesBuffer Buffer used to store decoded RMTES data
     * @param cacheBuffer Buffer containing encoded RMTES data
     * @return {@link CodecReturnCodes}
     */

    public int RMTESToUCS2(RmtesBuffer rmtesBuffer, RmtesCacheBuffer cacheBuffer);

    /**
     * Returns boolean for whether RmtesBuffer contains a partial update command
     * If RMTES content in an RmtesBuffer contains a partial update command,
     * return true If RMTES content in an RmtesBuffer does not contain a partial
     * update command, return false
     * 
     * @param inBuffer Buffer containing encoded RMTES Data.
     * @return Boolean whether the RMTES Data contains a partial RMTES Update
     */

    public boolean hasPartialRMTESUpdate(Buffer inBuffer);

    /**
     * Applies the inBuffer's partial update data to the cacheBuffer.
     * Preconditions: cacheBuffer is large enough to handle the additional data
     * cacheBuffer has already been populated with data Result: inBuffer's
     * partial update(s) are applied to outBuffer
     * 
     * @param inBuffer Buffer containing encoded RMTES data
     * @param cacheBuffer Buffer used to store encoded RMTES data and for
     *            finding any partial RMTES updates
     * @return {@link CodecReturnCodes}
     */

    public int RMTESApplyToCache(Buffer inBuffer, RmtesCacheBuffer cacheBuffer);

}
