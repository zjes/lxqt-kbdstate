#include <QDebug>
#include <KF5/KGuiAddons/KModifierKeyInfo>
#include "kbdwatcher.h"

KbdWatcher::KbdWatcher()
{
    m_modifierInfo = new KModifierKeyInfo(this);
    connect(m_modifierInfo, &KModifierKeyInfo::keyLocked, this, &KbdWatcher::modifierLocked);
    m_layout.init();
}

void KbdWatcher::setup()
{
    emit modifierStateChanged(Controls::Caps,   m_modifierInfo->isKeyLocked(Qt::Key_CapsLock));
    emit modifierStateChanged(Controls::Num,    m_modifierInfo->isKeyLocked(Qt::Key_NumLock));
    emit modifierStateChanged(Controls::Scroll, m_modifierInfo->isKeyLocked(Qt::Key_ScrollLock));

    if (!m_keeper || m_keeper->type() != Settings::instance().keeperType()){
        createKeeper(Settings::instance().keeperType());
    }
}

void KbdWatcher::createKeeper(KeeperType type)
{
    switch(type)
    {
    case KeeperType::Global:
        m_keeper.reset(new KbdKeeper(m_layout));
        break;
    case KeeperType::Window:
        m_keeper.reset(new WinKbdKeeper(m_layout));
        break;
    case KeeperType::Application:
        m_keeper.reset(new AppKbdKeeper(m_layout));
        break;
    }

    connect(m_keeper.data(), SIGNAL(changed()), this, SLOT(keeperChanged()));

    m_keeper->setup();
}

void KbdWatcher::keeperChanged()
{
    emit layoutChanged(m_keeper->sym(), m_keeper->name());
}

void KbdWatcher::modifierLocked(Qt::Key key, bool active)
{
    switch (key) {
    case Qt::Key_CapsLock:
        emit modifierStateChanged(Controls::Caps, active);
        break;
    case Qt::Key_NumLock:
        emit modifierStateChanged(Controls::Num, active);
        break;
    case Qt::Key_ScrollLock:
        emit modifierStateChanged(Controls::Scroll, active);
        break;
    default:
        break;
    }
}

void KbdWatcher::controlClicked(Controls cnt)
{
    switch(cnt){
    case Controls::Caps:
        m_modifierInfo->setKeyLocked(Qt::Key_CapsLock, !m_modifierInfo->isKeyLocked(Qt::Key_CapsLock));
        break;
    case Controls::Num:
        m_modifierInfo->setKeyLocked(Qt::Key_NumLock, !m_modifierInfo->isKeyLocked(Qt::Key_NumLock));
        break;
    case Controls::Scroll:
        m_modifierInfo->setKeyLocked(Qt::Key_ScrollLock, !m_modifierInfo->isKeyLocked(Qt::Key_ScrollLock));
        break;
    case Controls::Layout:
        m_keeper->switchToNext();
        break;
    }
}
