#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


class TBaseMoveImpl;
using TBaseMoveImplPtr = std::shared_ptr<TBaseMoveImpl>;
class TBaseMoveImpl {
    public:
        virtual ~TBaseMoveImpl() {}
        virtual void Apply(std::string &cube) const = 0;
        void SetMoveId(const std::string &id) {
            Id = id;
        }
        TBaseMoveImplPtr Clone() const {
            auto result = DoClone();
            result->SetMoveId(Id);
            return result;
        }

    private:
        std::string Id;

        virtual TBaseMoveImplPtr DoClone() const = 0;
};


class TEmptyMove : public TBaseMoveImpl {
    public:
        void Apply(std::string &) const override {
        }

    private:
        TBaseMoveImplPtr DoClone() const override {
            return TBaseMoveImplPtr(new TEmptyMove());
        }
};


class TSimpleCycleMove : public TBaseMoveImpl {
    public:
        TSimpleCycleMove(const std::vector<size_t> &cycle)
            : Cycle(cycle)
        {
        }
        void Apply(std::string &cube) const override {
            char t = cube[Cycle[0]];
            size_t n = Cycle.size();
            for (size_t i = n - 1; i > 0; --i)
                cube[Cycle[(i + 1) % n]] = cube[Cycle[i]];
            cube[Cycle[1]] = t;
        }

    private:
        std::vector<size_t> Cycle;

        TBaseMoveImplPtr DoClone() const override {
            return TBaseMoveImplPtr(new TSimpleCycleMove(Cycle));
        }
};


class TCompositeMove : public TBaseMoveImpl {
    public:
        TCompositeMove(TBaseMoveImplPtr left, TBaseMoveImplPtr right)
            : Left(left)
            , Right(right)
        {
        }
        void Apply(std::string &cube) const override {
            if (!!Left)
                Left->Apply(cube);
            if (!!Right)
                Right->Apply(cube);
        }

    private:
        TBaseMoveImplPtr Left;
        TBaseMoveImplPtr Right;

        TBaseMoveImplPtr DoClone() const override {
            auto left = !!Left ? Left->Clone() : TBaseMoveImplPtr();
            auto right = !!Right ? Right->Clone() : TBaseMoveImplPtr();
            return TBaseMoveImplPtr(new TCompositeMove(left, right));
        }
};


class TMove {
    public:
        TMove()
            : Impl(new TEmptyMove())
        {
        }

        TMove(const std::vector<size_t> &cycle)
            : Impl(new TSimpleCycleMove(cycle))
        {
        }

        TMove(const TMove &rgt)
            : Impl(rgt.Impl->Clone())
        {
        }

        TMove(TMove &&rgt)
            : Impl(std::move(rgt.Impl))
        {
        }

        TMove &operator = (const TMove &rgt) {
            TMove tmp(rgt);
            std::swap(Impl, tmp.Impl);
            return *this;
        }

        TMove &operator += (const TMove &rgt) {
            auto right = rgt.Impl->Clone();
            TBaseMoveImplPtr composite(new TCompositeMove(Impl, right));
            std::swap(Impl, composite);
            return *this;
        }

        void operator () (std::string &cube) const {
            if (!!Impl)
                Impl->Apply(cube);
        }

    private:
        TBaseMoveImplPtr Impl;
};

TMove operator + (TMove lft, const TMove &rgt) {
    lft += rgt;
    return std::move(lft);
}


template<typename T>
TMove CreateComposite(T move) {
    return TMove(move);
}

template<typename T, typename ...Targs>
TMove CreateComposite(T first, Targs ...args) {
    auto result = CreateComposite(first);;
    result += CreateComposite(args...);
    return result;
}


std::unordered_map<std::string, TMove> GenerateAllMoves() {

/*
          B
          U
        L F R
          D

          2
          1
        4 0 5
          3

          o
          b
        w r y
          g


               15 16 17
               18 19 20

               06 07 08
               09 10 11
               12 13 14 

     30 31 32  00 01 02  36 37 38
     33 34 35  03 04 05  39 40 41

               21 22 23
               24 25 26
               27 28 29

*/

    std::unordered_map<std::string, TMove> allMoves;
    allMoves["L2"] = CreateComposite(TMove({30, 35}), TMove({31, 34}), TMove({32, 33}), TMove({3, 18}), TMove({0, 15}), TMove({6, 21}), TMove({9, 24}), TMove({12, 27}));
    allMoves["R2"] = CreateComposite(TMove({36, 41}), TMove({37, 40}), TMove({38, 39}), TMove({5, 20}), TMove({2, 17}), TMove({14, 29}), TMove({11, 26}), TMove({8, 23}));
    allMoves["F2"] = CreateComposite(TMove({0, 5}), TMove({1, 4}), TMove({2, 3}), TMove({12, 23}), TMove({13, 22}), TMove({14, 21}), TMove({32, 39}), TMove({35, 36}));
    allMoves["B2"] = CreateComposite(TMove({15, 20}), TMove({16, 19}), TMove({17, 18}), TMove({6, 29}), TMove({7, 28}), TMove({8, 27}), TMove({30, 41}), TMove({33, 38}));
    auto u = CreateComposite(TMove({6, 8, 14, 12}), TMove({7, 11, 13, 9}), TMove({0, 30, 20, 36}), TMove({1, 31, 19, 37}), TMove({2, 32, 18, 38}));
    allMoves["U"] = u;
    allMoves["U2"] = CreateComposite(u, u);
    allMoves["U'"] = CreateComposite(u, u, u);
    auto d = CreateComposite(TMove({21, 23, 29, 27}), TMove({22, 26, 28, 24}), TMove({3, 39, 17, 33}), TMove({4, 40, 16, 34}), TMove({5, 41, 15, 35}));
    allMoves["D"] = d;
    allMoves["D2"] = CreateComposite(d, d);
    allMoves["D'"] = CreateComposite(d, d, d);
    return allMoves;
}


