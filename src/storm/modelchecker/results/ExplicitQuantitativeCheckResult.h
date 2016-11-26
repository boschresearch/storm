#ifndef STORM_MODELCHECKER_EXPLICITQUANTITATIVECHECKRESULT_H_
#define STORM_MODELCHECKER_EXPLICITQUANTITATIVECHECKRESULT_H_

#include <vector>
#include <map>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "storm/modelchecker/results/QuantitativeCheckResult.h"
#include "storm/storage/sparse/StateType.h"
#include "storm/storage/Scheduler.h"
#include "storm/utility/OsDetection.h"

namespace storm {
    namespace modelchecker {
        template<typename ValueType>
        class ExplicitQuantitativeCheckResult : public QuantitativeCheckResult<ValueType> {
        public:
            typedef std::vector<ValueType> vector_type;
            typedef std::map<storm::storage::sparse::state_type, ValueType> map_type;
            
            ExplicitQuantitativeCheckResult();
            ExplicitQuantitativeCheckResult(map_type const& values);
            ExplicitQuantitativeCheckResult(map_type&& values);
            ExplicitQuantitativeCheckResult(storm::storage::sparse::state_type const& state, ValueType const& value);
            ExplicitQuantitativeCheckResult(vector_type const& values);
            ExplicitQuantitativeCheckResult(vector_type&& values);
            
            ExplicitQuantitativeCheckResult(ExplicitQuantitativeCheckResult const& other) = default;
            ExplicitQuantitativeCheckResult& operator=(ExplicitQuantitativeCheckResult const& other) = default;
#ifndef WINDOWS
            ExplicitQuantitativeCheckResult(ExplicitQuantitativeCheckResult&& other) = default;
            ExplicitQuantitativeCheckResult& operator=(ExplicitQuantitativeCheckResult&& other) = default;
#endif
            virtual ~ExplicitQuantitativeCheckResult() = default;
            
            ValueType& operator[](storm::storage::sparse::state_type state);
            ValueType const& operator[](storm::storage::sparse::state_type state) const;

            virtual std::unique_ptr<CheckResult> compareAgainstBound(storm::logic::ComparisonType comparisonType, ValueType const& bound) const override;
            
            virtual bool isExplicit() const override;
            virtual bool isResultForAllStates() const override;
            
            virtual bool isExplicitQuantitativeCheckResult() const override;
            
            vector_type const& getValueVector() const;
            map_type const& getValueMap() const;
            
            virtual std::ostream& writeToStream(std::ostream& out) const override;

            virtual void filter(QualitativeCheckResult const& filter) override;

            virtual void oneMinus() override;
            
            
            virtual ValueType getMin() const override;
            virtual ValueType getMax() const override;
            virtual ValueType average() const override;
            virtual ValueType sum() const override;
            
            bool hasScheduler() const;
            void setScheduler(std::unique_ptr<storm::storage::Scheduler>&& scheduler);
            storm::storage::Scheduler const& getScheduler() const;
            
        private:
            // The values of the quantitative check result.
            boost::variant<vector_type, map_type> values;
            
            // An optional scheduler that accompanies the values.
            boost::optional<std::shared_ptr<storm::storage::Scheduler>> scheduler;
        };
    }
}

#endif /* STORM_MODELCHECKER_EXPLICITQUANTITATIVECHECKRESULT_H_ */