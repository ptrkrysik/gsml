/*--------------------------------------------------------------------------
 *
 *	table.c		Table handling routines
 *
 *	This is a collection of routines that provide a simple method of
 *	handling a set of data structures. 
 *
 *	Add more and a simple tutorial.
 *
 *	tbl = table_create(sizeof(TableEntry))
 *	table_free(tbl)
 *	
 *	Another way to look at the difference in this routine is where is in
 *	who's process space does the memory block reside? When one grabs the
 *	pointer to a row of a table, the table is managed by the SNMP manager
 *	and a pointer to the "live" data is returned. On the other hand, when
 *	one uses this function, rather than refering to the "live" data, a
 *	copy of the data is made into the buffer the task specified.
 *
 *	You may have noticed that table_setRow is the only set function for
 *	the table routines. Since the table_getNth routines return a pointer
 *	to the table row, it is not necessary to have an equivalent set of
 *	functions to write the data. The following code fragment illustrates
 *	the difference:
 *
 *	typedef struct Entry_t {
 *		int	item1;
 *		int	item2;
 *		int	item3;
 *	} Entry;
 *
 *	Table		*tbl;
 *	Entry		entry,	*row;
 *	int		idx;
 *
 *	tbl = table_alloc(sizeof(Entry));
 *
 *	The first method of loading entries into the table:
 *
 *	idx = -1;
 *	while (++idx < 10) {
 *		entry.item1=1;
 *		entry.item2=2;
 *		entry.item3=3;
 *		table_setEntry(tbl, idx, &entry, sizeof(Entry));
 *	}
 *
 *	The second method of loading entries into the table:
 *
 *	table_extend(tbl, 10);		 * Extend the table to handle 10 rows.
 *	idx = -1;			 * Otherwise table_getNth will return
 *	while (++idx < 10) {		 * a NULL ptr if the row is not there.
 *		row = (Entry *)table_getRow(tbl, idx);
 *		row->item1=1;
 *		row->item2=2;
 *		row->item3=3;
 *	}
 *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 *
 *	table_create	Allocate and initialize the Table control structure.
 *
 *	tbl = table_create(row_size)
 *		Table	*tbl;		 * Pointer to a table.
 *		int	row_size;	 * size of a row.
 *
 *	Return:
 *	    tbl =  A pointer to the initialized Table.
 *	    null = Unable to allocate the memory to initialize the table.
 *	     
 *	This routine is used to allocate a table. Tables consist of a table
 *	control structure and an array that contains the data in the table.
 *	When you create the table, you need to specify the size of the rows
 *	in the table. This is used when the table_* routines need to extend
 *	the table to accomidate new entries.
 *
 *--------------------------------------------------------------------------*/
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	"table.h"

Table	*
table_create(
    int		size)
{
    Table	*tbl;

	if (!(tbl = (Table *) malloc(sizeof(Table))))
		return False;

	memset(tbl, 0, sizeof(Table));	/* malloc does not clear memory. */
	tbl->magic = (Int16) TABLE_VALID;	/* Mark this as a table. */
	if (size < 4)			/* Enforce a minimum limit. 4 bytes */
	  size = 4;			/* is the same as a 32 bit word. */
	tbl->size = size;		/* Save the row size. */
	tbl->end = -1;			/* Nothing is in the table yet so*/
	tbl->index = -1;		/* all the indicies to indicate this */
	tbl->status = Success;		/* Mark this operation as successful.*/
	tbl->insert = -1;		
	return tbl;
}

/*--------------------------------------------------------------------------
 *
 *	err = table_free(tbl)
 *		Status	err;		 * Success or Failure (invalid tbl).
 *		Table	*tbl;		 * Pointer to a table.
 *
 *	This routine is used to release a table when it is no longer in use.
 *	This will deallocate and return the memory used to store the table
 *	back to the system.
 *
 *--------------------------------------------------------------------------*/

Status
table_free(
    Table	*tbl)
{
	if (!tbl || (tbl->magic != TABLE_VALID))
	    return Failure;

	tbl->magic = (Int16) TABLE_FREE;	/* Mark the table as free. */
	if (tbl->data)			/* Then free the data array. */
	  free(tbl->data);		/* Followed by the table control */
	free(tbl);			/* structure. */

	return Success;
}

