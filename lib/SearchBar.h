/*
    Copyright 2013 Christian Surlykke

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/
#ifndef _SEARCHBAR_H
#define	_SEARCHBAR_H

#include <QRegExp>

#include "ui_SearchBar.h"
#include "HistorySearch.h"

class SearchBar : public QWidget {
    Q_OBJECT
public:
    SearchBar(QWidget* parent = nullptr);
    ~SearchBar() override;
    virtual void show();
    QString searchText() const;
    bool useRegularExpression() const;
    bool matchCase() const;
    bool highlightAllMatches() const;
    void noMatchFound();
    void hide();

signals:
    void searchCriteriaChanged();
    void highlightMatchesChanged(bool highlightMatches);
    void findNext();
    void findPrevious();

protected:
    void keyReleaseEvent(QKeyEvent* keyEvent) override;

private:
    void clearBackgroundColor();

    Ui::SearchBar widget;
    QAction *m_matchCaseMenuEntry = nullptr;
    QAction *m_useRegularExpressionMenuEntry = nullptr;
    QAction *m_highlightMatchesMenuEntry = nullptr;
};

#endif	/* _SEARCHBAR_H */
