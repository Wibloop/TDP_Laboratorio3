#ifndef MICROSERVICE_H
#define MICROSERVICE_H

#include <string>

/**
 * Clase que representa un Microservicio logico en el sistema
 * Ahora agrupa a las instancias (replicas) y ya no posee la demanda directamente
 */
class Microservice {
private:
    std::string id;

public:
    explicit Microservice(const std::string& msId) : id(msId) {}

    std::string getId() const { return id; }
};

#endif // MICROSERVICE_H
