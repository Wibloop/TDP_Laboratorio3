#include <iostream>
#include <cassert>
#include "../include/core/Node.h"
#include "../include/core/NodeState.h"
#include "../include/core/OperationalState.h"
#include "../include/core/DegradedState.h"
#include "../include/core/OfflineState.h"

void test_nodestate() {
    // Validacion de transiciones polimorficas de estado
    Node node("N_STATE", 100.0);

    // Transicion permitida operacional a degradado
    node.degrade(0.8);
    assert(node.getStateName() == "DEGRADED");
    assert(node.getAvailableCapacity() == 80.0);

    // Transicion permitida degradado a offline
    node.fail();
    assert(node.getStateName() == "OFFLINE");
    assert(node.getAvailableCapacity() == 0.0);

    // Degradacion no permitida cuando el nodo esta offline
    node.degrade(0.5);
    assert(node.getStateName() == "OFFLINE");

    // Recuperacion desde offline a operacional
    node.recover();
    assert(node.getStateName() == "OPERATIONAL");
    assert(node.getAvailableCapacity() == 100.0);

    std::cout << "Prueba clase NodeState superada con exito" << std::endl;
}
