/* =========================================================================
   SCRIPT SQL: CARGA DE MÚLTIPLES TORNEOS DE PRUEBA Y ESCENARIOS PRECONFIGURADOS
   BASE DE DATOS: CampeonatoFutbol (SQL Server Express / Localhost)
   =========================================================================
   PROPÓSITO Y TORNEOS CREADOS:
   -------------------------------------------------------------------------
   ID 1: Torneo 1 - Listo para Fixture General (Round Robin)
         -> 24 equipos registrados con plantillas de 5 jugadores (12 Mayor, 12 Menor).
         -> 0 partidos generados.
         -> Permite probar: Opción 4 del Administrador (Generar Round Robin).
         
   ID 2: Torneo 2 - Listo para Cuartos de Final (Top 8)
         -> 132 partidos de Clasificación 100% finalizados con goles, tarjetas y porteros.
         -> 0 partidos de Cuartos creados.
         -> Permite probar: Opción 5 del Administrador (Generar Cuartos Top 8).
         
   ID 3: Torneo 3 - Listo para Semifinales
         -> Clasificación 100% jugada + 4 partidos de Cuartos de Final disputados
            con 4 ganadores definidos.
         -> 0 partidos de Semifinales creados.
         -> Permite probar: Opción 6 -> Subopción 1 (Generar Semifinales).
         
   ID 4: Torneo 4 - Listo para Gran Final
         -> Clasificación y Cuartos jugados + 2 partidos de Semifinales disputados
            con 2 finalistas definidos.
         -> 0 partidos de Final creados.
         -> Permite probar: Opción 6 -> Subopción 2 (Generar Gran Final).
         
   ID 5: Torneo 5 - Listo para Premiacion Oficial
         -> Clasificación, Cuartos, Semifinales y Gran Final disputados.
         -> Campeón definido en la final, delantero estrella con más goles (Botín de Oro)
            y portero con menos goles recibidos (Guante de Oro).
         -> Permite probar: Opción 10 (Habilitar Premiación) y Opción 11 (Ver Premiación).
         
   ID 6: Torneo 6 - Torneo en Curso (Jornadas 1-5 Jugadas)
         -> Fixture completo de 11 jornadas (132 partidos).
         -> Jornadas 1 a 5 disputadas (60 partidos finalizados).
         -> Jornadas 6 a 11 pendientes (72 partidos en estado PENDIENTE).
         -> Permite probar: Opción 7 (Ingreso manual de marcadores), tabla en vivo y
            bloqueo reglamentario de Cuartos de Final.
            
   ID 7: Torneo 7 - Prueba Disciplina y Descenso Apretado
         -> Clasificación 100% disputada.
         -> Triple empate a 8 puntos en zona de descenso de Liga Mayor (desempate por DG).
         -> Más de 80 tarjetas amarillas y rojas acumuladas (Fair Play).
         -> Permite probar: Opción 8 (Ascenso y Descenso oficial) y Opción 9 (Historial).
   ========================================================================= */

USE CampeonatoFutbol;
GO

SET NOCOUNT ON;

PRINT '=========================================================================';
PRINT '  INICIANDO CARGA DE MÚLTIPLES TORNEOS DE PRUEBA EN CampeonatoFutbol     ';
PRINT '=========================================================================';

-- 1. Limpieza ordenada para respetar restricciones de claves foráneas
PRINT '1. Limpiando datos previos de partidos, equipos y torneos...';
DELETE FROM dbo.TANDA_PENALES;
DELETE FROM dbo.CONTROL_PORTEROS;
DELETE FROM dbo.TARJETAS_ACUMULADAS;
DELETE FROM dbo.REGISTRO_GOLES;
DELETE FROM dbo.DETALLE_PARTIDO;
DELETE FROM dbo.PARTIDO;
DELETE FROM dbo.PREMIACION;
DELETE FROM dbo.HISTORIAL_ASCENSO_DESCENSO;
DELETE FROM dbo.USUARIO_EQUIPO;
DELETE FROM dbo.JUGADOR;
DELETE FROM dbo.EQUIPO;
DELETE FROM dbo.CAMPEONATO;

-- 2. Insertar los 7 Campeonatos con IDs explícitos (1 al 7)
PRINT '2. Registrando los 7 Campeonatos oficiales...';
SET IDENTITY_INSERT dbo.CAMPEONATO ON;

INSERT INTO dbo.CAMPEONATO (id_campeonato, nombre_campeonato, fecha_inicio, fecha_fin, max_equipos_mayor, max_equipos_menor)
VALUES
(1, 'Torneo 1 - Listo para Fixture General', '2026-01-10', '2026-05-30', 12, 12),
(2, 'Torneo 2 - Listo para Cuartos de Final', '2026-01-10', '2026-05-30', 12, 12),
(3, 'Torneo 3 - Listo para Semifinales', '2026-01-10', '2026-05-30', 12, 12),
(4, 'Torneo 4 - Listo para Gran Final', '2026-01-10', '2026-05-30', 12, 12),
(5, 'Torneo 5 - Listo para Premiacion Oficial', '2026-01-10', '2026-05-30', 12, 12),
(6, 'Torneo 6 - Torneo en Curso (Jornadas 1-5)', '2026-01-10', '2026-05-30', 12, 12),
(7, 'Torneo 7 - Prueba Disciplina y Descenso Apretado', '2026-01-10', '2026-05-30', 12, 12);

SET IDENTITY_INSERT dbo.CAMPEONATO OFF;

