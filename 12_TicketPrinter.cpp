#pragma once
#include <iostream>
#include <vector>
#include "08_Booking.cpp"
using namespace std;

class TicketPrinter {
public:
    static void printTicket(Booking* booking) {
        cout << "================ TICKET ================" << endl;
        cout << "Booking ID : " << booking->getBookingId() << endl;
        cout << "Movie      : " << booking->getShow()->getMovie()->getTitle() << endl;
        cout << "Screen     : Screen-" << booking->getShow()->getScreen()->getScreenNumber()
             << "  " << booking->getShow()->getStartTime() << endl;

        cout << "Seats      : ";
        vector<ShowSeat*> seats = booking->getSeats();
        for (int i = 0; i < (int)seats.size(); i++) {
            cout << seats[i]->getSeat().getSeatNumber();
            if (i != (int)seats.size() - 1) cout << ", ";
        }
        cout << endl;

        string statusText;
        BookingStatus status = booking->getStatus();
        if (status == BookingStatus::CONFIRMED) statusText = "CONFIRMED";
        else if (status == BookingStatus::CANCELLED) statusText = "CANCELLED";
        else statusText = "FAILED";
        cout << "Amount     : Rs." << booking->getTotalAmount() << "   Status: " << statusText << endl;
        cout << "=========================================" << endl;
    }
};
