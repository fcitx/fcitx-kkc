#ifndef RULEMODEL_H
#define RULEMODEL_H
#include <QAbstractListModel>

#include <libkkc/libkkc.h>

class Rule {
public:
    Rule(const QString& name, const QString& label) :
        m_name(name),
        m_label(label)
    {
    }

    const QString& name() const {
        return m_name;
    }

    const QString& label() const {
        return m_label;
    }

private:
    QString m_name;
    QString m_label;
};

class RuleModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit RuleModel(QObject* parent = 0);

    virtual ~RuleModel();

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    void load();
    int findRule(const QString& name);

private:
    QList<Rule> m_rules;
};

#endif // RULEMODEL_H
