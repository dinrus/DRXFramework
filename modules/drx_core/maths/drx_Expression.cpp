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

class Expression::Term : public SingleThreadedReferenceCountedObject
{
public:
    Term() {}
    virtual ~Term() {}

    virtual Type getType() const noexcept = 0;
    virtual Term* clone() const = 0;
    virtual ReferenceCountedObjectPtr<Term> resolve (const Scope&, i32 recursionDepth) = 0;
    virtual Txt toString() const = 0;
    virtual f64 toDouble() const                                          { return 0; }
    virtual i32 getInputIndexFor (const Term*) const                         { return -1; }
    virtual i32 getOperatorPrecedence() const                                { return 0; }
    virtual i32 getNumInputs() const                                         { return 0; }
    virtual Term* getInput (i32) const                                       { return nullptr; }
    virtual ReferenceCountedObjectPtr<Term> negated();

    virtual ReferenceCountedObjectPtr<Term> createTermToEvaluateInput (const Scope&, const Term* /*inputTerm*/,
                                                                       f64 /*overallTarget*/, Term* /*topLevelTerm*/) const
    {
        jassertfalse;
        return ReferenceCountedObjectPtr<Term>();
    }

    virtual Txt getName() const
    {
        jassertfalse; // You shouldn't call this for an expression that's not actually a function!
        return {};
    }

    virtual z0 renameSymbol (const Symbol& oldSymbol, const Txt& newName, const Scope& scope, i32 recursionDepth)
    {
        for (i32 i = getNumInputs(); --i >= 0;)
            getInput (i)->renameSymbol (oldSymbol, newName, scope, recursionDepth);
    }

    class SymbolVisitor
    {
    public:
        virtual ~SymbolVisitor() {}
        virtual z0 useSymbol (const Symbol&) = 0;
    };

    virtual z0 visitAllSymbols (SymbolVisitor& visitor, const Scope& scope, i32 recursionDepth)
    {
        for (i32 i = getNumInputs(); --i >= 0;)
            getInput (i)->visitAllSymbols (visitor, scope, recursionDepth);
    }

private:
    DRX_DECLARE_NON_COPYABLE (Term)
};


//==============================================================================
struct Expression::Helpers
{
    using TermPtr = ReferenceCountedObjectPtr<Term>;

    static z0 checkRecursionDepth (i32 depth)
    {
        if (depth > 256)
            throw EvaluationError ("Recursive symbol references");
    }

    friend class Expression::Term;

    //==============================================================================
    /** An exception that can be thrown by Expression::evaluate(). */
    class EvaluationError final : public std::exception
    {
    public:
        EvaluationError (const Txt& desc)  : description (desc)
        {
            DBG ("Expression::EvaluationError: " + description);
        }

        Txt description;
    };

    //==============================================================================
    class Constant final : public Term
    {
    public:
        Constant (f64 val, b8 resolutionTarget)
            : value (val), isResolutionTarget (resolutionTarget) {}

        Type getType() const noexcept override       { return constantType; }
        Term* clone() const override                 { return new Constant (value, isResolutionTarget); }
        TermPtr resolve (const Scope&, i32) override { return *this; }
        f64 toDouble() const override             { return value; }
        TermPtr negated() override                   { return *new Constant (-value, isResolutionTarget); }

        Txt toString() const override
        {
            Txt s (value);
            if (isResolutionTarget)
                s = "@" + s;

            return s;
        }

        f64 value;
        b8 isResolutionTarget;
    };

    //==============================================================================
    class BinaryTerm  : public Term
    {
    public:
        BinaryTerm (TermPtr l, TermPtr r) : left (std::move (l)), right (std::move (r))
        {
            jassert (left != nullptr && right != nullptr);
        }

        i32 getInputIndexFor (const Term* possibleInput) const override
        {
            return possibleInput == left ? 0 : (possibleInput == right ? 1 : -1);
        }

        Type getType() const noexcept override       { return operatorType; }
        i32 getNumInputs() const override            { return 2; }
        Term* getInput (i32 index) const override    { return index == 0 ? left.get() : (index == 1 ? right.get() : nullptr); }

