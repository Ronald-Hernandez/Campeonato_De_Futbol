/* =========================================================================
   SCRIPT SQL: INSTALACIÓN DE BASE DE DATOS BASE (ESQUEMA COMPLETO)
   SISTEMA: Campeonato de Fútbol
   COMPATIBILIDAD: SQL Server Express / SQL Server (Localhost)
   =========================================================================
   DESCRIPCIÓN:
   - Crea la base de datos 'CampeonatoFutbol'.
   - Crea las 15 tablas relacionales con restricciones PK, FK, CHECK y UNIQUE.
   - Crea índices optimizados para consultas de clasificación e incidencias.
   - Inserta catálogos iniciales obligatorios:
       * DIVISION: MAYOR (id=1), MENOR (id=2)
       * ROL: ADMINISTRADOR (id=1), REPRESENTANTE (id=2)
       * USUARIO: Usuario 'admin' (clave: 1234) y delegados base.
   - Crea vistas reglamentarias: vw_TABLA_POSICIONES y vw_TABLA_GOLEO.
   - Crea triggers oficiales: TR_TANDA_PENALES_SOLO_DESDE_CUARTOS.
   ========================================================================= */

USE master;
GO

IF DB_ID(N'CampeonatoFutbol') IS NULL
BEGIN
    CREATE DATABASE CampeonatoFutbol;
END;
GO

USE CampeonatoFutbol;
GO

/* =========================================================
   1. CATÁLOGOS Y SEGURIDAD
   ========================================================= */

