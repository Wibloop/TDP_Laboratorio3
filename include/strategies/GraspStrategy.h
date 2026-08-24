#ifndef GRASPSTRATEGY_H
#define GRASPSTRATEGY_H

#include "PlacementStrategy.h"
#include <cstdint>
#include <string>

// Estrategia H2 Metaheuristica adaptativa GRASP
// Combina una fase de construccion aleatorizada guiada RCL con una busqueda local
class GraspStrategy : public PlacementStrategy {
private:
    uint64_t seed;
    double eta;    // Parametro de aleatoriedad en rango cero a uno
    double lambda; // Parametro de peso de capacidad no negativo

public:
    // Constructor que valida parametros y lanza std::invalid_argument si son invalidos
    GraspStrategy(uint64_t seed_val, double eta_val, double lambda_val);

    SolveResult solve(const Snapshot& snapshot, int time_limit_ms) override;

private:
    // Funcion auxiliar para comparar dos asignaciones lexicograficamente en caso de empate
    bool isLexicographicallySmaller(const AssignmentMap& a, const AssignmentMap& b) const;
};

#endif // GRASPSTRATEGY_H
