#ifndef PLACEMENTSTRATEGY_H
#define PLACEMENTSTRATEGY_H

#include <string>
#include "../core/Snapshot.h"
#include "../solvers/Validator.h"

// Estructura que almacena el resultado de una solucion
struct SolveResult {
    bool hasSolution;
    AssignmentMap X;
    double Z;
    
    // Metricas operativas requeridas
    std::string status; 
    double runtime_ms;
    double LB;
    double UB;
    double gap_percent;
    int nodes;
    int lp_calls;

    SolveResult() : hasSolution(false), Z(0.0), status("NO_SOLUTION"), runtime_ms(0.0), 
                    LB(0.0), UB(0.0), gap_percent(0.0), nodes(0), lp_calls(0) {}
    
    SolveResult(bool found, const AssignmentMap& assignment, double objVal)
        : hasSolution(found), X(assignment), Z(objVal), status(found ? "FEASIBLE" : "NO_SOLUTION"), 
          runtime_ms(0.0), LB(0.0), UB(0.0), gap_percent(0.0), nodes(0), lp_calls(0) {}
};

// Interfaz base para el patron Strategy de algoritmos de ubicacion
class PlacementStrategy {
public:
    virtual ~PlacementStrategy() = default;

    // Resuelve el problema basandose en el snapshot St
    // Retorna la mejor asignacion encontrada o hasSolution igual a false
    virtual SolveResult solve(const Snapshot& snapshot, int time_limit_ms) = 0;
};

#endif // PLACEMENTSTRATEGY_H
