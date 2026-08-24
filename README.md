# Optimizador Multi-Banda: Asignacion de Microservicios

Este repositorio contiene la implementacion completa del motor de optimizacion para la ubicacion dinamica de microservicios sobre una infraestructura computacional sujeta a degradaciones 

## Requisitos del Entorno
Para compilar y ejecutar este proyecto, el sistema debe cumplir con los siguientes requerimientos:
- **Sistema Operativo:** Ubuntu (o distribuciones Linux compatibles).
- **Compilador:** g++ con soporte para el estandar **C++11** puro.
- **Dependencias COIN-OR:** El sistema hace uso de bibliotecas de optimizacion lineal y entera. Es imperativo tener instalados los paquetes de desarrollo de COIN-OR:
  - `Cbc` (Coin-or branch and cut)
  - `OsiClp` (Open Solver Interface para Clp)
  - `Clp` (Coin-or linear programming)
  - `CoinUtils`
  
  En distribuciones basadas en Debian/Ubuntu, se pueden instalar mediante:
  `sudo apt-get install coinor-libcbc-dev coinor-libclp-dev coinor-libosi-dev coinor-libcoinutils-dev`

## Instrucciones de Compilacion
El proyecto cuenta con un `Makefile` en el directorio raiz para automatizar el proceso de construccion modular.

1. **Compilar el binario principal:**
   Genera el ejecutable principal del sistema en `bin/app`.
  ```bash
  make all
  ```

2. **Compilar y ejecutar la suite de pruebas automatizadas:**
   Genera el binario de pruebas en `bin/test_app` y lo ejecuta inmediatamente verificando mapeos, validadores, logica de estados, empates y correctitud del solucionador Branch-and-Bound.
  ```bash
  make test
  ```

3. **Limpiar binarios y objetos:**
   Borra los directorios temporales `obj/` y `bin/` para una compilacion limpia.
  ```bash
  make clean
  ```

## Ejecucion e Interfaz CLI
Una vez compilado, ejecuta el simulador mediante el binario principal:
```bash
./bin/app
```

### Ejemplos de Uso en el Menu
Al iniciar, la consola desplegara un menu interactivo numerico:

1. **Cargar caso .rdm:**
   El programa solicitara la ruta relativa de la instancia. 
   - *Ejemplo de entrada:* `instances/demo_fallas.rdm`
2. **Mostrar y validar estado inicial:**
   Despliega en consola el snapshot actual (t=0) con las capacidades, demandas y un mapeo del estado operativo de cada nodo.
3. **Ejecutar una epoca (H1, H2, A0, R):**
   Aplica eventos deterministas sobre los nodos, y evalua el escenario estatico actual con las 4 estrategias.
4. **Ejecutar la secuencia completa:**
   Evalua escenarios dinamicamente, integrando eventos en multiples epocas temporales. Transfiere consolidaciones de p_i^{t+1} basadas en la mejor solucion factible generada.
5. **Exportacion CSV:**
   Habilita la escritura en disco (`results/metricas.csv`). Cada ejecucion posterior generara una traza de rendimiento.
6. **Salir.**

*Nota:* Asegurese de habilitar la opcion (5) si desea extraer mediciones como runtime, gaps o iteraciones LP