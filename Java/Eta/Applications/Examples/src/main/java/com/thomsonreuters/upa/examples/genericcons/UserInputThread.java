package com.thomsonreuters.upa.examples.genericcons;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.concurrent.ConcurrentLinkedQueue;

/** 
 * This class gets command line input entered by the user.
 */
public class UserInputThread implements Runnable
{
    private volatile boolean readyForInput;
    private BufferedReader _inputStream;
    private ConcurrentLinkedQueue<String> fileNameQueue = new ConcurrentLinkedQueue<String>();

    {
        _inputStream = new BufferedReader(new InputStreamReader(System.in));
    }

    /* (non-Javadoc)
     * @see java.lang.Runnable#run()
     */
    @Override
    /** Runs the thread to get user input. */
    public void run()
    {
        while (true)
        {
            try
            {
                if (readyForInput)
                {
                    String fileName = getInput("\nEnter filename(s) to retrieve (use \",\" to separate multiple files): ");
                    if (fileName.length() > 0)
                    {
                        readyForInput = false;
                        fileNameQueue.add(fileName);
                    }
                }
                Thread.sleep(100);
            }
            catch (Exception e)
            {
                System.out.println("");
                break;
            }
        }
    }

    /** Indicates that the user is ready for input. */
    public void readyForInput()
    {
        readyForInput = true;
    }
    
    /**
     * Retrieves next file name from the file name queue.
     *
     * @return the string
     */
    public String nextFilename()
    {
        return fileNameQueue.poll();
    }
    
    /* gets input from the user */
    private String getInput(String prompt)
    {
        String retStr = "";
        System.out.print(prompt);
        
        try
        {
            retStr = _inputStream.readLine();
        }
        catch(IOException e)
        {
            
            System.exit(-1);
        }
        
        return retStr;
    }
}
