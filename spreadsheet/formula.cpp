#include "formula.h"
#include "common.h"
#include "FormulaAST.h"
#include "cell.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <functional>

using namespace std::literals;

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) 
        try :ast_(ParseFormulaAST(std::move(expression)))
    {
        for (Position p : ast_.GetCells()) {
            referenced_.push_back(p);
        } 
        //std::sort(referenced_.begin(), referenced_.end()); - sorted in FormulaAST
        auto new_last = std::unique(referenced_.begin(), referenced_.end());
        auto new_size = new_last - referenced_.begin();
        referenced_.resize(new_size);

    }
    catch (std::exception& e) {
        throw FormulaException(e.what());
    }

    Value Evaluate(const SheetInterface& sheet) const override {

        std::function<double(Position)> get_args_values = [&sheet](Position pos)->double { 
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }

            const CellInterface* cell = sheet.GetCell(pos);
            if (!cell) {
                return 0;
            }

            CellInterface::Value val = cell->GetValue();
            if (std::holds_alternative<std::string>(val)) {
                double result = 0;
                std::string str_value = std::get<std::string>(val);
                if (str_value.empty()) {
                    return 0;
                }
                
                else {
                    std::istringstream in(str_value);
                    if (!(in >> result) || !in.eof()) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                    return result;
                }
            }
            if (std::holds_alternative<double>(val)) {
                return std::get<double>(val);
            }
            throw std::get<FormulaError>(val);
        };

        try {
            return ast_.Execute(get_args_values);
        }
        catch (FormulaError& e) {
            return e;
        }
    }
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }


    std::vector<Position> GetReferencedCells() const override {
        return referenced_;
    }

private:
    FormulaAST ast_;
    std::vector<Position> referenced_;
    
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {

        return std::make_unique<Formula>(std::move(expression));

}