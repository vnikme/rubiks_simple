#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


class TCube {
    public:
        explicit TCube(const std::string &value)
            : Value(value) {
        }
        const std::string &GetValue() const {
            return Value;
        }
        void Permute(const std::vector<size_t> &cycle) {
            char t = Value[cycle[0]];
            size_t n = cycle.size();
            for (size_t i = n - 1; i > 0; --i)
                Value[cycle[(i + 1) % n]] = Value[cycle[i]];
            Value[cycle[1]] = t;
        }


    private:
        std::string Value;
};


class TBaseMoveImpl;
using TBaseMoveImplPtr = std::shared_ptr<TBaseMoveImpl>;
class TBaseMoveImpl {
    public:
        virtual ~TBaseMoveImpl() {}
        virtual void Apply(TCube &cube) const = 0;
        void SetMoveId(const std::string &id) {
            Id = id;
        }
        TBaseMoveImplPtr Clone() const {
            auto result = DoClone();
            result->SetMoveId(Id);
            return result;
        }
        void FillMoveIds(std::vector<std::string> &ids) const {
            if (!Id.empty())
                ids.push_back(Id);
            else
                DoFillMoveIds(ids);
        }


    private:
        std::string Id;

        virtual TBaseMoveImplPtr DoClone() const = 0;
        virtual void DoFillMoveIds(std::vector<std::string> &ids) const = 0;
};


class TEmptyMove : public TBaseMoveImpl {
    public:
        void Apply(TCube &) const override {
        }

    private:
        TBaseMoveImplPtr DoClone() const override {
            return TBaseMoveImplPtr(new TEmptyMove());
        }
        void DoFillMoveIds(std::vector<std::string> &) const override {
        }
};


class TSimpleCycleMove : public TBaseMoveImpl {
    public:
        TSimpleCycleMove(const std::vector<size_t> &cycle)
            : Cycle(cycle)
        {
        }
        void Apply(TCube &cube) const override {
            cube.Permute(Cycle);
        }

    private:
        std::vector<size_t> Cycle;

        TBaseMoveImplPtr DoClone() const override {
            return TBaseMoveImplPtr(new TSimpleCycleMove(Cycle));
        }
        void DoFillMoveIds(std::vector<std::string> &) const override {
        }
};


