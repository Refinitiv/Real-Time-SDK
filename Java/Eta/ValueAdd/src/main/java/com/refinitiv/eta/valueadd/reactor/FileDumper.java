package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.time.Instant;

public class FileDumper {
    private final String xmlTraceFileName;
    private final boolean xmlTraceToMultipleFiles;
    private final long xmlTraceMaxFileSize;
    private Path currentFilePath;
    private OutputStream out;
    private boolean fileSizeReached = false;
    private long currentFileSize = 0;

    public FileDumper(String xmlTraceFileName, boolean xmlTraceToMultipleFiles, long xmlTraceMaxFileSize) {
        this.xmlTraceFileName = xmlTraceFileName;
        this.xmlTraceToMultipleFiles = xmlTraceToMultipleFiles;
        this.xmlTraceMaxFileSize = xmlTraceMaxFileSize;
        this.currentFilePath = createNewFile();
        openFile();
    }

    private Path createNewFile() {
        String fileName = xmlTraceFileName + "_" + ProcessHandle.current().pid() + "_" + Instant.now().toEpochMilli() + ".xml";
        Path filePath = Paths.get(fileName);
        try {
            Files.createFile(filePath);
        } catch (IOException ignored) {
            // IOException likely to occur due to lack of system resources when this debug is enabled;
            // thus, ignored by design
        }
        return filePath;
    }

    private boolean isCurrentFileOverSize() {
        return currentFileSize >= xmlTraceMaxFileSize;
    }

    private void openFile() {
        try {
            out = Files.newOutputStream(currentFilePath, StandardOpenOption.APPEND);
        } catch (IOException ignored) {
            // IOException likely to occur due to lack of system resources when this debug is enabled;
            // thus, ignored by design
        }
    }

    public void dump(String content) {
        if (fileSizeReached) return;
        else if (isCurrentFileOverSize()) {
            if (xmlTraceToMultipleFiles) {
                close();
                currentFilePath = createNewFile();
                openFile();
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
            } catch (IOException ignored) {
                // IOException likely to occur due to lack of system resources when this debug is enabled;
                // thus, ignored by design
            }
        }
        currentFileSize = 0;
    }
}