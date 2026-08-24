#include <iostream>
#include <fstream>
#include "../include/core/Snapshot.h"
#include "../include/core/Exporter.h"
#include "../include/parser/RDMParser.h"
#include "../include/strategies/PlacementStrategy.h"

void test_exporter() {
    // Validacion de generacion de traza de metricas
    Snapshot snapshot = RDMParser::parse("instances/caso_pequeno.rdm", 0);
    SolveResult res;
    res.hasSolution = true;
    res.status = "OPTIMAL";
    res.Z = 18.0;
    res.runtime_ms = 4.5;
    res.LB = 18.0;
    res.UB = 18.0;
    res.gap_percent = 0.0;
    res.nodes = 1;
    res.lp_calls = 2;
    res.X["I1"] = "N1";
    res.X["I2"] = "N2";
    res.X["I3"] = "N1";
    res.X["I4"] = "N2";

    std::ofstream dummyOut;
    Exporter::reportAndSave(1, 0, "A0", res, snapshot, dummyOut);

    std::cout << "Prueba modulo Exporter superada con exito" << std::endl;
}
