#include "api.h"  // Include this first for RankingJugador type
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stddef.h>
#include "juego.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl.h"
#include "interfaz.h"

#define SONIDOS_MAXIMOS 5
#define CANT_TIPOS_CARTAS 6
#define ANCHO_DEFAULT 800
#define ALTO_DEFAULT 600

/* Declaramos todo esto en el .c porque no se utiliza fuera de la interfaz */

/* Estado del audio */
static SDL_AudioSpec audioSpec;
static SDL_AudioDeviceID audioDispositivo = 0;
static int audioInicializado = 0;

/* Estructura para los archivos de audio */
typedef struct
{
    Uint8* data;
    Uint32 longitud;
    Uint32 pos;
    int reproduciendo;
    int loop; /* 1 si la idea es que se reproduzca en bucle, 0 para sonidos de una vez */
} Audio;

/* Inicializar todos los valores de los audios en 0 */
static Audio musicaMenu = {0};
static Audio musicaJuego = {0};
static Audio sonidoCarta = {0};
static Audio sonidoVictoria = {0};
static Audio sonidoDerrota = {0};

/* La música que suena en ese momento (importante para que no se resetee cuando vamos a ranking y volvemos) */
static Audio* musicaActual = NULL;

static Audio* sonidosActivos[SONIDOS_MAXIMOS] = {NULL, NULL, NULL, NULL, NULL};

/* Variables del GUI */
static SDL_Window *vent;
static SDL_GLContext glContext;
static struct nk_context *ctx;
static struct nk_colorf fondo;
static int corriendo = 1;
static int ventAncho = 800;  // Anchura default
static int ventAlto = 600; // Altura default

static void actualizarTamVent()
{
    SDL_GetWindowSize(vent, &ventAncho, &ventAlto);
    glViewport(0, 0, ventAncho, ventAlto);
}

/* Hacer un rectángulo para las ventanas GUI */
static struct nk_rect crearPanelRectangular(float xPorciento, float yPorciento, float anchoPorciento, float altoPorciento)
{
    float ancho = ventAncho * anchoPorciento;
    float alto = ventAlto * altoPorciento;
    float x = ventAncho * xPorciento;
    float y = ventAlto * yPorciento;
    return nk_rect(x, y, ancho, alto);
}

static struct nk_rect crearPanelArriba()
{
    return crearPanelRectangular(0.0f, 0.0f, 1.0f, 0.50f);
}

static struct nk_rect crearPanelAbajo()
{
    return crearPanelRectangular(0.0f, 0.50f, 1.0f, 0.50f);
}

static struct nk_rect crearPanelFullscreen()
{
    return nk_rect(0, 0, ventAncho, ventAlto);
}

/* Estado de la interfaz */
static int seleccionMenu = -1;
static int seleccionDificultad = -1;
static char nombreJugador[MAX_NOMBRE_JUGADOR] = "";
static const Partida* juegoActual = NULL;
static Uint32 tiempoUltimoFrame = 0;
static int pantallaJuego = 0;

/* Colores de las cartas */
static struct nk_color coloresCartas[CANT_TIPOS_CARTAS] =
{
    {45, 180, 45, 255},    // MAS2 - Verde oscuro
    {60, 200, 60, 255},    // MAS1 - Verde claro
    {200, 60, 60, 255},    // MENOS1 - Rojo claro
    {180, 45, 45, 255},    // MENOS2 - Rojo oscuro
    {60, 100, 230, 255},   // REPETIR - Azul
    {230, 230, 60, 255}    // ESPEJO - Amarillo
};

/* Colores de la fuente en las cartas */
static struct nk_color coloresFuenteCartas[CANT_TIPOS_CARTAS] =
{
    {255, 255, 255, 255},  // MAS2 - Blanco
    {255, 255, 255, 255},  // MAS1 - Blanco
    {255, 255, 255, 255},  // MENOS1 - Blanco
    {255, 255, 255, 255},  // MENOS2 - Blanco
    {255, 255, 255, 255},  // REPETIR - Blanco
    {0, 0, 0, 255}         // ESPEJO - Negro
};

/* Funciones del audio */
static void audioCallback(void* userdata, Uint8* stream, int longitud)
{
    (void)userdata; // No se usa el parámetro así que evitamos el warning
    Uint32 restante, copiar;
    Audio* sonido;

    /* Limpiamos el buffer del audio */
    SDL_memset(stream, 0, longitud);

    if (musicaActual && musicaActual->reproduciendo && musicaActual->data)
    {
        restante = musicaActual->longitud - musicaActual->pos;
        copiar = (Uint32)longitud < restante ? (Uint32)longitud : restante;

        if (copiar > 0)
        {
            SDL_MixAudioFormat(stream, musicaActual->data + musicaActual->pos, AUDIO_S16SYS, copiar, SDL_MIX_MAXVOLUME * 3 / 4);
            musicaActual->pos += copiar;

            /* Repetir la música si está en bucle */
            if (musicaActual->pos >= musicaActual->longitud && musicaActual->loop)
            {
                musicaActual->pos = 0;
            }
            else if (musicaActual->pos >= musicaActual->longitud)
            {
                musicaActual->reproduciendo = 0;
            }
        }
    }

    for (int i = 0; i < SONIDOS_MAXIMOS; i++)
    {
        sonido = sonidosActivos[i];
        if (sonido && sonido->reproduciendo && sonido->data)
        {
            restante = sonido->longitud - sonido->pos;
            copiar = (Uint32)longitud < restante ? (Uint32)longitud : restante;

            if (copiar > 0)
            {
                SDL_MixAudioFormat(stream, sonido->data + sonido->pos, AUDIO_S16SYS, copiar, SDL_MIX_MAXVOLUME);
                sonido->pos += copiar;

                /* Parar el sonido cuando termina porque no hace loop */
                if (sonido->pos >= sonido->longitud)
                {
                    sonido->reproduciendo = 0;
                    sonidosActivos[i] = NULL;
                }
            }
        }
    }
}

static int cargarArchivoDeAudio(const char* nombreArchivo, Audio* clip)
{
    SDL_AudioSpec wavEspecificacion;
    Uint8* wavBuffer;
    Uint32 wavLongitud;
    char wavNombreArchivo[256];
    char* ext;
    SDL_AudioCVT cvt;

    /* Inicializamos el clip */
    clip->data = NULL;
    clip->longitud = 0;
    clip->pos = 0;
    clip->reproduciendo = 0;
    clip->loop = 0;

    /* Intentar reproducir asumiendo que es .wav */
    strncpy(wavNombreArchivo, nombreArchivo, sizeof(wavNombreArchivo) - 1);
    wavNombreArchivo[sizeof(wavNombreArchivo) - 1] = '\0';

    /* Reemplazar .mp3 con .wav */
    ext = strrchr(wavNombreArchivo, '.');
    if (ext && strcmp(ext, ".mp3") == 0)
    {
        strcpy(ext, ".wav");

        if (SDL_LoadWAV(wavNombreArchivo, &wavEspecificacion, &wavBuffer, &wavLongitud) == NULL)
        {
            /* Volver al nombre de archivo anterior */
            if (SDL_LoadWAV(nombreArchivo, &wavEspecificacion, &wavBuffer, &wavLongitud) == NULL)
            {
                return 0;
            }
        }
    }
    else
    {
        /* Intentar el nombre de archivo original primero */
        if (SDL_LoadWAV(nombreArchivo, &wavEspecificacion, &wavBuffer, &wavLongitud) == NULL)
        {
            return 0;
        }
    }

    /* Comprobar si el archivo tiene contenido reproducible */
    if (wavLongitud == 0)
    {
        SDL_FreeWAV(wavBuffer);
        return 0;
    }

    /* Comprobar si el formato de audio cargado coincide con el formato de dispositivo de audio */
    if (wavEspecificacion.format != audioSpec.format ||
            wavEspecificacion.channels != audioSpec.channels ||
            wavEspecificacion.freq != audioSpec.freq)
    {

        /* Convertir el formato de audio en caso de ser necesario */
        if (SDL_BuildAudioCVT(&cvt, wavEspecificacion.format, wavEspecificacion.channels, wavEspecificacion.freq,
                              audioSpec.format, audioSpec.channels, audioSpec.freq) < 0)
        {
            SDL_FreeWAV(wavBuffer);
            return 0;
        }

        /* Alocar memoria para el audio convertido */
        cvt.buf = (Uint8*)malloc(wavLongitud * cvt.len_mult);
        if (!cvt.buf)
        {
            SDL_FreeWAV(wavBuffer);
            return 0;
        }

        /* Copiar el audio original al buffer de conversión */
        memcpy(cvt.buf, wavBuffer, wavLongitud);
        cvt.len = wavLongitud;

        /* Convertir el audio */
        if (SDL_ConvertAudio(&cvt) < 0)
        {
            free(cvt.buf);
            SDL_FreeWAV(wavBuffer);
            return 0;
        }

        /* Usar el audio convertido */
        clip->data = cvt.buf;
        clip->longitud = cvt.len_cvt;
        SDL_FreeWAV(wavBuffer);
    }
    else
    {
        /* Si el formato coincide, lo usamos como viene */
        clip->data = wavBuffer;
        clip->longitud = wavLongitud;
    }

    /* Salió todo bien, se cargó el audio */
    return 1;
}

