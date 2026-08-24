#include <iostream>
#include <memory>
#include <iomanip>
#include <string>
#include <fstream>
#include "../include/parser/RDMParser.h"
#include "../include/parser/Command.h"
#include "../include/core/Snapshot.h"
#include "../include/core/Exporter.h"
#include "../include/solvers/Validator.h"
#include "../include/strategies/GreedyStableStrategy.h"
#include "../include/strategies/GraspStrategy.h"
#include "../include/strategies/CbcStrategy.h"
#include "../include/strategies/ClpBBStrategy.h"

using namespace std;

// Imprimir estado del ecosistema
void printSystemState(const Snapshot& snapshot) {
    cout << "Ecosistema Global en t=" << snapshot.getTime() << endl;
    cout << "Nodos:" << endl;
    for (const auto& pair : snapshot.getNodes()) {
        auto node = pair.second;
        cout << " - ID: " << node->getId() << " | Estado: " << node->getStateName()
             << " | rho: " << node->getCurrentRho() << " | Disp: " << node->getAvailableCapacity()
             << " / " << node->getNominalCapacity() << endl;
    }
    cout << "\nInstancias:" << endl;
    for (const auto& pair : snapshot.getInstances()) {
        auto inst = pair.second;
        cout << " - Instancia: " << inst->getId() << " (MS: " << inst->getMicroserviceId() << ")"
             << " | Nodo p_i^t: " << inst->getCurrentNodeId() << " | Demanda: " << inst->getDemand()
             << " | Size: " << inst->getStateSize() << endl;
    }
}

// Consolidar la mejor solucion en el snapshot para la proxima epoca
Snapshot consolidarMejorSolucion(const Snapshot& actual, const SolveResult& best_res, int nueva_epoca) {
    Snapshot proximo(nueva_epoca);
    
    // Nodos se mantienen
    for (const auto& pair : actual.getNodes()) {
        auto node = pair.second;
        // Clonamos el nodo para el nuevo snapshot
        auto newNode = std::make_shared<Node>(node->getId(), node->getNominalCapacity());
        if (node->getStateName() == "DEGRADED") newNode->degrade(node->getCurrentRho());
        else if (node->getStateName() == "OFFLINE") newNode->fail();
        proximo.addNode(newNode);
    }

    // Microservicios se mantienen
    for (const auto& pair : actual.getMicroservices()) {
        proximo.addMicroservice(std::make_shared<Microservice>(pair.first));
    }

    // Instancias se mantienen, pero p_i^{t+1} se actualiza a su ubicacion efectiva X[i]
    for (const auto& pair : actual.getInstances()) {
        auto inst = pair.second;
        std::string new_node_id = inst->getCurrentNodeId();
        
        if (best_res.hasSolution) {
            auto it = best_res.X.find(inst->getId());
            if (it != best_res.X.end()) {
                new_node_id = it->second;
            }
        }

        auto newInst = std::make_shared<Instance>(inst->getId(), inst->getMicroserviceId(), 
                                                  new_node_id, inst->getDemand(), inst->getStateSize());
        proximo.addInstance(newInst);
    }

    return proximo;
}

void printMenu() {
    cout << "\n============================================================" << endl;
    cout << "RECONFIGURACION DE MICROSERVICIOS ANTE FALLAS (RMF)" << endl;
    cout << "============================================================" << endl;
    cout << "1. Cargar caso .rdm" << endl;
    cout << "2. Mostrar y validar estado inicial" << endl;
    cout << "3. Ejecutar una epoca con A0, H1, H2 y R" << endl;
    cout << "4. Ejecutar la secuencia completa" << endl;
    cout << "5. Exportar resultados CSV" << endl;
    cout << "6. Salir" << endl;
    cout << "============================================================" << endl;
    cout << "Opcion: ";
}

