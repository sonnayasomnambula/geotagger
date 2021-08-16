#ifndef ABSTRACTSETTINGS_H
#define ABSTRACTSETTINGS_H

#include <QSettings>
#include <QVariant>

class AbstractSettings
{
public:
    template <typename T>
    class Tag
    {
    public:
        Tag(const char* key) : mKey(key) {}
        Tag(const QString& key) : mKey(key) {}

        T value(const T& defaultValue = T()) const {
            return QSettings().value(mKey, defaultValue).template value<T>();
        }

        T operator ()(const T& defaultValue = T()) const {
            return QSettings().value(mKey, defaultValue).template value<T>();
        }

        void save(const T& value) {
            QSettings().setValue(mKey, value);
        }

        bool exists() const {
            return QSettings().contains(mKey);
        }

    protected:
        const QString mKey;
    };

    class VariantTag : public Tag<QVariant>
    {
    public:
        using Tag<QVariant>::Tag;

        template<typename T>
        T operator ()(const T& defaultValue) const {
            return QSettings().value(mKey, defaultValue).template value<T>();
        }
    };

    class State
    {
        Tag<QByteArray> mTag;

    public:
        State(const char* key) : mTag(key) {}
        State(const QString& key) : mTag(key) {}

        template <class Widget>
        void save(Widget* widget) {
            mTag.save(widget->saveState());
        }

        template <class Widget>
        void restore(Widget* widget) {
            widget->restoreState(mTag(widget->saveState()));
        }
    };

    class Geometry
    {
        Tag<QByteArray> mTag;

    public:
        Geometry(const char* key) : mTag(key) {}
        Geometry(const QString& key) : mTag(key) {}

        template <class Widget>
        void save(Widget* widget) {
            mTag.save(widget->saveGeometry());
        }

        template <class Widget>
        void restore(Widget* widget) {
            widget->restoreGeometry(mTag(widget->saveGeometry()));
        }
    };

    class Property
    {
        Tag<QVariant> mTag;
        QByteArray mPropertyName;

    public:
        Property(const char* key, const char* propertyName) : mTag(key), mPropertyName(propertyName) {}
        Property(const QString& key, const char* propertyName) : mTag(key), mPropertyName(propertyName) {}

        template <class Object>
        void save(Object* object) {
            mTag.save(object->property(mPropertyName));
        }

        template <class Object>
        void restore(Object* object) {
            object->setProperty(mPropertyName, mTag(object->property(mPropertyName)));
        }
    };
};

#endif // ABSTRACTSETTINGS_H
