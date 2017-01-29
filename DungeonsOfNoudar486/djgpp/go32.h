#include <SDL.h>

#ifndef TRUECOLOR_ONLY
 #define TRUECOLOR_ONLY 1
 #define REPLAY         0
#endif

// ^ This dummy autoincrementing timer provides suitable testing with valgrind.
static struct{ int operator()(...){return 0;} operator int() const { return 0; } }
    C80,_dos_ds,_my_ds,  _farsetsel,
    __dpmi_physical_address_mapping, __dpmi_lock_linear_region,
    __dpmi_set_segment_base_address, __dpmi_set_segment_limit,
    __dpmi_allocate_ldt_descriptors;
struct __dpmi_meminfo { unsigned size, address; };

static SDL_Surface* surface = 0;
static unsigned char vmem[(PC::W * PC::H)];
static unsigned palette[256] = { 0 }, palettecounter=0;
static void _farnspokeb(unsigned o, unsigned pix)
{
    vmem[ (o&0xFFFF) % (PC::W * PC::H) ] = pix;
}
static void outportb(unsigned o, unsigned value)
{
    if(o != 0x3C9) return;
    value = (value << 2) | (value >> 4);
    unsigned& v = palette[palettecounter/3];
    unsigned shift = 16 - (palettecounter%3)*8;
    v &= ~ (0xFF << shift);
    v |=  (value << shift);
    palettecounter = (palettecounter+1) % 768;
}

bool DisplayTrueColor = true || TRUECOLOR_ONLY, GrabWindow = true;

static void textmode(int mode)
{
    if(mode > 3)
    {
        // set graphics
        surface = SDL_SetVideoMode(PC::W, PC::H *
             ( (DisplayTrueColor?1:0)
             + (TRUECOLOR_ONLY  ?0:1)
               ), 32,
            0/*SDL_OPENGL*/);
        SDL_EnableKeyRepeat(150, 30);
      #if !REPLAY
        if(GrabWindow)
        {
            SDL_WM_GrabInput(SDL_GRAB_ON);
            SDL_ShowCursor(SDL_DISABLE);
        }
      #endif
    }
    else
    {
        // unset graphics
        SDL_Quit();
    }
}
static void updatescreen(unsigned* buf)
{
    #if REPLAY && TRUECOLOR_ONLY
        {/*static FILE* fp = popen("lzop -9 > framesT.lzop", "w");
        fwrite(buf, PC::W*PC::H, 4, fp);
        fflush(fp);*/
        return;}
    #endif

    SDL_LockSurface(surface);

#if !TRUECOLOR_ONLY
    if(DisplayTrueColor)
#endif
        memcpy( surface->pixels, buf, PC::W*PC::H*4 );

#if !TRUECOLOR_ONLY
    unsigned* target = PC::W*PC::H*DisplayTrueColor + (unsigned*)surface->pixels;
    for(unsigned p=0; p<PC::W*PC::H; ++p)
        target[p] = palette[vmem[p]];
#endif

    #if REPLAY && !TRUECOLOR_ONLY
        static FILE* fp = popen("lzop -7 > frames.lzop", "w");
        fwrite(target, PC::W*PC::H, 4, fp);
        fflush(fp);
    #endif

    SDL_UnlockSurface(surface);
    /*static int flip=0;
    if(flip++ % 2  == 0)*/
    SDL_Flip(surface);
}
static SDL_Event pending_ev;
static int nbuttonpresses[8] = { 0 };
static int kbhit()
{
    if(pending_ev.type == SDL_KEYDOWN) return 1;
    if(!SDL_PollEvent(&pending_ev)) return 0;
    if(pending_ev.type == SDL_MOUSEBUTTONDOWN)
        nbuttonpresses[pending_ev.button.button % 8] += 1;
    if(pending_ev.type == SDL_QUIT) SDL_Quit();
    if(pending_ev.type != SDL_KEYDOWN)
    {
        pending_ev.type = 0;
        return kbhit();
    }
    return 1;
}
static int getch()
{
    if(pending_ev.type == SDL_KEYDOWN)
        { pending_ev.type = 0; return pending_ev.key.keysym.sym; }
    SDL_Event ev;
    SDL_WaitEvent(&ev);
    if(ev.type == SDL_MOUSEBUTTONDOWN)
        nbuttonpresses[ev.button.button % 8] += 1;
    if(ev.type == SDL_KEYDOWN) return ev.key.keysym.sym;
    return getch();
}


union __dpmi_regs
{
    struct { short ax,bx,cx,dx; } x;
};
void __dpmi_int(int n, __dpmi_regs* r)
{
    if(n == 0x33)
        switch(r->x.ax)
        {
            case 0x0: break;
            case 0xB:
            {
                int x,y;
                static int count=2;
                SDL_GetRelativeMouseState(&x,&y);
                if(count > 0) { r->x.cx = r->x.dx = 0; --count; break; }
                r->x.cx = x;
                r->x.dx = y;
                break;
            }
            case 0x5:
            {
                int trans[3] = {1,3,2};
                int n = trans[r->x.bx%3];
                r->x.bx = nbuttonpresses[n];
                nbuttonpresses[n] = 0;
                break;
            }
        }
    else if(n == 0x10 && r->x.ax == 0x4F02)
        textmode(r->x.bx & 0xFFF);
}

static void movedata(unsigned,long src,unsigned,long, unsigned)
{
    updatescreen( (unsigned*)src );
}
