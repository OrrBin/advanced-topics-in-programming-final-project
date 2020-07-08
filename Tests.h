#include <cassert>
#include <ostream>
#include "Ship.h"

using namespace shipping;
using namespace std;

// region Assertions

/// Assert 'command' throws BadShipOperationException
#define AssertException(command, message) \
try { \
    command; \
} \
catch (BadShipOperationException &e) { \
    exceptionThrown = true; \
} \
if (!exceptionThrown) { \
    cout << "Expected to throw BadShipOperationException on line " << __LINE__ << ": " << message << endl; \
    exit(1); \
} \
exceptionThrown = false;
bool exceptionThrown = false;

/// Assert condition is true
#define AssertCondition(condition, message) \
if (!(condition)) { \
    cout << "Assertion failed on line " << __LINE__ << ": " << message << endl; \
    exit(1); \
}

/// Assert actual == expected
#define AssertEquals(actual, expected) \
if ((actual != expected)) { \
    cout << "Assertion failed on line " << __LINE__ << ": " << "expected: " << expected << ", actual: " << actual << endl; \
    exit(1); \
}

template<typename Container>
void AssertContainer(pair<tuple<X, Y, Height>, Container> &pair, int x, int y, int z, Container &container, int line) {
    if (!posEquals(pair.first, {X(x), Y(y), Height{z}})) {
        cerr << "Assertion failed on line " << line << ": " << "Ship position of element is invalid" << endl;
        exit(1); \
    }

    if (pair.second != 45) {
        cerr << "Assertion failed on line " << line << ": " << "Value of element is invalid" << endl;
        exit(1);
    }
}

// endregion

// region Printers, Help Functions

template<typename Container>
using ViewPair = vector<pair<tuple<X, Y, Height>, Container>>;

std::ostream &operator<<(std::ostream &os, const std::tuple<X, Y, Height> &position) {
    os << "(" << std::get<0>(position) << "," << std::get<1>(position) << "|" << std::get<2>(position) << ")";
    return os;
}

template<typename Container>
std::ostream &operator<<(std::ostream &os, const std::pair<const tuple<X, Y, Height>, const Container &> pair) {
    os << "Pos: " << pair.first << " | " << "Container: " << pair.second;
    return os;
}

bool operator==(const tuple<X, Y, Height> &pos1, const tuple<X, Y, Height> &pos2) {
    return get<0>(pos1) == get<0>(pos2) && get<1>(pos1) == get<1>(pos2) && get<2>(pos1) == get<2>(pos2);
}

bool posEquals(const tuple<X, Y, Height> &pos1, const tuple<X, Y, Height> &pos2) {
    return get<0>(pos1) == get<0>(pos2) && get<1>(pos1) == get<1>(pos2) && get<2>(pos1) == get<2>(pos2);
}

template<typename Container>
void sortPairs(vector<pair<tuple<X, Y, Height>, Container>> &pairs) {
    sort(pairs.begin(), pairs.end(), [](const auto& pair1, const auto& pair2) {
        return pair1.second < pair2.second;
    });
}

// endregion

// region My Tests

inline void testShipOps() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{1}, Y{0}, Height{1}),
            tuple(X{1}, Y{1}, Height{0}),
    };

    Ship<int> ship{X{2}, Y{3}, Height{3}, restrictions};

    ship.load(X{0}, Y{0}, 1);
    ship.load(X{0}, Y{0}, 2);
    ship.load(X{0}, Y{0}, 3);

    AssertException(ship.load(X{0}, Y{0}, 4), "load to (0,0) where there is no space left")  // no space
    AssertException(ship.load(X{0}, Y{0}, 5), "load to (0,0) where there is no space left")  // no space

    AssertException(ship.load(X{1}, Y{1}, 1), "load to (1,1), when position is restricted to no containers")  // restricted position

    AssertException(ship.load(X{2}, Y{0}, 5), "load to invalid position (2,0), x out of range")
    AssertException(ship.load(X{-1}, Y{0}, 5), "load to invalid position (-1,0), x is negative")
    AssertException(ship.load(X{1}, Y{3}, 5), "load to invalid position (1,3), y out of range")
    AssertException(ship.load(X{10}, Y{5}, 5), "load to invalid position (10,5), x and y out of range")

    ship.load(X{1}, Y{0}, 23);  // last slot in (1,0)
    AssertException(ship.load(X{1}, Y{0}, 25), "load to (0,0) where there is no space left")  // no more space

    AssertException(ship.move(X{0}, Y{0}, X{1}, Y{0}), "moved from (0,0) to (1,1), but no space in (1,1)")

    ship.move(X{0}, Y{0}, X{0}, Y{0});  // move to same place - no effect
    int x = ship.unload(X{0}, Y{0});
    AssertCondition(x == 3, "unloaded from (0,0), expected to see 3 but seen " + to_string(x))
}

inline void testShipIterator() {
    Ship<string> myShip{X{2}, Y{2}, Height{3},};

    vector<string> values = {"2", "3", "4", "7", "8", "9"};

    int i = 0;
    myShip.load(X{0}, Y{0}, values[i++]);
    myShip.load(X{0}, Y{0}, values[i++]);
    myShip.load(X{0}, Y{1}, values[i++]);
    myShip.load(X{1}, Y{1}, values[i++]);
    myShip.load(X{1}, Y{1}, values[i++]);
    myShip.load(X{1}, Y{1}, values[i++]);

    vector<string> res;

    for (const string &x : myShip) {
        res.push_back(x);
    }

    sort(res.begin(), res.end());
    AssertCondition(res.size() == values.size(), "expected to iterate over 6 values, but got " + to_string(res.size()))
    for (i = 0; i < values.size(); i++) {
        AssertCondition(values[i] == res[i], "expected to see " + values[i] + ", but seen " + res[i])
    }
}

inline void testShipViewByPosition() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{1}, Y{0}, Height{1}),
            tuple(X{2}, Y{2}, Height{0}),
    };

    Ship<string> myShip{X{5}, Y{5}, Height{4},};

    myShip.load(X{1}, Y{1}, "11");
    myShip.load(X{1}, Y{1}, "22");

    auto view = myShip.getContainersViewByPosition(X{1}, Y{1});

    myShip.load(X{1}, Y{1}, "33");
    myShip.load(X{1}, Y{1}, "44");
    myShip.unload(X{1}, Y{1});  // remove 44

    vector<string> res;
    for (auto &x: view) {
        res.push_back(x);
    }
    sort(res.begin(), res.end());
    AssertCondition(res.size() == 3, "expected view on (1,1) to produce 3 items")
    AssertCondition(res[0] == "11", "expected to see 11, but seen " + res[0])
    AssertCondition(res[1] == "22", "expected to see 22, but seen " + res[1]);
    AssertCondition(res[2] == "33", "expected to see 33, but seen " + res[2])
}

inline void testShipEmptyViewByPosition() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{1}, Y{0}, Height{1}),
            tuple(X{2}, Y{3}, Height{0}),
    };

    Ship<string> myShip{X{5}, Y{5}, Height{4}, restrictions};

    myShip.load(X{0}, Y{0}, "1");
    myShip.load(X{0}, Y{0}, "1");
    myShip.load(X{0}, Y{0}, "1");
    myShip.load(X{0}, Y{0}, "1");
    myShip.load(X{0}, Y{1}, "1");
    myShip.load(X{0}, Y{1}, "1");
    myShip.load(X{1}, Y{0}, "1");
    myShip.load(X{2}, Y{2}, "1");

    auto view = myShip.getContainersViewByPosition(X{-1}, Y{-2});

    for (auto &x: view) {
        AssertCondition(false, "getContainersViewByPosition on (-1,-2), expected iterator to be empty (out of range)")
    }

    auto view2 = myShip.getContainersViewByPosition(X{2}, Y{2});

    myShip.unload(X{2}, Y{2});  // unload the single container in (2,2), now its empty
    for (auto &x: view2) {
        AssertCondition(false, "getContainersViewByPosition on (2,2), expected iterator to be empty (no containers in position)")
    }

    auto view3 = myShip.getContainersViewByPosition(X{5}, Y{2});

    for (auto &x: view3) {
        AssertCondition(false, "getContainersViewByPosition on (5,2), expected iterator to be empty (out of range)")
    }

    auto view4 = myShip.getContainersViewByPosition(X{4}, Y{2});

    for (auto &x: view4) {
        AssertCondition(false, "getContainersViewByPosition on (4,2), expected iterator to be empty (no containers loaded)")
    }

    auto view5 = myShip.getContainersViewByPosition(X{2}, Y{3});

    for (auto &x: view5) {
        AssertCondition(false, "getContainersViewByPosition on (2,3), expected iterator to be empty (no containers loaded)")
    }
}

