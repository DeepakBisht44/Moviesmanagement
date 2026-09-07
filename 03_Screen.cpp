#pragma once
#include <vector>
#include "02_Seat.cpp"
using namespace std;

class Screen {
private:
    int screenNumber;
    vector<Seat> seats;

public:
    Screen(int screenNumber, vector<Seat> seats) {
        this->screenNumber = screenNumber;
        this->seats = seats;
    }

    int getScreenNumber() const { return screenNumber; }
    vector<Seat> getSeats() const { return seats; }
};