std::unordered_map<std::string, TMove> GenerateAll2Moves() {
    std::unordered_map<std::string, TMove> result;
    for (const auto &move : GenerateAllMoves()) {
        if (move.first.size() == 2 && move.first[1] == '2')
            result[move.first] = move.second;
    }
    return result;
}


bool DoSolve(const std::string &startCube, const std::string &endCube, const std::unordered_map<std::string, TMove> &allMoves, std::vector<std::string> &fwd, std::vector<std::string> &bwd) {
    if (startCube == endCube)
        return true;
    using TMoveCodes = std::vector<std::string>;
    using TDict = std::unordered_map<std::string, TMoveCodes>;
    using TQueue = std::list<std::string>;
    TDict fwdReached, bwdReached;
    TQueue fwdQueue, bwdQueue;
    fwdReached[startCube];
    fwdQueue.push_back(startCube);
    bwdReached[endCube];
    bwdQueue.push_back(endCube);
    while (!fwdQueue.empty() || !bwdQueue.empty()) {
        if (!fwdQueue.empty()) {
            auto current = fwdQueue.front();
            auto currentMove = fwdReached[current];
            //std::cout << fwdQueue.size() << ' ' << fwdReached.size() << ' ' << currentMove.size() << std::endl;
            fwdQueue.pop_front();
            for (const auto &move : allMoves) {
                auto cube = current;
                move.second(cube);
                if (fwdReached.find(cube) == fwdReached.end()) {
                    auto &nextMove = fwdReached[cube];
                    nextMove = currentMove;
                    nextMove.push_back(move.first);
                    fwdQueue.push_back(cube);
                }
                if (bwdReached.find(cube) != bwdReached.end()) {
                    fwd = fwdReached[cube];
                    bwd = bwdReached[cube];
                    return true;
                }
            }
        }
        if (!bwdQueue.empty()) {
            auto current = bwdQueue.front();
            auto currentMove = bwdReached[current];
            //std::cout << bwdQueue.size() << ' ' << bwdReached.size() << ' ' << currentMove.size() << std::endl;
            bwdQueue.pop_front();
            for (const auto &move : allMoves) {
                auto cube = current;
                move.second(cube);
                if (bwdReached.find(cube) == bwdReached.end()) {
                    auto &nextMove = bwdReached[cube];
                    nextMove = currentMove;
                    nextMove.push_back(move.first);
                    bwdQueue.push_back(cube);
                }
                if (fwdReached.find(cube) != fwdReached.end()) {
                    fwd = fwdReached[cube];
                    bwd = bwdReached[cube];
                    return true;
                }
            }
        }
        //std::cout << std::endl;
    }
    return false;
}


std::vector<std::string> ReverseMoves(const std::vector<std::string> &moves) {
    std::vector<std::string> result;
    for (const auto &move : moves) {
        if (move.size() > 1 && move[1] == '2')
            result.push_back(move);
        else if (move.size() > 1)
            result.push_back(move.substr(0, 1));
        else
            result.push_back(move + '\'');
    }
    std::reverse(result.begin(), result.end());
    return result;
}


std::string Project(const std::string &cube) {
    std::string result;
    for (size_t i = 0, n = cube.size(); i < n; ++i) {
        char ch = cube[i];
        if (ch == 'o')
            ch = 'r';
        else if (ch == 'g')
            ch = 'b';
        result += ch;
    }
    return result;
}


bool Solve(const std::string &startCube, const std::string &endCube, const std::unordered_map<std::string, TMove> &allMoves, std::vector<std::string> &result) {
    std::vector<std::string> fwd, bwd;
    if (!DoSolve(startCube, endCube, allMoves, fwd, bwd))
        return false;
    result.reserve(fwd.size() + bwd.size());
    for (const auto &id : fwd)
        result.push_back(id);
    for (const auto &id : ReverseMoves(bwd))
        result.push_back(id);
    return true;
}


bool Solve2Stages(const std::string &startCube, const std::string &endCube, std::vector<std::string> &result) {
    auto allMoves = GenerateAllMoves();
    if (!Solve(Project(startCube), Project(endCube), allMoves, result))
        return false;
    //std::cout << result.size() << std::endl;
    std::string cube = startCube;
    for (const auto &move : result)
        allMoves[move](cube);
    //result.clear();
    std::cout << cube << std::endl;
    return Solve(cube, endCube, GenerateAll2Moves(), result);
}


int main() {
    //std::string startCube;
    //std::cin >> startCube;
    //std::string startCube = "yyyorwggbgbbbgbrywwowgggbgbgbbowryryoworor";
    //std::string startCube = "wyywrwgbbgbggbgorwowyggbggbbbbyoryworyroor";
    std::string startCube = "ooorrrgbggbgbgbroorrobggbgbbbgwwywwywywyyy";
    std::string endCube = "rrrrrrbbbbbbbbboooooogggggggggwwwwwwyyyyyy";
    std::vector<std::string> moves;
    if (Solve2Stages(startCube, endCube, moves)) {
        for (const auto &id : moves)
            std::cout << ' ' << id;
        std::cout << std::endl;
    } else {
        std::cout << "No solution" << std::endl;
    }
    return 0;
}

