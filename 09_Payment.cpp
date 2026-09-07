#pragma once

class Payment {
public:
    virtual bool pay(double amount) = 0;
    virtual ~Payment() {}
};
