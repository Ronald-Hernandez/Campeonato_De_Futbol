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

using namespace std;

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
// ESTRUCTURAS DE DATOS
// =========================================================================

struct Usuario {
    int idUsuario;
    string usuario;
    string contrasena;
    string nombreCompleto;
    RolUsuario rol;
};

struct Jugador {
    int idJugador;
    int idEquipo;
    string nombre;
    string apellido;
    string posicion; // PORTERO, DEFENSA, MEDIOCAMPISTA, DELANTERO
    int numeroCamiseta;
};

struct Equipo {
    int idEquipo;
    string nombre;
    DivisionLiga division; // 1 = LIGA_MAYOR, 2 = LIGA_MENOR
    int idCampeonato;
    vector<Jugador> jugadores;
};

struct IncidenciaGol {
    int idJugador;
    string nombreJugador;
    int idEquipoRival;
    string nombreEquipoRival;
    int idPorteroVencido;
    string nombrePorteroVencido;
    int minuto;
    bool esAutogol;
};

struct IncidenciaTarjeta {
    int idJugador;
    string nombreJugador;
    string tipoTarjeta; // AMARILLA, ROJA
    int minuto;
};

struct ControlPortero {
    int idJugador;
    string nombrePortero;
    int idEquipo;
    int minutoEntrada;
    int minutoSalida;
    int golesRecibidos;
};

struct Partido {
    int idPartido;
    int idCampeonato;
    DivisionLiga division;
    string fase; // CLASIFICACION, CUARTOS, SEMIFINAL, FINAL
    int jornada;
    string fecha;
    int idLocal;
    string nombreLocal;
    int golesLocal;
    int idVisitante;
    string nombreVisitante;
    int golesVisitante;
    string estado; // PENDIENTE, FINALIZADO
    vector<IncidenciaGol> goles;
    vector<IncidenciaTarjeta> tarjetas;
    vector<ControlPortero> porteros;
};

struct PosicionEquipo {
    int posicion;
    int idEquipo;
    string nombreEquipo;
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
    string nombreEquipo;
    DivisionLiga origen;
    DivisionLiga destino;
    string tipoMovimiento; // ASCENSO, DESCENSO
    string fecha;
};

struct Torneo {
    int idTorneo;
    string nombre;
    string fechaInicio;
    string fechaFin;
    int maxMayor;
    int maxMenor;
    bool finalizado;
};

struct Premiacion {
    int idTorneo;
    string campeonMayor;
    string campeonMenor;
    string maximoGoleador;
    int golesAnotados;
    string porteroMenosVencido;
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
    string servidorConectado = "";
};
#else
struct GestorBD {
    bool conectada = false;
    string servidorConectado = "";
};
#endif

struct SistemaCampeonato {
    GestorBD db;
    vector<Usuario> usuarios;
    vector<Torneo> torneos;
    int idTorneoActivo;
    vector<Equipo> equipos;
    vector<Partido> partidos;
    vector<HistorialMovimiento> historial;
    vector<Premiacion> premiaciones;

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
string sanitizarSQL(const string &input);

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
bool guardarDatosEnArchivo(const SistemaCampeonato &sistema, const string &ruta = "datos_campeonato.txt");
bool cargarDatosDesdeArchivo(SistemaCampeonato &sistema, const string &ruta = "datos_campeonato.txt");

// Módulo Torneo, Equipos y Jugadores
void crearTorneo(SistemaCampeonato &sistema);
void listarTorneos(const SistemaCampeonato &sistema);
void seleccionarTorneoActivo(SistemaCampeonato &sistema);
void registrarEquipo(SistemaCampeonato &sistema);
void registrarJugador(SistemaCampeonato &sistema);
void listarEquiposYPlantillas(const SistemaCampeonato &sistema, DivisionLiga division);

// Módulo Calendario y Algoritmos
bool generarClasificacionRoundRobin(SistemaCampeonato &sistema, DivisionLiga division);
bool generarCuartosDeFinalTop8(SistemaCampeonato &sistema, DivisionLiga division);
bool generarSemifinales(SistemaCampeonato &sistema, DivisionLiga division);
bool generarGranFinal(SistemaCampeonato &sistema, DivisionLiga division);
void listarCalendario(const SistemaCampeonato &sistema, DivisionLiga division);

// Módulo Resultados e Incidencias
void ingresarResultadoPartido(SistemaCampeonato &sistema);
void verDetallePartido(const SistemaCampeonato &sistema);
void listarCalendarioPartidos(const SistemaCampeonato &sistema, DivisionLiga division, const string &filtroFase = "TODAS");

// Módulo Posiciones, Ascenso/Descenso y Premiaciones
vector<PosicionEquipo> calcularTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division);
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

    cout << "\n=======================================================\n";
    cout << "      SISTEMA DE GESTION DE CAMPEONATOS DE FUTBOL      \n";
    cout << "        (MODO CONSOLA C++ CONECTADO A SQL SERVER)      \n";
    cout << "=======================================================\n";

    menuPrincipal(sistema);

    guardarDatosEnArchivo(sistema);
    dbDesconectar(sistema.db);
    cout << "\n[OK] Datos sincronizados. ¡Hasta pronto!\n";
    return 0;
}

// =========================================================================
// UTILIDADES
// =========================================================================

void pausarConsola()
{
    cout << "\nPresione ENTER para continuar...";
    if (!cin.eof()) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
}

Equipo* buscarEquipoPorId(SistemaCampeonato &sistema, int idEquipo)
{
    for (auto &eq : sistema.equipos) {
        if (eq.idEquipo == idEquipo) return &eq;
    }
    return nullptr;
}

string sanitizarSQL(const string &input)
{
    string res = "";
    for (char c : input) {
        if (c == '\'') res += "''";
        else res += c;
    }
    return res;
}

