#ifndef COMMAND_H
#define COMMAND_H

#include "../core/Snapshot.h"
#include <string>

/**
 * Interfaz base para el patron Command
 * Define la operacion de ejecucion de un evento en el sistema
 */
class Command {
public:
    virtual ~Command() = default;

    // Ejecuta el comando sobre un Snapshot
    virtual void execute(Snapshot& snapshot) = 0;
};

/**
 * Comando para aplicar una falla a un nodo
 */
class FailCommand : public Command {
private:
    std::string nodeId;

public:
    explicit FailCommand(const std::string& id) : nodeId(id) {}

    void execute(Snapshot& snapshot) override;
};

/**
 * Comando para degradar la capacidad de un nodo
 */
class DegradeCommand : public Command {
private:
    std::string nodeId;
    double rho;

public:
    DegradeCommand(const std::string& id, double r) : nodeId(id), rho(r) {}

    void execute(Snapshot& snapshot) override;
};

/**
 * Comando para recuperar un nodo a su estado operativo normal
 */
class RecoverCommand : public Command {
private:
    std::string nodeId;

public:
    explicit RecoverCommand(const std::string& id) : nodeId(id) {}

    void execute(Snapshot& snapshot) override;
};

#endif // COMMAND_H
