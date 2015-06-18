#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QEvent>
#include "kbdstate.h"
#include "content.h"

Content::Content(KbdWatcher *watcher) :
    QWidget(),
    m_watcher(watcher)
{
    QHBoxLayout *box = new QHBoxLayout;
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(0);
    setLayout(box);

    m_capsLock = new QLabel("C");
    m_capsLock->setObjectName("CapsLockLabel");
    m_capsLock->setAlignment(Qt::AlignCenter);
    m_capsLock->installEventFilter(this);
    layout()->addWidget(m_capsLock);

    m_numLock = new QLabel("N");
    m_numLock->setObjectName("NumLockLabel");
    m_numLock->setAlignment(Qt::AlignCenter);
    m_numLock->installEventFilter(this);
    layout()->addWidget(m_numLock);

    m_scrollLock = new QLabel("S");
    m_scrollLock->setObjectName("ScrollLockLabel");
    m_scrollLock->setAlignment(Qt::AlignCenter);
    m_scrollLock->installEventFilter(this);
    layout()->addWidget(m_scrollLock);

    m_layout = new QLabel;
    m_layout->setObjectName("LayoutLabel");
    m_layout->setAlignment(Qt::AlignCenter);
    m_layout->installEventFilter(this);
    layout()->addWidget(m_layout);


    connect(m_watcher, &KbdWatcher::modifierStateChanged, [this](Controls mod, bool active){
        setEnabled(mod, active);
    });

    connect(m_watcher, &KbdWatcher::layoutChanged, [this](const QString & sym, const QString & name){
        m_layout->setText(sym.toUpper());
        m_layout->setToolTip(name);
    });
}

Content::~Content()
{}

bool Content::setup()
{
    m_capsLock->setVisible(Settings::instance().showCapLock());
    m_numLock->setVisible(Settings::instance().showNumLock());
    m_scrollLock->setVisible(Settings::instance().showScrollLock());
    m_layout->setVisible(m_watcher->isLayoutEnabled() && Settings::instance().showLayout());
    return true;
}

void Content::setEnabled(Controls cnt, bool enabled)
{
    widget(cnt)->setEnabled(enabled);
}

QWidget* Content::widget(Controls cnt) const
{
    switch(cnt){
    case Caps:   return m_capsLock;
    case Num:    return m_numLock;
    case Scroll: return m_scrollLock;
    case Layout: return m_layout;
    }
    return 0;
}

bool Content::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::QEvent::MouseButtonRelease)
    {
        if (object == m_capsLock)
            emit controlClicked(Controls::Caps);
        else if (object == m_numLock)
            emit controlClicked(Controls::Num);
        else if (object == m_scrollLock)
            emit controlClicked(Controls::Scroll);
        else if(object == m_layout){
            emit controlClicked(Controls::Layout);
        }
        return true;
    }

    return QObject::eventFilter(object, event);
}