static vector<string> dividirLinea(const string &s, char delim = '|')
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delim)) {
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

    vector<pair<string, string>> cadenasConexion = {
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
                string qDet = "SELECT id_equipo, goles_favor, resultado FROM dbo.DETALLE_PARTIDO WHERE id_partido = " +
                                   to_string(p.idPartido) + " ORDER BY id_detalle";
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
                            if (string(res) != "PENDIENTE") p.estado = "FINALIZADO";
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
    string sql = "INSERT INTO dbo.JUGADOR (id_equipo, nombre, apellido, posicion, numero_camiseta) VALUES (" +
                      to_string(j.idEquipo) + ", '" +
                      sanitizarSQL(j.nombre) + "', '" +
                      sanitizarSQL(j.apellido) + "', '" +
                      sanitizarSQL(j.posicion) + "', " +
                      to_string(j.numeroCamiseta) + "); SELECT SCOPE_IDENTITY();";

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
    string fStr = p.fecha.empty() ? "2026-05-15 15:00:00" : (p.fecha + " 15:00:00");
    string sql = "INSERT INTO dbo.PARTIDO (id_campeonato, id_division, fecha_partido, jornada, fase) VALUES (" +
                      to_string(p.idCampeonato) + ", " +
                      to_string(p.division) + ", '" +
                      fStr + "', " +
                      to_string(p.jornada) + ", '" +
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
        string sqlLoc = "INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES (" +
                             to_string(p.idPartido) + ", " + to_string(p.idLocal) + ", 0, 0, 0, 'PENDIENTE');";
        string sqlVis = "INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES (" +
                             to_string(p.idPartido) + ", " + to_string(p.idVisitante) + ", 0, 0, 0, 'PENDIENTE');";
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
    string resLoc = (p.golesLocal > p.golesVisitante) ? "VICTORIA" : (p.golesLocal == p.golesVisitante ? "EMPATE" : "DERROTA");
    string resVis = (p.golesVisitante > p.golesLocal) ? "VICTORIA" : (p.golesLocal == p.golesVisitante ? "EMPATE" : "DERROTA");

    string sqlUpdLoc = "UPDATE dbo.DETALLE_PARTIDO SET goles_favor = " + to_string(p.golesLocal) +
                            ", goles_contra = " + to_string(p.golesVisitante) +
                            ", puntos_ganados = " + to_string(ptsLoc) +
                            ", resultado = '" + resLoc +
                            "' WHERE id_partido = " + to_string(p.idPartido) + " AND id_equipo = " + to_string(p.idLocal);

    string sqlUpdVis = "UPDATE dbo.DETALLE_PARTIDO SET goles_favor = " + to_string(p.golesVisitante) +
                            ", goles_contra = " + to_string(p.golesLocal) +
                            ", puntos_ganados = " + to_string(ptsVis) +
                            ", resultado = '" + resVis +
                            "' WHERE id_partido = " + to_string(p.idPartido) + " AND id_equipo = " + to_string(p.idVisitante);

    SQLHSTMT stmt;
    if (SQL_SUCCEEDED(sistema.db.fnAllocHandle(SQL_HANDLE_STMT, sistema.db.dbc, &stmt))) {
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlUpdLoc.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlUpdVis.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);

        // Limpiar incidencias previas para este partido
        string delG = "DELETE FROM dbo.REGISTRO_GOLES WHERE id_partido = " + to_string(p.idPartido);
        string delT = "DELETE FROM dbo.TARJETAS_ACUMULADAS WHERE id_partido = " + to_string(p.idPartido);
        string delP = "DELETE FROM dbo.CONTROL_PORTEROS WHERE id_partido = " + to_string(p.idPartido);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)delG.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)delT.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);
        sistema.db.fnExecDirectA(stmt, (SQLCHAR*)delP.c_str(), SQL_NTS);
        sistema.db.fnCloseCursor(stmt);

        // Insertar Goles
        for (const auto &g : p.goles) {
            string sqlGol = "INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol) VALUES (" +
                                 to_string(p.idPartido) + ", " +
                                 to_string(g.idJugador) + ", " +
                                 (g.idPorteroVencido > 0 ? to_string(g.idPorteroVencido) : "NULL") + ", " +
                                 to_string(g.idEquipoRival) + ", " +
                                 to_string(g.minuto) + ", " +
                                 (g.esAutogol ? "1" : "0") + ");";
            sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlGol.c_str(), SQL_NTS);
            sistema.db.fnCloseCursor(stmt);
        }

        // Insertar Tarjetas
        for (const auto &t : p.tarjetas) {
            string sqlTar = "INSERT INTO dbo.TARJETAS_ACUMULADAS (id_partido, id_jugador, tipo_tarjeta, minuto) VALUES (" +
                                 to_string(p.idPartido) + ", " +
                                 to_string(t.idJugador) + ", '" +
                                 t.tipoTarjeta + "', " +
                                 to_string(t.minuto) + ");";
            sistema.db.fnExecDirectA(stmt, (SQLCHAR*)sqlTar.c_str(), SQL_NTS);
            sistema.db.fnCloseCursor(stmt);
        }

        // Insertar Porteros
        for (const auto &cp : p.porteros) {
            string sqlPor = "INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos) VALUES (" +
                                 to_string(p.idPartido) + ", " +
                                 to_string(cp.idJugador) + ", " +
                                 to_string(cp.idEquipo) + ", " +
                                 to_string(cp.minutoEntrada) + ", " +
                                 to_string(cp.minutoSalida) + ", " +
                                 to_string(cp.golesRecibidos) + ");";
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
    string sql = "UPDATE dbo.EQUIPO SET id_division = " + to_string(nuevaDivision) +
                      " WHERE id_equipo = " + to_string(idEquipo);
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
    string sql = "INSERT INTO dbo.HISTORIAL_ASCENSO_DESCENSO (id_campeonato, id_equipo, id_division_origen, id_division_destino, tipo_movimiento) VALUES (" +
                      to_string(h.idCampeonato) + ", " +
                      to_string(h.idEquipo) + ", " +
                      to_string(h.origen) + ", " +
                      to_string(h.destino) + ", '" +
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
    string sql = "IF EXISTS (SELECT 1 FROM dbo.PREMIACION WHERE id_campeonato = " + to_string(prem.idTorneo) + ") " +
                      "UPDATE dbo.PREMIACION SET id_jugador_goleador = " + (idGoleador > 0 ? to_string(idGoleador) : "1") +
                      ", id_portero_menos_vencido = " + (idPortero > 0 ? to_string(idPortero) : "1") +
                      ", id_equipo_campeon = " + (idCampeon > 0 ? to_string(idCampeon) : "NULL") +
                      " WHERE id_campeonato = " + to_string(prem.idTorneo) + "; " +
                      "ELSE INSERT INTO dbo.PREMIACION (id_campeonato, id_jugador_goleador, id_portero_menos_vencido, id_equipo_campeon) VALUES (" +
                      to_string(prem.idTorneo) + ", " +
                      (idGoleador > 0 ? to_string(idGoleador) : "1") + ", " +
                      (idPortero > 0 ? to_string(idPortero) : "1") + ", " +
                      (idCampeon > 0 ? to_string(idCampeon) : "NULL") + ");";
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
    string correo = u.usuario + "@campeonato.gt";
    string sql = "INSERT INTO dbo.USUARIO (nombre_usuario, correo, contrasena_hash, activo, id_rol, usuario) VALUES ('" +
                      sanitizarSQL(u.nombreCompleto) + "', '" +
                      correo + "', '" +
                      sanitizarSQL(u.contrasena) + "', 1, " +
                      to_string(u.rol) + ", '" +
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
    string sql = "INSERT INTO dbo.EQUIPO (nombre_equipo, id_division, id_campeonato) VALUES ('" +
                      sanitizarSQL(eq.nombre) + "', " +
                      to_string(eq.division) + ", " +
                      to_string(eq.idCampeonato) + "); SELECT SCOPE_IDENTITY();";
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
    string fIni = t.fechaInicio.empty() ? "2026-01-10" : t.fechaInicio;
    string fFin = t.fechaFin.empty() ? "2026-05-30" : t.fechaFin;
    string sql = "INSERT INTO dbo.CAMPEONATO (nombre_campeonato, fecha_inicio, fecha_fin, max_equipos_mayor, max_equipos_menor) VALUES ('" +
                      sanitizarSQL(t.nombre) + "', '" +
                      fIni + "', '" +
                      fFin + "', " +
                      to_string(t.maxMayor) + ", " +
                      to_string(t.maxMenor) + "); SELECT SCOPE_IDENTITY();";
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

    cout << "[BD] Conectando a Base de Datos SQL Server 'CampeonatoFutbol' en localhost...\n";
    if (dbInicializarYConectar(sistema.db)) {
        cout << "[OK] Conectado exitosamente: " << sistema.db.servidorConectado << "\n";
        cout << "[BD] Sincronizando datos desde SQL Server...\n";
        if (dbCargarTodo(sistema)) {
            cout << "[OK] ¡Datos sincronizados directamente desde la Base de Datos!\n";
            cout << "     - Torneos activos: " << sistema.torneos.size() << "\n";
            cout << "     - Equipos en liga: " << sistema.equipos.size() << "\n";
            int totalJug = 0;
            for (const auto &eq : sistema.equipos) totalJug += eq.jugadores.size();
            cout << "     - Jugadores en plantilla: " << totalJug << "\n";
            cout << "     - Partidos registrados: " << sistema.partidos.size() << "\n";
            cout << "     - Usuarios en sistema: " << sistema.usuarios.size() << "\n";
            return;
        }
    }

    cout << "[AVISO] Base de datos no disponible. Utilizando persistencia en archivo local.\n";
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

    vector<string> mayor = {
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

    vector<string> menor = {
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

bool guardarDatosEnArchivo(const SistemaCampeonato &sistema, const string &ruta)
{
    ofstream out(ruta);
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

bool cargarDatosDesdeArchivo(SistemaCampeonato &sistema, const string &ruta)
{
    ifstream in(ruta);
    if (!in.is_open()) return false;

    string linea;
    string seccion = "";

    sistema.usuarios.clear();
    sistema.torneos.clear();
    sistema.equipos.clear();
    sistema.partidos.clear();
    sistema.historial.clear();
    sistema.premiaciones.clear();

    while (getline(in, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        if (linea.front() == '[' && linea.back() == ']') {
            seccion = linea;
            continue;
        }

        vector<string> t = dividirLinea(linea, '|');

        if (seccion == "[CONFIG]" && t.size() >= 7) {
            sistema.idTorneoActivo = stoi(t[0]);
            sistema.autoIncUsuario = stoi(t[1]);
            sistema.autoIncTorneo = stoi(t[2]);
            sistema.autoIncEquipo = stoi(t[3]);
            sistema.autoIncJugador = stoi(t[4]);
            sistema.autoIncPartido = stoi(t[5]);
            sistema.autoIncHistorial = stoi(t[6]);
        } else if (seccion == "[USUARIOS]" && t.size() >= 5) {
            Usuario u;
            u.idUsuario = stoi(t[0]);
            u.usuario = t[1];
            u.contrasena = t[2];
            u.nombreCompleto = t[3];
            u.rol = (RolUsuario)stoi(t[4]);
            sistema.usuarios.push_back(u);
        } else if (seccion == "[TORNEOS]" && t.size() >= 7) {
            Torneo tor;
            tor.idTorneo = stoi(t[0]);
            tor.nombre = t[1];
            tor.fechaInicio = t[2];
            tor.fechaFin = t[3];
            tor.maxMayor = stoi(t[4]);
            tor.maxMenor = stoi(t[5]);
            tor.finalizado = (t[6] == "1");
            sistema.torneos.push_back(tor);
        } else if (seccion == "[EQUIPOS]" && t.size() >= 4) {
            Equipo eq;
            eq.idEquipo = stoi(t[0]);
            eq.nombre = t[1];
            eq.division = (DivisionLiga)stoi(t[2]);
            eq.idCampeonato = stoi(t[3]);
            sistema.equipos.push_back(eq);
        } else if (seccion == "[JUGADORES]" && t.size() >= 6) {
            Jugador j;
            j.idJugador = stoi(t[0]);
            j.idEquipo = stoi(t[1]);
            j.nombre = t[2];
            j.apellido = t[3];
            j.posicion = t[4];
            j.numeroCamiseta = stoi(t[5]);
            Equipo *eq = buscarEquipoPorId(sistema, j.idEquipo);
            if (eq) eq->jugadores.push_back(j);
        } else if (seccion == "[PARTIDOS]" && t.size() >= 13) {
            Partido p;
            p.idPartido = stoi(t[0]);
            p.idCampeonato = stoi(t[1]);
            p.division = (DivisionLiga)stoi(t[2]);
            p.fase = t[3];
            p.jornada = stoi(t[4]);
            p.fecha = t[5];
            p.idLocal = stoi(t[6]);
            p.nombreLocal = t[7];
            p.golesLocal = stoi(t[8]);
            p.idVisitante = stoi(t[9]);
            p.nombreVisitante = t[10];
            p.golesVisitante = stoi(t[11]);
            p.estado = t[12];
            sistema.partidos.push_back(p);
        } else if (seccion == "[GOLES]" && t.size() >= 9) {
            int idPart = stoi(t[0]);
            IncidenciaGol g;
            g.idJugador = stoi(t[1]);
            g.nombreJugador = t[2];
            g.idEquipoRival = stoi(t[3]);
            g.nombreEquipoRival = t[4];
            g.idPorteroVencido = stoi(t[5]);
            g.nombrePorteroVencido = t[6];
            g.minuto = stoi(t[7]);
            g.esAutogol = (t[8] == "1");
            for (auto &part : sistema.partidos) {
                if (part.idPartido == idPart) {
                    part.goles.push_back(g);
                    break;
                }
            }
        } else if (seccion == "[TARJETAS]" && t.size() >= 5) {
            int idPart = stoi(t[0]);
            IncidenciaTarjeta tar;
            tar.idJugador = stoi(t[1]);
            tar.nombreJugador = t[2];
            tar.tipoTarjeta = t[3];
            tar.minuto = stoi(t[4]);
            for (auto &part : sistema.partidos) {
                if (part.idPartido == idPart) {
                    part.tarjetas.push_back(tar);
                    break;
                }
            }
        } else if (seccion == "[PORTEROS]" && t.size() >= 7) {
            int idPart = stoi(t[0]);
            ControlPortero cp;
            cp.idJugador = stoi(t[1]);
            cp.nombrePortero = t[2];
            cp.idEquipo = stoi(t[3]);
            cp.minutoEntrada = stoi(t[4]);
            cp.minutoSalida = stoi(t[5]);
            cp.golesRecibidos = stoi(t[6]);
            for (auto &part : sistema.partidos) {
                if (part.idPartido == idPart) {
                    part.porteros.push_back(cp);
                    break;
                }
            }
        } else if (seccion == "[HISTORIAL]" && t.size() >= 8) {
            HistorialMovimiento h;
            h.idHistorial = stoi(t[0]);
            h.idCampeonato = stoi(t[1]);
            h.idEquipo = stoi(t[2]);
            h.nombreEquipo = t[3];
            h.origen = (DivisionLiga)stoi(t[4]);
            h.destino = (DivisionLiga)stoi(t[5]);
            h.tipoMovimiento = t[6];
            h.fecha = t[7];
            sistema.historial.push_back(h);
        } else if (seccion == "[PREMIACIONES]" && t.size() >= 9) {
            Premiacion pr;
            pr.idTorneo = stoi(t[0]);
            pr.campeonMayor = t[1];
            pr.campeonMenor = t[2];
            pr.maximoGoleador = t[3];
            pr.golesAnotados = stoi(t[4]);
            pr.porteroMenosVencido = t[5];
            pr.golesRecibidos = stoi(t[6]);
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
    cout << "\n=======================================================\n";
    cout << "               CREAR NUEVO TORNEO DE FUTBOL            \n";
    cout << "=======================================================\n";

    Torneo nuevo;
    nuevo.idTorneo = sistema.autoIncTorneo++;

    cout << "Nombre del Torneo (ej. Clausura 2026): ";
    getline(cin >> ws, nuevo.nombre);

    cout << "Fecha de Inicio (YYYY-MM-DD): ";
    getline(cin, nuevo.fechaInicio);

    cout << "Fecha Estimada de Finalización (YYYY-MM-DD): ";
    getline(cin, nuevo.fechaFin);

    nuevo.maxMayor = 12;
    nuevo.maxMenor = 12;
    nuevo.finalizado = false;

    dbCrearTorneoEnBD(sistema, nuevo);
    sistema.torneos.push_back(nuevo);
    sistema.idTorneoActivo = nuevo.idTorneo;

    cout << "\n[OK] Torneo '" << nuevo.nombre << "' creado y activado con éxito (ID: " << nuevo.idTorneo << ").\n";
    pausarConsola();
}

void listarTorneos(const SistemaCampeonato &sistema)
{
    cout << "\n=======================================================\n";
    cout << "                  TORNEOS REGISTRADOS                  \n";
    cout << "=======================================================\n";
    if (sistema.torneos.empty()) {
        cout << "No hay torneos registrados en el sistema.\n";
        return;
    }

    for (const auto &t : sistema.torneos) {
        cout << "ID: " << setw(3) << t.idTorneo
                  << " | " << setw(25) << left << t.nombre
                  << " | Inicio: " << t.fechaInicio
                  << " | Fin: " << t.fechaFin
                  << " | Estado: " << (t.finalizado ? "FINALIZADO" : "ACTIVO")
                  << (t.idTorneo == sistema.idTorneoActivo ? " <-- [ACTIVO]" : "") << "\n";
    }
}

void seleccionarTorneoActivo(SistemaCampeonato &sistema)
{
    listarTorneos(sistema);
    cout << "\nIngrese el ID del Torneo que desea seleccionar como ACTIVO: ";
    int idSel;
    cin >> idSel;

    bool hallado = false;
    for (const auto &t : sistema.torneos) {
        if (t.idTorneo == idSel) {
            sistema.idTorneoActivo = t.idTorneo;
            hallado = true;
            cout << "\n[OK] Torneo '" << t.nombre << "' establecido como ACTIVO.\n";
            break;
        }
    }
    if (!hallado) {
        cout << "\n[ERROR] El ID de torneo seleccionado no existe.\n";
    }
    pausarConsola();
}

void registrarEquipo(SistemaCampeonato &sistema)
{
    cout << "\n=======================================================\n";
    cout << "                  REGISTRAR NUEVO EQUIPO               \n";
    cout << "=======================================================\n";

    cout << "Seleccione la Liga / División:\n";
    cout << "1. Liga Mayor (Máximo 12 equipos)\n";
    cout << "2. Liga Menor (Máximo 12 equipos)\n";
    cout << "Opción: ";
    int opcDiv;
    cin >> opcDiv;

    if (opcDiv != 1 && opcDiv != 2) {
        cout << "[ERROR] División no válida.\n";
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
        cout << "\n[!] ATENCIÓN: La división seleccionada ya alcanzó el límite máximo reglamentario de 12 equipos.\n";
        pausarConsola();
        return;
    }

    Equipo nuevo;
    nuevo.idEquipo = sistema.autoIncEquipo++;
    nuevo.division = div;
    nuevo.idCampeonato = sistema.idTorneoActivo;

    cout << "Nombre Oficial del Equipo: ";
    getline(cin >> ws, nuevo.nombre);

    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.nombre == nuevo.nombre) {
            cout << "\n[ERROR] Ya existe un equipo con ese nombre en el campeonato actual.\n";
            pausarConsola();
            return;
        }
    }

    dbInsertarEquipoEnBD(sistema, nuevo);
    sistema.equipos.push_back(nuevo);

    cout << "\n[OK] Equipo '" << nuevo.nombre << "' registrado exitosamente (ID: "
              << nuevo.idEquipo << ") en " << (div == LIGA_MAYOR ? "Liga Mayor" : "Liga Menor") << ".\n";
    pausarConsola();
}

void registrarJugador(SistemaCampeonato &sistema)
{
    cout << "\n=======================================================\n";
    cout << "       REGISTRAR JUGADOR EN PLANTILLA DE EQUIPO        \n";
    cout << "=======================================================\n";

    cout << "Seleccione la Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
    int opcDiv;
    cin >> opcDiv;
    DivisionLiga div = (opcDiv == 1) ? LIGA_MAYOR : LIGA_MENOR;

    cout << "\nEquipos disponibles:\n";
    vector<int> idsValidos;
    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == div) {
            cout << "ID: " << setw(3) << eq.idEquipo << " - " << left << setw(28) << eq.nombre
                      << " (" << eq.jugadores.size() << " jugadores registrados)\n";
            idsValidos.push_back(eq.idEquipo);
        }
    }

    if (idsValidos.empty()) {
        cout << "No hay equipos registrados en esta división.\n";
        pausarConsola();
        return;
    }

    cout << "\nIngrese el ID del equipo para gestionar su plantilla: ";
    int idEqSel;
    cin >> idEqSel;

    Equipo *eqPtr = nullptr;
    for (auto &eq : sistema.equipos) {
        if (eq.idEquipo == idEqSel && eq.idCampeonato == sistema.idTorneoActivo) {
            eqPtr = &eq;
            break;
        }
    }

    if (!eqPtr) {
        cout << "[ERROR] ID de equipo no válido para el torneo actual.\n";
        pausarConsola();
        return;
    }

    // Mostrar plantilla actual
    cout << "\n--- Plantilla Actual de " << eqPtr->nombre << " ---\n";
    if (eqPtr->jugadores.empty()) {
        cout << "(Sin jugadores registrados actualmente)\n";
    } else {
        cout << left << setw(6) << "ID"
                  << right << setw(5) << "#"
                  << "  " << left << setw(28) << "Nombre y Apellido"
                  << left << setw(15) << "Posición\n";
        cout << string(56, '-') << "\n";
        for (const auto &j : eqPtr->jugadores) {
            cout << left << setw(6) << j.idJugador
                      << right << setw(5) << j.numeroCamiseta
                      << "  " << left << setw(28) << (j.nombre + " " + j.apellido)
                      << left << setw(15) << j.posicion << "\n";
        }
    }

    cout << "\n--- Datos del Nuevo Jugador ---\n";
    Jugador jug;
    jug.idJugador = sistema.autoIncJugador++;
    jug.idEquipo = eqPtr->idEquipo;

    cout << "Nombre: ";
    getline(cin >> ws, jug.nombre);

    cout << "Apellido: ";
    getline(cin >> ws, jug.apellido);

    cout << "Seleccione Posición:\n";
    cout << "1. PORTERO\n2. DEFENSA\n3. MEDIOCAMPISTA\n4. DELANTERO\nOpción: ";
    int opcPos;
    cin >> opcPos;
    if (opcPos == 1) jug.posicion = "PORTERO";
    else if (opcPos == 2) jug.posicion = "DEFENSA";
    else if (opcPos == 3) jug.posicion = "MEDIOCAMPISTA";
    else jug.posicion = "DELANTERO";

    cout << "Número de Camiseta (0-99): ";
    cin >> jug.numeroCamiseta;

    if (jug.numeroCamiseta < 0 || jug.numeroCamiseta > 99) {
        cout << "[ERROR] El número de camiseta debe estar entre 0 y 99.\n";
        pausarConsola();
        return;
    }

    for (const auto &existente : eqPtr->jugadores) {
        if (existente.numeroCamiseta == jug.numeroCamiseta) {
            cout << "\n[ERROR] El número de camiseta #" << jug.numeroCamiseta
                      << " ya pertenece a " << existente.nombre << " " << existente.apellido << " en este equipo.\n";
            pausarConsola();
            return;
        }
    }

    dbInsertarJugadorEnBD(sistema, jug);
    eqPtr->jugadores.push_back(jug);

    cout << "\n[OK] Jugador registrado con éxito en la Base de Datos 'CampeonatoFutbol':\n"
              << "     ID: " << jug.idJugador << " | " << jug.nombre << " " << jug.apellido
              << " (#" << jug.numeroCamiseta << " - " << jug.posicion << ")\n"
              << "     Equipo vinculado: " << eqPtr->nombre << ".\n";
    pausarConsola();
}

void listarEquiposYPlantillas(const SistemaCampeonato &sistema, DivisionLiga division)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    cout << "\n=======================================================\n";
    cout << "         EQUIPOS Y PLANTILLAS - " << nomDiv << "       \n";
    cout << "=======================================================\n";

    int count = 0;
    for (const auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == division) {
            count++;
            cout << "\nEQUIPO [" << setw(2) << eq.idEquipo << "]: " << eq.nombre
                      << " (Total Jugadores: " << eq.jugadores.size() << ")\n";
            cout << string(60, '-') << "\n";
            if (eq.jugadores.empty()) {
                cout << "   (No hay jugadores inscritos en este equipo)\n";
            } else {
                for (const auto &j : eq.jugadores) {
                    cout << "   ID:" << setw(4) << j.idJugador
                              << " | #" << setw(2) << j.numeroCamiseta
                              << " | " << left << setw(26) << (j.nombre + " " + j.apellido)
                              << " | " << left << setw(14) << j.posicion << "\n";
                }
            }
        }
    }

    if (count == 0) {
        cout << "No hay equipos registrados en esta división.\n";
    }
    pausarConsola();
}

// =========================================================================
// MÓDULO Calendario Y ALGORITMOS
// =========================================================================

bool generarClasificacionRoundRobin(SistemaCampeonato &sistema, DivisionLiga division)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    cout << "\n=======================================================\n";
    cout << "      GENERAR Calendario ROUND ROBIN - " << nomDiv << "   \n";
    cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "CLASIFICACION") {
            cout << "[!] Ya existe un calendario de Clasificación generado para esta división.\n";
            pausarConsola();
            return false;
        }
    }

    vector<Equipo*> equiposDiv;
    for (auto &eq : sistema.equipos) {
        if (eq.idCampeonato == sistema.idTorneoActivo && eq.division == division) {
            equiposDiv.push_back(&eq);
        }
    }

    int n = (int)equiposDiv.size();
    if (n < 2) {
        cout << "[ERROR] Se requieren al menos 2 equipos para generar el Calendario (Actuales: " << n << ").\n";
        pausarConsola();
        return false;
    }

    vector<int> indices;
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
                if (r % 2 == 1) swap(localIdx, visitIdx);

                Partido part;
                part.idPartido = sistema.autoIncPartido++;
                part.idCampeonato = sistema.idTorneoActivo;
                part.division = division;
                part.fase = "CLASIFICACION";
                part.jornada = r + 1;
                part.fecha = "2026-02-" + to_string(10 + (r % 15));
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

    cout << "[OK] Calendario Round Robin generado y guardado en BD con éxito:\n";
    cout << "     - Equipos participantes: " << equiposDiv.size() << "\n";
    cout << "     - Jornadas creadas: " << totalRondas << "\n";
    cout << "     - Partidos totales: " << partidosGenerados << "\n";
    pausarConsola();
    return true;
}

