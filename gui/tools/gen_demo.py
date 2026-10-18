#!/usr/bin/env python3
"""Generate data/demo_games.json: famous games reviewed with Stockfish.

Used by the built-in demo backend so the UI runs without the zugfish core.
The classification heuristics here are only for demo data; the real review
logic lives in your zugfish core.

    python3 tools/gen_demo.py /usr/games/stockfish data/demo_games.json
"""
import io, json, math, sys
import chess, chess.engine, chess.pgn

GAMES = {
 "opera": ('''[Event "Opera Game"][Site "Paris"][Date "1858.??.??"][White "Paul Morphy"][Black "Duke of Brunswick & Count Isouard"][Result "1-0"][Opening "Philidor Defense"]
1. e4 e5 2. Nf3 d6 3. d4 Bg4 4. dxe5 Bxf3 5. Qxf3 dxe5 6. Bc4 Nf6 7. Qb3 Qe7 8. Nc3 c6 9. Bg5 b5 10. Nxb5 cxb5 11. Bxb5+ Nbd7 12. O-O-O Rd8 13. Rxd7 Rxd7 14. Rd1 Qe6 15. Bxd7+ Nxd7 16. Qb8+ Nxb8 17. Rd8# 1-0'''),
 "immortal": ('''[Event "The Immortal Game"][Site "London"][Date "1851.06.21"][White "Adolf Anderssen"][Black "Lionel Kieseritzky"][Result "1-0"][Opening "King's Gambit Accepted"]
1. e4 e5 2. f4 exf4 3. Bc4 Qh4+ 4. Kf1 b5 5. Bxb5 Nf6 6. Nf3 Qh6 7. d3 Nh5 8. Nh4 Qg5 9. Nf5 c6 10. g4 Nf6 11. Rg1 cxb5 12. h4 Qg6 13. h5 Qg5 14. Qf3 Ng8 15. Bxf4 Qf6 16. Nc3 Bc5 17. Nd5 Qxb2 18. Bd6 Bxg1 19. e5 Qxa1+ 20. Ke2 Na6 21. Nxg7+ Kd8 22. Qf6+ Nxf6 23. Be7# 1-0'''),
 "legall": ('''[Event "Legal's Mate"][Site "Paris"][Date "1750.??.??"][White "Sire de Legal"][Black "Saint Brie"][Result "1-0"][Opening "Philidor Defense"]
1. e4 e5 2. Nf3 d6 3. Bc4 Bg4 4. Nc3 g6 5. Nxe5 Bxd1 6. Bxf7+ Ke7 7. Nd5# 1-0'''),
}
VAL = {chess.PAWN: 1, chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9, chess.KING: 0}
CLS = ["Brilliant","Great","Best","Excellent","Good","Book","Inaccuracy","Mistake","Blunder","Miss","Forced"]

def win(cp):            # lichess curve, 0..100 for White
    return 50 + 50 * (2 / (1 + math.exp(-0.00368208 * cp)) - 1)

def white_cp(score):    # (value, isMate) from a PovScore, White's point of view
    s = score.white()
    if s.is_mate():
        m = s.mate()
        return m, True
    return s.score(), False

def win_of(value, is_mate, board_after_is_mate=False):
    if is_mate:
        return 100.0 if value > 0 else 0.0 if value < 0 else 50.0
    return win(value)

def material(board, color):
    return sum(VAL[p.piece_type] for p in board.piece_map().values() if p.color == color)

def analyse(engine, board):
    if board.is_checkmate():
        # side to move is mated: the other side won
        return {"value": 0, "isMate": True, "win": 0.0 if board.turn == chess.WHITE else 100.0,
                "best": "", "bestSan": "", "second": None}
    if board.is_game_over():
        return {"value": 0, "isMate": False, "win": 50.0, "best": "", "bestSan": "", "second": None}
    infos = engine.analyse(board, chess.engine.Limit(time=0.30), multipv=2)
    top = infos[0]
    v, m = white_cp(top["score"])
    best = top["pv"][0]
    second = None
    if len(infos) > 1:
        v2, m2 = white_cp(infos[1]["score"])
        second = win_of(v2, m2)
    return {"value": v, "isMate": m, "win": win_of(v, m), "best": best.uci(),
            "bestSan": board.san(best), "second": second}

def normalise(text):
    text = text.replace("][", "]\n[")
    return text.replace("]\n1.", "]\n\n1.")

