/***************************************************************************
 *   Copyright (C) YEAR~YEAR by Your Name                                  *
 *   your-email@address.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcitx/ime.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utils.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/instance.h>
#include <fcitx/context.h>
#include <fcitx/module.h>
#include <fcitx/hook.h>
#include <fcitx/candidate.h>
#include <libkkc/libkkc.h>
#include <libintl.h>

#include "config.h"
#include "kkc-internal.h"

static void *FcitxKkcCreate(FcitxInstance *instance);
static void FcitxKkcDestroy(void *arg);
static void FcitxKkcReloadConfig(void *arg);
CONFIG_DEFINE_LOAD_AND_SAVE(Kkc, FcitxKkcConfig, "fcitx-kkc")
DECLARE_ADDFUNCTIONS(Kkc)

FCITX_DEFINE_PLUGIN(fcitx_kkc, ime2, FcitxIMClass2) = {
    FcitxKkcCreate,
    FcitxKkcDestroy,
    FcitxKkcReloadConfig,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

boolean FcitxKkcInit(void *arg); /**< FcitxIMInit */
void FcitxKkcResetIM(void *arg); /**< FcitxIMResetIM */
INPUT_RETURN_VALUE FcitxKkcDoInput(void *arg, FcitxKeySym, unsigned int); /**< FcitxIMDoInput */
INPUT_RETURN_VALUE FcitxKkcDoReleaseInput(void *arg, FcitxKeySym, unsigned int); /**< FcitxIMDoInput */
INPUT_RETURN_VALUE FcitxKkcGetCandWords(void *arg); /**< FcitxIMGetCandWords */
void FcitxKkcSave(void *arg); /**< FcitxIMSave */

static void*
FcitxKkcCreate(FcitxInstance *instance)
{
    FcitxKkc *kkc = fcitx_utils_new(FcitxKkc);
    bindtextdomain("fcitx-kkc", LOCALEDIR);
    kkc->owner = instance;

    g_type_init();
    kkc_init();

    KkcLanguageModel* model = kkc_language_model_load("sorted3", NULL);
    if (!model) {
        free(kkc);
        return NULL;
    }

    FcitxXDGMakeDirUser("kkc/rules");

    kkc->model = model;
    kkc->context = kkc_context_new(model);
    KkcDictionaryList* dictionaries = kkc_context_get_dictionaries(kkc->context);
    KkcSystemSegmentDictionary* dict = kkc_system_segment_dictionary_new("/usr/share/skk/SKK-JISYO.L", "EUC-JP", NULL);
    kkc_dictionary_list_add(dictionaries, KKC_DICTIONARY(dict));
    kkc_context_set_punctuation_style(kkc->context, KKC_PUNCTUATION_STYLE_JA_JA);
    kkc_context_set_input_mode(kkc->context, KKC_INPUT_MODE_HIRAGANA);

    KkcRule* rule = kkc_rule_new(kkc_rule_find_rule("default"), NULL);

    kkc_context_set_typing_rule(kkc->context, rule);

    if (!KkcLoadConfig(&kkc->config)) {
        free(kkc);
        return NULL;
    }

    FcitxIMIFace iface;
    memset(&iface, 0, sizeof(FcitxIMIFace));
    iface.Init = FcitxKkcInit;
    iface.DoInput = FcitxKkcDoInput;
    iface.DoReleaseInput = FcitxKkcDoReleaseInput;
    iface.GetCandWords = FcitxKkcGetCandWords;
    iface.Save = FcitxKkcSave;
    iface.ResetIM = FcitxKkcResetIM;

    FcitxInstanceRegisterIMv2(instance, kkc, "kkc", _("Kkc"), "kkc", iface, 1, "ja");

    FcitxKkcAddFunctions(instance);
    return kkc;
}

INPUT_RETURN_VALUE FcitxKkcDoInputReal(void* arg, FcitxKeySym sym, unsigned int state)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(kkc->owner);
    uint32_t keycode = FcitxInputStateGetKeyCode(input);
    state = state & (FcitxKeyState_SimpleMask | (1 << 30));
    KkcKeyEvent* key = kkc_key_event_new_from_x_event(sym, keycode, state, NULL);
    if (!key) {
        return IRV_TO_PROCESS;
    }

    gboolean retval = kkc_context_process_key_event(kkc->context, key);

    g_object_unref(key);
    if (retval) {
        return IRV_DISPLAY_CANDWORDS;
    }
    return IRV_TO_PROCESS;
}