static void liberarClip(Audio* clip)
{
    if (clip->data)
    {
        free(clip->data);
        clip->data = NULL;
    }
    clip->longitud = 0;
    clip->pos = 0;
    clip->reproduciendo = 0;
    clip->loop = 0;
}

static int inicializarSistemaDeAudio()
{
    if (audioInicializado)
    {
        return 1;
    }

    /* Ponemos la especificación de audio para el .wav */
    audioSpec.freq = 44100;
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.channels = 2;
    audioSpec.samples = 1024;
    audioSpec.callback = audioCallback;
    audioSpec.userdata = NULL;

    /* Abrir dispositivo de audio */
    audioDispositivo = SDL_OpenAudioDevice(NULL, 0, &audioSpec, NULL, 0);
    if (audioDispositivo == 0)
    {
        return 0;
    }

    cargarArchivoDeAudio("audio/musicaMenu.wav", &musicaMenu);
    cargarArchivoDeAudio("audio/musicaJuego.wav", &musicaJuego);
    cargarArchivoDeAudio("audio/carta.wav", &sonidoCarta);
    cargarArchivoDeAudio("audio/victoria.wav", &sonidoVictoria);
    cargarArchivoDeAudio("audio/derrota.wav", &sonidoDerrota);

    /* Poner la música en loop */
    musicaMenu.loop = 1;
    musicaJuego.loop = 1;

    /* Empezar la reproducción de audio */
    SDL_PauseAudioDevice(audioDispositivo, 0);

    audioInicializado = 1;
    return 1;
}

static void limpiarSistemaDeAudio()
{
    int i;

    if (!audioInicializado) return;

    /* Detener el audio */
    if (audioDispositivo)
    {
        SDL_PauseAudioDevice(audioDispositivo, 1);
        SDL_CloseAudioDevice(audioDispositivo);
        audioDispositivo = 0;
    }

    /* Liberar los clips */
    liberarClip(&musicaMenu);
    liberarClip(&musicaJuego);
    liberarClip(&sonidoCarta);
    liberarClip(&sonidoVictoria);
    liberarClip(&sonidoDerrota);

    /* Liberar el vector de sonidos activos */
    for (i = 0; i < SONIDOS_MAXIMOS; i++)
    {
        sonidosActivos[i] = NULL;
    }

    musicaActual = NULL;
    audioInicializado = 0;
}

static void reproducirMusica(Audio* musica)
{
    if (!audioInicializado || !musica || !musica->data)
    {
        return;
    }

    /* Detener la canción que está sonando */
    if (musicaActual)
    {
        musicaActual->reproduciendo = 0;
    }

    /* Empezar una música nueva */
    musicaActual = musica;
    musicaActual->pos = 0;
    musicaActual->reproduciendo = 1;
}

static void reproducirEfectoDeSonido(Audio* sonido)
{
    int i;

    if (!audioInicializado || !sonido || !sonido->data)
    {
        return;
    }

    /* Buscar un espacio vacío para el efecto de sonido */
    for (i = 0; i < SONIDOS_MAXIMOS; i++)
    {
        if (sonidosActivos[i] == NULL || !sonidosActivos[i]->reproduciendo)
        {
            sonido->pos = 0;
            sonido->reproduciendo = 1;
            sonidosActivos[i] = sonido;
            return;
        }
    }

    /* Si no hay ningún espacio disponible, reemplazar el primero */
    sonido->pos = 0;
    sonido->reproduciendo = 1;
    sonidosActivos[0] = sonido;
}

/* Versión pública de las funciones */
void reproducirMusicaJuego()
{
    reproducirMusica(&musicaJuego);
}

void reproducirSonidoCarta()
{
    reproducirEfectoDeSonido(&sonidoCarta);
}

void reproducirSonidoVictoria()
{
    reproducirEfectoDeSonido(&sonidoVictoria);
}

void reproducirSonidoDerrota()
{
    reproducirEfectoDeSonido(&sonidoDerrota);
}

