#include <iostream>

// Declaraciones de funciones ejecutoras de pruebas por modulo
void test_node();
void test_nodestate();
void test_microservice();
void test_instance();
void test_snapshot();
void test_exporter();
void test_rdmparser();
void test_command();
void test_validator();
void test_localsearch();
void test_strategies();

int main() {
    // Orquestacion secuencial de todas las unidades de prueba
    test_node();
    test_nodestate();
    test_microservice();
    test_instance();
    test_snapshot();
    test_exporter();
    test_rdmparser();
    test_command();
    test_validator();
    test_localsearch();
    test_strategies();
    return 0;
}
