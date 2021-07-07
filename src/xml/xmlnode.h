#ifndef XMLNODE_H
#define XMLNODE_H

#include <QString>

class QXmlStreamWriter;

/**
 * @brief Класс-обёртка над QXmlStreamWriter
 *
 * Ставит открывающий тег в конструкторе, закрывающий в деструкторе
 *
    @code
    QString xml;
    QXmlStreamWriter writer(&xml);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    XmlNode root("root", &writer);
    @endcode
 */
class XmlNode
{
public:
    /**
     * @brief Конструктор; ставит открывающий тег
     * @param name      имя узла
     * @param writer    экземпляр QXmlStreamWriter
     */
    XmlNode(const QString& name, QXmlStreamWriter* writer);

    /**
     * @brief Деструктор; ставит закрывающий тег
     */
    ~XmlNode();

    /**
     * @brief Создание атрибута
     * @param qualifiedName имя атрибута
     * @param value         значение атрибута
     */
    void createAttribute(const QString& qualifiedName, const QString& value);

    /**
     * @brief Установка содержимого узла
     * @param text содержимое узла
     */
    void setText(const QString& text);

    /**
     * @brief Установка числа в качестве содержимого узла
     * @tparam T тип числа (должен быть совместим с QString::number() без аргументов)
     * @param number число
     */
    template <typename T>
    void setNumber(T number) { setText(QString::number(number)); }

private:
    QXmlStreamWriter* mWriter; ///< указатель на экземпляр QXmlStreamWriter
};

#endif // XMLNODE_H
