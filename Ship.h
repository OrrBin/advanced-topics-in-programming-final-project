//orrbenyamini 316607696

#ifndef FINAL_PROJECT_SHIP_H

#include <unordered_map>
#include <functional>
#include <utility>
#include <vector>
#include <unordered_set>
#include <set>
#include <map>

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
    class BadShipOperationException : std::exception {
    private:
        std::string message;

    public:
        explicit BadShipOperationException(std::string msg) : message(std::move(msg)) {}
    };


    template<typename Container>
    using Grouping = std::unordered_map<std::string, std::function<std::string(const Container &)>>;

    template<typename Container>
    class Ship {
    public: // Forward Decelerations

        class ShipCargoIterator;

        class GroupView;

        class PositionView;

    private:
        X shipX;
        Y shipY;
        Height shipHeight;
        std::vector<std::vector<int>> spacesLeftAtPosition;
        std::vector<std::vector<Container>> containers;

        Grouping<Container> groupingFunctions;
        using PositionToContainer = std::map<Position, const Container &>;
        using Group = std::unordered_map<std::string, PositionToContainer>;
        mutable std::unordered_map<std::string, Group> groups;

    public:
        Ship(X x, Y y, Height height) noexcept
                : shipX(x), shipY(y), shipHeight(height) {
            spacesLeftAtPosition = std::vector<std::vector<int>>(x, std::vector<int>(y, height));
            containers.resize(x * y);

            // Reserve enough space for each position, otherwise the vector gets reallocated and the Views don't work well
            for (std::vector<Container> &vec: containers) {
                vec.reserve(height + 1);
            }
        }

        Ship(X x, Y y, Height max_height, const std::vector<Position> &restrictions) noexcept(false)
                : Ship(x, y, max_height) {
            validateRestrictions(restrictions);
            for (Position res : restrictions) {
                int resX = std::get<0>(res), resY = std::get<1>(res), resHeight = std::get<2>(res);
                spacesLeftAtPosition[resX][resY] = resHeight;
            }
        }

        Ship(X x, Y y, Height max_height, const std::vector<Position> &restrictions, Grouping<Container> groupingFunctions) noexcept(false)
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
                if (height < 0 || height >= shipHeight) {
                    throw BadShipOperationException(
                            "received position with bad height value. Height value is" + std::to_string(height) + ", ship X is " + std::to_string(shipHeight));
                }
                if (xyHistory.find({x, y}) != xyHistory.end()) {
                    std::string msg = "received duplicate restriction for X,Y : (" + std::to_string(x) + ", " +
                                      std::to_string(y) + ")";
                    throw BadShipOperationException(msg);
                }

                xyHistory.insert({x, y});
            }
        }

        /**
         * Validates (x, y) are legal
         */
        void validateXY(int x, int y) const noexcept(false) {
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
                groups[groupNameAndFunction.first][groupNameAndFunction.second(container)].insert({pos, container});
            }
        }

        /**
         * Removes container from all groups by it's position
         */
        void removeContainerFromAllGroups(Container &container, Position pos) {
            for (auto &groupNameAndFunction: groupingFunctions) {
                groups[groupNameAndFunction.first][groupNameAndFunction.second(container)].erase(pos);
            }
        }

    public:

        /**
         * Loads container to the given position if the position is legal and there is free space in it
         */
        void load(X x, Y y, Container c) noexcept(false) {
            validateXY(x, y);
            if (spacesLeftAtPosition[x][y] == 0) {
                throw BadShipOperationException("Can't load container, no space left in position : (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            }

            getContainers(x, y).push_back(c);
            --spacesLeftAtPosition[x][y];
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
            ++spacesLeftAtPosition[x][y];

            return container;
        }

        /**
         * Moves container from given source position to given target position
         * If there is container in the source position and space in the target position
         */
        void move(X fromX, Y fromY, X toX, Y toY) noexcept(false) {

            validateXY(fromX, fromY);
            validateXY(toX, toY);

            // First check if there is container to move
            if (getContainers(fromX, fromY).empty()) {
                throw BadShipOperationException(
                        "Can't move container, no container found in source position : (" + std::to_string(fromX) + ", " + std::to_string(fromY) + ")");
            }

            // If moving from position to the same position, do nothing
            if (fromX == toX && fromY == toY) {
                return;
            }

            // Check that there is space in the target position
            if (spacesLeftAtPosition[toX][toY] == 0) {
                throw BadShipOperationException(
                        "Can't move container, no space left in target position : (" + std::to_string(toX) + ", " + std::to_string(toY) + ")");
            }

            // Finally unload and then load
            load(toX, toY, unload(fromX, fromY));
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
                auto[insert_itr, _] = groups.insert({groupingName, Group{}});
                itr = insert_itr;
            }
            if (itr != groups.end()) {
                const auto &grouping = itr->second;
                auto itr2 = grouping.find(groupName);
                if (itr2 == grouping.end()) {
                    auto[insert_itr, _] = itr->second.insert({groupName, PositionToContainer{}});
                    itr2 = insert_itr;
                }
                return GroupView(itr2->second);
            }
            return GroupView{};
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * Iterator that iterates over all containers in the ship
         */
        class ShipCargoIterator {
            using CargoIterator = typename std::vector<std::vector<Container>>::const_iterator;
            using PositionIterator = typename std::vector<Container>::const_iterator;

            CargoIterator positionsIterator;  // Iterates over non-empty positions in the ship
            CargoIterator positionsIteratorEnd; // End of this iterator
            PositionIterator currentPositionIterator;  // Iterates over containers in the current position

            void setIteratorToNonEmptyPosition() {
                //progress current position iterator
                ++currentPositionIterator;
                // Check if we have more containers in the current position, if yes return
                if (currentPositionIterator != (*positionsIterator).end()) {
                    return;
                }

                // Find next not empty position
                ++positionsIterator;
                while (positionsIterator != positionsIteratorEnd && (*positionsIterator).empty()) {
                    ++positionsIterator;
                }

                // If we found not empty position set currentPositionIterator
                if (positionsIterator != positionsIteratorEnd) {
                    currentPositionIterator = (*positionsIterator).begin();
                }
            }

        public:
            ShipCargoIterator(CargoIterator itr_start, CargoIterator itr_end)
                    : positionsIterator(itr_start), positionsIteratorEnd(itr_end) {
                if (itr_start != itr_end) {
                    currentPositionIterator = (*itr_start).begin() - 1;
                    setIteratorToNonEmptyPosition();
                }
            }

            ShipCargoIterator operator++() {
                setIteratorToNonEmptyPosition();
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

        /**
         * View for a specific position containers
         */
        class PositionView {
            const std::vector<Container> *containers = nullptr;
            using iterType = typename std::vector<Container>::const_reverse_iterator;

        public:

            explicit PositionView(const std::vector<Container> &containers) : containers(&containers) {}

            PositionView() = default;;

            auto begin() const {
                return containers ? containers->rbegin() : iterType();
            }

            auto end() const {
                return containers ? containers->rend() : iterType();
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * View for a specific group containers
         */
        class GroupView {
            const std::map<Position, const Container &> *pGroup = nullptr;
            using iterType = typename std::map<Position, const Container &>::const_iterator;
        public:
            explicit GroupView(const std::map<Position, const Container &> &group) : pGroup(&group) {}

            GroupView() = default;

            auto begin() const {
                return pGroup ? pGroup->begin() : iterType{};
            }

            auto end() const {
                return pGroup ? pGroup->end() : iterType{};
            }
        };
    };
}
#define FINAL_PROJECT_SHIP_H

#endif //FINAL_PROJECT_SHIP_H
