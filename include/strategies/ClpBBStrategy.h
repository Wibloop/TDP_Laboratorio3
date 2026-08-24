#ifndef CLPBBSTRATEGY_H
#define CLPBBSTRATEGY_H

#include "PlacementStrategy.h"
#include <vector>
#include <map>
#include <memory>

// Estructura de resultado del adaptador LP
struct LPResult {
    int status; // Codigo de estado del solver lineal
    double objective;
    std::vector<double> primal;
    int iterations;
};

// Estructura de un nodo del arbol B&B
struct BBNode {
    int id;
    int depth;
    double lb;
    std::map<int, double> fixedVariables; // Mapa de k a valor fijado binario

    // Comparador para la cola de prioridad Best-Bound
    // Orden de extraccion: menor LB luego mayor profundidad y menor ID
    bool operator<(const BBNode& other) const {
        const double EPS = 1e-6;
        if (std::abs(lb - other.lb) > EPS) {
            return lb > other.lb; // Invertido para que el menor lb quede arriba
        }
        if (depth != other.depth) {
            return depth < other.depth; // Invertido para que mayor profundidad quede arriba
        }
        return id > other.id; // Invertido para que menor id quede arriba
    }
};

// Estrategia A0 Algoritmo propio de Branch-and-Bound apoyado en ClpSimplex
class ClpBBStrategy : public PlacementStrategy {
private:
    // Resolver adaptador para encapsular llamadas a ClpSimplex
    LPResult ResolverLP(class ClpSimplex& model, bool isRoot);

    // Heuristica de reparacion: Intenta construir solucion entera a partir del primal relajado
    SolveResult RepararDesdeLP(const std::vector<double>& primal, const Snapshot& snapshot, 
                               const std::vector<std::string>& instanceIds, 
                               const std::vector<std::string>& nodeIds);

public:
    SolveResult solve(const Snapshot& snapshot, int time_limit_ms) override;
};

#endif // CLPBBSTRATEGY_H
