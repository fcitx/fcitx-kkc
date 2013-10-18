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

UT_icd dict_icd = { sizeof(void*), 0, 0, 0};

#define _FcitxKeyState_Release (1 << 30)

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
void FcitxKkcOnClose(void* arg, FcitxIMCloseEventType event);
void FcitxKkcSave(void *arg); /**< FcitxIMSave */
void FcitxKkcApplyConfig(FcitxKkc* kkc);
void FcitxKkcResetHook(void *arg);
boolean FcitxKkcLoadDictionary(FcitxKkc* kkc);
boolean FcitxKkcLoadRule(FcitxKkc* kkc);

typedef struct _KkcStatus {
    const char* icon;
    const char* label;
    const char* description;
} KkcStatus;

KkcStatus input_mode_status[] = {
    {"",  "\xe3\x81\x82", N_("Hiragana") },
    {"", "\xe3\x82\xa2", N_("Katakana") },
    {"", "\xef\xbd\xb1", N_("Half width Katakana") },
    {"", "A", N_("Latin") },
    {"", "\xef\xbc\xa1", N_("Wide latin") },
};

const char* FcitxKkcGetInputModeIconName(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    return input_mode_status[kkc_context_get_input_mode(kkc->context)].icon;
}

void FcitxKkcUpdateInputModeMenu(struct _FcitxUIMenu *menu)
{
    FcitxKkc *kkc = (FcitxKkc*) menu->priv;
    menu->mark = kkc_context_get_input_mode(kkc->context);
}

boolean FcitxKkcInputModeMenuAction(struct _FcitxUIMenu *menu, int index)
{
    FcitxKkc *kkc = (FcitxKkc*) menu->priv;
    kkc_context_set_input_mode(kkc->context, (KkcInputMode) index);
    return true;
}

void FcitxKkcUpdateInputMode(FcitxKkc* kkc)
{
    KkcInputMode mode = kkc_context_get_input_mode(kkc->context);
    FcitxUISetStatusString(kkc->owner,
                           "kkc-input-mode",
                           _(input_mode_status[mode].label),
                           _(input_mode_status[mode].description));
}

static void  _kkc_input_mode_changed_cb                (GObject    *gobject,
                                                        GParamSpec *pspec,
                                                        gpointer    user_data)
{
    FcitxKkc *kkc = (FcitxKkc*) user_data;
    FcitxKkcUpdateInputMode(kkc);
}

