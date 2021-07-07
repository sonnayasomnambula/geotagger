#ifndef XMLNODEREADER_H
#define XMLNODEREADER_H

#include <QVariant>

class QString;
class QXmlStreamReader;

/**
 * @brief Класс-обёртка над QXmlStreamReader
 *
 * Класс представляет узел XML.
 *
 * Если узел имеет дочерние узлы, к ним можно перейти, вызвав readNextStartElement(), поэтому типичное
 * использование класса состоит в вызове readNextStartElement() в цикле while.
 *
 * Дочерний узел можно получить с помощью метода child(), тогда класс будет получать информацию
 * о попытках идентифицировать или прочитать дочерний узел.
 *
 * @code
    XmlNodeReader bookNode(&xml);
    while (bookNode.readNextStartElement()) // пока есть дочерние узлы
    {
        auto e = bookNode.child(); // получить дочерний узел

        if (e.isCalled("author") // В этот момент дочерний узел уведомляет узел родителя о том, что
                                 // тот должен содержать узел "author", и что узел найден (или не найден)
            && e.read()) // Теперь дочерний узел `e` пытается прочитать данные как QString и
                         // уведомляет узел bookNode о том, что узел "author" должен быть не только
                         // найден, но и прочитан, и о том, удалось ли его прочитать.
            book.author = e.value().toString(); // Если данные были успешно прочитаны, их можно записать

        // ...
 * @endcode
 *
 * Чтобы гарантированно охватить все строки цикла while, в конце цикла, когда все узлы уже прочитаны,
 * метод child() возвращает узел со специальным тегом XmlNodeReader::mcChecker. Чтобы задать
 * ограничение на допустимые имена узлов, в конце цикла нужно проверить это имя.
 *
 * @code
 *  QString chapterName;
    while (e.readNextStartElement())
    {
        auto chapter = e.child();

        if (chapter.findAndRead("chapter", chapterName))
            chapters.append(chapterName);
        else if (chapter.isCalled(XmlNodeReader::mcChecker))
            continue;
        else
            EXPECT_TRUE(false) << "only <chapter> nodes allowed";
    }
 * @endcode
 *
 * После завершения чтения можно выяснить его успешность: все ли узлы были найдены, все ли прочитаны.
 *
 * @code
 * // isValid() проверяет, что все узлы и все атрибуты успешно найдены и, если нужно, прочитаны
 * book.isValid = bookNode.isValid();
 * @endcode
 *
 * Пример использования см. tst_xmlelementreader.cpp
 */
class  XmlNodeReader
{
public:
    /** Статус узла */
    enum class Status
    {
        NotFound,   ///< не найден
        Found,      ///< найден
        ReadFailed, ///< чтение провалилось
        Read        ///< успешно прочитан
    };

    /** Обязательность узла */
    enum class Requirement
    {
        /// Обязателен
        Required,
        /// Необязателен - при вызове isValid() статус NotFound (но не ReadFailed) будет проигнорирован
        NotRequired
    };

    explicit XmlNodeReader(QXmlStreamReader* xml);

    /** Деструктор вычитывает узел до закрывающего тега */
    ~XmlNodeReader();

    XmlNodeReader(const XmlNodeReader&) = delete;

    XmlNodeReader(XmlNodeReader&&) = default;

    /**
     * @brief Читаем XML до тех пор, пока не найдём дочерний открывающий тег или не упрёмся в
     * закрывающий тег текущего узла или в конец документа
     * @return true, если найден открывающий тег
     * @see mIsOver
     */
    bool readNextStartElement();

    /**
     * @brief isCalled - "Называется"
     * Кроме проверки name == this->mName, уведомляет родительский узел о статусе дочернего узла с тегом name
     * @param name тег узла
     * @param requirement обязательность узла
     * @return true, если mName совпадает с name
     * @see isValid()
     */
    bool isCalled(const QString& name, Requirement requirement = Requirement::Required) const;

    /** @return имя тега */
    QString name() const { return mName; }

    /**
     * @brief Прочитать значение переменной
     * Использует QXmlStreamReader::readElementText()
     * @param type тип узла
     * @return true, если прочитанные данные могут быть приведены к типу type
     */
    bool read(QVariant::Type type = QVariant::String);