/* Poner los estilos customizados */
static void ponerEstilos(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(220, 220, 220, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(45, 45, 48, 230);
    table[NK_COLOR_HEADER] = nk_rgba(40, 40, 40, 230);
    table[NK_COLOR_BORDER] = nk_rgba(65, 65, 65, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(50, 50, 50, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(40, 40, 90, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(35, 35, 120, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(70, 70, 70, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(80, 80, 80, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(100, 100, 190, 255);
    table[NK_COLOR_SELECT] = nk_rgba(45, 45, 45, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(35, 35, 100, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(70, 70, 70, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(100, 100, 150, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(120, 120, 170, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(150, 150, 200, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(50, 50, 50, 255);
    table[NK_COLOR_EDIT] = nk_rgba(50, 50, 50, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
    table[NK_COLOR_COMBO] = nk_rgba(50, 50, 50, 255);
    table[NK_COLOR_CHART] = nk_rgba(50, 50, 50, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(100, 100, 150, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(150, 150, 200, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 50, 50, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(70, 70, 70, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(90, 90, 90, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(110, 110, 110, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(70, 70, 70, 255);
    nk_style_from_table(ctx, table);
}

/* Inicializar la GUI */
void iniciarInterfaz()
{
    struct nk_font_atlas *atlas;
    struct nk_font *fuente;

    float tituloAncho;
    float tituloAlto;
    float tituloX;
    float tituloY;

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        exit(1);
    }

    /* Set GL attributes for compatibility with Nuklear's GL2 implementation */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    vent = SDL_CreateWindow("Juego DoCe - Abstracto",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            ANCHO_DEFAULT, ALTO_DEFAULT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!vent)
    {
        SDL_Quit();
        exit(1);
    }

    glContext = SDL_GL_CreateContext(vent);
    if (!glContext)
    {
        SDL_DestroyWindow(vent);
        SDL_Quit();
        exit(1);
    }

    SDL_GL_MakeCurrent(vent, glContext);
    SDL_GL_SetSwapInterval(1); // vsync

    /* Inicializar la librería Nuklear */
    ctx = nk_sdl_init(vent);
    if (!ctx)
    {
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(vent);
        SDL_Quit();
        exit(1);
    }

    /* Cargar fuentes */
    nk_sdl_font_stash_begin(&atlas);
    fuente = nk_font_atlas_add_default(atlas, 16, NULL);
    nk_sdl_font_stash_end();

    /* Poner la fuente como activa */
    nk_style_set_font(ctx, &fuente->handle);

    /* Ponerle el estilo */
    ponerEstilos(ctx);

    /* Poner el color del fondo */
    fondo.r = 20.0f / 255.0f;
    fondo.g = 30.0f / 255.0f;
    fondo.b = 40.0f / 255.0f;
    fondo.a = 1.0f;

    /* Conseguir el tamaño inicial de la ventana */
    actualizarTamVent();

    /* Inicializar la semilla */
    srand((unsigned int)time(NULL));

    /* Guardar el tiempo inicial */
    tiempoUltimoFrame = SDL_GetTicks();

    /* Hacer un renderizado inicial para asegurarse de que la ventana muestre el contenido de inmediato */
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);

    if (nk_begin(ctx, "Bienvenido", crearPanelFullscreen(),
                 NK_WINDOW_NO_SCROLLBAR))
    {
        /* Usar el espacio del layout para el contenido centrado */
        nk_layout_space_begin(ctx, NK_STATIC, ventAlto, 10);

        /* Calcular el título centrado */
        tituloAncho = 300;
        tituloAlto = 60;
        tituloX = (ventAncho - tituloAncho) / 2;
        tituloY = (ventAlto - tituloAlto) / 2;

        nk_layout_space_push(ctx, nk_rect(tituloX, tituloY - 30, tituloAncho, 30));
        nk_label(ctx, "DoCe", NK_TEXT_CENTERED);
        nk_layout_space_push(ctx, nk_rect(tituloX, tituloY + 10, tituloAncho, 30));
        nk_label(ctx, "Cargando...", NK_TEXT_CENTERED);

        nk_layout_space_end(ctx);
    }
    nk_end(ctx);

    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_GL_SwapWindow(vent);

    /* Procesar eventos pendientes para que no se cuelgue la ventana */
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            exit(0);
        }
    }

    /* Inicializar el sistema de audio */
    inicializarSistemaDeAudio();
}

void cerrarInterfaz()
{
    limpiarSistemaDeAudio();

    if (ctx)
    {
        nk_sdl_shutdown();
        ctx = NULL;
    }
    if (glContext)
    {
        SDL_GL_DeleteContext(glContext);
        glContext = NULL;
    }
    if (vent)
    {
        SDL_DestroyWindow(vent);
        vent = NULL;
    }
    SDL_Quit();
}

void mostrarMenuPrincipal()
{
    SDL_Event evt;
    struct nk_rect menu_rect;
    struct nk_style_item estiloVisualizacion;
    struct nk_style_item estiloAnterior;
    struct nk_style_button estiloDeBotonAnterior;

    float menuAncho, menuAlto, menuX, menuY;
    int anchoActual, altoActual;

    /* Botones del menú */
    const float botonAlto = 60;
    const float botonEspacio = 20;
    float yActual;

    seleccionMenu = -1;

    /* Solo ponemos la música del menú si no está sonando ya */
    if (!musicaActual || musicaActual != &musicaMenu || !musicaActual->reproduciendo)
    {
        reproducirMusica(&musicaMenu);
    }

    do
    {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                corriendo = 0;
                seleccionMenu = 2; // salir
                break;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* Comprobar si cambió el tamaño de la ventana */
        SDL_GetWindowSize(vent, &anchoActual, &altoActual);
        if (anchoActual != ventAncho || altoActual != ventAlto)
        {
            actualizarTamVent();
        }

        menu_rect = crearPanelFullscreen();
        if (nk_begin(ctx, "Menú Principal", menu_rect, NK_WINDOW_NO_SCROLLBAR))
        {
            /* Estilo del fondo */
            estiloVisualizacion = ctx->style.window.fixed_background;
            ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(20, 30, 40, 255));

            /* Para los ítems del menú */
            menuAncho = ventAncho * 0.5f;
            menuAlto = ventAlto * 0.7f;
            menuX = (ventAncho - menuAncho) / 2;
            menuY = (ventAlto - menuAlto) / 2;

            /* Título del juego */
            nk_layout_space_begin(ctx, NK_STATIC, menuAlto, 10);

            /* Área del título */
            nk_layout_space_push(ctx, nk_rect(menuX, menuY, menuAncho, 80));
            {
                /* Alinear al centro el título del texto */
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 250, 255);
                ctx->style.button.text_alignment = NK_TEXT_CENTERED;

                /* Texto del título */
                nk_button_label(ctx, "DOCE");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }

            /* Separador decorativo */
            nk_layout_space_push(ctx, nk_rect(menuX, menuY + 90, menuAncho, 10));
            {
                nk_stroke_line(&ctx->current->buffer,
                               menuX, menuY + 95,
                               menuX + menuAncho, menuY + 95,
                               2.0f, nk_rgb(100, 100, 200));
            }

            yActual = menuY + 120;

            /* Botón de jugar */
            nk_layout_space_push(ctx, nk_rect(menuX, yActual, menuAncho, botonAlto));
            if (nk_button_label(ctx, "1. Jugar"))
            {
                seleccionMenu = 0;
            }
            yActual += botonAlto + botonEspacio;

            /* Botón de ranking */
            nk_layout_space_push(ctx, nk_rect(menuX, yActual, menuAncho, botonAlto));
            if (nk_button_label(ctx, "2. Ranking"))
            {
                seleccionMenu = 1;
            }
            yActual += botonAlto + botonEspacio;

            /* Botón de salir */
            nk_layout_space_push(ctx, nk_rect(menuX, yActual, menuAncho, botonAlto));
            if (nk_button_label(ctx, "3. Salir"))
            {
                seleccionMenu = 2;
            }

            /* Pie de página */
            nk_layout_space_push(ctx, nk_rect(menuX, menuY + menuAlto - 30, menuAncho, 20));
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(150, 150, 150, 255);
                ctx->style.button.text_alignment = NK_TEXT_RIGHT;

                nk_button_label(ctx, "Abstracto - Versión 1.0");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }

            nk_layout_space_end(ctx);

            /* Resetear el estilo */
            ctx->style.window.fixed_background = estiloVisualizacion;
        }
        nk_end(ctx);

        /* Renderizar */
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);

        /* Delay para no sobrecargar la CPU */
        SDL_Delay(10);

    }
    while (seleccionMenu == -1 && corriendo);
}

static void dibujarCarta(struct nk_context *ctx, int x, int y, int ancho, int alto,
                         TipoCarta tipo, const char* nombre, int highlight)
{
    /* Fondo de la carta */
    struct nk_rect rectanguloCarta = nk_rect(x, y, ancho, alto);
    struct nk_color colorFondo = coloresCartas[tipo];
    struct nk_color colorTexto = coloresFuenteCartas[tipo];

    /* Si la carta es seleccionada, ponerle un higlight */
    struct nk_color colorBorde = highlight ?
                                 nk_rgb(255, 255, 255) :
                                 nk_rgb(40, 40, 40);

    struct nk_rect rectanguloDescripcion = nk_rect(x + 10, y + alto - 30, ancho - 20, 20);

    struct nk_rect rectanguloTitulo;
    struct nk_rect circulo;
    struct nk_rect rectanguloTexto;
    struct nk_rect espejoMarco;
    struct nk_rect espejoSuperficie;

    int bordeAncho = highlight ? 3 : 1;

    int i, segmentos;
    float cx = x + ancho/2;
    float cy = y + alto/2;
    float r, x1, y1, x2, y2, cambioAngulo, angulo, anguloFlecha;

    const char* textoEfecto = "";

    /* Dibujar el fondo de la carta */
    nk_fill_rect(&ctx->current->buffer, rectanguloCarta, 10, colorFondo);

    /* Dibujar el borde */
    nk_stroke_rect(&ctx->current->buffer, rectanguloCarta, 10, bordeAncho, colorBorde);

    /* Dibujar el título */
    rectanguloTitulo = nk_rect(x + 10, y + 10, ancho - 20, 25);
    nk_draw_text(&ctx->current->buffer, rectanguloTitulo, nombre, strlen(nombre),
                 ctx->style.font, nk_rgba(0,0,0,0), colorTexto);

    /* Dibujar el símbolo según el tipo de carta */
    switch(tipo)
    {
    case MAS2:
        /* +2 con un círculo alrededor */
    {
        r = 20;

        /* Círculo del fondo */
        circulo = nk_rect(cx - r, cy - r, r * 2, r * 2);
        nk_fill_circle(&ctx->current->buffer, circulo, nk_rgb(30, 140, 30));

        /* Texto de +2 */
        rectanguloTexto = nk_rect(cx - 10, cy - 8, 20, 16);
        nk_draw_text(&ctx->current->buffer, rectanguloTexto, "+2", 2,
                     ctx->style.font, nk_rgba(0,0,0,0), colorTexto);
    }
    break;
    case MAS1:
        /* +1 con un círculo alrededor */
    {
        r = 20;

        /* Círculo del fondo */
        circulo = nk_rect(cx - r, cy - r, r * 2, r * 2);
        nk_fill_circle(&ctx->current->buffer, circulo, nk_rgb(40, 160, 40));

        /* Texto +1 */
        rectanguloTexto = nk_rect(cx - 10, cy - 8, 20, 16);
        nk_draw_text(&ctx->current->buffer, rectanguloTexto, "+1", 2,
                     ctx->style.font, nk_rgba(0,0,0,0), colorTexto);
    }
    break;
    case MENOS1:
        /* -1 con un círculo alrededor */
    {
        r = 20;

        /* Círculo del fondo */
        circulo = nk_rect(cx - r, cy - r, r * 2, r * 2);
        nk_fill_circle(&ctx->current->buffer, circulo, nk_rgb(160, 40, 40));

        /* Texto -1 */
        rectanguloTexto = nk_rect(cx - 10, cy - 8, 20, 16);
        nk_draw_text(&ctx->current->buffer, rectanguloTexto, "-1", 2,
                     ctx->style.font, nk_rgba(0,0,0,0), colorTexto);
    }
    break;
    case MENOS2:
        /* -2 con un círculo alrededor */
    {
        r = 20;

        /* Círculo del fondo */
        circulo = nk_rect(cx - r, cy - r, r * 2, r * 2);
        nk_fill_circle(&ctx->current->buffer, circulo, nk_rgb(140, 30, 30));

        /* Texto -2 */
        rectanguloTexto = nk_rect(cx - 10, cy - 8, 20, 16);
        nk_draw_text(&ctx->current->buffer, rectanguloTexto, "-2", 2,
                     ctx->style.font, nk_rgba(0,0,0,0), colorTexto);
    }
    break;
    case REPETIR:
        /* Flecha circular */
    {
        r = 18;

        /* Círculo de fondo */
        circulo = nk_rect(cx - r - 5, cy - r - 5, (r + 5) * 2, (r + 5) * 2);
        nk_fill_circle(&ctx->current->buffer, circulo, nk_rgb(50, 80, 200));

        segmentos = 16;
        cambioAngulo = 2 * 3.14159f * 0.85f / segmentos;
        angulo = -1.57f; // Empzamos de arriba (-pi/2)

        for (i = 0; i < segmentos; i++)
        {
            x1 = cx + r * cosf(angulo);
            y1 = cy + r * sinf(angulo);
            angulo += cambioAngulo;
            x2 = cx + r * cosf(angulo);
            y2 = cy + r * sinf(angulo);

            nk_stroke_line(&ctx->current->buffer, x1, y1, x2, y2, 2.5f, colorTexto);
        }

        /* Punta de la flecha */
        anguloFlecha = -1.57f; // -pi/2
        x1 = cx + r * cosf(anguloFlecha);
        y1 = cy + r * sinf(anguloFlecha);
        nk_stroke_line(&ctx->current->buffer, x1, y1, x1 + 5, y1 - 5, 2.0f, colorTexto);
        nk_stroke_line(&ctx->current->buffer, x1, y1, x1 + 5, y1 + 5, 2.0f, colorTexto);
    }
    break;
    case ESPEJO:
        /* Dibujar un espejo */
    {
        /* Círculo de fondo */
        circulo = nk_rect(cx - 22, cy - 22, 44, 44);
        nk_fill_circle(&ctx->current->buffer, circulo, nk_rgb(180, 180, 40));

        /* Marco */
        espejoMarco = nk_rect(cx - 15, cy - 20, 30, 40);
        nk_stroke_rect(&ctx->current->buffer, espejoMarco, 5, 2, colorTexto);

        /* Superficie del espejo */
        espejoSuperficie = nk_rect(cx - 12, cy - 17, 24, 34);
        nk_fill_rect(&ctx->current->buffer, espejoSuperficie, 3, nk_rgb(200, 200, 200));

        /* Línea del reflejo */
        nk_stroke_line(&ctx->current->buffer, cx, cy - 17, cx, cy + 17, 1.5f, nk_rgb(150, 150, 150));
    }
    break;
    }

    /* Descripción de la carta */
    switch(tipo)
    {
    case MAS2:
        textoEfecto = "+2 puntos";
        break;
    case MAS1:
        textoEfecto = "+1 punto";
        break;
    case MENOS1:
        textoEfecto = "-1 al rival";
        break;
    case MENOS2:
        textoEfecto = "-2 al rival";
        break;
    case REPETIR:
        textoEfecto = "Repetir turno";
        break;
    case ESPEJO:
        textoEfecto = "Anular efecto";
        break;
    }
    nk_draw_text(&ctx->current->buffer, rectanguloDescripcion, textoEfecto, strlen(textoEfecto),
                 ctx->style.font, nk_rgba(0,0,0,0), colorTexto);
}

static void dibujarDorsoCarta(struct nk_context *ctx, int x, int y, int ancho, int alto, int highlight)
{
    /* Fondo de la carta */
    struct nk_rect rectanguloCarta = nk_rect(x, y, ancho, alto);
    struct nk_color colorFondo = nk_rgb(80, 80, 100);
    struct nk_rect rectanguloTitulo = nk_rect(x + ancho/2 - 30, y + alto/2 - 15, 60, 30);

    /* Poner un highlight si se selecciona la carta */
    struct nk_color colorBorde = highlight ?
                                 nk_rgb(255, 255, 255) :
                                 nk_rgb(40, 40, 40);
    int bordeAncho = highlight ? 3 : 1;

    int i;

    /* Dibujar el fondo de la carta */
    nk_fill_rect(&ctx->current->buffer, rectanguloCarta, 10, colorFondo);

    /* Dibujar el borde */
    nk_stroke_rect(&ctx->current->buffer, rectanguloCarta, 10, bordeAncho, colorBorde);

    /* Dibujar el patrón */
    for (i = 0; i < ancho + alto; i += 10)
    {
        nk_stroke_line(&ctx->current->buffer,
                       x + (i < ancho ? i : 0),
                       y + (i < ancho ? 0 : i - ancho),
                       x + (i < alto ? 0 : i - alto),
                       y + (i < alto ? i : alto),
                       1.0f, nk_rgb(60, 60, 80));
    }

    /* Texto "DoCe" */
    nk_draw_text(&ctx->current->buffer, rectanguloTitulo, "DoCe", 4,
                 ctx->style.font, nk_rgba(0,0,0,0), nk_rgb(220, 220, 220));
}

/* Selección del menú */
int esperarMenuPrincipal()
{
    return seleccionMenu;
}

int mostrarSeleccionDificultad()
{
    SDL_Event evt;
    seleccionDificultad = -1;
    static int opcion = 0;
    int anchoActual, altoActual;

    struct nk_rect rectanguloFullscreen;
    struct nk_style_item estiloVisualizacion;
    struct nk_style_item estiloAnterior;
    struct nk_style_button estiloDeBotonAnterior;

    float panelAncho, panelAlto, panelX, panelY;
    float yActual;
    const float opcionAltura = 40;
    const float descripcionAltura = 30;
    const float espacio = 15;

    actualizarTamVent();

    /* Usar pantalla completa para el fondo */
    rectanguloFullscreen = crearPanelFullscreen();

    do
    {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                corriendo = 0;
                break;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* Comprobar si cambió el tamaño de la ventana */
        SDL_GetWindowSize(vent, &anchoActual, &altoActual);
        if (anchoActual != ventAncho || altoActual != ventAlto)
        {
            actualizarTamVent();
            rectanguloFullscreen = crearPanelFullscreen();
        }

        /* GUI */
        if (nk_begin(ctx, "Selección de Dificultad", rectanguloFullscreen, NK_WINDOW_NO_SCROLLBAR))
        {
            /* Estilizar el fondo */
            estiloVisualizacion = ctx->style.window.fixed_background;
            ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(20, 30, 40, 255));

            /* Calcular las dimensiones centradas del panel */
            panelAncho = ventAncho * 0.6f;
            panelAlto = ventAlto * 0.7f;
            panelX = (ventAncho - panelAncho) / 2;
            panelY = (ventAlto - panelAlto) / 2;

            /* Usar el espacio para posicionar */
            nk_layout_space_begin(ctx, NK_STATIC, panelAlto, 10);

            /* Área de título */
            nk_layout_space_push(ctx, nk_rect(panelX, panelY, panelAncho, 50));
            {
                /* Alinear el texto del título */
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 250, 255);
                ctx->style.button.text_alignment = NK_TEXT_CENTERED;

                nk_button_label(ctx, "SELECCIÓN DE DIFICULTAD");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }

            /* Separador decorativo */
            nk_layout_space_push(ctx, nk_rect(panelX, panelY + 60, panelAncho, 10));
            {
                nk_stroke_line(&ctx->current->buffer,
                               panelX, panelY + 65,
                               panelX + panelAncho, panelY + 65,
                               2.0f, nk_rgb(100, 100, 200));
            }

            /* Opciones de dificultad */
            yActual = panelY + 90;

            /* Fácil */
            nk_layout_space_push(ctx, nk_rect(panelX + 20, yActual, panelAncho - 40, opcionAltura));
            if (nk_option_label(ctx, "1. Fácil", opcion == 0)) opcion = 0;
            yActual += opcionAltura;

            /* Descripción */
            nk_layout_space_push(ctx, nk_rect(panelX + 40, yActual, panelAncho - 60, descripcionAltura));
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 180, 255);
                ctx->style.button.text_alignment = NK_TEXT_LEFT;

                nk_button_label(ctx, "La IA juega cartas al azar");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }
            yActual += descripcionAltura + espacio;

            /* Media */
            nk_layout_space_push(ctx, nk_rect(panelX + 20, yActual, panelAncho - 40, opcionAltura));
            if (nk_option_label(ctx, "2. Medio", opcion == 1)) opcion = 1;
            yActual += opcionAltura;

            /* Descripción */
            nk_layout_space_push(ctx, nk_rect(panelX + 40, yActual, panelAncho - 60, descripcionAltura));
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 180, 255);
                ctx->style.button.text_alignment = NK_TEXT_LEFT;

                nk_button_label(ctx, "La IA evita jugadas inefectivas");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }
            yActual += descripcionAltura + espacio;

            /* Difícil */
            nk_layout_space_push(ctx, nk_rect(panelX + 20, yActual, panelAncho - 40, opcionAltura));
            if (nk_option_label(ctx, "3. Difícil", opcion == 2)) opcion = 2;
            yActual += opcionAltura;

            /* Descripción */
            nk_layout_space_push(ctx, nk_rect(panelX + 40, yActual, panelAncho - 60, descripcionAltura));
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 180, 255);
                ctx->style.button.text_alignment = NK_TEXT_LEFT;

                nk_button_label(ctx, "La IA elige la mejor jugada disponible");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }
            yActual += descripcionAltura + espacio + 20;

            /* Acceptar */
            nk_layout_space_push(ctx, nk_rect(panelX + panelAncho/4, yActual, panelAncho/2, 50));
            if (nk_button_label(ctx, "Aceptar"))
            {
                seleccionDificultad = opcion;
            }

            nk_layout_space_end(ctx);

            /* Resetear el estilo */
            ctx->style.window.fixed_background = estiloVisualizacion;
        }
        nk_end(ctx);

        /* Renderizar */
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);

        /* Delay para no sobrecargar la CPU */
        SDL_Delay(10);

    }
    while (seleccionDificultad == -1 && corriendo);

    return seleccionDificultad;
}

void pedirNombreJugador(char* nombre)
{
    SDL_Event evt;
    int terminoInput = 0;
    strcpy(nombreJugador, "");

    struct nk_rect rectanguloFullscreen;
    struct nk_style_item estiloVisualizacion;
    struct nk_style_item estiloAnterior;
    struct nk_style_button estiloDeBotonAnterior;
    struct nk_style_edit EditarEstiloDelAnterior;

    float panelAncho, panelAlto, panelX, panelY, inputY;
    float cartaAncho, cartaAlto, cartaEspacio, cartasAnchoTotal, cartasX;

    /* Forzar un renderizado inicial antes de entrar al bucle de eventos */
    nk_input_begin(ctx);
    nk_input_end(ctx);

    actualizarTamVent();

    /* Usar pantalla completa para el fondo */
    rectanguloFullscreen = crearPanelFullscreen();

    do
    {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                corriendo = 0;
                strcpy(nombre, "Jugador");
                return;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* Comprobar si cambió el tamaño de la ventana */
        int anchoActual, altoActual;
        SDL_GetWindowSize(vent, &anchoActual, &altoActual);
        if (anchoActual != ventAncho || altoActual != ventAlto)
        {
            actualizarTamVent();
            rectanguloFullscreen = crearPanelFullscreen();
        }

        /* GUI */
        if (nk_begin(ctx, "Nombre Jugador", rectanguloFullscreen, NK_WINDOW_NO_SCROLLBAR))
        {
            /* Estilizar el fondo */
            estiloVisualizacion = ctx->style.window.fixed_background;
            ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(20, 30, 40, 255));

            /* Calcular las dimensiones centradas del panel */
            panelAncho = ventAncho * 0.6f;
            panelAlto = ventAlto * 0.7f;
            panelX = (ventAncho - panelAncho) / 2;
            panelY = (ventAlto - panelAlto) / 2;

            /* Usar el espacio para posicionar */
            nk_layout_space_begin(ctx, NK_STATIC, ventAlto, 20);

            /* Área de título */
            nk_layout_space_push(ctx, nk_rect(panelX, panelY, panelAncho, 50));
            {
                /* Alinear el texto del título */
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 250, 255);
                ctx->style.button.text_alignment = NK_TEXT_CENTERED;

                nk_button_label(ctx, "INGRESE SU NOMBRE");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }

            /* Separador decorativo */
            nk_layout_space_push(ctx, nk_rect(panelX, panelY + 60, panelAncho, 10));
            {
                nk_stroke_line(&ctx->current->buffer,
                               panelX, panelY + 65,
                               panelX + panelAncho, panelY + 65,
                               2.0f, nk_rgb(100, 100, 200));
            }

            /* Sección de entrada de nombre */
            inputY = panelY + 90;

            /* Texto de instrucción */
            nk_layout_space_push(ctx, nk_rect(panelX + 20, inputY, panelAncho - 40, 30));
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(220, 220, 220, 255);
                ctx->style.button.text_alignment = NK_TEXT_CENTERED;

                nk_button_label(ctx, "Por favor, escriba su nombre:");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }
            inputY += 40;

            /* Campo de entrada del nombre */
            nk_layout_space_push(ctx, nk_rect(panelX + panelAncho*0.15f, inputY, panelAncho*0.7f, 50));
            {
                EditarEstiloDelAnterior = ctx->style.edit;
                ctx->style.edit.normal = nk_style_item_color(nk_rgba(30, 40, 50, 255));
                ctx->style.edit.hover = nk_style_item_color(nk_rgba(35, 45, 55, 255));
                ctx->style.edit.active = nk_style_item_color(nk_rgba(40, 50, 60, 255));
                ctx->style.edit.cursor_size = 2;
                ctx->style.edit.cursor_normal = nk_rgb(200, 200, 200);
                ctx->style.edit.cursor_hover = nk_rgb(220, 220, 220);
                ctx->style.edit.cursor_text_normal = nk_rgb(220, 220, 220);
                ctx->style.edit.cursor_text_hover = nk_rgb(240, 240, 240);
                ctx->style.edit.text_normal = nk_rgb(220, 220, 220);
                ctx->style.edit.text_hover = nk_rgb(240, 240, 240);
                ctx->style.edit.text_active = nk_rgb(240, 240, 240);

                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nombreJugador,
                                               sizeof(nombreJugador), nk_filter_ascii);

                ctx->style.edit = EditarEstiloDelAnterior;
            }
            inputY += 60;

            /* Mensaje de error si el nombre es muy corto */
            nk_layout_space_push(ctx, nk_rect(panelX + 20, inputY, panelAncho - 40, 30));
            if (strlen(nombreJugador) > 0 && strlen(nombreJugador) < 3)
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(250, 90, 90, 255);
                ctx->style.button.text_alignment = NK_TEXT_CENTERED;

                nk_button_label(ctx, "El nombre debe tener al menos 3 caracteres");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }
            inputY += 50;

            /* Botón Aceptar */
            nk_layout_space_push(ctx, nk_rect(panelX + panelAncho/4, inputY, panelAncho/2, 50));
            {
                estiloDeBotonAnterior = ctx->style.button;

                /* Solo habilitar el botón si el nombre es válido */
                if (strlen(nombreJugador) < 3)
                {
                    ctx->style.button.normal = nk_style_item_color(nk_rgba(60, 60, 60, 255));
                    ctx->style.button.hover = nk_style_item_color(nk_rgba(60, 60, 60, 255));
                    ctx->style.button.active = nk_style_item_color(nk_rgba(60, 60, 60, 255));
                    ctx->style.button.text_normal = nk_rgb(150, 150, 150);
                    ctx->style.button.text_hover = nk_rgb(150, 150, 150);
                    ctx->style.button.text_active = nk_rgb(150, 150, 150);
                }

                if (nk_button_label(ctx, "Aceptar") && strlen(nombreJugador) >= 3)
                {
                    terminoInput = 1;
                }

                ctx->style.button = estiloDeBotonAnterior;
            }
            inputY += 80;

            /* Mostrar cartas de ejemplo */
            nk_layout_space_push(ctx, nk_rect(panelX + 20, inputY, panelAncho - 40, 30));
            {
                estiloAnterior = ctx->style.button.normal;
                estiloDeBotonAnterior = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.button.border = 0;
                ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
                ctx->style.button.text_normal = nk_rgba(180, 180, 180, 255);
                ctx->style.button.text_alignment = NK_TEXT_LEFT;

                nk_button_label(ctx, "Ejemplos de cartas:");

                ctx->style.button = estiloDeBotonAnterior;
                ctx->style.button.normal = estiloAnterior;
            }
            inputY += 40;

            /* Dibujar cartas de ejemplo */
            cartaAncho = panelAncho * 0.2f;
            cartaAlto = cartaAncho * 1.4f;
            cartaEspacio = cartaAncho * 0.2f;
            cartasAnchoTotal = cartaAncho * 3 + cartaEspacio * 2;
            cartasX = panelX + (panelAncho - cartasAnchoTotal) / 2;

            /* Dibujar cartas de ejemplo con diferentes tipos */
            dibujarCarta(ctx, cartasX, inputY, cartaAncho, cartaAlto, MAS1, "Sumar 1", 0);
            dibujarCarta(ctx, cartasX + cartaAncho + cartaEspacio, inputY, cartaAncho, cartaAlto, MENOS2, "Restar 2", 0);
            dibujarCarta(ctx, cartasX + (cartaAncho + cartaEspacio) * 2, inputY, cartaAncho, cartaAlto, ESPEJO, "Espejo", 0);

            nk_layout_space_end(ctx);

            /* Resetear el estilo */
            ctx->style.window.fixed_background = estiloVisualizacion;
        }
        nk_end(ctx);

        /* Renderizar */
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);

        /* Delay para no sobrecargar la CPU */
        SDL_Delay(16);

    }
    while (!terminoInput && corriendo);

    strncpy(nombre, nombreJugador, MAX_NOMBRE_JUGADOR - 1);
    nombre[MAX_NOMBRE_JUGADOR - 1] = '\0';
}

