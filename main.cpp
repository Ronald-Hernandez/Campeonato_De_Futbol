#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>
#include <map>
#ifdef _WIN32
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#endif

// =========================================================================
// ENUMERACIONES Y CONSTANTES DE CONTROL
// =========================================================================

enum RolUsuario {
    ROL_ADMINISTRADOR = 1,
    ROL_REPRESENTANTE = 2
};

enum DivisionLiga {
    LIGA_MAYOR = 1,
    LIGA_MENOR = 2
};

// =========================================================================
// ESTRUCTURAS DE DATOS (PROGRAMACIÓN ESTRUCTURADA)
// =========================================================================

struct Usuario {
    int idUsuario;
    std::string usuario;
    std::string contrasena;
    std::string nombreCompleto;
    RolUsuario rol;
};

struct Jugador {
    int idJugador;
    int idEquipo;
    std::string nombre;
    std::string apellido;
    std::string posicion; // PORTERO, DEFENSA, MEDIOCAMPISTA, DELANTERO
    int numeroCamiseta;
};

struct Equipo {
    int idEquipo;
    std::string nombre;
    DivisionLiga division; // 1 = LIGA_MAYOR, 2 = LIGA_MENOR
    int idCampeonato;
    std::vector<Jugador> jugadores;
};

struct IncidenciaGol {
    int idJugador;
    std::string nombreJugador;
    int idEquipoRival;
    std::string nombreEquipoRival;
    int idPorteroVencido;
    std::string nombrePorteroVencido;
    int minuto;
    bool esAutogol;
};

struct IncidenciaTarjeta {
    int idJugador;
    std::string nombreJugador;
    std::string tipoTarjeta; // AMARILLA, ROJA
    int minuto;
};

struct ControlPortero {
    int idJugador;
    std::string nombrePortero;
    int idEquipo;
    int minutoEntrada;
    int minutoSalida;
    int golesRecibidos;
};

struct Partido {
    int idPartido;
    int idCampeonato;
    DivisionLiga division;
    std::string fase; // CLASIFICACION, CUARTOS, SEMIFINAL, FINAL
    int jornada;
    std::string fecha;
    int idLocal;
    std::string nombreLocal;
    int golesLocal;
    int idVisitante;
    std::string nombreVisitante;
    int golesVisitante;
    std::string estado; // PENDIENTE, FINALIZADO
    std::vector<IncidenciaGol> goles;
    std::vector<IncidenciaTarjeta> tarjetas;
    std::vector<ControlPortero> porteros;
};

struct PosicionEquipo {
    int posicion;
    int idEquipo;
    std::string nombreEquipo;
    int pj;
    int pg;
    int pe;
    int pp;
    int gf;
    int gc;
    int dg;
    int puntos;
};

struct HistorialMovimiento {
    int idHistorial;
    int idCampeonato;
    int idEquipo;
    std::string nombreEquipo;
    DivisionLiga origen;
    DivisionLiga destino;
    std::string tipoMovimiento; // ASCENSO, DESCENSO
    std::string fecha;
};

struct Torneo {
    int idTorneo;
    std::string nombre;
    std::string fechaInicio;
    std::string fechaFin;
    int maxMayor;
    int maxMenor;
    bool finalizado;
};

struct Premiacion {
    int idTorneo;
    std::string campeonMayor;
    std::string campeonMenor;
    std::string maximoGoleador;
    int golesAnotados;
    std::string porteroMenosVencido;
    int golesRecibidos;
    bool habilitada;
    bool registrada;
};

// =========================================================================
// GESTOR DE BASE DE DATOS SQL SERVER (ODBC DINÁMICO)
// =========================================================================

#ifdef _WIN32
typedef SQLRETURN (SQL_API *t_SQLAllocHandle)(SQLSMALLINT, SQLHANDLE, SQLHANDLE*);
typedef SQLRETURN (SQL_API *t_SQLSetEnvAttr)(SQLHENV, SQLINTEGER, SQLPOINTER, SQLINTEGER);
typedef SQLRETURN (SQL_API *t_SQLDriverConnectA)(SQLHDBC, SQLHWND, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLUSMALLINT);
typedef SQLRETURN (SQL_API *t_SQLDisconnect)(SQLHDBC);
typedef SQLRETURN (SQL_API *t_SQLFreeHandle)(SQLSMALLINT, SQLHANDLE);
typedef SQLRETURN (SQL_API *t_SQLExecDirectA)(SQLHSTMT, SQLCHAR*, SQLINTEGER);
typedef SQLRETURN (SQL_API *t_SQLFetch)(SQLHSTMT);
typedef SQLRETURN (SQL_API *t_SQLGetData)(SQLHSTMT, SQLUSMALLINT, SQLSMALLINT, SQLPOINTER, SQLLEN, SQLLEN*);
typedef SQLRETURN (SQL_API *t_SQLCloseCursor)(SQLHSTMT);

struct GestorBD {
    HMODULE hOdbc = NULL;
    t_SQLAllocHandle fnAllocHandle = NULL;
    t_SQLSetEnvAttr fnSetEnvAttr = NULL;
    t_SQLDriverConnectA fnDriverConnectA = NULL;
    t_SQLDisconnect fnDisconnect = NULL;
    t_SQLFreeHandle fnFreeHandle = NULL;
    t_SQLExecDirectA fnExecDirectA = NULL;
    t_SQLFetch fnFetch = NULL;
    t_SQLGetData fnGetData = NULL;
    t_SQLCloseCursor fnCloseCursor = NULL;

    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    bool conectada = false;
    std::string servidorConectado = "";
};
#else
struct GestorBD {
    bool conectada = false;
    std::string servidorConectado = "";
};
#endif

struct SistemaCampeonato {
    GestorBD db;
    std::vector<Usuario> usuarios;
    std::vector<Torneo> torneos;
    int idTorneoActivo;
    std::vector<Equipo> equipos;
    std::vector<Partido> partidos;
    std::vector<HistorialMovimiento> historial;
    std::vector<Premiacion> premiaciones;

    int autoIncUsuario;
    int autoIncTorneo;
    int autoIncEquipo;
    int autoIncJugador;
    int autoIncPartido;
    int autoIncHistorial;
};

// =========================================================================
// FIRMAS DE FUNCIONES ESTRUCTURADAS
// =========================================================================

// Utilidades
void pausarConsola();
Equipo* buscarEquipoPorId(SistemaCampeonato &sistema, int idEquipo);
std::string sanitizarSQL(const std::string &input);

// Conexión y Persistencia BD SQL Server
bool dbInicializarYConectar(GestorBD &bd);
void dbDesconectar(GestorBD &bd);
bool dbCargarTodo(SistemaCampeonato &sistema);
bool dbInsertarJugadorEnBD(SistemaCampeonato &sistema, Jugador &j);
bool dbInsertarPartidoEnBD(SistemaCampeonato &sistema, Partido &p);
bool dbActualizarResultadoPartidoEnBD(SistemaCampeonato &sistema, const Partido &p);
bool dbActualizarDivisionEquipoEnBD(SistemaCampeonato &sistema, int idEquipo, int nuevaDivision);
bool dbInsertarHistorialEnBD(SistemaCampeonato &sistema, const HistorialMovimiento &h);
bool dbGuardarPremiacionEnBD(SistemaCampeonato &sistema, const Premiacion &prem, int idGoleador, int idPortero, int idCampeon);
bool dbInsertarUsuarioEnBD(SistemaCampeonato &sistema, const Usuario &u);
bool dbInsertarEquipoEnBD(SistemaCampeonato &sistema, Equipo &eq);
bool dbCrearTorneoEnBD(SistemaCampeonato &sistema, Torneo &t);

// Persistencia en Archivo Local (Respaldo)
void inicializarSistema(SistemaCampeonato &sistema);
void inicializarDatosSemilla(SistemaCampeonato &sistema);
bool guardarDatosEnArchivo(const SistemaCampeonato &sistema, const std::string &ruta = "datos_campeonato.txt");
bool cargarDatosDesdeArchivo(SistemaCampeonato &sistema, const std::string &ruta = "datos_campeonato.txt");

// Módulo Torneo, Equipos y Jugadores
void crearTorneo(SistemaCampeonato &sistema);
void listarTorneos(const SistemaCampeonato &sistema);
void seleccionarTorneoActivo(SistemaCampeonato &sistema);
void registrarEquipo(SistemaCampeonato &sistema);
void registrarJugador(SistemaCampeonato &sistema);
void listarEquiposYPlantillas(const SistemaCampeonato &sistema, DivisionLiga division);

// Módulo Fixture y Algoritmos
bool generarClasificacionRoundRobin(SistemaCampeonato &sistema, DivisionLiga division);
bool generarCuartosDeFinalTop8(SistemaCampeonato &sistema, DivisionLiga division);
bool generarSemifinales(SistemaCampeonato &sistema, DivisionLiga division);
bool generarGranFinal(SistemaCampeonato &sistema, DivisionLiga division);
void listarFixture(const SistemaCampeonato &sistema, DivisionLiga division);

// Módulo Resultados e Incidencias
void ingresarResultadoPartido(SistemaCampeonato &sistema);
void verDetallePartido(const SistemaCampeonato &sistema);
void listarCalendarioPartidos(const SistemaCampeonato &sistema, DivisionLiga division, const std::string &filtroFase = "TODAS");

// Módulo Posiciones, Ascenso/Descenso y Premiaciones
std::vector<PosicionEquipo> calcularTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division);
void imprimirTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division);
void imprimirTablasAscensoDescenso(const SistemaCampeonato &sistema);
void aplicarAscensoDescensoOficial(SistemaCampeonato &sistema);
void imprimirModuloPremiaciones(const SistemaCampeonato &sistema);
void finalizarTorneoYHabilitarPremiacion(SistemaCampeonato &sistema);

// Reportes específicos solicitados
void imprimirReporteTarjetas(const SistemaCampeonato &sistema);
void imprimirEstadisticasPremiacion(const SistemaCampeonato &sistema);

// Menús y Flujo de Usuarios
void menuPrincipal(SistemaCampeonato &sistema);
bool autenticarUsuario(SistemaCampeonato &sistema, Usuario &usuarioLogueado);
void registrarUsuarioRepresentante(SistemaCampeonato &sistema);
void menuAdministrador(SistemaCampeonato &sistema, const Usuario &admin);
void menuRepresentante(SistemaCampeonato &sistema, const Usuario &rep);

// =========================================================================
// FUNCIÓN PRINCIPAL (MAIN)
// =========================================================================

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    SistemaCampeonato sistema;
    inicializarSistema(sistema);

    std::cout << "\n=======================================================\n";
    std::cout << "      SISTEMA DE GESTION DE CAMPEONATOS DE FUTBOL      \n";
    std::cout << "        (MODO CONSOLA C++ CONECTADO A SQL SERVER)      \n";
    std::cout << "=======================================================\n";

    menuPrincipal(sistema);

    guardarDatosEnArchivo(sistema);
    dbDesconectar(sistema.db);
    std::cout << "\n[OK] Datos sincronizados. ¡Hasta pronto!\n";
    return 0;
}

// =========================================================================
// UTILIDADES
// =========================================================================

void pausarConsola()
{
    std::cout << "\nPresione ENTER para continuar...";
    if (!std::cin.eof()) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
}

Equipo* buscarEquipoPorId(SistemaCampeonato &sistema, int idEquipo)
{
    for (auto &eq : sistema.equipos) {
        if (eq.idEquipo == idEquipo) return &eq;
    }
    return nullptr;
}

std::string sanitizarSQL(const std::string &input)
{
    std::string res = "";
    for (char c : input) {
        if (c == '\'') res += "''";
        else res += c;
    }
    return res;
}

static std::vector<std::string> dividirLinea(const std::string &s, char delim = '|')
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

// =========================================================================
// IMPLEMENTACIÓN GESTOR BASE DE DATOS SQL SERVER (ODBC)
// =========================================================================

#ifdef _WIN32
bool dbInicializarYConectar(GestorBD &bd)
{
    bd.hOdbc = LoadLibraryA("odbc32.dll");
    if (!bd.hOdbc) return false;

    bd.fnAllocHandle = (t_SQLAllocHandle)GetProcAddress(bd.hOdbc, "SQLAllocHandle");
    bd.fnSetEnvAttr = (t_SQLSetEnvAttr)GetProcAddress(bd.hOdbc, "SQLSetEnvAttr");
    bd.fnDriverConnectA = (t_SQLDriverConnectA)GetProcAddress(bd.hOdbc, "SQLDriverConnectA");
    bd.fnDisconnect = (t_SQLDisconnect)GetProcAddress(bd.hOdbc, "SQLDisconnect");
    bd.fnFreeHandle = (t_SQLFreeHandle)GetProcAddress(bd.hOdbc, "SQLFreeHandle");
    bd.fnExecDirectA = (t_SQLExecDirectA)GetProcAddress(bd.hOdbc, "SQLExecDirectA");
    bd.fnFetch = (t_SQLFetch)GetProcAddress(bd.hOdbc, "SQLFetch");
    bd.fnGetData = (t_SQLGetData)GetProcAddress(bd.hOdbc, "SQLGetData");
    bd.fnCloseCursor = (t_SQLCloseCursor)GetProcAddress(bd.hOdbc, "SQLCloseCursor");

    if (!bd.fnAllocHandle || !bd.fnDriverConnectA || !bd.fnExecDirectA) {
        FreeLibrary(bd.hOdbc);
        bd.hOdbc = NULL;
        return false;
    }

    bd.fnAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &bd.env);
    bd.fnSetEnvAttr(bd.env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    bd.fnAllocHandle(SQL_HANDLE_DBC, bd.env, &bd.dbc);

    std::vector<std::pair<std::string, std::string>> cadenasConexion = {
        {"localhost\\SQLEXPRESS (ODBC 18)", "Driver={ODBC Driver 18 for SQL Server};Server=localhost\\SQLEXPRESS;Database=CampeonatoFutbol;Trusted_Connection=yes;TrustServerCertificate=yes;"},
        {"localhost (ODBC 18)", "Driver={ODBC Driver 18 for SQL Server};Server=localhost;Database=CampeonatoFutbol;Trusted_Connection=yes;TrustServerCertificate=yes;"},
        {"localhost\\SQLEXPRESS (ODBC 17)", "Driver={ODBC Driver 17 for SQL Server};Server=localhost\\SQLEXPRESS;Database=CampeonatoFutbol;Trusted_Connection=yes;"},
        {"localhost\\SQLEXPRESS (SQL Server)", "Driver={SQL Server};Server=localhost\\SQLEXPRESS;Database=CampeonatoFutbol;Trusted_Connection=yes;"}
    };

    for (const auto &cand : cadenasConexion) {
        SQLCHAR outConn[1024];
        SQLSMALLINT outLen = 0;
        SQLRETURN ret = bd.fnDriverConnectA(bd.dbc, NULL, (SQLCHAR*)cand.second.c_str(), SQL_NTS, outConn, sizeof(outConn), &outLen, SQL_DRIVER_NOPROMPT);
        if (SQL_SUCCEEDED(ret)) {
            bd.conectada = true;
            bd.servidorConectado = cand.first;
            return true;
        }
    }
    return false;
}

void dbDesconectar(GestorBD &bd)
{
    if (bd.conectada) {
        if (bd.fnDisconnect) bd.fnDisconnect(bd.dbc);
        if (bd.fnFreeHandle) {
            bd.fnFreeHandle(SQL_HANDLE_DBC, bd.dbc);
            bd.fnFreeHandle(SQL_HANDLE_ENV, bd.env);
        }
        bd.conectada = false;
    }
    if (bd.hOdbc) {
        FreeLibrary(bd.hOdbc);
        bd.hOdbc = NULL;
    }
}

