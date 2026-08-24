#include <iostream>
#include <cassert>
#include "../include/strategies/LocalSearch.h"
#include "../include/solvers/Validator.h"
#include "../include/parser/RDMParser.h"
#include "../include/core/Snapshot.h"

void test_localsearch() {
    // Validacion de optimizacion de vecindarios
    Snapshot snapshot = RDMParser::parse("instances/caso_pequeno.rdm", 0);
    AssignmentMap x_inicial;
    x_inicial["I1"] = "N2";
    x_inicial["I2"] = "N1";
    x_inicial["I3"] = "N2";
    x_inicial["I4"] = "N1";

    AssignmentMap x_opt = LocalSearch::optimize(x_inicial, snapshot, 500);
    auto valResult = Validator::validate(x_opt, snapshot);
    assert(valResult.first == true);

    std::cout << "Prueba modulo LocalSearch superada con exito" << std::endl;
}