void mostrarRanking(const JugadorRanking* ranking, int numJugadores)
{
    SDL_Event evt;

    struct nk_style_text estiloVisualizacion;

    char victorias[32];

    int i, continuarPresionado = 0;

    float anchoTabla;
    float padding;

    /* Forzar un renderizado inicial antes de entrar al bucle de eventos */
    nk_input_begin(ctx);
    nk_input_end(ctx);

    while (!continuarPresionado && corriendo)
    {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                corriendo = 0;
                break;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* Interfaz gráfica */
        if (nk_begin(ctx, "Ranking de Jugadores", crearPanelFullscreen(),
                     NK_WINDOW_NO_SCROLLBAR))
        {

            /* Título con elementos decorativos */
            nk_layout_row_dynamic(ctx, 40, 1);
            {
                /* Área del título */
                nk_layout_row_dynamic(ctx, 60, 1);
                nk_label(ctx, "RANKING DE JUGADORES", NK_TEXT_CENTERED);
            }

            /* Separador decorativo */
            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);

            /* Área central para la tabla (usamos solo el 60% del ancho) */
            anchoTabla = ventAncho * 0.6f;
            padding = (ventAncho - anchoTabla) / 2;

            /* Lista de jugadores */
            if (ranking && numJugadores > 0)
            {
                /* Encabezados con estilo personalizado */
                nk_layout_space_begin(ctx, NK_STATIC, 30, 2);

                /* Estilo para los encabezados */
                estiloVisualizacion = ctx->style.text;
                ctx->style.text.color = nk_rgb(220, 220, 220);  // Gris muy claro
                ctx->style.text.padding = nk_vec2(0, 2);

                /* Encabezado "Jugador" */
                nk_layout_space_push(ctx, nk_rect(padding, 0, anchoTabla/2, 30));
                nk_text_colored(ctx, "JUGADOR", 7, NK_TEXT_CENTERED, nk_rgb(255, 255, 255));

                /* Encabezado "Victorias" */
                nk_layout_space_push(ctx, nk_rect(padding + anchoTabla/2, 0, anchoTabla/2, 30));
                nk_text_colored(ctx, "VICTORIAS", 9, NK_TEXT_CENTERED, nk_rgb(255, 255, 255));

                /* Restaurar estilo original */
                ctx->style.text = estiloVisualizacion;
                nk_layout_space_end(ctx);

                /* Separador bajo los encabezados */
                nk_layout_space_begin(ctx, NK_STATIC, 2, 1);
                nk_layout_space_push(ctx, nk_rect(padding, 0, anchoTabla, 2));
                nk_spacing(ctx, 1);
                nk_layout_space_end(ctx);

                /* Espacio después del separador */
                nk_layout_row_dynamic(ctx, 5, 1);
                nk_spacing(ctx, 1);

                /* Datos de jugadores */
                for (i = 0; i < numJugadores; i++)
                {
                    nk_layout_space_begin(ctx, NK_STATIC, 30, 2);
                    nk_layout_space_push(ctx, nk_rect(padding, 0, anchoTabla/2, 30));
                    nk_label(ctx, ranking[i].nombre, NK_TEXT_CENTERED);
                    nk_layout_space_push(ctx, nk_rect(padding + anchoTabla/2, 0, anchoTabla/2, 30));
                    snprintf(victorias, sizeof(victorias), "%d", ranking[i].victorias);
                    nk_label(ctx, victorias, NK_TEXT_CENTERED);
                    nk_layout_space_end(ctx);
                }
            }
            else
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "No hay datos de ranking disponibles", NK_TEXT_CENTERED);
            }

            /* Espaciador antes del botón */
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_spacing(ctx, 1);

            /* Botón Continuar (centrado y con ancho limitado) */
            nk_layout_space_begin(ctx, NK_STATIC, 40, 1);
            nk_layout_space_push(ctx, nk_rect(ventAncho/2 - 100, 0, 200, 40));
            if (nk_button_label(ctx, "Continuar"))
            {
                continuarPresionado = 1;
            }
            nk_layout_space_end(ctx);
        }
        nk_end(ctx);

        /* Renderizar */
        SDL_GetWindowSize(vent, &ventAncho, &ventAlto);
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);
    }
}

