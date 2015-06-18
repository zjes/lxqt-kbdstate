#ifndef _CONTENT_H_
#define _CONTENT_H_

#include <QWidget>
#include "controls.h"

class QLabel;
class KbdWatcher;

class Content : public QWidget
{
    Q_OBJECT
public:
    Content(KbdWatcher *watcher);
    ~Content();

public:
    void setEnabled(Controls cnt, bool enabled);
    QWidget* widget(Controls cnt) const;
    bool setup();

    virtual bool eventFilter(QObject *object, QEvent *event);
signals:
    void controlClicked(Controls cnt);
private:
    KbdWatcher *m_watcher;
    QLabel     *m_capsLock;
    QLabel     *m_numLock;
    QLabel     *m_scrollLock;
    QLabel     *m_layout;
};

#endif
