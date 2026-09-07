#pragma once
#include <string>
#include <vector>
#include "05_Show.cpp"
#include "06_ShowSeat.cpp"
#include "07_Customer.cpp"
using namespace std;

enum class BookingStatus { CONFIRMED, FAILED, CANCELLED };

class Booking {
private:
    static int nextBookingId;
    string bookingId;
    Show* show;
    Customer customer;
    vector<ShowSeat*> seats;
    double totalAmount;
    BookingStatus status;

public:
    Booking(Show* show, Customer customer, vector<ShowSeat*> seats, double totalAmount)
        : customer(customer) {
        this->show = show;
        this->seats = seats;
        this->totalAmount = totalAmount;
        this->status = BookingStatus::FAILED;

        nextBookingId++;
        this->bookingId = "BK" + to_string(1000 + nextBookingId);
    }

    string getBookingId() const { return bookingId; }
    Show* getShow() const { return show; }
    vector<ShowSeat*> getSeats() const { return seats; }
    double getTotalAmount() const { return totalAmount; }
    BookingStatus getStatus() const { return status; }

    void confirm() { status = BookingStatus::CONFIRMED; }
    void fail() { status = BookingStatus::FAILED; }
    void cancel() { status = BookingStatus::CANCELLED; }
};

int Booking::nextBookingId = 0;
