#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, CellInterface::Value val) {
    if (std::holds_alternative<double>(val)) {
        output << std::get<double>(val);
    }
    else if (std::holds_alternative<std::string>(val)) {
        output << std::get<std::string>(val);
    }
    else {
        output << std::get<FormulaError>(val);
    }
    return output;
}

namespace maps {
template<typename T>
void RowsAndColumns<T>::SetCell(Position pos, std::unique_ptr<T>&& cell) {
    //check, if there already is a row #pos.row
    if (!rows_.count(pos.row + 1)) {
        //need to create a new row
        std::unique_ptr<Row<T>> new_row = std::make_unique<Row<T>>();
        rows_.insert({ pos.row + 1, std::move(new_row) });
        keys_in_order_.insert(pos.row + 1);
    }

    rows_.at(pos.row + 1)->SetCell(pos.col + 1, std::move(cell));

}

template<typename T>
const T* RowsAndColumns<T>::GetCell(Position pos) const {
    if (!rows_.count(pos.row + 1)) {
        return nullptr;
    }
    else return rows_.at(pos.row + 1)->GetCell(pos.col + 1);
}

template<typename T>
T* RowsAndColumns<T>::GetCell(Position pos) {
    if (!rows_.count(pos.row + 1)) {
        return nullptr;
    }
    else return rows_.at(pos.row + 1)->GetCell(pos.col + 1);
}

template<typename T>
void RowsAndColumns<T>::ClearCell(Position pos) {
    if (!rows_.count(pos.row + 1)) {
        return;
    }

    rows_.at(pos.row + 1)->ClearCell(pos.col + 1);

    if (rows_.at(pos.row + 1)->IsEmptyRow()) {
        rows_.erase(pos.row + 1);
        keys_in_order_.erase(pos.row + 1);
    }
}

template<typename T>
Size RowsAndColumns<T>::GetPrintableSize() const {
    Size s;
    if (rows_.empty()) {
        return s;
    }

    int r = *keys_in_order_.rbegin();
    s.rows = r;

    for (auto& row : rows_) {
        s.cols = std::max(s.cols, row.second->GetMaxIndex());
    }

    return s;
}

template<typename T>
void RowsAndColumns<T>::PrintValues(std::ostream& output) const {
    Size pr_size = GetPrintableSize();

    for (int i = 1; i - 1 < pr_size.rows; ++i) {

        if (rows_.count(i)) {
            rows_.at(i)->PrintValues(output, pr_size.cols);
        }
        else {
            PrintEmptyRow(output, pr_size.cols);
        }

        output << '\n';
    }

}

template<typename T>
void RowsAndColumns<T>::PrintEmptyRow(std::ostream& output, int size_col) const {
    for (int i = 1; i - 1 < size_col; ++i) {
        if (i < size_col) {
            output << '\t';
        }
    }
}

template<typename T>
void RowsAndColumns<T>::PrintTexts(std::ostream& output) const {
    Size pr_size = GetPrintableSize();

    for (int i = 1; i - 1 < pr_size.rows; ++i) {

        if (rows_.count(i)) {
            rows_.at(i)->PrintTexts(output, pr_size.cols);
        }
        else {
            PrintEmptyRow(output, pr_size.cols);
        }

        output << '\n';
    }

}

template<typename T>
void Row<T>::PrintTexts(std::ostream& output, int pr_size) const {

    for (int i = 1; i - 1 < pr_size; ++i) {
        if (cells_.count(i)) {
            output << cells_.at(i)->GetText();
        }

        if (i < pr_size) {
            output << '\t';
        }
    }

}

template<typename T>
void Row<T>::PrintValues(std::ostream& output, int pr_size) const {

    for (int i = 1; i - 1 < pr_size; ++i) {
        if (cells_.count(i)) {
            output << cells_.at(i)->GetValue();
        }

        if (i < pr_size) {
            output << '\t';
        }
    }

}


template<typename T>
void Row<T>::SetCell(int pos, std::unique_ptr<T>&& cell) {
    cells_[pos] = std::move(cell);

    keys_in_order_.insert(pos);
}

template<typename T>
const T* Row<T>::GetCell(int pos) const {
    if (!cells_.count(pos)) {
        return nullptr;
    }
    return cells_.at(pos).get();
}

template<typename T>
T* Row<T>::GetCell(int pos) {
    if (!cells_.count(pos)) {
        return nullptr;
    }
    return cells_.at(pos).get();
}

template<typename T>
void Row<T>::ClearCell(int pos) {
    if (!cells_.count(pos)) {
        return;
    }

    auto& cell = cells_.at(pos);
    cell->Clear();
    cell.reset();
    cells_.erase(pos);

    keys_in_order_.erase(pos);
}
}//namespace maps


void Sheet::CheckPosition(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    
    CheckPosition(pos);
   
    Cell* cell = data_.GetCell(pos);
    if (!cell) {
        std::unique_ptr<Cell> new_cell = std::make_unique<Cell>(*this, pos);
        new_cell->Set(std::move(text));
        data_.SetCell(pos, std::move(new_cell));
    }
    else {
      
        cell->Set(std::move(text));
    }

}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPosition(pos);
    return data_.GetCell(pos);
}
CellInterface* Sheet::GetCell(Position pos) {
    CheckPosition(pos);
    return data_.GetCell(pos);
}

const Cell* Sheet::GetCellSpecifically(Position pos) const {
    CheckPosition(pos);
    return data_.GetCell(pos);
}
Cell* Sheet::GetCellSpecifically(Position pos) {
    CheckPosition(pos);
    return data_.GetCell(pos);
}


void Sheet::ClearCell(Position pos) {
    CheckPosition(pos);

    data_.ClearCell(pos);
}

Size Sheet::GetPrintableSize() const {
    return data_.GetPrintableSize();
}

void Sheet::PrintValues(std::ostream& output) const {
    data_.PrintValues(output);
}
void Sheet::PrintTexts(std::ostream& output) const {
    data_.PrintTexts(output);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}