static void*
FcitxKkcCreate(FcitxInstance *instance)
{
    FcitxKkc *kkc = fcitx_utils_new(FcitxKkc);
    bindtextdomain("fcitx-kkc", LOCALEDIR);
    bind_textdomain_codeset("fcitx-kkc", "UTF-8");
    kkc->owner = instance;

    if (!KkcLoadConfig(&kkc->config)) {
        free(kkc);
        return NULL;
    }

#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif
    kkc_init();

    KkcLanguageModel* model = kkc_language_model_load("sorted3", NULL);
    if (!model) {
        free(kkc);
        return NULL;
    }

    FcitxXDGMakeDirUser("kkc/rules");
    FcitxXDGMakeDirUser("kkc/dictionary");

    kkc->model = model;
    kkc->context = kkc_context_new(model);

    if (!FcitxKkcLoadDictionary(kkc) || !FcitxKkcLoadRule(kkc)) {
        g_object_unref(kkc->context);
        free(kkc);
        return NULL;
    }
    kkc_context_set_punctuation_style(kkc->context, KKC_PUNCTUATION_STYLE_JA_JA);
    kkc_context_set_input_mode(kkc->context, KKC_INPUT_MODE_HIRAGANA);
    kkc->tempMsg = FcitxMessagesNew();


    FcitxKkcApplyConfig(kkc);

    FcitxIMIFace iface;
    memset(&iface, 0, sizeof(FcitxIMIFace));
    iface.Init = FcitxKkcInit;
    iface.DoInput = FcitxKkcDoInput;
    iface.DoReleaseInput = FcitxKkcDoReleaseInput;
    iface.GetCandWords = FcitxKkcGetCandWords;
    iface.Save = FcitxKkcSave;
    iface.ResetIM = FcitxKkcResetIM;
    iface.OnClose = FcitxKkcOnClose;

    FcitxInstanceRegisterIMv2(instance, kkc, "kkc", _("Kana Kanji"), "kkc", iface, 1, "ja");


#define INIT_MENU(VARNAME, NAME, I18NNAME, STATUS_NAME, STATUS_ARRAY, SIZE) \
    do { \
        FcitxUIRegisterComplexStatus(instance, kkc, \
            STATUS_NAME, \
            I18NNAME, \
            I18NNAME, \
            NULL, \
            FcitxKkcGet##NAME##IconName \
        ); \
        FcitxMenuInit(&VARNAME); \
        VARNAME.name = strdup(I18NNAME); \
        VARNAME.candStatusBind = strdup(STATUS_NAME); \
        VARNAME.UpdateMenu = FcitxKkcUpdate##NAME##Menu; \
        VARNAME.MenuAction = FcitxKkc##NAME##MenuAction; \
        VARNAME.priv = kkc; \
        VARNAME.isSubMenu = false; \
        int i; \
        for (i = 0; i < SIZE; i ++) \
            FcitxMenuAddMenuItem(&VARNAME, _(STATUS_ARRAY[i].label), MENUTYPE_SIMPLE, NULL); \
        FcitxUIRegisterMenu(instance, &VARNAME); \
        FcitxUISetStatusVisable(instance, STATUS_NAME, false); \
    } while(0)

    INIT_MENU(kkc->inputModeMenu, InputMode, _("Input Mode"), "kkc-input-mode", input_mode_status, KKC_INPUT_MODE_DIRECT);

    kkc->handler = g_signal_connect(kkc->context, "notify::input-mode", G_CALLBACK(_kkc_input_mode_changed_cb), kkc);
    FcitxKkcUpdateInputMode(kkc);

    kkc_context_set_input_mode(kkc->context, kkc->config.initialInputMode);

    FcitxIMEventHook hk;
    hk.arg = kkc;
    hk.func = FcitxKkcResetHook;
    FcitxInstanceRegisterResetInputHook(instance, hk);

    FcitxKkcAddFunctions(instance);
    return kkc;
}

INPUT_RETURN_VALUE FcitxKkcDoInputReal(void* arg, FcitxKeySym sym, unsigned int state)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    state = state & (FcitxKeyState_SimpleMask | _FcitxKeyState_Release);
    KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);

    if (kkc_candidate_list_get_page_visible(kkcCandidates)) {
        if (FcitxHotkeyIsHotKeyDigit(sym, state)) {
            return IRV_TO_PROCESS;
        } else if (FcitxHotkeyIsHotKey(sym, state, kkc->config.prevPageKey)) {
            return IRV_TO_PROCESS;
        } else if (FcitxHotkeyIsHotKey(sym, state, kkc->config.nextPageKey)) {
            return IRV_TO_PROCESS;
        } else if (FcitxHotkeyIsHotKey(sym, state, kkc->config.cursorUpKey)) {
            if (!(state & _FcitxKeyState_Release)) {
                KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
                kkc_candidate_list_cursor_up(kkcCandidates);
                return IRV_DISPLAY_CANDWORDS;
            } else {
                return IRV_TO_PROCESS;
            }
        } else if (FcitxHotkeyIsHotKey(sym, state, kkc->config.cursorDownKey)) {
            if (!(state & _FcitxKeyState_Release)) {
                KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
                kkc_candidate_list_cursor_down(kkcCandidates);
                return IRV_DISPLAY_CANDWORDS;
            } else {
                return IRV_TO_PROCESS;
            }
        }
    }

    FcitxInputState* input = FcitxInstanceGetInputState(kkc->owner);
    uint32_t keycode = FcitxInputStateGetKeyCode(input);
    KkcKeyEvent* key = kkc_key_event_new_from_x_event(sym, keycode, state);
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

    FcitxInstanceSetContext(kkc->owner, CONTEXT_ALTERNATIVE_PREVPAGE_KEY, kkc->config.prevPageKey);
    FcitxInstanceSetContext(kkc->owner, CONTEXT_ALTERNATIVE_NEXTPAGE_KEY, kkc->config.nextPageKey);
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
    state |= _FcitxKeyState_Release;
    return FcitxKkcDoInputReal(arg, sym, state);
}

