#pragma once

//#include "cell.h"
#include "common.h"

#include <functional>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <set>

class Cell;

namespace maps {

template<typename T>
class Row {
public:
    void SetCell(int pos, std::unique_ptr<T>&& cell);

    const T* GetCell(int pos) const;
    T* GetCell(int pos);

    void ClearCell(int pos);
    void PrintValues(std::ostream& output, int pr_size) const;
    void PrintTexts(std::ostream& output, int pr_size) const;

    bool IsEmptyRow() const {
        return cells_.empty();
    }

    int GetMaxIndex() const {
        return *keys_in_order_.rbegin();
    }

private:
    std::unordered_map<int, std::unique_ptr<T>> cells_;
    std::set<int> keys_in_order_;
};

template<typename T>
class RowsAndColumns {
public:
    void SetCell(Position pos, std::unique_ptr<T>&& cell);

    const T* GetCell(Position pos) const;
    T* GetCell(Position pos);

    void ClearCell(Position pos);

    Size GetPrintableSize() const;

    void PrintValues(std::ostream& output) const;
    void PrintTexts(std::ostream& output) const;
private:
    std::unordered_map<int, std::unique_ptr<Row<T>>> rows_; 
    std::set<int> keys_in_order_;

    void PrintEmptyRow(std::ostream& output, int size_col) const;
};


}//namespace maps

class Sheet final: public SheetInterface {
public:

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetCellSpecifically(Position pos) const;
    Cell* GetCellSpecifically(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    maps::RowsAndColumns<Cell> data_;

    void CheckPosition(Position pos) const;
    
};