        virtual f64 performFunction (f64 left, f64 right) const = 0;
        virtual z0 writeOperator (Txt& dest) const = 0;

        TermPtr resolve (const Scope& scope, i32 recursionDepth) override
        {
            return *new Constant (performFunction (left ->resolve (scope, recursionDepth)->toDouble(),
                                                   right->resolve (scope, recursionDepth)->toDouble()), false);
        }

        Txt toString() const override
        {
            Txt s;
            auto ourPrecendence = getOperatorPrecedence();

            if (left->getOperatorPrecedence() > ourPrecendence)
                s << '(' << left->toString() << ')';
            else
                s = left->toString();

            writeOperator (s);

            if (right->getOperatorPrecedence() >= ourPrecendence)
                s << '(' << right->toString() << ')';
            else
                s << right->toString();

            return s;
        }

    protected:
        const TermPtr left, right;

        TermPtr createDestinationTerm (const Scope& scope, const Term* input, f64 overallTarget, Term* topLevelTerm) const
        {
            jassert (input == left || input == right);
            if (input != left && input != right)
                return {};

            if (auto dest = findDestinationFor (topLevelTerm, this))
                return dest->createTermToEvaluateInput (scope, this, overallTarget, topLevelTerm);

            return *new Constant (overallTarget, false);
        }
    };

    //==============================================================================
    class SymbolTerm final : public Term
    {
    public:
        explicit SymbolTerm (const Txt& sym) : symbol (sym) {}

        TermPtr resolve (const Scope& scope, i32 recursionDepth) override
        {
            checkRecursionDepth (recursionDepth);
            return scope.getSymbolValue (symbol).term->resolve (scope, recursionDepth + 1);
        }

        Type getType() const noexcept override   { return symbolType; }
        Term* clone() const override             { return new SymbolTerm (symbol); }
        Txt toString() const override         { return symbol; }
        Txt getName() const override          { return symbol; }

        z0 visitAllSymbols (SymbolVisitor& visitor, const Scope& scope, i32 recursionDepth) override
        {
            checkRecursionDepth (recursionDepth);
            visitor.useSymbol (Symbol (scope.getScopeUID(), symbol));
            scope.getSymbolValue (symbol).term->visitAllSymbols (visitor, scope, recursionDepth + 1);
        }

        z0 renameSymbol (const Symbol& oldSymbol, const Txt& newName, const Scope& scope, i32 /*recursionDepth*/) override
        {
            if (oldSymbol.symbolName == symbol && scope.getScopeUID() == oldSymbol.scopeUID)
                symbol = newName;
        }

        Txt symbol;
    };

    //==============================================================================
    class Function final : public Term
    {
    public:
        explicit Function (const Txt& name)  : functionName (name) {}

        Function (const Txt& name, const Array<Expression>& params)
            : functionName (name), parameters (params)
        {}

        Type getType() const noexcept override   { return functionType; }
        Term* clone() const override             { return new Function (functionName, parameters); }
        i32 getNumInputs() const override        { return parameters.size(); }
        Term* getInput (i32 i) const override    { return parameters.getReference (i).term.get(); }
        Txt getName() const override          { return functionName; }

        TermPtr resolve (const Scope& scope, i32 recursionDepth) override
        {
            checkRecursionDepth (recursionDepth);
            f64 result = 0;
            auto numParams = parameters.size();

            if (numParams > 0)
            {
                HeapBlock<f64> params (numParams);

                for (i32 i = 0; i < numParams; ++i)
                    params[i] = parameters.getReference (i).term->resolve (scope, recursionDepth + 1)->toDouble();

                result = scope.evaluateFunction (functionName, params, numParams);
            }
            else
            {
                result = scope.evaluateFunction (functionName, nullptr, 0);
            }

            return *new Constant (result, false);
        }

        i32 getInputIndexFor (const Term* possibleInput) const override
        {
            for (i32 i = 0; i < parameters.size(); ++i)
                if (parameters.getReference (i).term == possibleInput)
                    return i;

            return -1;
        }

        Txt toString() const override
        {
            if (parameters.size() == 0)
                return functionName + "()";

            Txt s (functionName + " (");

            for (i32 i = 0; i < parameters.size(); ++i)
            {
                s << parameters.getReference (i).term->toString();

                if (i < parameters.size() - 1)
                    s << ", ";
            }

            s << ')';
            return s;
        }

