#ifndef CBCSTRATEGY_H
#define CBCSTRATEGY_H

#include "PlacementStrategy.h"

/**
 * Estrategia de Referencia R: Solucionador exacto usando COIN-OR Cbc
 * Modela el problema matematico completo y ejecuta branch-and-cut
 */
class CbcStrategy : public PlacementStrategy {
public:
    SolveResult solve(const Snapshot& snapshot, int time_limit_ms) override;
};

#endif // CBCSTRATEGY_H
