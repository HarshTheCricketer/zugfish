// Self-contained backend: replays pre-reviewed famous games from data/demo_games.json.
// It cannot parse arbitrary PGN; it recognises the bundled sample games.
#pragma once

#include "IBackend.h"

namespace zq {

class DemoBackend : public IBackend {
public:
    DemoBackend();

    QString name() const override { return QStringLiteral("Demo data"); }
    ParseResult parsePgn(const QString &pgnText) override;
    bool review(const GameData &game, int depth, const QString &enginePath,
                const ProgressFn &progress, const CancelFn &cancelled,
                ReviewData &out, QString &error) override;
    QVector<OnlineGame> fetchGames(Platform platform, const QString &user,
                                   int maxGames, QString &error) override;

private:
    struct Entry { QString id, pgn; GameData game; ReviewData review; };
    QVector<Entry> entries_;
    const Entry *find(const QMap<QString, QString> &headers) const;
};

} // namespace zq