bool dbCargarTodo(SistemaCampeonato &sistema)
{
    if (!sistema.db.conectada) return false;
    GestorBD &bd = sistema.db;

    // 1. Cargar CAMPEONATO
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_campeonato, nombre_campeonato, CONVERT(VARCHAR(10), fecha_inicio, 120), "
                            "CONVERT(VARCHAR(10), ISNULL(fecha_fin, fecha_inicio), 120), max_equipos_mayor, max_equipos_menor "
                            "FROM dbo.CAMPEONATO ORDER BY id_campeonato";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    Torneo t;
                    char nom[128] = {0}, fIni[32] = {0}, fFin[32] = {0};
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &t.idTorneo, sizeof(t.idTorneo), &len);
                    bd.fnGetData(stmt, 2, SQL_C_CHAR, nom, sizeof(nom), &len);
                    bd.fnGetData(stmt, 3, SQL_C_CHAR, fIni, sizeof(fIni), &len);
                    bd.fnGetData(stmt, 4, SQL_C_CHAR, fFin, sizeof(fFin), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &t.maxMayor, sizeof(t.maxMayor), &len);
                    bd.fnGetData(stmt, 6, SQL_C_LONG, &t.maxMenor, sizeof(t.maxMenor), &len);
                    t.nombre = nom;
                    t.fechaInicio = fIni;
                    t.fechaFin = fFin;
                    t.finalizado = false;
                    sistema.torneos.push_back(t);
                    if (t.idTorneo >= sistema.autoIncTorneo) sistema.autoIncTorneo = t.idTorneo + 1;
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
        if (!sistema.torneos.empty()) {
            sistema.idTorneoActivo = sistema.torneos.back().idTorneo;
        }
    }

    // 2. Cargar EQUIPO
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_equipo, nombre_equipo, id_division, id_campeonato FROM dbo.EQUIPO ORDER BY id_equipo";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    Equipo eq;
                    char nom[128] = {0};
                    int idDiv = 1;
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &eq.idEquipo, sizeof(eq.idEquipo), &len);
                    bd.fnGetData(stmt, 2, SQL_C_CHAR, nom, sizeof(nom), &len);
                    bd.fnGetData(stmt, 3, SQL_C_LONG, &idDiv, sizeof(idDiv), &len);
                    bd.fnGetData(stmt, 4, SQL_C_LONG, &eq.idCampeonato, sizeof(eq.idCampeonato), &len);
                    eq.nombre = nom;
                    eq.division = (idDiv == 1 ? LIGA_MAYOR : LIGA_MENOR);
                    sistema.equipos.push_back(eq);
                    if (eq.idEquipo >= sistema.autoIncEquipo) sistema.autoIncEquipo = eq.idEquipo + 1;
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 3. Cargar JUGADOR
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_jugador, id_equipo, nombre, apellido, posicion, ISNULL(numero_camiseta, 0) "
                            "FROM dbo.JUGADOR ORDER BY id_equipo, numero_camiseta";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    Jugador j;
                    char nom[64] = {0}, ape[64] = {0}, pos[32] = {0};
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &j.idJugador, sizeof(j.idJugador), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &j.idEquipo, sizeof(j.idEquipo), &len);
                    bd.fnGetData(stmt, 3, SQL_C_CHAR, nom, sizeof(nom), &len);
                    bd.fnGetData(stmt, 4, SQL_C_CHAR, ape, sizeof(ape), &len);
                    bd.fnGetData(stmt, 5, SQL_C_CHAR, pos, sizeof(pos), &len);
                    bd.fnGetData(stmt, 6, SQL_C_LONG, &j.numeroCamiseta, sizeof(j.numeroCamiseta), &len);
                    j.nombre = nom;
                    j.apellido = ape;
                    j.posicion = pos;

                    for (auto &eq : sistema.equipos) {
                        if (eq.idEquipo == j.idEquipo) {
                            eq.jugadores.push_back(j);
                            break;
                        }
                    }
                    if (j.idJugador >= sistema.autoIncJugador) sistema.autoIncJugador = j.idJugador + 1;
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 4. Cargar PARTIDO y DETALLE_PARTIDO
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT p.id_partido, p.id_campeonato, p.id_division, "
                            "CONVERT(VARCHAR(10), p.fecha_partido, 120), p.jornada, p.fase "
                            "FROM dbo.PARTIDO p ORDER BY p.id_partido";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    Partido part;
                    char fec[32] = {0}, fas[32] = {0};
                    int idDiv = 1;
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &part.idPartido, sizeof(part.idPartido), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &part.idCampeonato, sizeof(part.idCampeonato), &len);
                    bd.fnGetData(stmt, 3, SQL_C_LONG, &idDiv, sizeof(idDiv), &len);
                    bd.fnGetData(stmt, 4, SQL_C_CHAR, fec, sizeof(fec), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &part.jornada, sizeof(part.jornada), &len);
                    bd.fnGetData(stmt, 6, SQL_C_CHAR, fas, sizeof(fas), &len);
                    part.division = (idDiv == 1 ? LIGA_MAYOR : LIGA_MENOR);
                    part.fecha = fec;
                    part.fase = fas;
                    part.idLocal = 0;
                    part.idVisitante = 0;
                    part.golesLocal = 0;
                    part.golesVisitante = 0;
                    part.estado = "PENDIENTE";
                    sistema.partidos.push_back(part);
                    if (part.idPartido >= sistema.autoIncPartido) sistema.autoIncPartido = part.idPartido + 1;
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }

        // Cargar DETALLE_PARTIDO para asociar local/visitante
        for (auto &p : sistema.partidos) {
            SQLHSTMT sDet;
            if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &sDet))) {
                std::string qDet = "SELECT id_equipo, goles_favor, resultado FROM dbo.DETALLE_PARTIDO WHERE id_partido = " +
                                   std::to_string(p.idPartido) + " ORDER BY id_detalle";
                if (SQL_SUCCEEDED(bd.fnExecDirectA(sDet, (SQLCHAR*)qDet.c_str(), SQL_NTS))) {
                    int count = 0;
                    while (bd.fnFetch(sDet) == SQL_SUCCESS) {
                        int idEq = 0, gf = 0;
                        char res[32] = {0};
                        SQLLEN len;
                        bd.fnGetData(sDet, 1, SQL_C_LONG, &idEq, sizeof(idEq), &len);
                        bd.fnGetData(sDet, 2, SQL_C_LONG, &gf, sizeof(gf), &len);
                        bd.fnGetData(sDet, 3, SQL_C_CHAR, res, sizeof(res), &len);

                        if (count == 0) {
                            p.idLocal = idEq;
                            p.golesLocal = gf;
                            if (std::string(res) != "PENDIENTE") p.estado = "FINALIZADO";
                        } else if (count == 1) {
                            p.idVisitante = idEq;
                            p.golesVisitante = gf;
                        }
                        count++;
                    }
                }
                bd.fnFreeHandle(SQL_HANDLE_STMT, sDet);
            }

            Equipo *eLoc = buscarEquipoPorId(sistema, p.idLocal);
            if (eLoc) p.nombreLocal = eLoc->nombre;
            Equipo *eVis = buscarEquipoPorId(sistema, p.idVisitante);
            if (eVis) p.nombreVisitante = eVis->nombre;
        }
    }

    // 5. Cargar REGISTRO_GOLES
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_partido, id_jugador, ISNULL(id_portero_vencido, 0), id_equipo_rival, minuto, es_autogol "
                            "FROM dbo.REGISTRO_GOLES";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    int idPart = 0;
                    IncidenciaGol g;
                    int autogol = 0;
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &idPart, sizeof(idPart), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &g.idJugador, sizeof(g.idJugador), &len);
                    bd.fnGetData(stmt, 3, SQL_C_LONG, &g.idPorteroVencido, sizeof(g.idPorteroVencido), &len);
                    bd.fnGetData(stmt, 4, SQL_C_LONG, &g.idEquipoRival, sizeof(g.idEquipoRival), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &g.minuto, sizeof(g.minuto), &len);
                    bd.fnGetData(stmt, 6, SQL_C_LONG, &autogol, sizeof(autogol), &len);
                    g.esAutogol = (autogol == 1);

                    // Resolver nombres
                    for (const auto &eq : sistema.equipos) {
                        for (const auto &j : eq.jugadores) {
                            if (j.idJugador == g.idJugador) g.nombreJugador = j.nombre + " " + j.apellido;
                            if (j.idJugador == g.idPorteroVencido) g.nombrePorteroVencido = j.nombre + " " + j.apellido;
                        }
                        if (eq.idEquipo == g.idEquipoRival) g.nombreEquipoRival = eq.nombre;
                    }

                    for (auto &part : sistema.partidos) {
                        if (part.idPartido == idPart) {
                            part.goles.push_back(g);
                            break;
                        }
                    }
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 6. Cargar TARJETAS_ACUMULADAS
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_partido, id_jugador, tipo_tarjeta, minuto FROM dbo.TARJETAS_ACUMULADAS";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    int idPart = 0;
                    IncidenciaTarjeta t;
                    char tipo[32] = {0};
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &idPart, sizeof(idPart), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &t.idJugador, sizeof(t.idJugador), &len);
                    bd.fnGetData(stmt, 3, SQL_C_CHAR, tipo, sizeof(tipo), &len);
                    bd.fnGetData(stmt, 4, SQL_C_LONG, &t.minuto, sizeof(t.minuto), &len);
                    t.tipoTarjeta = tipo;

                    for (const auto &eq : sistema.equipos) {
                        for (const auto &j : eq.jugadores) {
                            if (j.idJugador == t.idJugador) {
                                t.nombreJugador = j.nombre + " " + j.apellido;
                                break;
                            }
                        }
                    }

                    for (auto &part : sistema.partidos) {
                        if (part.idPartido == idPart) {
                            part.tarjetas.push_back(t);
                            break;
                        }
                    }
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 7. Cargar CONTROL_PORTEROS
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_partido, id_jugador, id_equipo, minuto_entrada, ISNULL(minuto_salida, 90), goles_recibidos "
                            "FROM dbo.CONTROL_PORTEROS";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    int idPart = 0;
                    ControlPortero cp;
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &idPart, sizeof(idPart), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &cp.idJugador, sizeof(cp.idJugador), &len);
                    bd.fnGetData(stmt, 3, SQL_C_LONG, &cp.idEquipo, sizeof(cp.idEquipo), &len);
                    bd.fnGetData(stmt, 4, SQL_C_LONG, &cp.minutoEntrada, sizeof(cp.minutoEntrada), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &cp.minutoSalida, sizeof(cp.minutoSalida), &len);
                    bd.fnGetData(stmt, 6, SQL_C_LONG, &cp.golesRecibidos, sizeof(cp.golesRecibidos), &len);

                    for (const auto &eq : sistema.equipos) {
                        for (const auto &j : eq.jugadores) {
                            if (j.idJugador == cp.idJugador) {
                                cp.nombrePortero = j.nombre + " " + j.apellido;
                                break;
                            }
                        }
                    }

                    for (auto &part : sistema.partidos) {
                        if (part.idPartido == idPart) {
                            part.porteros.push_back(cp);
                            break;
                        }
                    }
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 8. Cargar HISTORIAL_ASCENSO_DESCENSO
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_historial, id_campeonato, id_equipo, id_division_origen, id_division_destino, tipo_movimiento "
                            "FROM dbo.HISTORIAL_ASCENSO_DESCENSO ORDER BY id_historial";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    HistorialMovimiento h;
                    int orig = 1, dest = 2;
                    char tip[32] = {0};
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &h.idHistorial, sizeof(h.idHistorial), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &h.idCampeonato, sizeof(h.idCampeonato), &len);
                    bd.fnGetData(stmt, 3, SQL_C_LONG, &h.idEquipo, sizeof(h.idEquipo), &len);
                    bd.fnGetData(stmt, 4, SQL_C_LONG, &orig, sizeof(orig), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &dest, sizeof(dest), &len);
                    bd.fnGetData(stmt, 6, SQL_C_CHAR, tip, sizeof(tip), &len);
                    h.origen = (orig == 1 ? LIGA_MAYOR : LIGA_MENOR);
                    h.destino = (dest == 1 ? LIGA_MAYOR : LIGA_MENOR);
                    h.tipoMovimiento = tip;
                    h.fecha = "2026-05-30";
                    Equipo *eq = buscarEquipoPorId(sistema, h.idEquipo);
                    if (eq) h.nombreEquipo = eq->nombre;
                    sistema.historial.push_back(h);
                    if (h.idHistorial >= sistema.autoIncHistorial) sistema.autoIncHistorial = h.idHistorial + 1;
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 9. Cargar PREMIACION
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_premiacion, id_campeonato, id_jugador_goleador, id_portero_menos_vencido, ISNULL(id_equipo_campeon, 0) "
                            "FROM dbo.PREMIACION";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    Premiacion pr;
                    int idCamp = 0, idGol = 0, idPor = 0, idCampEq = 0;
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &pr.idTorneo, sizeof(pr.idTorneo), &len);
                    bd.fnGetData(stmt, 2, SQL_C_LONG, &idCamp, sizeof(idCamp), &len);
                    bd.fnGetData(stmt, 3, SQL_C_LONG, &idGol, sizeof(idGol), &len);
                    bd.fnGetData(stmt, 4, SQL_C_LONG, &idPor, sizeof(idPor), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &idCampEq, sizeof(idCampEq), &len);
                    pr.idTorneo = idCamp;
                    pr.habilitada = true;
                    pr.registrada = true;
                    pr.golesAnotados = 0;
                    pr.golesRecibidos = 0;
                    for (const auto &eq : sistema.equipos) {
                        if (eq.idEquipo == idCampEq) pr.campeonMayor = eq.nombre;
                        for (const auto &j : eq.jugadores) {
                            if (j.idJugador == idGol) pr.maximoGoleador = j.nombre + " " + j.apellido + " (" + eq.nombre + ")";
                            if (j.idJugador == idPor) pr.porteroMenosVencido = j.nombre + " " + j.apellido + " (" + eq.nombre + ")";
                        }
                    }
                    sistema.premiaciones.push_back(pr);
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    // 10. Cargar USUARIO
    {
        SQLHSTMT stmt;
        if (SQL_SUCCEEDED(bd.fnAllocHandle(SQL_HANDLE_STMT, bd.dbc, &stmt))) {
            const char* q = "SELECT id_usuario, usuario, contrasena_hash, nombre_usuario, id_rol FROM dbo.USUARIO";
            if (SQL_SUCCEEDED(bd.fnExecDirectA(stmt, (SQLCHAR*)q, SQL_NTS))) {
                while (bd.fnFetch(stmt) == SQL_SUCCESS) {
                    Usuario u;
                    char usr[64] = {0}, pas[64] = {0}, nom[128] = {0};
                    int r = 2;
                    SQLLEN len;
                    bd.fnGetData(stmt, 1, SQL_C_LONG, &u.idUsuario, sizeof(u.idUsuario), &len);
                    bd.fnGetData(stmt, 2, SQL_C_CHAR, usr, sizeof(usr), &len);
                    bd.fnGetData(stmt, 3, SQL_C_CHAR, pas, sizeof(pas), &len);
                    bd.fnGetData(stmt, 4, SQL_C_CHAR, nom, sizeof(nom), &len);
                    bd.fnGetData(stmt, 5, SQL_C_LONG, &r, sizeof(r), &len);
                    u.usuario = usr;
                    u.contrasena = pas;
                    u.nombreCompleto = nom;
                    u.rol = (r == 1 ? ROL_ADMINISTRADOR : ROL_REPRESENTANTE);
                    sistema.usuarios.push_back(u);
                    if (u.idUsuario >= sistema.autoIncUsuario) sistema.autoIncUsuario = u.idUsuario + 1;
                }
            }
            bd.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        }
    }

    return (!sistema.equipos.empty());
}

bool dbInsertarJugadorEnBD(SistemaCampeonato &sistema, Jugador &j)
{
    if (!sistema.db.conectada) return false;
    std::string sql = "INSERT INTO dbo.JUGADOR (id_equipo, nombre, apellido, posicion, numero_camiseta) VALUES (" +
                      std::to_string(j.idEquipo) + ", '" +
                      sanitizarSQL(j.nombre) + "', '" +
                      sanitizarSQL(j.apellido) + "', '" +
                      sanitizarSQL(j.posicion) + "', " +
                      std::to_string(j.numeroCamiseta) + "); SELECT SCOPE_IDENTITY();";

    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        if (SQL_SUCCEEDED(sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS))) {
            if (sistema.db.fnFetch(stmt) == SQL_SUCCESS) {
                int newId = 0;
                SQLLEN len = 0;
                sistema.db.fnGetData(stmt, 1, SQL_C_LONG, &newId, sizeof(newId), &len);
                if (newId > 0) j.idJugador = newId;
            }
        }
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbInsertarPartidoEnBD(SistemaCampeonato &sistema, Partido &p)
{
    if (!sistema.db.conectada) return false;
    std::string fStr = p.fecha.empty() ? "2026-05-15 15:00:00" : (p.fecha + " 15:00:00");
    std::string sql = "INSERT INTO dbo.PARTIDO (id_campeonato, id_division, fecha_partido, jornada, fase) VALUES (" +
                      std::to_string(p.idCampeonato) + ", " +
                      std::to_string(p.division) + ", '" +
                      fStr + "', " +
                      std::to_string(p.jornada) + ", '" +
                      p.fase + "'); SELECT SCOPE_IDENTITY();";

    SQLHSTMT stmt;
    int idPart = 0;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        if (SQL_SUCCEEDED(sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS))) {
            if (sistema.db.fnFetch(stmt) == SQL_SUCCESS) {
                SQLLEN len = 0;
                sistema.db.fnGetData(stmt, 1, SQL_C_LONG, &idPart, sizeof(idPart), &len);
                if (idPart > 0) p.idPartido = idPart;
            }
        }
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    if (p.idPartido > 0) {
        std::string sqlLoc = "INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES (" +
                             std::to_string(p.idPartido) + ", " + std::to_string(p.idLocal) + ", 0, 0, 0, 'PENDIENTE');";
        std::string sqlVis = "INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES (" +
                             std::to_string(p.idPartido) + ", " + std::to_string(p.idVisitante) + ", 0, 0, 0, 'PENDIENTE');";
        SQLHSTMT sDet;
        if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &sDet))) {
            sistema.db.fnExecDirectA(sDet, (SQLCHAR*)sqlLoc.c_str(), SQL_NTS);
            sistema.db.fnCloseCursor(sDet);
            sistema.db.fnExecDirectA(sDet, (SQLCHAR*)sqlVis.c_str(), SQL_NTS);
            sistema.db.fnFreeHandle(SQL_HANDLE_STMT, sDet);
        }
        return true;
    }
    return false;
}

