/*
 *  This file is part of seq24/sequencer64.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          qseqmainwnd.cpp
 *
 *  This module declares/defines the base class for the main window of the
 *  application.
 *
 * \library       sequencer64 application
 * \author        Chris Ahlstrom
 * \date          2017-09-06
 * \updates       2017-09-08
 * \license       GNU GPLv2 or above
 *
 */

#include "perform.hpp"                  /* seq64::perform                   */
#include "qseqmainwnd.hpp"              /* Qt5 version of seq64::mainwnd    */
#include "forms/qseqmainwnd.ui.h"       /* UIC-generated header file        */

/*
 *  All library code for this project is in the "seq64" namespace.  Do not
 *  attempt to document this namespace; it breaks Doxygen.
 */

namespace seq64
{

/**
 *
 */

qseqmainwnd::qseqmainwnd
(
    perform & p,
    bool allowperf2,
    int ppqn
#if defined SEQ64_MULTI_MAINWID
    ,
    int mainwid_rows,
    int mainwid_cols,
    bool mainwid_indep,
#endif
    QWidget * parent
) :
    QMainWindow (parent),
    m_ui        (new Ui::qseqmainwnd)
{
    // show();
}

/**
 *
 */

qseqmainwnd::~qseqmainwnd()
{
    delete m_ui;
}

/*
void
qseqmainwnd::panic ()
{
    m_main_perf->panic();
}
*/

}           /* namespace seq64 */

/*
 * qseqmainwnd.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

