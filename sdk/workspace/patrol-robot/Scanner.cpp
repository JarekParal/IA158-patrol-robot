#include "Scanner.hpp"

Scanner::Scanner(ePortS sonar_port, PositionEvent& position_event)
    : _sonar(sonar_port) {
    position_event.insert(
            [this](PositionMessage msg) { received_position_message(msg); });
}

void Scanner::init() {}

void Scanner::task() {}

void Scanner::received_position_message(PositionMessage msg) {}
