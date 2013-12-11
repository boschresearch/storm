#include "src/storage/MaximalEndComponentDecomposition.h"

#include <list>
#include <queue>

namespace storm {
    namespace storage {
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>::MaximalEndComponentDecomposition() : Decomposition() {
            // Intentionally left empty.
        }
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>::MaximalEndComponentDecomposition(storm::models::AbstractNondeterministicModel<ValueType> const& model) {
            performMaximalEndComponentDecomposition(model, storm::storage::BitVector(model.getNumberOfStates(), true));
        }
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>::MaximalEndComponentDecomposition(storm::models::AbstractNondeterministicModel<ValueType> const& model, storm::storage::BitVector const& subsystem) {
            performMaximalEndComponentDecomposition(model, subsystem);
        }
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>::MaximalEndComponentDecomposition(MaximalEndComponentDecomposition const& other) : Decomposition(other) {
            // Intentionally left empty.
        }
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>& MaximalEndComponentDecomposition<ValueType>::operator=(MaximalEndComponentDecomposition const& other) {
            Decomposition::operator=(other);
            return *this;
        }
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>::MaximalEndComponentDecomposition(MaximalEndComponentDecomposition&& other) : Decomposition(std::move(other)) {
            // Intentionally left empty.
        }
        
        template<typename ValueType>
        MaximalEndComponentDecomposition<ValueType>& MaximalEndComponentDecomposition<ValueType>::operator=(MaximalEndComponentDecomposition&& other) {
            Decomposition::operator=(std::move(other));
            return *this;
        }
        
        template <typename ValueType>
        void MaximalEndComponentDecomposition<ValueType>::performMaximalEndComponentDecomposition(storm::models::AbstractNondeterministicModel<ValueType> const& model, storm::storage::BitVector const& subsystem) {
            // Get some references for convenient access.
            storm::storage::SparseMatrix<ValueType> backwardTransitions = model.getBackwardTransitions();
            std::vector<uint_fast64_t> const& nondeterministicChoiceIndices = model.getNondeterministicChoiceIndices();
            storm::storage::SparseMatrix<ValueType> const& transitionMatrix = model.getTransitionMatrix();
            
            // Initialize the maximal end component list to be the full state space.
            std::list<StateBlock> endComponentStateSets;
            endComponentStateSets.emplace_back(subsystem);
            storm::storage::BitVector statesToCheck(model.getNumberOfStates());
            

            // The iterator used here should really be a const_iterator.
            // However, gcc 4.8 (and assorted libraries) does not provide an erase(const_iterator) method for std::list but only an erase(iterator).
            // This is in compliance with the c++11 draft N3337, which specifies the change from iterator to const_iterator only for "set, multiset, map [and] multimap".
            // FIXME: As soon as gcc provides an erase(const_iterator) method, change this iterator back to a const_iterator.
            for (std::list<StateBlock>::iterator mecIterator = endComponentStateSets.begin(); mecIterator != endComponentStateSets.end();) {
                StateBlock const& mec = *mecIterator;

                // Keep track of whether the MEC changed during this iteration.
                bool mecChanged = false;
                
                // Get an SCC decomposition of the current MEC candidate.
                StronglyConnectedComponentDecomposition<ValueType> sccs(model, mec, true);
                mecChanged |= sccs.size() > 1;
                
                // Check for each of the SCCs whether there is at least one action for each state that does not leave the SCC.
                for (auto& scc : sccs) {
                    statesToCheck.set(scc.begin(), scc.end());
                    
                    while (!statesToCheck.empty()) {
                        storm::storage::BitVector statesToRemove(model.getNumberOfStates());
                        
                        for (auto state : statesToCheck) {
                            bool keepStateInMEC = false;
                            
                            for (uint_fast64_t choice = nondeterministicChoiceIndices[state]; choice < nondeterministicChoiceIndices[state + 1]; ++choice) {
                                bool choiceContainedInMEC = true;
                                for (auto const& entry : transitionMatrix.getRow(choice)) {
                                    if (!scc.contains(entry.first)) {
                                        choiceContainedInMEC = false;
                                        break;
                                    }
                                }
                                
                                // If there is at least one choice whose successor states are fully contained in the MEC, we can leave the state in the MEC.
                                if (choiceContainedInMEC) {
                                    keepStateInMEC = true;
                                    break;
                                }
                            }
                            
                            if (!keepStateInMEC) {
                                statesToRemove.set(state, true);
                            }
                        }
                        
                        // Now erase the states that have no option to stay inside the MEC with all successors.
                        mecChanged |= !statesToRemove.empty();
                        scc.erase(storm::storage::VectorSet<uint_fast64_t>(statesToRemove.begin(), statesToRemove.end()));
                        
                        // Now check which states should be reconsidered, because successors of them were removed.
                        statesToCheck.clear();
                        for (auto state : statesToRemove) {
                            for (auto const& entry : transitionMatrix.getRow(state)) {
                                if (scc.contains(entry.first)) {
                                    statesToCheck.set(entry.first);
                                }
                            }
                        }
                    }
                }
                
                // If the MEC changed, we delete it from the list of MECs and append the possible new MEC candidates to the list instead.
                if (mecChanged) {
                    for (StateBlock& scc : sccs) {
                        if (!scc.empty()) {
                            endComponentStateSets.push_back(std::move(scc));
                        }
                    }

                    std::list<StateBlock>::iterator eraseIterator(mecIterator);
                    ++mecIterator;
                    endComponentStateSets.erase(eraseIterator);
                } else {
                    // Otherwise, we proceed with the next MEC candidate.
                    ++mecIterator;
                }
                
            } // end of loop over all MEC candidates
            
            // Now that we computed the underlying state sets of the MECs, we need to properly identify the choices contained in the MEC.
            this->blocks.reserve(endComponentStateSets.size());
            for (auto const& mecStateSet : endComponentStateSets) {
                MaximalEndComponent newMec;
                
                for (auto state : mecStateSet) {
                    std::vector<uint_fast64_t> containedChoices;
                    for (uint_fast64_t choice = nondeterministicChoiceIndices[state]; choice < nondeterministicChoiceIndices[state + 1]; ++choice) {
                        bool choiceContained = true;
                        for (auto const& entry : transitionMatrix.getRow(choice)) {
                            if (!mecStateSet.contains(entry.first)) {
                                choiceContained = false;
                                break;
                            }
                        }
                        
                        if (choiceContained) {
                            containedChoices.push_back(choice);
                        }
                    }
                    
                    newMec.addState(state, std::move(containedChoices));
                }
                
                this->blocks.emplace_back(std::move(newMec));
            }
            
            LOG4CPLUS_INFO(logger, "Computed MEC decomposition of size " << this->size() << ".");
        }
        
        template class MaximalEndComponentDecomposition<double>;
    }
}