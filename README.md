# Campeonato_De_Futbol

Se describe como un sistema para registrar los movimientos de un torneo de fútbol real, para poder darle seguimiento y llevar un control de dicho torneo.

Este proyecto implementa un **Sistema de Gestión de Campeonatos de Fútbol** en **C++** bajo el paradigma de **Programación Estructurada**, conectado directamente a una base de datos relacional en **Microsoft SQL Server** mediante la API nativa de **ODBC** (`odbc32`).

---

## 🚀 Requisitos del Sistema

1. **Compilador C++:** MinGW-w64 (GCC 11+) o MSVC compatible con C++17/C++20.
2. **Base de Datos:** Microsoft SQL Server (Localhost / SQLEXPRESS).
3. **Controlador:** ODBC Driver 17 o 18 for SQL Server (o SQL Server Native Client).

---

## 🗄️ Configuración de la Base de Datos

En el directorio del proyecto se incluyen los dos scripts SQL necesarios para poner en marcha el sistema:

### 1. Instalación de la Base de Datos Base
Ejecutar en SQL Server Management Studio (SSMS) o mediante línea de comandos:
```sql
instalar_base_datos.sql
```
* **Propósito:** Crea la base de datos `CampeonatoFutbol`, el esquema de 15 tablas relacionales con restricciones de clave primaria, foránea y de verificación, índices, vistas (`vw_TABLA_POSICIONES`, `vw_TABLA_GOLEO`), trigger de penales (`TR_TANDA_PENALES_SOLO_DESDE_CUARTOS`) y los catálogos base (Divisiones: `MAYOR` y `MENOR`; Roles: `ADMINISTRADOR` y `REPRESENTANTE`; y el usuario inicial `admin`).

### 2. Carga de Datos de Prueba (Simulación Completa)
Para poblar el sistema con escenarios de prueba listos para cada fase:
```sql
cargar_datos_prueba.sql
```
* **Propósito:** Registra 7 torneos preconfigurados con sus respectivos equipos, plantillas de jugadores y resultados, permitiendo probar inmediatamente:
  * **Torneo 1:** Listo para generar la fase general (Round Robin).
  * **Torneo 2:** Listo para generar Cuartos de Final (Top 8 clasificados).
  * **Torneo 3:** Listo para generar Semifinales.
  * **Torneo 4:** Listo para generar la Gran Final.
  * **Torneo 5:** Listo para consultar y habilitar Premiaciones oficiales.
  * **Torneo 6:** Torneo en curso con jornadas pendientes para ingreso manual de resultados.
  * **Torneo 7:** Prueba de disciplina y desempate apretado en zona de descenso.

---

## 💻 Compilación y Ejecución

### Opción 1: Compilación Directa con MinGW g++
```bash
g++ main.cpp -o main.exe -lodbc32
.\main.exe
```

### Opción 2: Compilación con CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 🔑 Credenciales de Acceso

* **Administrador Principal:**
  * Usuario: `admin`
  * Contraseña: `1234` (o `admin123`)
* **Representante Deportivo (Usuario Regular):**
  * Usuario: `jperez` o `hernandezr`
  * Contraseña: `1234`
