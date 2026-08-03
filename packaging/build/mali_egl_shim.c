/*
 * mali_egl_shim.c
 *
 * LD_PRELOAD shim to make Qt5 eglfs work with Mali fbdev EGL.
 *
 * Mali fbdev EGL (libmali.so) uses a proprietary fbdev_window struct
 * as the native window, and does not support EGL configs with
 * depth/stencil buffers when using the fbdev display.
 *
 * This shim:
 *   1. Intercepts eglCreateWindowSurface() and replaces the native
 *      window handle with a Mali fbdev_window struct.
 *   2. Intercepts eglChooseConfig() and strips unsupported attributes,
 *      with fallback to a minimal attrib list when Mali returns 0 configs.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <execinfo.h>
#include <ucontext.h>
#include <unistd.h>
#include <EGL/egl.h>

static void segv_handler(int sig, siginfo_t *si, void *ctx) {
    ucontext_t *uc = (ucontext_t *)ctx;
    void *pc = NULL;
#ifdef __aarch64__
    pc = (void *)uc->uc_mcontext.pc;
#elif defined(__arm__)
    pc = (void *)uc->uc_mcontext.arm_pc;
#endif
    void *bt[32];
    int n = backtrace(bt, 32);
    fprintf(stderr, "\n[mali_egl_shim] SIGSEGV fault_addr=%p pc=%p\n",
            si->si_addr, pc);
    backtrace_symbols_fd(bt, n, 2);
    _exit(139);
}

/* Mali fbdev native window type */
typedef struct {
    unsigned short width;
    unsigned short height;
} fbdev_window;

static fbdev_window g_mali_window = { 720, 480 };

/* EGL function pointers — resolved lazily on first use */
static void *g_libegl = NULL;

static EGLSurface (*real_eglCreateWindowSurface)(EGLDisplay, EGLConfig,
        EGLNativeWindowType, const EGLint *) = NULL;
static EGLBoolean (*real_eglChooseConfig)(EGLDisplay, const EGLint *,
        EGLConfig *, EGLint, EGLint *) = NULL;
static EGLDisplay (*real_eglGetDisplay)(EGLNativeDisplayType) = NULL;
static EGLBoolean (*real_eglInitialize)(EGLDisplay, EGLint *, EGLint *) = NULL;
static EGLBoolean (*real_eglGetConfigAttrib)(EGLDisplay, EGLConfig,
        EGLint, EGLint *) = NULL;
static EGLBoolean (*real_eglGetConfigs)(EGLDisplay, EGLConfig *,
        EGLint, EGLint *) = NULL;
static EGLBoolean (*real_eglBindAPI)(EGLenum) = NULL;

/* Resolve all real EGL functions via explicit dlopen of libmali */
static void resolve_egl(void) {
    if (real_eglGetDisplay) return;  /* already done */

    /* Try RTLD_NEXT first (works if libmali is loaded before us) */
    real_eglGetDisplay          = dlsym(RTLD_NEXT, "eglGetDisplay");
    real_eglInitialize          = dlsym(RTLD_NEXT, "eglInitialize");
    real_eglCreateWindowSurface = dlsym(RTLD_NEXT, "eglCreateWindowSurface");
    real_eglChooseConfig        = dlsym(RTLD_NEXT, "eglChooseConfig");
    real_eglGetConfigAttrib     = dlsym(RTLD_NEXT, "eglGetConfigAttrib");
    real_eglGetConfigs          = dlsym(RTLD_NEXT, "eglGetConfigs");

    if (!real_eglGetDisplay) {
        /* Fall back: explicitly open libmali */
        const char *libs[] = {
            "/usr/lib/libmali.so",
            "libmali.so",
            "libEGL.so.1",
            "libEGL.so",
            NULL
        };
        for (int i = 0; libs[i] && !g_libegl; i++) {
            g_libegl = dlopen(libs[i], RTLD_NOW | RTLD_GLOBAL);
            if (g_libegl)
                fprintf(stderr, "[mali_egl_shim] opened %s\n", libs[i]);
        }
        if (g_libegl) {
            real_eglGetDisplay          = dlsym(g_libegl, "eglGetDisplay");
            real_eglInitialize          = dlsym(g_libegl, "eglInitialize");
            real_eglCreateWindowSurface = dlsym(g_libegl, "eglCreateWindowSurface");
            real_eglChooseConfig        = dlsym(g_libegl, "eglChooseConfig");
            real_eglGetConfigAttrib     = dlsym(g_libegl, "eglGetConfigAttrib");
            real_eglGetConfigs          = dlsym(g_libegl, "eglGetConfigs");
            real_eglBindAPI             = dlsym(g_libegl, "eglBindAPI");
        }
    }

    if (!real_eglBindAPI)
        real_eglBindAPI = dlsym(RTLD_NEXT, "eglBindAPI");

    fprintf(stderr, "[mali_egl_shim] EGL resolved: GetDisplay=%p Initialize=%p"
            " ChooseConfig=%p CreateWindowSurface=%p\n",
            (void*)real_eglGetDisplay, (void*)real_eglInitialize,
            (void*)real_eglChooseConfig, (void*)real_eglCreateWindowSurface);
}

