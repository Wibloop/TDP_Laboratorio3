// Implementacion de la estrategia heuristica Golosa Estable H1 con optimizacion local
// Construye una asignacion inicial determinista minimizando tuplas lexicograficas y aplica busqueda local
#include "GreedyStableStrategy.h"
#include "LocalSearch.h"
#include <tuple>
#include <vector>
#include <set>
#include <string>
#include <limits>
#include <chrono>

// Metodo principal de resolucion para la estrategia H1
SolveResult GreedyStableStrategy::solve(const Snapshot& snapshot, int time_limit_ms) {
    auto start_time = std::chrono::steady_clock::now();
    
    // Fase constructiva para generar la solucion inicial determinista
    SolveResult initial = buildInitialSolution(snapshot, time_limit_ms);
    if (!initial.hasSolution) {
        // Retorna sin solucion si no fue posible encontrar una asignacion factible
        return initial;
    }

    // Fase de mejora mediante busqueda local explorando vecindarios RELOCATE y SWAP
    AssignmentMap optimizedX = LocalSearch::optimize(initial.X, snapshot, time_limit_ms);
    
    // Evaluacion y validacion independiente de la solucion optimizada
    auto valResult = Validator::validate(optimizedX, snapshot);
    auto end_time = std::chrono::steady_clock::now();
    double runtime = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count();

    // Construccion de la estructura de resultado final con el tiempo de ejecucion
    SolveResult final_res(valResult.first, optimizedX, valResult.second);
    final_res.runtime_ms = runtime;
    return final_res;
}

// Construccion golosa de la asignacion inicial guiada por criticidad y desempate lexicografico
SolveResult GreedyStableStrategy::buildInitialSolution(const Snapshot& snapshot, int time_limit_ms) {
    auto start_time = std::chrono::steady_clock::now();
    const double EPSILON = 1e-6;

    // Conjunto U de identificadores de instancias pendientes de ubicacion
    std::set<std::string> U;
    for (const auto& pair : snapshot.getInstances()) {
        U.insert(pair.first);
    }

    // Seguimiento del consumo de capacidad y microservicios alojados por servidor
    std::unordered_map<std::string, double> nodeLoad;
    std::unordered_map<std::string, std::set<std::string>> nodeReplicas;
    
    // Inicializacion de la carga en cero para todos los servidores del cluster
    for (const auto& pair : snapshot.getNodes()) {
        nodeLoad[pair.first] = 0.0;
    }

    // Mapa de asignacion resultante
    AssignmentMap X;

    // Bucle iterativo hasta ubicar todas las instancias del conjunto U
    while (!U.empty()) {
        // Control de tiempo maximo de ejecucion por epoca
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (time_limit_ms > 0 && elapsed > time_limit_ms) {
            return SolveResult();
        }

        std::string best_i = "";
        // Inicializacion de la tupla de comparacion con valores infinitos para minimizar
        auto best_i_tuple = std::make_tuple(
            std::numeric_limits<int>::max(),
            0.0,
            0.0,
            std::string("~")
        );

        // Seleccion de la instancia mas critica minimizando la tupla lexicografica
        for (const std::string& i : U) {
            auto inst = snapshot.getInstance(i);
            int feasible_count = 0;
            
            // Conteo de servidores factibles que satisfacen capacidad y no colision de replicas
            for (const auto& pair : snapshot.getNodes()) {
                const std::string& j = pair.first;
                auto node = pair.second;
                
                bool capacity_ok = (nodeLoad[j] + inst->getDemand() <= node->getAvailableCapacity() + EPSILON);
                bool replica_ok = (nodeReplicas[j].find(inst->getMicroserviceId()) == nodeReplicas[j].end());
                
                if (capacity_ok && replica_ok) {
                    feasible_count++;
                }
            }

            // Tupla de ordenamiento con menor cantidad de destinos mayor estado mayor demanda y menor ID
            auto current_tuple = std::make_tuple(
                feasible_count,
                -inst->getStateSize(),
                -inst->getDemand(),
                i
            );

            // Comparacion lexicografica estricta nativa de C++
            if (best_i == "" || current_tuple < best_i_tuple) {
                best_i = i;
                best_i_tuple = current_tuple;
            }
        }

        // Si la instancia mas critica tiene cero destinos factibles el problema no tiene solucion factible
        if (std::get<0>(best_i_tuple) == 0) {
            return SolveResult();
        }

        // Seleccion del mejor servidor destino para la instancia critica elegida
        auto inst_star = snapshot.getInstance(best_i);
        std::string best_j = "";
        
        auto best_j_tuple = std::make_tuple(
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max(),
            std::string("~")
        );

        // Evaluacion de los servidores factibles para ubicar la instancia
        for (const auto& pair : snapshot.getNodes()) {
            const std::string& j = pair.first;
            auto node = pair.second;
            
            bool capacity_ok = (nodeLoad[j] + inst_star->getDemand() <= node->getAvailableCapacity() + EPSILON);
            bool replica_ok = (nodeReplicas[j].find(inst_star->getMicroserviceId()) == nodeReplicas[j].end());
            
            if (capacity_ok && replica_ok) {
                // Costo de migracion nulo si permanece en su nodo de origen o igual al estado si migra
                double c_ij = (j == inst_star->getCurrentNodeId()) ? 0.0 : inst_star->getStateSize();
                double residual_capacity = node->getAvailableCapacity() - nodeLoad[j] - inst_star->getDemand();

                // Tupla de seleccion minimizando costo de transferencia y capacidad residual
                auto current_j_tuple = std::make_tuple(c_ij, residual_capacity, j);
                
                if (best_j == "" || current_j_tuple < best_j_tuple) {
                    best_j = j;
                    best_j_tuple = current_j_tuple;
                }
            }
        }

        // Fijacion de la asignacion y actualizacion de estructuras de control
        X[best_i] = best_j;
        nodeLoad[best_j] += inst_star->getDemand();
        nodeReplicas[best_j].insert(inst_star->getMicroserviceId());
        U.erase(best_i);
    }

    // Validacion y calculo de la funcion objetivo de la solucion inicial
    auto valResult = Validator::validate(X, snapshot);
    return SolveResult(valResult.first, X, valResult.second);
}