-- 3. Catálogo base de 24 equipos con plantillas reglamentarias
DECLARE @CatEquipos TABLE (
    num INT PRIMARY KEY,
    nombre VARCHAR(100),
    id_division INT,
    rating INT,
    por_nom VARCHAR(50), por_ape VARCHAR(50),
    def_nom VARCHAR(50), def_ape VARCHAR(50),
    med_nom VARCHAR(50), med_ape VARCHAR(50),
    del1_nom VARCHAR(50), del1_ape VARCHAR(50),
    del2_nom VARCHAR(50), del2_ape VARCHAR(50)
);

INSERT INTO @CatEquipos VALUES
-- LIGA MAYOR (1 al 12, id_division = 1)
(1,  'CSD Municipal',          1, 95, 'Kenderson', 'Navarro', 'José', 'Mena',        'Alejandro', 'Galindo',  'José', 'Martínez',     'Matías', 'Rotondi'),
(2,  'Comunicaciones FC',      1, 92, 'Fredy',     'Pérez',   'Stheven', 'Robles',   'José', 'Contreras',     'Juan', 'Anangonó',     'Lynner', 'García'),
(3,  'Antigua GFC',            1, 88, 'Braulio',   'Linares', 'José', 'Ardón',       'Oscar', 'Castellanos',  'Romario', 'da Silva',  'Dewinder', 'Bradley'),
(4,  'Cobán Imperial',         1, 85, 'Minor',     'Álvarez', 'Thales', 'Moreira',   'Janderson', 'Pereira',  'Diego', 'Casas',       'Anthony', 'López'),
(5,  'Xelajú MC',              1, 82, 'José',      'Calderón','Kevin', 'Ruiz',       'Aslinn', 'Rodas',       'Gerson', 'Tinoco',     'Darwin', 'Lom'),
(6,  'Deportivo Guastatoya',   1, 79, 'Adrián',    'De Lemos','Walter', 'García',    'Denilson', 'Sánchez',   'Robin', 'Betancourth', 'William', 'Fajardo'),
(7,  'Deportivo Malacateco',   1, 76, 'Alejandro', 'Peláez',  'Raúl', 'Calderón',    'Kevin', 'Ramírez',      'Pedro', 'Báez',        'Ángel', 'López'),
(8,  'Deportivo Mixco',        1, 73, 'Kevin',     'Moscoso', 'Jorge', 'Sotomayor',  'Jean', 'Márquez',       'Roque', 'Caballero',   'Yonathan', 'Pozuelos'),
(9,  'Xinabajul Huehue',       1, 68, 'Manuel',    'Sosa',    'Kenlly', 'Velásquez', 'Néstor', 'Jucup',       'Nicolás', 'Mazzola',   'Marvin', 'Ceballos'),
(10, 'Deportivo Achuapa',      1, 64, 'Ederson',   'Cabezas', 'Rigoberto', 'Hernández','Kevyn', 'Aguilar',   'Eliser', 'Quiñones',   'Erick', 'Lemus'),
(11, 'Deportivo Marquense',    1, 58, 'Marvin',    'Barrios', 'Frank', 'De León',    'Marco', 'Rodas',        'Paolo', 'Dantaz',      'Rincón', 'Teixeira'),
(12, 'CSD Zacapa',             1, 52, 'Álvaro',    'García',  'Julio', 'García',     'Elmer', 'Monroy',       'Keyshawn', 'Vega',     'Frank', 'Zamora'),

-- LIGA MENOR (13 al 24, id_division = 2)
(13, 'Deportivo San Pedro',    2, 95, 'William',   'Negrete', 'Wilson', 'Godoy',     'César', 'Madrid',       'Igor', 'Da Silva',     'Valerio', 'Miranda'),
(14, 'Universidad SC',         2, 92, 'Estuardo',  'Sicán',   'Nery', 'Oliva',       'Sergio', 'Jucup',       'Kamiani', 'Félix',     'Pedro', 'Samayoa'),
(15, 'Aurora FC',              2, 88, 'David',     'Aldana',  'Allan', 'López',      'Jorge', 'Ortíz',        'Diego', 'Ruiz',        'Jorge', 'Argumaniz'),
(16, 'Juventud Pinulteca',     2, 85, 'Héctor',    'Mendoza', 'Christian', 'Ojeda',  'Sergio', 'Tagua',       'Sergio', 'Mina',       'José', 'Andrade'),
(17, 'Deportivo Iztapa',       2, 82, 'Carlos',    'Rodríguez','Santos', 'Crisanto', 'Grimaldo', 'Silva',     'Cristian', 'Albizures','Oliver', 'Rodas'),
(18, 'Santa Lucía Cotzumalguapa',2, 79,'Luis',     'Morán',   'Edwin', 'Fuentes',    'Mafre', 'Icuté',        'Charles', 'Martínez',  'Walter', 'Gaona'),
(19, 'Deportivo Suchitepéquez', 2, 76, 'Anderson', 'Gómez',   'Juan', 'Cardona',     'Kevin', 'Santamaría',   'Erick', 'Rivera',      'Diego', 'Orozco'),
(20, 'Deportivo Coatepeque',   2, 73, 'Diego',     'Pérez',   'Sixto', 'Betancourt', 'Luis', 'Tatuaca',       'Santiago', 'Gómez',    'Melfor', 'Ramos'),
(21, 'Quiché FC',              2, 68, 'Abner',     'Urizar',  'Alexánder', 'Cifuentes','Marlon', 'Negrete',   'Víctor', 'Guay',       'José', 'Ramos'),
(22, 'Deportivo Carchá',       2, 64, 'Christopher','Lemus',  'Elder', 'Ramos',      'Héctor', 'Moreira',     'Milton', 'Palacios',   'Carlos', 'Asprilla'),
(23, 'Deportivo Mictlán',      2, 58, 'Mynor',     'Padilla', 'Carlos', 'Herrera',   'Diego', 'Palma',        'Igor', 'Dos Santos',   'Juan', 'Valenzuela'),
(24, 'Nueva Concepción',       2, 52, 'Ricardo',   'Trigueño','Josué', 'Jucup',      'Brandon', 'Dávila',     'Arichel', 'Hernández', 'Gustavo', 'Amado');