class A {
public:
    std::string x = "?";
    int y = 0;

    A() {}

    A(const std::string &x) : x(x) {}

    A(int y) : y(y) {}

    friend ostream &operator<<(ostream &os, const A &a) {
        if (a.y != 0)
            os << a.y;
        if (a.x != "?")
            os << a.x;
        return os;
    }

    bool operator<(const A &other) const {
        if (y == other.y) {
            return x < other.x;
        }
        return y < other.y;
    }

    bool operator==(const A &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const A &rhs) const {
        return !(rhs == *this);
    }
};

inline void testShipViewByGroup() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{0}, Y{0}, Height{0}),
            tuple(X{4}, Y{4}, Height{3}),
    };

    Grouping<A> groupingFunctions = {
            {"starts_with",
                    [](const A &a) { return a.x.substr(0, 1); /* first letter */ }
            },

            {"modulo_10",
                    [](const A &a) { return std::to_string(a.y % 10); }
            }


    };

    Ship<A> myShip{X{5}, Y{5}, Height{4}, restrictions, groupingFunctions};

    myShip.load(X{4}, Y{4}, A(23));
    myShip.load(X{4}, Y{4}, A(42));
    myShip.load(X{1}, Y{1}, A(92));
    myShip.load(X{1}, Y{1}, A(14));

    auto view = myShip.getContainersViewByGroup("modulo_10", "2");

    myShip.load(X{1}, Y{1}, A(4132));
    myShip.unload(X{4}, Y{4});

    vector<pair<tuple<X, Y, Height>, A>> pairs;

    // expecting 92, 4132
    for (auto &pair: view) {
        pairs.push_back(pair);
//        cout << pair << endl;
    }
    sortPairs(pairs);
    AssertEquals(pairs.size(), 2)
    AssertCondition((posEquals(pairs[0].first, {X(1), Y(1), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[0].second.y, 92)
    AssertCondition((posEquals(pairs[1].first, {X(1), Y(1), Height{2}})), "Position of element is invalid")
    AssertEquals(pairs[1].second.y, 4132)

    auto view2 = myShip.getContainersViewByGroup("starts_with", "?");

    // expecting all: 23, 92, 14, 4132
    pairs.clear();
    for (auto &pair: view2) {
        pairs.push_back(pair);
//        cout << pair << endl;
    }
    sortPairs(pairs);
    AssertCondition(pairs.size() == 4, "expected 4 values in iterator, but got " + to_string(pairs.size()))
    pairs.clear();
}

inline void testShipViewByGroup2() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{0}, Y{0}, Height{0}),
            tuple(X{4}, Y{4}, Height{3}),
    };

    Grouping<A> groupingFunctions = {
            {"starts_with",
                    [](const A &a) { return a.x.substr(0, 1); /* first letter */ }
            },

            {"modulo_10",
                    [](const A &a) { return std::to_string(a.y % 10); }
            }


    };

    Ship<A> myShip{X{5}, Y{5}, Height{4}, restrictions, groupingFunctions};

    myShip.load(X{4}, Y{4}, A(23));
    myShip.load(X{4}, Y{4}, A(42));
    myShip.load(X{1}, Y{1}, A(92));
    myShip.load(X{1}, Y{1}, A(14));

    auto view3 = myShip.getContainersViewByGroup("modulo_10", "5");
    myShip.load(X{2}, Y{2}, A(45));  // group "5" was empty, this should be added

    vector<pair<tuple<X, Y, Height>, A>> pairs;
    for (auto &pair: view3) {
        pairs.push_back(pair);
    }
    AssertCondition(pairs.size() == 1, "expected 1 value in iterator, but got " + to_string(pairs.size()))
    AssertCondition((posEquals(pairs[0].first, {X(2), Y(2), Height{0}})), "Position of element is invalid")
    AssertCondition((pairs[0].second.y == 45), "Value of element is invalid")
}