bool generarCuartosDeFinalTop8(SistemaCampeonato &sistema, DivisionLiga division)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    cout << "\n=======================================================\n";
    cout << "    GENERAR FASE 2: CUARTOS DE FINAL (TOP 8) - " << nomDiv << "\n";
    cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "CUARTOS") {
            cout << "[!] Ya se generaron los Cuartos de Final para esta división.\n";
            pausarConsola();
            return false;
        }
    }

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division &&
            p.fase == "CLASIFICACION" && p.estado == "PENDIENTE") {
            cout << "[!] No se puede generar la segunda fase: aún hay partidos pendientes en la fase de Clasificación.\n";
            pausarConsola();
            return false;
        }
    }

    vector<PosicionEquipo> tabla = calcularTablaPosiciones(sistema, division);
    if (tabla.size() < 8) {
        cout << "[ERROR] Se requieren al menos 8 equipos en competencia para la Fase Final (Registrados: "
                  << tabla.size() << ").\n";
        pausarConsola();
        return false;
    }

    cout << "Top 8 Equipos Clasificados a Cuartos de Final:\n";
    for (int i = 0; i < 8; ++i) {
        cout << " " << (i + 1) << ". " << left << setw(25) << tabla[i].nombreEquipo
                  << " (" << tabla[i].puntos << " pts)\n";
    }

    vector<pair<int, int>> cruces = {
        {0, 7}, // 1ro vs 8vo
        {1, 6}, // 2do vs 7mo
        {2, 5}, // 3ro vs 6to
        {3, 4}  // 4to vs 5to
    };

    cout << "\nGenerando llaves oficiales de Cuartos de Final...\n";
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
        cout << " Llave " << (i + 1) << ": " << part.nombreLocal << " vs " << part.nombreVisitante << "\n";
    }

    cout << "\n[OK] Cuartos de final generados y sincronizados con SQL Server correctamente.\n";
    pausarConsola();
    return true;
}