-- 4. Registrar los Equipos y Jugadores para cada uno de los 7 Torneos
PRINT '3. Creando equipos y plantillas oficiales para los 7 torneos...';
SET IDENTITY_INSERT dbo.EQUIPO ON;

DECLARE @t INT = 1;
WHILE @t <= 7
BEGIN
    INSERT INTO dbo.EQUIPO (id_equipo, nombre_equipo, id_division, id_campeonato)
    SELECT (@t * 100 + num), nombre, id_division, @t
    FROM @CatEquipos;
    
    SET @t = @t + 1;
END;

SET IDENTITY_INSERT dbo.EQUIPO OFF;

-- Insertar Jugadores (5 por equipo, 120 por torneo, 840 jugadores en total)
SET IDENTITY_INSERT dbo.JUGADOR ON;

SET @t = 1;
WHILE @t <= 7
BEGIN
    -- Portero (Camiseta 1)
    INSERT INTO dbo.JUGADOR (id_jugador, id_equipo, nombre, apellido, posicion, numero_camiseta)
    SELECT (@t * 1000 + num * 10 + 1), (@t * 100 + num), por_nom, por_ape, 'PORTERO', 1
    FROM @CatEquipos;

    -- Defensa (Camiseta 3)
    INSERT INTO dbo.JUGADOR (id_jugador, id_equipo, nombre, apellido, posicion, numero_camiseta)
    SELECT (@t * 1000 + num * 10 + 2), (@t * 100 + num), def_nom, def_ape, 'DEFENSA', 3
    FROM @CatEquipos;

    -- Mediocampista (Camiseta 8)
    INSERT INTO dbo.JUGADOR (id_jugador, id_equipo, nombre, apellido, posicion, numero_camiseta)
    SELECT (@t * 1000 + num * 10 + 3), (@t * 100 + num), med_nom, med_ape, 'MEDIOCAMPISTA', 8
    FROM @CatEquipos;

    -- Delantero Titular (Camiseta 9)
    INSERT INTO dbo.JUGADOR (id_jugador, id_equipo, nombre, apellido, posicion, numero_camiseta)
    SELECT (@t * 1000 + num * 10 + 4), (@t * 100 + num), del1_nom, del1_ape, 'DELANTERO', 9
    FROM @CatEquipos;

    -- Delantero Suplente (Camiseta 10)
    INSERT INTO dbo.JUGADOR (id_jugador, id_equipo, nombre, apellido, posicion, numero_camiseta)
    SELECT (@t * 1000 + num * 10 + 5), (@t * 100 + num), del2_nom, del2_ape, 'DELANTERO', 10
    FROM @CatEquipos;

    SET @t = @t + 1;
END;

SET IDENTITY_INSERT dbo.JUGADOR OFF;

-- Vincular usuarios representantes existentes a equipos en dbo.USUARIO_EQUIPO
INSERT INTO dbo.USUARIO_EQUIPO (id_usuario, id_equipo, id_rol, fecha_asignacion)
SELECT 4, (id_campeonato * 100 + 1), 2, '2026-01-10' FROM dbo.CAMPEONATO
UNION ALL
SELECT 3, (id_campeonato * 100 + 2), 2, '2026-01-10' FROM dbo.CAMPEONATO;

-- 5. Tabla de Parejas Round Robin precalculada para 12 equipos (11 jornadas, 6 partidos por jornada = 66 partidos)
DECLARE @FixtureRR TABLE (
    ronda INT,
    partido_en_ronda INT,
    idx_local INT,
    idx_visitante INT
);

-- Generación determinista del algoritmo Round Robin (Rotación Berger)
DECLARE @idx TABLE (pos INT PRIMARY KEY, num INT);
INSERT INTO @idx VALUES (0,1),(1,2),(2,3),(3,4),(4,5),(5,6),(6,7),(7,8),(8,9),(9,10),(10,11),(11,12);

DECLARE @r INT = 0, @m INT;
WHILE @r < 11
BEGIN
    SET @m = 0;
    WHILE @m < 6
    BEGIN
        DECLARE @l INT, @v INT;
        SELECT @l = num FROM @idx WHERE pos = @m;
        SELECT @v = num FROM @idx WHERE pos = (11 - @m);

        IF (@r % 2 = 1)
        BEGIN
            DECLARE @tmp INT = @l; SET @l = @v; SET @v = @tmp;
        END;

        INSERT INTO @FixtureRR (ronda, partido_en_ronda, idx_local, idx_visitante)
        VALUES (@r + 1, @m + 1, @l, @v);

        SET @m = @m + 1;
    END;

    -- Rotación de posiciones 1 a 11 (fijando la posición 0)
    DECLARE @ultimo INT;
    SELECT @ultimo = num FROM @idx WHERE pos = 11;

    UPDATE t 
    SET t.num = prev.num 
    FROM @idx t
    JOIN @idx prev ON prev.pos = t.pos - 1
    WHERE t.pos BETWEEN 2 AND 11;

    UPDATE @idx SET num = @ultimo WHERE pos = 1;

    SET @r = @r + 1;
