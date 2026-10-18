// The seam between the UI and the chess logic.
// DemoBackend ships with the UI so it runs standalone; ZugfishBackend adapts the
// real zugfish core (see CMake option ZUGFISH_CORE_DIR).
#pragma once

#include "../ReviewTypes.h"

namespace zq {

class IBackend {
public:
    virtual ~IBackend() = default;

    virtual QString name() const = 0;

    // Parse the first game found in `pgnText`.
    virtual ParseResult parsePgn(const QString &pgnText) = 0;

    // Analyse every position. Called on a worker thread; must poll `cancelled`.
    virtual bool review(const GameData &game, int depth, const QString &enginePath,
                        const ProgressFn &progress, const CancelFn &cancelled,
                        ReviewData &out, QString &error) = 0;

    // Blocking network call; runs on a worker thread.
    virtual QVector<OnlineGame> fetchGames(Platform platform, const QString &user,
                                           int maxGames, QString &error) = 0;
};

} // namespace zq