inline void testShipViewByGroup3() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{0}, Y{0}, Height{0}),
            tuple(X{4}, Y{4}, Height{3}),
    };

    Grouping<A> groupingFunctions = {
            {"starts_with",
                    [](const A &a) { return a.x.substr(0, 1); /* first letter */ }
            },

            {"modulo_10",
                    [](const A &a) { return std::to_string(a.y % 10); }
            }


    };

    Ship<A> myShip{X{5}, Y{5}, Height{4}, restrictions, groupingFunctions};

    auto emptyView = myShip.getContainersViewByGroup("starts_with", "ab");
    auto emptyView2 = myShip.getContainersViewByGroup("invalid_group", "a");
    auto view3 = myShip.getContainersViewByGroup("starts_with", "c");

    myShip.load(X{1}, Y{2}, A("b1"));
    myShip.load(X{1}, Y{2}, A("c1"));
    myShip.load(X{1}, Y{2}, A("c2"));
    myShip.load(X{3}, Y{4}, A("c3"));
    myShip.load(X{4}, Y{2}, A("b3"));
    myShip.load(X{2}, Y{1}, A("c4"));
    myShip.move(X{1}, Y{2}, X{0}, Y{2});
    myShip.unload(X{1}, Y{2});
    myShip.move(X{2}, Y{1}, X{4}, Y{2});

    for (auto &x: emptyView) {
        AssertCondition(false, "getContainersViewByGroup on non-existing group, expected iterator to be empty")
    }

    for (auto &x: emptyView2) {
        AssertCondition(false, "getContainersViewByGroup on non-existing grouping, expected iterator to be empty")
    }

    ViewPair<A> pairs;
    for (auto &pair: view3) {
        pairs.push_back(pair);
    }
    sortPairs(pairs);
    AssertEquals(pairs.size(), 3)
    AssertCondition(pairs.size() == 3, "expected 3 values in iterator, but got " + to_string(pairs.size()))
    AssertCondition((posEquals(pairs[0].first, {X(0), Y(2), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[0].second.x, "c2")
    AssertCondition((posEquals(pairs[1].first, {X(3), Y(4), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[1].second.x, "c3")
    AssertCondition((posEquals(pairs[2].first, {X(4), Y(2), Height{1}})), "Position of element is invalid")
    AssertEquals(pairs[2].second.x, "c4")
    pairs.clear();
}

inline void testShipViewByPositionTwoShips() {
    Ship<string> myShip{X{5}, Y{5}, Height{4}};
    Ship<string> myShip2{X{5}, Y{5}, Height{4}};

    myShip.load(X{1}, Y{1}, "11");
    myShip.load(X{1}, Y{1}, "22");

    myShip2.load(X{1}, Y{1}, "22");
    myShip2.load(X{1}, Y{1}, "101");

    auto view11Ship1 = myShip.getContainersViewByPosition(X{1}, Y{1});
    auto view11Ship2 = myShip2.getContainersViewByPosition(X{1}, Y{1});

    auto itr1 = view11Ship1.begin();
    auto end1 = view11Ship1.end();

    auto itr2 = view11Ship2.begin();
    auto end2 = view11Ship2.end();

    for( ; itr2 != end2 && (*itr2) != "101"; ++itr2);
    for( ; itr1 != end1 && (*itr1) != "101"; ++itr1);
    AssertCondition(itr2 != end2, "expected not to get to the end since 101 is on ship")
    AssertCondition(itr1 != itr2, "expected to be different iterators")
    AssertCondition(!(itr1 != end1), "expected to get to the end since there is no 101 in ship")

    for( ; itr2 != end2 && (*itr2) != "22"; ++itr2);
    AssertCondition(itr2 != end2, "expected not to get to the end since 22 is on ship and after 101 on iterator")
    for( ; itr2 != end2 && (*itr2) != "202"; ++itr2);
    AssertCondition(!(itr2 != end2), "expected to get to the end since there is no 202 in ship")
    AssertCondition(itr2 != itr1, "expected to be false since these are two different iterators for two different ships")
}

inline void testShipViewByGroup4() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{0}, Y{0}, Height{1}),
            tuple(X{4}, Y{4}, Height{3}),
    };

    Grouping<A> groupingFunctions = {
            {"modulo_10",
                    [](const A &a) { return std::to_string(a.y % 10); }
            }
    };

    Ship<A> myShip{X{5}, Y{5}, Height{4}, restrictions, groupingFunctions};

    myShip.load(X{4}, Y{4}, A(25)); // will be placed in height 0
    myShip.load(X{4}, Y{4}, A(42)); // will be placed in height 1
    myShip.load(X{0}, Y{0}, A(95)); // will be placed in height 0


    auto view3 = myShip.getContainersViewByGroup("modulo_10", "5");
    myShip.load(X{4}, Y{4}, A(45));  // group "5" contained 25 only, this should be added - will be placed in height 2

    vector<pair<tuple<X, Y, Height>, A>> pairs;
    for (auto &pair: view3) {
        pairs.push_back(pair);
    }
    sortPairs(pairs);
    AssertCondition(pairs.size() == 3, "expected 3 value in iterator, but got " + to_string(pairs.size()))
    AssertCondition((posEquals(pairs[0].first, {X(4), Y(4), Height{0}})), "Position of element is invalid")
    AssertCondition((pairs[0].second.y == 25), "Value of element is invalid")
    AssertCondition((posEquals(pairs[1].first, {X(4), Y(4), Height{2}})), "Position of element is invalid")
    AssertCondition((pairs[1].second.y == 45), "Value of element is invalid")
    AssertCondition((posEquals(pairs[2].first, {X(0), Y(0), Height{0}})), "Position of element is invalid")
    AssertCondition((pairs[2].second.y == 95), "Value of element is invalid")
}

inline void testLoadWhenThereIsNoPlace() {
    vector<tuple<X, Y, Height>> restrictions = {
            tuple(X{0}, Y{0}, Height{0}),
            tuple(X{4}, Y{4}, Height{3}),
    };

    Ship<A> myShip{X{5}, Y{5}, Height{4}, restrictions};
    AssertException(myShip.load(X{0}, Y{0}, 4), "load to (0,0) where there is no space")
    myShip.load(X{4}, Y{4}, 4);
    myShip.load(X{4}, Y{4}, 1);
    myShip.load(X{4}, Y{4}, 2);
    AssertException(myShip.load(X{4}, Y{4}, 3), "load to (0,0) where there is no space left")
}

#define testPassed(name) cout << name << " passed" << endl;

inline void executeTests() {
    testShipOps();
    testPassed("testShipOps")

    testShipIterator();
    testPassed("testShipIterator")

    testShipViewByPosition();
    testPassed("testShipViewByPosition")

    testShipEmptyViewByPosition();
    testPassed("testShipEmptyViewByPosition")

    testShipViewByGroup();
    testPassed("testShipViewByGroup")

    testShipViewByGroup2();
    testPassed("testShipViewByGroup2")

    testShipViewByGroup3();
    testPassed("testShipViewByGroup3")

    testShipViewByPositionTwoShips();
    testPassed("testShipViewByPositionTwoShips")

    testShipViewByGroup4();
    testPassed("testShipViewByGroup4")

    testLoadWhenThereIsNoPlace();
    testPassed("testLoadWhenThereIsNoPlace")
}

// endregion

// region Amir Move/Copy Tests

template<typename Ship>
void test_move_assignment_operator() {
    // create simple Ship with no groupings
    Ship examHall{ X{4}, Y{8}, Height{1} };

    // inner scope
    {
        // create Ship2
        Grouping<std::string> groupingFunctions = {
                { "first_letter",
                        [](const std::string& s){ return std::string(1, s[0]); }
                }
        };
        Ship examHall2{ X{5}, Y{12}, Height{1}, {}, groupingFunctions };
        examHall2.load(X{1}, Y{2}, "galit");
        examHall2.unload(X{1}, Y{2}); // unsit galit
        examHall2.load(X{1}, Y{2}, "goni");
        // move assignment - do not use examHall2 after next line:
        examHall = std::move(examHall2);
        examHall.unload(X{1}, Y{2}); // unsit goni
        examHall.load(X{1}, Y{2}, "gideon");
    }

    auto view_g = examHall.getContainersViewByGroup("first_letter", "g");

    auto& from_hall = *examHall.begin();
    auto& from_view = view_g.begin()->second;
    if(from_hall != from_view) {
        std::cout << "move assignment test failed: view returned '" << from_view << "' expecting: '" << from_hall << "'" << std::endl;
    }
    else if(&from_hall != &from_view) {
        std::cout << "move assignment test failed: address from view: " << &from_view << " expecting: " << &from_hall << "'" << std::endl;
    }
}

template<typename Ship>
void test_assignment_operator() {
    // create simple Ship with no groupings
    Ship examHall{ X{4}, Y{8}, Height{1} };

    // inner scope
    {
        // create Ship2
        Grouping<std::string> groupingFunctions = {
                { "first_letter",
                        [](const std::string& s){ return std::string(1, s[0]); }
                }
        };
        Ship examHall2{ X{5}, Y{12}, Height{1}, {}, groupingFunctions };
        examHall2.load(X{1}, Y{2}, "galit");
        // assignment:
        examHall = examHall2;
        examHall2.unload(X{1}, Y{2}); // unsit galit
        examHall2.load(X{1}, Y{2}, "goni");
    }

    auto view_g = examHall.getContainersViewByGroup("first_letter", "g");

    auto& from_hall = *examHall.begin();
    auto& from_view = view_g.begin()->second;
    if(from_hall != from_view) {
        std::cout << "assignment test failed: view returned '" << from_view << "' expecting: '" << from_hall << "'" << std::endl;
    }
    else if(&from_hall != &from_view) {
        std::cout << "assignment test failed: address from view: " << &from_view << " expecting: " << &from_hall << "'" << std::endl;
    }
}

template<typename Ship>
void test_copy_constructor() {
    // create simple Ship with no groupings
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            }
    };
    Ship examHall{ X{5}, Y{12}, Height{1}, {}, groupingFunctions };
    examHall.load(X{1}, Y{2}, "galit");

    // create Ship2 with copy ctor
    Ship examHall2 (examHall);

    examHall.unload(X{1}, Y{2}); // unsit galit
    examHall.load(X{1}, Y{2}, "goni");

    auto view_g = examHall2.getContainersViewByGroup("first_letter", "g");

    auto& from_hall = *examHall2.begin();
    auto& from_view = view_g.begin()->second;
    if(from_hall != from_view) {
        std::cout << "copy ctor test failed: view returned '" << from_view << "' expecting: '" << from_hall << "'" << std::endl;
    }
    else if(&from_hall != &from_view) {
        std::cout << "copy ctor test failed: address from view: " << &from_view << " expecting: " << &from_hall << "'" << std::endl;
    }
}

template<typename Ship>
void test_move_constructor() {
    // create simple Ship with no groupings
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            }
    };
    Ship examHall{ X{5}, Y{12}, Height{1}, {}, groupingFunctions };
    examHall.load(X{1}, Y{2}, "galit");
    examHall.unload(X{1}, Y{2}); // unsit galit
    examHall.load(X{1}, Y{2}, "goni");

    // create Ship2 with move ctor - do not use examHall after next line:
    Ship examHall2 (std::move(examHall));
    examHall2.unload(X{1}, Y{2}); // unsit goni
    examHall2.load(X{1}, Y{2}, "gideon");

    auto view_g = examHall2.getContainersViewByGroup("first_letter", "g");

    auto& from_hall = *examHall2.begin();
    auto& from_view = view_g.begin()->second;
    if(from_hall != from_view) {
        std::cout << "move ctor test failed: view returned '" << from_view << "' expecting: '" << from_hall << "'" << std::endl;
    }
    else if(&from_hall != &from_view) {
        std::cout << "move ctor test failed: address from view: " << &from_view << " expecting: " << &from_hall << "'" << std::endl;
    }
}