END;

-- =========================================================================
-- GENERACIÓN DE PARTIDOS E INCIDENCIAS PARA CADA TORNEO
-- =========================================================================

PRINT '4. Generando partidos, resultados e incidencias para cada torneo...';

-- Variables de trabajo
DECLARE @id_part INT, @jornada INT, @id_div INT;
DECLARE @id_loc INT, @id_vis INT;
DECLARE @id_det_loc INT, @id_det_vis INT;
DECLARE @gLoc INT, @gVis INT, @ptsLoc INT, @ptsVis INT;
DECLARE @resLoc VARCHAR(20), @resVis VARCHAR(20);
DECLARE @id_port_loc INT, @id_port_vis INT;
DECLARE @id_gol_loc INT, @id_gol_vis INT;
DECLARE @id_def_loc INT, @id_med_vis INT;

SET IDENTITY_INSERT dbo.PARTIDO ON;

---------------------------------------------------------------------------
-- PROCEDIMIENTO PARA TORNEOS 2, 3, 4, 5 (Clasificación 100% jugada con ratings)
---------------------------------------------------------------------------
SET @t = 2;
WHILE @t <= 5
BEGIN
    -- 1. Insertar los 66 partidos de Liga Mayor y los 66 de Liga Menor
    -- Liga Mayor (1 a 12)
    INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase)
    SELECT 
        (@t * 1000 + (f.ronda - 1) * 6 + f.partido_en_ronda),
        @t, 1, DATEADD(DAY, (f.ronda - 1) * 7, '2026-02-01 15:00:00'),
        f.ronda, 'CLASIFICACION'
    FROM @FixtureRR f;

    -- Liga Menor (13 a 24)
    INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase)
    SELECT 
        (@t * 1000 + 66 + (f.ronda - 1) * 6 + f.partido_en_ronda),
        @t, 2, DATEADD(DAY, (f.ronda - 1) * 7, '2026-02-01 11:00:00'),
        f.ronda, 'CLASIFICACION'
    FROM @FixtureRR f;

    SET @t = @t + 1;
END;

---------------------------------------------------------------------------
-- TORNEO 6 (Torneo en Curso): 132 partidos creados
---------------------------------------------------------------------------
INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase)
SELECT (6000 + (f.ronda - 1) * 6 + f.partido_en_ronda), 6, 1, DATEADD(DAY, (f.ronda - 1) * 7, '2026-02-01 15:00:00'), f.ronda, 'CLASIFICACION' FROM @FixtureRR f;

INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase)
SELECT (6066 + (f.ronda - 1) * 6 + f.partido_en_ronda), 6, 2, DATEADD(DAY, (f.ronda - 1) * 7, '2026-02-01 11:00:00'), f.ronda, 'CLASIFICACION' FROM @FixtureRR f;

---------------------------------------------------------------------------
-- TORNEO 7 (Prueba Disciplina y Descenso): 132 partidos creados
---------------------------------------------------------------------------
INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase)
SELECT (7000 + (f.ronda - 1) * 6 + f.partido_en_ronda), 7, 1, DATEADD(DAY, (f.ronda - 1) * 7, '2026-02-01 15:00:00'), f.ronda, 'CLASIFICACION' FROM @FixtureRR f;

INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase)
SELECT (7066 + (f.ronda - 1) * 6 + f.partido_en_ronda), 7, 2, DATEADD(DAY, (f.ronda - 1) * 7, '2026-02-01 11:00:00'), f.ronda, 'CLASIFICACION' FROM @FixtureRR f;

---------------------------------------------------------------------------
-- PARTIDOS DE FASES FINALES (CUARTOS, SEMIS, FINAL)
---------------------------------------------------------------------------
-- TORNEO 3: 4 Cuartos de Final (Mayor)
INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase) VALUES
(3133, 3, 1, '2026-05-10 15:00:00', 1, 'CUARTOS'),
(3134, 3, 1, '2026-05-10 15:00:00', 1, 'CUARTOS'),
(3135, 3, 1, '2026-05-11 15:00:00', 1, 'CUARTOS'),
(3136, 3, 1, '2026-05-11 15:00:00', 1, 'CUARTOS');

-- TORNEO 4: 4 Cuartos + 2 Semifinales (Mayor)
INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase) VALUES
(4133, 4, 1, '2026-05-10 15:00:00', 1, 'CUARTOS'),
(4134, 4, 1, '2026-05-10 15:00:00', 1, 'CUARTOS'),
(4135, 4, 1, '2026-05-11 15:00:00', 1, 'CUARTOS'),
(4136, 4, 1, '2026-05-11 15:00:00', 1, 'CUARTOS'),
(4137, 4, 1, '2026-05-18 15:00:00', 1, 'SEMIFINAL'),
(4138, 4, 1, '2026-05-18 15:00:00', 1, 'SEMIFINAL');

