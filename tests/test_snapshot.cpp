#include <iostream>
#include <cassert>
#include <memory>
#include "../include/core/Node.h"
#include "../include/core/Microservice.h"
#include "../include/core/Instance.h"
#include "../include/core/Snapshot.h"

void test_snapshot() {
    // Validacion del contenedor global del estado del cluster
    Snapshot snapshot(5);
    assert(snapshot.getTime() == 5);

    auto node = std::make_shared<Node>("N1", 150.0);
    auto ms = std::make_shared<Microservice>("M1");
    auto inst = std::make_shared<Instance>("I1", "M1", "N1", 30.0, 10.0);

    snapshot.addNode(node);
    snapshot.addMicroservice(ms);
    snapshot.addInstance(inst);

    assert(snapshot.getNode("N1") != nullptr);
    assert(snapshot.getNode("N1")->getNominalCapacity() == 150.0);
    assert(snapshot.getNode("N_INEXISTENTE") == nullptr);

    assert(snapshot.getMicroservice("M1") != nullptr);
    assert(snapshot.getMicroservice("M1")->getId() == "M1");
    assert(snapshot.getMicroservice("M_INEXISTENTE") == nullptr);

    assert(snapshot.getInstance("I1") != nullptr);
    assert(snapshot.getInstance("I1")->getDemand() == 30.0);
    assert(snapshot.getInstance("I_INEXISTENTE") == nullptr);

    assert(snapshot.getNodes().size() == 1);
    assert(snapshot.getMicroservices().size() == 1);
    assert(snapshot.getInstances().size() == 1);

    std::cout << "Prueba clase Snapshot superada con exito" << std::endl;
}
