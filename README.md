# Campeonato de Futbol

Sistema para registrar y gestionar torneos de futbol en consola (C++) conectado a base de datos Microsoft SQL Server mediante ODBC.

## Requisitos

- Compilador de C++ (MinGW g++ o MSVC con soporte para C++17)
- Microsoft SQL Server (Localhost / SQLEXPRESS)
- ODBC Driver para SQL Server

## Base de Datos

En el proyecto se incluyen los scripts necesarios para configurar la base de datos:

1. **instalar_base_datos.sql**: Crea la base de datos `CampeonatoFutbol`, tablas, relaciones, vistas, indices y el usuario administrador inicial.
2. **cargar_datos_prueba.sql**: Carga torneos de prueba en diferentes fases para realizar pruebas del sistema.

## Compilacion y Ejecucion

### Compilacion con g++ (MinGW):
```bash
g++ main.cpp -o main.exe -lodbc32
.\main.exe
```

### Compilacion con CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usuarios del Sistema

- **Administrador:**
  - Usuario: `admin`
  - Contrasena: `1234`
- **Representante:**
  - Usuario: `jperez` o `hernandezr`
  - Contrasena: `1234`
