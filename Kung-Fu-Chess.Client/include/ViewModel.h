#pragma once

#include <optional>
#include <string>

#include "model/Position.h"
#include "utils/Types.h"

#include "ClientState.h"

// UI logic (§3): selection and soft (client-side, non-authoritative)
// move-legality checking. Purely a UX convenience — every move attempt this
// produces still goes through the server's own authoritative validation;
// this class's opinion is never trusted on its own (§3).
class ViewModel {
public:
    explicit ViewModel(Color my_color) : my_color_(my_color) {}

    // Returns the MOVE command JSON to send if this click completed a
    // soft-legal move attempt against the current snapshot; nullopt for a
    // selection change, a deselect, or a click ruled out client-side.
    std::optional<std::string> on_click(Position cell, const ClientState& state);

    std::optional<Position> selected() const { return selected_; }

private:
    Color my_color_;
    std::optional<Position> selected_;

    bool is_soft_legal(Position start, Position dest, const ClientState& state) const;
};