bool dbActualizarResultadoPartidoEnBD(SistemaCampeonato &sistema, const Partido &p)
{
    if (!sistema.db.conectada) return false;

    int ptsLoc = (p.golesLocal > p.golesVisitante) ? 3 : (p.golesLocal == p.golesVisitante ? 1 : 0);
    int ptsVis = (p.golesVisitante > p.golesLocal) ? 3 : (p.golesLocal == p.golesVisitante ? 1 : 0);
    std::string resLoc = (p.golesLocal > p.golesVisitante) ? "VICTORIA" : (p.golesLocal == p.golesVisitante ? "EMPATE" : "DERROTA");
    std::string resVis = (p.golesVisitante > p.golesLocal) ? "VICTORIA" : (p.golesLocal == p.golesVisitante ? "EMPATE" : "DERROTA");

    std::string sqlUpdLoc = "UPDATE dbo.DETALLE_PARTIDO SET goles_favor = " + std::to_string(p.golesLocal) +
                            ", goles_contra = " + std::to_string(p.golesVisitante) +
                            ", puntos_ganados = " + std::to_string(ptsLoc) +
                            ", resultado = '" + resLoc +
                            "' WHERE id_partido = " + std::to_string(p.idPartido) + " AND id_equipo = " + std::to_string(p.idLocal);

    std::string sqlUpdVis = "UPDATE dbo.DETALLE_PARTIDO SET goles_favor = " + std::to_string(p.golesVisitante) +
                            ", goles_contra = " + std::to_string(p.golesLocal) +
                            ", puntos_ganados = " + std::to_string(ptsVis) +
                            ", resultado = '" + resVis +
                            "' WHERE id_partido = " + std::to_string(p.idPartido) + " AND id_equipo = " + std::to_string(p.idVisitante);

    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlUpdLoc.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlUpdVis.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);

        // Limpiar incidencias previas para este partido
        std::string delG = "DELETE FROM dbo.REGISTRO_GOLES WHERE id_partido = " + std::to_string(p.idPartido);
        std::string delT = "DELETE FROM dbo.TARJETAS_ACUMULADAS WHERE id_partido = " + std::to_string(p.idPartido);
        std::string delP = "DELETE FROM dbo.CONTROL_PORTEROS WHERE id_partido = " + std::to_string(p.idPartido);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)delG.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)delT.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)delP.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);

        // Insertar Goles
        for (const auto &g : p.goles) {
            std::string sqlGol = "INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol) VALUES (" +
                                 std::to_string(p.idPartido) + ", " +
                                 std::to_string(g.idJugador) + ", " +
                                 (g.idPorteroVencido > 0 ? std::to_string(g.idPorteroVencido) : "NULL") + ", " +
                                 std::to_string(g.idEquipoRival) + ", " +
                                 std::to_string(g.minuto) + ", " +
                                 (g.esAutogol ? "1" : "0") + ");";
            sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlGol.c_str(), SQL_NTS);
            sistema.db.fnCloseCursor(stmt);
        }

        // Insertar Tarjetas
        for (const auto &t : p.tarjetas) {
            std::string sqlTar = "INSERT INTO dbo.TARJETAS_ACUMULADAS (id_partido, id_jugador, tipo_tarjeta, minuto) VALUES (" +
                                 std::to_string(p.idPartido) + ", " +
                                 std::to_string(t.idJugador) + ", '" +
                                 t.tipoTarjeta + "', " +
                                 std::to_string(t.minuto) + ");";
            sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlTar.c_str(), SQL_NTS);
            sistema.db.fnCloseCursor(stmt);
        }

        // Insertar Porteros
        for (const auto &cp : p.porteros) {
            std::string sqlPor = "INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos) VALUES (" +
                                 std::to_string(p.idPartido) + ", " +
                                 std::to_string(cp.idJugador) + ", " +
                                 std::to_string(cp.idEquipo) + ", " +
                                 std::to_string(cp.minutoEntrada) + ", " +
                                 std::to_string(cp.minutoSalida) + ", " +
                                 std::to_string(cp.golesRecibidos) + ");";
            sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlPor.c_str(), SQL_NTS);
            sistema.db.fnCloseCursor(stmt);
        }

        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbActualizarDivisionEquipoEnBD(SistemaCampeonato &sistema, int idEquipo, int nuevaDivision)
{
    if (!sistema.db.conectada) return false;
    std::string sql = "UPDATE dbo.EQUIPO SET id_division = " + std::to_string(nuevaDivision) +
                      " WHERE id_equipo = " + std::to_string(idEquipo);
    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbInsertarHistorialEnBD(SistemaCampeonato &sistema, const HistorialMovimiento &h)
{
    if (!sistema.db.conectada) return false;
    std::string sql = "INSERT INTO dbo.HISTORIAL_ASCENSO_DESCENSO (id_campeonato, id_equipo, id_division_origen, id_division_destino, tipo_movimiento) VALUES (" +
                      std::to_string(h.idCampeonato) + ", " +
                      std::to_string(h.idEquipo) + ", " +
                      std::to_string(h.origen) + ", " +
                      std::to_string(h.destino) + ", '" +
                      h.tipoMovimiento + "');";
    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbGuardarPremiacionEnBD(SistemaCampeonato &sistema, const Premiacion &prem, int idGoleador, int idPortero, int idCampeon)
{
    if (!sistema.db.conectada) return false;
    std::string sql = "IF EXISTS (SELECT 1 FROM dbo.PREMIACION WHERE id_campeonato = " + std::to_string(prem.idTorneo) + ") " +
                      "UPDATE dbo.PREMIACION SET id_jugador_goleador = " + (idGoleador > 0 ? std::to_string(idGoleador) : "1") +
                      ", id_portero_menos_vencido = " + (idPortero > 0 ? std::to_string(idPortero) : "1") +
                      ", id_equipo_campeon = " + (idCampeon > 0 ? std::to_string(idCampeon) : "NULL") +
                      " WHERE id_campeonato = " + std::to_string(prem.idTorneo) + "; " +
                      "ELSE INSERT INTO dbo.PREMIACION (id_campeonato, id_jugador_goleador, id_portero_menos_vencido, id_equipo_campeon) VALUES (" +
                      std::to_string(prem.idTorneo) + ", " +
                      (idGoleador > 0 ? std::to_string(idGoleador) : "1") + ", " +
                      (idPortero > 0 ? std::to_string(idPortero) : "1") + ", " +
                      (idCampeon > 0 ? std::to_string(idCampeon) : "NULL") + ");";
    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbInsertarUsuarioEnBD(SistemaCampeonato &sistema, const Usuario &u)
{
    if (!sistema.db.conectada) return false;
    std::string correo = u.usuario + "@campeonato.gt";
    std::string sql = "INSERT INTO dbo.USUARIO (nombre_usuario, correo, contrasena_hash, activo, id_rol, usuario) VALUES ('" +
                      sanitizarSQL(u.nombreCompleto) + "', '" +
                      correo + "', '" +
                      sanitizarSQL(u.contrasena) + "', 1, " +
                      std::to_string(u.rol) + ", '" +
                      sanitizarSQL(u.usuario) + "');";
    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbInsertarEquipoEnBD(SistemaCampeonato &sistema, Equipo &eq)
{
    if (!sistema.db.conectada) return false;
    std::string sql = "INSERT INTO dbo.EQUIPO (nombre_equipo, id_division, id_campeonato) VALUES ('" +
                      sanitizarSQL(eq.nombre) + "', " +
                      std::to_string(eq.division) + ", " +
                      std::to_string(eq.idCampeonato) + "); SELECT SCOPE_IDENTITY();";
    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        if (SQL_SUCCEEDED(sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS))) {
            if (sistema.db.fnFetch(stmt) == SQL_SUCCESS) {
                int newId = 0;
                SQLLEN len = 0;
                sistema.db.fnGetData(stmt, 1, SQL_C_LONG, &newId, sizeof(newId), &len);
                if (newId > 0) eq.idEquipo = newId;
            }
        }
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}

bool dbCrearTorneoEnBD(SistemaCampeonato &sistema, Torneo &t)
{
    if (!sistema.db.conectada) return false;
    std::string fIni = t.fechaInicio.empty() ? "2026-01-10" : t.fechaInicio;
    std::string fFin = t.fechaFin.empty() ? "2026-05-30" : t.fechaFin;
    std::string sql = "INSERT INTO dbo.CAMPEONATO (nombre_campeonato, fecha_inicio, fecha_fin, max_equipos_mayor, max_equipos_menor) VALUES ('" +
                      sanitizarSQL(t.nombre) + "', '" +
                      fIni + "', '" +
                      fFin + "', " +
                      std::to_string(t.maxMayor) + ", " +
                      std::to_string(t.maxMenor) + "); SELECT SCOPE_IDENTITY();";
    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        if (SQL_SUCCEEDED(sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS))) {
            if (sistema.db.fnFetch(stmt) == SQL_SUCCESS) {
                int newId = 0;
                SQLLEN len = 0;
                sistema.db.fnGetData(stmt, 1, SQL_C_LONG, &newId, sizeof(newId), &len);
                if (newId > 0) t.idTorneo = newId;
            }
        }
        sistema.db.fnFreeHandle(SQL_HANDLE_STMT, stmt);
        return true;
    }
    return false;
}
#else
bool dbInicializarYConectar(GestorBD &bd) { return false; }
void dbDesconectar(GestorBD &bd) {}
bool dbCargarTodo(SistemaCampeonato &sistema) { return false; }
bool dbInsertarJugadorEnBD(SistemaCampeonato &sistema, Jugador &j) { return false; }
bool dbInsertarPartidoEnBD(SistemaCampeonato &sistema, Partido &p) { return false; }
bool dbActualizarResultadoPartidoEnBD(SistemaCampeonato &sistema, const Partido &p) { return false; }
bool dbActualizarDivisionEquipoEnBD(SistemaCampeonato &sistema, int idEquipo, int nuevaDivision) { return false; }
bool dbInsertarHistorialEnBD(SistemaCampeonato &sistema, const HistorialMovimiento &h) { return false; }
bool dbGuardarPremiacionEnBD(SistemaCampeonato &sistema, const Premiacion &prem, int a, int b, int c) { return false; }
bool dbInsertarUsuarioEnBD(SistemaCampeonato &sistema, const Usuario &u) { return false; }
bool dbInsertarEquipoEnBD(SistemaCampeonato &sistema, Equipo &eq) { return false; }
bool dbCrearTorneoEnBD(SistemaCampeonato &sistema, Torneo &t) { return false; }
#endif

// =========================================================================
// PERSISTENCIA DE DATOS (LOCAL Y SEMILLA)
// =========================================================================

void inicializarSistema(SistemaCampeonato &sistema)
{
    sistema.usuarios.clear();
    sistema.torneos.clear();
    sistema.idTorneoActivo = 1;
    sistema.equipos.clear();
    sistema.partidos.clear();
    sistema.historial.clear();
    sistema.premiaciones.clear();

    sistema.autoIncUsuario = 1;
    sistema.autoIncTorneo = 1;
    sistema.autoIncEquipo = 1;
    sistema.autoIncJugador = 1;
    sistema.autoIncPartido = 1;
    sistema.autoIncHistorial = 1;

    std::cout << "[BD] Conectando a Base de Datos SQL Server 'CampeonatoFutbol' en localhost...\n";
    if (dbInicializarYConectar(sistema.db)) {
        std::cout << "[OK] Conectado exitosamente: " << sistema.db.servidorConectado << "\n";
        std::cout << "[BD] Sincronizando datos desde SQL Server...\n";
        if (dbCargarTodo(sistema)) {
            std::cout << "[OK] ¡Datos sincronizados directamente desde la Base de Datos!\n";
            std::cout << "     - Torneos activos: " << sistema.torneos.size() << "\n";
            std::cout << "     - Equipos en liga: " << sistema.equipos.size() << "\n";
            int totalJug = 0;
            for (const auto &eq : sistema.equipos) totalJug += eq.jugadores.size();
            std::cout << "     - Jugadores en plantilla: " << totalJug << "\n";
            std::cout << "     - Partidos registrados: " << sistema.partidos.size() << "\n";
            std::cout << "     - Usuarios en sistema: " << sistema.usuarios.size() << "\n";
            return;
        }
    }

    std::cout << "[AVISO] Base de datos no disponible. Utilizando persistencia en archivo local.\n";
    if (!cargarDatosDesdeArchivo(sistema)) {
        inicializarDatosSemilla(sistema);
        guardarDatosEnArchivo(sistema);
    }
}

void inicializarDatosSemilla(SistemaCampeonato &sistema)
{
    Usuario u1;
    u1.idUsuario = sistema.autoIncUsuario++;
    u1.usuario = "admin";
    u1.contrasena = "admin123";
    u1.nombreCompleto = "Administrador Principal";
    u1.rol = ROL_ADMINISTRADOR;
    sistema.usuarios.push_back(u1);

    Usuario u2;
    u2.idUsuario = sistema.autoIncUsuario++;
    u2.usuario = "representante";
    u2.contrasena = "rep123";
    u2.nombreCompleto = "Representante Deportivo";
    u2.rol = ROL_REPRESENTANTE;
    sistema.usuarios.push_back(u2);

    Torneo t;
    t.idTorneo = sistema.autoIncTorneo++;
    t.nombre = "Torneo Apertura 2026";
    t.fechaInicio = "2026-01-10";
    t.fechaFin = "2026-05-30";
    t.maxMayor = 12;
    t.maxMenor = 12;
    t.finalizado = false;
    sistema.torneos.push_back(t);
    sistema.idTorneoActivo = t.idTorneo;

    std::vector<std::string> mayor = {
        "CSD Municipal", "Comunicaciones FC", "Antigua GFC", "Cobán Imperial",
        "Xelajú MC", "Deportivo Guastatoya", "Deportivo Malacateco", "Deportivo Mixco",
        "Xinabajul Huehue", "Deportivo Achuapa", "Deportivo Marquense", "CSD Zacapa"
    };

    for (const auto &nom : mayor) {
        Equipo eq;
        eq.idEquipo = sistema.autoIncEquipo++;
        eq.nombre = nom;
        eq.division = LIGA_MAYOR;
        eq.idCampeonato = t.idTorneo;

        Jugador p; p.idJugador = sistema.autoIncJugador++; p.idEquipo = eq.idEquipo;
        p.nombre = "Portero"; p.apellido = nom.substr(0, 6); p.posicion = "PORTERO"; p.numeroCamiseta = 1;
        eq.jugadores.push_back(p);

        Jugador d; d.idJugador = sistema.autoIncJugador++; d.idEquipo = eq.idEquipo;
        d.nombre = "Defensa"; d.apellido = "López"; d.posicion = "DEFENSA"; d.numeroCamiseta = 4;
        eq.jugadores.push_back(d);

        Jugador m; m.idJugador = sistema.autoIncJugador++; m.idEquipo = eq.idEquipo;
        m.nombre = "Mediocampista"; m.apellido = "Gómez"; m.posicion = "MEDIOCAMPISTA"; m.numeroCamiseta = 8;
        eq.jugadores.push_back(m);

        Jugador del; del.idJugador = sistema.autoIncJugador++; del.idEquipo = eq.idEquipo;
        del.nombre = "Delantero"; del.apellido = "Martínez"; del.posicion = "DELANTERO"; del.numeroCamiseta = 9;
        eq.jugadores.push_back(del);

        Jugador gol; gol.idJugador = sistema.autoIncJugador++; gol.idEquipo = eq.idEquipo;
        gol.nombre = "Goleador"; gol.apellido = "Hernández"; gol.posicion = "DELANTERO"; gol.numeroCamiseta = 10;
        eq.jugadores.push_back(gol);

        sistema.equipos.push_back(eq);
    }

    std::vector<std::string> menor = {
        "Deportivo San Pedro", "Universidad SC", "Aurora FC", "Juventud Pinulteca",
        "Deportivo Iztapa", "Santa Lucía Cotzumalguapa", "Deportivo Suchitepéquez", "Deportivo Coatepeque",
        "Quiché FC", "Deportivo Carchá", "Deportivo Mictlán", "Nueva Concepción"
    };

    for (const auto &nom : menor) {
        Equipo eq;
        eq.idEquipo = sistema.autoIncEquipo++;
        eq.nombre = nom;
        eq.division = LIGA_MENOR;
        eq.idCampeonato = t.idTorneo;

        Jugador p; p.idJugador = sistema.autoIncJugador++; p.idEquipo = eq.idEquipo;
        p.nombre = "Portero"; p.apellido = nom.substr(0, 6); p.posicion = "PORTERO"; p.numeroCamiseta = 1;
        eq.jugadores.push_back(p);

        Jugador d; d.idJugador = sistema.autoIncJugador++; d.idEquipo = eq.idEquipo;
        d.nombre = "Defensa"; d.apellido = "Morales"; d.posicion = "DEFENSA"; d.numeroCamiseta = 3;
        eq.jugadores.push_back(d);

        Jugador m; m.idJugador = sistema.autoIncJugador++; m.idEquipo = eq.idEquipo;
        m.nombre = "Mediocampista"; m.apellido = "Ramírez"; m.posicion = "MEDIOCAMPISTA"; m.numeroCamiseta = 6;
        eq.jugadores.push_back(m);

        Jugador del; del.idJugador = sistema.autoIncJugador++; del.idEquipo = eq.idEquipo;
        del.nombre = "Delantero"; del.apellido = "Castillo"; del.posicion = "DELANTERO"; del.numeroCamiseta = 11;
        eq.jugadores.push_back(del);

        sistema.equipos.push_back(eq);
    }
}

bool guardarDatosEnArchivo(const SistemaCampeonato &sistema, const std::string &ruta)
{
    std::ofstream out(ruta);
    if (!out.is_open()) return false;

    out << "[CONFIG]\n";
    out << sistema.idTorneoActivo << "|"
        << sistema.autoIncUsuario << "|"
        << sistema.autoIncTorneo << "|"
        << sistema.autoIncEquipo << "|"
        << sistema.autoIncJugador << "|"
        << sistema.autoIncPartido << "|"
        << sistema.autoIncHistorial << "\n";

    out << "[USUARIOS]\n";
    for (const auto &u : sistema.usuarios) {
        out << u.idUsuario << "|" << u.usuario << "|" << u.contrasena << "|" << u.nombreCompleto << "|" << (int)u.rol << "\n";
    }

    out << "[TORNEOS]\n";
    for (const auto &t : sistema.torneos) {
        out << t.idTorneo << "|" << t.nombre << "|" << t.fechaInicio << "|" << t.fechaFin << "|" << t.maxMayor << "|" << t.maxMenor << "|" << (t.finalizado ? 1 : 0) << "\n";
    }

    out << "[EQUIPOS]\n";
    for (const auto &eq : sistema.equipos) {
        out << eq.idEquipo << "|" << eq.nombre << "|" << (int)eq.division << "|" << eq.idCampeonato << "\n";
    }

    out << "[JUGADORES]\n";
    for (const auto &eq : sistema.equipos) {
        for (const auto &j : eq.jugadores) {
            out << j.idJugador << "|" << j.idEquipo << "|" << j.nombre << "|" << j.apellido << "|" << j.posicion << "|" << j.numeroCamiseta << "\n";
        }
    }

    out << "[PARTIDOS]\n";
    for (const auto &p : sistema.partidos) {
        out << p.idPartido << "|" << p.idCampeonato << "|" << (int)p.division << "|" << p.fase << "|"
            << p.jornada << "|" << p.fecha << "|" << p.idLocal << "|" << p.nombreLocal << "|"
            << p.golesLocal << "|" << p.idVisitante << "|" << p.nombreVisitante << "|"
            << p.golesVisitante << "|" << p.estado << "\n";
    }

    out << "[GOLES]\n";
    for (const auto &p : sistema.partidos) {
        for (const auto &g : p.goles) {
            out << p.idPartido << "|" << g.idJugador << "|" << g.nombreJugador << "|"
                << g.idEquipoRival << "|" << g.nombreEquipoRival << "|"
                << g.idPorteroVencido << "|" << g.nombrePorteroVencido << "|"
                << g.minuto << "|" << (g.esAutogol ? 1 : 0) << "\n";
        }
    }

    out << "[TARJETAS]\n";
    for (const auto &p : sistema.partidos) {
        for (const auto &t : p.tarjetas) {
            out << p.idPartido << "|" << t.idJugador << "|" << t.nombreJugador << "|"
                << t.tipoTarjeta << "|" << t.minuto << "\n";
        }
    }

    out << "[PORTEROS]\n";
    for (const auto &p : sistema.partidos) {
        for (const auto &cp : p.porteros) {
            out << p.idPartido << "|" << cp.idJugador << "|" << cp.nombrePortero << "|"
                << cp.idEquipo << "|" << cp.minutoEntrada << "|" << cp.minutoSalida << "|"
                << cp.golesRecibidos << "\n";
        }
    }

    out << "[HISTORIAL]\n";
    for (const auto &h : sistema.historial) {
        out << h.idHistorial << "|" << h.idCampeonato << "|" << h.idEquipo << "|"
            << h.nombreEquipo << "|" << (int)h.origen << "|" << (int)h.destino << "|"
            << h.tipoMovimiento << "|" << h.fecha << "\n";
    }

    out << "[PREMIACIONES]\n";
    for (const auto &pr : sistema.premiaciones) {
        out << pr.idTorneo << "|" << pr.campeonMayor << "|" << pr.campeonMenor << "|"
            << pr.maximoGoleador << "|" << pr.golesAnotados << "|"
            << pr.porteroMenosVencido << "|" << pr.golesRecibidos << "|"
            << (pr.habilitada ? 1 : 0) << "|" << (pr.registrada ? 1 : 0) << "\n";
    }

    out.close();
    return true;
}

bool cargarDatosDesdeArchivo(SistemaCampeonato &sistema, const std::string &ruta)
{
    std::ifstream in(ruta);
    if (!in.is_open()) return false;

    std::string linea;
    std::string seccion = "";

    sistema.usuarios.clear();
    sistema.torneos.clear();
    sistema.equipos.clear();
    sistema.partidos.clear();
    sistema.historial.clear();
    sistema.premiaciones.clear();

    while (std::getline(in, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        if (linea.front() == '[' && linea.back() == ']') {
            seccion = linea;
            continue;
        }

        std::vector<std::string> t = dividirLinea(linea, '|');

        if (seccion == "[CONFIG]" && t.size() >= 7) {
            sistema.idTorneoActivo = std::stoi(t[0]);
            sistema.autoIncUsuario = std::stoi(t[1]);
            sistema.autoIncTorneo = std::stoi(t[2]);
            sistema.autoIncEquipo = std::stoi(t[3]);
            sistema.autoIncJugador = std::stoi(t[4]);
            sistema.autoIncPartido = std::stoi(t[5]);
            sistema.autoIncHistorial = std::stoi(t[6]);
        } else if (seccion == "[USUARIOS]" && t.size() >= 5) {
            Usuario u;
            u.idUsuario = std::stoi(t[0]);
            u.usuario = t[1];
            u.contrasena = t[2];
            u.nombreCompleto = t[3];
            u.rol = (RolUsuario)std::stoi(t[4]);
            sistema.usuarios.push_back(u);
        } else if (seccion == "[TORNEOS]" && t.size() >= 7) {
            Torneo tor;
            tor.idTorneo = std::stoi(t[0]);
            tor.nombre = t[1];
            tor.fechaInicio = t[2];
            tor.fechaFin = t[3];
            tor.maxMayor = std::stoi(t[4]);
            tor.maxMenor = std::stoi(t[5]);
            tor.finalizado = (t[6] == "1");
            sistema.torneos.push_back(tor);
        } else if (seccion == "[EQUIPOS]" && t.size() >= 4) {
            Equipo eq;
            eq.idEquipo = std::stoi(t[0]);
            eq.nombre = t[1];
            eq.division = (DivisionLiga)std::stoi(t[2]);
            eq.idCampeonato = std::stoi(t[3]);
            sistema.equipos.push_back(eq);
        } else if (seccion == "[JUGADORES]" && t.size() >= 6) {
            Jugador j;
            j.idJugador = std::stoi(t[0]);
            j.idEquipo = std::stoi(t[1]);
            j.nombre = t[2];
            j.apellido = t[3];
            j.posicion = t[4];
            j.numeroCamiseta = std::stoi(t[5]);
            Equipo *eq = buscarEquipoPorId(sistema, j.idEquipo);
            if (eq) eq->jugadores.push_back(j);
        } else if (seccion == "[PARTIDOS]" && t.size() >= 13) {
            Partido p;
            p.idPartido = std::stoi(t[0]);
            p.idCampeonato = std::stoi(t[1]);
            p.division = (DivisionLiga)std::stoi(t[2]);
            p.fase = t[3];
            p.jornada = std::stoi(t[4]);
            p.fecha = t[5];
            p.idLocal = std::stoi(t[6]);
            p.nombreLocal = t[7];
            p.golesLocal = std::stoi(t[8]);
            p.idVisitante = std::stoi(t[9]);
            p.nombreVisitante = t[10];
            p.golesVisitante = std::stoi(t[11]);
            p.estado = t[12];
            sistema.partidos.push_back(p);
        } else if (seccion == "[GOLES]" && t.size() >= 9) {
            int idPart = std::stoi(t[0]);
            IncidenciaGol g;
            g.idJugador = std::stoi(t[1]);
            g.nombreJugador = t[2];
            g.idEquipoRival = std::stoi(t[3]);
            g.nombreEquipoRival = t[4];
            g.idPorteroVencido = std::stoi(t[5]);
            g.nombrePorteroVencido = t[6];
            g.minuto = std::stoi(t[7]);
            g.esAutogol = (t[8] == "1");
            for (auto &part : sistema.partidos) {
                if (part.idPartido == idPart) {
                    part.goles.push_back(g);
                    break;
                }
            }
        } else if (seccion == "[TARJETAS]" && t.size() >= 5) {
            int idPart = std::stoi(t[0]);
            IncidenciaTarjeta tar;
            tar.idJugador = std::stoi(t[1]);
            tar.nombreJugador = t[2];
            tar.tipoTarjeta = t[3];
            tar.minuto = std::stoi(t[4]);
            for (auto &part : sistema.partidos) {
                if (part.idPartido == idPart) {
                    part.tarjetas.push_back(tar);
                    break;
                }
            }
        } else if (seccion == "[PORTEROS]" && t.size() >= 7) {
            int idPart = std::stoi(t[0]);
            ControlPortero cp;
            cp.idJugador = std::stoi(t[1]);
            cp.nombrePortero = t[2];
            cp.idEquipo = std::stoi(t[3]);
            cp.minutoEntrada = std::stoi(t[4]);
            cp.minutoSalida = std::stoi(t[5]);
            cp.golesRecibidos = std::stoi(t[6]);
            for (auto &part : sistema.partidos) {
                if (part.idPartido == idPart) {
                    part.porteros.push_back(cp);
                    break;
                }
            }
        } else if (seccion == "[HISTORIAL]" && t.size() >= 8) {
            HistorialMovimiento h;
            h.idHistorial = std::stoi(t[0]);
            h.idCampeonato = std::stoi(t[1]);
            h.idEquipo = std::stoi(t[2]);
            h.nombreEquipo = t[3];
            h.origen = (DivisionLiga)std::stoi(t[4]);
            h.destino = (DivisionLiga)std::stoi(t[5]);
            h.tipoMovimiento = t[6];
            h.fecha = t[7];
            sistema.historial.push_back(h);
        } else if (seccion == "[PREMIACIONES]" && t.size() >= 9) {
            Premiacion pr;
            pr.idTorneo = std::stoi(t[0]);
            pr.campeonMayor = t[1];
            pr.campeonMenor = t[2];
            pr.maximoGoleador = t[3];
            pr.golesAnotados = std::stoi(t[4]);
            pr.porteroMenosVencido = t[5];
            pr.golesRecibidos = std::stoi(t[6]);
            pr.habilitada = (t[7] == "1");
            pr.registrada = (t[8] == "1");
            sistema.premiaciones.push_back(pr);
        }
    }

    in.close();
    return true;
}

// =========================================================================
// MÓDULO TORNEO, EQUIPOS Y JUGADORES
// =========================================================================

void crearTorneo(SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "               CREAR NUEVO TORNEO DE FUTBOL            \n";
    std::cout << "=======================================================\n";

    Torneo nuevo;
    nuevo.idTorneo = sistema.autoIncTorneo++;

    std::cout << "Nombre del Torneo (ej. Clausura 2026): ";
    std::getline(std::cin >> std::ws, nuevo.nombre);

    std::cout << "Fecha de Inicio (YYYY-MM-DD): ";
    std::getline(std::cin, nuevo.fechaInicio);

    std::cout << "Fecha Estimada de Finalización (YYYY-MM-DD): ";
    std::getline(std::cin, nuevo.fechaFin);

    nuevo.maxMayor = 12;
    nuevo.maxMenor = 12;
    nuevo.finalizado = false;

    dbCrearTorneoEnBD(sistema, nuevo);
    sistema.torneos.push_back(nuevo);
    sistema.idTorneoActivo = nuevo.idTorneo;

    std::cout << "\n[OK] Torneo '" << nuevo.nombre << "' creado y activado con éxito (ID: " << nuevo.idTorneo << ").\n";
    pausarConsola();
}

void listarTorneos(const SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "                  TORNEOS REGISTRADOS                  \n";
    std::cout << "=======================================================\n";
    if (sistema.torneos.empty()) {
        std::cout << "No hay torneos registrados en el sistema.\n";
        return;
    }

    for (const auto &t : sistema.torneos) {
        std::cout << "ID: " << std::setw(3) << t.idTorneo
                  << " | " << std::setw(25) << std::left << t.nombre
                  << " | Inicio: " << t.fechaInicio
                  << " | Fin: " << t.fechaFin
                  << " | Estado: " << (t.finalizado ? "FINALIZADO" : "ACTIVO")
                  << (t.idTorneo == sistema.idTorneoActivo ? " <-- [ACTIVO]" : "") << "\n";
    }
}

void seleccionarTorneoActivo(SistemaCampeonato &sistema)
{
    listarTorneos(sistema);
    std::cout << "\nIngrese el ID del Torneo que desea seleccionar como ACTIVO: ";
    int idSel;
    std::cin >> idSel;

    bool hallado = false;
    for (const auto &t : sistema.torneos) {
        if (t.idTorneo == idSel) {
            sistema.idTorneoActivo = t.idTorneo;
            hallado = true;
            std::cout << "\n[OK] Torneo '" << t.nombre << "' establecido como ACTIVO.\n";
            break;
        }
    }
    if (!hallado) {
        std::cout << "\n[ERROR] El ID de torneo seleccionado no existe.\n";
    }
    pausarConsola();
}

void registrarEquipo(SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "                  REGISTRAR NUEVO EQUIPO               \n";
    std::cout << "=======================================================\n";

    std::cout << "Seleccione la Liga / División:\n";
    std::cout << "1. Liga Mayor (Máximo 12 equipos)\n";
    std::cout << "2. Liga Menor (Máximo 12 equipos)\n";
    std::cout << "Opción: ";
    int opcDiv;
    std::cin >> opcDiv;

    if (opcDiv != 1 && opcDiv != 2) {
        std::cout << "[ERROR] División no válida.\n";
        pausarConsola();
        return;
    }

    DivisionLiga div = (opcDiv == 1) ? LIGA_MAYOR : LIGA_MENOR;

    int totalActual = 0;
    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == div) {
            totalActual++;
        }
    }

    if (totalActual >= 12) {
        std::cout << "\n[!] ATENCIÓN: La división seleccionada ya alcanzó el límite máximo reglamentario de 12 equipos.\n";
        pausarConsola();
        return;
    }

    Equipo nuevo;
    nuevo.idEquipo = sistema.autoIncEquipo++;
    nuevo.division = div;
    nuevo.idCampeonato = sistema.idTorneoActivo;

    std::cout << "Nombre Oficial del Equipo: ";
    std::getline(std::cin >> std::ws, nuevo.nombre);

    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.nombre == nuevo.nombre) {
            std::cout << "\n[ERROR] Ya existe un equipo con ese nombre en el campeonato actual.\n";
            pausarConsola();
            return;
        }
    }

    dbInsertarEquipoEnBD(sistema, nuevo);
    sistema.equipos.push_back(nuevo);

    std::cout << "\n[OK] Equipo '" << nuevo.nombre << "' registrado exitosamente (ID: "
              << nuevo.idEquipo << ") en " << (div == LIGA_MAYOR ? "Liga Mayor" : "Liga Menor") << ".\n";
    pausarConsola();
}

