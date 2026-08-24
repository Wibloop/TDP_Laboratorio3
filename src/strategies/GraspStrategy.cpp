// Implementacion de la metaheuristica adaptativa GRASP H2 para reconfiguracion de microservicios
// Combina construccion aleatorizada guiada por la lista restringida RCL con busqueda local iterativa
#include "GraspStrategy.h"
#include "LocalSearch.h"
#include <tuple>
#include <vector>
#include <set>
#include <limits>
#include <chrono>
#include <random>
#include <stdexcept>
#include <iostream>
#include <algorithm>

// Constructor que valida y asigna los parametros de configuracion para GRASP
GraspStrategy::GraspStrategy(uint64_t seed_val, double eta_val, double lambda_val) 
    : seed(seed_val), eta(eta_val), lambda(lambda_val) {
    // Validacion de rango para el parametro de aleatoriedad eta
    if (eta < 0.0 || eta > 1.0) {
        throw std::invalid_argument("INVALID_CONFIG: eta debe estar en [0, 1]");
    }
    // Validacion de no negatividad para el parametro lambda de penalizacion de capacidad
    if (lambda < 0.0) {
        throw std::invalid_argument("INVALID_CONFIG: lambda debe estar en [0, +inf)");
    }
}

// Comparador auxiliar para desempatar asignaciones de igual costo mediante orden lexicografico
bool GraspStrategy::isLexicographicallySmaller(const AssignmentMap& a, const AssignmentMap& b) const {
    // Conversion de los mapas a vectores de pares clave valor ordenados
    std::vector<std::pair<std::string, std::string>> vecA(a.begin(), a.end());
    std::vector<std::pair<std::string, std::string>> vecB(b.begin(), b.end());
    std::sort(vecA.begin(), vecA.end());
    std::sort(vecB.begin(), vecB.end());
    return vecA < vecB;
}