def build(engine, key, text):
    text = normalise(text)
    game = chess.pgn.read_game(io.StringIO(text))
    board = game.board()
    positions = [analyse(engine, board)]
    plies, boards = [], [board.copy()]
    for mv in game.mainline_moves():
        san = board.san(mv)
        plies.append({"san": san, "uci": mv.uci(), "fenBefore": board.fen()})
        board.push(mv)
        boards.append(board.copy())
        positions.append(analyse(engine, board))
    out = []
    acc = {True: [], False: []}
    counts = {True: {c: 0 for c in CLS}, False: {c: 0 for c in CLS}}
    for i, ply in enumerate(plies, start=1):
        white = (i % 2 == 1)
        b0, p0, p1 = boards[i-1], positions[i-1], positions[i]
        mv = chess.Move.from_uci(ply["uci"])
        w0 = p0["win"] if white else 100 - p0["win"]
        w1 = p1["win"] if white else 100 - p1["win"]
        loss = max(0.0, w0 - w1)
        is_best = (ply["uci"] == p0["best"])
        legal = b0.legal_moves.count()
        # sacrifice test: moved piece (>=3) lands where the engine's reply captures it
        sac = False
        piece = b0.piece_at(mv.from_square)
        if piece and VAL[piece.piece_type] >= 3 and p1["best"]:
            rep = chess.Move.from_uci(p1["best"])
            if rep.to_square == mv.to_square and boards[i].is_capture(rep):
                gained = VAL[b0.piece_at(mv.to_square).piece_type] if b0.is_capture(mv) and b0.piece_at(mv.to_square) else 0
                if gained - VAL[piece.piece_type] <= -3:      # a real material sacrifice
                    sac = True
        gap = (p0["win"] - p0["second"]) if (p0["second"] is not None and white) else \
              ((p0["second"] - p0["win"]) if p0["second"] is not None else 0)
        if legal == 1:                      cls = "Forced"
        elif i <= 6 and loss < 2:           cls = "Book"
        elif sac and loss < 3 and (w1 >= 55 or p1["isMate"]) and (is_best or loss < 1): cls = "Brilliant"
        elif is_best and gap >= 25 and 25 < w0 < 97: cls = "Great"
        elif is_best or loss < 0.5:         cls = "Best"
        elif loss < 2:                      cls = "Excellent"
        elif loss < 5:                      cls = "Good"
        elif w0 >= 85 and loss >= 10:       cls = "Miss"
        elif loss >= 15:                    cls = "Blunder"
        elif loss >= 10:                    cls = "Mistake"
        else:                               cls = "Inaccuracy"
        cp_loss = 0
        if not p0["isMate"] and not p1["isMate"]:
            a = p0["value"] if white else -p0["value"]
            c = p1["value"] if white else -p1["value"]
            cp_loss = max(0, a - c)
        accuracy = max(0.0, min(100.0, 103.1668 * math.exp(-0.04354 * loss) - 3.1669))
        acc[white].append(accuracy)
        counts[white][cls] += 1
        out.append({"cls": CLS.index(cls), "winWhite": round(p1["win"], 2),
                    "scoreValue": p1["value"], "isMate": p1["isMate"],
                    "bestBeforeUci": p0["best"], "bestBeforeSan": p0["bestSan"],
                    "nextBestUci": p1["best"], "nextBestSan": p1["bestSan"],
                    "winLoss": round(loss / 100.0, 4), "cpLoss": int(cp_loss)})
    headers = {k: str(v) for k, v in game.headers.items() if v and v != "?"}
    return {"id": key, "headers": headers, "plies": plies, "finalFen": board.fen(),
            "review": {"initial": {"scoreValue": positions[0]["value"], "isMate": positions[0]["isMate"],
                                   "winWhite": round(positions[0]["win"], 2),
                                   "bestUci": positions[0]["best"], "bestSan": positions[0]["bestSan"]},
                       "plies": out,
                       "whiteAccuracy": round(sum(acc[True]) / max(1, len(acc[True])), 1),
                       "blackAccuracy": round(sum(acc[False]) / max(1, len(acc[False])), 1),
                       "counts": {"white": counts[True], "black": counts[False]}}}

if __name__ == "__main__":
    sf, dest = sys.argv[1], sys.argv[2]
    engine = chess.engine.SimpleEngine.popen_uci(sf)
    engine.configure({"Threads": 2, "Hash": 128})
    games = []
    for key, text in GAMES.items():
        g = build(engine, key, text)
        print(key, len(g["plies"]), "plies", g["review"]["whiteAccuracy"], g["review"]["blackAccuracy"],
              {w: {k: v for k, v in c.items() if v} for w, c in g["review"]["counts"].items()})
        games.append(g)
    engine.quit()
    with open(dest, "w") as fh:
        json.dump(games, fh, separators=(",", ":"))
    # samples.json: what the welcome screen offers (works with any backend)
    samples = []
    for g in games:
        h = g["headers"]
        pgn = normalise(GAMES[g["id"]])
        samples.append({"id": g["id"], "title": h.get("Event", g["id"]),
                        "subtitle": f'{h.get("White","?")} vs {h.get("Black","?")}',
                        "year": h.get("Date", "")[:4], "plies": len(g["plies"]), "pgn": pgn})
    with open(dest.replace("demo_games", "samples"), "w") as fh:
        json.dump(samples, fh, indent=1)
