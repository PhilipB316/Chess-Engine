#!/usr/bin/env python3
"""
TessMax UCI test suite.

Usage:  python3 test_uci.py [path_to_engine]
        python3 test_uci.py ./build/tessmax

Requires python-chess for move-legality and tactics sections:
        pip install chess
"""

import re
import subprocess
import sys

ENGINE  = sys.argv[1] if len(sys.argv) > 1 else "./tessmax"
TIMEOUT = 15

# ── colour output ─────────────────────────────────────────────────
OK   = "\033[32mPASS\033[0m"
ERR  = "\033[31mFAIL\033[0m"
SKIP = "\033[33mSKIP\033[0m"

passed = failed = skipped = 0


def ok(label, detail=""):
    global passed
    passed += 1
    suffix = f"  → {detail}" if detail else ""
    print(f"  {OK}  {label}{suffix}")


def fail(label, detail=""):
    global failed
    failed += 1
    suffix = f"  → {detail}" if detail else ""
    print(f"  {ERR}  {label}{suffix}")


def check(label, condition, detail=""):
    """Single assertion — exactly one of ok/fail is called."""
    if condition:
        ok(label, detail)
    else:
        fail(label, detail)


def skip(label, reason=""):
    global skipped
    skipped += 1
    print(f"  {SKIP}  {label}  [{reason}]")


def section(title):
    print(f"\n{'─' * 60}")
    print(f"  {title}")
    print(f"{'─' * 60}")


# ── raw engine communication ──────────────────────────────────────

def run_session(commands: list) -> str:
    payload = "\n".join(commands) + "\n"
    try:
        result = subprocess.run(
            [ENGINE],
            input=payload.encode(),
            capture_output=True,
            timeout=TIMEOUT,
        )
        return result.stdout.decode()
    except subprocess.TimeoutExpired:
        fail("ENGINE TIMED OUT", " ".join(commands[:3]))
        return ""
    except FileNotFoundError:
        print(f"\n  Engine not found: {ENGINE}")
        print("  Usage: python3 test_uci.py <path_to_engine>\n")
        sys.exit(1)


MOVE_RE = re.compile(r"^[a-h][1-8][a-h][1-8][qrbn]?$")


def extract_bestmove(output: str):
    for line in output.splitlines():
        if line.startswith("bestmove"):
            parts = line.split()
            return parts[1] if len(parts) >= 2 else None
    return None


def valid_uci_move(move) -> bool:
    return bool(MOVE_RE.match(move)) if move else False


# ═══════════════════════════════════════════════════════════════════
# 1. Protocol compliance
# ═══════════════════════════════════════════════════════════════════

section("1. Protocol compliance")

out = run_session(["uci", "quit"])
check("uci → uciok",    "uciok"     in out)
check("id name present", "id name"  in out)
check("id author present","id author" in out)

out = run_session(["isready", "quit"])
check("isready → readyok", "readyok" in out)

out = run_session(["uci", "isready", "quit"])
check("uci then isready both work", "uciok" in out and "readyok" in out)

out = run_session(["setoption name Hash value 32", "isready", "quit"])
check("setoption silently accepted", "readyok" in out)

out = run_session(["unknowncommand foo bar", "isready", "quit"])
check("unknown command ignored (no crash)", "readyok" in out)

# ═══════════════════════════════════════════════════════════════════
# 2. Bestmove format
# ═══════════════════════════════════════════════════════════════════

section("2. Bestmove format")

FORMAT_POSITIONS = [
    ("startpos",
     "position startpos"),
    ("after 1.e4 e5",
     "position startpos moves e2e4 e7e5"),
    ("after 1.e4 e5 2.Nf3 Nc6",
     "position startpos moves e2e4 e7e5 g1f3 b8c6"),
    ("mid-game FEN (black to move)",
     "position fen r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 2 3"),
    ("endgame FEN (white to move)",
     "position fen 8/8/3k4/8/8/3K4/4R3/8 w - - 0 1"),
]

for label, pos_cmd in FORMAT_POSITIONS:
    out = run_session([pos_cmd, "go movetime 500", "quit"])
    mv  = extract_bestmove(out)
    check(label, valid_uci_move(mv), mv or repr(mv))

# ═══════════════════════════════════════════════════════════════════
# 3. go parameter variants
# ═══════════════════════════════════════════════════════════════════

