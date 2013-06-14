#ifndef DICTMODEL_H
#define DICTMODEL_H
#include <QAbstractItemModel>
#include <QSet>
#include <QFile>

class DictModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DictModel(QObject* parent = 0);
    virtual ~DictModel();
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    void load();
    void load(QFile& file);
    void defaults();
    bool save();
    void add(const QMap<QString, QString>& dict);
    bool moveDown(const QModelIndex& currentIndex);
    bool moveUp(const QModelIndex& currentIndex);
private:
    QSet<QString> m_requiredKeys;
    QList< QMap< QString, QString> > m_dicts;
};

#endif // DICTMODEL_H