static void init_shim(void) __attribute__((constructor));
static void init_shim(void) {
    const char *w = getenv("MALI_WINDOW_WIDTH");
    const char *h = getenv("MALI_WINDOW_HEIGHT");
    if (w) g_mali_window.width  = (unsigned short)atoi(w);
    if (h) g_mali_window.height = (unsigned short)atoi(h);

    fprintf(stderr, "[mali_egl_shim] loaded. window=%dx%d\n",
            g_mali_window.width, g_mali_window.height);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = segv_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);

    /* Attempt early resolution — may be NULL if libmali not yet loaded */
    resolve_egl();
}

/* Log all available EGL configs after initialization */
static void dump_egl_configs(EGLDisplay dpy) {
    if (!real_eglGetConfigs || !real_eglGetConfigAttrib) return;
    EGLConfig cfgs[64];
    EGLint n = 0;
    real_eglGetConfigs(dpy, cfgs, 64, &n);
    fprintf(stderr, "[mali_egl_shim] Available EGL configs: %d\n", n);
    for (int i = 0; i < n && i < 16; i++) {
        EGLint r, g, b, a, d, s, surf, rend, id;
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_RED_SIZE,       &r);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_GREEN_SIZE,     &g);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_BLUE_SIZE,      &b);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_ALPHA_SIZE,     &a);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_DEPTH_SIZE,     &d);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_STENCIL_SIZE,   &s);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_SURFACE_TYPE,   &surf);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_RENDERABLE_TYPE,&rend);
        real_eglGetConfigAttrib(dpy, cfgs[i], EGL_CONFIG_ID,      &id);
        fprintf(stderr, "[mali_egl_shim]   cfg[%d] id=%d RGBA=%d%d%d%d d=%d s=%d surf=0x%x rend=0x%x\n",
                i, id, r, g, b, a, d, s, surf, rend);
    }
}

EGLDisplay eglGetDisplay(EGLNativeDisplayType native) {
    resolve_egl();
    if (!real_eglGetDisplay) {
        fprintf(stderr, "[mali_egl_shim] ERROR: real_eglGetDisplay is NULL!\n");
        return EGL_NO_DISPLAY;
    }
    /* Force EGL_DEFAULT_DISPLAY for Mali fbdev */
    EGLDisplay d = real_eglGetDisplay(EGL_DEFAULT_DISPLAY);
    fprintf(stderr, "[mali_egl_shim] eglGetDisplay(native=%p) -> %p\n", (void*)native, d);
    return d;
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
    resolve_egl();
    if (!real_eglInitialize) {
        fprintf(stderr, "[mali_egl_shim] ERROR: real_eglInitialize is NULL!\n");
        return EGL_FALSE;
    }
    EGLBoolean r = real_eglInitialize(dpy, major, minor);
    fprintf(stderr, "[mali_egl_shim] eglInitialize -> %d (EGL %d.%d)\n",
            r, major ? *major : -1, minor ? *minor : -1);
    if (r) dump_egl_configs(dpy);
    return r;
}

/* Force GLES API regardless of what Qt requests */
EGLBoolean eglBindAPI(EGLenum api) {
    resolve_egl();
    /* Always bind GLES — Mali doesn't support desktop EGL_OPENGL_API */
    if (api == EGL_OPENGL_API) {
        fprintf(stderr, "[mali_egl_shim] eglBindAPI: replacing EGL_OPENGL_API with EGL_OPENGL_ES_API\n");
        api = EGL_OPENGL_ES_API;
    } else {
        fprintf(stderr, "[mali_egl_shim] eglBindAPI: api=0x%x\n", api);
    }
    if (!real_eglBindAPI) return EGL_TRUE;  /* assume ES already default */
    return real_eglBindAPI(api);
}

