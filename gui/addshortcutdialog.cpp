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

#include <QDebug>

#include "addshortcutdialog.h"
#include "ui_addshortcutdialog.h"
#include "common.h"

AddShortcutDialog::AddShortcutDialog(QWidget* parent): QDialog(parent)
    ,m_ui(new Ui::AddShortcutDialog)
    ,m_length(0)
{
    m_ui->setupUi(this);
    m_ui->inputModeLabel->setText(_("&Input Mode"));
    m_ui->commandLabel->setText(_("&Command"));
    m_ui->keyLabel->setText(_("&Key"));
    m_ui->keyButton->setModifierlessAllowed(true);
    m_ui->keyButton->setMultiKeyShortcutsAllowed(false);
    for (int i = 0; i < KKC_INPUT_MODE_DIRECT; i++) {
        m_ui->inputModeComboBox->addItem(_(modeName[i]));
    }

    m_commands = kkc_keymap_commands(&m_length);

    for (int i = 0; i < m_length; i++) {
        gchar* label = kkc_keymap_get_command_label(m_commands[i]);
        m_ui->commandComboBox->addItem(QString::fromUtf8(label));
        g_free(label);
    }

    connect(m_ui->keyButton, SIGNAL(keySequenceChanged(QKeySequence,FcitxQtModifierSide)), this, SLOT(keyChanged()));

    keyChanged();
}

AddShortcutDialog::~AddShortcutDialog()
{
    for (int i = 0; i < m_length; i++) {
        g_free(m_commands[i]);
    }
    g_free(m_commands);
    delete m_ui;
}

void AddShortcutDialog::keyChanged()
{
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_ui->keyButton->keySequence().count() > 0);
}

ShortcutEntry AddShortcutDialog::shortcut()
{
    KkcInputMode mode = (KkcInputMode) m_ui->inputModeComboBox->currentIndex();
    const QString command = QString::fromUtf8(m_commands[m_ui->commandComboBox->currentIndex()]);
    int keyQt = m_ui->keyButton->keySequence()[0];
    int sym;
    uint state;
    FcitxQtKeySequenceWidget::keyQtToFcitx(keyQt, m_ui->keyButton->modifierSide(), sym, state);
    KkcKeyEvent* event = kkc_key_event_new_from_x_event((guint) sym, 0, (KkcModifierType) state);
    return ShortcutEntry(command, event, m_ui->commandComboBox->currentText(), mode);
}
