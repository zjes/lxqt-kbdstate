#include <QTimer>
#include <QEvent>

#include "kbdstate.h"
#include "kbdkeeper.h"
#include "kbdstateconfig.h"

KbdState::KbdState(const ILxQtPanelPluginStartupInfo &startupInfo):
    QObject(),
    ILxQtPanelPlugin(startupInfo),
    m_content(&m_watcher)
{
    Settings::instance().init(settings());
    connect(&m_content, &Content::controlClicked, &m_watcher, &KbdWatcher::controlClicked);
    settingsChanged();
}

KbdState::~KbdState()
{}

void KbdState::settingsChanged()
{
    m_content.setup();
    m_watcher.setup();
}

QDialog *KbdState::configureDialog()
{
    return new KbdStateConfig(&m_content);
}

void KbdState::realign()
{
    if (panel()->isHorizontal())
        m_content.setMinimumSize(0, panel()->iconSize());
    else
        m_content.setMinimumSize(panel()->iconSize(), 0);
}
