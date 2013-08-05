/***************************************************************************
 *   Copyright (C) 2013~2013 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