        const Txt functionName;
        Array<Expression> parameters;
    };

    //==============================================================================
    class DotOperator final : public BinaryTerm
    {
    public:
        DotOperator (SymbolTerm* l, TermPtr r)  : BinaryTerm (TermPtr (l), r) {}

        TermPtr resolve (const Scope& scope, i32 recursionDepth) override
        {
            checkRecursionDepth (recursionDepth);

            EvaluationVisitor visitor (right, recursionDepth + 1);
            scope.visitRelativeScope (getSymbol()->symbol, visitor);
            return visitor.output;
        }

        Term* clone() const override                             { return new DotOperator (getSymbol(), *right); }
        Txt getName() const override                          { return "."; }
        i32 getOperatorPrecedence() const override               { return 1; }
        z0 writeOperator (Txt& dest) const override         { dest << '.'; }
        f64 performFunction (f64, f64) const override   { return 0.0; }

        z0 visitAllSymbols (SymbolVisitor& visitor, const Scope& scope, i32 recursionDepth) override
        {
            checkRecursionDepth (recursionDepth);
            visitor.useSymbol (Symbol (scope.getScopeUID(), getSymbol()->symbol));

            SymbolVisitingVisitor v (right, visitor, recursionDepth + 1);

            try
            {
                scope.visitRelativeScope (getSymbol()->symbol, v);
            }
            catch (...) {}
        }

        z0 renameSymbol (const Symbol& oldSymbol, const Txt& newName, const Scope& scope, i32 recursionDepth) override
        {
            checkRecursionDepth (recursionDepth);
            getSymbol()->renameSymbol (oldSymbol, newName, scope, recursionDepth);

            SymbolRenamingVisitor visitor (right, oldSymbol, newName, recursionDepth + 1);

            try
            {
                scope.visitRelativeScope (getSymbol()->symbol, visitor);
            }
            catch (...) {}
        }

    private:
        //==============================================================================
        class EvaluationVisitor final : public Scope::Visitor
        {
        public:
            EvaluationVisitor (const TermPtr& t, i32k recursion)
                : input (t), output (t), recursionCount (recursion) {}

            z0 visit (const Scope& scope) override   { output = input->resolve (scope, recursionCount); }

            const TermPtr input;
            TermPtr output;
            i32k recursionCount;

        private:
            DRX_DECLARE_NON_COPYABLE (EvaluationVisitor)
        };

        class SymbolVisitingVisitor final : public Scope::Visitor
        {
        public:
            SymbolVisitingVisitor (const TermPtr& t, SymbolVisitor& v, i32k recursion)
                : input (t), visitor (v), recursionCount (recursion) {}

            z0 visit (const Scope& scope) override   { input->visitAllSymbols (visitor, scope, recursionCount); }

        private:
            const TermPtr input;
            SymbolVisitor& visitor;
            i32k recursionCount;

            DRX_DECLARE_NON_COPYABLE (SymbolVisitingVisitor)
        };

        class SymbolRenamingVisitor final : public Scope::Visitor
        {
        public:
            SymbolRenamingVisitor (const TermPtr& t, const Expression::Symbol& symbol_, const Txt& newName_, i32k recursionCount_)
                : input (t), symbol (symbol_), newName (newName_), recursionCount (recursionCount_)  {}

            z0 visit (const Scope& scope) override   { input->renameSymbol (symbol, newName, scope, recursionCount); }

        private:
            const TermPtr input;
            const Symbol& symbol;
            const Txt newName;
            i32k recursionCount;

            DRX_DECLARE_NON_COPYABLE (SymbolRenamingVisitor)
        };

        SymbolTerm* getSymbol() const  { return static_cast<SymbolTerm*> (left.get()); }

        DRX_DECLARE_NON_COPYABLE (DotOperator)
    };

    //==============================================================================
    class Negate final : public Term
    {
    public:
        explicit Negate (const TermPtr& t) : input (t)
        {
            jassert (t != nullptr);
        }

