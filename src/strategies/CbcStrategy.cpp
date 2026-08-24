// Implementacion del solucionador exacto de referencia R basado en COIN-OR Cbc
// Modela el problema de optimizacion MILP completo y resuelve mediante branch-and-cut
#include "CbcStrategy.h"
#include <OsiClpSolverInterface.hpp>
#include <CbcModel.hpp>
#include <CoinPackedMatrix.hpp>
#include <CoinPackedVector.hpp>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>

// Metodo principal de resolucion exacta con Cbc
SolveResult CbcStrategy::solve(const Snapshot& snapshot, int time_limit_ms) {
    auto start_time = std::chrono::steady_clock::now();
    
    // Preprocesamiento y ordenamiento lexicografico de identificadores para determinismo
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

    // Instanciacion del adaptador de solucionador lineal OSI
    OsiClpSolverInterface solver;
    solver.messageHandler()->setLogLevel(0);

    // Definicion de los coeficientes de la funcion objetivo y cotas de variables
    std::vector<double> obj(numCols, 0.0);
    std::vector<double> colLb(numCols, 0.0);
    std::vector<double> colUb(numCols, 1.0);

    // Asignacion del costo de migracion segun la ubicacion actual de cada replica
    for (int i = 0; i < numInstances; ++i) {
        auto inst = snapshot.getInstance(instanceIds[i]);
        for (int j = 0; j < numNodes; ++j) {
            int k = i * numNodes + j;
            if (nodeIds[j] != inst->getCurrentNodeId()) {
                obj[k] = inst->getStateSize();
            } else {
                obj[k] = 0.0;
            }
        }
    }

    // Construccion de la matriz de restricciones del problema
    CoinPackedMatrix matrix(false, 0, 0);
    std::vector<double> rowLb;
    std::vector<double> rowUb;

    // Restriccion de asignacion unica donde la suma de variables por instancia es igual a uno
    for (int i = 0; i < numInstances; ++i) {
        CoinPackedVector row;
        for (int j = 0; j < numNodes; ++j) {
            int k = i * numNodes + j;
            row.insert(k, 1.0);
        }
        matrix.appendRow(row);
        rowLb.push_back(1.0);
        rowUb.push_back(1.0);
    }

    // Restriccion de capacidad disponible donde la suma de demandas no excede la capacidad del nodo
    for (int j = 0; j < numNodes; ++j) {
        CoinPackedVector row;
        auto node = snapshot.getNode(nodeIds[j]);
        double capacity = node->getAvailableCapacity();
        for (int i = 0; i < numInstances; ++i) {
            auto inst = snapshot.getInstance(instanceIds[i]);
            int k = i * numNodes + j;
            row.insert(k, inst->getDemand());
        }
        matrix.appendRow(row);
        rowLb.push_back(-solver.getInfinity());
        rowUb.push_back(capacity);
    }

    // Restriccion de separacion de replicas donde no pueden coexistir dos replicas del mismo servicio
    for (int j = 0; j < numNodes; ++j) {
        for (const std::string& msId : msIds) {
            CoinPackedVector row;
            bool hasInstances = false;
            for (int i = 0; i < numInstances; ++i) {
                auto inst = snapshot.getInstance(instanceIds[i]);
                if (inst->getMicroserviceId() == msId) {
                    int k = i * numNodes + j;
                    row.insert(k, 1.0);
                    hasInstances = true;
                }
            }
            if (hasInstances) {
                matrix.appendRow(row);
                rowLb.push_back(-solver.getInfinity());
                rowUb.push_back(1.0);
            }
        }
    }

    // Carga de la formulacion matricial en el solucionador OSI
    solver.loadProblem(matrix, colLb.data(), colUb.data(), obj.data(), rowLb.data(), rowUb.data());

    // Configuracion de variables de decision como valores binarios estrictos
    for (int k = 0; k < numCols; ++k) {
        solver.setInteger(k);
    }

    // Configuracion del modelo Cbc y parametrizacion del tiempo limite
    CbcModel model(solver);
    model.setLogLevel(0);
    
    if (time_limit_ms > 0) {
        double seconds = static_cast<double>(time_limit_ms) / 1000.0;
        model.setMaximumSeconds(seconds);
    }

    // Ejecucion del algoritmo exacto de Branch-and-Cut
    model.branchAndBound();

    auto end_time = std::chrono::steady_clock::now();
    double runtime = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count();

    // Extraccion y analisis de la solucion encontrada por Cbc
    if (model.isProvenOptimal() || model.bestSolution() != nullptr) {
        const double* solution = model.bestSolution();
        if (!solution) {
            SolveResult res;
            res.status = "NO_SOLUTION";
            res.runtime_ms = runtime;
            return res;
        }

        // Reconstruccion del mapa de asignacion
        AssignmentMap X;
        for (int i = 0; i < numInstances; ++i) {
            for (int j = 0; j < numNodes; ++j) {
                int k = i * numNodes + j;
                if (solution[k] > 0.5) {
                    X[instanceIds[i]] = nodeIds[j];
                    break;
                }
            }
        }

        // Validacion cruzada y calculo formal del valor objetivo
        auto valResult = Validator::validate(X, snapshot);
        SolveResult res(valResult.first, X, valResult.second);
        
        res.runtime_ms = runtime;
        res.nodes = model.getNodeCount();
        res.lp_calls = model.getIterationCount();
        res.LB = model.getBestPossibleObjValue();
        res.UB = model.getObjValue();

        if (model.isProvenOptimal()) {
            res.status = "OPTIMAL";
            res.gap_percent = 0.0;
        } else {
            res.status = "FEASIBLE";
            double ub_abs = std::abs(res.UB);
            double denom = std::max(1.0, ub_abs);
            res.gap_percent = 100.0 * (res.UB - res.LB) / denom;
        }

        return res;
    }

    // Generacion de resultado infactible o sin incumbente
    SolveResult emptyRes;
    emptyRes.runtime_ms = runtime;
    if (model.isProvenInfeasible()) {
        emptyRes.status = "INFEASIBLE";
    } else {
        emptyRes.status = "NO_INCUMBENT";
    }
    emptyRes.LB = model.getBestPossibleObjValue();
    emptyRes.UB = std::numeric_limits<double>::infinity();
    emptyRes.gap_percent = -1.0;
    
    return emptyRes;
}
