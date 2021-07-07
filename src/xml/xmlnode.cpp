#include <QXmlStreamWriter>
#include <QString>
#include <QDebug>

#include "xmlnode.h"


XmlNode::XmlNode(const QString& name, QXmlStreamWriter* writer) :
    mWriter(writer)
{
    Q_ASSERT(mWriter);
    mWriter->writeStartElement(name);
}

XmlNode::~XmlNode()
{
    mWriter->writeEndElement();
}

void XmlNode::createAttribute(const QString& qualifiedName, const QString& value)
{
    mWriter->writeAttribute(qualifiedName, value);
}

void XmlNode::setText(const QString& text)
{
    mWriter->writeCharacters(text);
}
