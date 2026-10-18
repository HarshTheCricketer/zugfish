#include "OnlineGamesModel.h"

QVariant OnlineGamesModel::data(const QModelIndex &index, int role) const
{
    const zq::OnlineGame *g = at(index.row());
    if (!g)
        return {};
    switch (role) {
    case DateRole:        return g->date;
    case WhiteRole:       return g->white;
    case BlackRole:       return g->black;
    case WhiteRatingRole: return g->whiteRating;
    case BlackRatingRole: return g->blackRating;
    case SpeedRole:       return g->speed;
    case ResultRole:      return g->result;
    }
    return {};
}

QHash<int, QByteArray> OnlineGamesModel::roleNames() const
{
    return {{DateRole, "date"}, {WhiteRole, "white"}, {BlackRole, "black"},
            {WhiteRatingRole, "whiteRating"}, {BlackRatingRole, "blackRating"},
            {SpeedRole, "speed"}, {ResultRole, "result"}};
}

void OnlineGamesModel::setGames(const QVector<zq::OnlineGame> &games)
{
    beginResetModel();
    games_ = games;
    endResetModel();
}