/*--------------------------------------------------------------------------
 *
 *	table_getRow	Return a pointer to the particular row in a table.
 *
 *	row = table_getRow(tbl, idx);
 *		void	*row;		 * Pointer to the entry for the row.
 *		Table	*tbl;		 * Pointer to the table
 *		int	idx;		 * Index of the desired row.
 *
 *	This is a set of routines that are used to recover a pointer to the
 *	each row of data that is stored in the table. As the name of each
 *	routine indicates, they allow various ways to walk through the
 *	contents of the table. For this set of routines, there is no status
 *	return. Since these routines return a pointer to the row entry, if
 *	there was an error (i.e. an invalid table or no more entries), then
 *	a null pointer is returned.
 *
 *--------------------------------------------------------------------------*/

void	*
table_getRow(
    Table	*tbl,
    int		idx)
{
    void	*addr;

	if (!VALID_TABLE(tbl))
	    return Failure;


	if (idx < 0 || (idx > tbl->end)) {
	    tbl->status = Failure;
	    return Failure;
	}
	if (!tbl->data) {		/* Logically can't happen, parinoia abounds */
	    tbl->status = Failure;
	    return Failure;
	}
	
	if (tbl->magic == TABLE_SORT) {	/* Does the table need sorting? */
	    if (tbl->sort) {
		DMSG("sort", printf("Calling sort procedure. qsort(tbl->data @%x, tbl->end %d, tbl->size %d, tbl->sort @%x\n",
			(int) tbl->data, tbl->end+1, tbl->size, (int) tbl->sort)); 
		qsort(tbl->data, tbl->end+1, tbl->size, tbl->sort);
	    }
	    tbl->magic = TABLE_VALID;
	}
	tbl->index = idx; 
	addr = (void *) ((int)tbl->data + (idx*tbl->size));
	return addr;
}

/*--------------------------------------------------------------------------
 *
 *	table_rowPointer Return a pointer to the particular row in a table.
 *
 *	row = table_rowPointer(tbl, idx);
 *		void	*row;		 * Pointer to the entry for the row.
 *		Table	*tbl;		 * Pointer to the table
 *		int	idx;		 * Index of the desired row.
 *
 *	This is almost identical to the above routine. However, it does not
 *	cause the table to be sorted if it is out of order. This is called
 *	by the table_setEntry and table_setValue routines. When we set values,
 *	we do not worry if the table is in order. The magic flag will be get
 *	set to indicate the table is out of order. This will cause the table
 *	to be sorted before an item is retrieved with either table_getValue
 *	or table_getRow.
 *
 *--------------------------------------------------------------------------*/

void	*
table_rowPointer(
    Table	*tbl,
    int		idx)
{
    void	*addr;

	if (!VALID_TABLE(tbl))
	    return Failure;

	if (idx < 0 || (idx > tbl->end)) {
	    tbl->status = Failure;
	    return Failure;
	}

	if (!tbl->data) {	/* Logically can't happen, parinoia abounds */
	    tbl->status = Failure;
	    return Failure;
	}

	tbl->index = idx;
	addr = (void *) ((int)tbl->data + (idx * tbl->size));
	return addr; 
}
/*--------------------------------------------------------------------------
 *
 *	table_getValue Return a value or a pointer to a table entry.
 *
 *	value = table_getValue(tbl, idx);
 *		void	*value;		 * Pointer to the entry for the row.
 *		Table	*tbl;		 * Pointer to the table
 *		int	idx;		 * Index of the desired row.
 *
 *	This is a specialized form of the table_getRow call. A very handy
 *	use of the table routines is to manage a array of opaque 32 bit values
 *	such as pointers to data structures. There is special handling for the
 *	table in this case. Essentially, if the table is 4 or less bytes wide,
 *	we return the contents of the table item rather than a pointer to the
 *	contents of the data item. In this case table_getRow() will return a
 *	pointer to the table item rather than the contents. These types of
 *	tables can be sorted as well, in which case the qsort function has to
 *	take into account the indirection of the arguments.
 *
 *--------------------------------------------------------------------------*/

