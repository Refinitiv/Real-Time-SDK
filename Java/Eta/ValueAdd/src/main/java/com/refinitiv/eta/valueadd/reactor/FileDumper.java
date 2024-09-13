/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.management.ManagementFactory;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.time.Instant;

public class FileDumper {
    private final String xmlTraceFileName;
    private final boolean xmlTraceToMultipleFiles;
    private final long xmlTraceMaxFileSize;
    private OutputStream out;
    private boolean fileSizeReached = false;
    private long currentFileSize = 0;

    public FileDumper(String xmlTraceFileName, boolean xmlTraceToMultipleFiles, long xmlTraceMaxFileSize) {
        this.xmlTraceFileName = xmlTraceFileName;
        this.xmlTraceToMultipleFiles = xmlTraceToMultipleFiles;
        this.xmlTraceMaxFileSize = xmlTraceMaxFileSize;
    }

    private void createNewFile() {
        String[] Ids = ManagementFactory.getRuntimeMXBean().getName().split("@");
        String processId = Ids.length > 0 ? "_" + Ids[0] + "_" : "_";
        String fileName = xmlTraceFileName + processId + Instant.now().toEpochMilli() + ".xml";

        Path filePath = Paths.get(fileName);
        try {
            Files.createFile(filePath);
            out = Files.newOutputStream(filePath, StandardOpenOption.APPEND);
        } catch (IOException ignored) {
            // IOException likely to occur due to lack of system resources when this debug is enabled;
            // thus, ignored by design
        }
    }

    private boolean isCurrentFileOverSize() {
        return currentFileSize >= xmlTraceMaxFileSize;
    }

    public void dump(String content) {
        if (fileSizeReached) return;
        else if (isCurrentFileOverSize()) {
            if (xmlTraceToMultipleFiles) {
                close();
                createNewFile();
                try {
                    out.write(content.getBytes());
                } catch (IOException ignored) {
                    // IOException likely to occur due to lack of system resources when this debug is enabled;
                    // thus, ignored by design
                }
                currentFileSize += content.getBytes().length;
            } else {
                fileSizeReached = true;
                close();
            }

        } else {
            try {
                if (out == null) {
                    createNewFile();
                }
                out.write(content.getBytes());
            } catch (IOException ignored) {
                // IOException likely to occur due to lack of system resources when this debug is enabled;
                // thus, ignored by design
            }
            currentFileSize += content.getBytes().length;
        }
    }

    public void close() {
        if (out != null) {
            try {
                out.close();
                out = null;
            } catch (IOException ignored) {
                // IOException likely to occur due to lack of system resources when this debug is enabled;
                // thus, ignored by design
            }
        }
        currentFileSize = 0;
    }
}