void iniciarPantallaJuego(const Partida* partida)
{
    pantallaJuego = 1;
    juegoActual = partida;
}

void terminarPantallaJuego()
{
    pantallaJuego = 0;
    juegoActual = NULL;
}

static void manejarEventosSDL(struct nk_context* ctx, int* corriendo)
{
    SDL_Event evt;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&evt))
    {
        if (evt.type == SDL_QUIT)
        {
            *corriendo = 0;
            break;
        }
        nk_sdl_handle_event(&evt);
    }
    nk_input_end(ctx);

    /* Comprobar si cambió el tamaño de la ventana */
    int anchoActual, altoActual;
    SDL_GetWindowSize(vent, &anchoActual, &altoActual);
    if (anchoActual != ventAncho || altoActual != ventAlto)
    {
        actualizarTamVent();
    }
}

static void renderizarCarta(struct nk_context* ctx, const Carta* carta, float x, float y,
                            float ancho, float alto, int highlight, int bocaAbajo)
{
    if (bocaAbajo)
    {
        dibujarDorsoCarta(ctx, x, y, ancho, alto, highlight);
    }
    else
    {
        dibujarCarta(ctx, x, y, ancho, alto, carta->tipo, carta->nombre, highlight);
    }
}

static void renderizarPanelIA(struct nk_context* ctx, const Partida* partida,
                              int mostrarCartasBocaArriba, const char* tituloEspecial,
                              struct nk_color colorTitulo)
{
    char textoUltimaCarta[64], infoIA[64], puntajeIA[32];
    float cartaAncho, cartaAlto, anchoTotal, xInicial;
    float ultimaCartaAncho, ultimaCartaAlto, ultimaCartaX, ultimaCartaY;
    float x, y;
    int i;
    struct nk_rect areaCarta;
    nk_size progresoIA, puntosMaximos;