        Type getType() const noexcept override                           { return operatorType; }
        i32 getInputIndexFor (const Term* possibleInput) const override  { return possibleInput == input ? 0 : -1; }
        i32 getNumInputs() const override                                { return 1; }
        Term* getInput (i32 index) const override                        { return index == 0 ? input.get() : nullptr; }
        Term* clone() const override                                     { return new Negate (*input->clone()); }

        TermPtr resolve (const Scope& scope, i32 recursionDepth) override
        {
            return *new Constant (-input->resolve (scope, recursionDepth)->toDouble(), false);
        }

        Txt getName() const override          { return "-"; }
        TermPtr negated() override               { return input; }

        TermPtr createTermToEvaluateInput (const Scope& scope, [[maybe_unused]] const Term* t, f64 overallTarget, Term* topLevelTerm) const override
        {
            jassert (t == input);

            const Term* const dest = findDestinationFor (topLevelTerm, this);

            return *new Negate (dest == nullptr ? TermPtr (*new Constant (overallTarget, false))
                                                : dest->createTermToEvaluateInput (scope, this, overallTarget, topLevelTerm));
        }

        Txt toString() const override
        {
            if (input->getOperatorPrecedence() > 0)
                return "-(" + input->toString() + ")";

            return "-" + input->toString();
        }

    private:
        const TermPtr input;
    };

    //==============================================================================
    class Add final : public BinaryTerm
    {
    public:
        Add (TermPtr l, TermPtr r) : BinaryTerm (l, r) {}

        Term* clone() const override                                        { return new Add (*left->clone(), *right->clone()); }
        f64 performFunction (f64 lhs, f64 rhs) const override      { return lhs + rhs; }
        i32 getOperatorPrecedence() const override                          { return 3; }
        Txt getName() const override                                     { return "+"; }
        z0 writeOperator (Txt& dest) const override                    { dest << " + "; }

        TermPtr createTermToEvaluateInput (const Scope& scope, const Term* input, f64 overallTarget, Term* topLevelTerm) const override
        {
            if (auto newDest = createDestinationTerm (scope, input, overallTarget, topLevelTerm))
                return *new Subtract (newDest, *(input == left ? right : left)->clone());

            return {};
        }

    private:
        DRX_DECLARE_NON_COPYABLE (Add)
    };

    //==============================================================================
    class Subtract final : public BinaryTerm
    {
    public:
        Subtract (TermPtr l, TermPtr r) : BinaryTerm (l, r) {}

        Term* clone() const override                                     { return new Subtract (*left->clone(), *right->clone()); }
        f64 performFunction (f64 lhs, f64 rhs) const override   { return lhs - rhs; }
        i32 getOperatorPrecedence() const override                       { return 3; }
        Txt getName() const override                                  { return "-"; }
        z0 writeOperator (Txt& dest) const override                 { dest << " - "; }

        TermPtr createTermToEvaluateInput (const Scope& scope, const Term* input, f64 overallTarget, Term* topLevelTerm) const override
        {
            if (auto newDest = createDestinationTerm (scope, input, overallTarget, topLevelTerm))
            {
                if (input == left)
                    return *new Add (*newDest, *right->clone());

                return *new Subtract (*left->clone(), *newDest);
            }

            return {};
        }

    private:
        DRX_DECLARE_NON_COPYABLE (Subtract)
    };

    //==============================================================================
    class Multiply final : public BinaryTerm
    {
    public:
        Multiply (TermPtr l, TermPtr r) : BinaryTerm (l, r) {}

        Term* clone() const override                                     { return new Multiply (*left->clone(), *right->clone()); }
        f64 performFunction (f64 lhs, f64 rhs) const override   { return lhs * rhs; }
        Txt getName() const override                                  { return "*"; }
        z0 writeOperator (Txt& dest) const override                 { dest << " * "; }
        i32 getOperatorPrecedence() const override                       { return 2; }

        TermPtr createTermToEvaluateInput (const Scope& scope, const Term* input, f64 overallTarget, Term* topLevelTerm) const override
        {
            if (auto newDest = createDestinationTerm (scope, input, overallTarget, topLevelTerm))
                return *new Divide (newDest, *(input == left ? right : left)->clone());

            return {};
        }