void	*
table_getValue(
    Table	*tbl,
    int		idx)
{
    void	*value;

	if ((value = table_getRow(tbl, idx)) &&
	    (tbl->size <= 4))
		value = (void *)*(void **)value;

	return value;
}

/*--------------------------------------------------------------------------
 *
 *	table_get.... Retrieve various entries in the table.
 *
 *	row = table_getFirst(tbl);
 *	row = table_getLast(tbl);
 *	row = table_getNext(tbl);
 *	row = table_getPrev(tbl);
 *
 *	These are variations of the table_getRow routine. They are used to
 *	sequentially walk through the table. The functionality is similar to
 *	walking through a doublely linked list.
 *
 *--------------------------------------------------------------------------*/
void	*
table_getFirst(
    Table	*tbl)
{					   /* Just get the first entry in the*/
	return table_getRow(tbl, 0);	   /* table. getRow does the validate*/
}

void	*
table_getLast(				   /* This time, we need to look into*/
    Table	*tbl)			   /* tbl header, so we have to make */
{					   /* shure we have a valid header.  */
	if (!VALID_TABLE(tbl))
	    return Failure;

	return table_getRow(tbl, tbl->end);
}

void	*
table_getPrev(				   /* This one works off of tbl->idx*/
    Table	*tbl)			   /* which reflects the cur entry. */
{
	if (!VALID_TABLE(tbl))
	    return Failure;

	return table_getRow(tbl, tbl->index-1);
}

void	*
table_getNext(				   /* This one retrieves that next */
    Table	*tbl)			   /* sequential entry in the table. */
{
	if (!VALID_TABLE(tbl))
	    return Failure;

	return table_getRow(tbl, tbl->index+1);
}

/*--------------------------------------------------------------------------
 *
 *	err = table_getEntry(tbl, idx, row, size);
 *		Table	*tbl;		 * Pointer to the table.
 *		int	idx;		 * The desired row index.
 *		char	*row;		 * Buffer to receive entry data.
 *		int	size;		 * Size of the buffer.
 *
 *	The row entry itself can be retrieved, rather than a pointer to the
 *	row. At this level, the entry is seen as some number of bytes that
 *	are to be stores into a buffer. The size parameter specifies the
 *	size of the buffer that is being copied into. It may be less than
 *	the size specified for a row in the initial table_alloc() function.
 *	However, in this case, the state of the "extra" bytes at the end of
 *	the entry is undetermined. In any case, the smaller of the two values
 *	is used to determine the amount of data copied.
 *
 *--------------------------------------------------------------------------*/

Status
table_getEntry(
    Table	*tbl,			/* Pointer to the table. */
    int		idx,			/* The desired row index. */
    void	*row,			/* Buffer to receive entry data. */
    int		size)			/* Size of the buffer. */
{
    void	*ent;

	if ((ent = table_getRow(tbl, idx)) == Failure)
	    return Failure;

	memcpy(row, ent, tbl->size < size ? tbl->size : size);

	tbl->status = Success;
	return Success;
}

/*--------------------------------------------------------------------------
 *
 *	err = table_setEntry(tbl, idx, row, size);
 *		Table	*tbl;		 * Pointer to the table.
 *		int	idx;		 * The desired row index.
 *		Entry	*row;		 * Buffer to receive entry data.
 *		int	size;		 * Size of the buffer.
 *
 *	This function is the coverse of getRow. This is used to set the
 *	value of a row. The contents of the entry is simply copied to the
 *	table. The lesser of the buffer size or the row size specified when
 *	the table was created governs the amount of data that is copied.
 *
 *--------------------------------------------------------------------------*/

Status
table_setEntry(
    Table	*tbl,
    int		idx,
    void	*row,
    int		size)
{

    void	*ent;

	if (!VALID_TABLE(tbl))
	    return Failure;

	if (idx >= tbl->last)		 /* Is the table large enough? */
	    if (table_extend(tbl, idx) == Failure) {	/* No, extend it. */
		DMSG("table", printf("table_setEntry: TableExtend failed.\n"));
		return Failure;				/* Oops, alloc failed*/
	    }
	if ((ent = table_rowPointer(tbl, idx)) == Failure) {
	    DMSG("table", printf("table_setEntry: table_rowPointer(tbl, %d) failed!\n", idx));
	    return Failure;
	}
	memcpy(ent, row, tbl->size < size ? tbl->size : size);

	if (tbl->sort)			/* Is this a sorted table ? */
	    tbl->magic = TABLE_SORT;	/* Yes, trigger a qsort. */

	if (idx > tbl->end)		/* Keep track of the largest */
	    tbl->end = idx;		/* entry allocated in tbl->data */

	tbl->index = idx;
	tbl->status = Success;
	tbl->insert = idx;
	return Success;
}