void copy_move_tests() {
    if constexpr(!std::is_copy_constructible_v<Ship<std::string>>) {
        std::cout << "copy ctor is blocked - check if there is a reason for that" << std::endl;
    }
    else {
        test_copy_constructor<Ship<std::string>>();
        std::cout << "finished test_copy_constructor" << std::endl;
    }
    if constexpr(!std::is_assignable_v<Ship<std::string>&, Ship<std::string>&>) {
        std::cout << "assignment operator is blocked - check if there is a reason for that" << std::endl;
    }
    else {
        test_assignment_operator<Ship<std::string>>();
        std::cout << "finished test_assignment_operator" << std::endl;
    }
    if constexpr(!std::is_move_constructible_v<Ship<std::string>>) {
        std::cout << "move ctor is blocked - check if there is a reason for that" << std::endl;
    }
    else {
        test_move_constructor<Ship<std::string>>();
        std::cout << "finished test_move_constructor" << std::endl;
    }
    if constexpr(!std::is_move_assignable_v<Ship<std::string>>) {
        std::cout << "move assignment operator is blocked - check if there is a reason for that" << std::endl;
    }
    else {
        test_move_assignment_operator<Ship<std::string>>();
        std::cout << "finished test_move_assignment_operator" << std::endl;
    }
}

// endregion

// region Ziv Tests

int test1(){
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X,Y,Height>(X{2}, Y{6}, Height{0}),
            std::tuple<X,Y,Height>(X{2}, Y{7}, Height{1}),
            std::tuple<X,Y,Height>(X{2}, Y{5}, Height{6}),
    };
    try {
        restrictions.push_back( std::tuple<X,Y,Height>(X{2}, Y{5}, Height{6}) );
        Ship<std::string> myShip{ X{4}, Y{12}, Height{16}, restrictions };
    } catch(BadShipOperationException& e) {
        restrictions.pop_back(); // remove the duplicate restriction
        exceptionThrown = true;
    }

    try {
        Ship<std::string> myShip{ X{4}, Y{7}, Height{8}, restrictions };
    } catch(BadShipOperationException& e) {  // restrictions contains bad y value
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    try {
        Ship<std::string> myShip{ X{4}, Y{12}, Height{6}, restrictions };
    } catch(BadShipOperationException& e) {  // restrictions contains bad height value. original height is equal or smaller.
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    Ship<std::string> myShip{ X{4}, Y{8}, Height{8}, restrictions };
    try {
        myShip.load(X{2}, Y{6}, "Hello");
    } catch(BadShipOperationException& e) {
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    myShip.load(X{2}, Y{7}, "Hello");
    try {
        myShip.load(X{2}, Y{7}, "Hello");
    } catch(BadShipOperationException& e) {  // Position is full
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    try {
        std::string container = myShip.unload(X{1}, Y{1});
    } catch(BadShipOperationException& e) {  // Position is empty
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    try {
        myShip.load(X{1}, Y{8}, "Hi");
    } catch(BadShipOperationException& e) {  // Position out of range
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    std::string container = myShip.unload(X{2} ,Y{7});
    if (container!="Hello")
        return 1;
    return 0;
}

int test2(){
    Ship<int> myShip{ X{2}, Y{2}, Height{2}};
    try {
        myShip.move(X{0}, Y{0}, X{0}, Y{0});
    }catch (BadShipOperationException& e){  // Position is empty
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    myShip.load(X{0}, Y{0} , 5);
    myShip.move(X{0},Y{0}, X{1}, Y{1});
    try {
        myShip.unload(X{0}, Y{0});
    }catch (BadShipOperationException& e){  // Position is empty
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    myShip.load(X{0},Y{0},6);
    myShip.load(X{0},Y{0},7);
    try {
        myShip.move(X{1}, Y{1}, X{0}, Y{0});
    }catch (BadShipOperationException& e){  // Position is full
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    if(myShip.unload(X{1}, Y{1}) != 5)
        return 1;
    myShip.move(X{0}, Y{0}, X{0}, Y{0});
    if(myShip.unload(X{0}, Y{0}) != 7)
        return 1;
    if(myShip.unload(X{0}, Y{0}) != 6)
        return 1;
    return 0;

}

int test3(){
    Ship<int> myShip{ X{2}, Y{2}, Height{3}};
    myShip.load(X{0}, Y{0}, 1);
    myShip.load(X{0}, Y{0}, 2);
    myShip.load(X{0}, Y{0}, 3);
    myShip.load(X{1}, Y{0}, 2);
    myShip.load(X{1}, Y{0}, 3);
    myShip.load(X{0}, Y{1}, 2);
    int i=3;
    auto itr = myShip.getContainersViewByPosition(X{0}, Y{0});
    for(auto& container:itr){
        if(container != i)
            return 1;
        i--;
    }
    i=3;
    auto itr2 = myShip.getContainersViewByPosition(X{1}, Y{0});
    for(auto& container:itr2){
        if(container != i)
            return 1;
        i--;
    }
    auto itr3 = myShip.getContainersViewByPosition(X{0}, Y{1});
    for(auto& container:itr3){
        if(container != 2)
            return 1;
    }
    auto itr4 = myShip.getContainersViewByPosition(X{1}, Y{1});
    for(auto& container:itr4){
        (void)container;
        //empty view
        return 1;
    }
    auto itr5 = myShip.getContainersViewByPosition(X{0}, Y{0}).begin();
    ++itr5;
    if(*(itr5) != 2)
        return 1;
    myShip.unload(X{0}, Y{0});
    myShip.unload(X{0},Y{0});
    myShip.load(X{0},Y{0}, 17);
    if(*(itr5) != 17)
        return 1;
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X,Y,Height>(X{0}, Y{0}, Height{3}),
            std::tuple<X,Y,Height>(X{0}, Y{1}, Height{3})
    };
    Ship<int> myShip2{ X{2}, Y{2}, Height{5}, restrictions};
    myShip2.load(X{0}, Y{0}, 1);
    myShip2.load(X{0}, Y{0}, 2);
    i=2;
    auto itr6 = myShip2.getContainersViewByPosition(X{0}, Y{0});
    for(auto& container:itr6){
        if(container != i)
            return 1;
        i--;
    }
    auto itr7 = myShip2.getContainersViewByPosition(X{0}, Y{1});
    for(auto& container:itr7){
        (void)container;
        return 1;
    }
    auto itr8 = myShip2.getContainersViewByPosition(X{80}, Y{1});
    for(auto elem : itr8){
        (void)elem;
        return 1;
    }

    return 0;
}

int test4(){
    Ship<int> myShip{ X{2}, Y{2}, Height{3}};
    myShip.load(X{0}, Y{0}, 1);
    myShip.load(X{0}, Y{0}, 2);
    myShip.load(X{0}, Y{0}, 3);
    myShip.load(X{1}, Y{0}, 4);
    myShip.load(X{1}, Y{0}, 5);
    myShip.load(X{0}, Y{1}, 6);
    myShip.load(X{0}, Y{1}, 7);
    myShip.load(X{1}, Y{1}, 8);
    int i=0;
    for(auto& elem:myShip){
        (void)elem;
        i++;
    }
    if(i!=8)
        return 1;
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X,Y,Height>(X{0}, Y{0}, Height{3}),
            std::tuple<X,Y,Height>(X{0}, Y{1}, Height{3})
    };
    Ship<int> myShip2{ X{2}, Y{2}, Height{5},restrictions};
    myShip2.load(X{0}, Y{0}, 6);
    myShip2.load(X{0}, Y{0}, 7);
    myShip2.load(X{1}, Y{1}, 8);
    myShip2.load(X{0}, Y{1}, 9);
    i=0;
    for(auto& elem:myShip2){
        (void)elem;
        i++;
    }
    if(i!=4)
        return 1;
    return 0;
}

int test5(){
    Ship<int> myShip{ X{2}, Y{2}, Height{3}};
    myShip.load(X{0}, Y{0}, 1);
    myShip.load(X{0}, Y{0}, 2);
    myShip.load(X{0}, Y{0}, 3);
    myShip.load(X{1}, Y{0}, 4);
    myShip.load(X{1}, Y{0}, 5);
    myShip.load(X{0}, Y{1}, 6);
    myShip.load(X{0}, Y{1}, 7);
    myShip.load(X{1}, Y{1}, 8);
    auto itr = myShip.begin();
    myShip.load(X{1}, Y{1}, 15);
    for(; itr!=myShip.end(); ++itr){
        if(*(itr) == 15) {
            return 0;
        }
    }
    return 1;
}

int test6(){
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            },
            { "first_letter_toupper",
                    [](const std::string& s){ return std::string(1, char(std::toupper(s[0]))); }
            }
    };
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X,Y,Height>(X{2}, Y{6}, Height{4}),
            std::tuple<X,Y,Height>(X{2}, Y{7}, Height{6}),
    };
    Ship<std::string> myShip{ X{5}, Y{12}, Height{8},
                              restrictions,
                              groupingFunctions };

    myShip.load(X{0}, Y{0}, "Hello");
    myShip.load(X{1}, Y{1}, "hey");
    myShip.load(X{1}, Y{1}, "bye");
    auto view00 = myShip.getContainersViewByPosition(X{0}, Y{0});
    auto view_h = myShip.getContainersViewByGroup("first_letter", "h");
    auto view_Hh = myShip.getContainersViewByGroup("first_letter_toupper", "H");
    myShip.load(X{0}, Y{0}, "hi");
    int i=0;
    for(const auto& container : myShip) {
        i++;
        (void)container;
    }
    if(i!=4)
        return 1;
    i=1;
    for(const auto& container : view00) {
        if(i==1){
            if(container!="hi")
                return 1;
        }
        else{
            if(container!="Hello")
                return 1;
        }
        i++;
    }

    // loop on view_h:	pair { tuple{X{0}, Y{0}, Height{1}}, hi },
    // 						pair { tuple{X{1}, Y{1}, Height{0}}, hey }
    // - in some undefined order
    i=0;
    for(const auto& container_tuple : view_h) {
        i++;
        (void)container_tuple;
    }
    if(i!=2)
        return 1;

    i=0;
    for(const auto& container_tuple : view_Hh) {
        i++;
        (void)container_tuple;
    }
    if(i!=3)
        return 1;
    return 0;
}

int test7(){
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X,Y,Height>(X{2}, Y{6}, Height{4}),
            std::tuple<X,Y,Height>(X{2}, Y{7}, Height{6}),
    };
    Ship<int> myShip{ X{2}, Y{2}, Height{3}};
    myShip.load(X{0}, Y{0}, 1);
    myShip.load(X{1}, Y{1}, 2);
    myShip.load(X{1}, Y{1}, 3);
    auto view00 = myShip.getContainersViewByPosition(X{0}, Y{0});
    myShip.load(X{0}, Y{0}, 4);
    int i=1;
    for(const auto& container : view00) {
        if(i==1){
            if(container!=4)
                return 1;
        }
        else{
            if(container!=1)
                return 1;
        }
        i++;
    }
    return 0;

}

int test8(){
    // simple load, move, unload with restrictions.
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X, Y, Height>(X{0}, Y{0}, Height{0}),
    };

    auto ship = new Ship<std::string>(X{3}, Y{2}, Height{2}, restrictions);
    try {
        ship->load(X{0}, Y{1}, "str");
    } catch (BadShipOperationException& e) {
        std::cout << 1 << std::endl;
        return 1;
    }

    try {
        ship->move(X{0}, Y{1}, X{0}, Y{0});
    } catch (BadShipOperationException& e) {  // Position is full
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    try {
        auto output = ship->unload(X{0}, Y{0});
    } catch (BadShipOperationException& e) {  // Position is empty
        exceptionThrown = true;
    }
    if (!exceptionThrown) {
        exceptionThrown = false;
        return 1;
    }
    exceptionThrown = false;

    try {
        ship->move(X{0}, Y{1}, X{1}, Y{1});
    } catch (BadShipOperationException& e) {
        return 1;
    }

    try {
        auto output = ship->unload(X{1}, Y{1});
        if(output != "str"){
            std::cout << 5 << std::endl;
            return 1;
        }
    } catch (BadShipOperationException& e) {
        std::cout << 6 << std::endl;
        return 1;
    }
    return 0;
}

int test9() {
    // simple load, move, unload with restrictions and grouping.
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            },
            { "first_letter_toupper",
                    [](const std::string& s){ return std::string(1, char(std::toupper(s[0]))); }
            }
    };

    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X, Y, Height>(X{0}, Y{0}, Height{0}),
    };

    auto ship = new Ship<std::string>(X{3}, Y{2}, Height{2}, restrictions, groupingFunctions);
    try {
        ship->load(X{0}, Y{1}, "str");
    } catch (BadShipOperationException& e) {
        std::cout << 1 << std::endl;
        return 1;
    }

    try {
        ship->move(X{0}, Y{1}, X{0}, Y{0});
    } catch (BadShipOperationException& e) {

    }

    try {
        auto output = ship->unload(X{0}, Y{0});
    } catch (BadShipOperationException& e) {

    }

    try {
        ship->move(X{0}, Y{1}, X{1}, Y{1});
    } catch (BadShipOperationException& e) {
        return 1;
    }

    try {
        auto output = ship->unload(X{1}, Y{1});
        if(output != "str"){
            std::cout << 5 << std::endl;
            return 1;
        }
    } catch (BadShipOperationException& e) {
        std::cout << 6 << std::endl;
        return 1;
    }
    return 0;
}

