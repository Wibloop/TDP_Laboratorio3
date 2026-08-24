#include "Validator.h"
#include <limits>
#include <unordered_set>
#include <iostream>

std::pair<bool, double> Validator::validate(const AssignmentMap& X, const Snapshot& S) {
    const double EPSILON_FEAS = 1e-6;
    double Z = 0.0;

    // Mapa para acumular la demanda total asignada a cada nodo
    std::unordered_map<std::string, double> nodeUsedCapacity;

    // Mapa para llevar registro de que microservicios estan en que nodos
    // Key: NodeID, Value: Set of MicroserviceIDs
    std::unordered_map<std::string, std::unordered_set<std::string>> nodeMicroservices;

    // Recorrer todas las instancias esperadas en el Snapshot S
    for (const auto& pair : S.getInstances()) {
        const std::string& instId = pair.first;
        auto inst = pair.second;

        // El AssignmentMap garantiza una ubicacion por instancia y verificamos que este en el mapa
        auto it = X.find(instId);
        if (it == X.end()) {
            // Instancia no asignada en la matriz candidata
            return {false, std::numeric_limits<double>::infinity()};
        }

        const std::string& targetNodeId = it->second;

        // Validar que el nodo destino existe en el Snapshot
        auto targetNode = S.getNode(targetNodeId);
        if (!targetNode) {
            return {false, std::numeric_limits<double>::infinity()};
        }

        // Calcular funcion objetivo Zt con volumen migrado
        // Ecuacion 1: coeficientes c_ij^t
        if (targetNodeId != inst->getCurrentNodeId()) {
            // Si la instancia se movio de p_i^t a un j distinto, su coeficiente es s_i
            Z += inst->getStateSize();
        }

        // Acumular demandas para Restriccion 3c
        nodeUsedCapacity[targetNodeId] += inst->getDemand();

        // Validar Restriccion 3d: Separacion de replicas (mismo microservicio en distinto nodo)
        auto& msSet = nodeMicroservices[targetNodeId];
        const std::string& msId = inst->getMicroserviceId();
        
        if (msSet.find(msId) != msSet.end()) {
            // Ya hay otra replica de este microservicio en este mismo nodo
            return {false, std::numeric_limits<double>::infinity()};
        }
        msSet.insert(msId);
    }

    // Validar Restriccion 3c: Capacidad (sumatoria de demandas d_i <= K_j^t + epsilon)
    for (const auto& pair : S.getNodes()) {
        const std::string& nodeId = pair.first;
        auto node = pair.second;
        
        double usedCap = nodeUsedCapacity[nodeId];
        double availCap = node->getAvailableCapacity();
        
        if (usedCap > availCap + EPSILON_FEAS) {
            // Sobrepasa la capacidad disponible
            return {false, std::numeric_limits<double>::infinity()};
        }
    }

    // Pasa todas las validaciones
    return {true, Z};
}
