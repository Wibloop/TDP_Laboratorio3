#include <iostream>
#include <cassert>
#include "../include/core/Microservice.h"

void test_microservice() {
    // Validacion de inicializacion y consulta de identificador de microservicio
    Microservice ms("M_TEST");
    assert(ms.getId() == "M_TEST");

    std::cout << "Prueba clase Microservice superada con exito" << std::endl;
}
