#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "05_Show.cpp"
#include "06_ShowSeat.cpp"
#include "07_Customer.cpp"
#include "08_Booking.cpp"
#include "09_Payment.cpp"
#include "10_PaymentTypes.cpp"
#include "11_PriceCalculator.cpp"
#include "12_TicketPrinter.cpp"
using namespace std;

class BookingService {
private:
    vector<Booking*> bookings;

    vector<string> splitSeatNumbers(string input) {
        vector<string> result;
        stringstream ss(input);
        string token;
        while (getline(ss, token, ',')) {
            result.push_back(token);
        }
        return result;
    }

    ShowSeat* findShowSeat(Show* show, string seatNumber) {
        vector<ShowSeat>& seats = show->getShowSeats();
        for (int i = 0; i < (int)seats.size(); i++) {
            if (seats[i].getSeat().getSeatNumber() == seatNumber) {
                return &seats[i];
            }
        }
        return nullptr;
    }

public:
    Booking* bookTicket(Show* show, Customer customer, string seatInput, int paymentChoice, bool simulateSuccess) {
        vector<string> seatNumbers = splitSeatNumbers(seatInput);
        vector<ShowSeat*> selectedSeats;

        for (int i = 0; i < (int)seatNumbers.size(); i++) {
            ShowSeat* showSeat = findShowSeat(show, seatNumbers[i]);
            if (showSeat == nullptr) {
                cout << "Invalid seat number: " << seatNumbers[i] << endl;
                return nullptr;
            }
            if (!showSeat->isAvailable()) {
                cout << "Seat already booked: " << seatNumbers[i] << endl;
                return nullptr;
            }
            selectedSeats.push_back(showSeat);
        }

        double total = PriceCalculator::calculateTotal(selectedSeats);

        Payment* payment;
        if (paymentChoice == 1) payment = new UpiPayment();
        else if (paymentChoice == 2) payment = new CardPayment();
        else payment = new CashPayment();

        Booking* booking = new Booking(show, customer, selectedSeats, total);

        bool paymentSuccess = simulateSuccess && payment->pay(total);

        if (paymentSuccess) {
            for (int i = 0; i < (int)selectedSeats.size(); i++) {
                selectedSeats[i]->markBooked();
            }
            booking->confirm();
        } else {
            cout << "Payment failed. Booking not confirmed." << endl;
            booking->fail();
        }

        delete payment;
        bookings.push_back(booking);
        return booking;
    }

    bool cancelBooking(string bookingId) {
        for (int i = 0; i < (int)bookings.size(); i++) {
            if (bookings[i]->getBookingId() == bookingId &&
                bookings[i]->getStatus() == BookingStatus::CONFIRMED) {
                vector<ShowSeat*> seats = bookings[i]->getSeats();
                for (int j = 0; j < (int)seats.size(); j++) {
                    seats[j]->markAvailable();
                }
                bookings[i]->cancel();
                return true;
            }
        }
        return false;
    }

    vector<Booking*> getBookings() const { return bookings; }
};
