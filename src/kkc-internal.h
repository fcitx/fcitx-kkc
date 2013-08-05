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

#ifndef _FCITX_Kkc_INTERNAL_H_
#define _FCITX_Kkc_INTERNAL_H_

#include "kkc.h"
#include <fcitx/instance.h>
#include <fcitx/candidate.h>
#include <libkkc/libkkc.h>

#define _(x) dgettext("fcitx-kkc", x)
#define N_(x) (x)

typedef struct {
    FcitxGenericConfig gconfig;
    KkcPunctuationStyle punctuationStyle;
    KkcInputMode initialInputMode;
    FcitxCandidateLayoutHint candidateLayout;
    int nTriggersToShowCandWin;
    boolean autoCorrect;
    int pageSize;
    FcitxHotkey prevPageKey[2];
    FcitxHotkey nextPageKey[2];
    FcitxHotkey cursorUpKey[2];
    FcitxHotkey cursorDownKey[2];
    boolean showAnnotation;
} FcitxKkcConfig;

typedef struct {
    FcitxKkcConfig config;
    FcitxInstance *owner;
    KkcLanguageModel* model;
    KkcContext* context;
    FcitxUIMenu inputModeMenu;
    gulong handler;
    FcitxMessages* tempMsg;
} FcitxKkc;

CONFIG_BINDING_DECLARE(FcitxKkcConfig);

#endif