/*--------------------------------------------------------------------------
 *
 *	err = table_addEntry(tbl, row, size);
 *		Table	*tbl;		 * Pointer to the table.
 *		Entry	*row;		 * Buffer to receive entry data.
 *		int	size;		 * Size of the buffer.
 *
 *	This function is used to add an entry to the end of a list. We
 *	simply call table_setEntry with the tbl->end+1 as the index. If
 *	this is a sorted list, table_setEntry will set the TABLE_SORT
 *	magic flag so that the table will get sorted prior to get access.
 *
 *--------------------------------------------------------------------------*/

Status
table_addEntry(
    Table	*tbl,
    char	*row,
    int		size)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	if (tbl->end+1 >= tbl->last)		 /* Is the table large enough? */
	    if (table_extend(tbl, tbl->end+1) == Failure) {	/* No, extend it. */
		DMSG("table", printf("table_addEntry: TableExtend failed.\n"));
		return Failure;				/* Oops, alloc failed*/
	    }
	return table_setEntry(tbl, ++tbl->end, row, size);
}

Status
table_addValue(
    Table	*tbl,
    int		value)
{
    void	*row;
    int		size;

	if (!VALID_TABLE(tbl))
	    return Failure;

	row = (void *)&value;
	size = sizeof(int);

	if (++tbl->end >= tbl->limit) 
	    if (tbl->magic == TABLE_BUFFER) 
		bcopyLongs(tbl->data + 4, tbl->data, tbl->end--);

	return table_setEntry(tbl, tbl->end, row, size);
}

Status
table_removeEntry(
    Table	*tbl,
    int		row)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	if ((row < 0) || (row > tbl->end)) {
		tbl->status = Failure;
	    return Failure;
	}

	if (row == tbl->end) {		/* Is this the last item in the table ? */
	    tbl->end--;			/* Yes, just reduce the end pointer to  */
	    tbl->status = Success;	/* eliminate the item.			*/
	    return Success;
	}

	tbl->index = row;
	bcopy(tbl->data+((row+1)*tbl->size), tbl->data+(row*tbl->size), (tbl->end+1-row)*tbl->size);

	tbl->end--;
	tbl->status = Success;

	return Success;
}

Status
table_buffer(
    Table	*tbl,
    int		rows)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	tbl->magic = TABLE_BUFFER;
	tbl->end = -1;
	tbl->insert = -1;
	tbl->limit = rows;

	DMSG("table", printf("table_buffer('0x%x', %d) end= %d\n", (int)tbl, rows, tbl->end) );

	return Success;
}

/*--------------------------------------------------------------------------
 *
 *	table_extend	Extend the size of a table.
 *
 *	err = table_extend(tbl, rows)
 *		Table	*tbl;		 * Pointer to a table.
 *		int	rows;		 * Maximum number of rows to accomidate
 *
 *	Return:
 *	    Success = the table is large enough to handle the desired # of rows
 *	    Failure = invalid table pointer was passed or we could not assign
 *		      enough memory with malloc or realloc to extend the table.
 *
 *	This function is used to extent the table to accomidate a certain
 *	number of entries. This function can be used to pre-allocate a large
 *	table. The table is normally extended in small chunks to accomidate
 *	new entries. For example, if the table has 10 entries in it and the
 *	11th entry is referenced, the table may be extended to accomidate 20
 *	entries. Since the table itself must be contiguous, realloc() is called
 *	to allocate additional memory space which could cause a bcopy() to
 *	be performed. If the table is not very big, this processing is not
 *	significant and can be ignored. However, if the table has thousands
 *	of entries in it, the overhead can be reduced by simply extending the
 *	table to the maximum size in a single operation.
 *
 *--------------------------------------------------------------------------*/