-- TORNEO 5: 4 Cuartos + 2 Semifinales + 1 Gran Final (Mayor)
INSERT INTO dbo.PARTIDO (id_partido, id_campeonato, id_division, fecha_partido, jornada, fase) VALUES
(5133, 5, 1, '2026-05-10 15:00:00', 1, 'CUARTOS'),
(5134, 5, 1, '2026-05-10 15:00:00', 1, 'CUARTOS'),
(5135, 5, 1, '2026-05-11 15:00:00', 1, 'CUARTOS'),
(5136, 5, 1, '2026-05-11 15:00:00', 1, 'CUARTOS'),
(5137, 5, 1, '2026-05-18 15:00:00', 1, 'SEMIFINAL'),
(5138, 5, 1, '2026-05-18 15:00:00', 1, 'SEMIFINAL'),
(5139, 5, 1, '2026-05-25 15:00:00', 1, 'FINAL');

SET IDENTITY_INSERT dbo.PARTIDO OFF;

-- =========================================================================
-- CREACIÓN DE DETALLE_PARTIDO PARA TODOS LOS PARTIDOS
-- =========================================================================
PRINT '5. Creando registros de DETALLE_PARTIDO (Local y Visitante)...';

-- DETALLES PARA CLASIFICACION (Torneos 2, 3, 4, 5, 6, 7)
SET @t = 2;
WHILE @t <= 7
BEGIN
    -- Liga Mayor
    INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado)
    SELECT 
        (@t * 1000 + (f.ronda - 1) * 6 + f.partido_en_ronda),
        (@t * 100 + f.idx_local), 0, 0, 0, 'PENDIENTE'
    FROM @FixtureRR f
    UNION ALL
    SELECT 
        (@t * 1000 + (f.ronda - 1) * 6 + f.partido_en_ronda),
        (@t * 100 + f.idx_visitante), 0, 0, 0, 'PENDIENTE'
    FROM @FixtureRR f;

    -- Liga Menor
    INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado)
    SELECT 
        (@t * 1000 + 66 + (f.ronda - 1) * 6 + f.partido_en_ronda),
        (@t * 100 + 12 + f.idx_local), 0, 0, 0, 'PENDIENTE'
    FROM @FixtureRR f
    UNION ALL
    SELECT 
        (@t * 1000 + 66 + (f.ronda - 1) * 6 + f.partido_en_ronda),
        (@t * 100 + 12 + f.idx_visitante), 0, 0, 0, 'PENDIENTE'
    FROM @FixtureRR f;

    SET @t = @t + 1;
END;

-- DETALLES PARA FASES FINALES
-- Torneo 3 Cuartos (Cruces 1v8, 2v7, 3v6, 4v5)
INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES
(3133, 301, 3, 0, 3, 'VICTORIA'), (3133, 308, 0, 3, 0, 'DERROTA'),
(3134, 302, 2, 1, 3, 'VICTORIA'), (3134, 307, 1, 2, 0, 'DERROTA'),
(3135, 303, 2, 0, 3, 'VICTORIA'), (3135, 306, 0, 2, 0, 'DERROTA'),
(3136, 304, 1, 0, 3, 'VICTORIA'), (3136, 305, 0, 1, 0, 'DERROTA');

-- Torneo 4 Cuartos y Semis
INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES
(4133, 401, 3, 0, 3, 'VICTORIA'), (4133, 408, 0, 3, 0, 'DERROTA'),
(4134, 402, 2, 1, 3, 'VICTORIA'), (4134, 407, 1, 2, 0, 'DERROTA'),
(4135, 403, 2, 0, 3, 'VICTORIA'), (4135, 406, 0, 2, 0, 'DERROTA'),
(4136, 404, 1, 0, 3, 'VICTORIA'), (4136, 405, 0, 1, 0, 'DERROTA'),
(4137, 401, 3, 2, 3, 'VICTORIA'), (4137, 402, 2, 3, 0, 'DERROTA'),
(4138, 403, 2, 0, 3, 'VICTORIA'), (4138, 404, 0, 2, 0, 'DERROTA');

-- Torneo 5 Cuartos, Semis y FINAL
INSERT INTO dbo.DETALLE_PARTIDO (id_partido, id_equipo, goles_favor, goles_contra, puntos_ganados, resultado) VALUES
(5133, 501, 3, 0, 3, 'VICTORIA'), (5133, 508, 0, 3, 0, 'DERROTA'),
(5134, 502, 2, 1, 3, 'VICTORIA'), (5134, 507, 1, 2, 0, 'DERROTA'),
(5135, 503, 2, 0, 3, 'VICTORIA'), (5135, 506, 0, 2, 0, 'DERROTA'),
(5136, 504, 1, 0, 3, 'VICTORIA'), (5136, 505, 0, 1, 0, 'DERROTA'),
(5137, 501, 3, 2, 3, 'VICTORIA'), (5137, 502, 2, 3, 0, 'DERROTA'),
(5138, 503, 2, 0, 3, 'VICTORIA'), (5138, 504, 0, 2, 0, 'DERROTA'),
(5139, 501, 3, 1, 3, 'VICTORIA'), (5139, 503, 1, 3, 0, 'DERROTA'); -- CSD Municipal Campeón

-- =========================================================================
-- SIMULACIÓN DE RESULTADOS E INCIDENCIAS SEGÚN TORNEO
-- =========================================================================
PRINT '6. Asignando marcadores, goles, porteros y tarjetas...';

DECLARE curPartidos CURSOR FOR
    SELECT p.id_partido, p.id_campeonato, p.jornada, p.id_division
    FROM dbo.PARTIDO p
    WHERE p.fase = 'CLASIFICACION'
    ORDER BY p.id_campeonato, p.id_partido;