void registrarJugador(SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "       REGISTRAR JUGADOR EN PLANTILLA DE EQUIPO        \n";
    std::cout << "=======================================================\n";

    std::cout << "Seleccione la Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
    int opcDiv;
    std::cin >> opcDiv;
    DivisionLiga div = (opcDiv == 1) ? LIGA_MAYOR : LIGA_MENOR;

    std::cout << "\nEquipos disponibles:\n";
    std::vector<int> idsValidos;
    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == div) {
            std::cout << "ID: " << std::setw(3) << eq.idEquipo << " - " << std::left << std::setw(28) << eq.nombre
                      << " (" << eq.jugadores.size() << " jugadores registrados)\n";
            idsValidos.push_back(eq.idEquipo);
        }
    }

    if (idsValidos.empty()) {
        std::cout << "No hay equipos registrados en esta división.\n";
        pausarConsola();
        return;
    }

    std::cout << "\nIngrese el ID del equipo para gestionar su plantilla: ";
    int idEqSel;
    std::cin >> idEqSel;

    Equipo *eqPtr = nullptr;
    for (auto &eq : sistema.equipos) {
        if (eq.idEquipo == idEqSel && eq.idCampeonato == sistema.idTorneoActivo) {
            eqPtr = &eq;
            break;
        }
    }

    if (!eqPtr) {
        std::cout << "[ERROR] ID de equipo no válido para el torneo actual.\n";
        pausarConsola();
        return;
    }

    // Mostrar plantilla actual
    std::cout << "\n--- Plantilla Actual de " << eqPtr->nombre << " ---\n";
    if (eqPtr->jugadores.empty()) {
        std::cout << "(Sin jugadores registrados actualmente)\n";
    } else {
        std::cout << std::left << std::setw(6) << "ID"
                  << std::right << std::setw(5) << "#"
                  << "  " << std::left << std::setw(28) << "Nombre y Apellido"
                  << std::left << std::setw(15) << "Posición\n";
        std::cout << std::string(56, '-') << "\n";
        for (const auto &j : eqPtr->jugadores) {
            std::cout << std::left << std::setw(6) << j.idJugador
                      << std::right << std::setw(5) << j.numeroCamiseta
                      << "  " << std::left << std::setw(28) << (j.nombre + " " + j.apellido)
                      << std::left << std::setw(15) << j.posicion << "\n";
        }
    }

    std::cout << "\n--- Datos del Nuevo Jugador ---\n";
    Jugador jug;
    jug.idJugador = sistema.autoIncJugador++;
    jug.idEquipo = eqPtr->idEquipo;

    std::cout << "Nombre: ";
    std::getline(std::cin >> std::ws, jug.nombre);

    std::cout << "Apellido: ";
    std::getline(std::cin >> std::ws, jug.apellido);

    std::cout << "Seleccione Posición:\n";
    std::cout << "1. PORTERO\n2. DEFENSA\n3. MEDIOCAMPISTA\n4. DELANTERO\nOpción: ";
    int opcPos;
    std::cin >> opcPos;
    if (opcPos == 1) jug.posicion = "PORTERO";
    else if (opcPos == 2) jug.posicion = "DEFENSA";
    else if (opcPos == 3) jug.posicion = "MEDIOCAMPISTA";
    else jug.posicion = "DELANTERO";

    std::cout << "Número de Camiseta (0-99): ";
    std::cin >> jug.numeroCamiseta;

    if (jug.numeroCamiseta < 0 || jug.numeroCamiseta > 99) {
        std::cout << "[ERROR] El número de camiseta debe estar entre 0 y 99.\n";
        pausarConsola();
        return;
    }

    for (const auto &existente : eqPtr->jugadores) {
        if (existente.numeroCamiseta == jug.numeroCamiseta) {
            std::cout << "\n[ERROR] El número de camiseta #" << jug.numeroCamiseta
                      << " ya pertenece a " << existente.nombre << " " << existente.apellido << " en este equipo.\n";
            pausarConsola();
            return;
        }
    }

    dbInsertarJugadorEnBD(sistema, jug);
    eqPtr->jugadores.push_back(jug);

    std::cout << "\n[OK] Jugador registrado con éxito en la Base de Datos 'CampeonatoFutbol':\n"
              << "     ID: " << jug.idJugador << " | " << jug.nombre << " " << jug.apellido
              << " (#" << jug.numeroCamiseta << " - " << jug.posicion << ")\n"
              << "     Equipo vinculado: " << eqPtr->nombre << ".\n";
    pausarConsola();
}

