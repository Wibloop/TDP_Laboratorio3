#ifndef DEGRADEDSTATE_H
#define DEGRADEDSTATE_H

#include "NodeState.h"
#include <iostream>

/**
 * Estado Degradado
 * El nodo funciona a una fraccion (rho) de su capacidad nominal
 */
class DegradedState : public NodeState {
private:
    double currentRho; // Fraccion de capacidad actual (0 < rho < 1)

public:
    // Constructor
    explicit DegradedState(double rho) : currentRho(rho) {}

    void handleFail(Node& node) override;
    void handleDegrade(Node& node, double rho) override;
    void handleRecover(Node& node) override;

    double getRho() const override { return currentRho; }
    std::string getName() const override { return "DEGRADED"; }
};

#endif // DEGRADEDSTATE_H
