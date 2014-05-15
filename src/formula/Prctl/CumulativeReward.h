/*
 * InstantaneousReward.h
 *
 *  Created on: 26.12.2012
 *      Author: Christian Dehnert
 */

#ifndef STORM_FORMULA_PRCTL_CUMULATIVEREWARD_H_
#define STORM_FORMULA_PRCTL_CUMULATIVEREWARD_H_

#include "AbstractRewardPathFormula.h"
#include "src/formula/AbstractFormulaChecker.h"
#include <string>

namespace storm {
namespace property {
namespace prctl {

template <class T> class CumulativeReward;

/*!
 *  @brief Interface class for model checkers that support CumulativeReward.
 *
 *  All model checkers that support the formula class CumulativeReward must inherit
 *  this pure virtual class.
 */
template <class T>
class ICumulativeRewardModelChecker {
    public:
		/*!
         *  @brief Evaluates CumulativeReward formula within a model checker.
         *
         *  @param obj Formula object with subformulas.
         *  @return Result of the formula for every node.
         */
        virtual std::vector<T> checkCumulativeReward(const CumulativeReward<T>& obj, bool qualitative) const = 0;
};

/*!
 * @brief
 * Class for an abstract (path) formula tree with a Cumulative Reward node as root.
 *
 * The subtrees are seen as part of the object and deleted with the object
 * (this behavior can be prevented by setting them to NULL before deletion)
 *
 * @see AbstractPathFormula
 * @see AbstractPrctlFormula
 */
template <class T>
class CumulativeReward : public AbstractRewardPathFormula<T> {

public:
	/*!
	 * Empty constructor
	 */
	CumulativeReward() : bound(0){
		// Intentionally left empty
	}

	/*!
	 * Constructor
	 *
	 * @param bound The time bound of the reward formula
	 */
	CumulativeReward(T bound) : bound(bound){
		// Intentionally left empty.
	}

	/*!
	 * Empty destructor.
	 */
	virtual ~CumulativeReward() {
		// Intentionally left empty.
	}

	/*!
	 * Clones the called object.
	 *
	 * Performs a "deep copy", i.e. the subtrees of the new object are clones of the original ones
	 *
	 * @returns a new CumulativeReward-object that is identical the called object.
	 */
	virtual AbstractRewardPathFormula<T>* clone() const override {
		return new CumulativeReward(this->getBound());
	}


	/*!
	 * Calls the model checker to check this formula.
	 * Needed to infer the correct type of formula class.
	 *
	 * @note This function should only be called in a generic check function of a model checker class. For other uses,
	 *       the methods of the model checker should be used.
	 *
	 * @returns A vector indicating the probability that the formula holds for each state.
	 */
	virtual std::vector<T> check(const storm::modelchecker::prctl::AbstractModelChecker<T>& modelChecker, bool qualitative) const override {
		return modelChecker.template as<ICumulativeRewardModelChecker>()->checkCumulativeReward(*this, qualitative);
	}

	/*!
	 * @returns a string representation of the formula
	 */
	virtual std::string toString() const override {
		std::string result = "C <= ";
		result += std::to_string(bound);
		return result;
	}

	/*!
	 *  @brief Checks if all subtrees conform to some logic.
	 *
	 *  As CumulativeReward objects have no subformulas, we return true here.
	 *
	 *  @param checker Formula checker object.
	 *  @return true
	 */
	virtual bool validate(const AbstractFormulaChecker<T>& checker) const override {
		return true;
	}

	/*!
	 * @returns the time instance for the instantaneous reward operator
	 */
	T getBound() const {
		return bound;
	}

	/*!
	 * Sets the the time instance for the instantaneous reward operator
	 *
	 * @param bound the new bound.
	 */
	void setBound(T bound) {
		this->bound = bound;
	}

private:
	T bound;
};

} //namespace prctl
} //namespace property
} //namespace storm

#endif /* STORM_FORMULA_PRCTL_INSTANTANEOUSREWARD_H_ */
