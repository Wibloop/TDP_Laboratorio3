// Implementacion del solucionador exacto propio Branch-and-Bound A0 apoyado en ClpSimplex
// Administra el arbol de busqueda mediante cola de prioridad Best-Bound y heuristica RepararDesdeLP
#include "ClpBBStrategy.h"
#include <ClpSimplex.hpp>
#include <CoinPackedMatrix.hpp>
#include <CoinPackedVector.hpp>
#include <queue>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>

// Adaptador para resolver la relajacion lineal del modelo mediante ClpSimplex
LPResult ClpBBStrategy::ResolverLP(ClpSimplex& model, bool isRoot) {
    // Desactiva la salida por consola del solucionador lineal
    model.setLogLevel(0);

    // En la raiz se ejecuta resolucion dual completa y en los nodos hijos resolucion dual incremental
    if (isRoot) {
        model.initialDualSolve();
    } else {
        model.dual();
    }

    LPResult res;
    res.status = model.status();
    res.objective = model.objectiveValue();
    res.iterations = model.getIterationCount();

    // Copia profunda inmediata de la solucion primal para evitar referencias invalidas de memoria
    if (res.status == 0) {
        const double* primalArr = model.primalColumnSolution();
        int numCols = model.getNumCols();
        if (primalArr) {
            res.primal.assign(primalArr, primalArr + numCols);
        }
    }
    return res;
}

// Heuristica constructiva para reparar una solucion entera a partir del vector primal relajado
SolveResult ClpBBStrategy::RepararDesdeLP(const std::vector<double>& primal, const Snapshot& snapshot,
                                          const std::vector<std::string>& instanceIds,
                                          const std::vector<std::string>& nodeIds) {
    const double EPS = 1e-6;
    int numInstances = instanceIds.size();
    int numNodes = nodeIds.size();

    // Estructura de ordenamiento para candidatos prefiriendo mayor valor fraccionario
    struct AssignCandidate {
        int i;
        int j;
        double val;
        bool operator<(const AssignCandidate& other) const {
            return val > other.val;
        }
    };

    // Extrae y ordena descendentemente las asignaciones con soporte fraccionario positivo
    std::vector<AssignCandidate> candidates;
    for (int i = 0; i < numInstances; ++i) {
        for (int j = 0; j < numNodes; ++j) {
            int k = i * numNodes + j;
            if (primal[k] > EPS) {
                candidates.push_back({i, j, primal[k]});
            }
        }
    }
    std::sort(candidates.begin(), candidates.end());

    AssignmentMap X;
    std::unordered_map<std::string, double> nodeLoad;
    std::unordered_map<std::string, std::set<std::string>> nodeReplicas;

    for (const auto& nid : nodeIds) nodeLoad[nid] = 0.0;

    std::set<int> unassigned;
    for (int i = 0; i < numInstances; ++i) unassigned.insert(i);

    // Asignacion golosa de instancias priorizadas por la relajacion LP
    for (const auto& cand : candidates) {
        if (unassigned.find(cand.i) == unassigned.end()) continue;

        auto inst = snapshot.getInstance(instanceIds[cand.i]);
        auto node = snapshot.getNode(nodeIds[cand.j]);

        bool cap_ok = (nodeLoad[nodeIds[cand.j]] + inst->getDemand() <= node->getAvailableCapacity() + EPS);
        bool rep_ok = (nodeReplicas[nodeIds[cand.j]].find(inst->getMicroserviceId()) == nodeReplicas[nodeIds[cand.j]].end());

        if (cap_ok && rep_ok) {
            X[instanceIds[cand.i]] = nodeIds[cand.j];
            nodeLoad[nodeIds[cand.j]] += inst->getDemand();
            nodeReplicas[nodeIds[cand.j]].insert(inst->getMicroserviceId());
            unassigned.erase(cand.i);
        }
    }

    // Reinsercion factible para instancias no ubicadas
    for (int i : unassigned) {
        auto inst = snapshot.getInstance(instanceIds[i]);
        bool assigned = false;
        for (const std::string& j : nodeIds) {
            auto node = snapshot.getNode(j);
            bool cap_ok = (nodeLoad[j] + inst->getDemand() <= node->getAvailableCapacity() + EPS);
            bool rep_ok = (nodeReplicas[j].find(inst->getMicroserviceId()) == nodeReplicas[j].end());

            if (cap_ok && rep_ok) {
                X[instanceIds[i]] = j;
                nodeLoad[j] += inst->getDemand();
                nodeReplicas[j].insert(inst->getMicroserviceId());
                assigned = true;
                break;
            }
        }
        if (!assigned) {
            return SolveResult();
        }
    }

    // Validacion de la solucion reparada y retorno de resultado
    auto valResult = Validator::validate(X, snapshot);
    return SolveResult(valResult.first, X, valResult.second);
}

