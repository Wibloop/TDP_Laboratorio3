#ifndef GREEDYSTABLESTRATEGY_H
#define GREEDYSTABLESTRATEGY_H

#include "PlacementStrategy.h"

/**
 * Estrategia H1: Heuristica Golosa Estable acoplada con Busqueda Local
 * Se encarga de construir una solucion inicial determinista y mejorarla
 */
class GreedyStableStrategy : public PlacementStrategy {
public:
    SolveResult solve(const Snapshot& snapshot, int time_limit_ms) override;

private:
    // Fase de construccion: Heuristica Golosa Estable
    SolveResult buildInitialSolution(const Snapshot& snapshot, int time_limit_ms);
};

#endif // GREEDYSTABLESTRATEGY_H
