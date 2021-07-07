#ifndef PHOTOSLISTMODEL_H
#define PHOTOSLISTMODEL_H

#include <QAbstractListModel>
#include <QString>

class PhotosListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PhotosListModel(QObject* parent = nullptr);

    bool setFiles(const QStringList& files);
    QString lastError() const { return mLastError; }

    class Header
    {
    public:
        enum { Name, Time, Position, Count };
        static QVariant name(int section);

    };

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    using Item = std::array<QVariant, Header::Count>;
    QList<Item> mData;

    QString mLastError;
};

#endif // PHOTOSLISTMODEL_H
