#pragma once
#include <vector>
#include "06_ShowSeat.cpp"
using namespace std;

class PriceCalculator {
public:
    static const int SILVER_PRICE = 150;
    static const int GOLD_PRICE = 250;
    static const int PLATINUM_PRICE = 400;

    static double calculateTotal(vector<ShowSeat*> seats) {
        double total = 0;
        for (int i = 0; i < (int)seats.size(); i++) {
            SeatType type = seats[i]->getSeat().getType();
            if (type == SeatType::SILVER) total += SILVER_PRICE;
            else if (type == SeatType::GOLD) total += GOLD_PRICE;
            else total += PLATINUM_PRICE;
        }
        return total;
    }
};