section("3. go parameter variants")

GO_VARIANTS = [
    ("go movetime 200",
     "go movetime 200"),
    ("go depth 4",
     "go depth 4"),
    ("go depth 8",
     "go depth 8"),
    ("go wtime 60000 btime 60000",
     "go wtime 60000 btime 60000 winc 0 binc 0"),
    ("go wtime 5000 btime 5000 winc 500 binc 500",
     "go wtime 5000 btime 5000 winc 500 binc 500"),
    ("go wtime 500 btime 500 (low time)",
     "go wtime 500 btime 500"),
]

for label, go_cmd in GO_VARIANTS:
    out = run_session(["position startpos", go_cmd, "quit"])
    mv  = extract_bestmove(out)
    check(label, valid_uci_move(mv), mv or repr(mv))

# ═══════════════════════════════════════════════════════════════════
# 4. ucinewgame
# ═══════════════════════════════════════════════════════════════════

section("4. ucinewgame")

out = run_session(["ucinewgame", "isready", "quit"])
check("isready after ucinewgame", "readyok" in out)

out = run_session(["ucinewgame", "position startpos", "go movetime 300", "quit"])
mv  = extract_bestmove(out)
check("bestmove after ucinewgame", valid_uci_move(mv), mv or repr(mv))

out = run_session([
    "ucinewgame",
    "position startpos moves e2e4",
    "go movetime 300",
    "ucinewgame",
    "position startpos moves d2d4",
    "go movetime 300",
    "quit",
])
bm_count = sum(1 for l in out.splitlines() if l.startswith("bestmove"))
check("two sequential games both produce bestmove", bm_count == 2,
      f"got {bm_count} bestmove lines")

# ═══════════════════════════════════════════════════════════════════
# 5. position fen + moves list
# ═══════════════════════════════════════════════════════════════════

section("5. position fen + moves list")

FEN_MOVES_CASES = [
    ("Ruy Lopez + 1 extra move",
     "position fen r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3 moves a7a6",
     "go movetime 400"),
    ("Sicilian + 4 moves",
     "position fen rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"
     " moves g1f3 b8c6 d2d4 c5d4",
     "go movetime 400"),
    ("FEN with no castling rights",
     "position fen r1bqr1k1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQR1K1 w - - 4 9",
     "go movetime 400"),
]

for label, pos_cmd, go_cmd in FEN_MOVES_CASES:
    out = run_session([pos_cmd, go_cmd, "quit"])
    mv  = extract_bestmove(out)
    check(label, valid_uci_move(mv), mv or repr(mv))

# ═══════════════════════════════════════════════════════════════════
# 6. Special move positions
# ═══════════════════════════════════════════════════════════════════

section("6. Special move positions (format)")

SPECIAL_POSITIONS = [
    ("en passant available (d6)",
     "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3"),
    ("castling rights available (both sides)",
     "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"),
    ("white pawn on 7th (promotion imminent)",
     "8/P5k1/8/8/8/8/6K1/8 w - - 0 1"),
    ("black pawn on 2nd (promotion imminent)",
     "8/6k1/8/8/8/8/p5K1/8 b - - 0 1"),
]

for label, fen in SPECIAL_POSITIONS:
    out = run_session([f"position fen {fen}", "go movetime 600", "quit"])
    mv  = extract_bestmove(out)
    check(label, valid_uci_move(mv), mv or repr(mv))

# promotion must carry a piece suffix
out = run_session(["position fen 8/P5k1/8/8/8/8/6K1/8 w - - 0 1",
                   "go movetime 600", "quit"])
mv = extract_bestmove(out)
check("promotion move has piece suffix",
      valid_uci_move(mv) and len(mv) == 5 and mv[4] in "qrbn",
      mv or repr(mv))

# ═══════════════════════════════════════════════════════════════════
# python-chess sections — skip gracefully if not installed
# ═══════════════════════════════════════════════════════════════════

try:
    import chess
    import chess.engine
except ImportError:
    for s in ("7. Move legality", "8. Tactics — mate in 1",
              "9. Short game simulation", "10. Rapid position cycling"):
        skip(s, "pip install chess")
    section("Summary")
    total = passed + failed + skipped
    print(f"\n  {passed}/{total} passed  |  {failed} failed  |  {skipped} skipped\n")
    sys.exit(1 if failed else 0)

