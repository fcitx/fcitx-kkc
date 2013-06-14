#include "rulemodel.h"
#include <libkkc/libkkc.h>

RuleModel::RuleModel(QObject* parent): QAbstractListModel(parent)
{
}

RuleModel::~RuleModel()
{
}

void RuleModel::load()
{
    beginResetModel();
    int length;
    KkcRuleMetadata** rules = kkc_rule_list(&length);
    for (int i = 0; i < length; i++) {
        int priority;
        g_object_get(G_OBJECT(rules[i]), "priority", &priority, NULL);
        if (priority < 70) {
            continue;
        }
        gchar* name, *label;
        g_object_get(G_OBJECT(rules[i]), "label", &label, "name", &name, NULL);
        m_rules << Rule(QString::fromUtf8(name), QString::fromUtf8(label));
        g_object_unref(rules[i]);
        g_free(name);
        g_free(label);
    }
    g_free(rules);
    endResetModel();
}

int RuleModel::rowCount(const QModelIndex& parent) const
{
    return m_rules.size();
}

QVariant RuleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_rules.size() || index.column() != 0) {
        return QVariant();
    }

    switch(role) {
        case Qt::DisplayRole:
            return m_rules[index.row()].label();
        case Qt::UserRole:
            return m_rules[index.row()].name();
    }
    return QVariant();
}

int RuleModel::findRule(const QString& name)
{
    int i = 0;
    Q_FOREACH (const Rule& rule, m_rules) {
        if (rule.name() == name) {
            return i;
        }
        i ++;
    }
    return -1;
}
