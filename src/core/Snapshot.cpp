// Implementacion del contenedor inmutable Snapshot para aislar epocas temporales de optimizacion
// Administra los mapas asociativos de nodos microservicios e instancias en el instante temporal t
#include "Snapshot.h"

// Registro de un nuevo nodo de infraestructura en el mapa asociativo del snapshot
void Snapshot::addNode(std::shared_ptr<Node> node) {
    // Valida que el puntero no sea nulo antes de indexarlo por su identificador unico
    if (node) nodes[node->getId()] = node;
}

// Consulta de un nodo especifico a partir de su identificador unico
std::shared_ptr<Node> Snapshot::getNode(const std::string& id) const {
    // Busca en la tabla hash el nodo y retorna su puntero compartido o nullptr si no existe
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second : nullptr;
}

// Retorno de la referencia constante al mapa completo de nodos del cluster
const std::unordered_map<std::string, std::shared_ptr<Node>>& Snapshot::getNodes() const {
    // Expone la coleccion de servidores para iteraciones de solo lectura
    return nodes;
}

// Registro de un microservicio logico en el mapa asociativo del snapshot
void Snapshot::addMicroservice(std::shared_ptr<Microservice> ms) {
    // Valida que el puntero no sea nulo antes de insertarlo en la tabla hash por su ID
    if (ms) microservices[ms->getId()] = ms;
}

// Consulta de un microservicio logico a partir de su identificador unico
std::shared_ptr<Microservice> Snapshot::getMicroservice(const std::string& id) const {
    // Realiza la busqueda por clave y retorna el objeto correspondiente o puntero nulo
    auto it = microservices.find(id);
    return (it != microservices.end()) ? it->second : nullptr;
}

// Retorno de la referencia constante al mapa completo de microservicios logicos
const std::unordered_map<std::string, std::shared_ptr<Microservice>>& Snapshot::getMicroservices() const {
    // Expone la coleccion de microservicios para validacion de separacion de replicas
    return microservices;
}

// Registro de una replica ejecutable en el mapa asociativo de instancias
void Snapshot::addInstance(std::shared_ptr<Instance> inst) {
    // Valida que el puntero no sea nulo antes de insertarlo en el mapa indexado por ID
    if (inst) instances[inst->getId()] = inst;
}

// Consulta de una instancia de microservicio a partir de su identificador unico
std::shared_ptr<Instance> Snapshot::getInstance(const std::string& id) const {
    // Localiza la instancia por su identificador retornando el puntero o nullptr
    auto it = instances.find(id);
    return (it != instances.end()) ? it->second : nullptr;
}

// Retorno de la referencia constante al mapa asociativo de todas las instancias del snapshot
const std::unordered_map<std::string, std::shared_ptr<Instance>>& Snapshot::getInstances() const {
    // Expone el conjunto de replicas a posicionar para los algoritmos de optimizacion
    return instances;
}
