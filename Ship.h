// orrbenyamini 316607696
#ifndef FINAL_PROJECT_SHIP_H

#include <unordered_map>
#include <functional>
#include <utility>
#include <vector>
#include <unordered_set>
#include <set>
#include <map>

#pragma once

namespace shipping {
    class NamedIntegerType {
        int value;

    public:

        explicit NamedIntegerType(int value) : value{value} {}

        operator int() const { return value; }

        bool operator<(const NamedIntegerType &other) {
            return value < other.value;
        }
    };

    class X : public NamedIntegerType {
        using NamedIntegerType::NamedIntegerType;
    };

    class Y : public NamedIntegerType {
        using NamedIntegerType::NamedIntegerType;
    };

    class Height : public NamedIntegerType {
        using NamedIntegerType::NamedIntegerType;
    };

    using Position = std::tuple<X, Y, Height>;


    /**
     * Exception indicating bad operation occurred
     */
    class BadShipOperationException {
    private:
        std::string message;

    public:
        explicit BadShipOperationException(std::string msg) : message(msg) {}
    };


    template<typename Container>
    using Grouping = std::unordered_map<std::string, std::function<std::string(const Container &)>>;

    template<typename Container>
    class Ship {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        class ShipCargoIterator {
            using CargoIterator = typename std::vector<std::vector<Container>>::const_iterator;
            using PositionIter = typename std::vector<Container>::const_iterator;

            CargoIterator positionsIterator;  // Iterates over non-empty positions in the ship
            CargoIterator positionsIteratorEnd;
            PositionIter currentPositionIterator;  // Iterates over containers in the current position

            void progressIterator() {
                if (++currentPositionIterator != (*positionsIterator).end()) {
                    return;
                }

                while (++positionsIterator != positionsIteratorEnd && (*positionsIterator).empty());  // move to the next non-empty position

                if (positionsIterator != positionsIteratorEnd) {  // reached a non-empty position
                    currentPositionIterator = (*positionsIterator).begin();
                }
            }

        public:
            /**
             * Iterator that iterates over all containers on the ship
             * @param itr_start
             * @param itr_end
             */
            ShipCargoIterator(CargoIterator itr_start, CargoIterator itr_end)
                    : positionsIterator(itr_start), positionsIteratorEnd(itr_end) {
                if (itr_start != itr_end) {  // not an 'end' iterator
                    currentPositionIterator = (*itr_start).begin() - 1;
                    progressIterator();
                }
            }


            ShipCargoIterator operator++() {
                progressIterator();
                return *this;
            }

            const Container &operator*() const {
                return *currentPositionIterator;
            }

