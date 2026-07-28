#include "ThirdParty/doctest.h"

#include <sstream>

#include "model/GameEngine.h"
#include "input/Parser.h"
#include "control/RealTimeArbiter.h"

namespace {

std::string board_of(const GameEngine& engine) {
    std::ostringstream oss;
    engine.print(oss);
    return oss.str();
}

} // namespace

TEST_SUITE("GameEngine::rest") {

TEST_CASE("a piece cannot be selected immediately after its move settles") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 }); // wR to (1,0)
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);          // settles; now resting

    CHECK_FALSE(engine.is_selectable(Position{ 1, 0 }));
}

TEST_CASE("a piece cannot be moved again immediately after settling") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // settles; now resting

    CHECK_FALSE(engine.request_move(Position{ 1, 0 }, Position{ 2, 0 }));
    CHECK(board_of(engine) == ". wR .\n"); // unchanged
}

TEST_CASE("one millisecond before the rest window ends, the piece is still resting") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // settles; now resting

    engine.wait(RealTimeArbiter::REST_DURATION_MS - 1); // one ms shy of the rest window's end
    CHECK_FALSE(engine.request_move(Position{ 1, 0 }, Position{ 2, 0 }));
}

TEST_CASE("once the rest window fully elapses, the piece becomes selectable again") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // settles; now resting

    engine.wait(RealTimeArbiter::REST_DURATION_MS); // rest window fully elapsed
    CHECK(engine.is_selectable(Position{ 1, 0 }));
}

TEST_CASE("once the rest window fully elapses, the piece can be moved again") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // settles; now resting
    engine.wait(RealTimeArbiter::REST_DURATION_MS); // rest window fully elapsed

    CHECK(engine.request_move(Position{ 1, 0 }, Position{ 2, 0 }));
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(engine) == ". . wR\n");
}

TEST_CASE("a piece that has never moved is not resting") {
    GameEngine engine(Parser::parse_board({ "wK . ." }));
    CHECK(engine.is_selectable(Position{ 0, 0 }));
    CHECK(engine.request_move(Position{ 0, 0 }, Position{ 1, 0 }));
}

TEST_CASE("an enemy can still capture a resting piece") {
    GameEngine engine(Parser::parse_board({ "wR . bR" }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 }); // wR to (1,0)
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);          // settles; now resting

    CHECK(engine.request_move(Position{ 2, 0 }, Position{ 1, 0 })); // bR captures the resting wR
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(engine) == ". bR .\n");
}

TEST_CASE("capturing a resting piece gives the capturer its own fresh rest window, not the victim's") {
    GameEngine engine(Parser::parse_board({ "wR . bR ." }), 100); // move_ms_per_cell=100

    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(100); // clock=100; wR settles at (1,0), resting [100,1100)

    CHECK(engine.request_move(Position{ 2, 0 }, Position{ 1, 0 })); // bR captures wR
    engine.wait(100); // clock=200; bR settles at (1,0), fresh window [200,1200)

    engine.wait(899); // clock=1099
    CHECK_FALSE(engine.is_selectable(Position{ 1, 0 }));

    engine.wait(2); // clock=1101; past wR's original window end (1100), well before bR's expiry (1200)
    CHECK_FALSE(engine.is_selectable(Position{ 1, 0 })); // discriminating: no stale record from wR shields/expires this cell

    engine.wait(99); // clock=1200; bR's own window ends
    CHECK(engine.is_selectable(Position{ 1, 0 }));
}

TEST_CASE("has_activity is true only while a piece is resting") {
    GameEngine engine(Parser::parse_board({ "wR ." }), 100); // move_ms_per_cell=100

    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(100); // settles; no other pending/airborne activity
    CHECK(engine.has_activity());

    engine.wait(RealTimeArbiter::REST_DURATION_MS); // clock=1100, past the rest window
    CHECK_FALSE(engine.has_activity());
}

TEST_CASE("no resting record is created for a piece captured on arrival by an airborne guard") {
    GameEngine engine(Parser::parse_board({
        ".  .  .",
        "wK bR .",
        ".  .  .",
    }));
    engine.request_jump(Position{ 0, 1 });                    // wK at (0,1) jumps, guarding its cell
    engine.request_move(Position{ 1, 1 }, Position{ 0, 1 });   // bR moves onto wK's guarded cell

    engine.wait(GameEngine::JUMP_DURATION_MS); // bR is captured by the guard on arrival
    CHECK(board_of(engine) == ". . .\nwK . .\n. . .\n");
    CHECK(engine.is_selectable(Position{ 0, 1 })); // wK never moved; not resting
}

TEST_CASE("a resting piece can still jump") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });
    engine.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // settles; now resting

    CHECK(engine.request_jump(Position{ 1, 0 }));
}

TEST_CASE("rest gating takes effect within the same wait() call that produces the settle") {
    GameEngine engine(Parser::parse_board({ "wR ." }), 1000); // move_ms_per_cell=1000; a single step settles in one wait() call
    engine.request_move(Position{ 0, 0 }, Position{ 1, 0 });

    engine.wait(1000); // step and settle happen within this one call
    CHECK_FALSE(engine.is_selectable(Position{ 1, 0 }));
}

}
