#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>

/**
 * Clase que representa una instancia de un Microservicio
 * Posee demanda propia y un tamaño de estado de transferencia
 */
class Instance {
private:
    std::string id;
    std::string microserviceId; // ID del microservicio padre (para relacion de replicas)
    std::string currentNodeId;  // Nodo donde esta ubicada actualmente (p_i^t)
    double demand;              // Demanda requerida (d_i)
    double stateSize;           // Tamaño del estado transferido (s_i)

public:
    Instance(const std::string& instId, const std::string& msId, const std::string& nodeId, double d, double s)
        : id(instId), microserviceId(msId), currentNodeId(nodeId), demand(d), stateSize(s) {}

    // Getters
    std::string getId() const { return id; }
    std::string getMicroserviceId() const { return microserviceId; }
    std::string getCurrentNodeId() const { return currentNodeId; }
    double getDemand() const { return demand; }
    double getStateSize() const { return stateSize; }

    // Actualizar nodo, por ejemplo despues de una migracion efectiva
    void setCurrentNodeId(const std::string& nodeId) { currentNodeId = nodeId; }
};

#endif // INSTANCE_H
