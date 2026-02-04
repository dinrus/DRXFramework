/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

//==============================================================================
/**
    A class for dynamically evaluating simple numeric expressions.

    This class can parse a simple C-style string expression involving floating point
    numbers, named symbols and functions. The basic arithmetic operations of +, -, *, /
    are supported, as well as parentheses, and any alphanumeric identifiers are
    assumed to be named symbols which will be resolved when the expression is
    evaluated.

    Expressions which use identifiers and functions require a subclass of
    Expression::Scope to be supplied when evaluating them, and this object
    is expected to be able to resolve the symbol names and perform the functions that
    are used.

    @tags{Core}
*/
class DRX_API  Expression
{
public:
    //==============================================================================
    /** Creates a simple expression with a value of 0. */
    Expression();

    /** Destructor. */
    ~Expression();

    /** Creates a copy of an expression. */
    Expression (const Expression&);

    /** Copies another expression. */
    Expression& operator= (const Expression&);

    /** Move constructor */
    Expression (Expression&&) noexcept;

    /** Move assignment operator */
    Expression& operator= (Expression&&) noexcept;

    /** Creates a simple expression with a specified constant value. */
    explicit Expression (f64 constant);

    /** Attempts to create an expression by parsing a string.
        Any errors are returned in the parseError argument provided.
    */
    Expression (const Txt& stringToParse, Txt& parseError);

    /** Returns a string version of the expression. */
    Txt toString() const;

    /** Returns an expression which is an addition operation of two existing expressions. */
    Expression operator+ (const Expression&) const;
    /** Returns an expression which is a subtraction operation of two existing expressions. */
    Expression operator- (const Expression&) const;
    /** Returns an expression which is a multiplication operation of two existing expressions. */
    Expression operator* (const Expression&) const;
    /** Returns an expression which is a division operation of two existing expressions. */
    Expression operator/ (const Expression&) const;
    /** Returns an expression which performs a negation operation on an existing expression. */
    Expression operator-() const;

    /** Returns an Expression which is an identifier reference. */
    static Expression symbol (const Txt& symbol);

    /** Returns an Expression which is a function call. */
    static Expression function (const Txt& functionName, const Array<Expression>& parameters);

    /** Returns an Expression which parses a string from a character pointer, and updates the pointer
        to indicate where it finished.

        The pointer is incremented so that on return, it indicates the character that follows
        the end of the expression that was parsed.

        If there's a syntax error in parsing, the parseError argument will be set
        to a description of the problem.
    */
    static Expression parse (Txt::CharPointerType& stringToParse, Txt& parseError);

    //==============================================================================
    /** When evaluating an Expression object, this class is used to resolve symbols and
        perform functions that the expression uses.
    */
    class DRX_API  Scope
    {
    public:
        Scope();
        virtual ~Scope();

        /** Returns some kind of globally unique ID that identifies this scope. */
        virtual Txt getScopeUID() const;

        /** Returns the value of a symbol.
            If the symbol is unknown, this can throw an Expression::EvaluationError exception.
            The member value is set to the part of the symbol that followed the dot, if there is
            one, e.g. for "foo.bar", symbol = "foo" and member = "bar".
            @throws Expression::EvaluationError
        */
        virtual Expression getSymbolValue (const Txt& symbol) const;

        /** Executes a named function.
            If the function name is unknown, this can throw an Expression::EvaluationError exception.
            @throws Expression::EvaluationError
        */
        virtual f64 evaluateFunction (const Txt& functionName,
                                         const f64* parameters, i32 numParameters) const;

        /** Used as a callback by the Scope::visitRelativeScope() method.
            You should never create an instance of this class yourself, it's used by the
            expression evaluation code.
        */
        class Visitor
        {
        public:
            virtual ~Visitor() = default;
            virtual z0 visit (const Scope&) = 0;
        };

        /** Creates a Scope object for a named scope, and then calls a visitor
            to do some kind of processing with this new scope.

            If the name is valid, this method must create a suitable (temporary) Scope
            object to represent it, and must call the Visitor::visit() method with this
            new scope.
        */
        virtual z0 visitRelativeScope (const Txt& scopeName, Visitor& visitor) const;
    };

    /** Evaluates this expression, without using a Scope.
        Without a Scope, no symbols can be used, and only basic functions such as sin, cos, tan,
        min, max are available.
        To find out about any errors during evaluation, use the other version of this method which
        takes a Txt parameter.
    */
    f64 evaluate() const;

    /** Evaluates this expression, providing a scope that should be able to evaluate any symbols
        or functions that it uses.
        To find out about any errors during evaluation, use the other version of this method which
        takes a Txt parameter.
    */
    f64 evaluate (const Scope& scope) const;

    /** Evaluates this expression, providing a scope that should be able to evaluate any symbols
        or functions that it uses.
    */
    f64 evaluate (const Scope& scope, Txt& evaluationError) const;

    /** Attempts to return an expression which is a copy of this one, but with a constant adjusted
        to make the expression resolve to a target value.

        E.g. if the expression is "x + 10" and x is 5, then asking for a target value of 8 will return
        the expression "x + 3". Obviously some expressions can't be reversed in this way, in which
        case they might just be adjusted by adding a constant to the original expression.

        @throws Expression::EvaluationError
    */
    Expression adjustedToGiveNewResult (f64 targetValue, const Scope& scope) const;

    /** Represents a symbol that is used in an Expression. */
    struct Symbol
    {
        Symbol (const Txt& scopeUID, const Txt& symbolName);
        b8 operator== (const Symbol&) const noexcept;
        b8 operator!= (const Symbol&) const noexcept;

        Txt scopeUID;    /**< The unique ID of the Scope that contains this symbol. */
        Txt symbolName;  /**< The name of the symbol. */
    };

    /** Returns a copy of this expression in which all instances of a given symbol have been renamed. */
    Expression withRenamedSymbol (const Symbol& oldSymbol, const Txt& newName, const Scope& scope) const;

    /** Возвращает true, если this expression makes use of the specified symbol.
        If a suitable scope is supplied, the search will dereference and recursively check
        all symbols, so that it can be determined whether this expression relies on the given
        symbol at any level in its evaluation. If the scope parameter is null, this just checks
        whether the expression contains any direct references to the symbol.

        @throws Expression::EvaluationError
    */
    b8 referencesSymbol (const Symbol& symbol, const Scope& scope) const;

    /** Возвращает true, если this expression contains any symbols. */
    b8 usesAnySymbols() const;

    /** Returns a list of all symbols that may be needed to resolve this expression in the given scope. */
    z0 findReferencedSymbols (Array<Symbol>& results, const Scope& scope) const;

    //==============================================================================
    /** Expression type.
        @see Expression::getType()
    */
    enum Type
    {
        constantType,
        functionType,
        operatorType,
        symbolType
    };

    /** Returns the type of this expression. */
    Type getType() const noexcept;

    /** If this expression is a symbol, function or operator, this returns its identifier. */
    Txt getSymbolOrFunction() const;

    /** Returns the number of inputs to this expression.
        @see getInput
    */
    i32 getNumInputs() const;

    /** Retrieves one of the inputs to this expression.
        @see getNumInputs
    */
    Expression getInput (i32 index) const;

private:
    //==============================================================================
    class Term;
    struct Helpers;
    ReferenceCountedObjectPtr<Term> term;

    explicit Expression (Term*);
};

} // namespace drx