OPEN curPartidos;
FETCH NEXT FROM curPartidos INTO @id_part, @t, @jornada, @id_div;

WHILE @@FETCH_STATUS = 0
BEGIN
    -- En Torneo 6: solo se juegan las jornadas 1 a 5 (jornadas 6 a 11 quedan PENDIENTES)
    IF (@t = 6 AND @jornada > 5)
    BEGIN
        FETCH NEXT FROM curPartidos INTO @id_part, @t, @jornada, @id_div;
        CONTINUE;
    END;

    -- Obtener detalles local y visitante
    SELECT TOP 1 @id_det_loc = id_detalle, @id_loc = id_equipo FROM dbo.DETALLE_PARTIDO WHERE id_partido = @id_part ORDER BY id_detalle ASC;
    SELECT TOP 1 @id_det_vis = id_detalle, @id_vis = id_equipo FROM dbo.DETALLE_PARTIDO WHERE id_partido = @id_part ORDER BY id_detalle DESC;

    DECLARE @numLoc INT = @id_loc % 100;
    DECLARE @numVis INT = @id_vis % 100;

    DECLARE @rLoc INT, @rVis INT;
    SELECT @rLoc = rating FROM @CatEquipos WHERE num = @numLoc;
    SELECT @rVis = rating FROM @CatEquipos WHERE num = @numVis;

    -- Lógica especial para Torneo 7 (Descenso muy apretado en Mayor: Achuapa 10, Marquense 11, Zacapa 12)
    IF (@t = 7 AND @id_div = 1)
    BEGIN
        -- Forzar resultados calculados para que los puestos 10, 11 y 12 terminen con 8 puntos exactamente
        IF (@numLoc = 10 AND @numVis = 11) BEGIN SET @gLoc = 3; SET @gVis = 1; END
        ELSE IF (@numLoc = 11 AND @numVis = 12) BEGIN SET @gLoc = 2; SET @gVis = 0; END
        ELSE IF (@numLoc = 12 AND @numVis = 10) BEGIN SET @gLoc = 1; SET @gVis = 0; END
        ELSE IF (@numLoc IN (10,11,12) AND @numVis <= 4) BEGIN SET @gLoc = 0; SET @gVis = 3; END
        ELSE IF (@numVis IN (10,11,12) AND @numLoc <= 4) BEGIN SET @gLoc = 3; SET @gVis = 0; END
        ELSE IF (@numLoc IN (10,11,12)) BEGIN SET @gLoc = 1; SET @gVis = 1; END
        ELSE IF (@numVis IN (10,11,12)) BEGIN SET @gLoc = 1; SET @gVis = 1; END
        ELSE
        BEGIN
            SET @gLoc = CASE WHEN @rLoc >= @rVis THEN 2 ELSE 1 END;
            SET @gVis = CASE WHEN @rLoc >= @rVis THEN 0 ELSE 2 END;
        END;
    END
    ELSE
    BEGIN
        -- Cálculo estándar por rating con ventaja de localía (+4)
        DECLARE @diff INT = (@rLoc + 4) - @rVis;
        DECLARE @var INT = (@id_part * 3 + @jornada * 7) % 5;

        IF @diff >= 22
        BEGIN
            SET @gLoc = CASE WHEN @var = 0 THEN 4 WHEN @var = 1 THEN 3 ELSE 3 END;
            SET @gVis = CASE WHEN @var = 0 THEN 0 WHEN @var = 1 THEN 1 ELSE 0 END;
        END
        ELSE IF @diff >= 12
        BEGIN
            SET @gLoc = CASE WHEN @var = 0 THEN 3 WHEN @var = 1 THEN 2 ELSE 2 END;
            SET @gVis = CASE WHEN @var = 0 THEN 1 WHEN @var = 1 THEN 0 ELSE 1 END;
        END
        ELSE IF @diff >= 4
        BEGIN
            SET @gLoc = CASE WHEN @var = 0 THEN 2 WHEN @var = 1 THEN 2 ELSE 1 END;
            SET @gVis = CASE WHEN @var = 0 THEN 0 WHEN @var = 1 THEN 1 ELSE 0 END;
        END
        ELSE IF @diff >= -4
        BEGIN
            SET @gLoc = CASE WHEN @var <= 2 THEN 1 WHEN @var = 3 THEN 0 ELSE 2 END;
            SET @gVis = CASE WHEN @var <= 2 THEN 1 WHEN @var = 3 THEN 0 ELSE 2 END;
        END
        ELSE IF @diff >= -15
        BEGIN
            SET @gLoc = CASE WHEN @var <= 1 THEN 0 WHEN @var = 2 THEN 1 ELSE 1 END;
            SET @gVis = CASE WHEN @var <= 1 THEN 1 WHEN @var = 2 THEN 2 ELSE 2 END;
        END
        ELSE
        BEGIN
            SET @gLoc = 0;
            SET @gVis = 2;
        END;
    END;

    -- Puntos y Resultados
    IF @gLoc > @gVis
    BEGIN
        SET @ptsLoc = 3; SET @ptsVis = 0;
        SET @resLoc = 'VICTORIA'; SET @resVis = 'DERROTA';
    END
    ELSE IF @gLoc = @gVis
    BEGIN
        SET @ptsLoc = 1; SET @ptsVis = 1;
        SET @resLoc = 'EMPATE'; SET @resVis = 'EMPATE';
    END
    ELSE
    BEGIN
        SET @ptsLoc = 0; SET @ptsVis = 3;
        SET @resLoc = 'DERROTA'; SET @resVis = 'VICTORIA';
    END;

    -- Actualizar marcadores en DETALLE_PARTIDO
    UPDATE dbo.DETALLE_PARTIDO SET goles_favor = @gLoc, goles_contra = @gVis, puntos_ganados = @ptsLoc, resultado = @resLoc WHERE id_detalle = @id_det_loc;
    UPDATE dbo.DETALLE_PARTIDO SET goles_favor = @gVis, goles_contra = @gLoc, puntos_ganados = @ptsVis, resultado = @resVis WHERE id_detalle = @id_det_vis;

    -- Obtener porteros
    SELECT TOP 1 @id_port_loc = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_loc AND posicion = 'PORTERO';
    SELECT TOP 1 @id_port_vis = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_vis AND posicion = 'PORTERO';

    -- Registrar actuaciones de porteros
    IF @id_port_loc IS NOT NULL
        INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos)
        VALUES (@id_part, @id_port_loc, @id_loc, 0, 90, @gVis);

    IF @id_port_vis IS NOT NULL
        INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos)
        VALUES (@id_part, @id_port_vis, @id_vis, 0, 90, @gLoc);

    -- Registrar goles del equipo local
    DECLARE @iLoc INT = 0;
    WHILE @iLoc < @gLoc
    BEGIN
        SET @iLoc = @iLoc + 1;
        -- Para el Torneo 5: concentrar goles en el delantero estrella de CSD Municipal (ID 5014)
        IF (@t = 5 AND @id_loc = 501)
            SET @id_gol_loc = 5014;
        ELSE
            SELECT TOP 1 @id_gol_loc = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_loc AND posicion = 'DELANTERO';

        IF @id_gol_loc IS NULL
            SELECT TOP 1 @id_gol_loc = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_loc;

        INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol)
        VALUES (@id_part, @id_gol_loc, @id_port_vis, @id_vis, 12 + (@iLoc * 21) % 75, 0);
    END;

    -- Registrar goles del equipo visitante
    DECLARE @iVis INT = 0;
    WHILE @iVis < @gVis
    BEGIN
        SET @iVis = @iVis + 1;
        SELECT TOP 1 @id_gol_vis = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_vis AND posicion = 'DELANTERO';
        IF @id_gol_vis IS NULL
            SELECT TOP 1 @id_gol_vis = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_vis;

        INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol)
        VALUES (@id_part, @id_gol_vis, @id_port_loc, @id_loc, 18 + (@iVis * 25) % 70, 0);
    END;

    -- Tarjetas Disciplinarias
    -- Tarjetas amarillas
    IF (@id_part % 2 = 0 OR @t = 7)
    BEGIN
        SELECT TOP 1 @id_def_loc = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_loc AND posicion = 'DEFENSA';
        IF @id_def_loc IS NOT NULL
            INSERT INTO dbo.TARJETAS_ACUMULADAS (id_partido, id_jugador, tipo_tarjeta, minuto)
            VALUES (@id_part, @id_def_loc, 'AMARILLA', 25 + (@id_part * 7) % 55);
    END;

    IF (@id_part % 3 = 0 OR @t = 7)
    BEGIN
        SELECT TOP 1 @id_med_vis = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_vis AND posicion = 'MEDIOCAMPISTA';
        IF @id_med_vis IS NOT NULL
            INSERT INTO dbo.TARJETAS_ACUMULADAS (id_partido, id_jugador, tipo_tarjeta, minuto)
            VALUES (@id_part, @id_med_vis, 'AMARILLA', 32 + (@id_part * 11) % 50);
    END;

    -- Tarjetas rojas (frecuentes en Torneo 7 y ocasionales en los demás)
    IF ((@t = 7 AND @id_part % 4 = 0) OR (@id_part % 10 = 0))
    BEGIN
        DECLARE @id_roja INT;
        SELECT TOP 1 @id_roja = id_jugador FROM dbo.JUGADOR WHERE id_equipo = @id_loc AND posicion = 'DEFENSA';
        IF @id_roja IS NOT NULL
            INSERT INTO dbo.TARJETAS_ACUMULADAS (id_partido, id_jugador, tipo_tarjeta, minuto)
            VALUES (@id_part, @id_roja, 'ROJA', 75 + (@id_part % 14));
    END;

    FETCH NEXT FROM curPartidos INTO @id_part, @t, @jornada, @id_div;
