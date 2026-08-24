// Implementacion del modulo de busqueda local para la mejora iterativa de soluciones
// Aplica vecindarios 1-opt RELOCATE y 2-opt SWAP garantizando disminucion estricta del costo
#include "LocalSearch.h"
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>

namespace LocalSearch {

    // Ejecuta la rutina de mejora local iterativa sobre una asignacion factible X
    AssignmentMap optimize(AssignmentMap X, const Snapshot& snapshot, int time_limit_ms) {
        auto start_time = std::chrono::steady_clock::now();
        
        // Obtiene la evaluacion inicial de la asignacion de entrada
        auto currentVal = Validator::validate(X, snapshot);
        if (!currentVal.first) {
            // Si la solucion inicial fuera invalida se retorna sin alterar
            return X;
        }
        double currentZ = currentVal.second;

        // Extrae y ordena los identificadores de instancias para garantizar determinismo absoluto
        std::vector<std::string> instanceIds;
        for (const auto& pair : snapshot.getInstances()) {
            instanceIds.push_back(pair.first);
        }
        std::sort(instanceIds.begin(), instanceIds.end());

        // Extrae y ordena los identificadores de servidores del cluster
        std::vector<std::string> nodeIds;
        for (const auto& pair : snapshot.getNodes()) {
            nodeIds.push_back(pair.first);
        }
        std::sort(nodeIds.begin(), nodeIds.end());

        bool improvementFound = true;

        // Itera mientras se sigan encontrando mejoras estrictas en el costo
        while (improvementFound) {
            improvementFound = false;
            double best_delta = 0.0;
            AssignmentMap best_X_neighbor;

            // Control de tiempo maximo de ejecucion
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            if (time_limit_ms > 0 && elapsed > time_limit_ms) {
                break;
            }

            // Exploracion del vecindario 1-opt RELOCATE(i, j)
            // Reasigna una instancia i a un nuevo servidor j diferente al actual
            for (const std::string& i : instanceIds) {
                std::string originNode = X[i];
                for (const std::string& j : nodeIds) {
                    if (originNode == j) continue;

                    AssignmentMap neighborX = X;
                    neighborX[i] = j;

                    // Evaluacion de factibilidad y calculo de variacion de costo delta
                    auto val = Validator::validate(neighborX, snapshot);
                    if (val.first) {
                        double delta = val.second - currentZ;
                        // Criterio de mayor reduccion estricta de costo con desempate por primer movimiento
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_X_neighbor = neighborX;
                        }
                    }
                }
            }

            // Exploracion del vecindario 2-opt SWAP(i, k)
            // Intercambia las ubicaciones de dos instancias i y k ubicadas en distintos servidores
            for (size_t idx_i = 0; idx_i < instanceIds.size(); ++idx_i) {
                const std::string& i = instanceIds[idx_i];
                std::string originNodeI = X[i];

                for (size_t idx_k = idx_i + 1; idx_k < instanceIds.size(); ++idx_k) {
                    const std::string& k = instanceIds[idx_k];
                    std::string originNodeK = X[k];

                    // Si ambas instancias ya residen en el mismo nodo el intercambio no produce cambios
                    if (originNodeI == originNodeK) continue;

                    AssignmentMap neighborX = X;
                    neighborX[i] = originNodeK;
                    neighborX[k] = originNodeI;

                    // Evaluacion de factibilidad y calculo de variacion de costo delta
                    auto val = Validator::validate(neighborX, snapshot);
                    if (val.first) {
                        double delta = val.second - currentZ;
                        // Mismo criterio de dominancia estricta
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_X_neighbor = neighborX;
                        }
                    }
                }
            }

            // Aplicacion del mejor movimiento si reduce estrictamente la funcion objetivo
            if (best_delta < -1e-6) {
                X = best_X_neighbor;
                currentZ += best_delta;
                improvementFound = true;
            }
        }

        return X;
    }

}
