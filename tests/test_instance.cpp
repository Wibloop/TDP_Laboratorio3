#include <iostream>
#include <cassert>
#include "../include/core/Instance.h"

void test_instance() {
    // Validacion de atributos de instancia y modelo de datos
    Instance inst("I_TEST", "M_TEST", "N1", 45.0, 15.0);
    assert(inst.getId() == "I_TEST");
    assert(inst.getMicroserviceId() == "M_TEST");
    assert(inst.getCurrentNodeId() == "N1");
    assert(inst.getDemand() == 45.0);
    assert(inst.getStateSize() == 15.0);

    std::cout << "Prueba clase Instance superada con exito" << std::endl;
}
