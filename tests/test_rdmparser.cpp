#include <iostream>
#include <cassert>
#include "../include/parser/RDMParser.h"
#include "../include/core/Snapshot.h"

void test_rdmparser() {
    // Validacion de parseo de casos validos
    Snapshot s_pequeno = RDMParser::parse("instances/caso_pequeno.rdm", 0);
    assert(s_pequeno.getNodes().size() == 3);
    assert(s_pequeno.getMicroservices().size() == 2);
    assert(s_pequeno.getInstances().size() == 4);

    Snapshot s_mediano = RDMParser::parse("instances/caso_mediano.rdm", 0);
    assert(s_mediano.getNodes().size() == 5);
    assert(s_mediano.getMicroservices().size() == 4);
    assert(s_mediano.getInstances().size() == 8);

    Snapshot s_dificil = RDMParser::parse("instances/caso_dificil.rdm", 0);
    assert(s_dificil.getNodes().size() == 4);
    assert(s_dificil.getMicroservices().size() == 3);
    assert(s_dificil.getInstances().size() == 5);

    // Validacion de captura de error ante archivo no existente
    bool caught = false;
    try {
        RDMParser::parse("instances/archivo_no_existente.rdm", 0);
    } catch (const std::exception& e) {
        caught = true;
    }
    assert(caught);

    std::cout << "Prueba clase RDMParser superada con exito" << std::endl;
}
