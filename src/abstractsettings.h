#ifndef ABSTRACTSETTINGS_H
#define ABSTRACTSETTINGS_H

#include <QSettings>
#include <QVariant>

class AbstractSettings
{
public:
    class Tag
    {
    public:
        Tag(const char* key) : mKey(key) {}

        QVariant value() const {
            return QSettings().value(mKey);
        }

        template<typename T>
        T operator ()(const T& defaultValue) const {
            return QSettings().value(mKey, defaultValue).template value<T>();
        }

        void save(const QVariant& value) {
            QSettings().setValue(mKey, value);
        }

        bool exists() const {
            return QSettings().contains(mKey);
        }

        template<class Widget>
        void restoreState(Widget* widget) {
            widget->restoreState(operator ()(widget->saveState()));
        }

    private:
        const QString mKey;
    };

    class State
    {
        Tag mTag;

    public:
        State(const char* key) : mTag(key) {}

        template <class Widget>
        void save(Widget* widget) {
            mTag.save(widget->saveState());
        }

        template <class Widget>
        void restore(Widget* widget) {
            widget->restoreState(mTag.operator ()(widget->saveState()));
        }
    };

    class Geometry
    {
        Tag mTag;

    public:
        Geometry(const char* key) : mTag(key) {}

        template <class Widget>
        void save(Widget* widget) {
            mTag.save(widget->saveGeometry());
        }

        template <class Widget>
        void restore(Widget* widget) {
            widget->restoreGeometry(mTag.operator ()(widget->saveGeometry()));
        }
    };
};

#endif // ABSTRACTSETTINGS_H
