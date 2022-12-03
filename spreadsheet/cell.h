#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <set>
#include <functional>
#include <unordered_set>
#include <optional>

//class Sheet;

class Cell : public CellInterface {
public:
        Cell(Sheet& sheet, Position pos);
        virtual ~Cell();

        void Set(std::string text);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool IsReferenced() const;

        bool HasCache() const;

        Position GetCellPosition() const;

private:
        class Impl;
        class EmptyImpl;
        class TextImpl;
        class FormulaImpl;

        std::unique_ptr<Impl> impl_;
        Sheet& belongs_to_;
        Position position_;

        std::set<Cell*> referencies_; //������, � ������� this ���������� � �������
        std::set<Cell*> dependable_; //������, ������� ��������� �� this

        std::optional<std::set<Cell*>> HasCircularDependenciesPos(const Impl& impl);
        
        mutable std::optional<Value> cache_;
        void InvalidateCache();

        void ClearReferences();
};