    if (nk_begin(ctx, "Área IA", crearPanelArriba(), NK_WINDOW_NO_SCROLLBAR))
    {
        /* Información de la IA */
        nk_layout_row_dynamic(ctx, 30, 4);

        /* Última carta jugada por la IA */
        if (partida->ultimaCartaIA.nombre[0] != '\0')
        {
            snprintf(textoUltimaCarta, sizeof(textoUltimaCarta), "Última: %s", partida->ultimaCartaIA.nombre);
            nk_label_colored(ctx, textoUltimaCarta, NK_TEXT_LEFT, nk_rgb(150, 150, 255));
        }
        else
        {
            nk_label(ctx, "Última: -", NK_TEXT_LEFT);
        }

        /* Nombre y turno de la IA */
        if (partida->turnoActual == 1)
        {
            snprintf(infoIA, sizeof(infoIA), "%s (TURNO)", partida->ia.nombre);
            nk_label_colored(ctx, infoIA, NK_TEXT_LEFT, nk_rgb(255, 200, 100));
        }
        else
        {
            snprintf(infoIA, sizeof(infoIA), "%s", partida->ia.nombre);
            nk_label(ctx, infoIA, NK_TEXT_LEFT);
        }

        nk_spacing(ctx, 1);

        /* Puntaje de la IA */
        snprintf(puntajeIA, sizeof(puntajeIA), "Puntos: %d/%d", partida->ia.puntos, PUNTOS_GANAR);
        nk_label(ctx, puntajeIA, NK_TEXT_RIGHT);

        /* Barra de progreso de la IA */
        nk_layout_row_dynamic(ctx, 20, 1);
        progresoIA = (nk_size)partida->ia.puntos;
        puntosMaximos = (nk_size)PUNTOS_GANAR;
        nk_progress(ctx, &progresoIA, puntosMaximos, NK_FIXED);

        /* Título especial si existe */
        if (tituloEspecial)
        {
            nk_layout_row_dynamic(ctx, 40, 1);
            nk_label_colored(ctx, tituloEspecial, NK_TEXT_CENTERED, colorTitulo);
        }

        /* Cartas de la IA */
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Cartas de la IA:", NK_TEXT_LEFT);

        /* Espacio para las cartas */
        nk_layout_row_dynamic(ctx, 150, 1);
        areaCarta = nk_widget_bounds(ctx);

        /* Dibujar las cartas */
        cartaAncho = ventAncho * 0.12f;
        cartaAlto = cartaAncho * 1.5f;
        anchoTotal = cartaAncho * MAX_MANO * 1.2f;
        xInicial = (ventAncho - anchoTotal) / 2;
        y = areaCarta.y + 50;

        for (i = 0; i < MAX_MANO; i++)
        {
            if (partida->ia.mano[i].nombre[0] != '\0')
            {
                x = xInicial + i * cartaAncho * 1.2f;
                renderizarCarta(ctx, &partida->ia.mano[i], x, y, cartaAncho, cartaAlto, 0, !mostrarCartasBocaArriba);
            }
        }

        /* Mostrar última carta jugada */
        if (partida->ultimaCartaIA.nombre[0] != '\0')
        {
            ultimaCartaAncho = ventAncho * 0.10f;
            ultimaCartaAlto = ultimaCartaAncho * 1.5f;
            ultimaCartaX = ventAncho * 0.05f;
            ultimaCartaY = areaCarta.y + 5;

            renderizarCarta(ctx, &partida->ultimaCartaIA, ultimaCartaX, ultimaCartaY,
                            ultimaCartaAncho, ultimaCartaAlto, 1, 0);
        }
    }
    nk_end(ctx);
}

