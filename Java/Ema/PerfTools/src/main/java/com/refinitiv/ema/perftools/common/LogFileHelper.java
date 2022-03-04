/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.Objects;

public class LogFileHelper {

    public LogFileHelper() {
    }

    /**
     * Open the log file.
     *
     * @param name - name of the file.
     * @return information about initialized file or exit if file cannot be opened.
     */
    public static LogFileInfo initFile(String name) {
        LogFileInfo fileInfo = new LogFileInfo();
        fileInfo.file(new File(name));
        try {
            fileInfo.writer(new PrintWriter(fileInfo.file()));
            fileInfo.supportedWriting(true);
        } catch (FileNotFoundException e) {
            System.out.printf("Error: Failed to open summary file '%s'.\n", fileInfo.file().getName());
            System.exit(-1);
        }
        return fileInfo;
    }

    /**
     * Safe method for writing data to the file.
     *
     * @param fileInfo - information about file where information should be written to.
     * @param value    - information which must be written to the file.
     */
    public static void writeFile(LogFileInfo fileInfo, String value) {
        if (Objects.nonNull(fileInfo) && Objects.nonNull(fileInfo.writer()) && fileInfo.supportedWriting()) {
            fileInfo.writer().print(value);
            fileInfo.writer().flush();
        }
    }
}
