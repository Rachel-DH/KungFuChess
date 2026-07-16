#pragma once

// Pure animation phase for a single piece, derived from its current motion state.
// No timing/frame data yet — later steps add those on top of this mapping.
enum class PieceAnimPhase {
    Idle,
    Move,
    Jump
};

// Maps a piece's motion flags to its animation phase. Total and deterministic: if a
// piece is somehow both moving and airborne, Jump takes priority (defensive only —
// GameEngine's request_move/request_jump already prevent this from happening today).
inline PieceAnimPhase piece_anim_phase(bool is_moving, bool is_airborne) {
    if (is_airborne) {
        return PieceAnimPhase::Jump;
    }
    if (is_moving) {
        return PieceAnimPhase::Move;
    }
    return PieceAnimPhase::Idle;
}
