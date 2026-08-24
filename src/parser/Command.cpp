// Implementacion de los comandos del patron Command para la aplicacion de eventos en el cluster
// Encapsula las acciones atomicas de falla degradacion y recuperacion sobre los nodos del snapshot
#include "Command.h"
#include <iostream>

// Ejecucion del comando de falla sobre el nodo objetivo
void FailCommand::execute(Snapshot& snapshot) {
    // Busca el puntero compartido del nodo en el snapshot actual
    auto node = snapshot.getNode(nodeId);
    if (node) {
        // Ejecuta la transicion a OfflineState en el nodo
        node->fail();
    } else {
        // Notifica por flujo de error si el identificador del servidor no existe en el cluster
        std::cerr << "Error: FailCommand sobre nodo inexistente " << nodeId << std::endl;
    }
}

// Ejecucion del comando de degradacion sobre el nodo objetivo con factor de capacidad residual
void DegradeCommand::execute(Snapshot& snapshot) {
    // Busca el puntero compartido del nodo en el snapshot actual
    auto node = snapshot.getNode(nodeId);
    if (node) {
        // Ejecuta la transicion a DegradedState aplicando el nuevo factor rho
        node->degrade(rho);
    } else {
        // Notifica por flujo de error si el identificador del servidor no existe en el cluster
        std::cerr << "Error: DegradeCommand sobre nodo inexistente " << nodeId << std::endl;
    }
}

// Ejecucion del comando de recuperacion restaurando la operatividad completa del nodo
void RecoverCommand::execute(Snapshot& snapshot) {
    // Busca el puntero compartido del nodo en el snapshot actual
    auto node = snapshot.getNode(nodeId);
    if (node) {
        // Restablece el estado a OperationalState con disponibilidad completa
        node->recover();
    } else {
        // Notifica por flujo de error si el identificador del servidor no existe en el cluster
        std::cerr << "Error: RecoverCommand sobre nodo inexistente " << nodeId << std::endl;
    }
}