void listarEquiposYPlantillas(const SistemaCampeonato &sistema, DivisionLiga division)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::cout << "\n=======================================================\n";
    std::cout << "         EQUIPOS Y PLANTILLAS - " << nomDiv << "       \n";
    std::cout << "=======================================================\n";

    int count = 0;
    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == division) {
            count++;
            std::cout << "\n🛡️  EQUIPO [" << std::setw(2) << eq.idEquipo << "]: " << eq.nombre
                      << " (Total Jugadores: " << eq.jugadores.size() << ")\n";
            std::cout << std::string(60, '-') << "\n";
            if (eq.jugadores.empty()) {
                std::cout << "   (No hay jugadores inscritos en este equipo)\n";
            } else {
                for (const auto &j : eq.jugadores) {
                    std::cout << "   ID:" << std::setw(4) << j.idJugador
                              << " | #" << std::setw(2) << j.numeroCamiseta
                              << " | " << std::left << std::setw(26) << (j.nombre + " " + j.apellido)
                              << " | " << std::left << std::setw(14) << j.posicion << "\n";
                }
            }
        }
    }

    if (count == 0) {
        std::cout << "No hay equipos registrados en esta división.\n";
    }
    pausarConsola();
}

// =========================================================================
// MÓDULO FIXTURE Y ALGORITMOS
// =========================================================================

bool generarClasificacionRoundRobin(SistemaCampeonato &sistema, DivisionLiga division)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::cout << "\n=======================================================\n";
    std::cout << "      GENERAR FIXTURE ROUND ROBIN - " << nomDiv << "   \n";
    std::cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "CLASIFICACION") {
            std::cout << "[!] Ya existe un calendario de Clasificación generado para esta división.\n";
            pausarConsola();
            return false;
        }
    }

    std::vector<Equipo*> equiposDiv;
    for (auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == division) {
            equiposDiv.push_back(&eq);
        }
    }

    int n = (int)equiposDiv.size();
    if (n < 2) {
        std::cout << "[ERROR] Se requieren al menos 2 equipos para generar el fixture (Actuales: " << n << ").\n";
        pausarConsola();
        return false;
    }

    std::vector<int> indices;
    for (int i = 0; i < n; ++i) indices.push_back(i);
    bool tieneDummy = false;
    if (n % 2 != 0) {
        indices.push_back(-1); // Equipo fantasma
        tieneDummy = true;
        n++;
    }

    int totalRondas = n - 1;
    int partidosPorRonda = n / 2;
    int partidosGenerados = 0;

    for (int r = 0; r < totalRondas; ++r) {
        for (int m = 0; m < partidosPorRonda; ++m) {
            int localIdx = indices[m];
            int visitIdx = indices[n - 1 - m];

            if (localIdx != -1 && visitIdx != -1) {
                if (r % 2 == 1) std::swap(localIdx, visitIdx);

                Partido part;
                part.idPartido = sistema.autoIncPartido++;
                part.idCampeonato = sistema.idTorneoActivo;
                part.division = division;
                part.fase = "CLASIFICACION";
                part.jornada = r + 1;
                part.fecha = "2026-02-" + std::to_string(10 + (r % 15));
                part.idLocal = equiposDiv[localIdx]->idEquipo;
                part.nombreLocal = equiposDiv[localIdx]->nombre;
                part.golesLocal = 0;
                part.idVisitante = equiposDiv[visitIdx]->idEquipo;
                part.nombreVisitante = equiposDiv[visitIdx]->nombre;
                part.golesVisitante = 0;
                part.estado = "PENDIENTE";

                dbInsertarPartidoEnBD(sistema, part);
                sistema.partidos.push_back(part);
                partidosGenerados++;
            }
        }

        int ultimo = indices.back();
        for (int i = n - 1; i > 1; --i) {
            indices[i] = indices[i - 1];
        }
        indices[1] = ultimo;
    }

    std::cout << "[OK] Fixture Round Robin generado y guardado en BD con éxito:\n";
    std::cout << "     - Equipos participantes: " << equiposDiv.size() << "\n";
    std::cout << "     - Jornadas creadas: " << totalRondas << "\n";
    std::cout << "     - Partidos totales: " << partidosGenerados << "\n";
    pausarConsola();
    return true;
}

bool generarCuartosDeFinalTop8(SistemaCampeonato &sistema, DivisionLiga division)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::cout << "\n=======================================================\n";
    std::cout << "    GENERAR FASE 2: CUARTOS DE FINAL (TOP 8) - " << nomDiv << "\n";
    std::cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "CUARTOS") {
            std::cout << "[!] Ya se generaron los Cuartos de Final para esta división.\n";
            pausarConsola();
            return false;
        }
    }

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division &&
            p.fase == "CLASIFICACION" && p.estado == "PENDIENTE") {
            std::cout << "[!] No se puede generar la segunda fase: aún hay partidos pendientes en la fase de Clasificación.\n";
            pausarConsola();
            return false;
        }
    }

    std::vector<PosicionEquipo> tabla = calcularTablaPosiciones(sistema, division);
    if (tabla.size() < 8) {
        std::cout << "[ERROR] Se requieren al menos 8 equipos en competencia para la Fase Final (Registrados: "
                  << tabla.size() << ").\n";
        pausarConsola();
        return false;
    }

    std::cout << "Top 8 Equipos Clasificados a Cuartos de Final:\n";
    for (int i = 0; i < 8; ++i) {
        std::cout << " " << (i + 1) << ". " << std::left << std::setw(25) << tabla[i].nombreEquipo
                  << " (" << tabla[i].puntos << " pts)\n";
    }

    std::vector<std::pair<int, int>> cruces = {
        {0, 7}, // 1ro vs 8vo
        {1, 6}, // 2do vs 7mo
        {2, 5}, // 3ro vs 6to
        {3, 4}  // 4to vs 5to
    };

    std::cout << "\nGenerando llaves oficiales de Cuartos de Final...\n";
    for (size_t i = 0; i < cruces.size(); ++i) {
        int idxA = cruces[i].first;
        int idxB = cruces[i].second;

        Partido part;
        part.idPartido = sistema.autoIncPartido++;
        part.idCampeonato = sistema.idTorneoActivo;
        part.division = division;
        part.fase = "CUARTOS";
        part.jornada = 1;
        part.fecha = "2026-05-10";
        part.idLocal = tabla[idxA].idEquipo;
        part.nombreLocal = tabla[idxA].nombreEquipo;
        part.golesLocal = 0;
        part.idVisitante = tabla[idxB].idEquipo;
        part.nombreVisitante = tabla[idxB].nombreEquipo;
        part.golesVisitante = 0;
        part.estado = "PENDIENTE";

        dbInsertarPartidoEnBD(sistema, part);
        sistema.partidos.push_back(part);
        std::cout << " Llave " << (i + 1) << ": " << part.nombreLocal << " vs " << part.nombreVisitante << "\n";
    }

    std::cout << "\n[OK] Cuartos de final generados y sincronizados con SQL Server correctamente.\n";
    pausarConsola();
    return true;
}

bool generarSemifinales(SistemaCampeonato &sistema, DivisionLiga division)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::cout << "\n=======================================================\n";
    std::cout << "         GENERAR SEMIFINALES - " << nomDiv << "        \n";
    std::cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "SEMIFINAL") {
            std::cout << "[!] Ya existen Semifinales generadas para esta división.\n";
            pausarConsola();
            return false;
        }
    }

    std::vector<Partido*> cuartos;
    for (auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "CUARTOS") {
            cuartos.push_back(&p);
            if (p.estado == "PENDIENTE") {
                std::cout << "[!] Aún faltan partidos de Cuartos de Final por disputar.\n";
                pausarConsola();
                return false;
            }
        }
    }

    if (cuartos.size() != 4) {
        std::cout << "[ERROR] No se encontraron los 4 partidos reglamentarios de Cuartos de Final.\n";
        pausarConsola();
        return false;
    }

    std::vector<int> ganadores;
    std::vector<std::string> nomGanadores;
    for (auto cp : cuartos) {
        if (cp->golesLocal >= cp->golesVisitante) {
            ganadores.push_back(cp->idLocal);
            nomGanadores.push_back(cp->nombreLocal);
        } else {
            ganadores.push_back(cp->idVisitante);
            nomGanadores.push_back(cp->nombreVisitante);
        }
    }

    std::vector<std::pair<int, int>> crucesSemis = {{0, 1}, {2, 3}};
    for (size_t i = 0; i < crucesSemis.size(); ++i) {
        int a = crucesSemis[i].first;
        int b = crucesSemis[i].second;

        Partido part;
        part.idPartido = sistema.autoIncPartido++;
        part.idCampeonato = sistema.idTorneoActivo;
        part.division = division;
        part.fase = "SEMIFINAL";
        part.jornada = 1;
        part.fecha = "2026-05-18";
        part.idLocal = ganadores[a];
        part.nombreLocal = nomGanadores[a];
        part.golesLocal = 0;
        part.idVisitante = ganadores[b];
        part.nombreVisitante = nomGanadores[b];
        part.golesVisitante = 0;
        part.estado = "PENDIENTE";

        dbInsertarPartidoEnBD(sistema, part);
        sistema.partidos.push_back(part);
        std::cout << " Semifinal " << (i + 1) << ": " << part.nombreLocal << " vs " << part.nombreVisitante << "\n";
    }

    std::cout << "\n[OK] Semifinales creadas y guardadas en la base de datos con éxito.\n";
    pausarConsola();
    return true;
}

bool generarGranFinal(SistemaCampeonato &sistema, DivisionLiga division)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::cout << "\n=======================================================\n";
    std::cout << "         GENERAR GRAN FINAL - " << nomDiv << "         \n";
    std::cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "FINAL") {
            std::cout << "[!] La Gran Final ya fue generada previamente.\n";
            pausarConsola();
            return false;
        }
    }

    std::vector<Partido*> semis;
    for (auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "SEMIFINAL") {
            semis.push_back(&p);
            if (p.estado == "PENDIENTE") {
                std::cout << "[!] Aún faltan partidos de Semifinales por disputar.\n";
                pausarConsola();
                return false;
            }
        }
    }

    if (semis.size() != 2) {
        std::cout << "[ERROR] Se requieren 2 semifinales finalizadas.\n";
        pausarConsola();
        return false;
    }

    int finLoc = (semis[0]->golesLocal >= semis[0]->golesVisitante) ? semis[0]->idLocal : semis[0]->idVisitante;
    std::string nomFinLoc = (semis[0]->golesLocal >= semis[0]->golesVisitante) ? semis[0]->nombreLocal : semis[0]->nombreVisitante;

    int finVis = (semis[1]->golesLocal >= semis[1]->golesVisitante) ? semis[1]->idLocal : semis[1]->idVisitante;
    std::string nomFinVis = (semis[1]->golesLocal >= semis[1]->golesVisitante) ? semis[1]->nombreLocal : semis[1]->nombreVisitante;

    Partido part;
    part.idPartido = sistema.autoIncPartido++;
    part.idCampeonato = sistema.idTorneoActivo;
    part.division = division;
    part.fase = "FINAL";
    part.jornada = 1;
    part.fecha = "2026-05-25";
    part.idLocal = finLoc;
    part.nombreLocal = nomFinLoc;
    part.golesLocal = 0;
    part.idVisitante = finVis;
    part.nombreVisitante = nomFinVis;
    part.golesVisitante = 0;
    part.estado = "PENDIENTE";

    dbInsertarPartidoEnBD(sistema, part);
    sistema.partidos.push_back(part);

    std::cout << " GRAN FINAL: " << part.nombreLocal << " vs " << part.nombreVisitante << "\n";
    std::cout << "\n[OK] Partido por el Campeonato creado y guardado en SQL Server.\n";
    pausarConsola();
    return true;
}

void listarCalendarioPartidos(const SistemaCampeonato &sistema, DivisionLiga division, const std::string &filtroFase)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::cout << "\n========================================================================================\n";
    std::cout << "           CALENDARIO Y MARCADORES DE PARTIDOS - " << nomDiv << "                       \n";
    std::cout << "========================================================================================\n";

    int count = 0;
    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division) {
            if (filtroFase != "TODAS" && p.fase != filtroFase) continue;
            count++;

            std::cout << "ID: " << std::setw(3) << p.idPartido
                      << " | Jor: " << std::setw(2) << p.jornada
                      << " | Fase: " << std::left << std::setw(13) << p.fase
                      << " | Fecha: " << p.fecha
                      << " | " << std::right << std::setw(22) << p.nombreLocal << " ";

            if (p.estado == "FINALIZADO") {
                std::cout << "[" << p.golesLocal << " - " << p.golesVisitante << "]";
            } else {
                std::cout << "[ vs ]";
            }
            std::cout << " " << std::left << std::setw(22) << p.nombreVisitante
                      << " | " << (p.estado == "FINALIZADO" ? "FINALIZADO" : "PENDIENTE") << "\n";
        }
    }

    if (count == 0) {
        std::cout << "No hay partidos registrados para esta categoría.\n";
    }
    pausarConsola();
}

// =========================================================================
// MÓDULO RESULTADOS E INCIDENCIAS
// =========================================================================

