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
    connect(&m_layout, &KbdLayout::keyboardChanged, [this](){
        m_layout.readKbdInfo(m_info);
        emit changed();
    });

    connect(&m_layout, &KbdLayout::layoutChanged, [this](int layId){
        m_info.setCurrentGroup(layId);
        emit changed();
    });

    connect(&m_layout, &KbdLayout::checkState, [this](){
    });

    return true;
}

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

bool WinKbdKeeper::setup()
{
    connect(&m_layout, &KbdLayout::keyboardChanged, [this](){
        m_layout.readKbdInfo(m_info);
        emit changed();
    });

    connect(&m_layout, &KbdLayout::layoutChanged, [this](int layId){
        WId win = KWindowSystem::activeWindow();

        if (m_active == win){
            m_mapping[win] = layId;
            m_info.setCurrentGroup(layId);
        } else {
            if (!m_mapping.contains(win))
                m_mapping.insert(win, 0);
            m_layout.lockGroup(m_mapping[win]);
            m_active = win;
            m_info.setCurrentGroup(m_mapping[win]);
        }
        emit changed();
    });

    connect(&m_layout, &KbdLayout::checkState, [this](){
        WId win = KWindowSystem::activeWindow();

        if( m_active != win) {
            if (!m_mapping.contains(win))
                m_mapping.insert(win, 0);
            m_layout.lockGroup(m_mapping[win]);
            m_active = win;
            m_info.setCurrentGroup(m_mapping[win]);
            emit changed();
        }
    });

    return true;
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

bool AppKbdKeeper::setup()
{
    connect(&m_layout, &KbdLayout::keyboardChanged, [this](){
        m_layout.readKbdInfo(m_info);
        emit changed();
    });

    connect(&m_layout, &KbdLayout::layoutChanged, [this](int layId){
        KWindowInfo info = KWindowInfo(KWindowSystem::activeWindow(), 0, NET::WM2WindowClass);
        QString app = info.windowClassName();

        if (m_active == app){
            m_mapping[app] = layId;
            m_info.setCurrentGroup(layId);
        } else {
            if (!m_mapping.contains(app))
                m_mapping.insert(app, 0);

            m_layout.lockGroup(m_mapping[app]);
            m_active = app;
            m_info.setCurrentGroup(m_mapping[app]);
        }
        emit changed();
    });

    connect(&m_layout, &KbdLayout::checkState, [this](){
        KWindowInfo info = KWindowInfo(KWindowSystem::activeWindow(), 0, NET::WM2WindowClass);
        QString app = info.windowClassName();

        if( m_active != app) {
            if (!m_mapping.contains(app))
                m_mapping.insert(app, 0);

            m_layout.lockGroup(m_mapping[app]);
            m_active = app;
            m_info.setCurrentGroup(m_mapping[app]);
            emit changed();
        }
    });

    return true;
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