int main() {
    Snapshot snapshot(0);
    bool casoCargado = false;
    bool csvActivado = false;
    ofstream csvFile;

    int epoch = 1;

    while (true) {
        printMenu();
        int opcion;
        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (opcion == 6) {
            cout << "Saliendo" << endl;
            if (csvFile.is_open()) csvFile.close();
            break;
        }

        if (opcion == 1) {
            cout << "Ruta del archivo (ej. instances/demo_fallas.rdm): ";
            string path;
            cin >> path;
            try {
                snapshot = RDMParser::parse(path, 0);
                casoCargado = true;
                epoch = 1;
                cout << "[OK] Caso cargado exitosamente" << endl;
            } catch (const exception& e) {
                cout << "[ERROR] " << e.what() << endl;
            }
        } 
        else if (opcion == 2) {
            if (!casoCargado) { cout << "Cargue un caso primero" << endl; continue; }
            printSystemState(snapshot);
        }
        else if (opcion == 3 || opcion == 4) {
            if (!casoCargado) { cout << "Cargue un caso primero" << endl; continue; }
            
            int numEpocas = (opcion == 3) ? 1 : 3; // Simulamos 3 epocas si es secuencia
            
            for (int e = 0; e < numEpocas; ++e) {
                if (numEpocas > 1) {
                    cout << "\nEPOCA " << epoch << " (t=" << snapshot.getTime() << ")" << endl;
                    // Simulamos eventos en las siguientes epocas
                    if (e == 1) { FailCommand("N1").execute(snapshot); }
                    if (e == 2) { RecoverCommand("N1").execute(snapshot); DegradeCommand("N3", 0.5).execute(snapshot); }
                }

                SolveResult bestGlobal;
                bestGlobal.Z = std::numeric_limits<double>::infinity();

                // Ejecucion de H1
                GreedyStableStrategy h1;
                SolveResult resH1 = h1.solve(snapshot, 2000);
                Exporter::reportAndSave(epoch, snapshot.getTime(), "H1", resH1, snapshot, csvFile);
                if (resH1.hasSolution && resH1.Z < bestGlobal.Z) bestGlobal = resH1;

                // Ejecucion de H2
                try {
                    GraspStrategy h2(42, 0.3, 1.0);
                    SolveResult resH2 = h2.solve(snapshot, 3000);
                    Exporter::reportAndSave(epoch, snapshot.getTime(), "H2", resH2, snapshot, csvFile);
                    if (resH2.hasSolution && resH2.Z < bestGlobal.Z) bestGlobal = resH2;
                } catch(const exception& ex) {
                    SolveResult resErr; resErr.status = "INVALID_CONFIG";
                    Exporter::reportAndSave(epoch, snapshot.getTime(), "H2", resErr, snapshot, csvFile);
                }

                // Ejecucion de A0
                ClpBBStrategy a0;
                SolveResult resA0 = a0.solve(snapshot, 10000);
                Exporter::reportAndSave(epoch, snapshot.getTime(), "A0", resA0, snapshot, csvFile);
                if (resA0.hasSolution && resA0.Z < bestGlobal.Z) bestGlobal = resA0;

                // Ejecucion de R
                CbcStrategy cbc;
                SolveResult resR = cbc.solve(snapshot, 10000);
                Exporter::reportAndSave(epoch, snapshot.getTime(), "R", resR, snapshot, csvFile);
                if (resR.hasSolution && resR.Z < bestGlobal.Z) bestGlobal = resR;

                if (numEpocas > 1) {
                    // Consolidar la mejor solucion encontrada para la proxima epoca
                    snapshot = consolidarMejorSolucion(snapshot, bestGlobal, snapshot.getTime() + 10);
                    epoch++;
                }
            }
        }
        else if (opcion == 5) {
            if (csvActivado) {
                csvFile.close();
                csvActivado = false;
                cout << "[CSV] Exportacion desactivada" << endl;
            } else {
                csvFile.open("results/metricas.csv", ios::app);
                if (csvFile.is_open()) {
                    csvActivado = true;
                    cout << "[CSV] Exportacion activada en results/metricas.csv" << endl;
                } else {
                    cout << "[CSV] No se pudo abrir el archivo" << endl;
                }
            }
        }
    }

    return 0;
}
