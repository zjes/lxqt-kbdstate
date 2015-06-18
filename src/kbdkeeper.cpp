#include <QDebug>
#include <KWindowSystem/KWindowSystem>
#include <KWindowSystem/KWindowInfo>
#include <KWindowSystem/netwm_def.h>
#include "kbdkeeper.h"

//--------------------------------------------------------------------------------------------------

KbdKeeper::KbdKeeper(const KbdLayout & layout, KeeperType type):
    m_layout(layout),
    m_type(type)
{
    m_layout.readKbdInfo(m_info);
}

KbdKeeper::~KbdKeeper()
{}

bool KbdKeeper::setup()
{
    connect(&m_layout, SIGNAL(keyboardChanged()), SLOT(keyboardChanged()));
    connect(&m_layout, SIGNAL(layoutChanged(uint)), SLOT(layoutChanged(uint)));
    connect(&m_layout, SIGNAL(checkState()), SLOT(checkState()));

    return true;
}

void KbdKeeper::keyboardChanged()
{
    m_layout.readKbdInfo(m_info);
    emit changed();
}

void KbdKeeper::layoutChanged(uint group)
{
    m_info.setCurrentGroup(group);
    emit changed();
}

void KbdKeeper::checkState()
{}

void KbdKeeper::switchToNext()
{
    uint index = m_info.currentGroup();
    if (index < m_info.size() - 1)
        ++index;
    else
        index = 0;

    switchToGroup(index);
}

void KbdKeeper::switchToGroup(uint group)
{
    m_layout.lockGroup(group);
    emit changed();
}

//--------------------------------------------------------------------------------------------------

WinKbdKeeper::WinKbdKeeper(const KbdLayout & layout):
    KbdKeeper(layout, KeeperType::Window)
{}

WinKbdKeeper::~WinKbdKeeper()
{}

void WinKbdKeeper::layoutChanged(uint group)
{
    WId win = KWindowSystem::activeWindow();

    if (m_active == win){
        m_mapping[win] = group;
        m_info.setCurrentGroup(group);
    } else {
        if (!m_mapping.contains(win))
            m_mapping.insert(win, 0);
        m_layout.lockGroup(m_mapping[win]);
        m_active = win;
        m_info.setCurrentGroup(m_mapping[win]);
    }
    emit changed();
}

void WinKbdKeeper::checkState()
{
    WId win = KWindowSystem::activeWindow();

    if (!m_mapping.contains(win))
        m_mapping.insert(win, 0);
    m_layout.lockGroup(m_mapping[win]);
    m_active = win;
    m_info.setCurrentGroup(m_mapping[win]);
    emit changed();
}

void WinKbdKeeper::switchToGroup(uint group)
{
    WId win = KWindowSystem::activeWindow();
    m_mapping[win] = group;
    m_layout.lockGroup(group);
    m_info.setCurrentGroup(group);
    emit changed();
}


//--------------------------------------------------------------------------------------------------

AppKbdKeeper::AppKbdKeeper(const KbdLayout & layout):
    KbdKeeper(layout, KeeperType::Window)
{}

AppKbdKeeper::~AppKbdKeeper()
{}

void AppKbdKeeper::layoutChanged(uint group)
{
    KWindowInfo info = KWindowInfo(KWindowSystem::activeWindow(), 0, NET::WM2WindowClass);
    QString app = info.windowClassName();

    if (m_active == app){
        m_mapping[app] = group;
        m_info.setCurrentGroup(group);
    } else {
        if (!m_mapping.contains(app))
            m_mapping.insert(app, 0);

        m_layout.lockGroup(m_mapping[app]);
        m_active = app;
        m_info.setCurrentGroup(m_mapping[app]);
    }
    emit changed();
}

void AppKbdKeeper::checkState()
{
    KWindowInfo info = KWindowInfo(KWindowSystem::activeWindow(), 0, NET::WM2WindowClass);
    QString app = info.windowClassName();

    if (!m_mapping.contains(app))
        m_mapping.insert(app, 0);

    m_layout.lockGroup(m_mapping[app]);
    m_active = app;
    m_info.setCurrentGroup(m_mapping[app]);
    emit changed();
}


void AppKbdKeeper::switchToGroup(uint group)
{
    KWindowInfo info = KWindowInfo(KWindowSystem::activeWindow(), 0, NET::WM2WindowClass);
    QString app = info.windowClassName();

    m_mapping[app] = group;
    m_layout.lockGroup(group);
    m_info.setCurrentGroup(group);
    emit changed();
}
