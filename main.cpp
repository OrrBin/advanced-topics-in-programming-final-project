#include <iostream>
#include "Tests.h"
#include "Ship.h"  // add '#pragma once' to Ship.h, or remove this include

using namespace shipping;
using namespace std;

void separator() {
    cout << endl;
    cout << "--------------------------------------------------------------------";
    cout << endl << endl;
}

void testsStartMsg(string name) {
    cout << "---- Starting " << name << " Tests ----" << endl;
}

int main() {
    testsStartMsg("My");
    executeTests();
    separator();

    testsStartMsg("Amir");
    copy_move_tests();  // tests for copy/move ctr/assignment from forum
    separator();

    testsStartMsg("Ziv");
    zivTests();
    separator();

    testsStartMsg("Noa");
    noaTests();
    separator();

    cout << "Finished tests" << endl;
}

// region Amir Examples

void ex1() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{2}, Y{6}, Height{0}),
            tuple(X{2}, Y{7}, Height{1}),
            tuple(X{2}, Y{5}, Height{6}),
    };


    // create bad ship 1
    try {
        restrictions.push_back(tuple(X{2}, Y{5}, Height{6}));
        Ship<string> myShip{X{4}, Y{12}, Height{16}, restrictions};
    } catch (BadShipOperationException &e) {
        // exception: duplicate restrictions (whether or not it has same limit):
        // restriction with X{2}, Y{5} appears more than once (added in the try)
        restrictions.pop_back(); // remove the duplicate restriction
    }


    // create bad ship 2
    try {
        Ship<string> myShip{X{4}, Y{7}, Height{8}, restrictions};
    } catch (BadShipOperationException &e) {
        // exception due to bad restrictions:
        // restriction with Y=7, when the size of Y is 7
    }

    // create bad ship 3
    try {
        Ship<string> myShip{X{4}, Y{12}, Height{6}, restrictions};
    } catch (BadShipOperationException &e) {
        // exception due to bad restrictions:
        // restriction with height=6, when original height is equal or smaller
    }

    // create good ship
    Ship<string> myShip{X{4}, Y{8}, Height{8}, restrictions};
    // bad load - no room
    try {
        myShip.load(X{2}, Y{6}, "Hello");
    } catch (BadShipOperationException &e) { /* no room at this location */ }
    // good load
    myShip.load(X{2}, Y{7}, "Hello");

    // bad load - no room
    try {
        myShip.load(X{2}, Y{7}, "Hello");
    } catch (BadShipOperationException &e) { /* no room at this location */ }

    // bad unload - no container at location
    try {
        string container = myShip.unload(X{1}, Y{1});
    } catch (BadShipOperationException &e) { /* no container at this location */ }

    // bad load - wrong index
    try {
        myShip.load(X{1}, Y{8}, "Hi");
    } catch (BadShipOperationException &e) { /* bad index Y {8} */ }
}

void ex2() {
    Grouping<string> groupingFunctions = {
            {"first_letter",
                    [](const string &s) { return string(1, s[0]); }
            },
            {"first_letter_toupper",
                    [](const string &s) { return string(1, char(toupper(s[0]))); }
            }
    };

    vector<tuple<X, Y, Height>> restrictions = {
            tuple<X,Y,Height>(X{2}, Y{6}, Height{4}),
            tuple<X,Y,Height>(X{2}, Y{7}, Height{6}),
    };

    Ship<string> myShip{X{5}, Y{12}, Height{8},
                        restrictions,
                        groupingFunctions};
    // load “containers”
    myShip.load(X{0}, Y{0}, "Hello");
    myShip.load(X{1}, Y{1}, "hey");
    myShip.load(X{1}, Y{1}, "bye");

    auto view00 = myShip.getContainersViewByPosition(X{0}, Y{0});
    auto view_h = myShip.getContainersViewByGroup("first_letter", "h");
    auto view_Hh = myShip.getContainersViewByGroup("first_letter_toupper", "H");
    myShip.load(X{0}, Y{0}, "hi");

    // loop on all “containers”: Hello, hi, hey, bye - in some undefined order
    for (const auto &container : myShip) {
        cout << container << endl;
    }

    // loop on view00: hi, Hello - in this exact order
    for (const auto &container : view00) {
        cout << container << endl;
    }

    // loop on view_h: pair { tuple{X{0}, Y{0}, Height{1}}, hi },
    // pair { tuple{X{1}, Y{1}, Height{0}}, hey }
    // - in some undefined order
    for (auto &&container_tuple : view_h) {
        cout << container_tuple << endl;
    }

    // loop on view_Hh: pair { tuple{X{0}, Y{0}, Height{0}}, Hello },
    // pair { tuple{X{0}, Y{0}, Height{1}}, hi },
    // pair { tuple{X{1}, Y{1}, Height{0}}, hey }
    // - in some undefined order
    for (auto &&container_tuple : view_Hh) {
        cout << container_tuple << endl;
    }
}

// endregion