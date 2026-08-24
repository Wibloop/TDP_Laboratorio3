#ifndef RDMPARSER_H
#define RDMPARSER_H

#include <string>
#include <memory>
#include "../core/Snapshot.h"

/**
 * Clase encargada de parsear archivos .rdm para construir el snapshot inicial
 * Aplica validaciones estrictas segun la especificacion RDM_VERSION 5
 */
class RDMParser {
public:
    // Parsea un archivo RDM y devuelve un snapshot poblado (en t=0 normalmente)
    // Lanza std::runtime_error si el archivo tiene errores lexicos o inconsistencias
    static Snapshot parse(const std::string& filepath, int timeT = 0);

private:
    // Metodos auxiliares para parsear lineas especificas
    static void parseNode(const std::string& line, int lineNumber, Snapshot& snapshot);
    static void parseMicroservice(const std::string& line, int lineNumber, Snapshot& snapshot);
    static void parseInstance(const std::string& line, int lineNumber, Snapshot& snapshot);
    
    // Auxiliar para limpiar y dividir strings por espacios
    static std::vector<std::string> splitSpaces(const std::string& str);
};

#endif // RDMPARSER_H
