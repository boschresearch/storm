#include "src/solver/GmmxxNondeterministicLinearEquationSolver.h"

#include <utility>

#include "src/settings/Settings.h"
#include "src/adapters/GmmxxAdapter.h"
#include "src/utility/vector.h"

bool GmmxxNondeterministicLinearEquationSolverOptionsRegistered = storm::settings::Settings::registerNewModule([] (storm::settings::Settings* instance) -> bool {
	instance->addOption(storm::settings::OptionBuilder("GmmxxNondeterminsticLinearEquationSolver", "maxiter", "i", "The maximal number of iterations to perform before iterative solving is aborted.").addArgument(storm::settings::ArgumentBuilder::createUnsignedIntegerArgument("count", "The maximal iteration count.").setDefaultValueUnsignedInteger(10000).build()).build());
    
	instance->addOption(storm::settings::OptionBuilder("GmmxxNondeterminsticLinearEquationSolver", "precision", "", "The precision used for detecting convergence of iterative methods.").addArgument(storm::settings::ArgumentBuilder::createDoubleArgument("value", "The precision to achieve.").setDefaultValueDouble(1e-6).addValidationFunctionDouble(storm::settings::ArgumentValidators::doubleRangeValidatorExcluding(0.0, 1.0)).build()).build());
    
	instance->addOption(storm::settings::OptionBuilder("GmmxxNondeterminsticLinearEquationSolver", "absolute", "", "Whether the relative or the absolute error is considered for deciding convergence.").build());
    
	return true;
});

namespace storm {
    namespace solver {
        
        template<typename ValueType>
        GmmxxNondeterministicLinearEquationSolver<ValueType>::GmmxxNondeterministicLinearEquationSolver() {
            // Get the settings object to customize solving.
            storm::settings::Settings* settings = storm::settings::Settings::getInstance();
            
            // Get appropriate settings.
            maximalNumberOfIterations = settings->getOptionByLongName("maxiter").getArgument(0).getValueAsUnsignedInteger();
            precision = settings->getOptionByLongName("precision").getArgument(0).getValueAsDouble();
            relative = !settings->isSet("absolute");
        }
        
        template<typename ValueType>
        GmmxxNondeterministicLinearEquationSolver<ValueType>::GmmxxNondeterministicLinearEquationSolver(double precision, uint_fast64_t maximalNumberOfIterations, bool relative) : precision(precision), relative(relative), maximalNumberOfIterations(maximalNumberOfIterations) {
            // Intentionally left empty.
        }

        
        template<typename ValueType>
        NondeterministicLinearEquationSolver<ValueType>* GmmxxNondeterministicLinearEquationSolver<ValueType>::clone() const {
            return new GmmxxNondeterministicLinearEquationSolver<ValueType>(*this);
        }
        
        template<typename ValueType>
        void GmmxxNondeterministicLinearEquationSolver<ValueType>::solveEquationSystem(bool minimize, storm::storage::SparseMatrix<ValueType> const& A, std::vector<ValueType>& x, std::vector<ValueType> const& b, std::vector<uint_fast64_t> const& nondeterministicChoiceIndices, std::vector<ValueType>* multiplyResult, std::vector<ValueType>* newX) const {
            // Transform the transition probability matrix to the gmm++ format to use its arithmetic.
            std::unique_ptr<gmm::csr_matrix<ValueType>> gmmxxMatrix = storm::adapters::GmmxxAdapter::toGmmxxSparseMatrix<ValueType>(A);
            
            // Set up the environment for the power method. If scratch memory was not provided, we need to create it.
            bool multiplyResultMemoryProvided = true;
            if (multiplyResult == nullptr) {
                multiplyResult = new std::vector<ValueType>(A.getRowCount());
                multiplyResultMemoryProvided = false;
            }
            
            std::vector<ValueType>* currentX = &x;
            bool xMemoryProvided = true;
            if (newX == nullptr) {
                newX = new std::vector<ValueType>(x.size());
                xMemoryProvided = false;
            }
            std::vector<ValueType>* swap = nullptr;
            uint_fast64_t iterations = 0;
            bool converged = false;
            
            // Keep track of which of the vectors for x is the auxiliary copy.
            std::vector<ValueType>* copyX = newX;

            // Proceed with the iterations as long as the method did not converge or reach the user-specified maximum number
            // of iterations.
            while (!converged && iterations < maximalNumberOfIterations) {
                // Compute x' = A*x + b.
                gmm::mult(*gmmxxMatrix, *currentX, *multiplyResult);
                gmm::add(b, *multiplyResult);
                
                // Reduce the vector x by applying min/max over all nondeterministic choices.
                if (minimize) {
                    storm::utility::vector::reduceVectorMin(*multiplyResult, *newX, nondeterministicChoiceIndices);
                } else {
                    storm::utility::vector::reduceVectorMax(*multiplyResult, *newX, nondeterministicChoiceIndices);
                }
                
                // Determine whether the method converged.
                converged = storm::utility::vector::equalModuloPrecision(*currentX, *newX, this->precision, this->relative);
                
                // Update environment variables.
                std::swap(currentX, newX);
                ++iterations;
            }
            
            // Check if the solver converged and issue a warning otherwise.
            if (converged) {
                LOG4CPLUS_INFO(logger, "Iterative solver converged after " << iterations << " iterations.");
            } else {
                LOG4CPLUS_WARN(logger, "Iterative solver did not converge after " << iterations << " iterations.");
            }
            
            // If we performed an odd number of iterations, we need to swap the x and currentX, because the newest result
            // is currently stored in currentX, but x is the output vector.
            if (currentX == copyX) {
                std::swap(x, *currentX);
            }
            
            if (!xMemoryProvided) {
                delete copyX;
            }
            
            if (!multiplyResultMemoryProvided) {
                delete multiplyResult;
            }
        }
        
        template<typename ValueType>
        void GmmxxNondeterministicLinearEquationSolver<ValueType>::performMatrixVectorMultiplication(bool minimize, storm::storage::SparseMatrix<ValueType> const& A, std::vector<ValueType>& x, std::vector<uint_fast64_t> const& nondeterministicChoiceIndices, std::vector<ValueType>* b, uint_fast64_t n, std::vector<ValueType>* multiplyResult) const {
            // Transform the transition probability matrix to the gmm++ format to use its arithmetic.
            std::unique_ptr<gmm::csr_matrix<ValueType>> gmmxxMatrix = storm::adapters::GmmxxAdapter::toGmmxxSparseMatrix<ValueType>(A);
            
            bool multiplyResultMemoryProvided = true;
            if (multiplyResult == nullptr) {
                multiplyResult = new std::vector<ValueType>(A.getRowCount());
                multiplyResultMemoryProvided = false;
            }
            
            // Now perform matrix-vector multiplication as long as we meet the bound of the formula.
            for (uint_fast64_t i = 0; i < n; ++i) {
                gmm::mult(*gmmxxMatrix, x, *multiplyResult);
                
                if (b != nullptr) {
                    gmm::add(*b, *multiplyResult);
                }
                
                if (minimize) {
                    storm::utility::vector::reduceVectorMin(*multiplyResult, x, nondeterministicChoiceIndices);
                } else {
                    storm::utility::vector::reduceVectorMax(*multiplyResult, x, nondeterministicChoiceIndices);
                }
            }
            
            if (!multiplyResultMemoryProvided) {
                delete multiplyResult;
            }
        }

        // Explicitly instantiate the solver.
        template class GmmxxNondeterministicLinearEquationSolver<double>;
    } // namespace solver
} // namespace storm