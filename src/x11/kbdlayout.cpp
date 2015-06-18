#include <QCoreApplication>
#include <QAbstractNativeEventFilter>
#include <QDebug>
#include <QLabel>
#include <QFile>
#include <QDomDocument>
#include "kbdlayout.h"
#include <xkbcommon/xkbcommon-x11.h>
#include <xcb/xcb.h>
#define explicit _explicit
#include <xcb/xkb.h>
#include "../kbdinfo.h"

namespace pimpl {

class X11Kbd: public QAbstractNativeEventFilter
{
public:
    X11Kbd(::X11Kbd *pub):
        m_pub(pub)
    {}

    bool init()
    {
        m_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        m_connection = xcb_connect(0, 0);

        if (!m_connection || xcb_connection_has_error(m_connection)){
            qWarning() << "Couldn't connect to X server: error code"
                       << (m_connection ? xcb_connection_has_error(m_connection) : -1);
            return false;
        }

        xkb_x11_setup_xkb_extension(m_connection,
            XKB_X11_MIN_MAJOR_XKB_VERSION,
            XKB_X11_MIN_MINOR_XKB_VERSION,
            XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
            NULL, NULL, &m_eventType, NULL
        );

        m_deviceId = xkb_x11_get_core_keyboard_device_id(m_connection);
        qApp->installNativeEventFilter(this);

        readState();
        return true;
    }

    virtual ~X11Kbd()
    {
        xkb_state_unref(m_state);
        xkb_keymap_unref(m_keymap);
        xcb_disconnect(m_connection);
        xkb_context_unref(m_context);
    }

    bool isEnabled() const
    { return true;  }

    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *)
    {
        if (eventType != "xcb_generic_event_t")
            return false;

        xcb_generic_event_t *event = static_cast<xcb_generic_event_t*>(message);
        if ((event->response_type & ~0x80) == m_eventType){
            xcb_xkb_state_notify_event_t *sevent = reinterpret_cast<xcb_xkb_state_notify_event_t*>(event);
            switch(sevent->xkbType){
            case XCB_XKB_STATE_NOTIFY:
                emit m_pub->layoutChanged(sevent->group);
                break;
            case XCB_XKB_NEW_KEYBOARD_NOTIFY:
                readState();
                break;
            }
        }

        emit m_pub->checkState();
        return false;
    }

    void readKbdInfo(KbdInfo & info) const
    {
        info.clear();
        xkb_layout_index_t count = xkb_keymap_num_layouts(m_keymap);
        for(xkb_layout_index_t i = 0; i < count; ++i){
            QString name = xkb_keymap_layout_get_name(m_keymap, i);
            info.append({names()[name], name});
            if (xkb_state_layout_index_is_active(m_state, i, XKB_STATE_LAYOUT_EFFECTIVE))
                info.setCurrentGroup(i);
        }
    }

    void lockGroup(uint group)
    {
        xcb_void_cookie_t cookie = xcb_xkb_latch_lock_state(m_connection, m_deviceId, 0, 0, 1, group, 0, 0, 0);
        xcb_generic_error_t *error = xcb_request_check(m_connection, cookie);
        if (error){
            qWarning() << "Lock group error: " << error->error_code;
        }
    }

private:
    void readState()
    {
        if (m_keymap)
            xkb_keymap_unref(m_keymap);
        m_keymap = xkb_x11_keymap_new_from_device(m_context, m_connection, m_deviceId, (xkb_keymap_compile_flags)0);

        if (m_state)
            xkb_state_unref(m_state);
        m_state = xkb_x11_state_new_from_device(m_keymap, m_connection, m_deviceId);
        emit m_pub->keyboardChanged();
    }

    const QHash<QString, QString> & names() const
    {
        static QHash<QString, QString> names;
        if (names.empty()){
            if(QFile::exists("/usr/share/X11/xkb/rules/evdev.xml")){
                QDomDocument doc;

                QFile file("/usr/share/X11/xkb/rules/evdev.xml");
                if (file.open(QIODevice::ReadOnly)){
                    if (doc.setContent(&file)) {
                        QDomElement docElem = doc.documentElement();

                        auto layout= docElem.firstChildElement("layoutList");
                        for(int i = 0; i < layout.childNodes().count(); ++i){
                            auto conf = layout.childNodes().at(i).firstChildElement("configItem");
                            names.insert(
                                conf.firstChildElement("description").firstChild().toText().data(),
                                conf.firstChildElement("name").firstChild().toText().data()
                            );
                        }
                    }
                    file.close();
                }
            }
        }
        return names;
    }

private:
    struct xkb_context *m_context    = 0;
    xcb_connection_t   *m_connection = 0;
    int32_t             m_deviceId;
    uint8_t             m_eventType;
    xkb_state          *m_state      = 0;
    xkb_keymap         *m_keymap     = 0;
    ::X11Kbd           *m_pub;
};

}

X11Kbd::X11Kbd():
    m_priv(new pimpl::X11Kbd(this))
{}

X11Kbd::~X11Kbd()
{}

bool X11Kbd::init()
{ return m_priv->init(); }

bool X11Kbd::isEnabled() const
{ return true; }

void X11Kbd::readKbdInfo(KbdInfo & info) const
{ m_priv->readKbdInfo(info); }

void X11Kbd::lockGroup(uint layId) const
{
    m_priv->lockGroup(layId);
}