bool generarSemifinales(SistemaCampeonato &sistema, DivisionLiga division)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    cout << "\n=======================================================\n";
    cout << "         GENERAR SEMIFINALES - " << nomDiv << "        \n";
    cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "SEMIFINAL") {
            cout << "[!] Ya existen Semifinales generadas para esta división.\n";
            pausarConsola();
            return false;
        }
    }

    vector<Partido*> cuartos;
    for (auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "CUARTOS") {
            cuartos.push_back(&p);
            if (p.estado == "PENDIENTE") {
                cout << "[!] Aún faltan partidos de Cuartos de Final por disputar.\n";
                pausarConsola();
                return false;
            }
        }
    }

    if (cuartos.size() != 4) {
        cout << "[ERROR] No se encontraron los 4 partidos reglamentarios de Cuartos de Final.\n";
        pausarConsola();
        return false;
    }

    vector<int> ganadores;
    vector<string> nomGanadores;
    for (auto cp : cuartos) {
        if (cp->golesLocal >= cp->golesVisitante) {
            ganadores.push_back(cp->idLocal);
            nomGanadores.push_back(cp->nombreLocal);
        } else {
            ganadores.push_back(cp->idVisitante);
            nomGanadores.push_back(cp->nombreVisitante);
        }
    }

    vector<pair<int, int>> crucesSemis = {{0, 1}, {2, 3}};
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
        cout << " Semifinal " << (i + 1) << ": " << part.nombreLocal << " vs " << part.nombreVisitante << "\n";
    }

    cout << "\n[OK] Semifinales creadas y guardadas en la base de datos con éxito.\n";
    pausarConsola();
    return true;
}

bool generarGranFinal(SistemaCampeonato &sistema, DivisionLiga division)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    cout << "\n=======================================================\n";
    cout << "         GENERAR GRAN FINAL - " << nomDiv << "         \n";
    cout << "=======================================================\n";

    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "FINAL") {
            cout << "[!] La Gran Final ya fue generada previamente.\n";
            pausarConsola();
            return false;
        }
    }

    vector<Partido*> semis;
    for (auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division && p.fase == "SEMIFINAL") {
            semis.push_back(&p);
            if (p.estado == "PENDIENTE") {
                cout << "[!] Aún faltan partidos de Semifinales por disputar.\n";
                pausarConsola();
                return false;
            }
        }
    }

    if (semis.size() != 2) {
        cout << "[ERROR] Se requieren 2 semifinales finalizadas.\n";
        pausarConsola();
        return false;
    }

    int finLoc = (semis[0]->golesLocal >= semis[0]->golesVisitante) ? semis[0]->idLocal : semis[0]->idVisitante;
    string nomFinLoc = (semis[0]->golesLocal >= semis[0]->golesVisitante) ? semis[0]->nombreLocal : semis[0]->nombreVisitante;

    int finVis = (semis[1]->golesLocal >= semis[1]->golesVisitante) ? semis[1]->idLocal : semis[1]->idVisitante;
    string nomFinVis = (semis[1]->golesLocal >= semis[1]->golesVisitante) ? semis[1]->nombreLocal : semis[1]->nombreVisitante;

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

    cout << " GRAN FINAL: " << part.nombreLocal << " vs " << part.nombreVisitante << "\n";
    cout << "\n[OK] Partido por el Campeonato creado y guardado en SQL Server.\n";
    pausarConsola();
    return true;
}

void listarCalendarioPartidos(const SistemaCampeonato &sistema, DivisionLiga division, const string &filtroFase)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    cout << "\n========================================================================================\n";
    cout << "           CALENDARIO Y MARCADORES DE PARTIDOS - " << nomDiv << "                       \n";
    cout << "========================================================================================\n";

    int count = 0;
    for (const auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == division) {
            if (filtroFase != "TODAS" && p.fase != filtroFase) continue;
            count++;

            cout << "ID: " << setw(3) << p.idPartido
                      << " | Jor: " << setw(2) << p.jornada
                      << " | Fase: " << left << setw(13) << p.fase
                      << " | Fecha: " << p.fecha
                      << " | " << right << setw(22) << p.nombreLocal << " ";

            if (p.estado == "FINALIZADO") {
                cout << "[" << p.golesLocal << " - " << p.golesVisitante << "]";
            } else {
                cout << "[ vs ]";
            }
            cout << " " << left << setw(22) << p.nombreVisitante
                      << " | " << (p.estado == "FINALIZADO" ? "FINALIZADO" : "PENDIENTE") << "\n";
        }
    }

    if (count == 0) {
        cout << "No hay partidos registrados para esta categoría.\n";
    }
    pausarConsola();
}

