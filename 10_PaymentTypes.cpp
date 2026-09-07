#pragma once
#include <iostream>
#include "09_Payment.cpp"
using namespace std;

class UpiPayment : public Payment {
public:
    bool pay(double amount) override {
        cout << "[UPI] Rs." << amount << " paid successfully" << endl;
        return true;
    }
};

class CardPayment : public Payment {
public:
    bool pay(double amount) override {
        cout << "[CARD] Rs." << amount << " paid successfully" << endl;
        return true;
    }
};

class CashPayment : public Payment {
public:
    bool pay(double amount) override {
        cout << "[CASH] Rs." << amount << " paid successfully" << endl;
        return true;
    }
};
