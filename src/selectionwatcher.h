#ifndef SELECTIONWATCHER_H
#define SELECTIONWATCHER_H

#include <QObject>
#include <QQmlEngine>

QT_BEGIN_NAMESPACE
class QAbstractItemView;
QT_END_NAMESPACE

/* The class to pass current item between C++ and QML */

class SelectionWatcher : public QObject
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    QML_ELEMENT
#endif

    Q_PROPERTY(int current MEMBER mCurrent WRITE setCurrent NOTIFY currentChanged)

signals:
    void currentChanged(int);

public:
    SelectionWatcher() {} // QML-used objects must be destoyed after QML engine so don't pass parent here
    void setCurrent(int current);

private:
    int mCurrent = -1; // model index
};

#endif // SELECTIONWATCHER_H
