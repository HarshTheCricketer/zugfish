// One row per full move: "12.  Nxb5  cxb5". Feeds the Moves tab.
#pragma once

#include <QAbstractListModel>
#include "ReviewTypes.h"

class PlyModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        MoveNoRole = Qt::UserRole + 1,
        WhitePlyRole, WhiteSanRole, WhiteClassRole, WhiteEvalRole,
        BlackPlyRole, BlackSanRole, BlackClassRole, BlackEvalRole
    };
    explicit PlyModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = {}) const override { return parent.isValid() ? 0 : rows_.size(); }
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setGame(const zq::GameData &game);
    // evalTexts / classes have one entry per ply (empty = no review yet).
    void setReview(const QVector<int> &classes, const QVector<QString> &evalTexts);
    void clearReview();

private:
    struct Row { int moveNo; int wPly; QString wSan; int bPly; QString bSan; };
    QVector<Row> rows_;
    QVector<int> classes_;
    QVector<QString> evals_;
    int cls(int ply) const { return ply >= 1 && ply <= classes_.size() ? classes_[ply - 1] : -1; }
    QString ev(int ply) const { return ply >= 1 && ply <= evals_.size() ? evals_[ply - 1] : QString(); }
};
