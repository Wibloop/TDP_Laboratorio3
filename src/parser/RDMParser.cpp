// Implementacion del analizador sintactico para archivos de descripcion de instancias RDM
// Valida la cabecera RDM_VERSION 5 y construye el snapshot poblado con nodos microservicios e instancias
#include "RDMParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

// Division de cadenas de texto separadas por espacios en blanco tabulaciones o saltos de linea
std::vector<std::string> RDMParser::splitSpaces(const std::string& str) {
    // Vector de almacenamiento para los segmentos extraidos
    std::vector<std::string> tokens;
    // Flujo de cadena para procesar los delimitadores de espacio
    std::istringstream iss(str);
    std::string token;
    // Lectura iterativa de cada token
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Lectura y procesamiento del archivo RDM para construir el estado inicial del sistema en tiempo t
Snapshot RDMParser::parse(const std::string& filepath, int timeT) {
    // Apertura del archivo en modo lectura
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filepath);
    }

    // Instancia del contenedor de snapshot para el instante temporal especificado
    Snapshot snapshot(timeT);
    std::string line;
    int lineNumber = 0;
    bool headerFound = false;

    // Recorrido secuencial linea por linea del archivo fuente
    while (std::getline(file, line)) {
        lineNumber++;

        // Remueve retornos de carro de formato Windows si estan presentes
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Omite lineas completamente vacias
        if (line.empty()) continue;

        // Tokenizacion de la linea actual
        auto tokens = splitSpaces(line);
        if (tokens.empty()) continue;

        // Omite comentarios que inician con el caracter numeral
        if (tokens[0][0] == '#') continue;

        // Verificacion estricta del encabezado obligatorio en la primera linea util
        if (!headerFound) {
            if (tokens[0] == "RDM_VERSION" && tokens.size() >= 2 && tokens[1] == "5") {
                headerFound = true;
                continue;
            } else {
                throw std::runtime_error("El archivo debe comenzar con RDM_VERSION 5 (Linea " + std::to_string(lineNumber) + ")");
            }
        }

        // Clasificacion de directivas segun el tipo de entidad
        if (tokens[0] == "NODE") {
            parseNode(line, lineNumber, snapshot);
        } else if (tokens[0] == "MICROSERVICE") {
            parseMicroservice(line, lineNumber, snapshot);
        } else if (tokens[0] == "INSTANCE") {
            parseInstance(line, lineNumber, snapshot);
        } else {
            throw std::runtime_error("Comando desconocido en linea " + std::to_string(lineNumber) + ": " + tokens[0]);
        }
    }

    // Comprobacion final de que la cabecera fue detectada durante la lectura
    if (!headerFound) {
        throw std::runtime_error("Archivo invalido: RDM_VERSION 5 no encontrado");
    }

    return snapshot;
}

// Analisis de la directiva NODE para registrar un servidor en el snapshot
void RDMParser::parseNode(const std::string& line, int lineNumber, Snapshot& snapshot) {
    // Divide los argumentos de la directiva NODE id capacidad estado rho
    auto tokens = splitSpaces(line);
    if (tokens.size() != 5) throw std::runtime_error("Parametros incompletos para NODE en linea " + std::to_string(lineNumber));

    std::string id = tokens[1];
    double capacity = 0.0;
    std::string stateStr = tokens[3];
    double rho = 0.0;

    // Conversion de cadenas numericas a valores de punto flotante
    try {
        capacity = std::stod(tokens[2]);
        rho = std::stod(tokens[4]);
    } catch (...) {
        throw std::runtime_error("Error de formato numerico en NODE (Linea " + std::to_string(lineNumber) + ")");
    }

    // Validacion de no negatividad en la capacidad nominal
    if (capacity < 0.0) throw std::runtime_error("Capacidad negativa no permitida en NODE");

    // Validacion cruzada de coherencia entre el nombre del estado y el valor del factor rho
    if (stateStr == "OPERATIONAL") {
        if (rho != 1.0) throw std::runtime_error("Estado OPERATIONAL requiere rho=1.0");
    } else if (stateStr == "OFFLINE") {
        if (rho != 0.0) throw std::runtime_error("Estado OFFLINE requiere rho=0.0");
    } else if (stateStr == "DEGRADED") {
        if (rho <= 0.0 || rho >= 1.0) throw std::runtime_error("Estado DEGRADED requiere 0.0 < rho < 1.0");
    } else {
        throw std::runtime_error("Estado invalido en NODE: " + stateStr);
    }

    // Creacion del nodo e inyeccion del estado operativo correspondiente
    auto node = std::make_shared<Node>(id, capacity);
    if (stateStr == "DEGRADED") node->degrade(rho);
    else if (stateStr == "OFFLINE") node->fail();

    // Registro del servidor en la coleccion del snapshot
    snapshot.addNode(node);
}

// Analisis de la directiva MICROSERVICE para registrar la definicion logica de un servicio
void RDMParser::parseMicroservice(const std::string& line, int lineNumber, Snapshot& snapshot) {
    // Divide los argumentos de la directiva MICROSERVICE id
    auto tokens = splitSpaces(line);
    if (tokens.size() != 2) throw std::runtime_error("Parametros incompletos para MICROSERVICE en linea " + std::to_string(lineNumber));
    
    // Creacion del objeto de microservicio logico y registro en el snapshot
    auto ms = std::make_shared<Microservice>(tokens[1]);
    snapshot.addMicroservice(ms);
}

// Analisis de la directiva INSTANCE para registrar una replica de microservicio
void RDMParser::parseInstance(const std::string& line, int lineNumber, Snapshot& snapshot) {
    // Divide los argumentos de la directiva INSTANCE id microservicio_id nodo_id demanda tamano_estado
    auto tokens = splitSpaces(line);
    if (tokens.size() != 6) throw std::runtime_error("Parametros incompletos para INSTANCE en linea " + std::to_string(lineNumber));

    std::string id = tokens[1];
    std::string msId = tokens[2];
    std::string nodeId = tokens[3];
    double demand = 0.0;
    double stateSize = 0.0;

    // Conversion de cadenas a valores de demanda y tamano de estado
    try {
        demand = std::stod(tokens[4]);
        stateSize = std::stod(tokens[5]);
    } catch (...) {
        throw std::runtime_error("Error de formato numerico en INSTANCE (Linea " + std::to_string(lineNumber) + ")");
    }

    // Validacion de no negatividad para demandas y tamanos de estado
    if (demand < 0.0 || stateSize < 0.0) throw std::runtime_error("Valores negativos no permitidos en INSTANCE");

    // Creacion de la instancia y registro en la coleccion del snapshot
    auto inst = std::make_shared<Instance>(id, msId, nodeId, demand, stateSize);
    snapshot.addInstance(inst);
}