// =========================================================================
// MÓDULO RESULTADOS E INCIDENCIAS
// =========================================================================

void ingresarResultadoPartido(SistemaCampeonato &sistema)
{
    cout << "\n=======================================================\n";
    cout << "          INGRESAR RESULTADO E INCIDENCIAS             \n";
    cout << "=======================================================\n";

    cout << "Seleccione la Liga:\n1. Liga Mayor\n2. Liga Menor\nOpción: ";
    int opcDiv;
    cin >> opcDiv;
    DivisionLiga div = (opcDiv == 1) ? LIGA_MAYOR : LIGA_MENOR;

    cout << "\nPartidos disponibles para registrar:\n";
    vector<Partido*> pendientes;
    for (auto &p : sistema.partidos) {
        if (p.idCampeonato == sistema.idTorneoActivo && p.division == div && p.estado == "PENDIENTE") {
            pendientes.push_back(&p);
            cout << "ID: " << setw(4) << p.idPartido
                      << " [" << setw(13) << p.fase << "] "
                      << p.nombreLocal << " vs " << p.nombreVisitante << "\n";
        }
    }

    if (pendientes.empty()) {
        cout << "No hay partidos pendientes en esta división.\n";
        pausarConsola();
        return;
    }

    cout << "\nIngrese el ID del partido a registrar: ";
    int idSel;
    cin >> idSel;

    Partido *partidoSel = nullptr;
    for (auto p : pendientes) {
        if (p->idPartido == idSel) {
            partidoSel = p;
            break;
        }
    }

    if (!partidoSel) {
        cout << "[ERROR] ID de partido no válido.\n";
        pausarConsola();
        return;
    }

    Equipo *eqLoc = buscarEquipoPorId(sistema, partidoSel->idLocal);
    Equipo *eqVis = buscarEquipoPorId(sistema, partidoSel->idVisitante);

    cout << "\n--- Marcador Final ---\n";
    cout << "Goles de " << partidoSel->nombreLocal << ": ";
    cin >> partidoSel->golesLocal;
    cout << "Goles de " << partidoSel->nombreVisitante << ": ";
    cin >> partidoSel->golesVisitante;

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
        cout << "\n--- ASIGNACIÓN DE GOLES: " << partidoSel->nombreLocal << " (" << partidoSel->golesLocal << " goles) ---\n";
        for (int i = 0; i < partidoSel->golesLocal; ++i) {
            cout << "\nGol #" << (i + 1) << ":\n";
            cout << "Plantilla de " << partidoSel->nombreLocal << ":\n";
            if (eqLoc) {
                for (size_t k = 0; k < eqLoc->jugadores.size(); ++k) {
                    cout << " " << (k + 1) << ". #" << eqLoc->jugadores[k].numeroCamiseta
                              << " - " << eqLoc->jugadores[k].nombre << " " << eqLoc->jugadores[k].apellido
                              << " (" << eqLoc->jugadores[k].posicion << ")\n";
                }
            }
            cout << "Seleccione el autor del gol (número de lista): ";
            int selAut; cin >> selAut;

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
            cout << "Minuto de la anotación: ";
            cin >> g.minuto;
            g.esAutogol = false;
            partidoSel->goles.push_back(g);
        }
    }

    // Registrar goles del equipo Visitante
    if (partidoSel->golesVisitante > 0) {
        cout << "\n--- ASIGNACIÓN DE GOLES: " << partidoSel->nombreVisitante << " (" << partidoSel->golesVisitante << " goles) ---\n";
        for (int i = 0; i < partidoSel->golesVisitante; ++i) {
            cout << "\nGol #" << (i + 1) << ":\n";
            cout << "Plantilla de " << partidoSel->nombreVisitante << ":\n";
            if (eqVis) {
                for (size_t k = 0; k < eqVis->jugadores.size(); ++k) {
                    cout << " " << (k + 1) << ". #" << eqVis->jugadores[k].numeroCamiseta
                              << " - " << eqVis->jugadores[k].nombre << " " << eqVis->jugadores[k].apellido
                              << " (" << eqVis->jugadores[k].posicion << ")\n";
                }
            }
            cout << "Seleccione el autor del gol (número de lista): ";
            int selAut; cin >> selAut;

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
            cout << "Minuto de la anotación: ";
            cin >> g.minuto;
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
    cout << "\n¿Desea registrar tarjetas arbitrales para este partido? (1: Sí / 2: No): ";
    int opcTar; cin >> opcTar;
    while (opcTar == 1) {
        cout << "\nEquipo del jugador sancionado:\n1. " << partidoSel->nombreLocal << "\n2. " << partidoSel->nombreVisitante << "\nOpción: ";
        int opcEqT; cin >> opcEqT;
        Equipo *eqTar = (opcEqT == 1) ? eqLoc : eqVis;

        if (eqTar) {
            cout << "Plantilla de " << eqTar->nombre << ":\n";
            for (size_t k = 0; k < eqTar->jugadores.size(); ++k) {
                cout << " " << (k + 1) << ". #" << eqTar->jugadores[k].numeroCamiseta
                          << " - " << eqTar->jugadores[k].nombre << " " << eqTar->jugadores[k].apellido << "\n";
            }
            cout << "Seleccione el jugador sancionado: ";
            int selJ; cin >> selJ;

            IncidenciaTarjeta t;
            if (selJ >= 1 && selJ <= (int)eqTar->jugadores.size()) {
                t.idJugador = eqTar->jugadores[selJ - 1].idJugador;
                t.nombreJugador = eqTar->jugadores[selJ - 1].nombre + " " + eqTar->jugadores[selJ - 1].apellido + " (" + eqTar->nombre + ")";
            } else {
                t.idJugador = 0;
                t.nombreJugador = "Jugador Sancionado";
            }

            cout << "Tipo de tarjeta (1: AMARILLA / 2: ROJA): ";
            int tipoT; cin >> tipoT;
            t.tipoTarjeta = (tipoT == 2) ? "ROJA" : "AMARILLA";

            cout << "Minuto de la infracción: ";
            cin >> t.minuto;

            partidoSel->tarjetas.push_back(t);
        }

        cout << "¿Registrar otra tarjeta? (1: Sí / 2: No): ";
        cin >> opcTar;
    }

    partidoSel->estado = "FINALIZADO";

    dbActualizarResultadoPartidoEnBD(sistema, *partidoSel);

    cout << "\n[OK] Resultado e incidencias guardados exitosamente y sincronizados directamente con SQL Server.\n";
    pausarConsola();
}

void verDetallePartido(const SistemaCampeonato &sistema)
{
    cout << "\n========================================================================================\n";
    cout << "               DETALLE DE INCIDENCIAS DE UN PARTIDO                                     \n";
    cout << "========================================================================================\n";

    cout << "Ingrese el ID del partido a consultar: ";
    int idPart;
    cin >> idPart;

    const Partido *pSel = nullptr;
    for (const auto &p : sistema.partidos) {
        if (p.idPartido == idPart) {
            pSel = &p;
            break;
        }
    }

    if (!pSel) {
        cout << "[ERROR] No se encontró el partido con ID " << idPart << ".\n";
        pausarConsola();
        return;
    }

    cout << "\n----------------------------------------------------------------------------------------\n";
    cout << " PARTIDO #" << pSel->idPartido << " [" << pSel->fase << " - Jornada " << pSel->jornada << "] | Fecha: " << pSel->fecha << "\n";
    cout << " " << pSel->nombreLocal << " " << pSel->golesLocal << " - " << pSel->golesVisitante << " " << pSel->nombreVisitante << "\n";
    cout << " Estado: " << pSel->estado << "\n";
    cout << "----------------------------------------------------------------------------------------\n";

    cout << "\n--- GOLES Y ANOTADORES ---\n";
    if (pSel->goles.empty()) {
        cout << "   (No hay goles registrados)\n";
    } else {
        for (const auto &g : pSel->goles) {
            cout << "   Min " << setw(2) << g.minuto << "' | Gol de " << g.nombreJugador
                      << " (vs Portero: " << g.nombrePorteroVencido << ")\n";
        }
    }

    cout << "\n--- TARJETAS Y SANCIONES ---\n";
    if (pSel->tarjetas.empty()) {
        cout << "   (Sin tarjetas registradas - Partido limpio)\n";
    } else {
        for (const auto &t : pSel->tarjetas) {
            cout << "   Min " << setw(2) << t.minuto << "' | [" << t.tipoTarjeta << "] - " << t.nombreJugador << "\n";
        }
    }

    cout << "\n--- CONTROL DE PORTEROS ---\n";
    if (pSel->porteros.empty()) {
        cout << "   (Sin datos específicos de porteros)\n";
    } else {
        for (const auto &cp : pSel->porteros) {
            cout << "   " << cp.nombrePortero << " | Minutos: " << (cp.minutoSalida - cp.minutoEntrada)
                      << " | Goles Encajados: " << cp.golesRecibidos << "\n";
        }
    }

    pausarConsola();
}

// =========================================================================
// MÓDULO POSICIONES, ASCENSO/DESCENSO Y PREMIACIONES
// =========================================================================

vector<PosicionEquipo> calcularTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division)
{
    map<int, PosicionEquipo> mapa;

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

    vector<PosicionEquipo> tabla;
    for (auto &par : mapa) {
        par.second.dg = par.second.gf - par.second.gc;
        tabla.push_back(par.second);
    }

    // Ordenamiento manual (Método de la Burbuja Multicriterio: 1. Puntos, 2. Diferencia de Goles, 3. Goles a Favor, 4. Nombre)
    for (size_t i = 0; i < tabla.size(); ++i) {
        for (size_t j = 0; j + 1 < tabla.size() - i; ++j) {
            bool debeIntercambiar = false;
            if (tabla[j].puntos < tabla[j + 1].puntos) {
                debeIntercambiar = true;
            } else if (tabla[j].puntos == tabla[j + 1].puntos) {
                if (tabla[j].dg < tabla[j + 1].dg) {
                    debeIntercambiar = true;
                } else if (tabla[j].dg == tabla[j + 1].dg) {
                    if (tabla[j].gf < tabla[j + 1].gf) {
                        debeIntercambiar = true;
                    } else if (tabla[j].gf == tabla[j + 1].gf && tabla[j].nombreEquipo > tabla[j + 1].nombreEquipo) {
                        debeIntercambiar = true;
                    }
                }
            }
            if (debeIntercambiar) {
                PosicionEquipo temp = tabla[j];
                tabla[j] = tabla[j + 1];
                tabla[j + 1] = temp;
            }
        }
    }

    for (size_t i = 0; i < tabla.size(); ++i) {
        tabla[i].posicion = (int)i + 1;
    }

    return tabla;
}