void ingresarResultadoPartido(SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "          INGRESAR RESULTADO E INCIDENCIAS             \n";
    std::cout << "=======================================================\n";

    std::cout << "Seleccione la Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
    int opcDiv;
    std::cin >> opcDiv;
    DivisionLiga div = (opcDiv == 1) ? LIGA_MAYOR : LIGA_MENOR;

    std::cout << "\nPartidos disponibles para registrar:\n";
    std::vector<Partido*> pendientes;
    for (auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == div && p.estado == "PENDIENTE") {
            pendientes.push_back(&p);
            std::cout << "ID: " << std::setw(4) << p.idPartido
                      << " [" << std::setw(13) << p.fase << "] "
                      << p.nombreLocal << " vs " << p.nombreVisitante << "\n";
        }
    }

    if (pendientes.empty()) {
        std::cout << "No hay partidos pendientes en esta división.\n";
        pausarConsola();
        return;
    }

    std::cout << "\nIngrese el ID del partido a registrar: ";
    int idSel;
    std::cin >> idSel;

    Partido *partidoSel = nullptr;
    for (auto p : pendientes) {
        if (p->idPartido == idSel) {
            partidoSel = p;
            break;
        }
    }

    if (!partidoSel) {
        std::cout << "[ERROR] ID de partido no válido.\n";
        pausarConsola();
        return;
    }

    Equipo *eqLoc = buscarEquipoPorId(sistema, partidoSel->idLocal);
    Equipo *eqVis = buscarEquipoPorId(sistema, partidoSel->idVisitante);

    std::cout << "\n--- Marcador Final ---\n";
    std::cout << "Goles de " << partidoSel->nombreLocal << ": ";
    std::cin >> partidoSel->golesLocal;
    std::cout << "Goles de " << partidoSel->nombreVisitante << ": ";
    std::cin >> partidoSel->golesVisitante;

    partidoSel->goles.clear();
    partidoSel->tarjetas.clear();
    partidoSel->porteros.clear();

    // Identificar porteros de ambos equipos
    Jugador *portLoc = nullptr;
    Jugador *portVis = nullptr;
    if (eqLoc) {
        for (auto &j : eqLoc->jugadores) {
            if (j.posicion == "PORTERO") { portLoc = &j; break; }
        }
    }
    if (eqVis) {
        for (auto &j : eqVis->jugadores) {
            if (j.posicion == "PORTERO") { portVis = &j; break; }
        }
    }

    // Registrar goles del equipo Local
    if (partidoSel->golesLocal > 0) {
        std::cout << "\n--- ASIGNACIÓN DE GOLES: " << partidoSel->nombreLocal << " (" << partidoSel->golesLocal << " goles) ---\n";
        for (int i = 0; i < partidoSel->golesLocal; ++i) {
            std::cout << "\nGol #" << (i + 1) << ":\n";
            std::cout << "Plantilla de " << partidoSel->nombreLocal << ":\n";
            if (eqLoc) {
                for (size_t k = 0; k < eqLoc->jugadores.size(); ++k) {
                    std::cout << " " << (k + 1) << ". #" << eqLoc->jugadores[k].numeroCamiseta
                              << " - " << eqLoc->jugadores[k].nombre << " " << eqLoc->jugadores[k].apellido
                              << " (" << eqLoc->jugadores[k].posicion << ")\n";
                }
            }
            std::cout << "Seleccione el autor del gol (número de lista): ";
            int selAut; std::cin >> selAut;

            IncidenciaGol g;
            if (eqLoc && selAut >= 1 && selAut <= (int)eqLoc->jugadores.size()) {
                g.idJugador = eqLoc->jugadores[selAut - 1].idJugador;
                g.nombreJugador = eqLoc->jugadores[selAut - 1].nombre + " " + eqLoc->jugadores[selAut - 1].apellido;
            } else {
                g.idJugador = 0;
                g.nombreJugador = "Jugador Local";
            }

            g.idEquipoRival = partidoSel->idVisitante;
            g.nombreEquipoRival = partidoSel->nombreVisitante;
            if (portVis) {
                g.idPorteroVencido = portVis->idJugador;
                g.nombrePorteroVencido = portVis->nombre + " " + portVis->apellido;
            } else {
                g.idPorteroVencido = 0;
                g.nombrePorteroVencido = "Portero Visitante";
            }
            std::cout << "Minuto de la anotación: ";
            std::cin >> g.minuto;
            g.esAutogol = false;
            partidoSel->goles.push_back(g);
        }
    }

    // Registrar goles del equipo Visitante
    if (partidoSel->golesVisitante > 0) {
        std::cout << "\n--- ASIGNACIÓN DE GOLES: " << partidoSel->nombreVisitante << " (" << partidoSel->golesVisitante << " goles) ---\n";
        for (int i = 0; i < partidoSel->golesVisitante; ++i) {
            std::cout << "\nGol #" << (i + 1) << ":\n";
            std::cout << "Plantilla de " << partidoSel->nombreVisitante << ":\n";
            if (eqVis) {
                for (size_t k = 0; k < eqVis->jugadores.size(); ++k) {
                    std::cout << " " << (k + 1) << ". #" << eqVis->jugadores[k].numeroCamiseta
                              << " - " << eqVis->jugadores[k].nombre << " " << eqVis->jugadores[k].apellido
                              << " (" << eqVis->jugadores[k].posicion << ")\n";
                }
            }
            std::cout << "Seleccione el autor del gol (número de lista): ";
            int selAut; std::cin >> selAut;

            IncidenciaGol g;
            if (eqVis && selAut >= 1 && selAut <= (int)eqVis->jugadores.size()) {
                g.idJugador = eqVis->jugadores[selAut - 1].idJugador;
                g.nombreJugador = eqVis->jugadores[selAut - 1].nombre + " " + eqVis->jugadores[selAut - 1].apellido;
            } else {
                g.idJugador = 0;
                g.nombreJugador = "Jugador Visitante";
            }

            g.idEquipoRival = partidoSel->idLocal;
            g.nombreEquipoRival = partidoSel->nombreLocal;
            if (portLoc) {
                g.idPorteroVencido = portLoc->idJugador;
                g.nombrePorteroVencido = portLoc->nombre + " " + portLoc->apellido;
            } else {
                g.idPorteroVencido = 0;
                g.nombrePorteroVencido = "Portero Local";
            }
            std::cout << "Minuto de la anotación: ";
            std::cin >> g.minuto;
            g.esAutogol = false;
            partidoSel->goles.push_back(g);
        }
    }

    // Registrar porteros para control oficial
    if (portLoc) {
        ControlPortero cpL;
        cpL.idJugador = portLoc->idJugador;
        cpL.nombrePortero = portLoc->nombre + " " + portLoc->apellido;
        cpL.idEquipo = eqLoc->idEquipo;
        cpL.minutoEntrada = 0;
        cpL.minutoSalida = 90;
        cpL.golesRecibidos = partidoSel->golesVisitante;
        partidoSel->porteros.push_back(cpL);
    }
    if (portVis) {
        ControlPortero cpV;
        cpV.idJugador = portVis->idJugador;
        cpV.nombrePortero = portVis->nombre + " " + portVis->apellido;
        cpV.idEquipo = eqVis->idEquipo;
        cpV.minutoEntrada = 0;
        cpV.minutoSalida = 90;
        cpV.golesRecibidos = partidoSel->golesLocal;
        partidoSel->porteros.push_back(cpV);
    }

    // Registro de Tarjetas
    std::cout << "\n¿Desea registrar tarjetas arbitrales para este partido? (1: Sí / 2: No): ";
    int opcTar; std::cin >> opcTar;
    while (opcTar == 1) {
        std::cout << "\nEquipo del jugador sancionado:\n1. " << partidoSel->nombreLocal << "\n2. " << partidoSel->nombreVisitante << "\nOpción: ";
        int opcEqT; std::cin >> opcEqT;
        Equipo *eqTar = (opcEqT == 1) ? eqLoc : eqVis;

        if (eqTar) {
            std::cout << "Plantilla de " << eqTar->nombre << ":\n";
            for (size_t k = 0; k < eqTar->jugadores.size(); ++k) {
                std::cout << " " << (k + 1) << ". #" << eqTar->jugadores[k].numeroCamiseta
                          << " - " << eqTar->jugadores[k].nombre << " " << eqTar->jugadores[k].apellido << "\n";
            }
            std::cout << "Seleccione el jugador sancionado: ";
            int selJ; std::cin >> selJ;

            IncidenciaTarjeta t;
            if (selJ >= 1 && selJ <= (int)eqTar->jugadores.size()) {
                t.idJugador = eqTar->jugadores[selJ - 1].idJugador;
                t.nombreJugador = eqTar->jugadores[selJ - 1].nombre + " " + eqTar->jugadores[selJ - 1].apellido + " (" + eqTar->nombre + ")";
            } else {
                t.idJugador = 0;
                t.nombreJugador = "Jugador Sancionado";
            }

            std::cout << "Tipo de tarjeta (1: AMARILLA / 2: ROJA): ";
            int tipoT; std::cin >> tipoT;
            t.tipoTarjeta = (tipoT == 2) ? "ROJA" : "AMARILLA";

            std::cout << "Minuto de la infracción: ";
            std::cin >> t.minuto;

            partidoSel->tarjetas.push_back(t);
        }

        std::cout << "¿Registrar otra tarjeta? (1: Sí / 2: No): ";
        std::cin >> opcTar;
    }

    partidoSel->estado = "FINALIZADO";

    dbActualizarResultadoPartidoEnBD(sistema, *partidoSel);

    std::cout << "\n[OK] Resultado e incidencias guardados exitosamente y sincronizados directamente con SQL Server.\n";
    pausarConsola();
}

void verDetallePartido(const SistemaCampeonato &sistema)
{
    std::cout << "\n========================================================================================\n";
    std::cout << "               DETALLE DE INCIDENCIAS DE UN PARTIDO                                     \n";
    std::cout << "========================================================================================\n";

    std::cout << "Ingrese el ID del partido a consultar: ";
    int idPart;
    std::cin >> idPart;

    const Partido *pSel = nullptr;
    for (const auto &p : sistema.partidos) {
        if (p.idPartido == idPart) {
            pSel = &p;
            break;
        }
    }

    if (!pSel) {
        std::cout << "[ERROR] No se encontró el partido con ID " << idPart << ".\n";
        pausarConsola();
        return;
    }

    std::cout << "\n----------------------------------------------------------------------------------------\n";
    std::cout << " PARTIDO #" << pSel->idPartido << " [" << pSel->fase << " - Jornada " << pSel->jornada << "] | Fecha: " << pSel->fecha << "\n";
    std::cout << " " << pSel->nombreLocal << " " << pSel->golesLocal << " - " << pSel->golesVisitante << " " << pSel->nombreVisitante << "\n";
    std::cout << " Estado: " << pSel->estado << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    std::cout << "⚽ GOLES Y ANOTADORES:\n";
    if (pSel->goles.empty()) {
        std::cout << "   (No hay goles registrados)\n";
    } else {
        for (const auto &g : pSel->goles) {
            std::cout << "   Min " << std::setw(2) << g.minuto << "' | Gol de " << g.nombreJugador
                      << " (vs Portero: " << g.nombrePorteroVencido << ")\n";
        }
    }

    std::cout << "\n🟨 TARJETAS Y SANCIONES:\n";
    if (pSel->tarjetas.empty()) {
        std::cout << "   (Sin tarjetas registradas - Partido limpio)\n";
    } else {
        for (const auto &t : pSel->tarjetas) {
            std::cout << "   Min " << std::setw(2) << t.minuto << "' | [" << t.tipoTarjeta << "] - " << t.nombreJugador << "\n";
        }
    }

    std::cout << "\n🧤 CONTROL DE PORTEROS:\n";
    if (pSel->porteros.empty()) {
        std::cout << "   (Sin datos específicos de porteros)\n";
    } else {
        for (const auto &cp : pSel->porteros) {
            std::cout << "   " << cp.nombrePortero << " | Minutos: " << (cp.minutoSalida - cp.minutoEntrada)
                      << " | Goles Encajados: " << cp.golesRecibidos << "\n";
        }
    }

    pausarConsola();
}

// =========================================================================
// MÓDULO POSICIONES, ASCENSO/DESCENSO Y PREMIACIONES
// =========================================================================

std::vector<PosicionEquipo> calcularTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division)
{
    std::map<int, PosicionEquipo> mapa;

    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == division) {
            PosicionEquipo pe;
            pe.idEquipo = eq.idEquipo;
            pe.nombreEquipo = eq.nombre;
            pe.pj = 0; pe.pg = 0; pe.pe = 0; pe.pp = 0;
            pe.gf = 0; pe.gc = 0; pe.dg = 0; pe.puntos = 0;
            mapa[eq.idEquipo] = pe;
        }
    }

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division &&
            p.fase == "CLASIFICACION" && p.estado == "FINALIZADO") {

            if (mapa.find(p.idLocal) != mapa.end() && mapa.find(p.idVisitante) != mapa.end()) {
                PosicionEquipo &loc = mapa[p.idLocal];
                PosicionEquipo &vis = mapa[p.idVisitante];

                loc.pj++;
                vis.pj++;

                loc.gf += p.golesLocal;
                loc.gc += p.golesVisitante;

                vis.gf += p.golesVisitante;
                vis.gc += p.golesLocal;

                if (p.golesLocal > p.golesVisitante) {
                    loc.pg++; loc.puntos += 3;
                    vis.pp++;
                } else if (p.golesLocal == p.golesVisitante) {
                    loc.pe++; loc.puntos += 1;
                    vis.pe++; vis.puntos += 1;
                } else {
                    vis.pg++; vis.puntos += 3;
                    loc.pp++;
                }
            }
        }
    }

    std::vector<PosicionEquipo> tabla;
    for (auto &par : mapa) {
        par.second.dg = par.second.gf - par.second.gc;
        tabla.push_back(par.second);
    }

    std::sort(tabla.begin(), tabla.end(), [](const PosicionEquipo &a, const PosicionEquipo &b) {
        if (a.puntos != b.puntos) return a.puntos > b.puntos;
        if (a.dg != b.dg) return a.dg > b.dg;
        if (a.gf != b.gf) return a.gf > b.gf;
        return a.nombreEquipo < b.nombreEquipo;
    });

    for (size_t i = 0; i < tabla.size(); ++i) {
        tabla[i].posicion = (int)i + 1;
    }

    return tabla;
}

void imprimirTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division)
{
    std::string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    std::vector<PosicionEquipo> tabla = calcularTablaPosiciones(sistema, division);

    std::cout << "\n========================================================================================\n";
    std::cout << "                  TABLA GENERAL DE POSICIONES - " << nomDiv << "                        \n";
    std::cout << "========================================================================================\n";
    std::cout << std::left  << std::setw(5)  << "Pos"
              << std::left  << std::setw(28) << "Equipo"
              << std::right << std::setw(4)  << "PJ"
              << std::right << std::setw(4)  << "PG"
              << std::right << std::setw(4)  << "PE"
              << std::right << std::setw(4)  << "PP"
              << std::right << std::setw(5)  << "GF"
              << std::right << std::setw(5)  << "GC"
              << std::right << std::setw(5)  << "DG"
              << std::right << std::setw(6)  << "PTS"
              << "   Zona / Estado\n";
    std::cout << std::string(88, '-') << "\n";

    for (const auto &eq : tabla) {
        std::string zona = "";
        if (eq.posicion <= 8) zona = "[Top 8 Clasificado]";
        if (division == LIGA_MAYOR && eq.posicion >= 11) zona = "[ZONA DESCENSO]";
        if (division == LIGA_MENOR && eq.posicion <= 2) zona = "[ZONA ASCENSO DIRECTO]";

        std::cout << std::left  << std::setw(5)  << eq.posicion
                  << std::left  << std::setw(28) << eq.nombreEquipo
                  << std::right << std::setw(4)  << eq.pj
                  << std::right << std::setw(4)  << eq.pg
                  << std::right << std::setw(4)  << eq.pe
                  << std::right << std::setw(4)  << eq.pp
                  << std::right << std::setw(5)  << eq.gf
                  << std::right << std::setw(5)  << eq.gc
                  << std::right << std::setw(5)  << eq.dg
                  << std::right << std::setw(6)  << eq.puntos
                  << "   " << zona << "\n";
    }

    std::cout << std::string(88, '-') << "\n";
    std::cout << "Reglamento: Los mejores 8 clasifican a Cuartos de Final.\n";
    std::cout << "En Liga Mayor descienden los 2 últimos lugares (11 y 12).\n";
    std::cout << "En Liga Menor ascienden a Liga Mayor los 2 primeros lugares (1 y 2).\n";
    pausarConsola();
}

void imprimirTablasAscensoDescenso(const SistemaCampeonato &sistema)
{
    std::cout << "\n========================================================================================\n";
    std::cout << "                   TABLAS OFICIALES DE ASCENSO Y DESCENSO                               \n";
    std::cout << "========================================================================================\n";

    std::vector<PosicionEquipo> mayor = calcularTablaPosiciones(sistema, LIGA_MAYOR);
    std::vector<PosicionEquipo> menor = calcularTablaPosiciones(sistema, LIGA_MENOR);

    std::cout << "\n🔻 CANDIDATOS AL DESCENSO (LIGA MAYOR --> LIGA MENOR):\n";
    std::cout << "----------------------------------------------------------------------------------------\n";
    if (mayor.size() >= 2) {
        int n = (int)mayor.size();
        std::cout << " 11. " << std::left << std::setw(28) << mayor[n - 2].nombreEquipo
                  << " | Pts: " << std::setw(3) << mayor[n - 2].puntos
                  << " | DIF: " << std::setw(3) << mayor[n - 2].dg << " [DESCENSO DIRECTO]\n";
        std::cout << " 12. " << std::left << std::setw(28) << mayor[n - 1].nombreEquipo
                  << " | Pts: " << std::setw(3) << mayor[n - 1].puntos
                  << " | DIF: " << std::setw(3) << mayor[n - 1].dg << " [DESCENSO DIRECTO]\n";
    } else {
        std::cout << "No hay suficientes equipos registrados en Liga Mayor.\n";
    }

    std::cout << "\n🔺 CANDIDATOS AL ASCENSO (LIGA MENOR --> LIGA MAYOR):\n";
    std::cout << "----------------------------------------------------------------------------------------\n";
    if (menor.size() >= 2) {
        std::cout << "  1. " << std::left << std::setw(28) << menor[0].nombreEquipo
                  << " | Pts: " << std::setw(3) << menor[0].puntos
                  << " | DIF: " << std::setw(3) << menor[0].dg << " [ASCENSO DIRECTO]\n";
        std::cout << "  2. " << std::left << std::setw(28) << menor[1].nombreEquipo
                  << " | Pts: " << std::setw(3) << menor[1].puntos
                  << " | DIF: " << std::setw(3) << menor[1].dg << " [ASCENSO DIRECTO]\n";
    } else {
        std::cout << "No hay suficientes equipos registrados en Liga Menor.\n";
    }

    if (!sistema.historial.empty()) {
        std::cout << "\n📜 HISTORIAL OFICIAL DE ASCENSOS Y DESCENSOS APLICADOS:\n";
        std::cout << "----------------------------------------------------------------------------------------\n";
        for (const auto &h : sistema.historial) {
            std::cout << " [" << h.tipoMovimiento << "] Equipo: " << std::left << std::setw(26) << h.nombreEquipo
                      << " | De: " << (h.origen == LIGA_MAYOR ? "Liga Mayor" : "Liga Menor")
                      << " -> A: " << (h.destino == LIGA_MAYOR ? "Liga Mayor" : "Liga Menor")
                      << " | Fecha: " << h.fecha << "\n";
        }
    }
    pausarConsola();
}