Status
table_extend(
    Table	*tbl,			/* Pointer to the table. */
    int		rows)			/* Maximum rows to accomidate. */
{
    int		bytes;
    char	*buf;

	if (!VALID_TABLE(tbl))
	    return Failure;

	if (tbl->limit && (rows > tbl->limit))
	    return Failure;

	DMSG("table", printf("table_extend('0x%x', %d), last= %d\n", (int)tbl, rows, tbl->last) );

	if ((bytes = CHUNK(rows)) > tbl->last) {
	    bytes *= tbl->size;		/* How much memory is really needed? */
	    if ((buf = (char *) (tbl->data ? realloc(tbl->data, bytes)
					   : malloc(bytes)))) {
		tbl->data = buf;
		memset(buf+(tbl->last*tbl->size), 0, bytes-(tbl->last*tbl->size));
		tbl->last = CHUNK(rows);
		DMSG("table", printf("table_extend: tbl->data='0x%08x', tbl->last=%d\n", (int)buf, tbl->last));
	   } else {
		DMSG("table", printf("table_extend: alloc failed!!! tbl->data='0x%08x', tbl->last=%d\n", (int)buf, tbl->last));
		tbl->status = Failure;		/* Oops, alloc failed*/
		return Failure;
	    }
	}
	tbl->status = Success;		/* Mark this operation as successful.*/
	return Success;
}

/*--------------------------------------------------------------------------
 *
 *	table_limit	Limit the growth of a table.
 *
 *	err = table_limit(tbl, rows)
 *		Table	*tbl;
 *		int	rows;
 *
 *	Return:
 *	    Success = table limit was set.
 *	    Failure = invalid table pointer was passed.
 *
 *	This function is used to limit the size of a table. The functions
 *	table_getRow() and table_setEntry() automatically extend the tables
 *	as needed to accomidate the entry that is being referenced. This is
 *	used to specify the absolute limit that the table may grow to. If no
 *	limit is specified, the table will be allowed to grow as needed.
 *
 *--------------------------------------------------------------------------*/

Status
table_limit(
    Table	*tbl,
    int		rows)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	table_extend(tbl, rows);	/* First extend the table. */
	tbl->limit = rows;		/* Limit the table from further growth */
	tbl->end = rows;
	tbl->status = Success;		/* Mark this operation as successful */

	DMSG("table", printf("table_limit('0x%x', %d) end= %d\n", (int)tbl, rows, tbl->end) );

	return Success;
}


/*--------------------------------------------------------------------------
 *
 *	table_status	Return the status of the last table operation.
 *
 *	err = table_status(tbl)
 *		Table	*tbl;		 * Pointer to a table.
 *
 *	This function is used to return the status of the last operation on
 *	the particular table. A number of routines return a pointer to the
 *	entry or a NULL in the event of an error. This can be used to identify
 *	the particular error. For example, if table_getNext(tbl) returned a
 *	NULL entry, it could be because it was passed an invalid table pointer
 *	or because it ran out of entries to return. This can be used to see
 *	the precise error in such a case.
 *
 *--------------------------------------------------------------------------*/

Status
table_status(
    Table	*tbl)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	return (tbl->status);
}

/*--------------------------------------------------------------------------
 *
 *	table_size	Return the size of an entry in the table.
 *
 *	size = table_size(tbl)
 *		Table	*tbl;		 * Pointer to a table.
 *
 *	This function is used to return the size of the entries in the table
 *	or the width of the rows of the table. This value is normally set
 *	when the table is created.
 *
 *--------------------------------------------------------------------------*/

int
table_size(
    Table	*tbl)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	return (tbl->size);
}

/*--------------------------------------------------------------------------
 *
 *	table_end	Return the end (last item) of the table.
 *
 *	lastItem = table_end(tbl)
 *		Table	*tbl;		 * Pointer to a table.
 *
 *	This function is used to return the last item in the table. This
 *	can be used to find the number of items in the table.
 *
 *--------------------------------------------------------------------------*/
int
table_end(
    Table	*tbl)
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	return (tbl->end);
}