int test10() {
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            },
            { "first_letter_toupper",
                    [](const std::string& s){ return std::string(1, char(std::toupper(s[0]))); }
            }
    };

    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X, Y, Height>(X{0}, Y{0}, Height{0}),
    };

    auto ship = new Ship<std::string>(X{3}, Y{2}, Height{2}, restrictions, groupingFunctions);

    ship->load(X{0}, Y{1}, "str");
    ship->load(X{1}, Y{1}, "sprr");

    std::vector<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> v1 =
            {std::tuple<shipping::X, shipping::Y, shipping::Height,std::string>(X{0}, Y{1}, Height{0} ,"str"),
             std::tuple<shipping::X, shipping::Y, shipping::Height,std::string>(X{1}, Y{1}, Height{0} ,"sprr")};

    auto view_Hh = ship->getContainersViewByGroup("first_letter_toupper", "S");

    std::vector<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> v2;
    for(const auto& container_tuple : view_Hh) {
        v2.emplace_back(
                std::get<0>(container_tuple.first), std::get<1>(container_tuple.first), std::get<2>(container_tuple.first) ,container_tuple.second);
    }

    std::set<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> s1;
    s1.insert(v1.begin(), v1.end());

    std::set<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> s2;
    s2.insert(v2.begin(), v2.end());

    if(s1 == s2)
        return 0;

    return 1;
}

int test11() {
    Ship<int> ship{X{3}, Y{2}, Height{2}};

    ship.load(X{0}, Y{1}, 1);

    ship.load(X{0}, Y{1}, 3);
    ship.load(X{0}, Y{0}, 4);

    std::vector<int> v1 = {1, 3, 4};

    std::vector<int> v2;
    for (const auto& container : ship) {
        v2.push_back(container);
    }

    std::set<int> s1;
    s1.insert(v1.begin(), v1.end());

    std::set<int> s2;
    s2.insert(v2.begin(), v2.end());

    if(s1 == s2)
        return 0;

    return 1;
}

int test12() {
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple<X, Y, Height>(X{0}, Y{0}, Height{0}),
    };

    Ship<std::string> ship {X{3}, Y{2}, Height{2}, restrictions};

    ship.load(X{0}, Y{1}, "str");
    ship.load(X{0}, Y{1}, "sprr");
    ship.load(X{2}, Y{1}, "not_include");

    std::vector<std::string> v1 = {"sprr", "str"};

    auto view00 = ship.getContainersViewByPosition(X{0}, Y{1});

    std::vector<std::string> v2;
    for(const auto& container : view00) {
        v2.push_back(container);
    }

    if(v1 == v2)
        return 0;

    return 1;
}

int test13() {
    Ship<std::string> ship {X{3}, Y{2}, Height{2}};

    ship.load(X{0}, Y{1}, "str");
    ship.load(X{0}, Y{1}, "sprr");
    ship.load(X{2}, Y{1}, "not_include");

    std::vector<std::string> v1 = {};

    auto view = ship.getContainersViewByPosition(X{10}, Y{1});

    std::vector<std::string> v2;
    for(const auto& container : view) {
        v2.push_back(container);
    }

    if(v1 == v2)
        return 0;

    return 1;
}

int test14() {
    Ship<std::string> ship {X{3}, Y{2}, Height{2}};

    ship.load(X{0}, Y{1}, "str");
    ship.load(X{0}, Y{1}, "sprr");
    ship.load(X{2}, Y{1}, "not_include");

    std::vector<std::string> v1 = {};

    auto view = ship.getContainersViewByPosition(X{10}, Y{11});

    std::vector<std::string> v2;
    for(const auto& container : view) {
        v2.push_back(container);
    }

    if(v1 == v2)
        return 0;

    return 1;
}

