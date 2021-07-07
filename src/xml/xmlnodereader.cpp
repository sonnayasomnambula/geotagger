#include <QDebug>
#include <QString>
#include <QXmlStreamReader>

#include "xmlnodereader.h"


#undef qDebug
#define qDebug QT_NO_QDEBUG_MACRO

const QString XmlNodeReader::mcChecker;

XmlNodeReader::XmlNodeReader(QXmlStreamReader* xml) :
    mXml(xml)
{
    Q_ASSERT(mXml);
    mName = mXml->name().toString();

    mLineNumber = mXml->lineNumber();
    mColumnNumber = mXml->columnNumber();
    mCharacterOffset = mXml->characterOffset();
}

XmlNodeReader::~XmlNodeReader()
{
    if (mName == mcChecker)
        return;

    while (readNextStartElement())
        ;
}

bool XmlNodeReader::read(QVariant::Type type)
{
    if (mName == mcChecker)
        return false;

    mValue = mXml->readElementText(QXmlStreamReader::SkipChildElements);
    if (!mValue.convert(static_cast<int>(type)))
    {
        qWarning() << "invalid" << mName << "xml value:" << mValue.toString();
        log();
        registerStatus(Status::ReadFailed);
        return false;
    }

    registerStatus(Status::Read);
    mIsOver = true;
    return true;
}

bool XmlNodeReader::readAttribute(const QString& name, QVariant::Type type)
{
    if (mName == mcChecker)
        return false;

    if (!mXml->attributes().hasAttribute(name))
    {
        mAttributes[name] = Status::NotFound;
        return false;
    }

    mValue = mXml->attributes().value(name).toString();

    if (!mValue.convert(static_cast<int>(type)))
    {
        qWarning() << "invalid or missing" << name << "xml attribute:" << mValue.toString();
        log();
        mAttributes[name] = Status::ReadFailed;
        return false;
    }

    mAttributes[name] = Status::Read;
    return true;
}

bool XmlNodeReader::readNextStartElement()
{
    for (;;)
    {
        if (mXml->atEnd())
            return false;

        if (mIsOver)
            return false;

        // возможно, узел уже вычитан в другом месте
        if (mXml->isEndElement() && mXml->name() == mName)
            return false;

        const auto tokenType = mXml->readNext();

        if (tokenType == QXmlStreamReader::EndDocument)
            return false;

        if (tokenType == QXmlStreamReader::EndElement && mXml->name() == mName)
        {
            mIsOver = true;
            // проходим ещё раз, чтобы убедиться, что все теги прочитаны
            return true;
        }

        if (tokenType == QXmlStreamReader::StartElement)
            return true;
    }
}

void XmlNodeReader::setChildStatus(const QString& childName, Status status)
{
    if (childName != mcChecker)
        setStatus(mChilds, childName, status);
}

void XmlNodeReader::setAttributeStatus(const QString& attributeName, XmlNodeReader::Status status)
{
    setStatus(mAttributes, attributeName, status);
}

void XmlNodeReader::setStatus(Statuses& map, const QString& name, XmlNodeReader::Status status)
{
    auto i = map.find(name);
    if (i == map.end())
        map.insert(name, status);
    else
        *i = std::max(*i, status);
}

void XmlNodeReader::registerStatus(const QString& name, Status status) const
{
    if (mParent)
        mParent->setChildStatus(name, status);
}

void XmlNodeReader::registerStatus(XmlNodeReader::Status status) const
{
    registerStatus(mName, status);
}

XmlNodeReader XmlNodeReader::child()
{
    XmlNodeReader r(mXml);
    if (mIsOver)
    {
        r.mName = mcChecker;
    }
    else
    {
        Q_ASSERT(mXml->isStartElement());
        setChildStatus(r.mName, Status::Found);
    }
    r.setParent(this);
    return r;
}

QVariant XmlNodeReader::value() const
{
    return mValue;
}

void XmlNodeReader::log()
{
    static QHash<QXmlStreamReader::TokenType, QString> type = {
        {QXmlStreamReader::StartDocument, "[<doc>]"},
        {QXmlStreamReader::EndDocument, "[</doc>]"},
        {QXmlStreamReader::StartElement, "[<>]"},
        {QXmlStreamReader::EndElement, "[</>]"},
        {QXmlStreamReader::Characters, "[char]"},
        {QXmlStreamReader::Comment, "[comm]"},
        {QXmlStreamReader::DTD, "[DTD]"},
        {QXmlStreamReader::EntityReference, "[ERef]"},
    };

    qWarning() << "xml element" << mXml->name() << type[mXml->tokenType()]
               << "at" << mXml->lineNumber() << ":" << mXml->columnNumber();
}

bool XmlNodeReader::isCalled(const QString& name, Requirement requirement) const
{
    if (mName == name)
    {
        registerStatus(Status::Found);
        return true;
    }
    else
    {
        if (requirement == Requirement::Required)
            registerStatus(name, Status::NotFound);
        return false;
    }
}

bool XmlNodeReader::isValid() const
{
    bool valid = true;
    for (auto i = mChilds.begin(); i != mChilds.end(); ++i)
    {
        if (*i == Status::NotFound)
        {
            qWarning().noquote() << QString("A node <%1> is not found in the <%2> node at %3:%4(%5)")
                              .arg(i.key())
                              .arg(mName)
                              .arg(mLineNumber)
                              .arg(mColumnNumber)
                              .arg(mCharacterOffset);
            valid = false;
        }
        if (*i == Status::ReadFailed)
        {
            qWarning().noquote() << QString("A node <%1> is read failed in the <%2> node at %3:%4(%5)")
                              .arg(i.key())
                              .arg(mName)
                              .arg(mLineNumber)
                              .arg(mColumnNumber)
                              .arg(mCharacterOffset);
            valid = false;
        }
    }

    for (auto i = mAttributes.begin(); i != mAttributes.end(); ++i)
    {
        if (*i == Status::NotFound)
        {
            qWarning().noquote() << QString("Attribute '%1' is not found in the node <%2> at %3:%4(%5)")
                              .arg(i.key())
                              .arg(mName)
                              .arg(mLineNumber)
                              .arg(mColumnNumber)
                              .arg(mCharacterOffset);
            valid = false;
        }
        if (*i == Status::ReadFailed)
        {
            qWarning().noquote() << QString("Attribute '%1' read failed in the node <%2> at %3:%4(%5)")
                              .arg(i.key())
                              .arg(mName)
                              .arg(mLineNumber)
                              .arg(mColumnNumber)
                              .arg(mCharacterOffset);
            valid = false;
        }
    }

    return valid;
}

bool XmlNodeReader::isEmpty() const
{
    for (auto i = mAttributes.cbegin(); i != mAttributes.cend(); ++i)
        if (i.value() != Status::NotFound)
            return false;

    for (auto i = mChilds.cbegin(); i != mChilds.cend(); ++i)
        if (i.value() != Status::NotFound)
            return false;

    return true;
}
