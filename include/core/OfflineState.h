#ifndef OFFLINESTATE_H
#define OFFLINESTATE_H

#include "NodeState.h"
#include <iostream>

/**
 * Estado Inactivo
 * El nodo falla completamente, su capacidad disponible es 0
 */
class OfflineState : public NodeState {
public:
    void handleFail(Node& node) override;
    void handleDegrade(Node& node, double rho) override;
    void handleRecover(Node& node) override;

    double getRho() const override { return 0.0; }
    std::string getName() const override { return "OFFLINE"; }
};

#endif // OFFLINESTATE_H
