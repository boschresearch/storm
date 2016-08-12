#include "src/abstraction/MenuGame.h"

#include "src/exceptions/InvalidOperationException.h"
#include "src/exceptions/InvalidArgumentException.h"

#include "src/storage/dd/Bdd.h"
#include "src/storage/dd/Add.h"
#include "src/storage/dd/DdManager.h"

#include "src/models/symbolic/StandardRewardModel.h"

namespace storm {
    namespace abstraction {
        
        template<storm::dd::DdType Type, typename ValueType>
        MenuGame<Type, ValueType>::MenuGame(std::shared_ptr<storm::dd::DdManager<Type>> manager,
                                            storm::dd::Bdd<Type> reachableStates,
                                            storm::dd::Bdd<Type> initialStates,
                                            storm::dd::Bdd<Type> deadlockStates,
                                            storm::dd::Add<Type, ValueType> transitionMatrix,
                                            storm::dd::Bdd<Type> bottomStates,
                                            std::set<storm::expressions::Variable> const& rowVariables,
                                            std::set<storm::expressions::Variable> const& columnVariables,
                                            std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs,
                                            std::set<storm::expressions::Variable> const& player1Variables,
                                            std::set<storm::expressions::Variable> const& player2Variables,
                                            std::set<storm::expressions::Variable> const& allNondeterminismVariables,
                                            std::set<storm::expressions::Variable> const& probabilisticBranchingVariables,
                                            std::map<storm::expressions::Expression, storm::dd::Bdd<Type>> const& expressionToBddMap) : storm::models::symbolic::StochasticTwoPlayerGame<Type>(manager, reachableStates, initialStates, deadlockStates, transitionMatrix.sumAbstract(probabilisticBranchingVariables), rowVariables, nullptr, columnVariables, nullptr, rowColumnMetaVariablePairs, player1Variables, player2Variables, allNondeterminismVariables), probabilisticBranchingVariables(probabilisticBranchingVariables), expressionToBddMap(expressionToBddMap), bottomStates(bottomStates) {
            // Intentionally left empty.
            this->getTransitionMatrix().exportToDot("trans.dot");
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Bdd<Type> MenuGame<Type, ValueType>::getStates(std::string const& label) const {
            STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Menu games do not provide labels.");
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Bdd<Type> MenuGame<Type, ValueType>::getStates(storm::expressions::Expression const& expression) const {
            return this->getStates(expression, false);
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Bdd<Type> MenuGame<Type, ValueType>::getStates(storm::expressions::Expression const& expression, bool negated) const {
            if (expression.isTrue()) {
                return this->getReachableStates();
            } else if (expression.isFalse()) {
                return this->getManager().getBddZero();
            }
            auto it = expressionToBddMap.find(expression);
            STORM_LOG_THROW(it != expressionToBddMap.end(), storm::exceptions::InvalidArgumentException, "The given expression was not used in the abstraction process and can therefore not be retrieved.");
            if (negated) {
                return !it->second && this->getReachableStates();
            } else {
                return it->second && this->getReachableStates();
            }
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Bdd<Type> MenuGame<Type, ValueType>::getBottomStates() const {
            return bottomStates;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        bool MenuGame<Type, ValueType>::hasLabel(std::string const& label) const {
            return false;
        }
        
        template class MenuGame<storm::dd::DdType::CUDD, double>;
        template class MenuGame<storm::dd::DdType::Sylvan, double>;
        
    }
}

