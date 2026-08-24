#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include "../core/Snapshot.h"

// Tipo de dato para representar la matriz de asignacion X candidata
// Clave principal: ID de la instancia
// Valor: ID del nodo al cual fue asignada para garantizar la asignacion unica
using AssignmentMap = std::unordered_map<std::string, std::string>;

// Clase Validator para evaluar viabilidad y funcion objetivo
class Validator {
public:
    // Retorna true con Z si es valida o false con infinito si viola restricciones
    static std::pair<bool, double> validate(const AssignmentMap& X, const Snapshot& S);
};

#endif // VALIDATOR_H
