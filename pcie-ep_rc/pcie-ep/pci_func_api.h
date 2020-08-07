#ifndef __PCI_FUNC_API_H
#define __PCI_FUNC_API_H

#ifndef TYPE_GV_U8
#define TYPE_GV_U8
typedef unsigned char           GV_U8;
#endif

#ifndef TYPE_GV_U16
#define TYPE_GV_U16
typedef unsigned short          GV_U16;
#endif

#ifndef TYPE_GV_U32
#define TYPE_GV_U32
typedef unsigned int            GV_U32;
#endif

#ifndef TYPE_GV_UL32
#define TYPE_GV_UL32
typedef unsigned long           GV_UL32;
#endif

#ifndef TYPE_GV_S8
#define TYPE_GV_S8
typedef signed char             GV_S8;
#endif

#ifndef TYPE_GV_S16
#define TYPE_GV_S16
typedef short                   GV_S16;
#endif

#ifndef TYPE_GV_S32
#define TYPE_GV_S32
typedef int                     GV_S32;
#endif

#ifndef TYPE_GV_SL32
#define TYPE_GV_SL32
typedef signed long             GV_SL32;
#endif

/*float*/
#ifndef TYPE_GV_FLOAT
#define TYPE_GV_FLOAT
typedef float               	GV_FLOAT;
#endif


/*double*/
#ifndef TYPE_GV_DOUBLE
#define TYPE_GV_DOUBLE
typedef double                  GV_DOUBLE;
#endif

#ifndef TYPE_GV_VOID
#define TYPE_GV_VOID
typedef void                    GV_VOID;
#endif



extern GV_S32 gv_pci_ep_init();
extern GV_S32 gv_pci_ep_read(GV_S8 * ptrReadBuf, GV_U32 readBuflen, GV_S32 ms);
extern GV_S32 gv_pci_ep_write(GV_S8 * ptrWriteBuf, GV_U32 writeSize);
extern GV_VOID gv_pci_ep_deinit();

#endif

