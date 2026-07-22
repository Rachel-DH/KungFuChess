#include "GameEngine.h"

#include "Parser.h"
#include "Piece.h"

namespace {

    PiecePhase piece_phase(bool is_moving, bool is_airborne) {
        if (is_airborne) {
            return PiecePhase::Jump;
        }
        if (is_moving) {
            return PiecePhase::Move;
        }
        return PiecePhase::Idle;
    }

} // namespace

GameEngine::GameEngine(Board board, long long move_ms_per_cell)
    : board_(std::move(board)), arbiter_(move_ms_per_cell) {
}

Board GameEngine::standard_start_board() {
    return Parser::parse_board({
        "bR bN bB bQ bK bB bN bR",
        "bP bP bP bP bP bP bP bP",
        ".  .  .  .  .  .  .  .",
        ".  .  .  .  .  .  .  .",
        ".  .  .  .  .  .  .  .",
        ".  .  .  .  .  .  .  .",
        "wP wP wP wP wP wP wP wP",
        "wR wN wB wQ wK wB wN wR",
    });
}

GameEngine GameEngine::standard_start(long long move_ms_per_cell) {
    return GameEngine(standard_start_board(), move_ms_per_cell);
}

bool GameEngine::is_selectable(Position cell) const {
    if (state_ == GameState::GameOver) {
        return false;
    }
    std::optional<Cell> piece = board_.get_at(cell.x, cell.y);
    return piece.has_value() && !arbiter_.is_moving(cell.x, cell.y) && !arbiter_.is_airborne(cell.x, cell.y);
}

std::optional<Color> GameEngine::color_at(Position cell) const {
    std::optional<Cell> piece = board_.get_at(cell.x, cell.y);
    if (!piece.has_value()) {
        return std::nullopt;
    }
    return piece->color;
}

bool GameEngine::request_move(Position start, Position dest) {
    if (state_ == GameState::GameOver) {
        return false;
    }

    std::optional<Cell> piece_at_start = board_.get_at(start.x, start.y);
    if (!piece_at_start.has_value()) {
        return false;
    }

    // Already moving: reject a redirect rather than rescheduling mid-flight (explicit, don't rely on conflicts_with_pending_move's incidental route-sharing rejection).
    if (arbiter_.is_moving(start.x, start.y)) {
        return false;
    }

    // An airborne piece is committed to its jump; it cannot move until it lands.
    if (arbiter_.is_airborne(start.x, start.y)) {
        return false;
    }

    const Piece* piece = PieceFactory::get_piece(piece_at_start->type);
    if (!piece || !piece->is_available_move(start.x, start.y, dest.x, dest.y, board_)) {
        return false;
    }

    if (arbiter_.conflicts_with_pending_move(start.x, start.y, dest.x, dest.y)) {
        return false;
    }

    arbiter_.schedule_move(start, dest, *piece_at_start);
    return true;
}

bool GameEngine::request_jump(Position cell) {
    if (state_ == GameState::GameOver) {
        return false;
    }

    std::optional<Cell> piece = board_.get_at(cell.x, cell.y);
    if (!piece.has_value() || arbiter_.is_moving(cell.x, cell.y) || arbiter_.is_airborne(cell.x, cell.y)) {
        return false;
    }

    arbiter_.start_jump(cell, *piece, kJumpDurationMs);
    return true;
}

// Ends the game if an enemy king was captured while settling.
void GameEngine::wait(int milliseconds) {
    if (milliseconds > 0 && arbiter_.advance(milliseconds, board_)) {
        state_ = GameState::GameOver;
    }
}

void GameEngine::print(std::ostream& out) const {
    out << Parser::board_to_string(board_) << "\n";
}

std::vector<PieceDisplayState> GameEngine::piece_display_states() const {
    std::vector<PieceDisplayState> states;
    for (int y = 0; y < board_.get_height(); ++y) {
        for (int x = 0; x < board_.get_width(); ++x) {
            std::optional<Cell> piece = board_.get_at(x, y);
            if (!piece.has_value()) {
                continue;
            }
            bool is_moving = arbiter_.is_moving(x, y);
            bool is_airborne = arbiter_.is_airborne(x, y);
            states.push_back(PieceDisplayState{
                y * board_.get_width() + x,
                Position{ x, y },
                piece->type,
                piece->color,
                is_moving,
                is_airborne,
                piece_phase(is_moving, is_airborne),
            });
        }
    }
    return states;
}