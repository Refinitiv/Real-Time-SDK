/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.newsviewer;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JList;

/**
 * This class handles mouse events
 */
public class NewsHeadlineListener extends MouseAdapter
{
    private final NewsStoryViewer _viewer;
    private NewsHandler _mpHandler;

    NewsHeadlineListener(NewsStoryViewer viewer, NewsHandler MPHandler)
    {
        _viewer = viewer;
        _mpHandler = MPHandler;
    }

    public void mouseClicked(MouseEvent e)
    {
        if (e.getClickCount() == 2)
        {
            @SuppressWarnings("unchecked")
            JList<Headline> list = (JList<Headline>)e.getComponent();
            int index = list.locationToIndex(e.getPoint());
            if (index != -1)
            {
                Headline hl = list.getModel().getElementAt(index);
                _viewer.openStory(hl.get_lang(), hl.get_pnac(), hl.get_text(),
                                  _mpHandler);
            }
        }
    }
}
