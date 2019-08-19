/* PDCurses */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <curspriv.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <signal.h>
#include <ctype.h>
#include <pwd.h>

#include <sys/types.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>   /* AIX needs this for FD_ZERO etc macros */
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <Intrinsic.h>
#include <StringDefs.h>
#include <Shell.h>

#ifdef USE_XAW3D
# include <Xaw3d/Box.h>
# include <Xaw3d/Scrollbar.h>
#elif defined(USE_NEXTAW)
# include <neXtaw/Box.h>
# include <neXtaw/Scrollbar.h>
#else
# include <Xaw/Box.h>
# include <Xaw/Scrollbar.h>
#endif
#include "ScrollBox.h"

#include "Xmu/StdSel.h"
#include "Xmu/Atoms.h"

#include <keysym.h>
#include <Xatom.h>

#define XCURSCR_Y_SIZE      (XCursesLINES * XCursesCOLS * sizeof(chtype))
#define XCURSCR_FLAG_SIZE   (XCursesLINES * sizeof(int))
#define XCURSCR_START_SIZE  (XCursesLINES * sizeof(int))
#define XCURSCR_LENGTH_SIZE (XCursesLINES * sizeof(int))
#define XCURSCR_ATRTAB_SIZE (PDC_COLOR_PAIRS * 2 * sizeof(short))
#define XCURSCR_SIZE        (XCURSCR_FLAG_SIZE + XCURSCR_START_SIZE + \
        XCURSCR_LENGTH_SIZE + XCURSCR_Y_SIZE + XCURSCR_ATRTAB_SIZE + \
        sizeof(XColor))

#define XCURSCR_Y_OFF(y)    ((y) * XCursesCOLS * sizeof(chtype))
#define XCURSCR_FLAG_OFF    (XCURSCR_Y_OFF(0) + XCURSCR_Y_SIZE)
#define XCURSCR_START_OFF   (XCURSCR_FLAG_OFF + XCURSCR_FLAG_SIZE)
#define XCURSCR_LENGTH_OFF  (XCURSCR_START_OFF + XCURSCR_START_SIZE)
#define XCURSCR_ATRTAB_OFF  (XCURSCR_LENGTH_OFF + XCURSCR_LENGTH_SIZE)
#define XCURSCR_XCOLOR_OFF  (XCURSCR_ATRTAB_OFF + XCURSCR_ATRTAB_SIZE)

typedef struct
{
    int lines;
    int cols;
    Pixel cursorColor;
    Pixel colorBlack;
    Pixel colorRed;
    Pixel colorGreen;
    Pixel colorYellow;
    Pixel colorBlue;
    Pixel colorMagenta;
    Pixel colorCyan;
    Pixel colorWhite;
    Pixel colorBoldBlack;
    Pixel colorBoldRed;
    Pixel colorBoldGreen;
    Pixel colorBoldYellow;
    Pixel colorBoldBlue;
    Pixel colorBoldMagenta;
    Pixel colorBoldCyan;
    Pixel colorBoldWhite;
    Pixel pointerForeColor;
    Pixel pointerBackColor;
    XFontStruct *normalFont;
    XFontStruct *italicFont;
    XFontStruct *boldFont;
    char *bitmap;
    char *pixmap;
    Cursor pointer;
    int borderWidth;
    int borderColor;
    int clickPeriod;
    int doubleClickPeriod;
    int scrollbarWidth;
    int cursorBlinkRate;
    char *textCursor;
    int textBlinkRate;
} XCursesAppData;

extern XCursesAppData xc_app_data;

#define XCLOGMSG ("")

int PDC_display_cursor(int, int, int, int, int);

void XCursesCursor(int, int, int, int);
void XCursesDisplayCursor(void);
void XCursesTitle(const char *);

unsigned long XCursesKeyPress(XEvent *);
unsigned long XCursesMouse(XEvent *);

int XCursesInitscr(int, char **);
int XCursesSetupX(int, char **);
void XCursesExit(void);

void XC_resize(void);
void XC_refresh_screen(void);
void XC_refresh_scrollbar(void);
void XC_set_blink(bool);
XColor XC_get_color(short);
void XC_set_color(short, XColor);
void XC_get_selection(void);
int XC_set_selection(const char *, long);

#ifdef _HPUX_SOURCE
# define FD_SET_CAST int *
#else
# define FD_SET_CAST fd_set *
#endif

extern XtAppContext app_context;
extern Widget topLevel;
extern fd_set xc_readfds;

extern unsigned char *Xcurscr;
extern int XCursesLINES;
extern int XCursesCOLS;

typedef void (*signal_handler)();

signal_handler XCursesSetSignal(int, signal_handler);

#ifdef PDCDEBUG
void XC_say(const char *msg);
# define XC_LOG(x) XC_say x
#else
# define XC_LOG(x)
#endif

#ifdef MOUSE_DEBUG
# define MOUSE_LOG(x) printf x
#else
# define MOUSE_LOG(x)
#endif

extern short *xc_atrtab;

extern bool xc_resize_now;
extern char *xc_selection;
extern long xc_selection_len;