boolean FcitxKkcInit(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    boolean flag = true;
    FcitxInstanceSetContext(kkc->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "jp");
    FcitxInstanceSetContext(kkc->owner, CONTEXT_DISABLE_AUTOENG, &flag);
    FcitxInstanceSetContext(kkc->owner, CONTEXT_DISABLE_QUICKPHRASE, &flag);
    FcitxInstanceSetContext(kkc->owner, CONTEXT_DISABLE_FULLWIDTH, &flag);
    FcitxInstanceSetContext(kkc->owner, CONTEXT_DISABLE_AUTO_FIRST_CANDIDATE_HIGHTLIGHT, &flag);
    return true;
}

INPUT_RETURN_VALUE FcitxKkcDoInput(void* arg, FcitxKeySym _sym, unsigned int _state)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(kkc->owner);
    FcitxKeySym sym = (FcitxKeySym) FcitxInputStateGetKeySym(input);
    uint32_t state = FcitxInputStateGetKeyState(input);
    return FcitxKkcDoInputReal(arg, sym, state);
}

INPUT_RETURN_VALUE FcitxKkcDoReleaseInput(void* arg, FcitxKeySym _sym, unsigned int _state)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(kkc->owner);
    FcitxKeySym sym = (FcitxKeySym) FcitxInputStateGetKeySym(input);
    uint32_t state = FcitxInputStateGetKeyState(input);
    state |= (1 << 30);
    return FcitxKkcDoInputReal(arg, sym, state);
}

INPUT_RETURN_VALUE FcitxKkcGetCandWord(void* arg, FcitxCandidateWord* word)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
    int idx = *((int*)word->priv);
    gboolean retval = kkc_candidate_list_select_at(kkcCandidates, idx);
    if (retval) {
        if (kkc_context_has_output(kkc->context)) {
            gchar* str = kkc_context_poll_output(kkc->context);
            FcitxInstanceCommitString(kkc->owner, FcitxInstanceGetCurrentIC(kkc->owner), str);
            g_free(str);
        }
        return IRV_DISPLAY_CANDWORDS;
    }

    return IRV_TO_PROCESS;
}

INPUT_RETURN_VALUE FcitxKkcGetCandWords(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(kkc->owner);
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    FcitxMessages* clientPreedit = FcitxInputStateGetClientPreedit(input);
    FcitxMessages* preedit = FcitxInputStateGetPreedit(input);

    KkcSegmentList* segments = kkc_context_get_segments(kkc->context);
    if (kkc_segment_list_get_cursor_pos(segments) >= 0) {
        int i = 0;
        for (i = 0; i < kkc_segment_list_get_size(segments); i ++) {
            KkcSegment* segment = kkc_segment_list_get(segments, i);
            const gchar* str = kkc_segment_get_output(segment);
            FcitxMessageType messageType = MSG_INPUT;
            if (i == kkc_segment_list_get_cursor_pos(segments)) {
                messageType = MSG_HIGHLIGHT;
            }
            FcitxMessagesAddMessageAtLast(clientPreedit, messageType, "%s", str);
        }
    } else {
        gchar* str = kkc_context_get_input(kkc->context);
        FcitxMessagesAddMessageAtLast(clientPreedit, MSG_INPUT, "%s", str);
        g_free(str);
    }

    KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
    if (kkc_candidate_list_get_size(kkcCandidates) > 0) {
        int i;

        for (i = 0; i < kkc_candidate_list_get_size(kkcCandidates); i++ ) {
            KkcCandidate* kkcCandidate = kkc_candidate_list_get(kkcCandidates, i);
            FcitxCandidateWord candWord;
            candWord.callback = FcitxKkcGetCandWord;
            candWord.extraType = MSG_OTHER;
            candWord.owner = kkc;
            int* idx = fcitx_utils_new(int);
            *idx = i;
            candWord.priv = idx;
            candWord.strExtra = NULL;
            candWord.strWord = strdup(kkc_candidate_get_output(kkcCandidate));
            FcitxCandidateWordAppend(candList, &candWord);
        }
    }

    return IRV_DISPLAY_CANDWORDS;
}

void FcitxKkcResetIM(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    kkc_context_reset(kkc->context);
}

void FcitxKkcSave(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    kkc_dictionary_list_save(kkc_context_get_dictionaries(kkc->context));
}



static void
FcitxKkcDestroy(void *arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    free(kkc);
}

static void
FcitxKkcReloadConfig(void *arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    KkcLoadConfig(&kkc->config);
}

#include "fcitx-kkc-addfunctions.h"
