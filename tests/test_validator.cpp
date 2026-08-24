#include <iostream>
#include <cassert>
#include "../include/solvers/Validator.h"
#include "../include/parser/RDMParser.h"
#include "../include/core/Snapshot.h"

void test_validator() {
    // Validacion exhaustiva de restricciones matematicas
    Snapshot snapshot = RDMParser::parse("instances/caso_pequeno.rdm", 0);

    // Asignacion valida inicial
    AssignmentMap x_valida;
    x_valida["I1"] = "N1";
    x_valida["I2"] = "N2";
    x_valida["I3"] = "N1";
    x_valida["I4"] = "N2";
    auto res_valida = Validator::validate(x_valida, snapshot);
    assert(res_valida.first == true);
    assert(res_valida.second == 0.0);

    // Violacion de unicidad por instancia faltante
    AssignmentMap x_incompleta;
    x_incompleta["I1"] = "N1";
    x_incompleta["I2"] = "N2";
    x_incompleta["I3"] = "N1";
    auto res_incompleta = Validator::validate(x_incompleta, snapshot);
    assert(res_incompleta.first == false);

    // Violacion de capacidad en nodo degradado
    AssignmentMap x_sobrecap;
    x_sobrecap["I1"] = "N3";
    x_sobrecap["I2"] = "N1";
    x_sobrecap["I3"] = "N3";
    x_sobrecap["I4"] = "N2";
    auto res_sobrecap = Validator::validate(x_sobrecap, snapshot);
    assert(res_sobrecap.first == false);

    // Violacion de separacion de replicas
    AssignmentMap x_colision;
    x_colision["I1"] = "N1";
    x_colision["I2"] = "N1";
    x_colision["I3"] = "N2";
    x_colision["I4"] = "N2";
    auto res_colision = Validator::validate(x_colision, snapshot);
    assert(res_colision.first == false);

    std::cout << "Prueba clase Validator superada con exito" << std::endl;
}