/*--------------------------------------------------------------------------
 *
 *	table_sort	Specify a qsort procedure for a table.
 *
 *	size = table_sort(tbl, proc)
 *		Table	*tbl;		 * Pointer to a table.
 *		void	*proc();	 * Qsort process.
 *
 *	This function is used to set the qsort proc for a table. All we have
 *	to do is store the proc handle in the table header and set the magic
 *	flag to TABLE_SORT. This indicates that the table needs to be sorted
 *	prior to getValue returning an item.
 *
 *--------------------------------------------------------------------------*/

Status
table_sort(
    Table	*tbl,
    int		(*sort)(const void *, const void *))
{

	if (!VALID_TABLE(tbl))
	    return Failure;

	tbl->sort = sort;		/* Save the sort procedure.          */
	tbl->magic = TABLE_SORT;	/* Trigger the initial sort.         */
	tbl->status = Success;		/* Mark this operation as successful */
	return Success;		
}

/*--------------------------------------------------------------------------
 *
 *	table_find	Find an entry in a table.
 *
 *	addr = table_find(tbl, test, arg)
 *	addr = table_find_next(tbl, test, arg)
 *		Table	*tbl;		 * Pointer to a table.
 *		void	(*test)(void*, void*);	 * Qsort like test process.
 *		void	*arg;		 * Item we are looking for.
 *
 *	This function is used to set the qsort proc for a table. All we have
 *	to do is store the proc handle in the table header and set the magic
 *	flag to TABLE_SORT. This indicates that the table needs to be sorted
 *	prior to getValue returning an item.
 *
 *--------------------------------------------------------------------------*/

void *
table_find(
    Table	*tbl,
    int	(*test)(const void *,const void *),
    void	*arg)
{
	if (!VALID_TABLE(tbl))
	    return Failure;

	tbl->index = -1;			/* Start at the top of the table. */
	return table_find_next(tbl, test, arg);
}

