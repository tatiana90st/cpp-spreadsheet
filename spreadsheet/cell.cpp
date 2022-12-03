#include "cell.h"
#include "formula.h"

#include <stack>
#include <cassert>
#include <iostream>
#include <string>
#include <optional>


class Cell::Impl {
public:

	virtual ~Impl() = default;
	virtual Value GetValue(const SheetInterface& sheet) const = 0;
	virtual std::string GetText() const = 0;
	virtual std::vector<Position> GetReferencedCells() const {
		return{};
	}
};

class Cell::EmptyImpl final : public Impl {
	Value GetValue(const SheetInterface&) const override {
		return "";
	}
	std::string GetText() const override {
		return "";
	}
};

class Cell::TextImpl : public Impl {
public:
	TextImpl(std::string text)
		:text_(std::move(text))
	{
	}

	Value GetValue(const SheetInterface&) const override {
		if (text_[0] == ESCAPE_SIGN) {
			return text_.substr(1);
		}
		return text_;
	}

	std::string GetText() const override {
		return text_;
	}
private:
	std::string text_;

};

class Cell::FormulaImpl : public Impl {
public:
	FormulaImpl(std::string expression) {

		formula_ = ParseFormula(std::move(expression.substr(1)));
		
	}
	Value GetValue(const SheetInterface& sheet) const override {
		FormulaInterface::Value val =  formula_->Evaluate(sheet);
		if (std::holds_alternative<double>(val)) {
			return std::get<double>(val);
		}
		else {
			return std::get<FormulaError>(val);
		}
	}

	std::string GetText() const override {
		return FORMULA_SIGN + formula_->GetExpression();
	}

	std::vector<Position> GetReferencedCells() const override {
		return formula_->GetReferencedCells();
	}

private:
	std::unique_ptr<FormulaInterface> formula_;
};


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet, Position pos)
	:belongs_to_(sheet)
	,position_(pos) {}

Cell::~Cell() {}

//было бы удобно, если бы Set возвращал указатель на ячейку
void Cell::Set(std::string text) {
	//create new impl first
	std::unique_ptr<Impl> new_impl;
	if (text.empty()) {
		new_impl = std::make_unique<EmptyImpl>();
	}
	else if (text[0] == FORMULA_SIGN && text.size()>1) {
		new_impl = std::make_unique<FormulaImpl>(text);
	}
	else {
		new_impl = std::make_unique<TextImpl>(text);
	}
	
	std::optional<std::set<Cell*>> refers = HasCircularDependenciesPos(*new_impl);
	if (!refers) {
		throw CircularDependencyException("attempt to build circular dependency");
	}
	//if no exceptions happened, change impl_ to new
	impl_ = std::move(new_impl);
	
	cache_ = std::nullopt;
	if (!dependable_.empty()) {
		InvalidateCache();
	}

	//clear old reference info
	ClearReferences();

	//set new ref info
	referencies_ = std::move(refers.value());
	for (Cell* c : referencies_) {
		c->dependable_.insert(this);
	}

}


void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();

	//clear old reference info
	ClearReferences();
}

Cell::Value Cell::GetValue() const {
	if (!cache_) {
		cache_ = impl_->GetValue(belongs_to_);
	}
	return cache_.value();
}
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
	return !dependable_.empty();
}

bool Cell::HasCache() const {
	return cache_.has_value();
}

Position Cell::GetCellPosition() const {
	return position_;
}

void Cell::InvalidateCache() {
	std::stack<Cell*> recursion;
	recursion.push(this);
	std::set<Cell*> been_there;

	while (!recursion.empty()) {
		Cell* here = recursion.top();
		recursion.pop();
		here->cache_ = std::nullopt;
		been_there.insert(here);

		for (Cell* dep : here->dependable_) {
			if (!been_there.count(dep)) {
				recursion.push(dep);
			}
		}
	}

}

void Cell::ClearReferences() {
	if (!referencies_.empty()) {
		for (Cell* c : referencies_) {
			c->dependable_.erase(this);
		}
		referencies_.clear();
	}
}

std::optional<std::set<Cell*>> Cell::HasCircularDependenciesPos(const Impl& impl) {
	std::set<Position> referencies;
	std::set<Cell*> ref;
	for (Position p : impl.GetReferencedCells()) {
		auto* cell = belongs_to_.GetCellSpecifically(p);
		if (!cell) {
			belongs_to_.SetCell(p, "");
			cell = belongs_to_.GetCellSpecifically(p);
		}
		ref.insert(cell);
		referencies.insert(p);
	}

	if (ref.empty()) {
		//nothing to check
		return ref;
	}

	std::stack<Cell*> recursion;
	recursion.push(this);
	std::set<Cell*> been_there;

	while (!recursion.empty()) {

		Cell* here = recursion.top();
		recursion.pop();
		if (referencies.count(here->GetCellPosition())) {
			return std::nullopt;
		}
		been_there.insert(here);

		for (Cell* dep : here->dependable_) {
			if (!been_there.count(dep)) {
				recursion.push(dep);
			}
		}
	}

	return ref;
}

