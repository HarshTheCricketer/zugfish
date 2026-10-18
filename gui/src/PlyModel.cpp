#include "PlyModel.h"

QVariant PlyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size())
        return {};
    const Row &r = rows_[index.row()];
    switch (role) {
    case MoveNoRole:     return r.moveNo;
    case WhitePlyRole:   return r.wPly;
    case WhiteSanRole:   return r.wSan;
    case WhiteClassRole: return cls(r.wPly);
    case WhiteEvalRole:  return ev(r.wPly);
    case BlackPlyRole:   return r.bPly;      // 0 when the game ended on White's move
    case BlackSanRole:   return r.bSan;
    case BlackClassRole: return r.bPly ? cls(r.bPly) : -1;
    case BlackEvalRole:  return r.bPly ? ev(r.bPly) : QString();
    }
    return {};
}

QHash<int, QByteArray> PlyModel::roleNames() const
{
    return {{MoveNoRole, "moveNo"},
            {WhitePlyRole, "wPly"}, {WhiteSanRole, "wSan"}, {WhiteClassRole, "wCls"}, {WhiteEvalRole, "wEval"},
            {BlackPlyRole, "bPly"}, {BlackSanRole, "bSan"}, {BlackClassRole, "bCls"}, {BlackEvalRole, "bEval"}};
}

void PlyModel::setGame(const zq::GameData &game)
{
    beginResetModel();
    rows_.clear();
    classes_.clear();
    evals_.clear();
    for (int i = 0; i < game.plies.size(); i += 2) {
        Row r;
        r.moveNo = i / 2 + 1;
        r.wPly = i + 1;
        r.wSan = game.plies[i].san;
        r.bPly = (i + 1 < game.plies.size()) ? i + 2 : 0;
        r.bSan = r.bPly ? game.plies[i + 1].san : QString();
        rows_.push_back(r);
    }
    endResetModel();
}

void PlyModel::setReview(const QVector<int> &classes, const QVector<QString> &evalTexts)
{
    classes_ = classes;
    evals_ = evalTexts;
    if (!rows_.isEmpty())
        emit dataChanged(index(0), index(rows_.size() - 1));
}

void PlyModel::clearReview()
{
    setReview({}, {});
}