static void renderizarPanelJugador(struct nk_context* ctx, const Partida* partida,
                                   int permitirInteraccion, const char* mensajeInferior,
                                   int* cartaSeleccionada)
{
    char textoUltimaCarta[64], infoJugador[64], puntajeJugador[32], descripcion[256];
    const char* descripcionEfecto = "";

    struct nk_rect limites;
    struct nk_rect limitesCartas;
    nk_size progresoJugador, puntosMaximos;

    static int cartaHighlight = -1;
    int i, tieneHighlight;

    float cartaAncho, cartaAlto, anchoTotal, xInicial;
    float x, y;
    float highlightOffset;

    if (nk_begin(ctx, "Área del jugador", crearPanelAbajo(), NK_WINDOW_NO_SCROLLBAR))
    {
        /* Información del jugador */
        nk_layout_row_dynamic(ctx, 30, 4);

        /* Última carta jugada */
        if (partida->ultimaCartaJugador.nombre[0] != '\0')
        {
            snprintf(textoUltimaCarta, sizeof(textoUltimaCarta), "Última: %s", partida->ultimaCartaJugador.nombre);
            nk_label_colored(ctx, textoUltimaCarta, NK_TEXT_LEFT, nk_rgb(150, 255, 150));
        }
        else
        {
            nk_label(ctx, "Última: -", NK_TEXT_LEFT);
        }

        /* Nombre y turno del jugador */
        if (partida->turnoActual == 0)
        {
            snprintf(infoJugador, sizeof(infoJugador), "%s (TURNO)", partida->jugador.nombre);
            nk_label_colored(ctx, infoJugador, NK_TEXT_LEFT, nk_rgb(100, 255, 100));
        }
        else
        {
            snprintf(infoJugador, sizeof(infoJugador), "%s", partida->jugador.nombre);
            nk_label(ctx, infoJugador, NK_TEXT_LEFT);
        }

        nk_spacing(ctx, 1);

        /* Puntaje del jugador */
        snprintf(puntajeJugador, sizeof(puntajeJugador), "Puntos: %d/%d", partida->jugador.puntos, PUNTOS_GANAR);
        nk_label(ctx, puntajeJugador, NK_TEXT_RIGHT);

        /* Barra de progreso del jugador */
        nk_layout_row_dynamic(ctx, 20, 1);
        progresoJugador = (nk_size)partida->jugador.puntos;
        puntosMaximos = (nk_size)PUNTOS_GANAR;
        nk_progress(ctx, &progresoJugador, puntosMaximos, NK_FIXED);

        /* Mensaje inferior, si existe */
        if (mensajeInferior)
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, mensajeInferior, NK_TEXT_LEFT);
        }

        /* Cartas del jugador */
        nk_layout_row_dynamic(ctx, 180, 1);
        limites = nk_widget_bounds(ctx);

        cartaAncho = ventAncho * 0.18f;
        cartaAlto = cartaAncho * 1.5f;
        anchoTotal = cartaAncho * MAX_MANO * 1.2f;
        xInicial = (ventAncho - anchoTotal) / 2;

        /* Resetear highlight */
        cartaHighlight = -1;

        /* Dibujar cartas con interacción si está permitida */
        for (i = 0; i < MAX_MANO; i++)
        {
            if (partida->jugador.mano[i].nombre[0] != '\0')
            {
                x = xInicial + i * cartaAncho * 1.2f;
                y = limites.y + 10;
                highlightOffset = 0;
                tieneHighlight = 0;

                if (permitirInteraccion)
                {
                    limitesCartas = nk_rect(x, y, cartaAncho, cartaAlto);
                    tieneHighlight = nk_input_is_mouse_hovering_rect(&ctx->input, limitesCartas);

                    if (tieneHighlight)
                    {
                        cartaHighlight = i;
                        highlightOffset = -15;

                        if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT))
                        {
                            *cartaSeleccionada = i;
                        }
                    }
                }

                renderizarCarta(ctx, &partida->jugador.mano[i], x, y + highlightOffset,
                                cartaAncho, cartaAlto, tieneHighlight, 0);
            }
        }

        /* Descripción de la carta seleccionada */
        if (permitirInteraccion && cartaHighlight >= 0)
        {
            switch(partida->jugador.mano[cartaHighlight].tipo)
            {
            case MAS2:
                descripcionEfecto = "Suma 2 puntos a tu puntaje";
                break;
            case MAS1:
                descripcionEfecto = "Suma 1 punto a tu puntaje";
                break;
            case MENOS1:
                descripcionEfecto = "Resta 1 punto al oponente";
                break;
            case MENOS2:
                descripcionEfecto = "Resta 2 puntos al oponente";
                break;
            case REPETIR:
                descripcionEfecto = "Jugás de vuelta y agarrás una carta";
                break;
            case ESPEJO:
                descripcionEfecto = "Anula y devuelve los puntos que te quieran restar";
                break;
            }

            nk_layout_row_dynamic(ctx, 60, 1);
            snprintf(descripcion, sizeof(descripcion), "Efecto: %s", descripcionEfecto);
            nk_label(ctx, descripcion, NK_TEXT_LEFT);
        }
    }
    nk_end(ctx);
}

