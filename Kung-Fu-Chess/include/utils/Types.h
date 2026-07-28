#pragma once

enum class Color {
    w,
    b
};

enum class PieceType {
    K, // King
    Q, // Queen
    R, // Rook
    B, // Bishop
    N, // Knight
    P  // Pawn
};

enum class PiecePhase {
    Idle,
    Move,
    Jump
};

struct Cell {
    Color color;
    PieceType type;
};