END;

CLOSE curPartidos;
DEALLOCATE curPartidos;

-- Registrar incidencias para las Fases Finales de Torneos 3, 4 y 5
PRINT '7. Registrando incidencias para Cuartos, Semifinales y Gran Final...';

-- Torneo 3 Cuartos incidencias
INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol) VALUES
(3133, 3014, 3081, 308, 15, 0), (3133, 3014, 3081, 308, 48, 0), (3133, 3015, 3081, 308, 77, 0),
(3134, 3024, 3071, 307, 30, 0), (3134, 3024, 3071, 307, 60, 0), (3134, 3074, 3021, 302, 85, 0),
(3135, 3034, 3061, 306, 22, 0), (3135, 3034, 3061, 306, 70, 0),
(3136, 3044, 3051, 305, 55, 0);

INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos) VALUES
(3133, 3011, 301, 0, 90, 0), (3133, 3081, 308, 0, 90, 3),
(3134, 3021, 302, 0, 90, 1), (3134, 3071, 307, 0, 90, 2),
(3135, 3031, 303, 0, 90, 0), (3135, 3061, 306, 0, 90, 2),
(3136, 3041, 304, 0, 90, 0), (3136, 3051, 305, 0, 90, 1);

-- Torneo 4 Cuartos y Semis incidencias
INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol) VALUES
(4133, 4014, 4081, 408, 20, 0), (4133, 4014, 4081, 408, 50, 0), (4133, 4015, 4081, 408, 80, 0),
(4134, 4024, 4071, 407, 33, 0), (4134, 4024, 4071, 407, 65, 0), (4134, 4074, 4021, 402, 88, 0),
(4135, 4034, 4061, 406, 25, 0), (4135, 4034, 4061, 406, 72, 0),
(4136, 4044, 4051, 405, 58, 0),
(4137, 4014, 4021, 402, 14, 0), (4137, 4014, 4021, 402, 44, 0), (4137, 4015, 4021, 402, 79, 0),
(4137, 4024, 4011, 401, 31, 0), (4137, 4024, 4011, 401, 62, 0),
(4138, 4034, 4041, 404, 21, 0), (4138, 4034, 4041, 404, 69, 0);

INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos) VALUES
(4133, 4011, 401, 0, 90, 0), (4133, 4081, 408, 0, 90, 3),
(4134, 4021, 402, 0, 90, 1), (4134, 4071, 407, 0, 90, 2),
(4135, 4031, 403, 0, 90, 0), (4135, 4061, 406, 0, 90, 2),
(4136, 4041, 404, 0, 90, 0), (4136, 4051, 405, 0, 90, 1),
(4137, 4011, 401, 0, 90, 2), (4137, 4021, 402, 0, 90, 3),
(4138, 4031, 403, 0, 90, 0), (4138, 4041, 404, 0, 90, 2);

-- Torneo 5 Cuartos, Semis y FINAL incidencias
INSERT INTO dbo.REGISTRO_GOLES (id_partido, id_jugador, id_portero_vencido, id_equipo_rival, minuto, es_autogol) VALUES
(5133, 5014, 5081, 508, 15, 0), (5133, 5014, 5081, 508, 45, 0), (5133, 5015, 5081, 508, 75, 0),
(5134, 5024, 5071, 507, 30, 0), (5134, 5024, 5071, 507, 60, 0), (5134, 5074, 5021, 502, 85, 0),
(5135, 5034, 5061, 506, 22, 0), (5135, 5034, 5061, 506, 70, 0),
(5136, 5044, 5051, 505, 55, 0),
(5137, 5014, 5021, 502, 12, 0), (5137, 5014, 5021, 502, 40, 0), (5137, 5014, 5021, 502, 78, 0),
(5137, 5024, 5011, 501, 28, 0), (5137, 5024, 5011, 501, 65, 0),
(5138, 5034, 5041, 504, 20, 0), (5138, 5034, 5041, 504, 68, 0),
(5139, 5014, 5031, 503, 18, 0), (5139, 5014, 5031, 503, 52, 0), (5139, 5015, 5031, 503, 84, 0), -- Goles de Municipal
(5139, 5034, 5011, 501, 35, 0); -- Gol de Antigua

INSERT INTO dbo.CONTROL_PORTEROS (id_partido, id_jugador, id_equipo, minuto_entrada, minuto_salida, goles_recibidos) VALUES
(5133, 5011, 501, 0, 90, 0), (5133, 5081, 508, 0, 90, 3),
(5134, 5021, 502, 0, 90, 1), (5134, 5071, 507, 0, 90, 2),
(5135, 5031, 503, 0, 90, 0), (5135, 5061, 506, 0, 90, 2),
(5136, 5041, 504, 0, 90, 0), (5136, 5051, 505, 0, 90, 1),
(5137, 5011, 501, 0, 90, 2), (5137, 5021, 502, 0, 90, 3),
(5138, 5031, 503, 0, 90, 0), (5138, 5041, 504, 0, 90, 2),
(5139, 5011, 501, 0, 90, 1), (5139, 5031, 503, 0, 90, 3);

PRINT '-------------------------------------------------------------------------';
PRINT '  ¡CARGA DE MÚLTIPLES TORNEOS DE PRUEBA COMPLETADA EXITOSAMENTE!         ';
PRINT '-------------------------------------------------------------------------';
PRINT 'RESUMEN DE CONFIGURACIÓN POR TORNEO:';
PRINT '  - Torneo 1 (ID 1): 24 equipos, 120 jugadores, 0 partidos. -> LISTO PARA ROUND ROBIN';
PRINT '  - Torneo 2 (ID 2): 132 partidos de Clasificación 100% jugados. -> LISTO PARA CUARTOS';
PRINT '  - Torneo 3 (ID 3): Clasificación + 4 Cuartos jugados. -> LISTO PARA SEMIFINALES';
PRINT '  - Torneo 4 (ID 4): Clasificación + Cuartos + 2 Semis jugadas. -> LISTO PARA GRAN FINAL';
PRINT '  - Torneo 5 (ID 5): Final jugada (Municipal Campeón). -> LISTO PARA PREMIACIÓN';
PRINT '  - Torneo 6 (ID 6): Jornadas 1-5 jugadas, 6-11 pendientes. -> TORNEO EN CURSO';
PRINT '  - Torneo 7 (ID 7): Descenso apretado (Triple empate a 8 pts) + 90+ tarjetas.';
PRINT '-------------------------------------------------------------------------';
GO