/* Attribute name for logging */
static const char *egl_attr_name(EGLint attr) {
    switch (attr) {
        case 0x3020: return "BUFFER_SIZE";
        case 0x3021: return "ALPHA_SIZE";
        case 0x3022: return "BLUE_SIZE";
        case 0x3023: return "GREEN_SIZE";
        case 0x3024: return "RED_SIZE";
        case 0x3025: return "DEPTH_SIZE";
        case 0x3026: return "STENCIL_SIZE";
        case 0x3031: return "SAMPLE_BUFFERS";
        case 0x3032: return "SAMPLES";
        case 0x3033: return "SURFACE_TYPE";
        case 0x3034: return "TRANSPARENT_TYPE";
        case 0x3040: return "RENDERABLE_TYPE";
        case 0x3042: return "CONFORMANT";
        default: { static char buf[16]; snprintf(buf,16,"0x%x",attr); return buf; }
    }
}

/* Strip depth/stencil/samples from attrib list to get a config Mali accepts */
EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
                            EGLConfig *configs, EGLint config_size,
                            EGLint *num_config)
{
    resolve_egl();
    EGLint filtered[128];
    int fi = 0;

    fprintf(stderr, "[mali_egl_shim] eglChooseConfig attribs:\n");
    if (attrib_list) {
        for (int i = 0; attrib_list[i] != EGL_NONE; i += 2) {
            EGLint attr = attrib_list[i];
            EGLint val  = attrib_list[i + 1];
            fprintf(stderr, "[mali_egl_shim]   %s = %d\n", egl_attr_name(attr), val);

            /* Skip depth, stencil, sample buffers — Mali fbdev doesn't support them */
            if (attr == EGL_DEPTH_SIZE   && val > 0) continue;
            if (attr == EGL_STENCIL_SIZE && val > 0) continue;
            if (attr == EGL_SAMPLE_BUFFERS)           continue;
            if (attr == EGL_SAMPLES)                  continue;

            /* Replace EGL_OPENGL_BIT (8) with EGL_OPENGL_ES2_BIT (4) —
             * Mali only supports GLES, not desktop OpenGL */
            if (attr == EGL_RENDERABLE_TYPE && (val & 0x8) && !(val & 0x4)) {
                fprintf(stderr, "[mali_egl_shim] replacing RENDERABLE_TYPE 0x%x -> EGL_OPENGL_ES2_BIT\n", val);
                val = EGL_OPENGL_ES2_BIT;
            }

            if (fi + 2 < 126) {
                filtered[fi++] = attr;
                filtered[fi++] = val;
            }
        }
    }
    filtered[fi++] = EGL_NONE;

    if (!real_eglChooseConfig) {
        fprintf(stderr, "[mali_egl_shim] ERROR: real_eglChooseConfig is NULL!\n");
        return EGL_FALSE;
    }

    EGLBoolean r = real_eglChooseConfig(dpy, filtered, configs, config_size, num_config);
    fprintf(stderr, "[mali_egl_shim] eglChooseConfig (filtered) -> %d, num_configs=%d\n",
            r, num_config ? *num_config : -1);

    if (r && num_config && *num_config == 0) {
        /* Mali returned no configs — try progressively more permissive fallbacks */
        fprintf(stderr, "[mali_egl_shim] Retrying with minimal GLES2 attrib list\n");

        /* Fallback 1: GLES2 + window bit only */
        EGLint fallback1[] = {
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
        };
        r = real_eglChooseConfig(dpy, fallback1, configs, config_size, num_config);
        fprintf(stderr, "[mali_egl_shim] Fallback1 -> %d, num_configs=%d\n",
                r, num_config ? *num_config : -1);
    }

    if (r && num_config && *num_config == 0) {
        /* Fallback 2: absolutely minimal — let Mali pick anything */
        fprintf(stderr, "[mali_egl_shim] Retrying with empty attrib list\n");
        EGLint fallback2[] = { EGL_NONE };
        r = real_eglChooseConfig(dpy, fallback2, configs, config_size, num_config);
        fprintf(stderr, "[mali_egl_shim] Fallback2 -> %d, num_configs=%d\n",
                r, num_config ? *num_config : -1);
    }

    return r;
}

/* Replace whatever native window Qt passes with a Mali fbdev_window */
EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                   EGLNativeWindowType win,
                                   const EGLint *attrib_list)
{
    resolve_egl();
    fprintf(stderr, "[mali_egl_shim] eglCreateWindowSurface: using fbdev_window %dx%d\n",
            g_mali_window.width, g_mali_window.height);
    if (!real_eglCreateWindowSurface) {
        fprintf(stderr, "[mali_egl_shim] ERROR: real_eglCreateWindowSurface is NULL!\n");
        return EGL_NO_SURFACE;
    }
    return real_eglCreateWindowSurface(dpy, config,
                (EGLNativeWindowType)&g_mali_window, attrib_list);
}