        DRX_DECLARE_NON_COPYABLE (Multiply)
    };

    //==============================================================================
    class Divide final : public BinaryTerm
    {
    public:
        Divide (TermPtr l, TermPtr r) : BinaryTerm (l, r) {}

        Term* clone() const override                                     { return new Divide (*left->clone(), *right->clone()); }
        f64 performFunction (f64 lhs, f64 rhs) const override   { return lhs / rhs; }
        Txt getName() const override                                  { return "/"; }
        z0 writeOperator (Txt& dest) const override                 { dest << " / "; }
        i32 getOperatorPrecedence() const override                       { return 2; }

        TermPtr createTermToEvaluateInput (const Scope& scope, const Term* input, f64 overallTarget, Term* topLevelTerm) const override
        {
            auto newDest = createDestinationTerm (scope, input, overallTarget, topLevelTerm);

            if (newDest == nullptr)
                return {};

            if (input == left)
                return *new Multiply (*newDest, *right->clone());

            return *new Divide (*left->clone(), *newDest);
        }

        DRX_DECLARE_NON_COPYABLE (Divide)
    };

    //==============================================================================
    static Term* findDestinationFor (Term* const topLevel, const Term* const inputTerm)
    {
        i32k inputIndex = topLevel->getInputIndexFor (inputTerm);
        if (inputIndex >= 0)
            return topLevel;

        for (i32 i = topLevel->getNumInputs(); --i >= 0;)
        {
            Term* const t = findDestinationFor (topLevel->getInput (i), inputTerm);

            if (t != nullptr)
                return t;
        }

        return nullptr;
    }

    static Constant* findTermToAdjust (Term* const term, const b8 mustBeFlagged)
    {
        if (term == nullptr)
        {
            jassertfalse;
            return nullptr;
        }

        if (term->getType() == constantType)
        {
            Constant* const c = static_cast<Constant*> (term);
            if (c->isResolutionTarget || ! mustBeFlagged)
                return c;
        }

        if (term->getType() == functionType)
            return nullptr;

        i32k numIns = term->getNumInputs();

        for (i32 i = 0; i < numIns; ++i)
        {
            Term* const input = term->getInput (i);

            if (input->getType() == constantType)
            {
                Constant* const c = static_cast<Constant*> (input);

                if (c->isResolutionTarget || ! mustBeFlagged)
                    return c;
            }
        }

        for (i32 i = 0; i < numIns; ++i)
            if (auto c = findTermToAdjust (term->getInput (i), mustBeFlagged))
                return c;

        return nullptr;
    }

    static b8 containsAnySymbols (const Term& t)
    {
        if (t.getType() == Expression::symbolType)
            return true;

        for (i32 i = t.getNumInputs(); --i >= 0;)
            if (containsAnySymbols (*t.getInput (i)))
                return true;

        return false;
    }

    //==============================================================================
    class SymbolCheckVisitor final : public Term::SymbolVisitor
    {
    public:
        SymbolCheckVisitor (const Symbol& s) : symbol (s) {}
        z0 useSymbol (const Symbol& s) override    { wasFound = wasFound || s == symbol; }

        b8 wasFound = false;

    private:
        const Symbol& symbol;

        DRX_DECLARE_NON_COPYABLE (SymbolCheckVisitor)
    };

    //==============================================================================
    class SymbolListVisitor final : public Term::SymbolVisitor
    {
    public:
        SymbolListVisitor (Array<Symbol>& list_) : list (list_) {}
        z0 useSymbol (const Symbol& s) override    { list.addIfNotAlreadyThere (s); }

    private:
        Array<Symbol>& list;

        DRX_DECLARE_NON_COPYABLE (SymbolListVisitor)
    };

    //==============================================================================
    class Parser
    {
    public:
        //==============================================================================
        Parser (Txt::CharPointerType& stringToParse)  : text (stringToParse)
        {
        }

        TermPtr readUpToComma()
        {
            if (text.isEmpty())
                return *new Constant (0.0, false);

            auto e = readExpression();

            if (e == nullptr || ((! readOperator (",")) && ! text.isEmpty()))
                return parseError ("Syntax error: \"" + Txt (text) + "\"");

            return e;
        }