INPUT_RETURN_VALUE FcitxKkcGetCandWord(void* arg, FcitxCandidateWord* word)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
    int idx = *((int*)word->priv);
    gboolean retval = kkc_candidate_list_select_at(kkcCandidates, idx % kkc_candidate_list_get_page_size(kkcCandidates));
    if (retval) {
        return IRV_DISPLAY_CANDWORDS;
    }

    return IRV_TO_PROCESS;
}

boolean FcitxKkcPaging(void* arg, boolean prev) {
    FcitxKkc *kkc = (FcitxKkc*)arg;
    KkcCandidateList* skkCandList = kkc_context_get_candidates(kkc->context);
    if (kkc_candidate_list_get_page_visible(skkCandList)) {
        if (prev)
            kkc_candidate_list_page_up(skkCandList);
        else
            kkc_candidate_list_page_down(skkCandList);
        FcitxKkcGetCandWords(kkc);
        return true;
    }
    return false;
}

INPUT_RETURN_VALUE FcitxKkcGetCandWords(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(kkc->owner);
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    FcitxMessages* clientPreedit = FcitxInputStateGetClientPreedit(input);
    FcitxMessages* preedit = FcitxInputStateGetPreedit(input);
    FcitxInstanceCleanInputWindow(kkc->owner);

    FcitxMessages* message = FcitxInstanceICSupportPreedit(kkc->owner, FcitxInstanceGetCurrentIC(kkc->owner)) ? clientPreedit : preedit;

    FcitxCandidateWordSetChoose(candList, DIGIT_STR_CHOOSE);
    FcitxCandidateWordSetPageSize(candList, kkc->config.pageSize);
    FcitxCandidateWordSetLayoutHint(candList, kkc->config.candidateLayout);
    FcitxInputStateSetShowCursor(input, true);

    KkcSegmentList* segments = kkc_context_get_segments(kkc->context);
    if (kkc_segment_list_get_cursor_pos(segments) >= 0) {
        int i = 0;
        int offset = 0;
        for (i = 0; i < kkc_segment_list_get_size(segments); i ++) {
            KkcSegment* segment = kkc_segment_list_get(segments, i);
            const gchar* str = kkc_segment_get_output(segment);
            FcitxMessageType messageType = MSG_INPUT;
            if (i < kkc_segment_list_get_cursor_pos(segments)) {
                offset += strlen(str);
            }
            if (i == kkc_segment_list_get_cursor_pos(segments)) {
                messageType = (FcitxMessageType) (MSG_HIGHLIGHT | MSG_OTHER);
            }
            FcitxMessagesAddMessageAtLast(message, messageType, "%s", str);
        }

        if (message == clientPreedit) {
            FcitxInputStateSetClientCursorPos(input, offset);
        } else {
            FcitxInputStateSetCursorPos(input, offset);
        }
    } else {
        gchar* str = kkc_context_get_input(kkc->context);
        if (str && str[0]) {
            FcitxMessagesAddMessageAtLast(message, MSG_INPUT, "%s", str);

            if (message == clientPreedit) {
                FcitxInputStateSetClientCursorPos(input, strlen(str));
            } else {
                FcitxInputStateSetCursorPos(input, strlen(str));
            }
        }
        g_free(str);
    }

    KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
    if (kkc_candidate_list_get_page_visible(kkcCandidates)) {
        int i, j;
        guint size = kkc_candidate_list_get_size(kkcCandidates);
        gint cursor_pos = kkc_candidate_list_get_cursor_pos(kkcCandidates);
        guint page_start = kkc_candidate_list_get_page_start(kkcCandidates);
        guint page_size = kkc_candidate_list_get_page_size(kkcCandidates);
        for (i = kkc_candidate_list_get_page_start(kkcCandidates), j = 0; i < size; i ++, j++) {
            FcitxCandidateWord word;
            word.callback = FcitxKkcGetCandWord;
            word.extraType = MSG_OTHER;
            word.owner = kkc;
            int* id = fcitx_utils_new(int);
            *id = j;
            word.priv = id;
            word.strExtra = NULL;
            word.strExtra = MSG_TIPS;
            KkcCandidate* kkcCandidate = kkc_candidate_list_get(kkcCandidates, i);
            if (kkc->config.showAnnotation && kkc_candidate_get_annotation(kkcCandidate)) {
                fcitx_utils_alloc_cat_str(word.strExtra, " [", kkc_candidate_get_annotation(kkcCandidate), "]");
            }
            word.strWord = strdup(kkc_candidate_get_text(kkcCandidate));
            if (i == cursor_pos) {
                word.wordType = MSG_CANDIATE_CURSOR;
            } else {
                word.wordType = MSG_OTHER;
            }

            FcitxCandidateWordAppend(candList, &word);
        }
        FcitxCandidateWordSetFocus(candList, cursor_pos - page_start);

        FcitxCandidateWordSetOverridePaging(candList,
                                            (cursor_pos - page_start) >= page_size,
                                            (size - page_start) / page_size != (cursor_pos - page_start) / page_size,
                                            FcitxKkcPaging,
                                            kkc,
                                            NULL);
    }

    if (kkc_context_has_output(kkc->context)) {
        gchar* str = kkc_context_poll_output(kkc->context);
        FcitxInstanceCommitString(kkc->owner, FcitxInstanceGetCurrentIC(kkc->owner), str);
        g_free(str);
    }

    return IRV_DISPLAY_CANDWORDS;
}

