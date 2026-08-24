// Implementacion de las transiciones de estado para los nodos del cluster
// Modela la maquina de estados finita con OperationalState DegradedState y OfflineState
#include "OperationalState.h"
#include "DegradedState.h"
#include "OfflineState.h"
#include "Node.h"

// Transicion de estado operacional ante un evento de falla
void OperationalState::handleFail(Node& node) {
    // Muta el estado del nodo a OfflineState con capacidad disponible igual a cero
    node.setState(std::unique_ptr<NodeState>(new OfflineState()));
}

// Transicion de estado operacional ante un evento de degradacion parcial
void OperationalState::handleDegrade(Node& node, double rho) {
    // Valida que el factor rho este estrictamente entre cero y uno antes de aplicar DegradedState
    if (rho > 0.0 && rho < 1.0) {
        node.setState(std::unique_ptr<NodeState>(new DegradedState(rho)));
    }
}

// Manejo de evento de recuperacion en un nodo que ya se encuentra totalmente operativo
void OperationalState::handleRecover(Node& node) {
    // Operacion nula debido a que el nodo ya posee disponibilidad completa del cien por ciento
    (void)node; 
}

// Transicion de estado degradado ante un evento de falla total
void DegradedState::handleFail(Node& node) {
    // Muta el estado del nodo a OfflineState reduciendo su capacidad disponible a cero
    node.setState(std::unique_ptr<NodeState>(new OfflineState()));
}

// Transicion de estado degradado ante un nuevo evento de degradacion con diferente factor
void DegradedState::handleDegrade(Node& node, double rho) {
    // Actualiza el factor rho reasignando una nueva instancia de DegradedState si el valor es valido
    if (rho > 0.0 && rho < 1.0) {
        node.setState(std::unique_ptr<NodeState>(new DegradedState(rho)));
    }
}

// Transicion de estado degradado ante un evento de recuperacion
void DegradedState::handleRecover(Node& node) {
    // Restaura la operatividad nominal mutando el estado a OperationalState con rho igual a uno
    node.setState(std::unique_ptr<NodeState>(new OperationalState()));
}

// Manejo de evento de falla sobre un nodo que ya se encuentra en estado caido
void OfflineState::handleFail(Node& node) {
    // Operacion nula debido a que el nodo ya esta fuera de servicio
    (void)node;
}

// Manejo de evento de degradacion sobre un nodo que se encuentra fuera de servicio
void OfflineState::handleDegrade(Node& node, double rho) {
    // Operacion no permitida segun el modelo sin antes pasar por una recuperacion explicita
    (void)node;
    (void)rho;
}

// Transicion de estado offline ante un evento de recuperacion
void OfflineState::handleRecover(Node& node) {
    // Restablece el servicio del nodo mutando su estado a OperationalState
    node.setState(std::unique_ptr<NodeState>(new OperationalState()));
}