/* GUI para elegir una carta por el humano */
int elegirCartaGUI(const Jugador* jugador, const Partida* partida)
{
    int cartaSeleccionada = -1;

    while (cartaSeleccionada == -1 && corriendo)
    {
        /* Manejar eventos SDL */
        manejarEventosSDL(ctx, &corriendo);

        /* Limpiar la pantalla */
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);

        /* Botón para terminar partida */
        struct nk_rect rectanguloTerminarPartida = nk_rect(ventAncho - 200, 80, 180, 50);
        if (nk_begin(ctx, "Terminar Partida", rectanguloTerminarPartida, NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "Terminar Partida"))
            {
                cartaSeleccionada = SALIR_DEL_JUEGO;
            }
        }
        nk_end(ctx);

        /* Renderizar paneles */
        renderizarPanelIA(ctx, partida, 0, NULL, nk_rgb(0, 0, 0));
        renderizarPanelJugador(ctx, partida, 1, "Hacé clic para seleccionar:", &cartaSeleccionada);

        /* Renderizar y esperar */
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);
        SDL_Delay(16);
    }

    return cartaSeleccionada;
}

/* Mostrar de quién es el turno */
void mostrarTurnoJugador(const char* nombreJugador, int esIA)
{
    static int esTurnoRepetido = 0;
    Uint32 tiempoInicio, tiempoDelay;

    /* Reset del estado si se llama con parámetros nulos */
    if (nombreJugador == NULL && esIA == -1)
    {
        esTurnoRepetido = 0;
        return;
    }

    /* Solo procesar si estamos en la pantalla de juego */
    if (!pantallaJuego || !juegoActual)
    {
        return;
    }

    /* Si es turno de la IA, agregar delay artificial */
    if (esIA)
    {
        tiempoInicio = SDL_GetTicks();
        tiempoDelay = esTurnoRepetido ? 400 : 1500;
        esTurnoRepetido = 1;

        while (SDL_GetTicks() - tiempoInicio < tiempoDelay && corriendo)
        {
            /* Manejar eventos SDL */
            manejarEventosSDL(ctx, &corriendo);

            /* Limpiar la pantalla */
            glViewport(0, 0, ventAncho, ventAlto);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);

            /* Renderizar paneles */
            renderizarPanelIA(ctx, juegoActual, 0, NULL, nk_rgb(0, 0, 0));
            renderizarPanelJugador(ctx, juegoActual, 0, "Espere mientras la IA realiza su jugada...", NULL);

            /* Renderizar y esperar */
            nk_sdl_render(NK_ANTI_ALIASING_ON);
            SDL_GL_SwapWindow(vent);
            SDL_Delay(16);
        }
    }
}

/* Mostrar el resultado final del juego */
void mostrarResultadoPartida(const Partida* partida)
{
    const char* titulo;
    struct nk_color colorTitulo;
    struct nk_rect rectanguloBoton;
    int continuar = 0;

    /* Determinar título y color según el resultado */
    if (partida->jugador.esVencedor)
    {
        titulo = "¡VICTORIA!";
        colorTitulo = nk_rgb(100, 255, 100);
    }
    else
    {
        titulo = "DERROTA :(";
        colorTitulo = nk_rgb(255, 100, 100);
    }

    while (!continuar && corriendo)
    {
        /* Manejar eventos SDL */
        manejarEventosSDL(ctx, &corriendo);

        /* Limpiar la pantalla */
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);

        /* Renderizar paneles */
        renderizarPanelIA(ctx, partida, 1, titulo, colorTitulo);
        renderizarPanelJugador(ctx, partida, 0, NULL, NULL);

        /* Botón para volver al menú */
        rectanguloBoton = nk_rect(ventAncho/2 - 150, ventAlto * 0.15f, 300, 50);
        if (nk_begin(ctx, "Continuar", rectanguloBoton, NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "Volver al menú principal"))
            {
                continuar = 1;
            }
        }
        nk_end(ctx);

        /* Renderizar y esperar */
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);
        SDL_Delay(16);
    }

    pantallaJuego = 0;
    juegoActual = NULL;
}

/* Preguntarle al jugador si quiere defenderse con un espejo */
int preguntarUsarEspejoGUI(const Jugador* jugador, TipoCarta cartaAtacante)
{
    SDL_Event evt;
    struct nk_rect rectanguloDialogo;
    struct nk_rect limites;
    struct nk_style_item estiloVisualizacion;

    char mensajeDeAtaque[128];
    const char* nombreDeAtaque;

    int eleccionEspejo = -1;
    int indiceCartaEspejo = -1;
    int i;

    float cartaAncho, cartaAlto, cartaX, cartaY;

    /* Encontrar la posición de la carta espejo */
    for (i = 0; i < MAX_MANO; i++)
    {
        if (jugador->mano[i].tipo == ESPEJO)
        {
            indiceCartaEspejo = i;
            break;
        }
    }

    /* Si no hay carta espejo... */
    if (indiceCartaEspejo < 0)
    {
        return -1;
    }

    /* Mostrar el cuadro de diálogo para usar la carta */
    while (eleccionEspejo == -1 && corriendo)
    {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                corriendo = 0;
                break;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        actualizarTamVent();

        /* Limpiar el fondo */
        glViewport(0, 0, ventAncho, ventAlto);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(fondo.r, fondo.g, fondo.b, fondo.a);

        /* Diálogo de la carta espejo*/
        rectanguloDialogo = crearPanelRectangular(0.15f, 0.25f, 0.7f, 0.5f);
        if (nk_begin(ctx, "¿Usar Carta Espejo?", rectanguloDialogo, NK_WINDOW_NO_SCROLLBAR))
        {
            /* Background styling */
            estiloVisualizacion = ctx->style.window.fixed_background;
            ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(40, 50, 60, 255));

            nk_layout_row_dynamic(ctx, 35, 1);
            nk_label(ctx, "¡Tu oponente está atacando!", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 25, 1);
            nombreDeAtaque = (cartaAtacante == MENOS1) ? "Carta -1" : "Carta -2";
            snprintf(mensajeDeAtaque, sizeof(mensajeDeAtaque), "Recibiste un ataque de: %s", nombreDeAtaque);
            nk_label(ctx, mensajeDeAtaque, NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Tenés una Carta Espejo disponible.", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);

            /* Mostrar la carta espejo */
            nk_layout_row_dynamic(ctx, 90, 1);
            limites = nk_widget_bounds(ctx);
            cartaAncho = 70;
            cartaAlto = 100;
            cartaX = limites.x + (limites.w - cartaAncho) / 2;
            cartaY = limites.y + (limites.h - cartaAlto) / 2;
            dibujarCarta(ctx, cartaX, cartaY, cartaAncho, cartaAlto,
                         ESPEJO, jugador->mano[indiceCartaEspejo].nombre, 1);

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "¿Querés usar tu Carta Espejo para reflejar el ataque?", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 35, 2);
            if (nk_button_label(ctx, "SÍ - Usar Espejo"))
            {
                eleccionEspejo = indiceCartaEspejo;
            }
            if (nk_button_label(ctx, "NO - Perder puntos"))
            {
                eleccionEspejo = -2; // No quiere usar = -2
            }

            /* Resetear el estilo */
            ctx->style.window.fixed_background = estiloVisualizacion;
        }
        nk_end(ctx);

        /* Renderizar */
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(vent);

        /* Poner límite en el frame rate */
        SDL_Delay(16);
    }

    return (eleccionEspejo == -2) ? -1 : eleccionEspejo;
}

/* Comprobar si la interfaz debería seguir ejecutándose */
int interfazSigueCorriendo()
{
    return corriendo;
}
