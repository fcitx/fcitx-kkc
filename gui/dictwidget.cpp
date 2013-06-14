/***************************************************************************
 *   Copyright (C) 2013~2013 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "common.h"
#include "dictwidget.h"
#include "adddictdialog.h"
#include "dictmodel.h"
#include "shortcutmodel.h"
#include "ui_dictwidget.h"

KkcDictWidget::KkcDictWidget(QWidget* parent): FcitxQtConfigUIWidget(parent)
    ,m_ui(new Ui::KkcDictWidget)
{
    m_ui->setupUi(this);
    m_dictModel = new DictModel(this);

    m_ui->dictionaryView->setModel(m_dictModel);

    connect(m_ui->addDictButton, SIGNAL(clicked(bool)), this, SLOT(addDictClicked()));
    connect(m_ui->defaultDictButton, SIGNAL(clicked(bool)), this,  SLOT(defaultDictClicked()));
    connect(m_ui->removeDictButton, SIGNAL(clicked(bool)), this, SLOT(removeDictClicked()));
    connect(m_ui->moveUpDictButton, SIGNAL(clicked(bool)), this, SLOT(moveUpDictClicked()));
    connect(m_ui->moveDownDictButton, SIGNAL(clicked(bool)), this, SLOT(moveDownClicked()));

    load();
}

KkcDictWidget::~KkcDictWidget()
{
    delete m_ui;
}

QString KkcDictWidget::addon()
{
    return "fcitx-kkc";
}

QString KkcDictWidget::title()
{
    return _("Kana Kanji Dictionary Manager");
}

void KkcDictWidget::load()
{
    m_dictModel->load();
    Q_EMIT changed(false);
}

void KkcDictWidget::save()
{
    m_dictModel->save();
    Q_EMIT changed(false);
}

void KkcDictWidget::addDictClicked()
{
    AddDictDialog dialog;
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        m_dictModel->add(dialog.dictionary());
        Q_EMIT changed(true);
    }
}

void KkcDictWidget::defaultDictClicked()
{
    m_dictModel->defaults();
    Q_EMIT changed(true);
}

void KkcDictWidget::removeDictClicked()
{
    if (m_ui->dictionaryView->currentIndex().isValid()) {
        m_dictModel->removeRow(m_ui->dictionaryView->currentIndex().row());
        Q_EMIT changed(true);
    }
}

void KkcDictWidget::moveUpDictClicked()
{
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveUp(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row - 1), QItemSelectionModel::ClearAndSelect);
        Q_EMIT changed(true);
    }
}

void KkcDictWidget::moveDownClicked()
{
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveDown(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row + 1), QItemSelectionModel::ClearAndSelect);
        Q_EMIT changed(true);
    }
}
