// Implementacion de metodos para la entidad de infraestructura Node
// Administra la capacidad nominal y delega el comportamiento dinamico al patron State
#include "Node.h"
#include "OperationalState.h"

// Constructor de Node que asigna identificador y capacidad nominal iniciando en estado operativo
Node::Node(const std::string& nodeId, double capacity)
    : id(nodeId), nominalCapacity(capacity), state(std::unique_ptr<NodeState>(new OperationalState())) {
    // Inicializa el puntero de estado polimorfico con una instancia de OperationalState
}

// Falla del nodo delegando la transicion al estado actual mediante el patron State
void Node::fail() {
    state->handleFail(*this);
}

// Degradacion de capacidad del nodo pasando el factor residual rho al estado actual
void Node::degrade(double rho) {
    state->handleDegrade(*this, rho);
}

// Recuperacion del nodo restaurando la operatividad total a traves del estado actual
void Node::recover() {
    state->handleRecover(*this);
}

// Asignacion directa de un nuevo objeto de estado polimorfico al nodo
void Node::setState(std::unique_ptr<NodeState> newState) {
    state = std::move(newState);
}

// Calculo de la capacidad disponible multiplicando la capacidad nominal por el factor rho del estado
double Node::getAvailableCapacity() const {
    return state->getRho() * nominalCapacity;
}