    /**
     * @brief Прочитать значение переменной
     * Метод объединяет isCalled() и read()
     * @param name имя тега
     * @param variable переменная, которой будет присвоено прочитанное значение
     * @param requirement обязательность узла
     * @return true, если имя узла name и данные читаются как тип T
     * @note довольно тонкий код, основанный на том, что QVariant::Type и QMetaType::Type в общем совпадают
     * (что несколько прощупано в тесте (RlsSensorTest, metaTypes))
     */
    template <typename T>
    bool findAndRead(const QString& name, T& variable, Requirement requirement = Requirement::Required)
    {
        const auto typeId = static_cast<QVariant::Type>(qMetaTypeId<T>());
        if (isCalled(name, requirement) && read(typeId))
        {
            variable = value().value<T>();
            return true;
        }

        return false;
    }

    /**
     * @brief Чтение атрибута узла
     * @param name имя атрибута
     * @param type тип атрибута
     * @return false, если атрибут attributeName отсутствует или не может быть
     *         приведён к типу type
     */
    bool readAttribute(const QString& name, QVariant::Type type = QVariant::String);

    /**
     * @return XmlNodeReader для текущего узла mXml
     * При этом делается отметка о том, что в текущем узле найден новый дочерний узел
     * @code
     * XmlNodeReader node(&xml);
     * while (bookNode.readNextStartElement())
     * {
     *     auto child = node.child();
     *     // ...
     * }
     * @endcode
     */
    XmlNodeReader child();

    /**
     * @return прочитанное значение;
     * При приведении QVariant к int, double и т.п. проверка не требуется,
     * т.к. она уже была произведена при вызове read() / readAttribute()
     */
    QVariant value() const;

    /**
     * @brief Вывод информации о текущем элементе в qWarning()
     */
    void log();

    /**
     * @return true, если все вызовы readAttribute() были успешны, а для всех дочерних узлов,
     * полученных с помощью child(), был хотя бы один успешный вызов isCalled() или read()
     * Проверяются только дочерние узлы первого уровня
     */
    bool isValid() const;

    /**
     * @return true, если не было прочитано ни одного атрибута, и не было найдено ни одного
     * дочернего элемента
     */
    bool isEmpty() const;

    static const QString mcChecker; ///< специальный тег для проверки того, что все дочерние узлы прочитаны

private:
    /** установка статуса дочернего узла childName */
    void setChildStatus(const QString& childName, Status status);

    /** установка статуса атрибута attributeName */
    void setAttributeStatus(const QString& attributeName, Status status);

    using Statuses = QMap<QString, Status>;

    /**
     * @brief Установка статуса дочернего узла или атрибута
     * @param map mChilds или mAttributes
     * @param name имя статуса или атрибута
     * @param status новый статус
     * Статус обновляется только если новый статус больше старого, чтобы
     * статус 'Не найден' не затёр статуса 'Найден' или 'Прочитан'
     */
    void setStatus(Statuses& map, const QString& name, Status status);

    /**
     * @brief Задать родительский узел; узел будет получать уведомления о результатах
     * вызова isCalled() и read()
     * @param parent родительский узел
     */
    void setParent(XmlNodeReader* parent) { mParent = parent; }

    /**
     * @brief Регистрация нового статуса в mParent
     * @param name имя дочернего узла
     * @param status новый статус
     */
    void registerStatus(const QString& name, Status status) const;

    /**
     * @copybrief registerStatus(const QString&,Status)
     * @param status новый статус
     * имя дочернего узла - mName
     */
    void registerStatus(Status status) const;

    QXmlStreamReader* mXml;           ///< QXmlStreamReader
    QString mName;                    ///< имя узла
    QVariant mValue;                  ///< прочитанное значение
    XmlNodeReader* mParent = nullptr; ///< родительский элемент
    Statuses mChilds;                 ///< статусы дочерних элементов
    Statuses mAttributes;             ///< статусы атрибутов
    bool mIsOver = false;             ///< достигнут закрывающий тег текущего узла

    /// данные о положении текущего узла в xml-документе
    qint64 mLineNumber = 0;
    qint64 mColumnNumber = 0;
    qint64 mCharacterOffset = 0;
};

#endif // XMLNODEREADER_H
