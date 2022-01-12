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
        const QString mKey;

    public:
        Tag(const char* key) : mKey(key) {}

        T operator ()(const T& defaultValue) const {
            return QSettings().value(mKey, defaultValue).template value<T>();
        }

        operator T() const {
            return operator ()(T());
        }

        Tag& operator =(const T& value) {
            QSettings().setValue(mKey, value);
            return *this;
        }

        Tag& operator +=(const T& value) {
            QSettings settings;
            T existing = settings.value(mKey).template value<T>();
            existing += value;
            settings.setValue(mKey, existing);
            return *this;
        }

        bool isNull() const {
            return !QSettings().contains(mKey);
        }
    };

    class VariantTag : public Tag<QVariant>
    {
    public:
        using Tag<QVariant>::Tag;

        template<typename T>
        T operator ()(const T& defaultValue) const {
            return Tag::operator()(defaultValue).template value<T>();
        }
    };

    class State
    {
        Tag<QByteArray> mState;

    public:
        State(const char* key) : mState(key) {}

        template <class Widget>
        void save(Widget* widget) { mState = widget->saveState(); }

        template <class Widget>
        void restore(Widget* widget) { if (!mState.isNull()) widget->restoreState(mState); }
    };

    class Geometry
    {
        Tag<QByteArray> mGeometry;

    public:
        Geometry(const char* key) : mGeometry(key) {}

        template <class Widget>
        void save(Widget* widget) { mGeometry = widget->saveGeometry(); }

        template <class Widget>
        void restore(Widget* widget) { if (!mGeometry.isNull()) widget->restoreGeometry(mGeometry); }
    };

    class Property
    {
        Tag<QVariant> mProperty;
        QByteArray mPropertyName;

    public:
        Property(const char* key, const char* propertyName) : mProperty(key), mPropertyName(propertyName) {}

        template <class Object>
        void save(Object* object) { mProperty = object->property(mPropertyName); }

        template <class Object>
        void restore(Object* object) { if (!mProperty.isNull()) object->setProperty(mPropertyName, mProperty); }
    };
};

#endif // ABSTRACTSETTINGS_H