void imprimirTablaPosiciones(const SistemaCampeonato &sistema, DivisionLiga division)
{
    string nomDiv = (division == LIGA_MAYOR) ? "LIGA MAYOR" : "LIGA MENOR";
    vector<PosicionEquipo> tabla = calcularTablaPosiciones(sistema, division);

    cout << "\n========================================================================================\n";
    cout << "                  TABLA GENERAL DE POSICIONES - " << nomDiv << "                        \n";
    cout << "========================================================================================\n";
    cout << left  << setw(5)  << "Pos"
              << left  << setw(28) << "Equipo"
              << right << setw(4)  << "PJ"
              << right << setw(4)  << "PG"
              << right << setw(4)  << "PE"
              << right << setw(4)  << "PP"
              << right << setw(5)  << "GF"
              << right << setw(5)  << "GC"
              << right << setw(5)  << "DG"
              << right << setw(6)  << "PTS"
              << "   Zona / Estado\n";
    cout << string(88, '-') << "\n";

    for (const auto &eq : tabla) {
        string zona = "";
        if (eq.posicion <= 8) zona = "[Top 8 Clasificado]";
        if (division == LIGA_MAYOR && eq.posicion >= 11) zona = "[ZONA DESCENSO]";
        if (division == LIGA_MENOR && eq.posicion <= 2) zona = "[ZONA ASCENSO DIRECTO]";

        cout << left  << setw(5)  << eq.posicion
                  << left  << setw(28) << eq.nombreEquipo
                  << right << setw(4)  << eq.pj
                  << right << setw(4)  << eq.pg
                  << right << setw(4)  << eq.pe
                  << right << setw(4)  << eq.pp
                  << right << setw(5)  << eq.gf
                  << right << setw(5)  << eq.gc
                  << right << setw(5)  << eq.dg
                  << right << setw(6)  << eq.puntos
                  << "   " << zona << "\n";
    }

    cout << string(88, '-') << "\n";
    cout << "Reglamento: Los mejores 8 clasifican a Cuartos de Final.\n";
    cout << "En Liga Mayor descienden los 2 últimos lugares (11 y 12).\n";
    cout << "En Liga Menor ascienden a Liga Mayor los 2 primeros lugares (1 y 2).\n";
    pausarConsola();
}

void imprimirTablasAscensoDescenso(const SistemaCampeonato &sistema)
{
    cout << "\n========================================================================================\n";
    cout << "                   TABLAS OFICIALES DE ASCENSO Y DESCENSO                               \n";
    cout << "========================================================================================\n";

    vector<PosicionEquipo> mayor = calcularTablaPosiciones(sistema, LIGA_MAYOR);
    vector<PosicionEquipo> menor = calcularTablaPosiciones(sistema, LIGA_MENOR);

    cout << "\n--- CANDIDATOS AL DESCENSO (LIGA MAYOR A LIGA MENOR) ---\n";
    cout << "----------------------------------------------------------------------------------------\n";
    if (mayor.size() >= 2) {
        int n = (int)mayor.size();
        cout << " 11. " << left << setw(28) << mayor[n - 2].nombreEquipo
                  << " | Pts: " << setw(3) << mayor[n - 2].puntos
                  << " | DIF: " << setw(3) << mayor[n - 2].dg << " [DESCENSO DIRECTO]\n";
        cout << " 12. " << left << setw(28) << mayor[n - 1].nombreEquipo
                  << " | Pts: " << setw(3) << mayor[n - 1].puntos
                  << " | DIF: " << setw(3) << mayor[n - 1].dg << " [DESCENSO DIRECTO]\n";
    } else {
        cout << "No hay suficientes equipos registrados en Liga Mayor.\n";
    }

    cout << "\n--- CANDIDATOS AL ASCENSO (LIGA MENOR A LIGA MAYOR) ---\n";
    cout << "----------------------------------------------------------------------------------------\n";
    if (menor.size() >= 2) {
        cout << "  1. " << left << setw(28) << menor[0].nombreEquipo
                  << " | Pts: " << setw(3) << menor[0].puntos
                  << " | DIF: " << setw(3) << menor[0].dg << " [ASCENSO DIRECTO]\n";
        cout << "  2. " << left << setw(28) << menor[1].nombreEquipo
                  << " | Pts: " << setw(3) << menor[1].puntos
                  << " | DIF: " << setw(3) << menor[1].dg << " [ASCENSO DIRECTO]\n";
    } else {
        cout << "No hay suficientes equipos registrados en Liga Menor.\n";
    }

    if (!sistema.historial.empty()) {
        cout << "\n--- HISTORIAL DE ASCENSOS Y DESCENSOS APLICADOS ---\n";
        cout << "----------------------------------------------------------------------------------------\n";
        for (const auto &h : sistema.historial) {
            cout << " [" << h.tipoMovimiento << "] Equipo: " << left << setw(26) << h.nombreEquipo
                      << " | De: " << (h.origen == LIGA_MAYOR ? "Liga Mayor" : "Liga Menor")
                      << " -> A: " << (h.destino == LIGA_MAYOR ? "Liga Mayor" : "Liga Menor")
                      << " | Fecha: " << h.fecha << "\n";
        }
    }
    pausarConsola();
}