int test15() {
    Ship<std::string> ship {X{3}, Y{2}, Height{2}};

    ship.load(X{0}, Y{1}, "str");
    ship.load(X{0}, Y{1}, "sprr");
    ship.load(X{2}, Y{1}, "not_include");

    std::vector<std::string> v1 = {};

    auto view = ship.getContainersViewByPosition(X{0}, Y{11});

    std::vector<std::string> v2;
    for(const auto& container : view) {
        v2.push_back(container);
    }

    if(v1 == v2)
        return 0;

    return 1;
}

int test16() {
    Ship<std::string> ship {X{3}, Y{2}, Height{2}};

    ship.load(X{0}, Y{1}, "str");
    ship.load(X{0}, Y{1}, "sprr");
    ship.load(X{2}, Y{1}, "not_include");

    std::vector<std::string> v1 = {};

    auto view = ship.getContainersViewByPosition(X{0}, Y{0});

    std::vector<std::string> v2;
    for(const auto& container : view) {
        v2.push_back(container);
    }

    if(v1 == v2)
        return 0;

    return 1;
}

int test17() {
    Ship<std::string> ship {X{3}, Y{2}, Height{2}};

    ship.load(X{0}, Y{0}, "str");

    auto view0 = ship.getContainersViewByPosition(X{0}, Y{0});
    auto view1 = ship.getContainersViewByPosition(X{1}, Y{1});

    ship.move(X{0}, Y{0}, X{1}, Y{1});
    ship.load(X{0}, Y{0}, "sprr");


    std::vector<std::string> v11 = {"sprr"};

    std::vector<std::string> v21;
    for(const auto& container : view0) {
        v21.push_back(container);
    }

    if(v11 != v21)
        return 1;

    std::vector<std::string> v12 = {"str"};

    std::vector<std::string> v22;
    for(const auto& container : view1) {
        v22.push_back(container);
    }

    if(v12 != v22)
        return 1;

    return 0;
}

int test18() {
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            }
    };

    std::vector<std::tuple<X, Y, Height>> restrictions = {};

    auto ship = new Ship<std::string>(X{3}, Y{2}, Height{2}, restrictions, groupingFunctions);

    ship->load(X{0}, Y{1}, "str");
    ship->load(X{0}, Y{1}, "str12");
    auto view_Hh = ship->getContainersViewByGroup("first_letter", "s");
    ship->unload(X{0}, Y{1});
    ship->load(X{1}, Y{1}, "sprr");

    std::vector<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> v1 =
            {std::tuple<shipping::X, shipping::Y, shipping::Height,std::string>(X{0}, Y{1}, Height{0} ,"str"),
             std::tuple<shipping::X, shipping::Y, shipping::Height,std::string>(X{1}, Y{1}, Height{0} ,"sprr")};

    std::vector<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> v2;
    for(const auto& container_tuple : view_Hh) {
        v2.emplace_back(
                std::get<0>(container_tuple.first), std::get<1>(container_tuple.first), std::get<2>(container_tuple.first) ,container_tuple.second);
    }

    std::set<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> s1;
    s1.insert(v1.begin(), v1.end());

    std::set<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> s2;
    s2.insert(v2.begin(), v2.end());

    if(s1 == s2)
        return 0;

    return 1;
}

int test19() {
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const std::string& s){ return std::string(1, s[0]); }
            }
    };

    std::vector<std::tuple<X, Y, Height>> restrictions = {};

    auto ship = new Ship<std::string>(X{3}, Y{2}, Height{2}, restrictions, groupingFunctions);

    ship->load(X{0}, Y{1}, "str");
    ship->load(X{0}, Y{1}, "str12");
    auto view_Hh = ship->getContainersViewByGroup("first_letter_not_real", "s");
    ship->unload(X{0}, Y{1});
    ship->load(X{1}, Y{1}, "sprr");

    std::vector<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> v1 = {};

    std::vector<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> v2;
    for(const auto& container_tuple : view_Hh) {
        v2.emplace_back(
                std::get<0>(container_tuple.first), std::get<1>(container_tuple.first), std::get<2>(container_tuple.first) ,container_tuple.second);
    }

    std::set<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> s1;
    s1.insert(v1.begin(), v1.end());

    std::set<std::tuple<shipping::X, shipping::Y, shipping::Height, std::string>> s2;
    s2.insert(v2.begin(), v2.end());

    if(s1 == s2)
        return 0;

    return 1;
}

void zivTests() {
    bool somethingFailed = false;
    if(test1()){
        somethingFailed = true;
        std::cout << "Test 1 failed" << std::endl;
    } else
        std::cout << "Test 1 passed" << std::endl;

    if(test2()){
        somethingFailed = true;
        std::cout << "Test 2 failed" << std::endl;
    } else
        std::cout << "Test 2 passed" << std::endl;

    if(test3()){
        somethingFailed = true;
        std::cout << "Test 3 failed" << std::endl;
    } else
        std::cout << "Test 3 passed" << std::endl;

    if(test4()){
        somethingFailed = true;
        std::cout << "Test 4 failed" << std::endl;
    } else
        std::cout << "Test 4 passed" << std::endl;

    if(test5()){
        somethingFailed = true;
        std::cout << "Test 5 failed" << std::endl;
    }else
        std::cout << "Test 5 passed" << std::endl;

    if(test6()){
        somethingFailed = true;
        std::cout << "Test 6 failed" << std::endl;
    } else
        std::cout << "Test 6 passed" << std::endl;

    if(test7()){
        somethingFailed = true;
        std::cout << "Test 7 failed" << std::endl;
    } else
        std::cout << "Test 7 passed" << std::endl;

    if(test8()){
        somethingFailed = true;
        std::cout << "Test 8 failed" << std::endl;
    } else
        std::cout << "Test 8 passed" << std::endl;

    if(test9()){
        somethingFailed = true;
        std::cout << "Test 9 failed" << std::endl;
    } else
        std::cout << "Test 9 passed" << std::endl;

    if(test10()){
        somethingFailed = true;
        std::cout << "Test 10 failed" << std::endl;
    } else
        std::cout << "Test 10 passed" << std::endl;

    if(test11()){
        somethingFailed = true;
        std::cout << "Test 11 failed" << std::endl;
    } else
        std::cout << "Test 11 passed" << std::endl;

    if(test12()){
        somethingFailed = true;
        std::cout << "Test 12 failed" << std::endl;
    } else
        std::cout << "Test 12 passed" << std::endl;

    if(test13()){
        somethingFailed = true;
        std::cout << "Test 13 failed" << std::endl;
    } else
        std::cout << "Test 13 passed" << std::endl;

    if(test14()){
        somethingFailed = true;
        std::cout << "Test 14 failed" << std::endl;
    } else
        std::cout << "Test 14 passed" << std::endl;

    if(test15()){
        somethingFailed = true;
        std::cout << "Test 15 failed" << std::endl;
    } else
        std::cout << "Test 15 passed" << std::endl;

    if(test16()){
        somethingFailed = true;
        std::cout << "Test 16 failed" << std::endl;
    } else
        std::cout << "Test 16 passed" << std::endl;

    if(test17()){
        somethingFailed = true;
        std::cout << "Test 17 failed" << std::endl;
    } else
        std::cout << "Test 17 passed" << std::endl;

    if(test18()){
        somethingFailed = true;
        std::cout << "Test 18 failed" << std::endl;
    } else
        std::cout << "Test 18 passed" << std::endl;

    if(test19()){
        somethingFailed = true;
        std::cout << "Test 19 failed" << std::endl;
    } else
        std::cout << "Test 19 passed" << std::endl;

    if(!somethingFailed) {
        std::cout << " " << std::endl;
        std::cout << "All Good!!" << std::endl;
    }
}

// endregion

// region Noa Tests

#define TEST_PASSED(x) std::cout << "test " << x << " passed" << std::endl;
#define TEST_FAILED(x) std::cout << "test " << x << " failed" << std::endl;
#define DUMMY(x) "Error " << x