void FcitxKkcResetIM(void* arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    kkc_context_reset(kkc->context);
}

void FcitxKkcOnClose(void* arg, FcitxIMCloseEventType event)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    if (event == CET_LostFocus) {
        // TODO
    } else if (event == CET_ChangeByUser) {
        kkc_context_reset(kkc->context);
    } else if (event == CET_ChangeByInactivate) {
        KkcSegmentList* segments = kkc_context_get_segments(kkc->context);
        FcitxGlobalConfig* config = FcitxInstanceGetGlobalConfig(kkc->owner);
        if (config->bSendTextWhenSwitchEng) {
            FcitxMessagesSetMessageCount(kkc->tempMsg, 0);
            if (kkc_segment_list_get_cursor_pos(segments) >= 0) {
                int i = 0;
                for (i = 0; i < kkc_segment_list_get_size(segments); i ++) {
                    KkcSegment* segment = kkc_segment_list_get(segments, i);
                    const gchar* str = kkc_segment_get_output(segment);
                    FcitxMessagesAddMessageAtLast(kkc->tempMsg, MSG_INPUT, "%s", str);
                }
            } else {
                gchar* str = kkc_context_get_input(kkc->context);
                FcitxMessagesAddMessageAtLast(kkc->tempMsg, MSG_INPUT, "%s", str);
                g_free(str);
            }
            if (FcitxMessagesGetMessageCount(kkc->tempMsg) > 0) {
                char* commit = FcitxUIMessagesToCString(kkc->tempMsg);
                FcitxInstanceCommitString(kkc->owner, FcitxInstanceGetCurrentIC(kkc->owner), commit);
                free(commit);
            }
        }
        kkc_context_reset(kkc->context);
    }
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

    g_signal_handler_disconnect(kkc->context, kkc->handler);
    g_object_unref(kkc->context);

    free(kkc->tempMsg);

    free(kkc);
}

static void
FcitxKkcReloadConfig(void *arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    KkcLoadConfig(&kkc->config);
    FcitxKkcApplyConfig(kkc);
    FcitxKkcLoadDictionary(kkc);
    FcitxKkcLoadRule(kkc);
}

void FcitxKkcApplyConfig(FcitxKkc* kkc)
{
    KkcCandidateList* kkcCandidates = kkc_context_get_candidates(kkc->context);
    kkc_candidate_list_set_page_start(kkcCandidates, kkc->config.nTriggersToShowCandWin);
    kkc_candidate_list_set_page_size(kkcCandidates, kkc->config.pageSize);
    kkc_context_set_punctuation_style(kkc->context, kkc->config.punctuationStyle);
    kkc_context_set_auto_correct(kkc->context, kkc->config.autoCorrect);
}

void FcitxKkcResetHook(void *arg)
{
    FcitxKkc *kkc = (FcitxKkc*)arg;
    FcitxIM* im = FcitxInstanceGetCurrentIM(kkc->owner);
#define RESET_STATUS(STATUS_NAME) \
    if (im && strcmp(im->uniqueName, "kkc") == 0) \
        FcitxUISetStatusVisable(kkc->owner, STATUS_NAME, true); \
    else \
        FcitxUISetStatusVisable(kkc->owner, STATUS_NAME, false);

    RESET_STATUS("kkc-input-mode")
}

