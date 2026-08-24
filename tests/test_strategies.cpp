#include <iostream>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include "../include/strategies/GreedyStableStrategy.h"
#include "../include/strategies/GraspStrategy.h"
#include "../include/strategies/ClpBBStrategy.h"
#include "../include/strategies/CbcStrategy.h"
#include "../include/solvers/Validator.h"
#include "../include/parser/RDMParser.h"
#include "../include/core/Snapshot.h"

void test_strategies() {
    // Validacion de solucionador constructivo H1
    GreedyStableStrategy h1;

    Snapshot s_pequeno = RDMParser::parse("instances/caso_pequeno.rdm", 0);
    auto res_pequeno_h1 = h1.solve(s_pequeno, 1000);
    assert(res_pequeno_h1.hasSolution == true);
    assert(Validator::validate(res_pequeno_h1.X, s_pequeno).first == true);

    Snapshot s_mediano = RDMParser::parse("instances/caso_mediano.rdm", 0);
    auto res_mediano_h1 = h1.solve(s_mediano, 1000);
    assert(res_mediano_h1.hasSolution == true);
    assert(Validator::validate(res_mediano_h1.X, s_mediano).first == true);

    Snapshot s_dificil = RDMParser::parse("instances/caso_dificil.rdm", 0);
    auto res_dificil_h1 = h1.solve(s_dificil, 1000);
    assert(res_dificil_h1.hasSolution == true);
    assert(Validator::validate(res_dificil_h1.X, s_dificil).first == true);

    std::cout << "Prueba clase GreedyStableStrategy superada con exito" << std::endl;

    // Validacion de metaheuristica H2 y control de parametros
    bool caught = false;
    try {
        GraspStrategy bad_eta(1, 1.5, 1.0);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        GraspStrategy bad_lambda(1, 0.5, -0.5);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    assert(caught);

    // Validacion de reproducibilidad en GRASP con semilla fija
    GraspStrategy g1(42, 0.3, 1.0);
    GraspStrategy g2(42, 0.3, 1.0);

    auto res1 = g1.solve(s_pequeno, 200);
    auto res2 = g2.solve(s_pequeno, 200);
    assert(res1.hasSolution == true);
    assert(res2.hasSolution == true);
    assert(res1.Z == res2.Z);

    auto res_mediano_h2 = g1.solve(s_mediano, 200);
    assert(res_mediano_h2.hasSolution == true);
    assert(Validator::validate(res_mediano_h2.X, s_mediano).first == true);

    auto res_dificil_h2 = g1.solve(s_dificil, 200);
    assert(res_dificil_h2.hasSolution == true);
    assert(Validator::validate(res_dificil_h2.X, s_dificil).first == true);

    std::cout << "Prueba clase GraspStrategy superada con exito" << std::endl;

    // Validacion de solucionador exacto personalizado A0
    ClpBBStrategy a0;

    auto res_pequeno_a0 = a0.solve(s_pequeno, 2000);
    assert(res_pequeno_a0.hasSolution == true);
    assert(res_pequeno_a0.status == "OPTIMAL");
    assert(Validator::validate(res_pequeno_a0.X, s_pequeno).first == true);
    assert(res_pequeno_a0.LB <= res_pequeno_a0.UB + 1e-6);

    auto res_mediano_a0 = a0.solve(s_mediano, 2000);
    assert(res_mediano_a0.hasSolution == true);
    assert(Validator::validate(res_mediano_a0.X, s_mediano).first == true);

    auto res_dificil_a0 = a0.solve(s_dificil, 2000);
    assert(res_dificil_a0.hasSolution == true);
    assert(Validator::validate(res_dificil_a0.X, s_dificil).first == true);

    std::cout << "Prueba clase ClpBBStrategy superada con exito" << std::endl;

    // Validacion de solucionador exacto MILP R
    CbcStrategy r;

    auto res_pequeno_r = r.solve(s_pequeno, 2000);
    assert(res_pequeno_r.hasSolution == true);
    assert(res_pequeno_r.status == "OPTIMAL");
    assert(Validator::validate(res_pequeno_r.X, s_pequeno).first == true);

    auto res_mediano_r = r.solve(s_mediano, 2000);
    assert(res_mediano_r.hasSolution == true);
    assert(Validator::validate(res_mediano_r.X, s_mediano).first == true);

    auto res_dificil_r = r.solve(s_dificil, 2000);
    assert(res_dificil_r.hasSolution == true);
    assert(Validator::validate(res_dificil_r.X, s_dificil).first == true);

    std::cout << "Prueba clase CbcStrategy superada con exito" << std::endl;

    // Validacion cruzada integral caso facil
    assert(res_pequeno_h1.hasSolution && res_pequeno_h2.hasSolution);
    assert(res_pequeno_a0.hasSolution && res_pequeno_r.hasSolution);
    std::cout << "Prueba integracion caso facil superada con exito" << std::endl;

    // Validacion cruzada integral caso medio
    assert(res_mediano_h1.hasSolution && res_mediano_h2.hasSolution);
    assert(res_mediano_a0.hasSolution && res_mediano_r.hasSolution);
    std::cout << "Prueba integracion caso medio superada con exito" << std::endl;

    // Validacion cruzada integral caso dificil
    assert(res_dificil_h1.hasSolution && res_dificil_h2.hasSolution);
    assert(res_dificil_a0.hasSolution && res_dificil_r.hasSolution);
    std::cout << "Prueba integracion caso dificil superada con exito" << std::endl;
}