void test2b(){
// create restrictions for specific locations on the ship
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple(X{2}, Y{6}, Height{0}),
            std::tuple(X{2}, Y{7}, Height{1}),
            std::tuple(X{2}, Y{5}, Height{6})
    };

    // create bad ship 1
    try {
        restrictions.push_back( std::tuple(X{2}, Y{5}, Height{6}) );
        Ship<std::string> myShip{ X{4}, Y{12}, Height{16}, restrictions };
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - create bad ship 1 - exception - duplicate restriction at(2,5) " << std::endl;
        // exception: duplicate restrictions (whether or not it has same limit):
        // restriction with X{2}, Y{5} appears more than once (added in the try)
        restrictions.pop_back(); // remove the duplicate restriction
    }
    // create bad ship 2
    try {
        Ship<std::string> myShip{ X{4}, Y{7}, Height{8}, restrictions };
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - create bad ship 2 - exception - restriction with Y=7 meaning exceeding the dimensions " << std::endl;
        // exception due to bad restrictions:
        // restriction with Y=7, when the size of Y is 7
    }
    // create bad ship 3
    try {
        Ship<std::string> myShip{ X{4}, Y{12}, Height{6}, restrictions };
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - create bad ship 3 - exception - restriction with H=6 meaning exceeding the dimensions " << std::endl;
        // exception due to bad restrictions:
        // restriction with height=6, when original height is equal or smaller
    }
    // create good ship
    Ship<std::string> myShip{ X{4}, Y{8}, Height{8}, restrictions };
    // bad load - no room
    try {
        myShip.load(X{2}, Y{6}, "Hello");
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - bad load - exception - no room for this container at (2,6) " << std::endl;
        /* no room at this location */ }
    // good load
    myShip.load(X{2}, Y{7}, "Hello");
    // bad load - no room
    try {
        myShip.load(X{2}, Y{7}, "Hello");
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - bad load after loading to (2,7) - exception - no room for this container at (2,7) " << std::endl;
        /* no room at this location */ }

    // bad unload - no container at location
    try {
        std::string container = myShip.unload(X{1}, Y{1});
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - bad unload from(1,1) - exception - no container at this location " << std::endl;
        /* no container at this location */ }

    // bad load - wrong index
    try {
        myShip.load(X{1}, Y{8}, "Hi");
    } catch(BadShipOperationException& e) {
        std::cout << " suppose - bad load - exception - bad index " << std::endl;
        /* bad index Y {8} */ }

    std::cout << "finished test 2 " << std::endl;
}

void test3b(){
    // create grouping pairs
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const string& s){ return string(1, s[0]); }
            },
            { "first_letter_toupper",
                    [](const string& s){ return string(1, char(std::toupper(s[0]))); }
            }
    };
    // create restrictions
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple(X{2}, Y{6}, Height{4}),
            std::tuple(X{2}, Y{7}, Height{6})
    };
    // create ship
    Ship<std::string> myShip{ X{5}, Y{12}, Height{8},
                              restrictions,
                              groupingFunctions };
    // load “containers”
    myShip.load(X{0}, Y{0}, "Hello");
    myShip.load(X{1}, Y{1}, "hey");
    myShip.load(X{1}, Y{1}, "bye");

    auto view00 = myShip.getContainersViewByPosition(X{0}, Y{0});
    auto view_h = myShip.getContainersViewByGroup("first_letter", "h");
    auto view_Hh = myShip.getContainersViewByGroup("first_letter_toupper", "H");

    myShip.load(X{0}, Y{0}, "hi");

    vector<string> words;
    for(auto& container : myShip) {  //
        words.push_back(container);
    }
    sort(words.begin(), words.end()); // expecting Hello, hi, hey, bye after sort
    AssertEquals(words.size(), 4);
    AssertEquals(words[0], "Hello");
    AssertEquals(words[1], "bye");
    AssertEquals(words[2], "hey");
    AssertEquals(words[3], "hi");

    words.clear();
    for(auto& container : view00) {  // expecting hi, Hello - in this order
        words.push_back(container);
    }
    AssertCondition(words.size() == 2, "Expected 2 words, got " + to_string(words.size()));
    AssertEquals(words[0], "hi");
    AssertEquals(words[1], "Hello");

    ViewPair<string> pairs;