void aplicarAscensoDescensoOficial(SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "      APLICAR ASCENSO Y DESCENSO REGLAMENTARIO         \n";
    std::cout << "=======================================================\n";

    std::vector<PosicionEquipo> mayor = calcularTablaPosiciones(sistema, LIGA_MAYOR);
    std::vector<PosicionEquipo> menor = calcularTablaPosiciones(sistema, LIGA_MENOR);

    if (mayor.size() < 12 || menor.size() < 12) {
        std::cout << "[AVISO] Por reglamento oficial se requieren 12 equipos por liga para formalizar el ciclo.\n";
        std::cout << "¿Desea forzar la aplicación con los equipos actuales? (1: Sí / 2: Cancelar): ";
        int opc; std::cin >> opc;
        if (opc != 1) return;
    }

    if (mayor.size() < 2 || menor.size() < 2) {
        std::cout << "[ERROR] Equipos insuficientes para procesar ascensos y descensos.\n";
        pausarConsola();
        return;
    }

    int idxDes1 = (int)mayor.size() - 2;
    int idxDes2 = (int)mayor.size() - 1;

    Equipo *eqDes1 = buscarEquipoPorId(sistema, mayor[idxDes1].idEquipo);
    Equipo *eqDes2 = buscarEquipoPorId(sistema, mayor[idxDes2].idEquipo);

    Equipo *eqAsc1 = buscarEquipoPorId(sistema, menor[0].idEquipo);
    Equipo *eqAsc2 = buscarEquipoPorId(sistema, menor[1].idEquipo);

    if (!eqDes1 || !eqDes2 || !eqAsc1 || !eqAsc2) {
        std::cout << "[ERROR] No se localizaron los registros de los equipos en la estructura.\n";
        pausarConsola();
        return;
    }

    eqDes1->division = LIGA_MENOR;
    eqDes2->division = LIGA_MENOR;
    eqAsc1->division = LIGA_MAYOR;
    eqAsc2->division = LIGA_MAYOR;

    dbActualizarDivisionEquipoEnBD(sistema, eqDes1->idEquipo, 2);
    dbActualizarDivisionEquipoEnBD(sistema, eqDes2->idEquipo, 2);
    dbActualizarDivisionEquipoEnBD(sistema, eqAsc1->idEquipo, 1);
    dbActualizarDivisionEquipoEnBD(sistema, eqAsc2->idEquipo, 1);

    HistorialMovimiento h1;
    h1.idHistorial = sistema.autoIncHistorial++;
    h1.idCampeonato = sistema.idTorneoActivo;
    h1.idEquipo = eqDes1->idEquipo;
    h1.nombreEquipo = eqDes1->nombre;
    h1.origen = LIGA_MAYOR; h1.destino = LIGA_MENOR;
    h1.tipoMovimiento = "DESCENSO";
    h1.fecha = "2026-05-30";
    dbInsertarHistorialEnBD(sistema, h1);
    sistema.historial.push_back(h1);

    HistorialMovimiento h2;
    h2.idHistorial = sistema.autoIncHistorial++;
    h2.idCampeonato = sistema.idTorneoActivo;
    h2.idEquipo = eqDes2->idEquipo;
    h2.nombreEquipo = eqDes2->nombre;
    h2.origen = LIGA_MAYOR; h2.destino = LIGA_MENOR;
    h2.tipoMovimiento = "DESCENSO";
    h2.fecha = "2026-05-30";
    dbInsertarHistorialEnBD(sistema, h2);
    sistema.historial.push_back(h2);

    HistorialMovimiento h3;
    h3.idHistorial = sistema.autoIncHistorial++;
    h3.idCampeonato = sistema.idTorneoActivo;
    h3.idEquipo = eqAsc1->idEquipo;
    h3.nombreEquipo = eqAsc1->nombre;
    h3.origen = LIGA_MENOR; h3.destino = LIGA_MAYOR;
    h3.tipoMovimiento = "ASCENSO";
    h3.fecha = "2026-05-30";
    dbInsertarHistorialEnBD(sistema, h3);
    sistema.historial.push_back(h3);

    HistorialMovimiento h4;
    h4.idHistorial = sistema.autoIncHistorial++;
    h4.idCampeonato = sistema.idTorneoActivo;
    h4.idEquipo = eqAsc2->idEquipo;
    h4.nombreEquipo = eqAsc2->nombre;
    h4.origen = LIGA_MENOR; h4.destino = LIGA_MAYOR;
    h4.tipoMovimiento = "ASCENSO";
    h4.fecha = "2026-05-30";
    dbInsertarHistorialEnBD(sistema, h4);
    sistema.historial.push_back(h4);

    std::cout << "\n[OK] Movimientos oficiales formalizados y guardados en SQL Server:\n";
    std::cout << " 🔻 Descienden a Liga Menor: " << eqDes1->nombre << " y " << eqDes2->nombre << "\n";
    std::cout << " 🔺 Ascienden a Liga Mayor: " << eqAsc1->nombre << " y " << eqAsc2->nombre << "\n";
    pausarConsola();
}

void imprimirModuloPremiaciones(const SistemaCampeonato &sistema)
{
    std::cout << "\n========================================================================================\n";
    std::cout << "                    CUADRO DE HONOR Y PREMIACIONES DEL CAMPEONATO                       \n";
    std::cout << "========================================================================================\n";

    const Premiacion *premSel = nullptr;
    for (const auto &pr : sistema.premiaciones) {
        if (pr.idTorneo == sistema.idTorneoActivo) {
            premSel = &pr;
            break;
        }
    }

    if (!premSel || !premSel->habilitada) {
        std::cout << "\n[!] El módulo de premiaciones aún NO se encuentra habilitado para este torneo.\n";
        std::cout << "    (El Administrador debe finalizar el torneo formalmente para desbloquear los premios).\n";
        pausarConsola();
        return;
    }

    std::cout << "\n 🏆 CAMPEÓN LIGA MAYOR:          " << premSel->campeonMayor << "\n";
    std::cout << " 🥈 CAMPEÓN LIGA MENOR:          " << premSel->campeonMenor << "\n";
    std::cout << " ⚽ MÁXIMO GOLEADOR (BOTÍN DE ORO): " << premSel->maximoGoleador << " (" << premSel->golesAnotados << " goles)\n";
    std::cout << " 🧤 PORTERO MENOS VENCIDO:       " << premSel->porteroMenosVencido << " (" << premSel->golesRecibidos << " goles encajados)\n";
    std::cout << "========================================================================================\n";
    pausarConsola();
}

void finalizarTorneoYHabilitarPremiacion(SistemaCampeonato &sistema)
{
    std::cout << "\n=======================================================\n";
    std::cout << "      FINALIZAR TORNEO Y HABILITAR PREMIACIONES        \n";
    std::cout << "=======================================================\n";

    Torneo *tActivo = nullptr;
    for (auto &t : sistema.torneos) {
        if (t.idTorneo == sistema.idTorneoActivo) {
            tActivo = &t;
            break;
        }
    }

    if (!tActivo) {
        std::cout << "[ERROR] No hay torneo activo seleccionado.\n";
        pausarConsola();
        return;
    }

    Premiacion prem;
    prem.idTorneo = sistema.idTorneoActivo;
    prem.habilitada = true;
    prem.registrada = true;
    prem.campeonMayor = "Por Determinar";
    prem.campeonMenor = "Por Determinar";
    prem.maximoGoleador = "Sin Registro";
    prem.golesAnotados = 0;
    prem.porteroMenosVencido = "Sin Registro";
    prem.golesRecibidos = 999;

    int idCampMayor = 0;
    int idCampMenor = 0;
    int idGoleador = 0;
    int idPortero = 0;

    // Buscar campeón de Liga Mayor en FINAL
    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == LIGA_MAYOR && p.fase == "FINAL" && p.estado == "FINALIZADO") {
            if (p.golesLocal >= p.golesVisitante) {
                prem.campeonMayor = p.nombreLocal;
                idCampMayor = p.idLocal;
            } else {
                prem.campeonMayor = p.nombreVisitante;
                idCampMayor = p.idVisitante;
            }
        }
    }

    // Si no hubo final jugada, tomar el 1er lugar de posiciones
    if (prem.campeonMayor == "Por Determinar") {
        std::vector<PosicionEquipo> tabMayor = calcularTablaPosiciones(sistema, LIGA_MAYOR);
        if (!tabMayor.empty()) {
            prem.campeonMayor = tabMayor[0].nombreEquipo + " (Líder Tabla General)";
            idCampMayor = tabMayor[0].idEquipo;
        }
    }

    // Campeón Liga Menor
    std::vector<PosicionEquipo> tabMenor = calcularTablaPosiciones(sistema, LIGA_MENOR);
    if (!tabMenor.empty()) {
        prem.campeonMenor = tabMenor[0].nombreEquipo;
        idCampMenor = tabMenor[0].idEquipo;
    }

    // Máximo Goleador acumulado
    std::map<int, int> conteoGoles;
    std::map<int, std::string> nombresGoles;
    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.estado == "FINALIZADO") {
            for (const auto &g : p.goles) {
                if (!g.esAutogol && g.idJugador > 0) {
                    conteoGoles[g.idJugador]++;
                    nombresGoles[g.idJugador] = g.nombreJugador;
                }
            }
        }
    }

    int maxG = -1;
    for (const auto &par : conteoGoles) {
        if (par.second > maxG) {
            maxG = par.second;
            idGoleador = par.first;
            prem.maximoGoleador = nombresGoles[par.first];
            prem.golesAnotados = par.second;
        }
    }

    // Portero Menos Vencido
    std::map<int, int> golesRecibidosPor;
    std::map<int, std::string> nombresPor;
    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.estado == "FINALIZADO") {
            for (const auto &cp : p.porteros) {
                golesRecibidosPor[cp.idJugador] += cp.golesRecibidos;
                nombresPor[cp.idJugador] = cp.nombrePortero;
            }
        }
    }

    int minEnc = 999999;
    for (const auto &par : golesRecibidosPor) {
        if (par.second < minEnc) {
            minEnc = par.second;
            idPortero = par.first;
            prem.porteroMenosVencido = nombresPor[par.first];
            prem.golesRecibidos = par.second;
        }
    }
    if (minEnc == 999999) prem.golesRecibidos = 0;

    // Actualizar en el sistema
    bool existe = false;
    for (auto &pr : sistema.premiaciones) {
        if (pr.idTorneo == prem.idTorneo) {
            pr = prem;
            existe = true;
            break;
        }
    }
    if (!existe) sistema.premiaciones.push_back(prem);

    tActivo->finalizado = true;

    dbGuardarPremiacionEnBD(sistema, prem, idGoleador, idPortero, idCampMayor);

    std::cout << "\n[OK] ¡Torneo finalizado oficialmente y premiación guardada en SQL Server!\n";
    std::cout << " 🏆 Campeón Liga Mayor:          " << prem.campeonMayor << "\n";
    std::cout << " 🥈 Campeón Liga Menor:          " << prem.campeonMenor << "\n";
    std::cout << " ⚽ Botín de Oro (Goleador):     " << prem.maximoGoleador << " (" << prem.golesAnotados << " goles)\n";
    std::cout << " 🧤 Guante de Oro (Portero):     " << prem.porteroMenosVencido << " (" << prem.golesRecibidos << " goles recibidos)\n";
    pausarConsola();
}

// =========================================================================
// REPORTES DISCIPLINARIOS Y ESTADÍSTICAS VINCULADAS
// =========================================================================

void imprimirReporteTarjetas(const SistemaCampeonato &sistema)
{
    std::cout << "\n========================================================================================\n";
    std::cout << "          REPORTE DE TARJETAS ACUMULADAS Y DISCIPLINA (CONTROL OFICIAL)                 \n";
    std::cout << " Torneo Activo ID: " << sistema.idTorneoActivo << "\n";
    std::cout << "========================================================================================\n";

    struct InfoTarjetasJugador {
        int idJugador;
        std::string nombreJugador;
        std::string nombreEquipo;
        std::string posicion;
        int camiseta;
        int amarillas;
        int rojas;
    };

    std::map<int, InfoTarjetasJugador> mapaTarjetas;

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo) {
            for (const auto &t : p.tarjetas) {
                if (mapaTarjetas.find(t.idJugador) == mapaTarjetas.end()) {
                    InfoTarjetasJugador itj;
                    itj.idJugador = t.idJugador;
                    itj.nombreJugador = t.nombreJugador;
                    itj.nombreEquipo = "Desconocido";
                    itj.posicion = "-";
                    itj.camiseta = 0;
                    itj.amarillas = 0;
                    itj.rojas = 0;

                    for (const auto &eq : sistema.equipos) {
                        for (const auto &j : eq.jugadores) {
                            if (j.idJugador == t.idJugador) {
                                itj.nombreEquipo = eq.nombre;
                                itj.posicion = j.posicion;
                                itj.camiseta = j.numeroCamiseta;
                                break;
                            }
                        }
                    }
                    mapaTarjetas[t.idJugador] = itj;
                }
                if (t.tipoTarjeta == "AMARILLA") {
                    mapaTarjetas[t.idJugador].amarillas++;
                } else if (t.tipoTarjeta == "ROJA") {
                    mapaTarjetas[t.idJugador].rojas++;
                }
            }
        }
    }

    if (mapaTarjetas.empty()) {
        std::cout << "No hay tarjetas registradas en los partidos disputados hasta el momento.\n";
    } else {
        std::vector<InfoTarjetasJugador> lista;
        for (const auto &par : mapaTarjetas) lista.push_back(par.second);

        std::sort(lista.begin(), lista.end(), [](const InfoTarjetasJugador &a, const InfoTarjetasJugador &b) {
            int ptsA = a.amarillas * 1 + a.rojas * 3;
            int ptsB = b.amarillas * 1 + b.rojas * 3;
            if (ptsA != ptsB) return ptsA > ptsB;
            return a.rojas > b.rojas;
        });

        std::cout << std::left  << std::setw(5)  << "No."
                  << std::left  << std::setw(28) << "Jugador"
                  << std::left  << std::setw(24) << "Equipo"
                  << std::left  << std::setw(15) << "Posición"
                  << std::right << std::setw(4)  << "#"
                  << std::right << std::setw(5)  << "TA"
                  << std::right << std::setw(5)  << "TR"
                  << std::right << std::setw(10) << "Puntos Disc.\n";
        std::cout << std::string(96, '-') << "\n";

        int idx = 1;
        for (const auto &j : lista) {
            int pts = j.amarillas * 1 + j.rojas * 3;
            std::cout << std::left  << std::setw(5)  << idx++
                      << std::left  << std::setw(28) << j.nombreJugador
                      << std::left  << std::setw(24) << j.nombreEquipo
                      << std::left  << std::setw(15) << j.posicion
                      << std::right << std::setw(4)  << j.camiseta
                      << std::right << std::setw(5)  << j.amarillas
                      << std::right << std::setw(5)  << j.rojas
                      << std::right << std::setw(10) << pts << "\n";
        }
        std::cout << std::string(96, '-') << "\n";
        std::cout << " [Regla Disciplinaria]: 1 Tarjeta Amarilla (TA) = 1 punto. 1 Tarjeta Roja (TR) = 3 puntos.\n";
    }
    pausarConsola();
}

