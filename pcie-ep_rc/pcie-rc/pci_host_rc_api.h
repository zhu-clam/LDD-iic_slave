#ifndef __PCI_HOST_RC_API_H
#define __PCI_HOST_RC_API_H

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


typedef struct
{
	GV_U8 * ptr;
	GV_UL32	size;
	GV_U32	dma;
}GV_PCI_TRANSMIT_S;


extern GV_S32 gv_host_pci_rc_init();
extern GV_S32 gv_host_pci_rc_write(GV_PCI_TRANSMIT_S *pst_pci_st);
extern GV_S32 gv_host_pci_rc_read(GV_PCI_TRANSMIT_S *pst_pci_st);
extern GV_S32 gv_host_pci_rc_deinit();

#endif
