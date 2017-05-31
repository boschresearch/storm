#include <storm/models/sparse/StochasticTwoPlayerGame.h>
#include "storm/utility/builder.h"

#include "storm/models/sparse/Dtmc.h"
#include "storm/models/sparse/Ctmc.h"
#include "storm/models/sparse/Mdp.h"
#include "storm/models/sparse/MarkovAutomaton.h"

namespace storm {
    namespace utility {
        namespace builder {
            
            template<typename ValueType, typename RewardModelType>
            std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> buildModelFromComponents(storm::models::ModelType modelType, storm::storage::sparse::ModelComponents<ValueType, RewardModelType>&& components) {
                switch (modelType) {
                    case storm::models::ModelType::Dtmc:
                        return std::make_shared<storm::models::sparse::Dtmc<ValueType, RewardModelType>>(std::move(components));
                    case storm::models::ModelType::Ctmc:
                        return std::make_shared<storm::models::sparse::Ctmc<ValueType, RewardModelType>>(std::move(components));
                    case storm::models::ModelType::Mdp:
                        return std::make_shared<storm::models::sparse::Mdp<ValueType, RewardModelType>>(std::move(components));
                    case storm::models::ModelType::MarkovAutomaton:
                        return std::make_shared<storm::models::sparse::MarkovAutomaton<ValueType, RewardModelType>>(std::move(components));
                    case storm::models::ModelType::S2pg:
                        return std::make_shared<storm::models::sparse::StochasticTwoPlayerGame<ValueType, RewardModelType>>(std::move(components));
                }
            }
            
            template std::shared_ptr<storm::models::sparse::Model<double>> buildModelFromComponents(storm::models::ModelType modelType, storm::storage::sparse::ModelComponents<double>&& components);
            template std::shared_ptr<storm::models::sparse::Model<double, storm::models::sparse::StandardRewardModel<storm::Interval>>> buildModelFromComponents(storm::models::ModelType modelType, storm::storage::sparse::ModelComponents<double, storm::models::sparse::StandardRewardModel<storm::Interval>>&& components);
            template std::shared_ptr<storm::models::sparse::Model<storm::RationalNumber>> buildModelFromComponents(storm::models::ModelType modelType, storm::storage::sparse::ModelComponents<storm::RationalNumber>&& components);
            template std::shared_ptr<storm::models::sparse::Model<storm::RationalFunction>> buildModelFromComponents(storm::models::ModelType modelType, storm::storage::sparse::ModelComponents<storm::RationalFunction>&& components);
        }
    }
}