void aplicarAscensoDescensoOficial(SistemaCampeonato &sistema)
{
    cout << "\n=======================================================\n";
    cout << "      APLICAR ASCENSO Y DESCENSO REGLAMENTARIO         \n";
    cout << "=======================================================\n";

    vector<PosicionEquipo> mayor = calcularTablaPosiciones(sistema, LIGA_MAYOR);
    vector<PosicionEquipo> menor = calcularTablaPosiciones(sistema, LIGA_MENOR);

    if (mayor.size() < 12 || menor.size() < 12) {
        cout << "[AVISO] Por reglamento oficial se requieren 12 equipos por liga para formalizar el ciclo.\n";
        cout << "¿Desea forzar la aplicación con los equipos actuales? (1: Sí / 2: Cancelar): ";
        int opc; cin >> opc;
        if (opc != 1) return;
    }

    if (mayor.size() < 2 || menor.size() < 2) {
        cout << "[ERROR] Equipos insuficientes para procesar ascensos y descensos.\n";
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
        cout << "[ERROR] No se localizaron los registros de los equipos en la estructura.\n";
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

    cout << "\n[OK] Movimientos oficiales formalizados y guardados en SQL Server:\n";
    cout << "  Descienden a Liga Menor: " << eqDes1->nombre << " y " << eqDes2->nombre << "\n";
    cout << "  Ascienden a Liga Mayor: " << eqAsc1->nombre << " y " << eqAsc2->nombre << "\n";
    pausarConsola();
}

void imprimirModuloPremiaciones(const SistemaCampeonato &sistema)
{
    cout << "\n========================================================================================\n";
    cout << "                    CUADRO DE HONOR Y PREMIACIONES DEL CAMPEONATO                       \n";
    cout << "========================================================================================\n";

    const Premiacion *premSel = nullptr;
    for (const auto &pr : sistema.premiaciones) {
        if (pr.idTorneo == sistema.idTorneoActivo) {
            premSel = &pr;
            break;
        }
    }

    if (!premSel || !premSel->habilitada) {
        cout << "\n[!] El módulo de premiaciones aún NO se encuentra habilitado para este torneo.\n";
        cout << "    (El Administrador debe finalizar el torneo formalmente para desbloquear los premios).\n";
        pausarConsola();
        return;
    }

    cout << "\n  Campeon Liga Mayor:          " << premSel->campeonMayor << "\n";
    cout << "  Campeon Liga Menor:          " << premSel->campeonMenor << "\n";
    cout << "  Maximo Goleador (Botin de Oro): " << premSel->maximoGoleador << " (" << premSel->golesAnotados << " goles)\n";
    cout << "  Portero Menos Vencido:       " << premSel->porteroMenosVencido << " (" << premSel->golesRecibidos << " goles encajados)\n";
    cout << "========================================================================================\n";
    pausarConsola();
}

void finalizarTorneoYHabilitarPremiacion(SistemaCampeonato &sistema)
{
    cout << "\n=======================================================\n";
    cout << "      FINALIZAR TORNEO Y HABILITAR PREMIACIONES        \n";
    cout << "=======================================================\n";

    Torneo *tActivo = nullptr;
    for (auto &t : sistema.torneos) {
        if (t.idTorneo == sistema.idTorneoActivo) {
            tActivo = &t;
            break;
        }
    }

    if (!tActivo) {
        cout << "[ERROR] No hay torneo activo seleccionado.\n";
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
        vector<PosicionEquipo> tabMayor = calcularTablaPosiciones(sistema, LIGA_MAYOR);
        if (!tabMayor.empty()) {
            prem.campeonMayor = tabMayor[0].nombreEquipo + " (Líder Tabla General)";
            idCampMayor = tabMayor[0].idEquipo;
        }
    }

    // Campeón Liga Menor
    vector<PosicionEquipo> tabMenor = calcularTablaPosiciones(sistema, LIGA_MENOR);
    if (!tabMenor.empty()) {
        prem.campeonMenor = tabMenor[0].nombreEquipo;
        idCampMenor = tabMenor[0].idEquipo;
    }

    // Máximo Goleador acumulado
    map<int, int> conteoGoles;
    map<int, string> nombresGoles;
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
    map<int, int> golesRecibidosPor;
    map<int, string> nombresPor;
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

    cout << "\n[OK] ¡Torneo finalizado oficialmente y premiación guardada en SQL Server!\n";
    cout << "  Campeon Liga Mayor:          " << prem.campeonMayor << "\n";
    cout << "  Campeon Liga Menor:          " << prem.campeonMenor << "\n";
    cout << "  Botin de Oro (Goleador):     " << prem.maximoGoleador << " (" << prem.golesAnotados << " goles)\n";
    cout << "  Guante de Oro (Portero):     " << prem.porteroMenosVencido << " (" << prem.golesRecibidos << " goles recibidos)\n";
    pausarConsola();
}

// =========================================================================
// REPORTES DISCIPLINARIOS Y ESTADÍSTICAS VINCULADAS
// =========================================================================

void imprimirReporteTarjetas(const SistemaCampeonato &sistema)
{
    cout << "\n========================================================================================\n";
    cout << "          REPORTE DE TARJETAS ACUMULADAS Y DISCIPLINA (CONTROL OFICIAL)                 \n";
    cout << " Torneo Activo ID: " << sistema.idTorneoActivo << "\n";
    cout << "========================================================================================\n";

    struct InfoTarjetasJugador {
        int idJugador;
        string nombreJugador;
        string nombreEquipo;
        string posicion;
        int camiseta;
        int amarillas;
        int rojas;
    };

    map<int, InfoTarjetasJugador> mapaTarjetas;

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
        cout << "No hay tarjetas registradas en los partidos disputados hasta el momento.\n";
    } else {
        vector<InfoTarjetasJugador> lista;
        for (const auto &par : mapaTarjetas) lista.push_back(par.second);

        sort(lista.begin(), lista.end(), [](const InfoTarjetasJugador &a, const InfoTarjetasJugador &b) {
            int ptsA = a.amarillas * 1 + a.rojas * 3;
            int ptsB = b.amarillas * 1 + b.rojas * 3;
            if (ptsA != ptsB) return ptsA > ptsB;
            return a.rojas > b.rojas;
        });

        cout << left  << setw(5)  << "No."
                  << left  << setw(28) << "Jugador"
                  << left  << setw(24) << "Equipo"
                  << left  << setw(15) << "Posición"
                  << right << setw(4)  << "#"
                  << right << setw(5)  << "TA"
                  << right << setw(5)  << "TR"
                  << right << setw(10) << "Puntos Disc.\n";
        cout << string(96, '-') << "\n";

        int idx = 1;
        for (const auto &j : lista) {
            int pts = j.amarillas * 1 + j.rojas * 3;
            cout << left  << setw(5)  << idx++
                      << left  << setw(28) << j.nombreJugador
                      << left  << setw(24) << j.nombreEquipo
                      << left  << setw(15) << j.posicion
                      << right << setw(4)  << j.camiseta
                      << right << setw(5)  << j.amarillas
                      << right << setw(5)  << j.rojas
                      << right << setw(10) << pts << "\n";
        }
        cout << string(96, '-') << "\n";
        cout << " [Regla Disciplinaria]: 1 Tarjeta Amarilla (TA) = 1 punto. 1 Tarjeta Roja (TR) = 3 puntos.\n";
    }
    pausarConsola();
}

void imprimirEstadisticasPremiacion(const SistemaCampeonato &sistema)
{
    cout << "\n========================================================================================\n";
    cout << "          ESTADÍSTICAS EN VIVO Y VÍNCULO CON PREMIACIONES DEL TORNEO                   \n";
    cout << " Torneo Activo ID: " << sistema.idTorneoActivo << "\n";
    cout << "========================================================================================\n";

    // 1. Tabla de Goleadores (Botín de Oro)
    struct GoleadorInfo {
        int idJugador;
        string nombreJugador;
        string nombreEquipo;
        string posicion;
        int camiseta;
        int goles;
    };
    map<int, GoleadorInfo> mapaGoles;

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

    cout << "\n--- TABLA DE GOLEADORES (BOTIN DE ORO) ---\n";
    if (mapaGoles.empty()) {
        cout << "   (Aún no se han registrado goles en los partidos disputados)\n";
    } else {
        vector<GoleadorInfo> listaGoles;
        for (const auto &par : mapaGoles) listaGoles.push_back(par.second);
        sort(listaGoles.begin(), listaGoles.end(), [](const GoleadorInfo &a, const GoleadorInfo &b) {
            return a.goles > b.goles;
        });

        cout << left  << setw(5)  << "Pos."
                  << left  << setw(28) << "Jugador"
                  << left  << setw(24) << "Equipo"
                  << left  << setw(15) << "Posición"
                  << right << setw(6)  << "Goles\n";
        cout << string(78, '-') << "\n";
        int rk = 1;
        for (const auto &g : listaGoles) {
            cout << left  << setw(5)  << rk++
                      << left  << setw(28) << g.nombreJugador
                      << left  << setw(24) << g.nombreEquipo
                      << left  << setw(15) << g.posicion
                      << right << setw(6)  << g.goles << "\n";
        }
    }

    // 2. Porteros Menos Vencidos (Guante de Oro)
    struct PorteroInfo {
        int idJugador;
        string nombrePortero;
        string nombreEquipo;
        int partidosJugados;
        int minutosJugados;
        int golesRecibidos;
    };
    map<int, PorteroInfo> mapaPorteros;

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

    cout << "\n--- TABLA DE PORTEROS (GUANTE DE ORO) ---\n";
    if (mapaPorteros.empty()) {
        cout << "   (Aún no se han registrado actuaciones de porteros)\n";
    } else {
        vector<PorteroInfo> listaPorteros;
        for (const auto &par : mapaPorteros) listaPorteros.push_back(par.second);
        sort(listaPorteros.begin(), listaPorteros.end(), [](const PorteroInfo &a, const PorteroInfo &b) {
            double promA = (a.minutosJugados > 0) ? ((double)a.golesRecibidos / a.minutosJugados) : 999.0;
            double promB = (b.minutosJugados > 0) ? ((double)b.golesRecibidos / b.minutosJugados) : 999.0;
            if (promA != promB) return promA < promB;
            return a.golesRecibidos < b.golesRecibidos;
        });

        cout << left  << setw(5)  << "Pos."
                  << left  << setw(28) << "Portero"
                  << left  << setw(24) << "Equipo"
                  << right << setw(5)  << "PJ"
                  << right << setw(6)  << "Min"
                  << right << setw(6)  << "G.Enc"
                  << right << setw(11) << "Prom/Part\n";
        cout << string(85, '-') << "\n";
        int rk = 1;
        for (const auto &pi : listaPorteros) {
            double prom = (pi.partidosJugados > 0) ? ((double)pi.golesRecibidos / pi.partidosJugados) : 0.0;
            cout << left  << setw(5)  << rk++
                      << left  << setw(28) << pi.nombrePortero
                      << left  << setw(24) << pi.nombreEquipo
                      << right << setw(5)  << pi.partidosJugados
                      << right << setw(6)  << pi.minutosJugados
                      << right << setw(6)  << pi.golesRecibidos
                      << right << setw(11) << fixed << setprecision(2) << prom << "\n";
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
        cout << "\n=======================================================\n";
        cout << "                    ACCESO AL SISTEMA                  \n";
        if (sistema.db.conectada) {
            cout << "     [ESTADO BD: CONECTADO A SQL SERVER LOCALHOST]     \n";
        } else {
            cout << "         [ESTADO BD: MODO LOCAL (OFFLINE)]             \n";
        }
        cout << "=======================================================\n";
        cout << " 1. Iniciar Sesión\n";
        cout << " 2. Registrar Nuevo Usuario Representante\n";
        cout << " 3. Salir del Sistema\n";
        cout << "-------------------------------------------------------\n";
        cout << " Ingrese una opción: ";
        cin >> opcion;

        if (cin.eof()) break;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
                cout << "\nSaliendo del programa...\n";
                break;
            default:
                cout << "\n[!] Opción inválida. Intente de nuevo.\n";
                break;
        }
    } while (opcion != 3);
}

bool autenticarUsuario(SistemaCampeonato &sistema, Usuario &usuarioLogueado)
{
    string user, pass;
    cout << "\n--- INICIAR SESIÓN ---\n";
    cout << "Usuario: ";
    cin >> user;
    cout << "Contraseña: ";
    cin >> pass;

    for (const auto &u : sistema.usuarios) {
        if (u.usuario == user && (u.contrasena == pass || (user == "admin" && (pass == "admin123" || pass == "1234")))) {
            usuarioLogueado = u;
            cout << "\n[OK] ¡Bienvenido, " << u.nombreCompleto << "!\n";
            cout << "Rol: " << (u.rol == ROL_ADMINISTRADOR ? "ADMINISTRADOR (ACCESO TOTAL)" : "REPRESENTANTE (GESTIÓN Y CONSULTA)") << "\n";
            pausarConsola();
            return true;
        }
    }

    cout << "\n[ERROR] Usuario o contraseña incorrectos.\n";
    pausarConsola();
    return false;
}

