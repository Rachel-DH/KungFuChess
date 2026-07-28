#pragma once

#include <string>
#include <vector>

#include "model/GameEngine.h" // reuses PieceDisplayState's shape

// Client-side mirror of the server's authoritative GAME_STATE broadcast
// (§3): board, pieces, and game-over status. Never authoritative — every
// apply_game_state_json() call fully replaces the previous snapshot, the
// same way a real GAME_STATE broadcast is meant to overwrite local guesses
// (§3, §6): no delta/patch logic, no "trust the last predicted move".
class ClientState {
public:
    // Parses one GAME_STATE JSON message (GameStateSerializer::to_json's
    // wire shape) and replaces the current snapshot. Malformed JSON is
    // ignored, leaving the last known-good snapshot in place — a corrupt
    // server message should never crash the client or blank the board.
    void apply_game_state_json(const std::string& json_text);

    const std::vector<PieceDisplayState>& pieces() const { return pieces_; }
    bool game_over() const { return game_over_; }
    bool has_state() const { return has_state_; }

    // nullopt if no piece occupies cell in the current snapshot.
    const PieceDisplayState* piece_at(Position cell) const;

private:
    std::vector<PieceDisplayState> pieces_;
    bool game_over_ = false;
    bool has_state_ = false;
};