# ═══════════════════════════════════════════════════════════════════
# 7. Move legality
# ═══════════════════════════════════════════════════════════════════

section("7. Move legality (python-chess)")

LEGALITY_POSITIONS = [
    ("startpos",                   chess.Board()),
    ("after 1.e4 e5 2.Nf3 Nc6",   chess.Board("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3")),
    ("en passant position",        chess.Board("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3")),
    ("castling position",          chess.Board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1")),
    ("promotion position (white)", chess.Board("8/P5k1/8/8/8/8/6K1/8 w - - 0 1")),
    ("promotion position (black)", chess.Board("8/6k1/8/8/8/8/p5K1/8 b - - 0 1")),
    ("R+K vs K endgame",           chess.Board("8/8/3k4/8/8/8/4R3/3K4 w - - 0 1")),
    ("Q+K vs K endgame",           chess.Board("8/8/2k5/8/8/8/4Q3/3K4 w - - 0 1")),
]

eng = chess.engine.SimpleEngine.popen_uci(ENGINE)

for label, board in LEGALITY_POSITIONS:
    try:
        result = eng.play(board, chess.engine.Limit(time=0.5))
        if result.move in board.legal_moves:
            ok(label, board.san(result.move))
        else:
            fail(label, f"illegal: {result.move}")
    except Exception as e:
        fail(label, str(e))

# ═══════════════════════════════════════════════════════════════════
# 8. Tactics — mate in 1
# ═══════════════════════════════════════════════════════════════════

section("8. Tactics — mate in 1")

MATE_IN_1 = [
    ("back-rank mate",       "6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1"),
    ("smothered mate",       "6rk/6pp/7N/8/8/8/8/6K1 w - - 0 1"),
    ("ladder mate",          "k7/8/1R6/R7/8/8/8/4K3 w - - 0 1"),
    ("queen mate",           "k7/8/1K6/8/8/8/8/7Q w - - 0 1"),
]

for label, fen in MATE_IN_1:
    board = chess.Board(fen)
    try:
        result = eng.play(board, chess.engine.Limit(time=1.0))
        after = board.copy()
        after.push(result.move)
        if after.is_checkmate():
            ok(label, board.san(result.move))
        elif result.move in board.legal_moves:
            fail(label, f"missed mate — played {board.san(result.move)}")
        else:
            fail(label, f"illegal move: {result.move}")
    except Exception as e:
        fail(label, str(e))

# ═══════════════════════════════════════════════════════════════════
# 9. Short game simulation (20 plies, engine plays both sides)
# ═══════════════════════════════════════════════════════════════════

section("9. Short game simulation (20 plies)")

board = chess.Board()
try:
    for ply in range(20):
        if board.is_game_over():
            ok(f"game ended naturally at ply {ply} ({board.result()})")
            break
        result = eng.play(board, chess.engine.Limit(time=100))
        if result.move not in board.legal_moves:
            fail(f"illegal move at ply {ply + 1}", str(result.move))
            break
        board.push(result.move)
    else:
        ok("20 plies completed, all moves legal")
except Exception as e:
    fail("game simulation crashed", str(e))

# ═══════════════════════════════════════════════════════════════════
# 10. Rapid position cycling
# ═══════════════════════════════════════════════════════════════════

section("10. Rapid position cycling")

CYCLE_POSITIONS = [
    chess.Board(),
    chess.Board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"),
    chess.Board("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"),
    chess.Board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"),
    chess.Board("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3"),
    chess.Board("8/P5k1/8/8/8/8/6K1/8 w - - 0 1"),
]

errors = 0
for board in CYCLE_POSITIONS:
    try:
        result = eng.play(board, chess.engine.Limit(time=0.1))
        if result.move not in board.legal_moves:
            errors += 1
    except Exception as e:
        errors += 1

check("rapid cycling (6 positions × 0.1 s)", errors == 0,
      f"{errors} errors" if errors else "")

eng.quit()

# ── summary ───────────────────────────────────────────────────────
section("Summary")
total = passed + failed + skipped
bar   = "█" * passed + "░" * failed + "·" * skipped
print(f"\n  [{bar}]")
print(f"\n  {passed}/{total} passed  |  {failed} failed  |  {skipped} skipped\n")
sys.exit(1 if failed else 0)
