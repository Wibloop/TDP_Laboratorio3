#include <iostream>
#include <cassert>
#include "../include/core/Node.h"

void test_node() {
    // Validacion de instanciacion y calculo de capacidad nominal y disponible
    Node node("N_TEST", 120.0);
    assert(node.getId() == "N_TEST");
    assert(node.getNominalCapacity() == 120.0);
    assert(node.getAvailableCapacity() == 120.0);
    assert(node.getStateName() == "OPERATIONAL");
    assert(node.getCurrentRho() == 1.0);

    // Cambio de estado hacia degradado
    node.degrade(0.5);
    assert(node.getStateName() == "DEGRADED");
    assert(node.getCurrentRho() == 0.5);
    assert(node.getAvailableCapacity() == 60.0);

    // Cambio de estado hacia falla total
    node.fail();
    assert(node.getStateName() == "OFFLINE");
    assert(node.getCurrentRho() == 0.0);
    assert(node.getAvailableCapacity() == 0.0);

    // Recuperacion a estado operacional
    node.recover();
    assert(node.getStateName() == "OPERATIONAL");
    assert(node.getCurrentRho() == 1.0);
    assert(node.getAvailableCapacity() == 120.0);

    std::cout << "Prueba clase Node superada con exito" << std::endl;
}