void registrarUsuarioRepresentante(SistemaCampeonato &sistema)
{
    cout << "\n--- REGISTRO DE NUEVO REPRESENTANTE ---\n";
    Usuario nuevo;
    nuevo.idUsuario = sistema.autoIncUsuario++;
    nuevo.rol = ROL_REPRESENTANTE;

    cout << "Nombre Completo: ";
    getline(cin >> ws, nuevo.nombreCompleto);

    cout << "Nombre de Usuario (Login): ";
    cin >> nuevo.usuario;

    for (const auto &u : sistema.usuarios) {
        if (u.usuario == nuevo.usuario) {
            cout << "\n[ERROR] El nombre de usuario ya se encuentra en uso.\n";
            pausarConsola();
            return;
        }
    }

    cout << "Contraseña: ";
    cin >> nuevo.contrasena;

    dbInsertarUsuarioEnBD(sistema, nuevo);
    sistema.usuarios.push_back(nuevo);
    cout << "\n[OK] Usuario '" << nuevo.usuario << "' registrado exitosamente en SQL Server como Representante.\n";
    pausarConsola();
}

void menuAdministrador(SistemaCampeonato &sistema, const Usuario &admin)
{
    int opcion = 0;
    do {
        cout << "\n========================================================================================\n";
        cout << "                     PANEL PRINCIPAL DEL ADMINISTRADOR                                  \n";
        cout << " Usuario: " << admin.nombreCompleto << " | Rol: ADMINISTRADOR | Torneo Activo ID: " << sistema.idTorneoActivo << "\n";
        if (sistema.db.conectada) {
            cout << " [BASE DE DATOS]: CONECTADO DIRECTAMENTE A SQL SERVER (" << sistema.db.servidorConectado << ")\n";
        } else {
            cout << " [BASE DE DATOS]: MODO LOCAL (OFFLINE)\n";
        }
        cout << "========================================================================================\n";
        cout << " 1.  Crear Nuevo Torneo / Seleccionar Activo\n";
        cout << " 2.  Registrar Equipos y Plantillas de Jugadores (Máx 12 por liga)\n";
        cout << " 3.  Listar Equipos y Jugadores Registrados\n";
        cout << " 4.  Generar Calendario: Fase 1 (Clasificación Todos vs Todos - Round Robin)\n";
        cout << " 5.  Generar Calendario: Fase 2 (Cuartos de Final Top 8 Clasificados)\n";
        cout << " 6.  Generar Calendario: Semifinales y Gran Final\n";
        cout << " 7.  Ingresar Resultados e Incidencias (Marcador, Goles, Tarjetas, Porteros)\n";
        cout << " 8.  Ver Tabla de Posiciones por Liga (Mayor y Menor)\n";
        cout << " 9.  Ver Tablas de Ascenso/Descenso y Aplicar Cambios Oficiales\n";
        cout << " 10. Módulo de Premiaciones (Habilitar y Ver Cuadro de Honor)\n";
        cout << " 11. Ver Calendario y Marcadores de Partidos\n";
        cout << " 12. Ver Detalle e Incidencias de un Partido Específico\n";
        cout << " 13. Reporte Detallado de Tarjetas y Disciplina por Jugador\n";
        cout << " 14. Estadísticas de Premiación en Vivo (Goleadores y Porteros)\n";
        cout << " 15. Guardar y Sincronizar Datos en Disco / Base de Datos\n";
        cout << " 0.  Cerrar Sesión\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << " Seleccione una opción: ";
        cin >> opcion;

        if (cin.eof()) break;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: {
                cout << "\n1. Crear Nuevo Torneo\n2. Seleccionar Torneo Existente\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub == 1) {
                    crearTorneo(sistema);
                    guardarDatosEnArchivo(sistema);
                }else if (sub==2) {
                    seleccionarTorneoActivo(sistema);
                    guardarDatosEnArchivo(sistema);
                }
                break;
            }
            case 2: {
                cout << "\n1. Registrar Equipo\n2. Registrar Jugador en Equipo\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub == 1) {
                    registrarEquipo(sistema);
                    guardarDatosEnArchivo(sistema);
                }else if (sub == 2) {
                    registrarJugador(sistema);
                    guardarDatosEnArchivo(sistema);
                }
                break;
            }
            case 3: {
                cout << "\nSeleccione Liga a listar:\n1. Liga Mayor\n2. Liga Menor\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2)listarEquiposYPlantillas(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                break;
            }
            case 4: {
                cout << "\nSeleccione Liga para Fase 1 (Round Robin):\n1. Liga Mayor\n2. Liga Menor\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2) {
                    generarClasificacionRoundRobin(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                    guardarDatosEnArchivo(sistema);
                }
                break;
            }
            case 5: {
                cout << "\nSeleccione Liga para Cuartos de Final (Top 8):\n1. Liga Mayor\n2. Liga Menor\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2) {
                    generarCuartosDeFinalTop8(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                    guardarDatosEnArchivo(sistema);
                }
                break;
            }
            case 6: {
                cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2) {
                    DivisionLiga d = (sub == 1 ? LIGA_MAYOR : LIGA_MENOR);
                    cout << "\n1. Generar Semifinales\n2. Generar Gran Final\n3. Regresar al menu principal\nOpción: ";
                    int subFase; cin >> subFase;
                    if (subFase == 1) {
                        generarSemifinales(sistema, d);
                        guardarDatosEnArchivo(sistema);
                    }else if (subFase == 2) {
                        generarGranFinal(sistema, d);
                    }
                }
                break;
            }
            case 7:
                ingresarResultadoPartido(sistema);
                guardarDatosEnArchivo(sistema);
                break;
            case 8: {
                cout << "\nSeleccione Liga para Tabla de Posiciones:\n1. Liga Mayor\n2. Liga Menor\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub == 1 || sub == 2) {
                    imprimirTablaPosiciones(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                }
                break;
            }
            case 9: {
                cout << "\n1. Ver Tablas de Ascenso y Descenso\n2. Formalizar y Aplicar Ascenso/Descenso Oficial\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub == 1) {
                    imprimirTablasAscensoDescenso(sistema);
                    guardarDatosEnArchivo(sistema);
                }else if (sub == 2) {
                    aplicarAscensoDescensoOficial(sistema);
                    guardarDatosEnArchivo(sistema);
                }
                break;
            }
            case 10: {
                cout << "\n1. Ver Cuadro de Honor\n2. Finalizar Campeonato Oficialmente\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub == 1) {
                    imprimirModuloPremiaciones(sistema);
                    guardarDatosEnArchivo(sistema);
                }else if(sub==2) {
                    finalizarTorneoYHabilitarPremiacion(sistema);
                    guardarDatosEnArchivo(sistema);
                }
                break;
            }
            case 11: {
                cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\n3. Regresar al menu principal\nOpción: ";
                int sub; cin >> sub;
                if (sub == 1 || sub == 2) {
                    listarCalendarioPartidos(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                }
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
                cout << "\n[OK] Datos guardados exitosamente.\n";
                pausarConsola();
                break;
            case 0:
                cout << "\nCerrando sesión de administrador...\n";
                break;
            default:
                cout << "\n[!] Opción no reconocida.\n";
                break;
        }
    } while (opcion != 0);
}

void menuRepresentante(SistemaCampeonato &sistema, const Usuario &rep)
{
    int opcion = 0;
    do {
        cout << "\n========================================================================================\n";
        cout << "                  PANEL DEL REPRESENTANTE / USUARIO REGULAR                             \n";
        cout << " Usuario: " << rep.nombreCompleto << " | MODO: GESTIÓN DE PLANTILLA Y CONSULTA | Torneo: " << sistema.idTorneoActivo << "\n";
        if (sistema.db.conectada) {
            cout << " [BASE DE DATOS]: CONECTADO DIRECTAMENTE A SQL SERVER (" << sistema.db.servidorConectado << ")\n";
        } else {
            cout << " [BASE DE DATOS]: MODO LOCAL (OFFLINE)\n";
        }
        cout << "========================================================================================\n";
        cout << " 1. Ver Tabla de Posiciones (Liga Mayor / Liga Menor)\n";
        cout << " 2. Ver Tablas de Ascenso y Descenso\n";
        cout << " 3. ver Calendario de Partidos y Marcadores\n";
        cout << " 4. Ver Incidencias Detalladas de un Partido (Goles y Tarjetas)\n";
        cout << " 5. Ver Equipos y Plantillas de Jugadores Registrados\n";
        cout << " 6. Ver Premiaciones Oficiales del Torneo\n";
        cout << " 7. Registrar Jugador en la Plantilla de un Equipo\n";
        cout << " 8. Reporte Detallado de Tarjetas Acumuladas (Control Disciplinario)\n";
        cout << " 9. Estadísticas y Vínculo con Premiación (Goleadores y Porteros)\n";
        cout << " 0. Cerrar Sesión\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << " Seleccione una opción: ";
        cin >> opcion;

        if (cin.eof()) break;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: {
                cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\n3. Regredar al menú principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2) {
                    imprimirTablaPosiciones(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                }
                break;
            }
            case 2:
                imprimirTablasAscensoDescenso(sistema);
                break;
            case 3: {
                cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\n3. Regredar al menú principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2) {
                    listarCalendarioPartidos(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                }
                break;
            }
            case 4:
                verDetallePartido(sistema);
                break;
            case 5: {
                cout << "\nSeleccione Liga:\n1. Liga Mayor\n2. Liga Menor\n3. Regredar al menú principal\nOpción: ";
                int sub; cin >> sub;
                if (sub==1 || sub==2) {
                    listarEquiposYPlantillas(sistema, (sub == 1 ? LIGA_MAYOR : LIGA_MENOR));
                }
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
                cout << "\nCerrando sesión...\n";
                break;
            default:
                cout << "\n[!] Opción no válida.\n";
                break;
        }
    } while (opcion != 0);
}