class TCompositeMove : public TBaseMoveImpl {
    public:
        TCompositeMove(TBaseMoveImplPtr left, TBaseMoveImplPtr right)
            : Left(left)
            , Right(right)
        {
        }
        void Apply(TCube &cube) const override {
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

        void DoFillMoveIds(std::vector<std::string> &ids) const override {
            if (!!Left)
                Left->FillMoveIds(ids);
            if (!!Right)
                Right->FillMoveIds(ids);
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

        void SetMoveId(const std::string &id) {
            if (!!Impl)
                Impl->SetMoveId(id);
        }

        std::vector<std::string> GetMoveIds() const {
            std::vector<std::string> result;
            if (!!Impl)
                Impl->FillMoveIds(result);
            return result;
        }

        void operator () (TCube &cube) const {
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
TMove DoCreateComposite(T move) {
    return TMove(move);
}

template<typename T, typename ...Targs>
TMove DoCreateComposite(T first, Targs ...args) {
    auto result = DoCreateComposite(first);;
    result += DoCreateComposite(args...);
    return result;
}

template<typename ...Targs>
TMove CreateComposite(const std::string &id, Targs ...args) {
    auto move = DoCreateComposite(args...);
    move.SetMoveId(id);
    return move;
}


std::vector<TMove> GenerateAllMoves() {

/*
          B
          U
        L F R
          D

          2
          1
        4 0 5
          3

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

    std::vector<TMove> allMoves;
    allMoves.push_back(CreateComposite("L2", TMove({30, 35}), TMove({31, 34}), TMove({32, 33}), TMove({3, 18}), TMove({0, 15}), TMove({6, 21}), TMove({9, 24}), TMove({12, 27})));
    allMoves.push_back(CreateComposite("R2", TMove({36, 41}), TMove({37, 40}), TMove({38, 39}), TMove({5, 20}), TMove({2, 17}), TMove({14, 29}), TMove({11, 26}), TMove({8, 23})));
    allMoves.push_back(CreateComposite("F2", TMove({0, 5}), TMove({1, 4}), TMove({2, 3}), TMove({12, 23}), TMove({13, 22}), TMove({14, 21}), TMove({32, 39}), TMove({35, 36})));
    allMoves.push_back(CreateComposite("B2", TMove({15, 20}), TMove({16, 19}), TMove({17, 18}), TMove({6, 29}), TMove({7, 28}), TMove({8, 27}), TMove({30, 41}), TMove({33, 38})));
    auto u = CreateComposite("U", TMove({6, 8, 14, 12}), TMove({7, 11, 13, 9}), TMove({0, 30, 20, 36}), TMove({1, 31, 19, 37}), TMove({2, 32, 18, 38}));
    allMoves.push_back(u);
    allMoves.push_back(CreateComposite("U2", u, u));
    allMoves.push_back(CreateComposite("U'", u, u, u));
    auto d = CreateComposite("D", TMove({21, 23, 29, 27}), TMove({22, 26, 28, 24}), TMove({3, 39, 17, 33}), TMove({4, 40, 16, 34}), TMove({5, 41, 15, 35}));
    allMoves.push_back(d);
    allMoves.push_back(CreateComposite("D2", d, d));
    allMoves.push_back(CreateComposite("D'", d, d, d));
    return allMoves;
}



bool DoSolve(const std::string &startString, TMove &fwd, TMove &bwd) {
    auto allMoves = GenerateAllMoves();
    TCube startCube(startString), endCube("rrrrrrbbbbbbbbboooooogggggggggwwwwwwyyyyyy");
    //TCube startCube("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"), endCube("rrrrrrbbbbbbbbbrrrrrrbbbbbbbbbwwwwwwwwwwww");
    //TCube startCube("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"), endCube("rrrrrrbbbbbbbbboooooogggggggggwwwwwwyyyyyy");
    using TDict = std::unordered_map<std::string, TMove>;
    using TQueue = std::list<std::string>;
    TDict fwdReached, bwdReached;
    TQueue fwdQueue, bwdQueue;
    fwdReached[startCube.GetValue()] = TMove();
    fwdQueue.push_back(startCube.GetValue());
    bwdReached[endCube.GetValue()] = TMove();
    bwdQueue.push_back(endCube.GetValue());
    while (!fwdQueue.empty() || !bwdQueue.empty()) {
        if (!fwdQueue.empty()) {
            auto cubeString = fwdQueue.front();
            auto currentMove = fwdReached[cubeString];
            fwdQueue.pop_front();
            for (const auto &move : allMoves) {
                TCube cube(cubeString);
                move(cube);
                if (fwdReached.find(cube.GetValue()) == fwdReached.end()) {
                    fwdReached[cube.GetValue()] = currentMove + move;
                    fwdQueue.push_back(cube.GetValue());
                }
                if (bwdReached.find(cube.GetValue()) != bwdReached.end()) {
                    fwd = fwdReached[cube.GetValue()];
                    bwd = bwdReached[cube.GetValue()];
                    return true;
                }
            }
        }
        if (!bwdQueue.empty()) {
            auto cubeString = bwdQueue.front();
            auto currentMove = bwdReached[cubeString];
            std::cout << bwdQueue.size() << ' ' << bwdReached.size() << ' ' << currentMove.GetMoveIds().size() << std::endl;
            bwdQueue.pop_front();
            for (const auto &move : allMoves) {
                TCube cube(cubeString);
                move(cube);
                if (bwdReached.find(cube.GetValue()) == bwdReached.end()) {
                    bwdReached[cube.GetValue()] = currentMove + move;
                    bwdQueue.push_back(cube.GetValue());
                }
                if (fwdReached.find(cube.GetValue()) != fwdReached.end()) {
                    fwd = fwdReached[cube.GetValue()];
                    bwd = bwdReached[cube.GetValue()];
                    return true;
                }
            }
        }
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


int main() {
    std::string startString;
    std::cin >> startString;
    TMove fwd, bwd;
    if (DoSolve(startString, fwd, bwd)) {
        for (const auto &id : fwd.GetMoveIds())
            std::cout << ' ' << id;
        for (const auto &id : ReverseMoves(bwd.GetMoveIds()))
            std::cout << ' ' << id;
        std::cout << std::endl;
    } else {
        std::cout << "No solution" << std::endl;
    }
    return 0;
}

