/*-------------------------------------------------------------------------
 *
 *	table.h		Table processing routines
 *
 *	These routines are used to manipulate tables. They comprise a simple
 *	alternative data structure to static arrays or linked lists for
 *	handling data records. It allows the program to easily reference the
 *	table and step through its entries.
 * 
 ***************************************************************************
 * Modification history
 * ----------------------------
 * 03/04/00  Bhawani   Change _DEBUG from DEBUG
 *
 *--------------------------------------------------------------------------
 */

#ifndef TABLE_H_DEFINED
#define	TABLE_H_DEFINED


typedef	int	Int;
typedef	char	Int8;
typedef short	Int16;
typedef long	Int32;
typedef double	Int64;

typedef	unsigned int	uInt;
typedef	unsigned char	uInt8;
typedef unsigned short	uInt16;
typedef unsigned long	uInt32;
typedef		     double	uInt64;

typedef	int		Status;			/* Status codes returned: */
#define	True		1
#define	False		0
#define	Success		True
#define	Failure		False
#ifndef Null
#define	Null		0
#endif

typedef struct Table_t {
	Int16		magic;			/* Magic Marker. */
	Int16		size;			/* Size of a row. */
	Int16		end;			/* End of the table. */
	Int16		last;			/* Last row allocated. */
	Int16		index;			/* current index. */
	Int16		status;			/* Last status of table. */
	Int16		limit;			/* Limit to the table growth. */
	Int16		insert;			/* Buffer insert pointer. */
	char		*data;			/* Pointer to actual data. */
	int		(*sort)(const void *, const void *);	/* Qsort process if any. */
} Table;

#define VALID_TABLE(tbl) (tbl&&((tbl->magic == TABLE_VALID)||(tbl->magic == TABLE_SORT)||(tbl->magic == TABLE_BUFFER)))
#define FREE_TABLE(tbl)  (tbl&&(tbl->magic == TABLE_FREE))
#define	TABLE_VALID	(Int16) 0xAAAA
#define TABLE_SORT	(Int16) 0xBBBB
#define	TABLE_BUFFER	(Int16) 0xCCCC
#define TABLE_FREE	(Int16) 0xF3EE
#define	TABLE_CHUNK	10			/* # of entries per realloc() */
#define CHUNK(siz) (((siz/TABLE_CHUNK)+1)*TABLE_CHUNK)

/* ANSI Function prototypes.  */

extern Table	*table_create(int);
extern Status	 table_free(Table *);
extern void	*table_getRow(Table *, int);
extern void	*table_getValue(Table *, int);
extern void	*table_getFirst(Table *);
extern void	*table_getLast(Table *);
extern void	*table_getPrev(Table *);
extern void	*table_getNext(Table *);
extern Status	 table_getEntry(Table *, int, void *, int);
extern Status	 table_setEntry(Table *, int, void *, int);
extern Status	 table_addEntry(Table *, char *, int);
extern Status	 table_removeEntry(Table *, int);
extern Status	 table_setValue(Table *, int, int);
extern Status	 table_addValue(Table *, int);
extern Status	 table_extend(Table *, int);
extern Status	 table_limit(Table *, int);
extern Status	 table_buffer(Table *, int);
extern Status	 table_status(Table *);
extern int	 table_size(Table *);
extern int	 table_end(Table *);
extern void	*table_find(Table *, int (*)(const void *, const void *), void *);
extern void	*table_find_next(Table *, int (*)(const void *, const void *), void *);
extern Status	 table_sort(Table *, int (*)(const void *, const void *));


#endif /* TABLE_H_DEFINED */

#ifndef DEBUG_H_DEFINED
#define	DEBUG_H_DEFINED

extern	void	debug_msg(char *);
extern	void	debug_stop(char *);
extern	int	debug_print();
extern	int	debug_test(char *, char *, int);
extern	void	debug_index();
extern	void	debug_catalog(char *, char *, int);
extern	int	debug_catalogFind(const void *, const void *);
extern	int	strncasecmp(char *, char *, int);
extern	int	strcasecmp(char *, char *);
extern	char	*strdup(char *);

#ifdef	_DEBUG
#define	DBG(a)	   (debug_test(a, __FILE__, __LINE__))
#define	DMSG(a, b) (debug_test(a, __FILE__, __LINE__) ? (b) : 0)
#else
#define	DBG(a)
#define DMSG(a, b)
#endif

#endif