            bool operator!=(ShipCargoIterator other) {
                return positionsIterator != other.positionsIterator;
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        class PositionView {
            const std::vector<Container> *containers = nullptr;
            using iterType = typename std::vector<Container>::const_reverse_iterator;

        public:

            PositionView(const std::vector<Container> &containers) : containers(&containers) {}

            PositionView() {};

            auto begin() const {
                return containers ? containers->rbegin() : iterType();
            }

            auto end() const {
                return containers ? containers->rend() : iterType();
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        class GroupView {
            const std::map<Position, const Container &> *pGroup = nullptr;
            using iterType = typename std::map<Position, const Container &>::const_iterator;
        public:
            GroupView(const std::map<Position, const Container &> &group) : pGroup(&group) {}

            GroupView() {}

            auto begin() const {
                return pGroup ? pGroup->begin() : iterType{};
            }

            auto end() const {
                return pGroup ? pGroup->end() : iterType{};
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        X shipX;
        Y shipY;
        Height shipHeight;
        std::vector<std::vector<int>> emptySpacesAtPosition;
        std::vector<std::vector<Container>> containers;

        Grouping<Container> groupingFunctions;
        using PositionToContainer = std::map<Position, const Container &>;
        using Group = std::unordered_map<std::string, PositionToContainer>;
        mutable std::unordered_map<std::string, Group> groups;

    public:
        Ship(X x, Y y, Height height) noexcept
                : shipX(x), shipY(y), shipHeight(height) {
            emptySpacesAtPosition = std::vector<std::vector<int>>(x, std::vector<int>(y, height));
            containers.resize(x * y);

            for (auto& container: containers) {
                container.reserve(height + 1);
            }
        }

        Ship(X x, Y y, Height max_height, std::vector<Position> restrictions) noexcept(false)
                : Ship(x, y, max_height) {
            validateRestrictions(restrictions);
            for (Position res : restrictions) {
                int resX = std::get<0>(res), resY = std::get<1>(res), resHeight = std::get<2>(res);
                emptySpacesAtPosition[resX][resY] = resHeight;
            }
        }

        Ship(X x, Y y, Height max_height, std::vector<Position> restrictions, Grouping<Container> groupingFunctions) noexcept(false)
                : Ship(x, y, max_height, restrictions) {
            this->groupingFunctions = groupingFunctions;
        }

        Ship(const Ship &) = delete;

        Ship &operator=(const Ship &) = delete;

        Ship(Ship &&) = delete;

        Ship &operator=(Ship &&) = delete;

    private:
        /**
         * Returns the containers from the given (x, y) position - const version
         */
        const std::vector<Container> &getContainers(int x, int y) const {
            return containers[x * shipY + y];
        }

        /**
         * Returns the containers from the given (x, y) position
         */
        std::vector<Container> &getContainers(int x, int y) {
            return containers[x * shipY + y];
        }

        /**
         * Validates the given restrictions
         */
        void validateRestrictions(const std::vector<Position> &restrictions) noexcept(false) {
            std::set<std::tuple<int, int>> xyHistory;

            for (Position res : restrictions) {
                int x = std::get<0>(res), y = std::get<1>(res), height = std::get<2>(res);
                validateXY(x, y);
                if (height < 0 || height >= shipY)
                    if (xyHistory.find({x, y}) != xyHistory.end()) {
                        std::string msg = "received duplicate restriction for X,Y : (" + std::to_string(x) + ", " +
                                          std::to_string(y) + ")";
                        throw BadShipOperationException(msg);
                    }

                xyHistory.insert({x, y});
            }
        }

        const void validateXY(int x, int y) noexcept(false) {
            if (x < 0 || x >= shipX)
                throw BadShipOperationException(
                        "received position with bad X value. X value is" + std::to_string(x) + ", ship X is " + std::to_string(shipX));

            if (y < 0 || y >= shipY)
                throw BadShipOperationException(
                        "received position with bad Y value. Y value is" + std::to_string(y) + ", ship Y is " + std::to_string(shipY));
        }

        /**
         * Adds container to all relevant groups by it's position
         */
        void addContainerToAllGroups(Container &container, Position pos) {
            for (auto &groupNameAndFunction: groupingFunctions) {
                const std::string &groupName = groupNameAndFunction.first;
                auto &groupingFunc = groupNameAndFunction.second;
                groups[groupName][groupingFunc(container)].insert({pos, container});
            }
        }

        /**
         * Removes container from all groups by it's position
         */
        void removeContainerFromAllGroups(Container &container, Position pos) {
            for (auto &groupNameAndFunction: groupingFunctions) {
                const std::string &groupName = groupNameAndFunction.first;
                auto &groupingFunc = groupNameAndFunction.second;
                groups[groupName][groupingFunc(container)].erase(pos);
            }
        }

    public:

        /**
         * Loads container to the given position if the position is legal and there is free space in it
         */
        void load(X x, Y y, Container c) noexcept(false) {
            validateXY(x, y);
            if (emptySpacesAtPosition[x][y] == 0) {
                throw BadShipOperationException("Can't load container, no space left in position : (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            }

            getContainers(x, y).push_back(c);
            --emptySpacesAtPosition[x][y];
            int height = getContainers(x, y).size() - 1;
            addContainerToAllGroups(getContainers(x, y).back(), {X{x}, Y{y}, Height{height}});
        }

        /**
         * Unloads container from given position if the position is legal and there is at least one container there
         */
        Container unload(X x, Y y) noexcept(false) {
            validateXY(x, y);
            if (getContainers(x, y).empty()) {
                throw BadShipOperationException(
                        "Can't unload container, no container found in position : (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            }

            int height = getContainers(x, y).size() - 1;
            Container container = getContainers(x, y).back();
            removeContainerFromAllGroups(container, {X{x}, Y{y}, Height{height}});

            getContainers(x, y).pop_back();
            ++emptySpacesAtPosition[x][y];

            return container;
        }

        /**
         * Moves container from given source position to given target position
         * If there is container in the source position and space in the target position
         */
        void move(X from_x, Y from_y, X to_x, Y to_y) noexcept(false) {

            validateXY(from_x, from_y);
            validateXY(to_x, to_y);

            if (getContainers(from_x, from_y).empty()) {
                throw BadShipOperationException(
                        "Can't move container, no container found in source position : (" + std::to_string(from_x) + ", " + std::to_string(from_y) + ")");
            }

            if (from_x == to_x && from_y == to_y) {
                return;
            }

            if (emptySpacesAtPosition[to_x][to_y] == 0) {
                throw BadShipOperationException(
                        "Can't move container, no space left in target position : (" + std::to_string(to_x) + ", " + std::to_string(to_y) + ")");
            }

            load(to_x, to_y, unload(from_x, from_y));
        }

        ShipCargoIterator begin() const {
            return ShipCargoIterator(containers.begin(), containers.end());
        }

        ShipCargoIterator end() const {
            return ShipCargoIterator(containers.end(), containers.end());
        }

        /**
         * Returns view of containers in the given (x, y) position
         */
        PositionView getContainersViewByPosition(X x, Y y) const {
            if (x < 0 || x >= shipX || y < 0 || y >= shipY)
                return PositionView();
            return PositionView(getContainers(x, y));
        }

        /**
         * Returns view of containers of the given group
         */
        GroupView getContainersViewByGroup(const std::string &groupingName, const std::string &groupName) const {
            auto itr = groups.find(groupingName);
            if (itr == groups.end() && groupingFunctions.find(groupingName) != groupingFunctions.end()) {
                auto [insert_itr, _] = groups.insert({groupingName, Group{}});
                itr = insert_itr;
            }
            if (itr != groups.end()) {
                const auto &grouping = itr->second;
                auto itr2 = grouping.find(groupName);
                if (itr2 == grouping.end()) {
                    auto [insert_itr, _] = itr->second.insert({groupName, PositionToContainer {}});
                     itr2 = insert_itr;
                }
                return GroupView(itr2->second);
            }
            return GroupView{};
        }

    };
}
#define FINAL_PROJECT_SHIP_H

#endif //FINAL_PROJECT_SHIP_H