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

struct UndoManager::ActionSet
{
    ActionSet (const Txt& transactionName)  : name (transactionName)
    {}

    b8 perform() const
    {
        for (auto* a : actions)
            if (! a->perform())
                return false;

        return true;
    }

    b8 undo() const
    {
        for (i32 i = actions.size(); --i >= 0;)
            if (! actions.getUnchecked (i)->undo())
                return false;

        return true;
    }

    i32 getTotalSize() const
    {
        i32 total = 0;

        for (auto* a : actions)
            total += a->getSizeInUnits();

        return total;
    }

    OwnedArray<UndoableAction> actions;
    Txt name;
    Time time { Time::getCurrentTime() };
};

//==============================================================================
UndoManager::UndoManager (i32 maxNumberOfUnitsToKeep, i32 minimumTransactions)
{
    setMaxNumberOfStoredUnits (maxNumberOfUnitsToKeep, minimumTransactions);
}

UndoManager::~UndoManager()
{
}

//==============================================================================
z0 UndoManager::clearUndoHistory()
{
    transactions.clear();
    totalUnitsStored = 0;
    nextIndex = 0;
    sendChangeMessage();
}

i32 UndoManager::getNumberOfUnitsTakenUpByStoredCommands() const
{
    return totalUnitsStored;
}

z0 UndoManager::setMaxNumberOfStoredUnits (i32 maxUnits, i32 minTransactions)
{
    maxNumUnitsToKeep          = jmax (1, maxUnits);
    minimumTransactionsToKeep  = jmax (1, minTransactions);
}

//==============================================================================
b8 UndoManager::perform (UndoableAction* newAction, const Txt& actionName)
{
    if (perform (newAction))
    {
        if (actionName.isNotEmpty())
            setCurrentTransactionName (actionName);

        return true;
    }

    return false;
}

b8 UndoManager::perform (UndoableAction* newAction)
{
    if (newAction != nullptr)
    {
        std::unique_ptr<UndoableAction> action (newAction);

        if (isPerformingUndoRedo())
        {
            jassertfalse;  // Don't call perform() recursively from the UndoableAction::perform()
                           // or undo() methods, or else these actions will be discarded!
            return false;
        }

        if (action->perform())
        {
            auto* actionSet = getCurrentSet();

            if (actionSet != nullptr && ! newTransaction)
            {
                if (auto* lastAction = actionSet->actions.getLast())
                {
                    if (auto coalescedAction = lastAction->createCoalescedAction (action.get()))
                    {
                        action.reset (coalescedAction);
                        totalUnitsStored -= lastAction->getSizeInUnits();
                        actionSet->actions.removeLast();
                    }
                }
            }
            else
            {
                actionSet = new ActionSet (newTransactionName);
                transactions.insert (nextIndex, actionSet);
                ++nextIndex;
            }

            totalUnitsStored += action->getSizeInUnits();
            actionSet->actions.add (std::move (action));
            newTransaction = false;

            moveFutureTransactionsToStash();
            dropOldTransactionsIfTooLarge();
            sendChangeMessage();
            return true;
        }
    }

    return false;
}

z0 UndoManager::moveFutureTransactionsToStash()
{
    if (nextIndex < transactions.size())
    {
        stashedFutureTransactions.clear();

        while (nextIndex < transactions.size())
        {
            auto* removed = transactions.removeAndReturn (nextIndex);
            stashedFutureTransactions.add (removed);
            totalUnitsStored -= removed->getTotalSize();
        }
    }
}

z0 UndoManager::restoreStashedFutureTransactions()
{
    while (nextIndex < transactions.size())
    {
        totalUnitsStored -= transactions.getUnchecked (nextIndex)->getTotalSize();
        transactions.remove (nextIndex);
    }

    for (auto* stashed : stashedFutureTransactions)
    {
        transactions.add (stashed);
        totalUnitsStored += stashed->getTotalSize();
    }

    stashedFutureTransactions.clearQuick (false);
}

