#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "Node.h"
#include "Microservice.h"
#include "Instance.h"

/**
 * Clase que representa el snapshot global del sistema en una epoca t
 * Administra unica y exclusivamente los nodos, instancias y microservicios
 */
class Snapshot {
private:
    int timeT;
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
    std::unordered_map<std::string, std::shared_ptr<Microservice>> microservices;
    std::unordered_map<std::string, std::shared_ptr<Instance>> instances;

public:
    explicit Snapshot(int t) : timeT(t) {}

    int getTime() const { return timeT; }

    // Administracion de Nodos
    void addNode(std::shared_ptr<Node> node);
    std::shared_ptr<Node> getNode(const std::string& id) const;
    const std::unordered_map<std::string, std::shared_ptr<Node>>& getNodes() const;

    // Administracion de Microservicios
    void addMicroservice(std::shared_ptr<Microservice> ms);
    std::shared_ptr<Microservice> getMicroservice(const std::string& id) const;
    const std::unordered_map<std::string, std::shared_ptr<Microservice>>& getMicroservices() const;

    // Administracion de Instancias
    void addInstance(std::shared_ptr<Instance> inst);
    std::shared_ptr<Instance> getInstance(const std::string& id) const;
    const std::unordered_map<std::string, std::shared_ptr<Instance>>& getInstances() const;
};

#endif // SNAPSHOT_H
