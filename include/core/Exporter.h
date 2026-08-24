#ifndef EXPORTER_H
#define EXPORTER_H

#include "../include/strategies/PlacementStrategy.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Exporter {

    inline void reportAndSave(int epoch, int timestamp, const std::string& method, 
                              const SolveResult& res, const Snapshot& snapshot, 
                              std::ofstream& csvFile) {
        
        // Calcular instancias migradas
        int migrated = 0;
        if (res.hasSolution) {
            for (const auto& pair : res.X) {
                auto inst = snapshot.getInstance(pair.first);
                if (inst && inst->getCurrentNodeId() != pair.second) {
                    migrated++;
                }
            }
        }

        // Formatear placement como string "i:j,i2:j2"
        std::string placementStr = "";
        if (res.hasSolution) {
            bool first = true;
            for (const auto& pair : res.X) {
                if (!first) placementStr += ",";
                placementStr += pair.first + ":" + pair.second;
                first = false;
            }
        } else {
            placementStr = "NA";
        }

        // Formatear metricas exclusivas de arboles o NA para heuristicas
        std::string lb_str = (method == "H1" || method == "H2" || !res.hasSolution) ? "NA" : std::to_string(res.LB);
        std::string ub_str = (method == "H1" || method == "H2" || !res.hasSolution) ? "NA" : std::to_string(res.UB);
        std::string gap_str = (method == "H1" || method == "H2" || res.gap_percent < 0) ? "NA" : std::to_string(res.gap_percent);
        std::string nodes_str = (method == "H1" || method == "H2") ? "NA" : std::to_string(res.nodes);
        std::string lp_str = (method == "H1" || method == "H2") ? "NA" : std::to_string(res.lp_calls);
        std::string z_str = res.hasSolution ? std::to_string(res.Z) : "NA";
        std::string statusStr = res.status;

        // Construir string final
        std::ostringstream oss;
        oss << "epoch=" << epoch 
            << " timestamp=" << timestamp 
            << " method=" << method 
            << " status=" << statusStr 
            << " Z=" << z_str 
            << " migrated_instances=" << (res.hasSolution ? std::to_string(migrated) : "NA")
            << " runtime_ms=" << res.runtime_ms 
            << " LB=" << lb_str 
            << " UB=" << ub_str 
            << " gap_percent=" << gap_str 
            << " nodes=" << nodes_str 
            << " lp_calls=" << lp_str 
            << " placement=" << placementStr;

        std::string out = oss.str();
        
        // Imprimir en consola
        std::cout << out << std::endl;

        // Guardar en CSV si esta abierto
        if (csvFile.is_open()) {
            csvFile << out << "\n";
            csvFile.flush();
        }
    }
}

#endif // EXPORTER_H
