# Электронная таблица  
Упрощённый аналог листа таблицы Microsoft Excel или Google Sheets  
В ячейках таблицы могут быть текст или формулы. Формулы могут содержать индексы ячеек  
Проект использует ANTLR (https://www.antlr.org/) для лексического и синтаксического анализа — составления дерева  
[файл с грамматикой для ANTLR](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/Formula.g4)
### Общие структуры и интерфейсы
[common.h](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/common.h)  
[structures.cpp](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/structures.cpp)  
#### struct Position  
Индекс ячейки, нумерация с нуля. Методы:
- bool IsValid(): проверяет валидность позиции, то есть что ячейка (row, col) не выходит за ограничения и что значения полей row и col неотрицательны  
- std::string ToString(): возвращает строку — позицию в формате пользовательского индекса. Если позиция невалидна, метод возвращает пустую строку   
- static Position FromString(std::string_view str): возвращает позицию, соответствующую индексу, заданному в str. Если индекс задан в неверном формате — “abc”, “111”, “12jfd” — или выходит за предельные значения, он не валиден.  
#### class CellInterface  
Описывает интерфейс ячейки  
Методы:
- virtual void Set(std::string text): задаёт содержимое ячейки  
- virtual Value GetValue(): возвращает видимое значение ячейки  
- virtual std::string GetText(): возвращает внутренний текст ячейки, как если бы мы начали её редактирование  
#### class SheetInterface
Описывает интерфейс листа таблицы  
Методы:  
- virtual void SetCell(Position pos, std::string text): задаёт содержимое ячейки  
- virtual const CellInterface* GetCell(Position pos), virtual CellInterface* GetCell(Position pos): возвращают указатель на ячейку по позиции  
- virtual void ClearCell(Position pos): очищает ячейку  
- virtual Size GetPrintableSize(): возвращает размер печатной области  
- virtual void PrintValues(std::ostream& output): выводит значения ячеек в поток  
- virtual void PrintTexts(std::ostream& output): выводит текст ячеек в поток  
#### class FormulaError
Описывает ошибки, возможные при вычислении формул:
- Если ячейку, чей индекс входит в формулу, нельзя проинтерпретировать как число  
- Если делитель равен 0  
- Cсылка на ячейку, которая выходит за границы возможного размера таблицы  
Ошибки распространяются вверх по зависимостям  
#### Исключения
Если пользователь вызывает методы с некорректными аргументами, программа не меняет таблицу, а кидает исключения
- class InvalidPositionException - исключение, выбрасываемое при попытке передать в метод некорректную позицию  
- class FormulaException - исключение, выбрасываемое при попытке задать синтаксически некорректную формулу  
- class CircularDependencyException - исключение, выбрасываемое при попытке задать формулу, которая приводит к циклической зависимости между ячейками  
### Дерево для вычисления формул 
#### class FormulaAST
[FormulaAST.h](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/FormulaAST.h)  
[FormulaAST.cpp](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/FormulaAST.cpp)  
Класс представляет собой дерево. Узлы реализованы классом Expr. Само дерево задаётся указателем на его корень root_expr_. Узлы дерева могут содержать бинарные и унарные операции либо числа или индексы ячеек. Для них реализованы дочерние классы для класса Expr: BinaryOpExpr, UnaryOpExpr, NumberExpr, CellExpr. Узел содержит информацию о себе — вид операции, значение числа или позиция ячейки — и указатели на своих потомков. Метод Evaluate()вычисляет значение выражения в узле.  
Метод ParseFormulaAST(std::istream&) создаёт дерево из текста формулы. Он поочерёдно вызывает лексический и синтаксический анализаторы, а затем рекурсивно обходит дерево разбора и строит заготовку дерева для вычислений.
#### class Formula  
[formula.h](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/formula.h)  
[formula.cpp](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/formula.cpp)  
Класс-обёртка над FormulaAST, обрабатывает ошибки в формулах и обеспечивает устойчивую работу, даже если пользователь введёт синтаксически неверную формулу либо формулу, содержащую деление на 0  
### Ячейки  
[cell.h](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/cell.h)  
[cell.cpp](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/cell.cpp)  
#### class Cell  
Реализует CellInterface  
Ячейки могут трактоваться как текстовые и формульные. Тип зависит от текста, который пользователь задал в методе Cell::Set().
Тип существующей ячейки может меняться, если пользователь задал в ячейке новый текст методом Set(): формульная ячейка может стать текстовой и наоборот  
### Таблица
[sheet.h](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/sheet.h)  
[sheet.cpp](https://github.com/tatiana90st/cpp-spreadsheet/blob/main/spreadsheet/sheet.cpp)  
#### class Sheet
Реализует SheetInterface  
Таблица эффективна по памяти в случае, если таблица разрежена (минимальная печатная область большая, а непустых ячеек мало), не вызывает утечек памяти при удалении ячеек или таблицы целиком, предоставляет доступ к своим ячейкам по индексу Position за О(1). Повторный вызов метода Cell::GetValue(), если между вызовами значения ячеек, от которых данная ячейка зависит напрямую или опосредованно, не менялись — за O(1)  