//    std::cout <<  "loop on view_h:	pair { tuple{X{0}, Y{0}, Height{1}}, hi }, pair { tuple{X{1}, Y{1}, Height{0}}, hey } - in some undefined order" << std::endl;
    for(const std::pair<std::tuple<X,Y,Height>,std::string>& container_tuple : view_h) {
        pairs.push_back(container_tuple);
    }
    sortPairs(pairs);
    AssertCondition(pairs.size() == 2, "expected 2 values in iterator, but got " + to_string(pairs.size()))
    AssertCondition((posEquals(pairs[0].first, {X(1), Y(1), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[0].second, "hey");
    AssertCondition((posEquals(pairs[1].first, {X(0), Y(0), Height{1}})), "Position of element is invalid")
    AssertEquals(pairs[1].second, "hi");

    pairs.clear();
//    std::cout << "loop on view_Hh: pair { tuple{X{0}, Y{0}, Height{0}}, Hello }, pair { tuple{X{0}, Y{0}, Height{1}}, hi }, pair { tuple{X{1}, Y{1}, Height{0}}, hey } - in some undefined order" << std::endl;
    for(const auto& container_tuple : view_Hh) {
        pairs.push_back(container_tuple);
    }
    sortPairs(pairs);
    AssertEquals(pairs.size(), 3)
    AssertCondition((posEquals(pairs[0].first, {X(0), Y(0), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[0].second, "Hello")
    AssertCondition((posEquals(pairs[1].first, {X(1), Y(1), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[1].second, "hey");
    AssertCondition((posEquals(pairs[2].first, {X(0), Y(0), Height{1}})), "Position of element is invalid")
    AssertEquals(pairs[2].second, "hi");
    TEST_PASSED(3)
}

void test4b(){
    Ship<int> myShip{X{2},Y{2},Height{2}};
    myShip.load(X{1},Y{1},13);
    myShip.move(X{1},Y{1},X{0},Y{0});
    int res = myShip.unload(X{0},Y{0});
    AssertEquals(res, 13)
    TEST_PASSED(4);
}

/*This test checks if the view order is correct*/
void test5b(){
    Ship<int> myShip{X{2},Y{2},Height{8}};
    for(int i = 0; i < 8; i++){
        myShip.load(X{1},Y{1},i);
    }
    auto viewPos = myShip.getContainersViewByPosition(X{1},Y{1});
    int i = 8;
    for(auto &cont : viewPos) {
        i--;
        AssertEquals(cont, i)
    }
    TEST_PASSED(5);
}

void test6b(){
    Grouping<std::string> groupingFunctions = {
            { "first_letter",
                    [](const string& s){ return string(1, s[0]); }
            },
            { "first_letter_toupper",
                    [](const string& s){ return string(1, char(std::toupper(s[0]))); }
            }
    };
    // create restrictions
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple(X{2}, Y{6}, Height{4}),
            std::tuple(X{2}, Y{7}, Height{6})
    };
    // create ship
    Ship<std::string> myShip{ X{5}, Y{12}, Height{8},
                              restrictions,
                              groupingFunctions };
    // load “containers”
    myShip.load(X{0}, Y{0}, "Hello");
    myShip.load(X{1}, Y{1}, "hey");
    myShip.load(X{1}, Y{1}, "bye");
    int numItems1 = 0;
    int numItems2 = 0;

    auto view00 = myShip.getContainersViewByPosition(X{0}, Y{0});
    auto view_h = myShip.getContainersViewByGroup("first_letter", "h");
    auto view_Hh = myShip.getContainersViewByGroup("first_letter_toupper", "H");

    myShip.load(X{0}, Y{0}, "hi");

    for(const auto& container : myShip) {
//        std::cout << container << std::endl;
        numItems1++;
    }
    for(const auto& container : myShip) {
//        std::cout << container << std::endl;
        numItems2++;
    }
    AssertCondition(numItems1 == numItems2, "Fa")
    if(numItems1 != numItems2) {
        TEST_FAILED(6);
        std::cout << "Failed at checking that we able to iterate 2 times on the ship" << std::endl;
    }

    numItems1 = 0;
    numItems2 = 0;
    std::cout << "================" << std::endl;
    for(const auto& container : view00) {
        std::cout << container << std::endl;
        numItems1++;
    }
    std::cout << "================" << std::endl;
    for(const auto& container : view00) {
        std::cout << container << std::endl;
        numItems2++;
    }
    std::cout << "================" << std::endl;
    if(numItems1 != 2 || numItems2 > 0){
        TEST_FAILED(6);
        std::cout << "disabling grouping view00 by position twice failed" << std::endl;
    }
    numItems1 = 0;
    numItems2 = 0;

    for(const auto& container_tuple : view_h) {
        std::cout << container_tuple.second << std::endl;
        numItems1++;
    }
    for(const auto& container_tuple : view_h) {
        std::cout << container_tuple.second << std::endl;
        numItems2++;
    }
    if(numItems1 != 2 || numItems2 > 0){
        TEST_FAILED(6);
        std::cout << "disabling grouping view_h by group twice failed" << std::endl;
    }
    numItems1 = 0;
    numItems2 = 0;

    for(const auto& container_tuple : view_Hh) {
        std::cout << container_tuple.second << std::endl;
        numItems1++;
    }
    for(const auto& container_tuple : view_Hh) {
        std::cout << container_tuple.second << std::endl;
        numItems2++;
    }
    if(numItems1 != 3 || numItems2 > 0){
        TEST_FAILED(6);
        std::cout << "disabling grouping view_Hh by group twice failed" << std::endl;
    }

    numItems1 = 0;

    auto viewbyPos2 = myShip.getContainersViewByPosition(X{0}, Y{0});
    for(const auto& container : viewbyPos2) {
        std::cout << container << std::endl;
        numItems1++;
    }
    if(numItems1 == 0){
        TEST_FAILED(6);
        std::cout << "creating 2nd viewByPos on same x,y as before failed to iterate" << std::endl;
    }

    numItems1 = 0;
    auto view_h2 = myShip.getContainersViewByGroup("first_letter", "h");
    for(const auto& container_tuple : view_h2) {
        std::cout << container_tuple.second << std::endl;
        numItems1++;
    }
    if(numItems1 == 0){
        TEST_FAILED(6);
        std::cout << "creating 2nd view by grouping on same x,y as before failed to iterate" << std::endl;
    }
    TEST_PASSED(6);

}

void test7b() {
    Grouping<int> groupingFunctions = {
            {"0_mod_3",
                    [](const int &num) { return string(1, num%3); }
            },
            {"1_mod_3",
                    [](const int &num) { return string(1, num%3); }
            },
            {"1_mod_6",
                    [](const int &num) { return string(1, num%6); }
            }
    };
    // create restrictions
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple(X{1}, Y{1}, Height{1}),
            std::tuple(X{1}, Y{0}, Height{3}),
            std::tuple(X{0}, Y{1}, Height{2})
    };
    // create ship
    Ship<int> myShip{X{2}, Y{2}, Height{4},
                     restrictions,
                     groupingFunctions};

    // load “containers”
    myShip.load(X{0}, Y{0}, 1);
    myShip.load(X{0}, Y{0}, 2);
    myShip.load(X{0}, Y{0}, 3);
    myShip.load(X{0}, Y{0}, 4);
    try {
        myShip.load(X{0}, Y{0}, 5);
    }
    catch (BadShipOperationException &e) {
        /* no more space in 0,0 */ }
    myShip.load(X{1}, Y{0}, 5);
    myShip.load(X{1}, Y{0}, 8);
    myShip.load(X{1}, Y{1}, 3);

    auto view00 = myShip.getContainersViewByPosition(X{0}, Y{0});
    auto view01 = myShip.getContainersViewByPosition(X{0}, Y{1});

    myShip.move(X{0}, Y{0}, X{0}, Y{1});

    vector<int> actual;
    vector<int> expected = {1, 2, 3, 3, 4, 5, 8};
    for (const auto &container : myShip) {
        actual.push_back(container);
    }
    sort(actual.begin(), actual.end()); // expecting 1 2 3 3 4 5 8
    AssertEquals(actual.size(), expected.size())
    for (int i = 0; i < expected.size(); i++) {
        AssertEquals(actual[i], expected[i]);
    }
    actual.clear();

    // same test again
    for (const auto &container : myShip) {
        actual.push_back(container);
    }
    sort(actual.begin(), actual.end()); // expecting 1 2 3 3 4 5 8
    AssertEquals(actual.size(), expected.size())
    for (int i = 0; i < expected.size(); i++) {
        AssertEquals(actual[i], expected[i]);
    }
    actual.clear();

    expected = {3, 2, 1};
    for (const auto &container : view00) {  // should give 3 2 1 - in order
        actual.push_back(container);
    }
    AssertEquals(actual.size(), expected.size())
    for (int i = 0; i < expected.size(); i++) {
        AssertEquals(actual[i], expected[i]);
    }
    actual.clear();

    for (auto container : view01) {  // should give 4
        actual.push_back(container);
    }
    AssertEquals(actual.size(), 1)
    AssertEquals(actual[0], 4)

    TEST_PASSED(7);
}

void test8b(){
    Grouping<int> groupingFunctions = {
            {"0_mod_3",
                    [](const int &num) { return std::to_string(num % 3); }
            },
            {"1_mod_3",
                    [](const int &num) { return std::to_string(num % 3); }
            },
            {"1_mod_6",
                    [](const int &num) { return std::to_string(num % 6); }
            }
    };
    // create restrictions
    std::vector<std::tuple<X, Y, Height>> restrictions = {
            std::tuple(X{1}, Y{1}, Height{1}),
            std::tuple(X{1}, Y{0}, Height{3}),
            std::tuple(X{0}, Y{1}, Height{2})
    };
    // create ship
    Ship<int> myShip{X{2}, Y{2}, Height{4},
                     restrictions,
                     groupingFunctions};

    // load “containers”
    myShip.load(X{0}, Y{0}, 1);
    myShip.load(X{0}, Y{0}, 2);
    myShip.load(X{0}, Y{0}, 3);
    myShip.load(X{0}, Y{1}, 4);
    myShip.load(X{1}, Y{0}, 5);
    myShip.load(X{1}, Y{0}, 8);
    myShip.load(X{1}, Y{1}, 3);

    auto view_1mod3 = myShip.getContainersViewByGroup("1_mod_3", "1");
    auto view_1mod6 = myShip.getContainersViewByGroup("1_mod_6", "1");
    auto view_0mod3 = myShip.getContainersViewByGroup("0_mod_3", "0");

    ViewPair<int> pairs;
    for(const auto& container_tuple : view_1mod3) {  // expecting: (0,0|0), 1 and (0,1|0), 4
        pairs.push_back(container_tuple);
    }
    sortPairs(pairs);
    AssertEquals(pairs.size(), 2)
    AssertCondition((posEquals(pairs[0].first, {X(0), Y(0), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[0].second, 1)
    AssertCondition((posEquals(pairs[1].first, {X(0), Y(1), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[1].second, 4)
    pairs.clear();

    for(const auto& container_tuple : view_1mod6) {  // expecting: (0,0|0), 1
        pairs.push_back(container_tuple);

    }
    AssertEquals(pairs.size(), 1)
    AssertCondition((posEquals(pairs[0].first, {X(0), Y(0), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[0].second, 1)
    pairs.clear();

    for(const auto& container_tuple : view_0mod3) { // expecting: (0,0|2), 3 and (1,1|0), 3
        pairs.push_back(container_tuple);
    }
    sortPairs(pairs);
    AssertEquals(pairs.size(), 2)
    // can't check the positions because order is undefined - both are 3
//    AssertCondition((posEquals(pairs[0].first, {X(0), Y(0), Height{2}})), "Position of element is invalid")
    AssertEquals(pairs[0].second, 3)
//    AssertCondition((posEquals(pairs[1].first, {X(1), Y(1), Height{0}})), "Position of element is invalid")
    AssertEquals(pairs[1].second, 3)
    pairs.clear();

    TEST_PASSED(8);
}

void noaTests() {
//    std::cout << "==================test1===================" << std::endl;
//    test1();
//    std::cout << "==================test2===================" << std::endl;
//    test2b();
    std::cout << "==================test3 - thier 2nd test with supposed output then output===================" << std::endl;
    test3b();
    std::cout << "==================test4 - move operation===================" << std::endl;
    test4b();
    std::cout << "==================test5 - viewbyPos===================" << std::endl;
    test5b();
//    std::cout << "==================test6 - checking invalidate of 2nd iteration of a view===================" << std::endl;
//    test6b();
    std::cout << "==================test7 - checking viewByPos ===================" << std::endl;
    test7b();
    std::cout << "==================test8 - checking viewGrope ===================" << std::endl;
    test8b();
}

// endregion