IF OBJECT_ID('dbo.ROL', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.ROL
    (
        id_rol INT IDENTITY(1,1) NOT NULL,
        nombre_rol VARCHAR(30) NOT NULL,
        CONSTRAINT PK_ROL PRIMARY KEY (id_rol),
        CONSTRAINT UQ_ROL_nombre UNIQUE (nombre_rol),
        CONSTRAINT CK_ROL_nombre CHECK (nombre_rol IN ('ADMINISTRADOR', 'REPRESENTANTE'))
    );
END;
GO

IF OBJECT_ID('dbo.USUARIO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.USUARIO
    (
        id_usuario INT IDENTITY(1,1) NOT NULL,
        nombre_usuario VARCHAR(100) NOT NULL,
        usuario VARCHAR(50) NOT NULL,
        correo VARCHAR(150) NOT NULL,
        contrasena_hash VARCHAR(255) NOT NULL,
        id_rol INT NOT NULL CONSTRAINT DF_USUARIO_rol DEFAULT (2),
        activo BIT NOT NULL CONSTRAINT DF_USUARIO_activo DEFAULT (1),
        CONSTRAINT PK_USUARIO PRIMARY KEY (id_usuario),
        CONSTRAINT UQ_USUARIO_correo UNIQUE (correo),
        CONSTRAINT UQ_USUARIO_login UNIQUE (usuario),
        CONSTRAINT FK_USUARIO_ROL FOREIGN KEY (id_rol)
            REFERENCES dbo.ROL(id_rol)
    );
END;
GO

/* =========================================================
   2. CAMPEONATO, DIVISIONES, EQUIPOS Y JUGADORES
   ========================================================= */

IF OBJECT_ID('dbo.CAMPEONATO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.CAMPEONATO
    (
        id_campeonato INT IDENTITY(1,1) NOT NULL,
        nombre_campeonato VARCHAR(100) NOT NULL,
        fecha_inicio DATE NOT NULL,
        fecha_fin DATE NULL,
        max_equipos_mayor INT NOT NULL CONSTRAINT DF_CAMPEONATO_mayor DEFAULT (12),
        max_equipos_menor INT NOT NULL CONSTRAINT DF_CAMPEONATO_menor DEFAULT (12),
        CONSTRAINT PK_CAMPEONATO PRIMARY KEY (id_campeonato),
        CONSTRAINT CK_CAMPEONATO_fechas CHECK
        (
            fecha_fin IS NULL OR fecha_fin >= fecha_inicio
        ),
        CONSTRAINT CK_CAMPEONATO_max_mayor CHECK (max_equipos_mayor > 0),
        CONSTRAINT CK_CAMPEONATO_max_menor CHECK (max_equipos_menor > 0)
    );
END;
GO

IF OBJECT_ID('dbo.DIVISION', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.DIVISION
    (
        id_division INT IDENTITY(1,1) NOT NULL,
        nombre_division VARCHAR(50) NOT NULL,
        CONSTRAINT PK_DIVISION PRIMARY KEY (id_division),
        CONSTRAINT UQ_DIVISION_nombre UNIQUE (nombre_division),
        CONSTRAINT CK_DIVISION_nombre CHECK (nombre_division IN ('MAYOR', 'MENOR'))
    );
END;
GO

IF OBJECT_ID('dbo.EQUIPO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.EQUIPO
    (
        id_equipo INT IDENTITY(1,1) NOT NULL,
        nombre_equipo VARCHAR(100) NOT NULL,
        id_division INT NOT NULL,
        id_campeonato INT NOT NULL,
        CONSTRAINT PK_EQUIPO PRIMARY KEY (id_equipo),
        CONSTRAINT UQ_EQUIPO_campeonato_nombre UNIQUE
        (
            id_campeonato,
            nombre_equipo
        ),
        CONSTRAINT FK_EQUIPO_DIVISION FOREIGN KEY (id_division)
            REFERENCES dbo.DIVISION(id_division),
        CONSTRAINT FK_EQUIPO_CAMPEONATO FOREIGN KEY (id_campeonato)
            REFERENCES dbo.CAMPEONATO(id_campeonato)
    );
END;
GO

IF OBJECT_ID('dbo.JUGADOR', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.JUGADOR
    (
        id_jugador INT IDENTITY(1,1) NOT NULL,
        id_equipo INT NOT NULL,
        nombre VARCHAR(100) NOT NULL,
        apellido VARCHAR(100) NOT NULL,
        posicion VARCHAR(50) NULL,
        numero_camiseta INT NULL,
        CONSTRAINT PK_JUGADOR PRIMARY KEY (id_jugador),
        CONSTRAINT FK_JUGADOR_EQUIPO FOREIGN KEY (id_equipo)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT CK_JUGADOR_numero CHECK
        (
            numero_camiseta IS NULL OR numero_camiseta BETWEEN 0 AND 99
        ),
        CONSTRAINT UQ_JUGADOR_equipo_camiseta UNIQUE
        (
            id_equipo,
            numero_camiseta
        )
    );
END;
GO

IF OBJECT_ID('dbo.USUARIO_EQUIPO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.USUARIO_EQUIPO
    (
        id_usuario_equipo INT IDENTITY(1,1) NOT NULL,
        id_usuario INT NOT NULL,
        id_equipo INT NOT NULL,
        id_rol INT NOT NULL,
        fecha_asignacion DATE NOT NULL
            CONSTRAINT DF_USUARIO_EQUIPO_fecha DEFAULT (CONVERT(DATE, GETDATE())),
        CONSTRAINT PK_USUARIO_EQUIPO PRIMARY KEY (id_usuario_equipo),
        CONSTRAINT FK_USUARIO_EQUIPO_USUARIO FOREIGN KEY (id_usuario)
            REFERENCES dbo.USUARIO(id_usuario),
        CONSTRAINT FK_USUARIO_EQUIPO_EQUIPO FOREIGN KEY (id_equipo)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT FK_USUARIO_EQUIPO_ROL FOREIGN KEY (id_rol)
            REFERENCES dbo.ROL(id_rol),
        CONSTRAINT UQ_USUARIO_EQUIPO_ROL UNIQUE
        (
            id_usuario,
            id_equipo,
            id_rol
        )
    );
END;
GO

/* =========================================================
   3. PARTIDOS Y CONTROL DE PUNTOS
   ========================================================= */

IF OBJECT_ID('dbo.PARTIDO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.PARTIDO
    (
        id_partido INT IDENTITY(1,1) NOT NULL,
        id_campeonato INT NOT NULL,
        id_division INT NOT NULL,
        fecha_partido DATETIME2(0) NOT NULL,
        jornada INT NOT NULL,
        fase VARCHAR(50) NOT NULL,
        CONSTRAINT PK_PARTIDO PRIMARY KEY (id_partido),
        CONSTRAINT FK_PARTIDO_CAMPEONATO FOREIGN KEY (id_campeonato)
            REFERENCES dbo.CAMPEONATO(id_campeonato),
        CONSTRAINT FK_PARTIDO_DIVISION FOREIGN KEY (id_division)
            REFERENCES dbo.DIVISION(id_division),
        CONSTRAINT CK_PARTIDO_jornada CHECK (jornada > 0),
        CONSTRAINT CK_PARTIDO_fase CHECK
        (
            fase IN
            (
                'CLASIFICACION',
                'OCTAVOS',
                'CUARTOS',
                'SEMIFINAL',
                'FINAL'
            )
        )
    );
END;
GO

IF OBJECT_ID('dbo.DETALLE_PARTIDO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.DETALLE_PARTIDO
    (
        id_detalle INT IDENTITY(1,1) NOT NULL,
        id_partido INT NOT NULL,
        id_equipo INT NOT NULL,
        goles_favor INT NOT NULL CONSTRAINT DF_DETALLE_goles_favor DEFAULT (0),
        goles_contra INT NOT NULL CONSTRAINT DF_DETALLE_goles_contra DEFAULT (0),
        puntos_ganados INT NOT NULL CONSTRAINT DF_DETALLE_puntos DEFAULT (0),
        resultado VARCHAR(20) NOT NULL,
        CONSTRAINT PK_DETALLE_PARTIDO PRIMARY KEY (id_detalle),
        CONSTRAINT FK_DETALLE_PARTIDO_PARTIDO FOREIGN KEY (id_partido)
            REFERENCES dbo.PARTIDO(id_partido),
        CONSTRAINT FK_DETALLE_PARTIDO_EQUIPO FOREIGN KEY (id_equipo)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT UQ_DETALLE_PARTIDO_EQUIPO UNIQUE (id_partido, id_equipo),
        CONSTRAINT CK_DETALLE_goles CHECK (goles_favor >= 0 AND goles_contra >= 0),
        CONSTRAINT CK_DETALLE_puntos CHECK (puntos_ganados >= 0),
        CONSTRAINT CK_DETALLE_resultado CHECK
        (
            resultado IN ('VICTORIA', 'EMPATE', 'DERROTA', 'PENDIENTE')
        )
    );
END;
GO

/* =========================================================
   4. TARJETAS, GOLES Y PORTEROS
   ========================================================= */

IF OBJECT_ID('dbo.TARJETAS_ACUMULADAS', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.TARJETAS_ACUMULADAS
    (
        id_tarjeta INT IDENTITY(1,1) NOT NULL,
        id_partido INT NOT NULL,
        id_jugador INT NOT NULL,
        tipo_tarjeta VARCHAR(20) NOT NULL,
        minuto INT NOT NULL,
        CONSTRAINT PK_TARJETAS_ACUMULADAS PRIMARY KEY (id_tarjeta),
        CONSTRAINT FK_TARJETAS_PARTIDO FOREIGN KEY (id_partido)
            REFERENCES dbo.PARTIDO(id_partido),
        CONSTRAINT FK_TARJETAS_JUGADOR FOREIGN KEY (id_jugador)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT CK_TARJETAS_tipo CHECK
        (
            tipo_tarjeta IN ('AMARILLA', 'ROJA')
        ),
        CONSTRAINT CK_TARJETAS_minuto CHECK (minuto BETWEEN 0 AND 130)
    );
END;
GO

IF OBJECT_ID('dbo.REGISTRO_GOLES', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.REGISTRO_GOLES
    (
        id_gol INT IDENTITY(1,1) NOT NULL,
        id_partido INT NOT NULL,
        id_jugador INT NOT NULL,
        id_portero_vencido INT NULL,
        id_equipo_rival INT NOT NULL,
        minuto INT NOT NULL,
        es_autogol BIT NOT NULL CONSTRAINT DF_GOLES_autogol DEFAULT (0),
        CONSTRAINT PK_REGISTRO_GOLES PRIMARY KEY (id_gol),
        CONSTRAINT FK_GOLES_PARTIDO FOREIGN KEY (id_partido)
            REFERENCES dbo.PARTIDO(id_partido),
        CONSTRAINT FK_GOLES_JUGADOR FOREIGN KEY (id_jugador)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT FK_GOLES_PORTERO FOREIGN KEY (id_portero_vencido)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT FK_GOLES_EQUIPO_RIVAL FOREIGN KEY (id_equipo_rival)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT CK_GOLES_minuto CHECK (minuto BETWEEN 0 AND 130)
    );
END;
GO

IF OBJECT_ID('dbo.CONTROL_PORTEROS', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.CONTROL_PORTEROS
    (
        id_control_portero INT IDENTITY(1,1) NOT NULL,
        id_partido INT NOT NULL,
        id_jugador INT NOT NULL,
        id_equipo INT NOT NULL,
        minuto_entrada INT NOT NULL,
        minuto_salida INT NULL,
        goles_recibidos INT NOT NULL CONSTRAINT DF_PORTEROS_goles DEFAULT (0),
        CONSTRAINT PK_CONTROL_PORTEROS PRIMARY KEY (id_control_portero),
        CONSTRAINT FK_PORTEROS_PARTIDO FOREIGN KEY (id_partido)
            REFERENCES dbo.PARTIDO(id_partido),
        CONSTRAINT FK_PORTEROS_JUGADOR FOREIGN KEY (id_jugador)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT FK_PORTEROS_EQUIPO FOREIGN KEY (id_equipo)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT CK_PORTEROS_minutos CHECK
        (
            minuto_entrada >= 0 AND
            (minuto_salida IS NULL OR minuto_salida >= minuto_entrada)
        ),
        CONSTRAINT CK_PORTEROS_goles CHECK (goles_recibidos >= 0)
    );
END;
GO

IF OBJECT_ID('dbo.TANDA_PENALES', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.TANDA_PENALES
    (
        id_penal INT IDENTITY(1,1) NOT NULL,
        id_partido INT NOT NULL,
        id_jugador INT NOT NULL,
        id_equipo INT NOT NULL,
        anotado BIT NOT NULL,
        CONSTRAINT PK_TANDA_PENALES PRIMARY KEY (id_penal),
        CONSTRAINT FK_PENALES_PARTIDO FOREIGN KEY (id_partido)
            REFERENCES dbo.PARTIDO(id_partido),
        CONSTRAINT FK_PENALES_JUGADOR FOREIGN KEY (id_jugador)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT FK_PENALES_EQUIPO FOREIGN KEY (id_equipo)
            REFERENCES dbo.EQUIPO(id_equipo)
    );
END;
GO

/* =========================================================
   5. PREMIACIÓN Y ASCENSO/DESCENSO
   ========================================================= */

IF OBJECT_ID('dbo.PREMIACION', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.PREMIACION
    (
        id_premiacion INT IDENTITY(1,1) NOT NULL,
        id_campeonato INT NOT NULL,
        id_jugador_goleador INT NOT NULL,
        id_portero_menos_vencido INT NOT NULL,
        id_equipo_campeon INT NULL,
        CONSTRAINT PK_PREMIACION PRIMARY KEY (id_premiacion),
        CONSTRAINT FK_PREMIACION_CAMPEONATO FOREIGN KEY (id_campeonato)
            REFERENCES dbo.CAMPEONATO(id_campeonato),
        CONSTRAINT FK_PREMIACION_GOLEADOR FOREIGN KEY (id_jugador_goleador)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT FK_PREMIACION_PORTERO FOREIGN KEY (id_portero_menos_vencido)
            REFERENCES dbo.JUGADOR(id_jugador),
        CONSTRAINT FK_PREMIACION_CAMPEON FOREIGN KEY (id_equipo_campeon)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT UQ_PREMIACION_CAMPEONATO UNIQUE (id_campeonato)
    );
END;
GO

IF OBJECT_ID('dbo.HISTORIAL_ASCENSO_DESCENSO', 'U') IS NULL
BEGIN
    CREATE TABLE dbo.HISTORIAL_ASCENSO_DESCENSO
    (
        id_historial INT IDENTITY(1,1) NOT NULL,
        id_campeonato INT NOT NULL,
        id_equipo INT NOT NULL,
        id_division_origen INT NOT NULL,
        id_division_destino INT NOT NULL,
        tipo_movimiento VARCHAR(20) NOT NULL,
        CONSTRAINT PK_HISTORIAL_ASCENSO_DESCENSO PRIMARY KEY (id_historial),
        CONSTRAINT FK_HISTORIAL_CAMPEONATO FOREIGN KEY (id_campeonato)
            REFERENCES dbo.CAMPEONATO(id_campeonato),
        CONSTRAINT FK_HISTORIAL_EQUIPO FOREIGN KEY (id_equipo)
            REFERENCES dbo.EQUIPO(id_equipo),
        CONSTRAINT FK_HISTORIAL_DIVISION_ORIGEN FOREIGN KEY (id_division_origen)
            REFERENCES dbo.DIVISION(id_division),
        CONSTRAINT FK_HISTORIAL_DIVISION_DESTINO FOREIGN KEY (id_division_destino)
            REFERENCES dbo.DIVISION(id_division),
        CONSTRAINT CK_HISTORIAL_tipo CHECK
        (
            tipo_movimiento IN ('ASCENSO', 'DESCENSO')
        ),
        CONSTRAINT CK_HISTORIAL_divisiones CHECK
        (
            id_division_origen <> id_division_destino
        )
    );
END;
GO

/* =========================================================
   6. ÍNDICES DE RENDIMIENTO
   ========================================================= */

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_EQUIPO_DIVISION' AND object_id = OBJECT_ID('dbo.EQUIPO'))
    CREATE INDEX IX_EQUIPO_DIVISION ON dbo.EQUIPO(id_division);

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_EQUIPO_CAMPEONATO' AND object_id = OBJECT_ID('dbo.EQUIPO'))
    CREATE INDEX IX_EQUIPO_CAMPEONATO ON dbo.EQUIPO(id_campeonato);

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_JUGADOR_EQUIPO' AND object_id = OBJECT_ID('dbo.JUGADOR'))
    CREATE INDEX IX_JUGADOR_EQUIPO ON dbo.JUGADOR(id_equipo);

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_PARTIDO_CAMPEONATO_FECHA' AND object_id = OBJECT_ID('dbo.PARTIDO'))
    CREATE INDEX IX_PARTIDO_CAMPEONATO_FECHA ON dbo.PARTIDO(id_campeonato, fecha_partido);

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_DETALLE_PARTIDO_EQUIPO' AND object_id = OBJECT_ID('dbo.DETALLE_PARTIDO'))
    CREATE INDEX IX_DETALLE_PARTIDO_EQUIPO ON dbo.DETALLE_PARTIDO(id_equipo);

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_GOLES_JUGADOR' AND object_id = OBJECT_ID('dbo.REGISTRO_GOLES'))
    CREATE INDEX IX_GOLES_JUGADOR ON dbo.REGISTRO_GOLES(id_jugador);

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_TARJETAS_JUGADOR' AND object_id = OBJECT_ID('dbo.TARJETAS_ACUMULADAS'))
    CREATE INDEX IX_TARJETAS_JUGADOR ON dbo.TARJETAS_ACUMULADAS(id_jugador);
GO

/* =========================================================
   7. DATOS INICIALES (CATÁLOGOS Y USUARIOS BASE)
   ========================================================= */

-- Catálogo de Divisiones
IF NOT EXISTS (SELECT 1 FROM dbo.DIVISION WHERE nombre_division = 'MAYOR')
    INSERT INTO dbo.DIVISION (nombre_division) VALUES ('MAYOR');

IF NOT EXISTS (SELECT 1 FROM dbo.DIVISION WHERE nombre_division = 'MENOR')
    INSERT INTO dbo.DIVISION (nombre_division) VALUES ('MENOR');

-- Catálogo de Roles
IF NOT EXISTS (SELECT 1 FROM dbo.ROL WHERE nombre_rol = 'ADMINISTRADOR')
    INSERT INTO dbo.ROL (nombre_rol) VALUES ('ADMINISTRADOR');

IF NOT EXISTS (SELECT 1 FROM dbo.ROL WHERE nombre_rol = 'REPRESENTANTE')
    INSERT INTO dbo.ROL (nombre_rol) VALUES ('REPRESENTANTE');

-- Usuarios Base Iniciales
IF NOT EXISTS (SELECT 1 FROM dbo.USUARIO WHERE usuario = 'admin')
BEGIN
    INSERT INTO dbo.USUARIO (nombre_usuario, usuario, correo, contrasena_hash, id_rol, activo)
    VALUES ('Administrador General', 'admin', 'admin@campeonato.com', '1234', 1, 1);
END;

IF NOT EXISTS (SELECT 1 FROM dbo.USUARIO WHERE usuario = 'jperez')
BEGIN
    INSERT INTO dbo.USUARIO (nombre_usuario, usuario, correo, contrasena_hash, id_rol, activo)
    VALUES ('Juan Pérez', 'jperez', 'jperez@campeonato.com', '1234', 2, 1);
END;

IF NOT EXISTS (SELECT 1 FROM dbo.USUARIO WHERE usuario = 'hernandezr')
BEGIN
    INSERT INTO dbo.USUARIO (nombre_usuario, usuario, correo, contrasena_hash, id_rol, activo)
    VALUES ('Ronald Hernández', 'hernandezr', 'hernandezr@campeonato.com', '1234', 2, 1);
END;
GO

/* =========================================================
   8. VISTAS DE CONSULTA
   ========================================================= */

CREATE OR ALTER VIEW dbo.vw_TABLA_GOLEO
AS
SELECT
    j.id_jugador,
    CONCAT(j.nombre, ' ', j.apellido) AS jugador,
    e.id_equipo,
    e.nombre_equipo,
    COUNT(g.id_gol) AS goles
FROM dbo.JUGADOR AS j
INNER JOIN dbo.EQUIPO AS e
    ON e.id_equipo = j.id_equipo
LEFT JOIN dbo.REGISTRO_GOLES AS g
    ON g.id_jugador = j.id_jugador
   AND g.es_autogol = 0
GROUP BY
    j.id_jugador,
    j.nombre,
    j.apellido,
    e.id_equipo,
    e.nombre_equipo;
GO

CREATE OR ALTER VIEW dbo.vw_TABLA_POSICIONES
AS
SELECT
    e.id_equipo,
    e.nombre_equipo,
    e.id_division,
    e.id_campeonato,
    COALESCE(SUM(dp.puntos_ganados), 0) AS puntos,
    COALESCE(SUM(dp.goles_favor), 0) AS goles_favor,
    COALESCE(SUM(dp.goles_contra), 0) AS goles_contra,
    COALESCE(SUM(dp.goles_favor), 0)
      - COALESCE(SUM(dp.goles_contra), 0) AS diferencia_goles
FROM dbo.EQUIPO AS e
LEFT JOIN dbo.DETALLE_PARTIDO AS dp
    ON dp.id_equipo = e.id_equipo
GROUP BY
    e.id_equipo,
    e.nombre_equipo,
    e.id_division,
    e.id_campeonato;
GO

/* =========================================================
   9. TRIGGER: REGLAS DE NEGOCIO EN FASES FINALES
   ========================================================= */

CREATE OR ALTER TRIGGER dbo.TR_TANDA_PENALES_SOLO_DESDE_CUARTOS
ON dbo.TANDA_PENALES
AFTER INSERT, UPDATE
AS
BEGIN
    SET NOCOUNT ON;

    IF EXISTS
    (
        SELECT 1
        FROM inserted AS i
        INNER JOIN dbo.PARTIDO AS p
            ON p.id_partido = i.id_partido
        WHERE p.fase NOT IN ('CUARTOS', 'SEMIFINAL', 'FINAL')
    )
    BEGIN
        THROW 50001,
              'La tanda de penales solo puede registrarse desde CUARTOS de final.',
              1;
    END;
END;
GO

PRINT '===============================================================';
PRINT '  Base de datos CampeonatoFutbol instalada correctamente.      ';
PRINT '  Tablas, vistas, índices, catálogos y usuarios listos.        ';
PRINT '===============================================================';
GO