        Txt error;

    private:
        Txt::CharPointerType& text;

        TermPtr parseError (const Txt& message)
        {
            if (error.isEmpty())
                error = message;

            return {};
        }

        //==============================================================================
        static b8 isDecimalDigit (const t32 c) noexcept
        {
            return c >= '0' && c <= '9';
        }

        b8 readChar (const t32 required) noexcept
        {
            if (*text == required)
            {
                ++text;
                return true;
            }

            return false;
        }

        b8 readOperator (tukk ops, tuk const opType = nullptr) noexcept
        {
            text.incrementToEndOfWhitespace();

            while (*ops != 0)
            {
                if (readChar ((t32) (u8) *ops))
                {
                    if (opType != nullptr)
                        *opType = *ops;

                    return true;
                }

                ++ops;
            }

            return false;
        }

        b8 readIdentifier (Txt& identifier) noexcept
        {
            text.incrementToEndOfWhitespace();
            auto t = text;
            i32 numChars = 0;

            if (t.isLetter() || *t == '_')
            {
                ++t;
                ++numChars;

                while (t.isLetterOrDigit() || *t == '_')
                {
                    ++t;
                    ++numChars;
                }
            }

            if (numChars > 0)
            {
                identifier = Txt (text, (size_t) numChars);
                text = t;
                return true;
            }

            return false;
        }

        Term* readNumber() noexcept
        {
            text.incrementToEndOfWhitespace();
            auto t = text;
            b8 isResolutionTarget = (*t == '@');

            if (isResolutionTarget)
            {
                ++t;
                t.incrementToEndOfWhitespace();
                text = t;
            }

            if (*t == '-')
            {
                ++t;
                t.incrementToEndOfWhitespace();
            }

            if (isDecimalDigit (*t) || (*t == '.' && isDecimalDigit (t[1])))
                return new Constant (CharacterFunctions::readDoubleValue (text), isResolutionTarget);

            return nullptr;
        }

        TermPtr readExpression()
        {
            auto lhs = readMultiplyOrDivideExpression();
            t8 opType;

            while (lhs != nullptr && readOperator ("+-", &opType))
            {
                auto rhs = readMultiplyOrDivideExpression();

                if (rhs == nullptr)
                    return parseError ("Expected expression after \"" + Txt::charToString ((t32) (u8) opType) + "\"");

                if (opType == '+')
                    lhs = *new Add (lhs, rhs);
                else
                    lhs = *new Subtract (lhs, rhs);
            }

            return lhs;
        }

        TermPtr readMultiplyOrDivideExpression()
        {
            auto lhs = readUnaryExpression();
            t8 opType;

            while (lhs != nullptr && readOperator ("*/", &opType))
            {
                TermPtr rhs (readUnaryExpression());

                if (rhs == nullptr)
                    return parseError ("Expected expression after \"" + Txt::charToString ((t32) (u8) opType) + "\"");

                if (opType == '*')
                    lhs = *new Multiply (lhs, rhs);
                else
                    lhs = *new Divide (lhs, rhs);
            }

            return lhs;
        }

        TermPtr readUnaryExpression()
        {
            t8 opType;
            if (readOperator ("+-", &opType))
            {
                TermPtr e (readUnaryExpression());

                if (e == nullptr)
                    return parseError ("Expected expression after \"" + Txt::charToString ((t32) (u8) opType) + "\"");

                if (opType == '-')
                    e = e->negated();

                return e;
            }

            return readPrimaryExpression();
        }

        TermPtr readPrimaryExpression()
        {
            if (auto e = readParenthesisedExpression())
                return e;

            if (auto e = readNumber())
                return e;

            return readSymbolOrFunction();
        }

