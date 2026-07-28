#pragma once

#include <optional>
#include <string>
#include <vector>

#include "model/GameEngine.h"
#include "utils/Types.h"

#include "EventBus.h"

enum class PlayerRole {
    Opponent,
    Spectator
};

struct JoinResult {
    PlayerRole role;
    std::optional<Color> color; // set only when role == Opponent
};

// One match's session state (§4.2): which sockets are opponent vs. spectator,
// disconnect countdown timers, and the room-scoped EventBus. Owns a
// GameEngine to run the match on but implements no rules logic itself —
// GameManager only ever reaches this far, never into GameEngine's rules.
class GameRoom {
public:
    // Exact value is an open question in the plan (§10 #2); 15s is a
    // reasonable placeholder pending Rachel's final call.
    static constexpr long long kDisconnectTimeoutMs = 15000;

    explicit GameRoom(std::string id);

    const std::string& id() const { return id_; }
    GameEngine& engine() { return engine_; }
    const GameEngine& engine() const { return engine_; }
    EventBus& event_bus() { return event_bus_; }

    // First joiner becomes White, second becomes Black, everyone after that
    // is a spectator (§7). Re-joining with the same player_id you already
    // hold a slot under returns that same slot rather than assigning a new one.
    JoinResult join(const std::string& player_id);

    // Starts (or restarts, per §6 — the countdown resets on every individual
    // disconnect) a fresh countdown for player_id. A no-op for a player who
    // holds no opponent slot (spectators don't forfeit by disconnecting).
    void on_disconnect(const std::string& player_id);

    // Cancels any running countdown for player_id.
    void on_reconnect(const std::string& player_id);

    // A deliberate forfeit (§4.1/§6): ends the match immediately, bypassing
    // the disconnect-countdown path entirely.
    void resign(const std::string& player_id);

    // Advances every running disconnect countdown by elapsed_ms; returns the
    // color that just timed out and lost, if any (at most one per tick).
    std::optional<Color> tick(long long elapsed_ms);

    // nullopt for a spectator or a player_id that holds no slot at all — the
    // server main loop's anti-cheat gate (§4.1) rejects a MOVE/JUMP whose
    // sender isn't the color of the piece at the command's source cell.
    std::optional<Color> color_of_player(const std::string& player_id) const { return color_of(player_id); }

    // True for either opponent or a spectator — every connection this room's
    // GAME_STATE broadcast (§4.1, §4.4) needs to reach.
    bool has_player(const std::string& player_id) const;
    std::vector<std::string> all_players() const;

private:
    struct DisconnectTimer {
        std::string player_id;
        long long remaining_ms;
    };

    std::string id_;
    GameEngine engine_;
    EventBus event_bus_;
    std::optional<std::string> white_player_;
    std::optional<std::string> black_player_;
    std::vector<std::string> spectators_;
    std::vector<DisconnectTimer> disconnect_timers_;

    std::optional<Color> color_of(const std::string& player_id) const;
};
