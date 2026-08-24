# DEFENSA ARQUITECTONICA

Tenemos 5 decisiones criticas de diseño implementadas durante el ciclo de desarrollo, fundamentadas matematicamente, con el proposito de asegurar mantenibilidad, determinismo y alta escalabilidad computacional en el manejo de estructuras NP-Duras

### Decision 1: Uso del patron Strategy para solucionadores
**Fundamento:** La inyeccion y permutacion en tiempo de ejecucion
Al desacoplar los cuatro modelos algoritmicos (A0, H1, H2, R) de la unidad de despliegue mediante la interfaz abstracta `PlacementStrategy`, se encapsulo la complejidad polimorfica que contenia el proyecto. Esto permite que el componente iterador o controlador (en `main.cpp`) aplique la regla OCP (Open-Closed Principle): integrar un futuro solucionador no requiere modificar el orquestador principal, logrando que el intercambio de heuristica a tecnica exacta ocurra transparente e uniformemente mediante el struct `SolveResult`.

### Decision 2: Patron State para la logica de Nodos
**Fundamento:** Administracion inmutable de transiciones y aislamiento semantico
En lugar de condicionar el flujo de disponibilidad de infraestructura mediante banderas (booleanos) o enumeradores acoplados a un switch gigante, el patron State (`Operational`, `Degraded`, `Offline`) delega las transiciones validas a entidades aisladas, asegurando que la matriz de transicion no se rompa (por ejemplo impidiendo transiciones logicas imposibles como una degradacion desde un estado offline). La capacidad `K_j^t = rho * K_nominal` se resuelve nativamente de forma contextual minimizando ramificaciones

### Decision 3: Eficiencia de memoria en Branch-and-Bound (ClpBBStrategy)
**Fundamento:** Evitar el desbordamiento de memoria por clonacion matricial
En escenarios tipicos de B&B sobre problemas combinatorios grandes, instanciar la `CoinPackedMatrix` por cada nodo hijo provoca colapsos en la pila. La decision implementada construye un unico objeto de restricciones raiz y se inyecta por referencia. Durante la exploracion del arbol, solo se realizan modificaciones ligeras y transitorias operando el limite dual de las variables (`setColBounds(k, val, val)`). Tras evaluar, los limites retornan a [0, 1]. Esto reduce la complejidad espacial de una operacion polinomial densa a una constante diferencial que se aloja en memoria RAM

### Decision 4: Logica adaptativa con RCL Uniforme en GRASP (H2)
**Fundamento:** Exploracion acotada contra sobreajuste local
En GRASP, la convergencia prematura a optimos locales debidos a sesgos deterministas es peligrosa. Para combatirlo, en la fase constructiva se definio la funcion de costo `h(i*, j)` normalizando la carga transferida `c_ij` y la capacidad residual penalizada por el factor multiplicador `lambda`. A partir de alli, los candidatos entran a una RCL segun la ventana heuristica definida por el gradiente `eta`. La seleccion en la RCL es controlada por distribucion uniforme `std::uniform_int_distribution`, garantizando ergodicidad local en el muestreo, con un generador pseudoaleatorio pre-sembrado estrictamente *una sola vez por epoca* para garantizar reproducibilidad perfecta sin contaminacion inter-iteraciones

### Decision 5: Orden lexicografico estricto con std::tuple en C++
**Fundamento:** Trazabilidad absoluta y desempates asimetricos de costos
En los metodos constructivos (H1) y en el operador `LocalSearch`, lidiar con valores de prioridad empatados requiere resoluciones predecibles. Usar arrays de desempate manual genera estructuras laberinticas con altas propensiones a error. En su lugar, al aprovechar la sobrecarga natural y la reduccion transitiva implementada en `std::tuple`, pudimos definir prioridades estrictas como `(|F(i)|, -s_i, -d_i, i)`. Esta caracteristica nativa inyecta un orden total asimetrico: C++ resuelve las comparaciones indice por indice a velocidad de compilador. Un empate total nunca ocurre ya que el indexador fundamental (`i` y `j`) forma la clave primaria terminal de desempate, blindando el determinismo total de las ejecuciones sin la necesidad de usar comparadores externos