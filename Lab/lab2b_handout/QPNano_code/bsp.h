/*****************************************************************************
* bsp.h for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/
#ifndef bsp_h
#define bsp_h


                                              


/* bsp functions ..........................................................*/

void init_intc(void);
void init_tmrctr(void);
void init_btngpio(void);
void init_encodergpio(void);
void init_lcdspi(void);
void BSP_init(void);

void rst_tmr(void);
void tmrint_handler(void);
void btnint_handler(void);
void encoderint_handler(void);

#define BSP_showState(prio_, state_) ((void)0)


#endif                                                             /* bsp_h */


