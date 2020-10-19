package com.refinitiv.ansi;

import java.io.ByteArrayInputStream;

public interface Ansi
{
    int qa_decode(PageType page, ByteArrayInputStream is, ListType u_list);

    short qa_end_of_row();

    short qa_page_columns();

    short qa_page_rows();

    short qa_scroll_bot();

    void qa_set_columns(short cl);

    void qa_set_end_of_row(short rw);

    void qa_set_rows(short rw);

    void qa_set_scroll_bot(short cl);

    int MajorVersion = 1;
    int MinorVersion = 0;
}
