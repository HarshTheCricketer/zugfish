// Adapter from the real zugfish core (UCI engine, PGN parser, reviewer, online importer)
// to the UI's IBackend. Enabled with -DZUGFISH_CORE_DIR=/path/to/zugfish.
#pragma once

#include "IBackend.h"

namespace zq {

class ZugfishBackend : public IBackend {
public:
    QString name() const override { return QStringLiteral("zugfish core"); }
    ParseResult parsePgn(const QString &pgnText) override;
    bool review(const GameData &game, int depth, const QString &enginePath,
                const ProgressFn &progress, const CancelFn &cancelled,
                ReviewData &out, QString &error) override;
    QVector<OnlineGame> fetchGames(Platform platform, const QString &user,
                                   int maxGames, QString &error) override;
};

} // namespace zq