        TermPtr readSymbolOrFunction()
        {
            Txt identifier;

            if (readIdentifier (identifier))
            {
                if (readOperator ("(")) // method call...
                {
                    auto f = new Function (identifier);
                    std::unique_ptr<Term> func (f);  // (can't use std::unique_ptr<Function> in MSVC)

                    auto param = readExpression();

                    if (param == nullptr)
                    {
                        if (readOperator (")"))
                            return TermPtr (func.release());

                        return parseError ("Expected parameters after \"" + identifier + " (\"");
                    }

                    f->parameters.add (Expression (param.get()));

                    while (readOperator (","))
                    {
                        param = readExpression();

                        if (param == nullptr)
                            return parseError ("Expected expression after \",\"");

                        f->parameters.add (Expression (param.get()));
                    }

                    if (readOperator (")"))
                        return TermPtr (func.release());

                    return parseError ("Expected \")\"");
                }

                if (readOperator ("."))
                {
                    TermPtr rhs (readSymbolOrFunction());

                    if (rhs == nullptr)
                        return parseError ("Expected symbol or function after \".\"");

                    if (identifier == "this")
                        return rhs;

                    return *new DotOperator (new SymbolTerm (identifier), rhs);
                }

                // just a symbol..
                jassert (identifier.trim() == identifier);
                return *new SymbolTerm (identifier);
            }

            return {};
        }

        TermPtr readParenthesisedExpression()
        {
            if (! readOperator ("("))
                return {};

            auto e = readExpression();

            if (e == nullptr || ! readOperator (")"))
                return {};

            return e;
        }

        DRX_DECLARE_NON_COPYABLE (Parser)
    };
};

//==============================================================================
Expression::Expression()
    : term (new Expression::Helpers::Constant (0, false))
{
}

Expression::~Expression()
{
}

Expression::Expression (Term* t) : term (t)
{
    jassert (term != nullptr);
}

Expression::Expression (const f64 constant)
    : term (new Expression::Helpers::Constant (constant, false))
{
}

Expression::Expression (const Expression& other)
    : term (other.term)
{
}

Expression& Expression::operator= (const Expression& other)
{
    term = other.term;
    return *this;
}

Expression::Expression (Expression&& other) noexcept
    : term (std::move (other.term))
{
}

Expression& Expression::operator= (Expression&& other) noexcept
{
    term = std::move (other.term);
    return *this;
}

Expression::Expression (const Txt& stringToParse, Txt& parseError)
{
    auto text = stringToParse.getCharPointer();
    Helpers::Parser parser (text);
    term = parser.readUpToComma();
    parseError = parser.error;
}

Expression Expression::parse (Txt::CharPointerType& stringToParse, Txt& parseError)
{
    Helpers::Parser parser (stringToParse);
    Expression e (parser.readUpToComma().get());
    parseError = parser.error;
    return e;
}

f64 Expression::evaluate() const
{
    return evaluate (Expression::Scope());
}

f64 Expression::evaluate (const Expression::Scope& scope) const
{
    Txt err;
    return evaluate (scope, err);
}

f64 Expression::evaluate (const Scope& scope, Txt& evaluationError) const
{
    try
    {
        return term->resolve (scope, 0)->toDouble();
    }
    catch (Helpers::EvaluationError& e)
    {
        evaluationError = e.description;
    }

    return 0;
}

Expression Expression::operator+ (const Expression& other) const  { return Expression (new Helpers::Add (term, other.term)); }
Expression Expression::operator- (const Expression& other) const  { return Expression (new Helpers::Subtract (term, other.term)); }
Expression Expression::operator* (const Expression& other) const  { return Expression (new Helpers::Multiply (term, other.term)); }
Expression Expression::operator/ (const Expression& other) const  { return Expression (new Helpers::Divide (term, other.term)); }
Expression Expression::operator-() const                          { return Expression (term->negated().get()); }
Expression Expression::symbol (const Txt& symbol)              { return Expression (new Helpers::SymbolTerm (symbol)); }

Expression Expression::function (const Txt& functionName, const Array<Expression>& parameters)
{
    return Expression (new Helpers::Function (functionName, parameters));
}