// Metodo principal del algoritmo Branch-and-Bound A0
SolveResult ClpBBStrategy::solve(const Snapshot& snapshot, int time_limit_ms) {
    auto start_time = std::chrono::steady_clock::now();
    const double EPS = 1e-6;

    // Preprocesamiento y ordenamiento lexicografico de identificadores
    std::vector<std::string> instanceIds;
    for (const auto& pair : snapshot.getInstances()) instanceIds.push_back(pair.first);
    std::sort(instanceIds.begin(), instanceIds.end());

    std::vector<std::string> nodeIds;
    for (const auto& pair : snapshot.getNodes()) nodeIds.push_back(pair.first);
    std::sort(nodeIds.begin(), nodeIds.end());

    std::vector<std::string> msIds;
    for (const auto& pair : snapshot.getMicroservices()) msIds.push_back(pair.first);
    std::sort(msIds.begin(), msIds.end());

    int numInstances = instanceIds.size();
    int numNodes = nodeIds.size();
    int numCols = numInstances * numNodes;

    if (numCols == 0) return SolveResult();

    // Construccion de la matriz de coeficientes y cotas de variables una sola vez
    std::vector<double> obj(numCols, 0.0);
    std::vector<double> colLb(numCols, 0.0);
    std::vector<double> colUb(numCols, 1.0);

    for (int i = 0; i < numInstances; ++i) {
        auto inst = snapshot.getInstance(instanceIds[i]);
        for (int j = 0; j < numNodes; ++j) {
            int k = i * numNodes + j;
            obj[k] = (nodeIds[j] != inst->getCurrentNodeId()) ? inst->getStateSize() : 0.0;
        }
    }

    CoinPackedMatrix matrix(false, 0, 0);
    std::vector<double> rowLb;
    std::vector<double> rowUb;

    // Restriccion de asignacion unica
    for (int i = 0; i < numInstances; ++i) {
        CoinPackedVector row;
        for (int j = 0; j < numNodes; ++j) {
            row.insert(i * numNodes + j, 1.0);
        }
        matrix.appendRow(row);
        rowLb.push_back(1.0);
        rowUb.push_back(1.0);
    }

    // Restriccion de capacidad disponible por servidor
    for (int j = 0; j < numNodes; ++j) {
        CoinPackedVector row;
        double capacity = snapshot.getNode(nodeIds[j])->getAvailableCapacity();
        for (int i = 0; i < numInstances; ++i) {
            row.insert(i * numNodes + j, snapshot.getInstance(instanceIds[i])->getDemand());
        }
        matrix.appendRow(row);
        rowLb.push_back(-1e30);
        rowUb.push_back(capacity);
    }

    // Restriccion de separacion de replicas
    for (int j = 0; j < numNodes; ++j) {
        for (const std::string& msId : msIds) {
            CoinPackedVector row;
            bool hasInst = false;
            for (int i = 0; i < numInstances; ++i) {
                if (snapshot.getInstance(instanceIds[i])->getMicroserviceId() == msId) {
                    row.insert(i * numNodes + j, 1.0);
                    hasInst = true;
                }
            }
            if (hasInst) {
                matrix.appendRow(row);
                rowLb.push_back(-1e30);
                rowUb.push_back(1.0);
            }
        }
    }

    // Carga del modelo en la instancia de ClpSimplex
    ClpSimplex model;
    model.loadProblem(matrix, colLb.data(), colUb.data(), obj.data(), rowLb.data(), rowUb.data());

    // Administracion del arbol mediante cola de prioridad Best-Bound
    std::priority_queue<BBNode> OPEN;
    int nextId = 0;
    
    BBNode root = {nextId++, 0, 0.0, {}};
    OPEN.push(root);

    double UB = 1e30;
    SolveResult best_solution;

    int total_nodes = 0;
    int lp_calls = 0;

    // Bucle de exploracion del arbol Branch-and-Bound
    while (!OPEN.empty()) {
        // Control de tiempo maximo de ejecucion
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        if (time_limit_ms > 0 && elapsed > time_limit_ms) break;

        BBNode current = OPEN.top();
        OPEN.pop();
        total_nodes++;

        // Poda por cota inferior frente al incumbente actual
        if (current.lb >= UB - EPS) continue;

        // Modificacion ligera de limites de variables fijadas en el nodo actual
        for (int k = 0; k < numCols; ++k) {
            auto it = current.fixedVariables.find(k);
            if (it != current.fixedVariables.end()) {
                model.setColBounds(k, it->second, it->second);
            } else {
                model.setColBounds(k, 0.0, 1.0);
            }
        }

        // Resolucion de la relajacion lineal del subproblema
        LPResult res = ResolverLP(model, current.depth == 0);
        lp_calls++;

        // Poda por infactibilidad si el solver dual no encuentra solucion factible
        if (res.status != 0) continue;

        // Poda por cota con el valor objetivo real obtenido
        if (res.objective >= UB - EPS) continue;

        // Heuristica de reparacion para obtener cotas superiores tempranas
        SolveResult repRes = RepararDesdeLP(res.primal, snapshot, instanceIds, nodeIds);
        if (repRes.hasSolution && repRes.Z < UB - EPS) {
            UB = repRes.Z;
            best_solution = repRes;
        }

        // Verificacion de integralidad y seleccion de variable de ramificacion VARIABLEBRANCH
        bool isInteger = true;
        int best_k = -1;
        double min_dist = 1.0;
        int best_i = numInstances, best_j = numNodes;

        for (int i = 0; i < numInstances; ++i) {
            for (int j = 0; j < numNodes; ++j) {
                int k = i * numNodes + j;
                double val = res.primal[k];
                double dist = std::abs(val - 0.5);
                
                if (val > EPS && val < 1.0 - EPS) {
                    isInteger = false;
                    
                    // Regla VARIABLEBRANCH minimizando distancia a 0.5 con desempate por menor i y menor j
                    if (dist < min_dist - EPS || 
                       (std::abs(dist - min_dist) <= EPS && i < best_i) ||
                       (std::abs(dist - min_dist) <= EPS && i == best_i && j < best_j)) {
                        
                        min_dist = dist;
                        best_k = k;
                        best_i = i;
                        best_j = j;
                    }
                }
            }
        }

        // Si la solucion es completamente entera se actualiza el incumbente
        if (isInteger) {
            if (res.objective < UB - EPS) {
                UB = res.objective;
                
                AssignmentMap X;
                for (int i = 0; i < numInstances; ++i) {
                    for (int j = 0; j < numNodes; ++j) {
                        int k = i * numNodes + j;
                        if (res.primal[k] > 0.5) X[instanceIds[i]] = nodeIds[j];
                    }
                }
                auto valResult = Validator::validate(X, snapshot);
                best_solution = SolveResult(valResult.first, X, valResult.second);
            }
        } else {
            // Ramificacion generando dos nodos hijos con la variable fijada a cero y a uno
            BBNode child0 = current;
            child0.id = nextId++;
            child0.depth++;
            child0.lb = res.objective;
            child0.fixedVariables[best_k] = 0.0;
            OPEN.push(child0);

            BBNode child1 = current;
            child1.id = nextId++;
            child1.depth++;
            child1.lb = res.objective;
            child1.fixedVariables[best_k] = 1.0;
            OPEN.push(child1);
        }
    }

    // Calculo del tiempo de ejecucion total
    auto end_time = std::chrono::steady_clock::now();
    best_solution.runtime_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count();
    
    // Determinacion de la cota inferior final a partir del minimo LB de la frontera abierta
    double final_LB = UB;
    if (!OPEN.empty()) {
        final_LB = std::min(final_LB, OPEN.top().lb);
    }
    
    best_solution.LB = final_LB;
    best_solution.UB = UB;
    best_solution.nodes = total_nodes;
    best_solution.lp_calls = lp_calls;

    // Asignacion de estados normativos y calculo del porcentaje de brecha gap
    if (best_solution.hasSolution) {
        if (OPEN.empty()) {
            best_solution.status = "OPTIMAL";
            best_solution.gap_percent = 0.0;
        } else {
            best_solution.status = "FEASIBLE";
            double ub_abs = std::abs(UB);
            double denom = std::max(1.0, ub_abs);
            best_solution.gap_percent = 100.0 * (UB - final_LB) / denom;
        }
    } else {
        if (OPEN.empty()) {
            best_solution.status = "INFEASIBLE";
        } else {
            best_solution.status = "NO_INCUMBENT";
        }
        best_solution.LB = final_LB;
        best_solution.UB = std::numeric_limits<double>::infinity();
        best_solution.gap_percent = -1.0;
    }

    return best_solution;
}