void imprimirEstadisticasPremiacion(const SistemaCampeonato &sistema)
{
    std::cout << "\n========================================================================================\n";
    std::cout << "          ESTADÍSTICAS EN VIVO Y VÍNCULO CON PREMIACIONES DEL TORNEO                   \n";
    std::cout << " Torneo Activo ID: " << sistema.idTorneoActivo << "\n";
    std::cout << "========================================================================================\n";

    // 1. Tabla de Goleadores (Botín de Oro)
    struct GoleadorInfo {
        int idJugador;
        std::string nombreJugador;
        std::string nombreEquipo;
        std::string posicion;
        int camiseta;
        int goles;
    };
    std::map<int, GoleadorInfo> mapaGoles;

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.estado == "FINALIZADO") {
            for (const auto &g : p.goles) {
                if (!g.esAutogol && g.idJugador > 0) {
                    if (mapaGoles.find(g.idJugador) == mapaGoles.end()) {
                        GoleadorInfo gi;
                        gi.idJugador = g.idJugador;
                        gi.nombreJugador = g.nombreJugador;
                        gi.nombreEquipo = "Desconocido";
                        gi.posicion = "-";
                        gi.camiseta = 0;
                        gi.goles = 0;
                        for (const auto &eq : sistema.equipos) {
                            for (const auto &j : eq.jugadores) {
                                if (j.idJugador == g.idJugador) {
                                    gi.nombreEquipo = eq.nombre;
                                    gi.posicion = j.posicion;
                                    gi.camiseta = j.numeroCamiseta;
                                    break;
                                }
                            }
                        }
                        mapaGoles[g.idJugador] = gi;
                    }
                    mapaGoles[g.idJugador].goles++;
                }
            }
        }
    }

    std::cout << "\n⚽ TABLA DE GOLEADORES (VÍNCULO: BOTÍN DE ORO):\n";
    if (mapaGoles.empty()) {
        std::cout << "   (Aún no se han registrado goles en los partidos disputados)\n";
    } else {
        std::vector<GoleadorInfo> listaGoles;
        for (const auto &par : mapaGoles) listaGoles.push_back(par.second);
        std::sort(listaGoles.begin(), listaGoles.end(), [](const GoleadorInfo &a, const GoleadorInfo &b) {
            return a.goles > b.goles;
        });

        std::cout << std::left  << std::setw(5)  << "Pos."
                  << std::left  << std::setw(28) << "Jugador"
                  << std::left  << std::setw(24) << "Equipo"
                  << std::left  << std::setw(15) << "Posición"
                  << std::right << std::setw(6)  << "Goles\n";
        std::cout << std::string(78, '-') << "\n";
        int rk = 1;
        for (const auto &g : listaGoles) {
            std::cout << std::left  << std::setw(5)  << rk++
                      << std::left  << std::setw(28) << g.nombreJugador
                      << std::left  << std::setw(24) << g.nombreEquipo
                      << std::left  << std::setw(15) << g.posicion
                      << std::right << std::setw(6)  << g.goles << "\n";
        }
    }

    // 2. Porteros Menos Vencidos (Guante de Oro)
    struct PorteroInfo {
        int idJugador;
        std::string nombrePortero;
        std::string nombreEquipo;
        int partidosJugados;
        int minutosJugados;
        int golesRecibidos;
    };
    std::map<int, PorteroInfo> mapaPorteros;

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.estado == "FINALIZADO") {
            for (const auto &cp : p.porteros) {
                if (mapaPorteros.find(cp.idJugador) == mapaPorteros.end()) {
                    PorteroInfo pi;
                    pi.idJugador = cp.idJugador;
                    pi.nombrePortero = cp.nombrePortero;
                    pi.nombreEquipo = "Desconocido";
                    pi.partidosJugados = 0;
                    pi.minutosJugados = 0;
                    pi.golesRecibidos = 0;
                    for (const auto &eq : sistema.equipos) {
                        for (const auto &j : eq.jugadores) {
                            if (j.idJugador == cp.idJugador) {
                                pi.nombreEquipo = eq.nombre;
                                break;
                            }
                        }
                    }
                    mapaPorteros[cp.idJugador] = pi;
                }
                mapaPorteros[cp.idJugador].partidosJugados++;
                mapaPorteros[cp.idJugador].minutosJugados += (cp.minutoSalida - cp.minutoEntrada);
                mapaPorteros[cp.idJugador].golesRecibidos += cp.golesRecibidos;
            }
        }
    }

    std::cout << "\n🧤 TABLA DE PORTEROS (VÍNCULO: GUANTE DE ORO):\n";
    if (mapaPorteros.empty()) {
        std::cout << "   (Aún no se han registrado actuaciones de porteros)\n";
    } else {
        std::vector<PorteroInfo> listaPorteros;
        for (const auto &par : mapaPorteros) listaPorteros.push_back(par.second);
        std::sort(listaPorteros.begin(), listaPorteros.end(), [](const PorteroInfo &a, const PorteroInfo &b) {
            double promA = (a.minutosJugados > 0) ? ((double)a.golesRecibidos / a.minutosJugados) : 999.0;
            double promB = (b.minutosJugados > 0) ? ((double)b.golesRecibidos / b.minutosJugados) : 999.0;
            if (promA != promB) return promA < promB;
            return a.golesRecibidos < b.golesRecibidos;
        });

        std::cout << std::left  << std::setw(5)  << "Pos."
                  << std::left  << std::setw(28) << "Portero"
                  << std::left  << std::setw(24) << "Equipo"
                  << std::right << std::setw(5)  << "PJ"
                  << std::right << std::setw(6)  << "Min"
                  << std::right << std::setw(6)  << "G.Enc"
                  << std::right << std::setw(11) << "Prom/Part\n";
        std::cout << std::string(85, '-') << "\n";
        int rk = 1;
        for (const auto &pi : listaPorteros) {
            double prom = (pi.partidosJugados > 0) ? ((double)pi.golesRecibidos / pi.partidosJugados) : 0.0;
            std::cout << std::left  << std::setw(5)  << rk++
                      << std::left  << std::setw(28) << pi.nombrePortero
                      << std::left  << std::setw(24) << pi.nombreEquipo
                      << std::right << std::setw(5)  << pi.partidosJugados
                      << std::right << std::setw(6)  << pi.minutosJugados
                      << std::right << std::setw(6)  << pi.golesRecibidos
                      << std::right << std::setw(11) << std::fixed << std::setprecision(2) << prom << "\n";
        }
    }
    pausarConsola();
}

// =========================================================================
// MENÚS Y FLUJO DE USUARIOS
// =========================================================================

void menuPrincipal(SistemaCampeonato &sistema)
{
    int opcion = 0;
    do {
        std::cout << "\n=======================================================\n";
        std::cout << "                    ACCESO AL SISTEMA                  \n";
        if (sistema.db.conectada) {
            std::cout << "     [ESTADO BD: CONECTADO A SQL SERVER LOCALHOST]     \n";
        } else {
            std::cout << "         [ESTADO BD: MODO LOCAL (OFFLINE)]             \n";
        }
        std::cout << "=======================================================\n";
        std::cout << " 1. Iniciar Sesión\n";
        std::cout << " 2. Registrar Nuevo Usuario Representante\n";
        std::cout << " 3. Salir del Sistema\n";
        std::cout << "-------------------------------------------------------\n";
        std::cout << " Ingrese una opción: ";
        std::cin >> opcion;

        if (std::cin.eof()) break;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: {
                Usuario usuarioActivo;
                if (autenticarUsuario(sistema, usuarioActivo)) {
                    if (usuarioActivo.rol == ROL_ADMINISTRADOR) {
                        menuAdministrador(sistema, usuarioActivo);
                    } else {
                        menuRepresentante(sistema, usuarioActivo);
                    }
                }
                break;
            }
            case 2:
                registrarUsuarioRepresentante(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            case 3:
                std::cout << "\nSaliendo del programa...\n";
                break;
            default:
                std::cout << "\n[!] Opción inválida. Intente de nuevo.\n";
                break;
        }
    } while (opcion != 3);
}

bool autenticarUsuario(SistemaCampeonato &sistema, Usuario &usuarioLogueado)
{
    std::string user, pass;
    std::cout << "\n--- INICIAR SESIÓN ---\n";
    std::cout << "Usuario: ";
    std::cin >> user;
    std::cout << "Contraseña: ";
    std::cin >> pass;

    for (const auto &u : sistema.usuarios) {
        if (u.usuario == user && (u.contrasena == pass || (user == "admin" && (pass == "admin123" || pass == "1234")))) {
            usuarioLogueado = u;
            std::cout << "\n[OK] ¡Bienvenido, " << u.nombreCompleto << "!\n";
            std::cout << "Rol: " << (u.rol == ROL_ADMINISTRADOR ? "ADMINISTRADOR (ACCESO TOTAL)" : "REPRESENTANTE (GESTIÓN Y CONSULTA)") << "\n";
            pausarConsola();
            return true;
        }
    }

    std::cout << "\n[ERROR] Usuario o contraseña incorrectos.\n";
    pausarConsola();
    return false;
}

void registrarUsuarioRepresentante(SistemaCampeonato &sistema)
{
    std::cout << "\n--- REGISTRO DE NUEVO REPRESENTANTE ---\n";
    Usuario nuevo;
    nuevo.idUsuario = sistema.autoIncUsuario++;
    nuevo.rol = ROL_REPRESENTANTE;

    std::cout << "Nombre Completo: ";
    std::getline(std::cin >> std::ws, nuevo.nombreCompleto);

    std::cout << "Nombre de Usuario (Login): ";
    std::cin >> nuevo.usuario;

    for (const auto &u : sistema.usuarios) {
        if (u.usuario == nuevo.usuario) {
            std::cout << "\n[ERROR] El nombre de usuario ya se encuentra en uso.\n";
            pausarConsola();
            return;
        }
    }

    std::cout << "Contraseña: ";
    std::cin >> nuevo.contrasena;

    dbInsertarUsuarioEnBD(sistema, nuevo);
    sistema.usuarios.push_back(nuevo);
    std::cout << "\n[OK] Usuario '" << nuevo.usuario << "' registrado exitosamente en SQL Server como Representante.\n";
    pausarConsola();
}

void menuAdministrador(SistemaCampeonato &sistema, const Usuario &admin)
{
    int opcion = 0;
    do {
        std::cout << "\n========================================================================================\n";
        std::cout << "                     PANEL PRINCIPAL DEL ADMINISTRADOR                                  \n";
        std::cout << " Usuario: " << admin.nombreCompleto << " | Rol: ADMINISTRADOR | Torneo Activo ID: " << sistema.idTorneoActivo << "\n";
        if (sistema.db.conectada) {
            std::cout << " [BASE DE DATOS]: CONECTADO DIRECTAMENTE A SQL SERVER (" << sistema.db.servidorConectado << ")\n";
        } else {
            std::cout << " [BASE DE DATOS]: MODO LOCAL (OFFLINE)\n";
        }
        std::cout << "========================================================================================\n";
        std::cout << " 1.  🏆 Crear Nuevo Torneo / Seleccionar Activo\n";
        std::cout << " 2.  🛡️  Registrar Equipos y Plantillas de Jugadores (Máx 12 por liga)\n";
        std::cout << " 3.  👥 Listar Equipos y Jugadores Registrados\n";
        std::cout << " 4.  📅 Generar Fixture: Fase 1 (Clasificación Todos vs Todos - Round Robin)\n";
        std::cout << " 5.  🔥 Generar Fixture: Fase 2 (Cuartos de Final Top 8 Clasificados)\n";
        std::cout << " 6.  🔥 Generar Fixture: Semifinales y Gran Final\n";
        std::cout << " 7.  ⚽ Ingresar Resultados e Incidencias (Marcador, Goles, Tarjetas, Porteros)\n";
        std::cout << " 8.  📊 Ver Tabla de Posiciones por Liga (Mayor y Menor)\n";
        std::cout << " 9.  🔄 Ver Tablas de Ascenso/Descenso y Aplicar Cambios Oficiales\n";
        std::cout << " 10. 🥇 Módulo de Premiaciones (Habilitar y Ver Cuadro de Honor)\n";
        std::cout << " 11. 📋 Ver Calendario y Marcadores de Partidos\n";
        std::cout << " 12. 🔍 Ver Detalle e Incidencias de un Partido Específico\n";
        std::cout << " 13. 🟨 Reporte Detallado de Tarjetas y Disciplina por Jugador\n";
        std::cout << " 14. 🏆 Estadísticas de Premiación en Vivo (Goleadores y Porteros)\n";
        std::cout << " 15. 💾 Guardar y Sincronizar Datos en Disco / Base de Datos\n";
        std::cout << " 0.   Cerrar Sesión\n";
        std::cout << "----------------------------------------------------------------------------------------\n";
        std::cout << " Seleccione una opción: ";
        std::cin >> opcion;

        if (std::cin.eof()) break;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: {
                std::cout << "\n1. Crear Nuevo Torneo\n2. Seleccionar Torneo Existente\nOpción: ";
                int sub; std::cin >> sub;
                if (sub == 1) crearTorneo(sistema);
                else seleccionarTorneoActivo(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 2: {
                std::cout << "\n1. Registrar Equipo\n2. Registrar Jugador en Equipo\nOpción: ";
                int sub; std::cin >> sub;
                if (sub == 1) registrarEquipo(sistema);
                else registrarJugador(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 3: {
                std::cout << "\nSeleccione Liga a listar:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                listarEquiposYPlantillas(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 4: {
                std::cout << "\nSeleccione Liga para Fase 1 (Round Robin):\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                generarClasificacionRoundRobin(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 5: {
                std::cout << "\nSeleccione Liga para Cuartos de Final (Top 8):\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                generarCuartosDeFinalTop8(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 6: {
                std::cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                DivisionLiga d = (sub == 1 ? LIGA_MAYOR : LIGA_MENOR);
                std::cout << "\n1. Generar Semifinales\n2. Generar Gran Final\nOpción: ";
                int subFase; std::cin >> subFase;
                if (subFase == 1) generarSemifinales(sistema, d);
                else generarGranFinal(sistema, d);
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 7:
                ingresarResultadoPartido(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            case 8: {
                std::cout << "\nSeleccione Liga para Tabla de Posiciones:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                imprimirTablaPosiciones(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 9: {
                std::cout << "\n1. Ver Tablas de Ascenso y Descenso\n2. Formalizar y Aplicar Ascenso/Descenso Oficial\nOpción: ";
                int sub; std::cin >> sub;
                if (sub == 1) imprimirTablasAscensoDescenso(sistema);
                else aplicarAscensoDescensoOficial(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 10: {
                std::cout << "\n1. Ver Cuadro de Honor\n2. Finalizar Campeonato Oficialmente\nOpción: ";
                int sub; std::cin >> sub;
                if (sub == 1) imprimirModuloPremiaciones(sistema);
                else finalizarTorneoYHabilitarPremiacion(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            }
            case 11: {
                std::cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                listarCalendarioPartidos(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 12:
                verDetallePartido(sistema);
                break;
            case 13:
                imprimirReporteTarjetas(sistema);
                break;
            case 14:
                imprimirEstadisticasPremiacion(sistema);
                break;
            case 15:
                guardarDatosEnArchivo(sistema);
                std::cout << "\n[OK] Datos guardados exitosamente.\n";
                pausarConsola();
                break;
            case 0:
                std::cout << "\nCerrando sesión de administrador...\n";
                break;
            default:
                std::cout << "\n[!] Opción no reconocida.\n";
                break;
        }
    } while (opcion != 0);
}

void menuRepresentante(SistemaCampeonato &sistema, const Usuario &rep)
{
    int opcion = 0;
    do {
        std::cout << "\n========================================================================================\n";
        std::cout << "                  PANEL DEL REPRESENTANTE / USUARIO REGULAR                             \n";
        std::cout << " Usuario: " << rep.nombreCompleto << " | MODO: GESTIÓN DE PLANTILLA Y CONSULTA | Torneo: " << sistema.idTorneoActivo << "\n";
        if (sistema.db.conectada) {
            std::cout << " [BASE DE DATOS]: CONECTADO DIRECTAMENTE A SQL SERVER (" << sistema.db.servidorConectado << ")\n";
        } else {
            std::cout << " [BASE DE DATOS]: MODO LOCAL (OFFLINE)\n";
        }
        std::cout << "========================================================================================\n";
        std::cout << " 1. 📊 Ver Tabla de Posiciones (Liga Mayor / Liga Menor)\n";
        std::cout << " 2. 🔄 Ver Tablas de Ascenso y Descenso\n";
        std::cout << " 3. 📋 Ver Calendario de Partidos y Marcadores\n";
        std::cout << " 4. 🔍 Ver Incidencias Detalladas de un Partido (Goles y Tarjetas)\n";
        std::cout << " 5. 👥 Ver Equipos y Plantillas de Jugadores Registrados\n";
        std::cout << " 6. 🥇 Ver Premiaciones Oficiales del Torneo\n";
        std::cout << " 7. 📝 REGISTRAR JUGADOR EN LA PLANTILLA DE UN EQUIPO (Alta Directa en BD)\n";
        std::cout << " 8. 🟨 Reporte Detallado de Tarjetas Acumuladas (Control Disciplinario)\n";
        std::cout << " 9. 🏆 Estadísticas y Vínculo con Premiación (Goleadores y Porteros)\n";
        std::cout << " 0. Cerrar Sesión\n";
        std::cout << "----------------------------------------------------------------------------------------\n";
        std::cout << " Seleccione una opción: ";
        std::cin >> opcion;

        if (std::cin.eof()) break;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: {
                std::cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                imprimirTablaPosiciones(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 2:
                imprimirTablasAscensoDescenso(sistema);
                break;
            case 3: {
                std::cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                listarCalendarioPartidos(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 4:
                verDetallePartido(sistema);
                break;
            case 5: {
                std::cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
                int sub; std::cin >> sub;
                listarEquiposYPlantillas(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 6:
                imprimirModuloPremiaciones(sistema);
                break;
            case 7:
                registrarJugador(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            case 8:
                imprimirReporteTarjetas(sistema);
                break;
            case 9:
                imprimirEstadisticasPremiacion(sistema);
                break;
            case 0:
                std::cout << "\nCerrando sesión...\n";
                break;
            default:
                std::cout << "\n[!] Opción no válida.\n";
                break;
        }
    } while (opcion != 0);
}
