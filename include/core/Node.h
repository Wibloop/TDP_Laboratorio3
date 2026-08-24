#ifndef NODE_H
#define NODE_H

#include <string>
#include <memory>
#include "NodeState.h"

/**
 * Clase que representa un nodo en la infraestructura
 * Mantiene su capacidad nominal y delega el comportamiento de estado a NodeState
 */
class Node {
private:
    std::string id;
    double nominalCapacity;
    std::unique_ptr<NodeState> state;

public:
    Node(const std::string& nodeId, double capacity);
    
    // Metodos delegados al patron State para transiciones
    void fail();
    void degrade(double rho);
    void recover();

    // Setter para cambiar el estado desde las clases State
    void setState(std::unique_ptr<NodeState> newState);

    // Obtener la capacidad disponible en base al estado actual (K_j = rho * K_nominal)
    double getAvailableCapacity() const;

    // Getters
    std::string getId() const { return id; }
    double getNominalCapacity() const { return nominalCapacity; }
    std::string getStateName() const { return state->getName(); }
    double getCurrentRho() const { return state->getRho(); }
};

#endif // NODE_H
