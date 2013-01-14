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
#include <QMenu>
#include <QAction>
#include <QRegExp>
#include <QDebug>

#include "SearchBar.h"

SearchBar::SearchBar(QWidget *parent) : 
        QWidget(parent),
        m_regExp("")
{
    widget.setupUi(this);
    connect(widget.closeButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(widget.searchTextEdit, SIGNAL(textChanged(QString)), this, SLOT(searchTextChanged()));
    connect(widget.findPreviousButton, SIGNAL(clicked()), this, SLOT(findPrevious()));
    connect(widget.findNextButton, SIGNAL(clicked()), this, SLOT(findNext()));

    QMenu *optionsMenu = new QMenu(widget.optionsButton);
    widget.optionsButton->setMenu(optionsMenu);
    
    m_matchCaseMenuEntry = optionsMenu->addAction(tr("Match case"));
    m_matchCaseMenuEntry->setCheckable(true);
    m_matchCaseMenuEntry->setChecked(true);
    
    m_useRegularExpressionMenuEntry = optionsMenu->addAction(tr("Regular expression"));
    m_useRegularExpressionMenuEntry->setCheckable(true);

    m_highlightMatchesMenuEntry = optionsMenu->addAction(tr("Higlight all matches"));
    m_highlightMatchesMenuEntry->setCheckable(true);
    m_highlightMatchesMenuEntry->setChecked(true);
}

SearchBar::~SearchBar() {
}

void SearchBar::show()
{
    QWidget::show();
    widget.searchTextEdit->setFocus();
}

void SearchBar::hide()
{
    QWidget::hide();
}

void SearchBar::toggleShown()
{
    isHidden() ? show() : hide();
}

void SearchBar::searchTextChanged()
{
    m_regExp = QRegExp(widget.searchTextEdit->text());

    if (! m_useRegularExpressionMenuEntry->isChecked()) {
        m_regExp.setPatternSyntax(QRegExp::FixedString);
    }
    if (! m_matchCaseMenuEntry->isChecked()) {
        m_regExp.setCaseSensitivity(Qt::CaseInsensitive);
    }

    emit search(m_regExp, true, false);
}

void SearchBar::findNext()
{
    emit search(m_regExp, true, true);
}

void SearchBar::findPrevious()
{
    emit search(m_regExp, false, false);
}
