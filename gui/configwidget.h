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

#ifndef FCITX_KKC_GUI_CONFIGWIDGET_H
#define FCITX_KKC_GUI_CONFIGWIDGET_H

#include <fcitx-qt/fcitxqtconfiguiwidget.h>

namespace Ui {
class KkcConfigWidget;
}

class KkcConfigWidget : public FcitxQtConfigUIWidget
{
    Q_OBJECT
public:
    explicit KkcConfigWidget(QWidget* parent = 0);
    virtual ~KkcConfigWidget();

    virtual void load();
    virtual void save();
    virtual QString title();
    virtual QString addon();

private:
    Ui::KkcConfigWidget* m_ui;
};


#endif // FCITX_KKC_GUI_CONFIGWIDGET_H
