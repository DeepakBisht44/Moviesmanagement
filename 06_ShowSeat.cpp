#pragma once
#include "02_Seat.cpp"
using namespace std;

enum class SeatStatus { AVAILABLE, BOOKED };

class ShowSeat {
private:
    Seat seat;
    SeatStatus status;

public:
    ShowSeat(Seat seat) : seat(seat) {
        status = SeatStatus::AVAILABLE;
    }

    Seat getSeat() const { return seat; }
    SeatStatus getStatus() const { return status; }
    bool isAvailable() const { return status == SeatStatus::AVAILABLE; }

    void markBooked() { status = SeatStatus::BOOKED; }
    void markAvailable() { status = SeatStatus::AVAILABLE; }
};