// Metodo principal de ejecucion del ciclo GRASP dentro del limite de tiempo especificado
SolveResult GraspStrategy::solve(const Snapshot& snapshot, int time_limit_ms) {
    auto start_time = std::chrono::steady_clock::now();
    const double EPSILON = 1e-6;

    // Inicializacion del generador pseudoaleatorio determinista presembrado por epoca
    std::mt19937_64 rng(seed + snapshot.getTime());

    // Estructura para registrar la mejor solucion factible encontrada
    SolveResult best_solution;
    best_solution.hasSolution = false;
    best_solution.Z = std::numeric_limits<double>::max();

    int total_iterations = 0;
    int feasible_constructions = 0;

    // Precalculo del maximo tamano de estado para normalizacion de la funcion de costo
    double max_s_k = 0.0;
    for (const auto& pair : snapshot.getInstances()) {
        if (pair.second->getStateSize() > max_s_k) {
            max_s_k = pair.second->getStateSize();
        }
    }
    // Salvaguarda para evitar divisiones por cero en caso de instancias sin estado
    if (max_s_k < EPSILON) max_s_k = 1.0;

    // Bucle principal de iteraciones constructivas y busqueda local
    while (true) {
        // Control de limite de tiempo
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (time_limit_ms > 0 && elapsed > time_limit_ms) {
            break;
        }

        total_iterations++;

        // Inicializacion del conjunto de instancias pendientes de ubicacion
        std::set<std::string> U;
        for (const auto& pair : snapshot.getInstances()) {
            U.insert(pair.first);
        }

        // Estructuras de carga y control de replicas por servidor
        std::unordered_map<std::string, double> nodeLoad;
        std::unordered_map<std::string, std::set<std::string>> nodeReplicas;
        for (const auto& pair : snapshot.getNodes()) {
            nodeLoad[pair.first] = 0.0;
        }

        AssignmentMap X;
        bool construction_failed = false;

        // Fase de construccion aleatorizada guiada
        while (!U.empty()) {
            // Seleccion de la instancia mas critica
            std::string best_i = "";
            auto best_i_tuple = std::make_tuple(
                std::numeric_limits<int>::max(),
                0.0, 0.0, std::string("~")
            );

            for (const std::string& i : U) {
                auto inst = snapshot.getInstance(i);
                int feasible_count = 0;
                
                for (const auto& pair : snapshot.getNodes()) {
                    const std::string& j = pair.first;
                    auto node = pair.second;
                    bool capacity_ok = (nodeLoad[j] + inst->getDemand() <= node->getAvailableCapacity() + EPSILON);
                    bool replica_ok = (nodeReplicas[j].find(inst->getMicroserviceId()) == nodeReplicas[j].end());
                    if (capacity_ok && replica_ok) feasible_count++;
                }

                auto current_tuple = std::make_tuple(
                    feasible_count,
                    -inst->getStateSize(),
                    -inst->getDemand(),
                    i
                );

                if (best_i == "" || current_tuple < best_i_tuple) {
                    best_i = i;
                    best_i_tuple = current_tuple;
                }
            }

            // Si no existen opciones factibles se descarta la construccion actual
            if (std::get<0>(best_i_tuple) == 0) {
                construction_failed = true;
                break;
            }

            // Evaluacion de destinos candidatos y calculo de la funcion de seleccion h
            auto inst_star = snapshot.getInstance(best_i);
            std::vector<std::string> F_i;
            std::vector<double> h_values;
            double h_min = std::numeric_limits<double>::max();
            double h_max = -std::numeric_limits<double>::max();

            for (const auto& pair : snapshot.getNodes()) {
                const std::string& j = pair.first;
                auto node = pair.second;
                bool capacity_ok = (nodeLoad[j] + inst_star->getDemand() <= node->getAvailableCapacity() + EPSILON);
                bool replica_ok = (nodeReplicas[j].find(inst_star->getMicroserviceId()) == nodeReplicas[j].end());
                
                if (capacity_ok && replica_ok) {
                    double c_ij = (j == inst_star->getCurrentNodeId()) ? 0.0 : inst_star->getStateSize();
                    double max_cap_term = std::max(1.0, node->getNominalCapacity());
                    double k_j_t = node->getAvailableCapacity();
                    double denominator = std::max(1.0, k_j_t);

                    // Funcion de evaluacion normalizada h combinando costo de migracion y penalizacion de carga
                    double term1 = c_ij / max_s_k;
                    double term2 = lambda * ((k_j_t - nodeLoad[j] - inst_star->getDemand()) / denominator);
                    double h_val = term1 + term2;

                    F_i.push_back(j);
                    h_values.push_back(h_val);
                    if (h_val < h_min) h_min = h_val;
                    if (h_val > h_max) h_max = h_val;
                }
            }

            // Construccion de la Lista Restringida de Candidatos RCL segun el parametro eta
            double threshold = h_min + eta * (h_max - h_min);
            std::vector<std::string> RCL;
            for (size_t k = 0; k < F_i.size(); ++k) {
                if (h_values[k] <= threshold + EPSILON) {
                    RCL.push_back(F_i[k]);
                }
            }

            // Seleccion aleatoria con distribucion uniforme dentro de la lista RCL
            std::uniform_int_distribution<size_t> dist(0, RCL.size() - 1);
            std::string best_j = RCL[dist(rng)];

            // Aplicacion de la asignacion y actualizacion de los recursos consumidos
            X[best_i] = best_j;
            nodeLoad[best_j] += inst_star->getDemand();
            nodeReplicas[best_j].insert(inst_star->getMicroserviceId());
            U.erase(best_i);
        }

        // Si la construccion fue infactible se descarta y continua a la siguiente iteracion
        if (construction_failed) continue;

        feasible_constructions++;

        // Fase de optimizacion local con el tiempo remanente
        auto now = std::chrono::steady_clock::now();
        int time_left = time_limit_ms - std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        if (time_left < 0) time_left = 0;

        AssignmentMap X_opt = LocalSearch::optimize(X, snapshot, time_left);

        // Validacion formal y actualizacion del incumbente si mejora el costo o el orden lexicografico
        auto valResult = Validator::validate(X_opt, snapshot);
        if (valResult.first) {
            bool improves = false;
            if (!best_solution.hasSolution) {
                improves = true;
            } else if (valResult.second < best_solution.Z - EPSILON) {
                improves = true;
            } else if (std::abs(valResult.second - best_solution.Z) <= EPSILON) {
                if (isLexicographicallySmaller(X_opt, best_solution.X)) {
                    improves = true;
                }
            }

            if (improves) {
                best_solution.hasSolution = true;
                best_solution.X = X_opt;
                best_solution.Z = valResult.second;
            }
        }
    }

    // Registro del tiempo total transcurrido
    auto end_time = std::chrono::steady_clock::now();
    best_solution.runtime_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count();

    return best_solution;
}
