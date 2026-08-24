#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "../core/Snapshot.h"
#include "../solvers/Validator.h"

namespace LocalSearch {

    /**
     * Aplica la rutina de mejora iterativa sobre una asignacion factible X
     * Retorna la nueva asignacion optimizada, que puede ser la misma si es un minimo local
     */
    AssignmentMap optimize(AssignmentMap X, const Snapshot& snapshot, int time_limit_ms);

}

#endif // LOCALSEARCH_H
