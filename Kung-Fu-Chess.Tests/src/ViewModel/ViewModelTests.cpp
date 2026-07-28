#include "ThirdParty/doctest.h"

#include "ViewModel.h"

namespace {

ClientState standard_start_state() {
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":0,"y":6,"type":"P","color":"w","phase":"idle"},)"
        R"({"id":1,"x":0,"y":1,"type":"P","color":"b","phase":"idle"},)"
        R"({"id":2,"x":1,"y":7,"type":"N","color":"w","phase":"idle"}]})");
    return state;
}

} // namespace

TEST_SUITE("ViewModel::on_click") {

TEST_CASE("clicking my own idle piece selects it and sends nothing") {
    ViewModel vm(Color::w);
    ClientState state = standard_start_state();

    CHECK_FALSE(vm.on_click(Position{ 0, 6 }, state).has_value());
    REQUIRE(vm.selected().has_value());
    CHECK(*vm.selected() == Position{ 0, 6 });
}

TEST_CASE("clicking an opponent's piece with nothing selected does not select it") {
    ViewModel vm(Color::w);
    ClientState state = standard_start_state();

    CHECK_FALSE(vm.on_click(Position{ 0, 1 }, state).has_value());
    CHECK_FALSE(vm.selected().has_value());
}

TEST_CASE("selecting a piece then clicking a soft-legal destination sends a move command") {
    ViewModel vm(Color::w);
    ClientState state = standard_start_state();

    vm.on_click(Position{ 0, 6 }, state); // select the white pawn
    auto command = vm.on_click(Position{ 0, 4 }, state); // two squares forward

    REQUIRE(command.has_value());
    CHECK(command->find("\"type\":\"move\"") != std::string::npos);
    CHECK(command->find(R"("start":{"x":0,"y":6})") != std::string::npos);
    CHECK(command->find(R"("dest":{"x":0,"y":4})") != std::string::npos);
    CHECK_FALSE(vm.selected().has_value()); // selection clears once a move is sent
}

TEST_CASE("selecting a piece then clicking a soft-illegal destination sends nothing") {
    ViewModel vm(Color::w);
    ClientState state = standard_start_state();

    vm.on_click(Position{ 0, 6 }, state);
    auto command = vm.on_click(Position{ 5, 5 }, state); // not a legal pawn move

    CHECK_FALSE(command.has_value());
    CHECK_FALSE(vm.selected().has_value());
}

TEST_CASE("clicking the same cell twice deselects without sending anything") {
    ViewModel vm(Color::w);
    ClientState state = standard_start_state();

    vm.on_click(Position{ 0, 6 }, state);
    auto command = vm.on_click(Position{ 0, 6 }, state);

    CHECK_FALSE(command.has_value());
    CHECK_FALSE(vm.selected().has_value());
}

TEST_CASE("clicking a different one of my own pieces re-selects it instead of attempting a move") {
    ViewModel vm(Color::w);
    ClientState state = standard_start_state();

    vm.on_click(Position{ 0, 6 }, state);
    auto command = vm.on_click(Position{ 1, 7 }, state);

    CHECK_FALSE(command.has_value());
    REQUIRE(vm.selected().has_value());
    CHECK(*vm.selected() == Position{ 1, 7 });
}

TEST_CASE("a piece mid-move cannot be selected") {
    ViewModel vm(Color::w);
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":0,"y":5,"type":"P","color":"w","phase":"move"}]})");

    CHECK_FALSE(vm.on_click(Position{ 0, 5 }, state).has_value());
    CHECK_FALSE(vm.selected().has_value());
}

}
