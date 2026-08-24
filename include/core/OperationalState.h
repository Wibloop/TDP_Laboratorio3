#ifndef OPERATIONALSTATE_H
#define OPERATIONALSTATE_H

#include "NodeState.h"
#include <iostream>

/**
 * Estado Operacional
 * El nodo funciona al 100% de su capacidad nominal
 */
class OperationalState : public NodeState {
public:
    void handleFail(Node& node) override;
    void handleDegrade(Node& node, double rho) override;
    void handleRecover(Node& node) override;

    double getRho() const override { return 1.0; }
    std::string getName() const override { return "OPERATIONAL"; }
};

#endif // OPERATIONALSTATE_H
