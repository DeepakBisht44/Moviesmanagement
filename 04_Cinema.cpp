#pragma once
#include <string>
#include <vector>
#include "03_Screen.cpp"
using namespace std;

class Cinema {
private:
    string name;
    vector<Screen> screens;

public:
    Cinema(string name) {
        this->name = name;
    }

    void addScreen(Screen screen) {
        screens.push_back(screen);
    }

    vector<Screen>& getScreens() { return screens; }
    string getName() const { return name; }
};
