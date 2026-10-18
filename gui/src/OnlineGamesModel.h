#pragma once

#include <QAbstractListModel>
#include "ReviewTypes.h"

class OnlineGamesModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { DateRole = Qt::UserRole + 1, WhiteRole, BlackRole, WhiteRatingRole, BlackRatingRole, SpeedRole, ResultRole };
    explicit OnlineGamesModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = {}) const override { return parent.isValid() ? 0 : games_.size(); }
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setGames(const QVector<zq::OnlineGame> &games);
    const zq::OnlineGame *at(int row) const { return row >= 0 && row < games_.size() ? &games_[row] : nullptr; }

private:
    QVector<zq::OnlineGame> games_;
};
