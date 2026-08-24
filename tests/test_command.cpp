#include <iostream>
#include <cassert>
#include "../include/parser/Command.h"
#include "../include/parser/RDMParser.h"
#include "../include/core/Snapshot.h"

void test_command() {
    // Validacion de ejecucion de comandos atomicos sobre snapshot
    Snapshot snapshot = RDMParser::parse("instances/caso_pequeno.rdm", 0);

    // Comando de falla
    FailCommand failCmd("N1");
    failCmd.execute(snapshot);
    assert(snapshot.getNode("N1")->getStateName() == "OFFLINE");
    assert(snapshot.getNode("N1")->getAvailableCapacity() == 0.0);

    // Comando de recuperacion
    RecoverCommand recCmd("N1");
    recCmd.execute(snapshot);
    assert(snapshot.getNode("N1")->getStateName() == "OPERATIONAL");
    assert(snapshot.getNode("N1")->getAvailableCapacity() == 100.0);

    // Comando de degradacion
    DegradeCommand degCmd("N2", 0.75);
    degCmd.execute(snapshot);
    assert(snapshot.getNode("N2")->getStateName() == "DEGRADED");
    assert(snapshot.getNode("N2")->getCurrentRho() == 0.75);
    assert(snapshot.getNode("N2")->getAvailableCapacity() == 75.0);

    std::cout << "Prueba clase Command superada con exito" << std::endl;
}
