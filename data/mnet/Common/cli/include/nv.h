#if !defined(__NV_H__)
#define __NV_H__



extern void nv_init(void);

extern void nv_configure(uchar);
extern int nv_current_to_nv (boolean vocal);
extern char* nv_current_to_buffer(boolean, uchar, boolean);
extern void nv_review(parseinfo *);

extern STATUS write_nv_config_from_fd( int );

extern void nv_add (boolean , char *, ...);
extern void nv_write (boolean , char *, ...);
extern int nv_erase(boolean);

extern boolean ok_to_write_nv(void);

#endif /* !defined(__NV_H__) */
