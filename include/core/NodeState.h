#ifndef NODESTATE_H
#define NODESTATE_H

#include <string>

// Declaracion adelantada para evitar dependencias circulares
class Node;

/**
 * Interfaz base para el patron State de los nodos
 * Define el comportamiento comun y las transiciones permitidas
 */
class NodeState {
public:
    virtual ~NodeState() = default;

    // Transicion hacia el estado FALLIDO
    virtual void handleFail(Node& node) = 0;

    // Transicion hacia el estado DEGRADADO
    // Recibe el nuevo valor de rho (factor de capacidad)
    virtual void handleDegrade(Node& node, double rho) = 0;

    // Transicion hacia el estado OPERACIONAL
    virtual void handleRecover(Node& node) = 0;

    // Obtener el factor de capacidad actual (0.0 a 1.0)
    virtual double getRho() const = 0;

    // Obtener el nombre del estado para reportes
    virtual std::string getName() const = 0;
};

#endif // NODESTATE_H