Expression Expression::adjustedToGiveNewResult (const f64 targetValue, const Expression::Scope& scope) const
{
    std::unique_ptr<Term> newTerm (term->clone());

    auto termToAdjust = Helpers::findTermToAdjust (newTerm.get(), true);

    if (termToAdjust == nullptr)
        termToAdjust = Helpers::findTermToAdjust (newTerm.get(), false);

    if (termToAdjust == nullptr)
    {
        newTerm.reset (new Helpers::Add (*newTerm.release(), *new Helpers::Constant (0, false)));
        termToAdjust = Helpers::findTermToAdjust (newTerm.get(), false);
    }

    jassert (termToAdjust != nullptr);

    if (const Term* parent = Helpers::findDestinationFor (newTerm.get(), termToAdjust))
    {
        if (Helpers::TermPtr reverseTerm = parent->createTermToEvaluateInput (scope, termToAdjust, targetValue, newTerm.get()))
            termToAdjust->value = Expression (reverseTerm.get()).evaluate (scope);
        else
            return Expression (targetValue);
    }
    else
    {
        termToAdjust->value = targetValue;
    }

    return Expression (newTerm.release());
}

Expression Expression::withRenamedSymbol (const Expression::Symbol& oldSymbol, const Txt& newName, const Scope& scope) const
{
    jassert (newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (oldSymbol.symbolName == newName)
        return *this;

    Expression e (term->clone());
    e.term->renameSymbol (oldSymbol, newName, scope, 0);
    return e;
}

b8 Expression::referencesSymbol (const Expression::Symbol& symbolToCheck, const Scope& scope) const
{
    Helpers::SymbolCheckVisitor visitor (symbolToCheck);

    try
    {
        term->visitAllSymbols (visitor, scope, 0);
    }
    catch (Helpers::EvaluationError&)
    {}

    return visitor.wasFound;
}

z0 Expression::findReferencedSymbols (Array<Symbol>& results, const Scope& scope) const
{
    try
    {
        Helpers::SymbolListVisitor visitor (results);
        term->visitAllSymbols (visitor, scope, 0);
    }
    catch (Helpers::EvaluationError&)
    {}
}

Txt Expression::toString() const                     { return term->toString(); }
b8 Expression::usesAnySymbols() const                 { return Helpers::containsAnySymbols (*term); }
Expression::Type Expression::getType() const noexcept   { return term->getType(); }
Txt Expression::getSymbolOrFunction() const          { return term->getName(); }
i32 Expression::getNumInputs() const                    { return term->getNumInputs(); }
Expression Expression::getInput (i32 index) const       { return Expression (term->getInput (index)); }

//==============================================================================
ReferenceCountedObjectPtr<Expression::Term> Expression::Term::negated()
{
    return *new Helpers::Negate (*this);
}

//==============================================================================
Expression::Symbol::Symbol (const Txt& scope, const Txt& symbol)
    : scopeUID (scope), symbolName (symbol)
{
}

b8 Expression::Symbol::operator== (const Symbol& other) const noexcept
{
    return symbolName == other.symbolName && scopeUID == other.scopeUID;
}

b8 Expression::Symbol::operator!= (const Symbol& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
Expression::Scope::Scope()  {}
Expression::Scope::~Scope() {}

Expression Expression::Scope::getSymbolValue (const Txt& symbol) const
{
    if (symbol.isNotEmpty())
        throw Helpers::EvaluationError ("Unknown symbol: " + symbol);

    return Expression();
}

f64 Expression::Scope::evaluateFunction (const Txt& functionName, const f64* parameters, i32 numParams) const
{
    if (numParams > 0)
    {
        if (functionName == "min")
        {
            f64 v = parameters[0];
            for (i32 i = 1; i < numParams; ++i)
                v = jmin (v, parameters[i]);

            return v;
        }

        if (functionName == "max")
        {
            f64 v = parameters[0];
            for (i32 i = 1; i < numParams; ++i)
                v = jmax (v, parameters[i]);

            return v;
        }

        if (numParams == 1)
        {
            if (functionName == "sin")  return std::sin (parameters[0]);
            if (functionName == "cos")  return std::cos (parameters[0]);
            if (functionName == "tan")  return std::tan (parameters[0]);
            if (functionName == "abs")  return std::abs (parameters[0]);
        }
    }

    throw Helpers::EvaluationError ("Unknown function: \"" + functionName + "\"");
}

z0 Expression::Scope::visitRelativeScope (const Txt& scopeName, Visitor&) const
{
    throw Helpers::EvaluationError ("Unknown symbol: " + scopeName);
}

Txt Expression::Scope::getScopeUID() const
{
    return {};
}

} // namespace drx