boolean FcitxKkcLoadDictionary(FcitxKkc* kkc)
{
    FILE* fp = FcitxXDGGetFileWithPrefix("kkc", "dictionary_list", "r", NULL);
    if (!fp) {
        return false;
    }

    UT_array dictionaries;
    utarray_init(&dictionaries, &dict_icd);

    char *buf = NULL;
    size_t len = 0;
    char *trimmed = NULL;

    while (getline(&buf, &len, fp) != -1) {
        if (trimmed)
            free(trimmed);
        trimmed = fcitx_utils_trim(buf);

        UT_array* list = fcitx_utils_split_string(trimmed, ',');
        do {
            if (utarray_len(list) < 3) {
                break;
            }
            boolean typeFile = false;
            char* path = NULL;
            int mode = 0;
            utarray_foreach(item, list, char*) {
                char* key = *item;
                char* value = strchr(*item, '=');
                if (!value)
                    continue;
                *value = '\0';
                value++;

                if (strcmp(key, "type") == 0) {
                    if (strcmp(value, "file") == 0) {
                        typeFile = true;
                    }
                } else if (strcmp(key, "file") == 0) {
                    path = value;
                } else if (strcmp(key, "mode") == 0) {
                    if (strcmp(value, "readonly") == 0) {
                        mode = 1;
                    } else if (strcmp(value, "readwrite") == 0) {
                        mode = 2;
                    }
                }
            }

            if (mode == 0 || path == NULL || !typeFile) {
                break;
            }

            if (mode == 1) {
                KkcSystemSegmentDictionary* dict = kkc_system_segment_dictionary_new(path, "EUC-JP", NULL);
                if (dict) {
                    utarray_push_back(&dictionaries, &dict);
                }
            } else {
                char* needfree = NULL;
                char* realpath = NULL;
                if (strncmp(path, "$FCITX_CONFIG_DIR/", strlen("$FCITX_CONFIG_DIR/")) == 0) {
                    FcitxXDGGetFileUserWithPrefix("", path + strlen("$FCITX_CONFIG_DIR/"), NULL, &needfree);
                    realpath = needfree;
                } else {
                    realpath = path;
                }
                KkcUserDictionary* userdict = kkc_user_dictionary_new(realpath, NULL);
                if (needfree) {
                    free(needfree);
                }
                if (userdict) {
                    utarray_push_back(&dictionaries, &userdict);
                }
            }
        } while(0);
        fcitx_utils_free_string_list(list);
    }

    if (buf)
        free(buf);
    if (trimmed)
        free(trimmed);

    boolean result = false;
    if (utarray_len(&dictionaries) != 0) {
        result = true;
        KkcDictionaryList* kkcdicts = kkc_context_get_dictionaries(kkc->context);
        kkc_dictionary_list_clear(kkcdicts);
        utarray_foreach(dict, &dictionaries, KkcDictionary*) {
            kkc_dictionary_list_add(kkcdicts, KKC_DICTIONARY(*dict));
        }
    }

    utarray_done(&dictionaries);
    return result;
}

boolean FcitxKkcLoadRule(FcitxKkc* kkc)
{
    FILE* fp = FcitxXDGGetFileWithPrefix("kkc", "rule", "r", NULL);
    KkcRuleMetadata* meta = NULL;

    do {
        if (!fp) {
            break;
        }

        char* line = NULL;
        size_t bufsize = 0;
        getline(&line, &bufsize, fp);
        fclose(fp);

        if (!line) {
            break;
        }

        char* trimmed = fcitx_utils_trim(line);
        meta = kkc_rule_metadata_find(trimmed);
        free(trimmed);
        free(line);
    } while(0);

    if (!meta) {
        meta = kkc_rule_metadata_find("default");
        if (!meta) {
            return false;
        }
    }

    char* fcitxBasePath = NULL;
    FcitxXDGGetFileUserWithPrefix("kkc", "rules", NULL, &fcitxBasePath);
    KkcUserRule* userRule = kkc_user_rule_new(meta, fcitxBasePath, "fcitx-kkc", NULL);
    if (!userRule) {
        return false;
    }

    kkc_context_set_typing_rule(kkc->context, KKC_RULE(userRule));
    return true;
}


#include "fcitx-kkc-addfunctions.h"
