/*
 *  Copyright (C) 2024 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AutoTypeWayland.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "autotype/AutoTypeAction.h"
#include "core/Tools.h"
#include "gui/osutils/nixutils/X11Funcs.h"

#include <fcntl.h>
#include <unistd.h>

#include <linux/uinput.h>

// taken from ydotool
#define FLAG_UPPERCASE 0x00010000

static const int _enable_keys[] = {KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_LEFTSHIFT, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT, KEY_KPASTERISK, KEY_LEFTALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_KP7, KEY_KP8, KEY_KP9, KEY_KPMINUS, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KPPLUS, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP0, KEY_KPDOT, KEY_ZENKAKUHANKAKU, KEY_102ND, KEY_F11, KEY_F12, KEY_RO, KEY_KATAKANA, KEY_HIRAGANA, KEY_HENKAN, KEY_KATAKANAHIRAGANA, KEY_MUHENKAN, KEY_KPJPCOMMA, KEY_KPENTER, KEY_RIGHTCTRL, KEY_KPSLASH, KEY_SYSRQ, KEY_RIGHTALT, KEY_LINEFEED, KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_LEFT, KEY_RIGHT, KEY_END, KEY_DOWN, KEY_PAGEDOWN, KEY_INSERT, KEY_DELETE, KEY_MACRO, KEY_MUTE, KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_POWER, KEY_KPEQUAL, KEY_KPPLUSMINUS, KEY_PAUSE, KEY_SCALE, KEY_KPCOMMA, KEY_HANGEUL, KEY_HANGUEL, KEY_HANJA, KEY_YEN, KEY_LEFTMETA, KEY_RIGHTMETA, KEY_COMPOSE, KEY_STOP, KEY_AGAIN, KEY_PROPS, KEY_UNDO, KEY_FRONT, KEY_COPY, KEY_OPEN, KEY_PASTE, KEY_FIND, KEY_CUT, KEY_HELP, KEY_MENU, KEY_CALC, KEY_SETUP, KEY_SLEEP, KEY_WAKEUP, KEY_FILE, KEY_SENDFILE, KEY_DELETEFILE, KEY_XFER, KEY_PROG1, KEY_PROG2, KEY_WWW, KEY_MSDOS, KEY_COFFEE, KEY_SCREENLOCK, KEY_ROTATE_DISPLAY, KEY_DIRECTION, KEY_CYCLEWINDOWS, KEY_MAIL, KEY_BOOKMARKS, KEY_COMPUTER, KEY_BACK, KEY_FORWARD, KEY_CLOSECD, KEY_EJECTCD, KEY_EJECTCLOSECD, KEY_NEXTSONG, KEY_PLAYPAUSE, KEY_PREVIOUSSONG, KEY_STOPCD, KEY_RECORD, KEY_REWIND, KEY_PHONE, KEY_ISO, KEY_CONFIG, KEY_HOMEPAGE, KEY_REFRESH, KEY_EXIT, KEY_MOVE, KEY_EDIT, KEY_SCROLLUP, KEY_SCROLLDOWN, KEY_KPLEFTPAREN, KEY_KPRIGHTPAREN, KEY_NEW, KEY_REDO, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_PLAYCD, KEY_PAUSECD, KEY_PROG3, KEY_PROG4, KEY_DASHBOARD, KEY_SUSPEND, KEY_CLOSE, KEY_PLAY, KEY_FASTFORWARD, KEY_BASSBOOST, KEY_PRINT, KEY_HP, KEY_CAMERA, KEY_SOUND, KEY_QUESTION, KEY_EMAIL, KEY_CHAT, KEY_SEARCH, KEY_CONNECT, KEY_FINANCE, KEY_SPORT, KEY_SHOP, KEY_ALTERASE, KEY_CANCEL, KEY_BRIGHTNESSDOWN, KEY_BRIGHTNESSUP, KEY_MEDIA, KEY_SWITCHVIDEOMODE, KEY_KBDILLUMTOGGLE, KEY_KBDILLUMDOWN, KEY_KBDILLUMUP, KEY_SEND, KEY_REPLY, KEY_FORWARDMAIL, KEY_SAVE, KEY_DOCUMENTS, KEY_BATTERY, KEY_BLUETOOTH, KEY_WLAN, KEY_UWB, KEY_UNKNOWN, KEY_VIDEO_NEXT, KEY_VIDEO_PREV, KEY_BRIGHTNESS_CYCLE, KEY_BRIGHTNESS_AUTO, KEY_BRIGHTNESS_ZERO, KEY_DISPLAY_OFF, KEY_WWAN, KEY_WIMAX, KEY_RFKILL, KEY_MICMUTE, KEY_OK, KEY_SELECT, KEY_GOTO, KEY_CLEAR, KEY_POWER2, KEY_OPTION, KEY_INFO, KEY_TIME, KEY_VENDOR, KEY_ARCHIVE, KEY_PROGRAM, KEY_CHANNEL, KEY_FAVORITES, KEY_EPG, KEY_PVR, KEY_MHP, KEY_LANGUAGE, KEY_TITLE, KEY_SUBTITLE, KEY_ANGLE, KEY_ZOOM, KEY_MODE, KEY_KEYBOARD, KEY_SCREEN, KEY_PC, KEY_TV, KEY_TV2, KEY_VCR, KEY_VCR2, KEY_SAT, KEY_SAT2, KEY_CD, KEY_TAPE, KEY_RADIO, KEY_TUNER, KEY_PLAYER, KEY_TEXT, KEY_DVD, KEY_AUX, KEY_MP3, KEY_AUDIO, KEY_VIDEO, KEY_DIRECTORY, KEY_LIST, KEY_MEMO, KEY_CALENDAR, KEY_RED, KEY_GREEN, KEY_YELLOW, KEY_BLUE, KEY_CHANNELUP, KEY_CHANNELDOWN, KEY_FIRST, KEY_LAST, KEY_AB, KEY_NEXT, KEY_RESTART, KEY_SLOW, KEY_SHUFFLE, KEY_BREAK, KEY_PREVIOUS, KEY_DIGITS, KEY_TEEN, KEY_TWEN, KEY_VIDEOPHONE, KEY_GAMES, KEY_ZOOMIN, KEY_ZOOMOUT, KEY_ZOOMRESET, KEY_WORDPROCESSOR, KEY_EDITOR, KEY_SPREADSHEET, KEY_GRAPHICSEDITOR, KEY_PRESENTATION, KEY_DATABASE, KEY_NEWS, KEY_VOICEMAIL, KEY_ADDRESSBOOK, KEY_MESSENGER, KEY_DISPLAYTOGGLE, KEY_BRIGHTNESS_TOGGLE, KEY_SPELLCHECK, KEY_LOGOFF, KEY_DOLLAR, KEY_EURO, KEY_FRAMEBACK, KEY_FRAMEFORWARD, KEY_CONTEXT_MENU, KEY_MEDIA_REPEAT, KEY_10CHANNELSUP, KEY_10CHANNELSDOWN, KEY_IMAGES, KEY_DEL_EOL, KEY_DEL_EOS, KEY_INS_LINE, KEY_DEL_LINE, KEY_FN, KEY_FN_ESC, KEY_FN_F1, KEY_FN_F2, KEY_FN_F3, KEY_FN_F4, KEY_FN_F5, KEY_FN_F6, KEY_FN_F7, KEY_FN_F8, KEY_FN_F9, KEY_FN_F10, KEY_FN_F11, KEY_FN_F12, KEY_FN_1, KEY_FN_2, KEY_FN_D, KEY_FN_E, KEY_FN_F, KEY_FN_S, KEY_FN_B, KEY_BRL_DOT1, KEY_BRL_DOT2, KEY_BRL_DOT3, KEY_BRL_DOT4, KEY_BRL_DOT5, KEY_BRL_DOT6, KEY_BRL_DOT7, KEY_BRL_DOT8, KEY_BRL_DOT9, KEY_BRL_DOT10, KEY_NUMERIC_0, KEY_NUMERIC_1, KEY_NUMERIC_2, KEY_NUMERIC_3, KEY_NUMERIC_4, KEY_NUMERIC_5, KEY_NUMERIC_6, KEY_NUMERIC_7, KEY_NUMERIC_8, KEY_NUMERIC_9, KEY_NUMERIC_STAR, KEY_NUMERIC_POUND, KEY_NUMERIC_A, KEY_NUMERIC_B, KEY_NUMERIC_C, KEY_NUMERIC_D, KEY_CAMERA_FOCUS, KEY_WPS_BUTTON, KEY_TOUCHPAD_TOGGLE, KEY_TOUCHPAD_ON, KEY_TOUCHPAD_OFF, KEY_CAMERA_ZOOMIN, KEY_CAMERA_ZOOMOUT, KEY_CAMERA_UP, KEY_CAMERA_DOWN, KEY_CAMERA_LEFT, KEY_CAMERA_RIGHT, KEY_ATTENDANT_ON, KEY_ATTENDANT_OFF, KEY_ATTENDANT_TOGGLE, KEY_LIGHTS_TOGGLE, KEY_ALS_TOGGLE, KEY_BUTTONCONFIG, KEY_TASKMANAGER, KEY_JOURNAL, KEY_CONTROLPANEL, KEY_APPSELECT, KEY_SCREENSAVER, KEY_VOICECOMMAND, KEY_BRIGHTNESS_MIN, KEY_BRIGHTNESS_MAX, KEY_KBDINPUTASSIST_PREV, KEY_KBDINPUTASSIST_NEXT, KEY_KBDINPUTASSIST_PREVGROUP, KEY_KBDINPUTASSIST_NEXTGROUP, KEY_KBDINPUTASSIST_ACCEPT, KEY_KBDINPUTASSIST_CANCEL, KEY_RIGHT_UP, KEY_RIGHT_DOWN, KEY_LEFT_UP, KEY_LEFT_DOWN, KEY_ROOT_MENU, KEY_MEDIA_TOP_MENU, KEY_NUMERIC_11, KEY_NUMERIC_12, KEY_AUDIO_DESC, KEY_3D_MODE, KEY_NEXT_FAVORITE, KEY_STOP_RECORD, KEY_PAUSE_RECORD, KEY_VOD, KEY_UNMUTE, KEY_FASTREVERSE, KEY_SLOWREVERSE, KEY_DATA, KEY_MIN_INTERESTING, BTN_MISC, BTN_0, BTN_1, BTN_2, BTN_3, BTN_4, BTN_5, BTN_6, BTN_7, BTN_8, BTN_9, BTN_MOUSE, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA, BTN_FORWARD, BTN_BACK, BTN_TASK, BTN_JOYSTICK, BTN_TRIGGER, BTN_THUMB, BTN_THUMB2, BTN_TOP, BTN_TOP2, BTN_PINKIE, BTN_BASE, BTN_BASE2, BTN_BASE3, BTN_BASE4, BTN_BASE5, BTN_BASE6, BTN_DEAD, BTN_GAMEPAD, BTN_SOUTH, BTN_A, BTN_EAST, BTN_B, BTN_C, BTN_NORTH, BTN_X, BTN_WEST, BTN_Y, BTN_Z, BTN_TL, BTN_TR, BTN_TL2, BTN_TR2, BTN_SELECT, BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR, BTN_DIGI, BTN_TOOL_PEN, BTN_TOOL_RUBBER, BTN_TOOL_BRUSH, BTN_TOOL_PENCIL, BTN_TOOL_AIRBRUSH, BTN_TOOL_FINGER, BTN_TOOL_MOUSE, BTN_TOOL_LENS, BTN_TOOL_QUINTTAP, BTN_TOUCH, BTN_STYLUS, BTN_STYLUS2, BTN_TOOL_DOUBLETAP, BTN_TOOL_TRIPLETAP, BTN_TOOL_QUADTAP, BTN_WHEEL, BTN_GEAR_DOWN, BTN_GEAR_UP, BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT, BTN_TRIGGER_HAPPY, BTN_TRIGGER_HAPPY1, BTN_TRIGGER_HAPPY2, BTN_TRIGGER_HAPPY3, BTN_TRIGGER_HAPPY4, BTN_TRIGGER_HAPPY5, BTN_TRIGGER_HAPPY6, BTN_TRIGGER_HAPPY7, BTN_TRIGGER_HAPPY8, BTN_TRIGGER_HAPPY9, BTN_TRIGGER_HAPPY10, BTN_TRIGGER_HAPPY11, BTN_TRIGGER_HAPPY12, BTN_TRIGGER_HAPPY13, BTN_TRIGGER_HAPPY14, BTN_TRIGGER_HAPPY15, BTN_TRIGGER_HAPPY16, BTN_TRIGGER_HAPPY17, BTN_TRIGGER_HAPPY18, BTN_TRIGGER_HAPPY19, BTN_TRIGGER_HAPPY20, BTN_TRIGGER_HAPPY21, BTN_TRIGGER_HAPPY22, BTN_TRIGGER_HAPPY23, BTN_TRIGGER_HAPPY24, BTN_TRIGGER_HAPPY25, BTN_TRIGGER_HAPPY26, BTN_TRIGGER_HAPPY27, BTN_TRIGGER_HAPPY28, BTN_TRIGGER_HAPPY29, BTN_TRIGGER_HAPPY30, BTN_TRIGGER_HAPPY31, BTN_TRIGGER_HAPPY32, BTN_TRIGGER_HAPPY33, BTN_TRIGGER_HAPPY34, BTN_TRIGGER_HAPPY35, BTN_TRIGGER_HAPPY36, BTN_TRIGGER_HAPPY37, BTN_TRIGGER_HAPPY38, BTN_TRIGGER_HAPPY39, BTN_TRIGGER_HAPPY40};
static const struct uinput_setup _usetup = {.id = {.bustype = BUS_VIRTUAL, .vendor = 0xDEAD, .product = 0xBEEF, .version = 1}, .name = "keepassxc virtual device", .ff_effects_max = 0};
static const int _ascii2keycode_map[128] = {
    -1,-1,-1,-1,-1,-1,-1,-1,
    -1,KEY_TAB,KEY_ENTER,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,
    KEY_SPACE,KEY_1|FLAG_UPPERCASE,KEY_APOSTROPHE|FLAG_UPPERCASE,KEY_3|FLAG_UPPERCASE,KEY_4|FLAG_UPPERCASE,KEY_5|FLAG_UPPERCASE,KEY_7|FLAG_UPPERCASE,KEY_APOSTROPHE,
    KEY_9|FLAG_UPPERCASE,KEY_0|FLAG_UPPERCASE,KEY_8|FLAG_UPPERCASE,KEY_EQUAL|FLAG_UPPERCASE,KEY_COMMA,KEY_MINUS,KEY_DOT,KEY_SLASH,
    KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,
    KEY_8,KEY_9,KEY_SEMICOLON|FLAG_UPPERCASE,KEY_SEMICOLON,KEY_COMMA|FLAG_UPPERCASE,KEY_EQUAL,KEY_DOT|FLAG_UPPERCASE,KEY_SLASH|FLAG_UPPERCASE,
    KEY_2|FLAG_UPPERCASE,KEY_A|FLAG_UPPERCASE,KEY_B|FLAG_UPPERCASE,KEY_C|FLAG_UPPERCASE,KEY_D|FLAG_UPPERCASE,KEY_E|FLAG_UPPERCASE,KEY_F|FLAG_UPPERCASE,KEY_G|FLAG_UPPERCASE,
    KEY_H|FLAG_UPPERCASE,KEY_I|FLAG_UPPERCASE,KEY_J|FLAG_UPPERCASE,KEY_K|FLAG_UPPERCASE,KEY_L|FLAG_UPPERCASE,KEY_M|FLAG_UPPERCASE,KEY_N|FLAG_UPPERCASE,KEY_O|FLAG_UPPERCASE,
    KEY_P|FLAG_UPPERCASE,KEY_Q|FLAG_UPPERCASE,KEY_R|FLAG_UPPERCASE,KEY_S|FLAG_UPPERCASE,KEY_T|FLAG_UPPERCASE,KEY_U|FLAG_UPPERCASE,KEY_V|FLAG_UPPERCASE,KEY_W|FLAG_UPPERCASE,
    KEY_X|FLAG_UPPERCASE,KEY_Y|FLAG_UPPERCASE,KEY_Z|FLAG_UPPERCASE,KEY_LEFTBRACE,KEY_BACKSLASH,KEY_RIGHTBRACE,KEY_6|FLAG_UPPERCASE,KEY_MINUS|FLAG_UPPERCASE,
    KEY_GRAVE,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,
    KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,
    KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,
    KEY_X,KEY_Y,KEY_Z,KEY_LEFTBRACE|FLAG_UPPERCASE,KEY_BACKSLASH|FLAG_UPPERCASE,KEY_RIGHTBRACE|FLAG_UPPERCASE,KEY_GRAVE|FLAG_UPPERCASE,-1
};


AutoTypePlatformWayland::AutoTypePlatformWayland()
{
    uifd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    ioctl(uifd, UI_SET_EVBIT, EV_KEY);
    for (size_t i = 0; i < sizeof(_enable_keys) / sizeof(int); i++)
    {
        ioctl(uifd, UI_SET_KEYBIT, _enable_keys[i]);
    }
    ioctl(uifd, UI_DEV_SETUP, &_usetup);
    ioctl(uifd, UI_DEV_CREATE);
}

void AutoTypePlatformWayland::sendUiKey(uint16_t code, int32_t value)
{
    input_event ie {};
    ie.type = EV_KEY;
    ie.code = code;
    ie.value = value;
    write(uifd, &ie, sizeof(ie));
    ie.type = EV_SYN;
    ie.code = SYN_REPORT;
    ie.value = 0;
    write(uifd, &ie, sizeof(ie));
}

bool AutoTypePlatformWayland::isAvailable()
{
    return true;
}

void AutoTypePlatformWayland::unload()
{
    ioctl(uifd, UI_DEV_DESTROY);
    close(uifd);
}

QString AutoTypePlatformWayland::activeWindowTitle()
{
    // Ask focused-window-dbus extension nicely
    // https://extensions.gnome.org/extension/5592/focused-window-d-bus/
    // https://github.com/flexagoon/focused-window-dbus
    QDBusMessage reply = QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall("org.gnome.Shell", "/org/gnome/shell/extensions/FocusedWindow", "org.gnome.shell.extensions.FocusedWindow", "Get"));
    if (reply.type() == QDBusMessage::ReplyMessage)
    {
        QString strReply = reply.arguments().at(0).toString();
        QJsonParseError parseError;
        QJsonDocument jsonReply = QJsonDocument::fromJson(strReply.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError)
        {
            QJsonObject jsonData = jsonReply.object();
            if (jsonData.contains("title"))
            {
                return jsonData["title"].toString();
            }
            else
            {
                qWarning() << "AutoTypePlatformWayland::activeWindowTitle() no title in reply " << strReply;
            }
        }
        else
        {
            qWarning() << "AutoTypePlatformWayland::activeWindowTitle() got error " << parseError.errorString() << " parsing reply " << strReply;
        }
    }
    else
    {
        qWarning() << "AutoTypePlatformWayland::activeWindowTitle() got unexpected reply (no FocusedWindow extension present?): " << reply;
    }

    return {};
}

WId AutoTypePlatformWayland::activeWindow()
{
    return 0;
}

AutoTypeExecutor* AutoTypePlatformWayland::createExecutor()
{
    return new AutoTypeExecutorWayland(this);
}

bool AutoTypePlatformWayland::raiseWindow(WId window)
{
    Q_UNUSED(window)
    return false;
}

QStringList AutoTypePlatformWayland::windowTitles()
{
    return {};
}

AutoTypeExecutorWayland::AutoTypeExecutorWayland(AutoTypePlatformWayland* platform)
    : m_platform(platform)
{
}

AutoTypeAction::Result AutoTypeExecutorWayland::execBegin(const AutoTypeBegin* action)
{
    Q_UNUSED(action)
    return AutoTypeAction::Result::Ok();
}

AutoTypeAction::Result AutoTypeExecutorWayland::execType(const AutoTypeKey* action)
{
    int kdef = -1;

    switch(action->key)
    {
        case Qt::Key_Tab:
            kdef = KEY_TAB;
            break;
        case Qt::Key_Enter:
            kdef = KEY_ENTER;
            break;
        case Qt::Key_Space:
            kdef = KEY_SPACE;
            break;
        case Qt::Key_Up:
            kdef = KEY_UP;
            break;
        case Qt::Key_Down:
            kdef = KEY_DOWN;
            break;
        case Qt::Key_Left:
            kdef = KEY_LEFT;
            break;
        case Qt::Key_Right:
            kdef = KEY_RIGHT;
            break;
        case Qt::Key_Insert:
            kdef = KEY_INSERT;
            break;
        case Qt::Key_Delete:
            kdef = KEY_DELETE;
            break;
        case Qt::Key_Home:
            kdef = KEY_HOME;
            break;
        case Qt::Key_End:
            kdef = KEY_END;
            break;
        case Qt::Key_PageUp:
            kdef = KEY_PAGEUP;
            break;
        case Qt::Key_PageDown:
            kdef = KEY_PAGEDOWN;
            break;
        case Qt::Key_Backspace:
            kdef = KEY_BACKSPACE;
            break;
        case Qt::Key_Pause:
            kdef = KEY_PAUSE;
            break;
        case Qt::Key_CapsLock:
            kdef = KEY_CAPSLOCK;
            break;
        case Qt::Key_Escape:
            kdef = KEY_ESC;
            break;
        case Qt::Key_Help:
            kdef = KEY_HELP;
            break;
        case Qt::Key_NumLock:
            kdef = KEY_NUMLOCK;
            break;
        case Qt::Key_Print:
            kdef = KEY_PRINT;
            break;
        case Qt::Key_ScrollLock:
            kdef = KEY_SCROLLLOCK;
            break;
        case Qt::Key_Shift:
            kdef = KEY_LEFTSHIFT;
            break;
        case Qt::Key_Control:
            kdef = KEY_LEFTCTRL;
            break;
        case Qt::Key_Alt:
            kdef = KEY_LEFTALT;
            break;
        default:
            if (action->key >= Qt::Key_F1 && action->key <= Qt::Key_F16)
            {
                kdef = KEY_F1 + (action->key - Qt::Key_F1);
            }
            else if (action->key >= Qt::Key_Space && action->key <= Qt::Key_AsciiTilde)
            {
                kdef = _ascii2keycode_map[action->key];
            }
            else
            {
                int c = action->character.toLatin1();
                kdef = ((c >= 0) && (c < 128)) ? _ascii2keycode_map[c] : -1;
            }
    }

    if (kdef > 0)
    {
        uint16_t kc = kdef & 0xffff;

        if (action->modifiers.testFlag(Qt::ShiftModifier) || (kdef & FLAG_UPPERCASE))
        {
            m_platform->sendUiKey(KEY_LEFTSHIFT, 1);
        }
        if (action->modifiers.testFlag(Qt::ControlModifier)) {
            m_platform->sendUiKey(KEY_LEFTCTRL, 1);
        }
        if (action->modifiers.testFlag(Qt::AltModifier)) {
            m_platform->sendUiKey(KEY_LEFTALT, 1);
        }
        if (action->modifiers.testFlag(Qt::MetaModifier)) {
            m_platform->sendUiKey(KEY_LEFTMETA, 1);
        }

        m_platform->sendUiKey(kc, 1);
        Tools::sleep(20);
        m_platform->sendUiKey(kc, 0);

        if (action->modifiers.testFlag(Qt::MetaModifier)) {
            m_platform->sendUiKey(KEY_LEFTMETA, 0);
        }
        if (action->modifiers.testFlag(Qt::AltModifier)) {
            m_platform->sendUiKey(KEY_LEFTALT, 0);
        }
        if (action->modifiers.testFlag(Qt::ControlModifier)) {
            m_platform->sendUiKey(KEY_LEFTCTRL, 0);
        }
        if (action->modifiers.testFlag(Qt::ShiftModifier) || (kdef & FLAG_UPPERCASE)) {
            m_platform->sendUiKey(KEY_LEFTSHIFT, 0);
        }

        Tools::sleep(execDelayMs);
    }
    else
    {
        qWarning() << "ExecType unknown: key " << action->key << " knkc " << qtToNativeKeyCode(action->key) << " char " << action->character << " cnkc " << qcharToNativeKeyCode(action->character);
    }

    return AutoTypeAction::Result::Ok();
}

AutoTypeAction::Result AutoTypeExecutorWayland::execClearField(const AutoTypeClearField* action)
{
    Q_UNUSED(action)
    execType(new AutoTypeKey(Qt::Key_Home));
    execType(new AutoTypeKey(Qt::Key_End, Qt::ShiftModifier));
    execType(new AutoTypeKey(Qt::Key_Backspace));

    return AutoTypeAction::Result::Ok();
}
