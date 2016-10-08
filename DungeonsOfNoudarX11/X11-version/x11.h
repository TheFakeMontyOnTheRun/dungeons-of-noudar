#ifndef X11_COMMON_H
#define X11_COMMON_H

extern void make_x_window(Display *x_dpy, EGLDisplay egl_dpy,
              const char *name,
              int x, int y, int width, int height,
              Window *winRet,
              EGLContext *ctxRet,
              EGLSurface *surfRet);


extern void event_loop(Display *dpy, Window win,
           EGLDisplay egl_dpy, EGLSurface egl_surf );
#endif
