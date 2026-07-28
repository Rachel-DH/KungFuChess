#include "ThirdParty/doctest.h"

#include "Board.h"
#include "RealTimeArbiter.h"

namespace {

constexpr long long MOVE_MS_PER_CELL = 1000;

} // namespace

TEST_SUITE("RealTimeArbiter::advance") {

TEST_CASE("a move onto an empty next cell steps normally") {
    Board board(3, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 2, 0 }, Cell{ Color::w, PieceType::R });
    CHECK_FALSE(arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board));

    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->color == Color::w);
    CHECK(board.get_at(1, 0)->type == PieceType::R);
    CHECK_FALSE(board.get_at(0, 0).has_value());
}

TEST_CASE("a move stops one cell short of a static friendly piece and does not capture it") {
    Board board(3, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    board.place_at(1, 0, Cell{ Color::w, PieceType::N });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 2, 0 }, Cell{ Color::w, PieceType::R });
    CHECK_FALSE(arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board));

    REQUIRE(board.get_at(0, 0).has_value());
    CHECK(board.get_at(0, 0)->type == PieceType::R);
    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->type == PieceType::N);
}

TEST_CASE("a blocked move does not skip ahead even when given enough time for the whole trip in one advance call") {
    Board board(3, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    board.place_at(1, 0, Cell{ Color::w, PieceType::N });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 2, 0 }, Cell{ Color::w, PieceType::R });
    arbiter.advance(static_cast<int>(2 * MOVE_MS_PER_CELL), board); // enough time for the whole trip

    REQUIRE(board.get_at(0, 0).has_value());
    CHECK(board.get_at(0, 0)->type == PieceType::R);
    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->type == PieceType::N);
}

TEST_CASE("a previously-blocked move resumes once the blocker is gone") {
    Board board(3, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    board.place_at(1, 0, Cell{ Color::w, PieceType::N });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 2, 0 }, Cell{ Color::w, PieceType::R });
    arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board); // blocked; stays at (0,0)

    board.clear_at(1, 0); // blocker removed directly
    arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board);

    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->color == Color::w);
    CHECK(board.get_at(1, 0)->type == PieceType::R);
    CHECK_FALSE(board.get_at(0, 0).has_value());
}

TEST_CASE("a move onto a cell occupied by an enemy piece is not blocked and captures in place") {
    Board board(3, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    board.place_at(1, 0, Cell{ Color::b, PieceType::N });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 2, 0 }, Cell{ Color::w, PieceType::R });
    arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board);

    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->color == Color::w);
    CHECK(board.get_at(1, 0)->type == PieceType::R);
    CHECK_FALSE(board.get_at(0, 0).has_value());
}

TEST_CASE("a move that reaches its destination by capturing the enemy king ends the game") {
    Board board(2, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    board.place_at(1, 0, Cell{ Color::b, PieceType::K });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 1, 0 }, Cell{ Color::w, PieceType::R });
    CHECK(arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board));

    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->color == Color::w);
    CHECK(board.get_at(1, 0)->type == PieceType::R);
}

TEST_CASE("a move captures an enemy king on an intermediate cell before reaching its declared destination") {
    Board board(3, 1);
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    board.place_at(1, 0, Cell{ Color::b, PieceType::K });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.schedule_move(Position{ 0, 0 }, Position{ 2, 0 }, Cell{ Color::w, PieceType::R });
    CHECK(arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board)); // only enough time for one step

    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->color == Color::w);
    CHECK(board.get_at(1, 0)->type == PieceType::R);
    CHECK(arbiter.is_moving(1, 0)); // still in transit toward (2,0)
}

TEST_CASE("a landing guard captures an enemy piece that stepped onto its cell mid-transit and ghost-cleans its pending move") {
    Board board(4, 1);
    board.place_at(1, 0, Cell{ Color::w, PieceType::B });
    board.place_at(0, 0, Cell{ Color::b, PieceType::N });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.start_jump(Position{ 1, 0 }, Cell{ Color::w, PieceType::B }, MOVE_MS_PER_CELL);
    arbiter.schedule_move(Position{ 0, 0 }, Position{ 3, 0 }, Cell{ Color::b, PieceType::N });
    arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board);

    CHECK_FALSE(board.get_at(1, 0).has_value());
    CHECK_FALSE(board.get_at(0, 0).has_value());
    CHECK_FALSE(arbiter.is_moving(1, 0));
}

TEST_CASE("a landing guard beats an arriving move to the same cell on the same tick") {
    Board board(2, 1);
    board.place_at(1, 0, Cell{ Color::b, PieceType::Q });
    board.place_at(0, 0, Cell{ Color::w, PieceType::R });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);

    arbiter.start_jump(Position{ 1, 0 }, Cell{ Color::b, PieceType::Q }, MOVE_MS_PER_CELL);
    arbiter.schedule_move(Position{ 0, 0 }, Position{ 1, 0 }, Cell{ Color::w, PieceType::R });
    arbiter.advance(static_cast<int>(MOVE_MS_PER_CELL), board);

    CHECK_FALSE(board.get_at(1, 0).has_value());
    CHECK_FALSE(board.get_at(0, 0).has_value());
}

TEST_CASE("a landing guard capturing a static enemy king ends the game") {
    Board board(2, 1);
    board.place_at(1, 0, Cell{ Color::b, PieceType::K });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);
    long long jump_duration_ms = MOVE_MS_PER_CELL / 2;

    arbiter.start_jump(Position{ 1, 0 }, Cell{ Color::w, PieceType::Q }, jump_duration_ms);
    CHECK(arbiter.advance(static_cast<int>(jump_duration_ms), board));

    CHECK_FALSE(board.get_at(1, 0).has_value());
}

TEST_CASE("a landing guard on an empty cell simply expires without capturing") {
    Board board(2, 1);
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);
    long long jump_duration_ms = MOVE_MS_PER_CELL / 2;

    arbiter.start_jump(Position{ 1, 0 }, Cell{ Color::w, PieceType::N }, jump_duration_ms);
    CHECK_FALSE(arbiter.advance(static_cast<int>(jump_duration_ms), board));

    CHECK_FALSE(board.get_at(1, 0).has_value());
}

TEST_CASE("a landing guard on its own undisturbed piece leaves it in place") {
    Board board(2, 1);
    board.place_at(1, 0, Cell{ Color::w, PieceType::Q });
    RealTimeArbiter arbiter(MOVE_MS_PER_CELL);
    long long jump_duration_ms = MOVE_MS_PER_CELL / 2;

    arbiter.start_jump(Position{ 1, 0 }, Cell{ Color::w, PieceType::Q }, jump_duration_ms);
    CHECK_FALSE(arbiter.advance(static_cast<int>(jump_duration_ms), board));

    REQUIRE(board.get_at(1, 0).has_value());
    CHECK(board.get_at(1, 0)->color == Color::w);
    CHECK(board.get_at(1, 0)->type == PieceType::Q);
}

}
