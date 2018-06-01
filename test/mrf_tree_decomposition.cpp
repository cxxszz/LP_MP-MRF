#include "test_mrf.hxx"
#include "LP_FWMAP.hxx"
#include "LP_conic_bundle.hxx"

using namespace LP_MP;

int main() 
{
    using FMC = FMC_SRMP;
    using VisitorType = StandardVisitor;

    auto construct_problem = [](auto& solver) {
        auto& mrf = solver.template GetProblemConstructor<0>();

        // square with three negative and two positive Potts potentials
        mrf.AddUnaryFactor(std::vector<REAL>({1.1, 0.0}));
        mrf.AddUnaryFactor(std::vector<REAL>({0.0, 0.0}));
        mrf.AddUnaryFactor(std::vector<REAL>({0.0, 0.0}));
        mrf.AddUnaryFactor(std::vector<REAL>({0.0, 1.0}));

        auto potts_pot = construct_potts(2,2, 0.0, 1.0);
        mrf.AddPairwiseFactor(0,1, potts_pot);
        assert(potts_pot(0,0) == 0.0 && potts_pot(0,1) == 1.0 && potts_pot(1,0) == 1.0 && potts_pot(1,1) == 0.0);
        mrf.AddPairwiseFactor(1,2, potts_pot);
        assert(potts_pot(0,0) == 0.0 && potts_pot(0,1) == 1.0 && potts_pot(1,0) == 1.0 && potts_pot(1,1) == 0.0);
        mrf.AddPairwiseFactor(2,3, potts_pot);
        assert(potts_pot(0,0) == 0.0 && potts_pot(0,1) == 1.0 && potts_pot(1,0) == 1.0 && potts_pot(1,1) == 0.0);
        mrf.AddPairwiseFactor(0,3, potts_pot);
        assert(potts_pot(0,0) == 0.0 && potts_pot(0,1) == 1.0 && potts_pot(1,0) == 1.0 && potts_pot(1,1) == 0.0);

    };

    // message passing
    {
        using SolverType = Solver<LP<FMC>, VisitorType>;
        SolverType s(solver_options);
        construct_problem(s);
        s.Solve();
        test(std::abs(s.GetLP().LowerBound() - 1.0) <= eps);
    }

    // FWMAP
    {
        using SolverType = Solver<LP_tree_FWMAP<FMC>,VisitorType>;

        SolverType s(solver_options);
        construct_problem(s);
        auto& mrf = s.template GetProblemConstructor<0>();
        auto trees = mrf.compute_forest_cover();
        for(auto& tree : trees) { s.GetLP().add_tree(tree); }
        s.Solve(); 

        test(std::abs(s.GetLP().decomposition_lower_bound() - 1.0) < LP_MP::eps);
    }

    // conic bundle
    {
        using SolverType = Solver<LP_conic_bundle<FMC>,VisitorType>;

        SolverType s(solver_options);
        construct_problem(s);
        auto& mrf = s.template GetProblemConstructor<0>();
        auto trees = mrf.compute_forest_cover();
        for(auto& tree : trees) { s.GetLP().add_tree(tree); }

        s.Solve();

        test(std::abs(s.GetLP().decomposition_lower_bound() - 1.0) < 1e-6); // conic bundle exits earlier tbefore reaching eps accuracy
    }

    // subgradient ascent
    {
        using SolverType = Solver<LP_subgradient_ascent<FMC>,VisitorType>;

        SolverType s(solver_options);
        construct_problem(s);
        auto& mrf = s.template GetProblemConstructor<0>();
        auto trees = mrf.compute_forest_cover();
        for(auto& tree : trees) { s.GetLP().add_tree(tree); }

        s.Solve();

        test(std::abs(s.GetLP().decomposition_lower_bound() - 1.0) < LP_MP::eps);
    }
}