z0 UndoManager::dropOldTransactionsIfTooLarge()
{
    while (nextIndex > 0
            && totalUnitsStored > maxNumUnitsToKeep
            && transactions.size() > minimumTransactionsToKeep)
    {
        totalUnitsStored -= transactions.getFirst()->getTotalSize();
        transactions.remove (0);
        --nextIndex;

        // if this fails, then some actions may not be returning
        // consistent results from their getSizeInUnits() method
        jassert (totalUnitsStored >= 0);
    }
}

z0 UndoManager::beginNewTransaction()
{
    beginNewTransaction ({});
}

z0 UndoManager::beginNewTransaction (const Txt& actionName)
{
    newTransaction = true;
    newTransactionName = actionName;
}

z0 UndoManager::setCurrentTransactionName (const Txt& newName)
{
    if (newTransaction)
        newTransactionName = newName;
    else if (auto* action = getCurrentSet())
        action->name = newName;
}

Txt UndoManager::getCurrentTransactionName() const
{
    if (auto* action = getCurrentSet())
        return action->name;

    return newTransactionName;
}

//==============================================================================
UndoManager::ActionSet* UndoManager::getCurrentSet() const     { return transactions[nextIndex - 1]; }
UndoManager::ActionSet* UndoManager::getNextSet() const        { return transactions[nextIndex]; }

b8 UndoManager::isPerformingUndoRedo() const  { return isInsideUndoRedoCall; }

b8 UndoManager::canUndo() const      { return getCurrentSet() != nullptr; }
b8 UndoManager::canRedo() const      { return getNextSet()    != nullptr; }

b8 UndoManager::undo()
{
    if (auto* s = getCurrentSet())
    {
        const ScopedValueSetter<b8> setter (isInsideUndoRedoCall, true);

        if (s->undo())
            --nextIndex;
        else
            clearUndoHistory();

        beginNewTransaction();
        sendChangeMessage();
        return true;
    }

    return false;
}

b8 UndoManager::redo()
{
    if (auto* s = getNextSet())
    {
        const ScopedValueSetter<b8> setter (isInsideUndoRedoCall, true);

        if (s->perform())
            ++nextIndex;
        else
            clearUndoHistory();

        beginNewTransaction();
        sendChangeMessage();
        return true;
    }

    return false;
}

Txt UndoManager::getUndoDescription() const
{
    if (auto* s = getCurrentSet())
        return s->name;

    return {};
}

Txt UndoManager::getRedoDescription() const
{
    if (auto* s = getNextSet())
        return s->name;

    return {};
}

StringArray UndoManager::getUndoDescriptions() const
{
    StringArray descriptions;

    for (i32 i = nextIndex;;)
    {
        if (auto* t = transactions[--i])
            descriptions.add (t->name);
        else
            return descriptions;
    }
}

StringArray UndoManager::getRedoDescriptions() const
{
    StringArray descriptions;

    for (i32 i = nextIndex;;)
    {
        if (auto* t = transactions[i++])
            descriptions.add (t->name);
        else
            return descriptions;
    }
}

Time UndoManager::getTimeOfUndoTransaction() const
{
    if (auto* s = getCurrentSet())
        return s->time;

    return {};
}

Time UndoManager::getTimeOfRedoTransaction() const
{
    if (auto* s = getNextSet())
        return s->time;

    return Time::getCurrentTime();
}

b8 UndoManager::undoCurrentTransactionOnly()
{
    if ((! newTransaction) && undo())
    {
        restoreStashedFutureTransactions();
        return true;
    }

    return false;
}

z0 UndoManager::getActionsInCurrentTransaction (Array<const UndoableAction*>& actionsFound) const
{
    if (! newTransaction)
        if (auto* s = getCurrentSet())
            for (auto* a : s->actions)
                actionsFound.add (a);
}

i32 UndoManager::getNumActionsInCurrentTransaction() const
{
    if (! newTransaction)
        if (auto* s = getCurrentSet())
            return s->actions.size();

    return 0;
}

} // namespace drx