void *
table_find_next(
    Table	*tbl,
    int		(*test)(const void *, const void *),
    void	*arg)
{
#if LATER
    void	*addr, *tst;
    int		i, beg, end, t;
#else
    void	*addr;
    int		i;
#endif

	if (!VALID_TABLE(tbl))
	    return Failure;

	if (!tbl || !((tbl->magic == TABLE_VALID) ||
		      (tbl->magic == TABLE_SORT)))
	    return Failure;

	if (tbl->sort && (tbl->magic == TABLE_SORT)) {
	    qsort(tbl->data, tbl->end+1, tbl->size, tbl->sort);
	    tbl->magic = TABLE_VALID;
	}

#if LATER
	if (tbl->sort) {		/* Is this a sorted table ? */
	    beg = i = tbl->index;	/* Yes, we can do a binary  */
	    end = tbl->end;		/* search on the data.      */
	    while (beg < end) {
		addr = (void *) ((int)tbl->data + (i * tbl->size));
		t = (*test)(arg, addr);
		DMSG("table", printf("table_find(): testing item %d, beg %d, end %d, t %d\n",
			i, beg, end, t) );
		if (t == 0) {
		    /* Exact Match. Well almost, there can be duplicate
		     * entries in a sorted table. If this is the case,
		     * then we need to back up in the list to the first
		     * instance of this entry.
		     * Note: the beg index has been moved. tbl->index
		     *       contains the start of the table segment
	             *       we are searching. */

		    while (i > tbl->index) {
			tst = (void *) ((int)tbl->data + ((i-1) * tbl->size));
			if ((*test)(arg, tst))
			    break;	/* This index is ok. */
			addr = tst;	/* This is a duplicate index*/
			i--;		/* go back one item and test*/
			DMSG("table", printf("              duplicate detected. backing up to item %d\n", i));
		    }			/* again to the top of segmt*/
		    tbl->index = i;	/* Make this the current item. */
		    tbl->status = Success;
		    DMSG("table", printf("              found item %d\n", i));
		    return addr;

		} else if (t > 0) {
		    /* What we are testing for (arg) is less than the
		     * item (addr) that we are considering. We need to
		     * look further in the table for a match. */

		    beg = i;
		    i += ((end-beg)+1)/2;

		} else if (t < 0) {
		    /* What we are testing for (arg) is greater than the
		     * item (addr) that we are considering. We need to
		     * look earlier in the table for a match. */

		    end = i;
		    i -= ((end-beg)+1)/2;
		}
	    }
	} else {
#else
	{
#endif
	    i = tbl->index;		/* The table is not sorted. */
	    while (++i <= tbl->end) {	/* We have to scan each item*/
		addr = (void *) ((int)tbl->data + (i * tbl->size));
		if ((*test)(arg, addr) == 0) {
		    tbl->index = i;
		    tbl->status = Success;
		    return addr;
		}
	    }
	}
	tbl->index = tbl->end;
	tbl->status = Failure;
	return Failure;
}

/*------------------------------------------------------------------------
 *
 *	debug.c		Simple handling of debugging messages.
 *
 *	These routines are used to manage simple outputing of messages
 *	based on a key. The user can dynamically enable or disable the
 *	printing of messages with the use of the debug_print("type") and
 *	debug_stop("type") routines. This works in conjunction with the
 *	DMSG("type", <statment>)); macros which can be sprinkled through
 *	the code. When the particular "type" catagory has been enabled,
 *	the statment will be executed. Typically, this is printf or a
 *	call to a debugging or other logging routine. When the DEBUG
 *	flag is not defined, to reduce the program size, these statments
 *	do not get reduced to code.
 *
 *------------------------------------------------------------------------*/


#define	True	1
#define	False	0
#define	DebugTblSize	20

char	*debugTypesEnabled[DebugTblSize];

typedef struct	DebugCatalog_t {
	char	tag[20];
	char	file[20];
	int	line;
	int	count;
} DebugCatalog;

Table	*debugCatalog;

int
dmsg(
    char	*typ)
{
	if (!typ)
	     debug_print();
	else debug_msg(typ);
	return 0;
}

int
dstop(
    char	*typ)
{
	debug_stop(typ);
	return 0;
}

int
dindex()
{
	debug_index();
	return 0;
}

int
dprint()
{
	debug_print();
	return 0;
}

void
debug_msg(
    char	*typ)
{
    int		argc, i;
    char	c, *tok, *t, *argv[20], buf[80];

	tok = buf;				/* Tokenize the input */
	argv[argc = 0] = tok;
	while ((c = *tok++ = *typ++))
	    if ((c == ' ') || (c == '	')) {
		*--tok = 0;
		argv[++argc] = ++tok;
	    }
	argv[++argc] = 0;

	/* Now put them on the Types Enabled list. */
	argc = -1;
next:	while ((tok = argv[++argc])) {
	    i = -1;
	    while ((t = debugTypesEnabled[++i])) 
		if (!strncmp(tok, t, strlen(tok)))
		    goto next;
	    debugTypesEnabled[i] = strdup(tok);
	}
}

void
debug_stop(
    char	*typ)
{
    int		argc, i, j;
    char	c, *tok, *t, *argv[20], buf[80];

	tok = buf;				/* Tokenize the input */
	argv[argc = 0] = tok;
	while ((c = *tok++ = *typ++))
	    if ((c == ' ') || (c == '	')) {
		*--tok = 0;
		argv[++argc] = ++tok;
	    }
	argv[++argc] = 0;

	/* Now remove them from the Types Enabled list. */
	argc = -1;
	while ((tok = argv[++argc])) {
	    i = -1;
	    while (++i < DebugTblSize)
		if ((t = debugTypesEnabled[i]) && !strncasecmp(tok, t, strlen(t))) {
		    free(debugTypesEnabled[i]);	/* Free the strdup memory */
		    j = i;
		    while (++j < DebugTblSize)
		        debugTypesEnabled[j-1] = debugTypesEnabled[j];
		    debugTypesEnabled[j] = 0;
	    }
	}
}


int
debug_print()
{
    int		i;
    char	*tok;

	printf("The following categories are enabled:\n");
	i = -1;
	while ((tok = debugTypesEnabled[++i]))
	    printf("%d: '%s'\n", i, tok);
	printf("-----\n");
	return 0;
}

int
debug_test(
    char	*typ,
    char	*file,
    int		line)
{
    int		argc, i;
    char	c, *t, *tok, *argv[20], buf[80];


	tok = buf;	/* Tokenize the input */	
	argv[argc = 0] = tok;
	while ((c = *tok++ = *typ++))
	    if ((c == ' ') || (c == '	')) {
		*--tok = 0;
		argv[++argc] = ++tok;
	    }
	argv[++argc] = 0;

#if 0
	if (debugCatalog)	/* Should we catalog this message? */
	    debug_catalog(typ, file, line);
#endif

	/* Now look for them on the Types Enabled list. */
	argc = -1;
	while ((tok = argv[++argc])) {
	    i = -1;
	    while (++i < DebugTblSize)
	        if ((t = debugTypesEnabled[i]) && !strncasecmp(tok, t, strlen(tok)))
		    return True;
	}
	return False;
}

void
debug_catalog(
    char	*typ,
    char	*file,
    int		line)
{
    Table	*cat;
    DebugCatalog *itm, tst;

	if (!(cat = debugCatalog))
	    return;

	strcpy(tst.tag, typ);
	strcpy(tst.file, file);
	tst.line = line;
	if ((itm = table_find(cat, debug_catalogFind, (void *)&tst)))
		itm->count++;
	else {
	    itm = calloc(1, sizeof(DebugCatalog));
	    strcpy(itm->file, file);
	    strcpy(itm->tag, typ);
	    itm->line = line;
	    table_addEntry(cat, (void *)itm, sizeof(DebugCatalog));
	}
}

void
debug_index()
{
    Table	*cat;
    int		i;
    DebugCatalog	*itm;

	if (!(cat = debugCatalog)) {
	    debugCatalog = table_create(sizeof(DebugCatalog));
	    table_sort(debugCatalog, debug_catalogFind);
	    printf("Accumulating index of DMSG statments\n");
	} else {
	    printf("DMSG was called with the following tags:\n\n");
	    i = -1;
	    while (++i < table_end(cat))
		if ((itm = table_getRow(cat, i)))
		    printf("%12s  %d (%s:%d)\n", itm->tag, itm->count, itm->file, itm->line);
	}
}


/* 
 *	This function is used as a qsort process to sort tables based on the tags.
 *	We are passed pointers to DebugCatalog types. Since we want to keep a detailed
 *	entry based on file / line, we keep concantinate all the items to a string and
 *	then do the compare.
 */

int
debug_catalogFind(
    const void *arg1,
    const void *arg2)
{
    DebugCatalog *itm1 = (DebugCatalog *)arg1;
    DebugCatalog *itm2 = (DebugCatalog *)arg2;
    char	buf1[40], buf2[40];

	sprintf(buf1, "%s%s%d", itm1->tag, itm1->file, itm1->line);
	sprintf(buf2, "%s%s%d", itm2->tag, itm2->file, itm2->line);	
	return strcasecmp (buf1, buf2);
}

int
strncasecmp(
    char	*s1,
    char	*s2,
    int		n)
{


	if (!s1 || !s2)
	    return False;
#if 0
	printf("StrCaseNCmp(0x%x='%s', 0x%x='%s', %d)\n", (int)s1, s1, (int)s2,  s2, n);
#endif
	while (*s1 && *s2 && (n-- > 0))
	    if ((*s1 == '*') || (*s2 == '*'))
		return 0;
	    else if ((*s1++ | ' ') != (*s2++ | ' '))
		return *--s1 - *--s2;
	return (n && (*s1 || *s2)) ? 1 : 0;
}

int
strcasecmp(
    char	*s1,
    char	*s2)
{
	if (!s1 || !s2)
	    return False;

#if 0
	printf("StrCaseCmp(0x%x='%s', 0x%x='%s')\n", (int)s1, s1, (int)s2,  s2);
#endif
	while (*s1 && *s2)
#if 0
	    if ((*s1 == '*') || (*s2 == '*'))
		return 0;
	    else
#endif
	    if ((*s1++ | ' ') != (*s2++ | ' '))
		return *--s1 - *--s2;
	
	return (*s1 || *s2) ? 1 : 0;
}

char *
strdup(
    char *str)
{
    int		i;
    char	*cpy;

	if (!str)
	    return False;
	i = strlen(str);
	if ((cpy = malloc(i+1)))
	    strcpy(cpy, str);
	return cpy;
}
