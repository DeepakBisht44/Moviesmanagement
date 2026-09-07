#include <iostream>
#include <vector>
#include <string>
#include "01_Movie.cpp"
#include "02_Seat.cpp"
#include "03_Screen.cpp"
#include "04_Cinema.cpp"
#include "06_ShowSeat.cpp"
#include "05_Show.cpp"
#include "07_Customer.cpp"
#include "09_Payment.cpp"
#include "10_PaymentTypes.cpp"
#include "08_Booking.cpp"
#include "11_PriceCalculator.cpp"
#include "12_TicketPrinter.cpp"
#include "13_BookingService.cpp"
using namespace std;

vector<Seat> buildStandardSeats() {
    vector<Seat> seats;
    seats.push_back(Seat("A1", SeatType::SILVER));
    seats.push_back(Seat("A2", SeatType::SILVER));
    seats.push_back(Seat("A3", SeatType::SILVER));
    seats.push_back(Seat("A4", SeatType::SILVER));
    seats.push_back(Seat("B1", SeatType::GOLD));
    seats.push_back(Seat("B2", SeatType::GOLD));
    seats.push_back(Seat("B3", SeatType::GOLD));
    seats.push_back(Seat("C1", SeatType::PLATINUM));
    seats.push_back(Seat("C2", SeatType::PLATINUM));
    return seats;
}

void printSeatLayout(Show* show) {
    cout << endl;
    cout << "SCREEN-" << show->getScreen()->getScreenNumber() << "   " << show->getStartTime()
         << "  |  " << show->getMovie()->getTitle() << endl;

    vector<ShowSeat>& seats = show->getShowSeats();
    string types[3] = {"SILVER", "GOLD", "PLATINUM"};
    SeatType typeEnums[3] = {SeatType::SILVER, SeatType::GOLD, SeatType::PLATINUM};

    for (int t = 0; t < 3; t++) {
        cout << types[t] << "  ";
        for (int i = 0; i < (int)seats.size(); i++) {
            if (seats[i].getSeat().getType() == typeEnums[t]) {
                cout << seats[i].getSeat().getSeatNumber()
                     << (seats[i].isAvailable() ? "[ ]" : "[X]") << " ";
            }
        }
        cout << endl;
    }
    cout << endl;
    cout << "( [ ] = available   [X] = booked )" << endl;
}

int main() {
    Cinema cinema("PVR");

    Screen screen1(1, buildStandardSeats());
    Screen screen2(2, buildStandardSeats());
    cinema.addScreen(screen1);
    cinema.addScreen(screen2);

    vector<Movie> movies;
    movies.push_back(Movie("3 Idiots", "Hindi", 170));
    movies.push_back(Movie("Interstellar", "English", 169));

    vector<Show> shows;
    shows.push_back(Show(&movies[0], &cinema.getScreens()[0], "06:00 PM"));
    shows.push_back(Show(&movies[1], &cinema.getScreens()[1], "09:00 PM"));

    BookingService bookingService;

    int choice;
    while (true) {
        cout << endl;
        cout << "===== MOVIE TICKET BOOKING =====" << endl;
        cout << "1. Movies  2. Book  3. Cancel  4. My tickets   0. Exit" << endl;
        cout << "Choose: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input." << endl;
            continue;
        }

        if (choice == 0) {
            break;
        }
        else if (choice == 1) {
            cout << endl;
            for (int i = 0; i < (int)movies.size(); i++) {
                cout << "[" << i + 1 << "] " << movies[i].getTitle() << "  "
                     << movies[i].getLanguage() << "  " << movies[i].getDuration() << " min" << endl;
            }
        }
        else if (choice == 2) {
            cout << endl;
            for (int i = 0; i < (int)movies.size(); i++) {
                cout << "[" << i + 1 << "] " << movies[i].getTitle() << endl;
            }
            cout << "Choose movie: ";
            int movieChoice;
            cin >> movieChoice;

            if (movieChoice < 1 || movieChoice > (int)movies.size()) {
                cout << "Invalid movie choice." << endl;
                continue;
            }

            vector<Show*> matchingShows;
            for (int i = 0; i < (int)shows.size(); i++) {
                if (&movies[movieChoice - 1] == shows[i].getMovie()) {
                    matchingShows.push_back(&shows[i]);
                }
            }

            if (matchingShows.empty()) {
                cout << "No shows for this movie." << endl;
                continue;
            }

            for (int i = 0; i < (int)matchingShows.size(); i++) {
                cout << "[" << i + 1 << "] Screen-" << matchingShows[i]->getScreen()->getScreenNumber()
                     << "  " << matchingShows[i]->getStartTime() << endl;
            }
            cout << "Choose show: ";
            int showChoice;
            cin >> showChoice;

            if (showChoice < 1 || showChoice > (int)matchingShows.size()) {
                cout << "Invalid show choice." << endl;
                continue;
            }

            Show* selectedShow = matchingShows[showChoice - 1];
            printSeatLayout(selectedShow);

            cout << "Seats (e.g. A1,B2): ";
            string seatInput;
            cin >> seatInput;

            cout << "Enter customer name: ";
            string name;
            cin >> name;
            cout << "Enter phone: ";
            string phone;
            cin >> phone;
            Customer customer(name, phone);

            cout << "Pay by: 1.UPI  2.Card  3.Cash > ";
            int paymentChoice;
            cin >> paymentChoice;

            cout << "Simulate payment: 1.Success  2.Failure > ";
            int simChoice;
            cin >> simChoice;
            bool simulateSuccess = (simChoice == 1);

            Booking* booking = bookingService.bookTicket(selectedShow, customer, seatInput, paymentChoice, simulateSuccess);

            if (booking != nullptr && booking->getStatus() == BookingStatus::CONFIRMED) {
                TicketPrinter::printTicket(booking);
            }
        }
        else if (choice == 3) {
            cout << "Enter booking ID to cancel: ";
            string bookingId;
            cin >> bookingId;
            bool cancelled = bookingService.cancelBooking(bookingId);
            if (cancelled) {
                cout << "Booking cancelled. Seats released." << endl;
            } else {
                cout << "Booking not found or already cancelled." << endl;
            }
        }
        else if (choice == 4) {
            vector<Booking*> allBookings = bookingService.getBookings();
            if (allBookings.empty()) {
                cout << "No bookings yet." << endl;
            }
            for (int i = 0; i < (int)allBookings.size(); i++) {
                TicketPrinter::printTicket(allBookings[i]);
            }
        }
        else {
            cout << "Invalid choice." << endl;
        }
    }

    cout << "Thank you for visiting!" << endl;
    return 0;
}
