/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.newsviewer;

import java.awt.Font;
import java.util.List;
import java.util.Vector;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;

/**
 * This class is responsible for building GUI to display news story data.
 */

public class NewsStoryViewer
{

    private JScrollPane _scrollStory;
    private JTextArea _story;

    private List<String> tempList = new Vector<String>();

    private Error error = TransportFactory.createError();

    NewsStoryViewer()
    {
        Font font = new Font("Arial Unicode MS", Font.PLAIN, 12);
        _story = new JTextArea(30, 80);
        _story.setFont(font);
        _scrollStory = new JScrollPane(_story);
        _scrollStory.setAutoscrolls(true);
    }

    NewsStoryViewer(Font font)
    {
        _story = new JTextArea(30, 80);
        _story.setFont(font);
        _scrollStory = new JScrollPane(_story);
        _scrollStory.setAutoscrolls(true);

    }

    public JScrollPane component()
    {
        return _scrollStory;
    }

    void openStory(String lang_ind, String pnac, String headline, NewsHandler MPHandler)
    {
        _story.setText(null);
        _story.append(headline);
        _story.append("\n");
        _story.append(pnac);
        _story.append("\n\n");

        tempList.add(pnac);

        MPHandler.sendPnacRequest(tempList, error);

        tempList.clear();

    }

    void writeBody(String body)
    {
        _story.append(body);
    }

}
