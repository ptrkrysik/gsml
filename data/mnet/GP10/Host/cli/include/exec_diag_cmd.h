
#define VERBOSE          0
#define NO_VERBOSE       1
#define TYPE_TEST_ALLMEM 3
#define TYPE_INCR_MEM    4
#define TYPE_ALL_MEM     7
#define HALT_ON_ERROR    10

enum {
  DIAG_CFTEST_CMD,
  DIAG_STOPDIAG_CMD,
  DIAG_I2CLOOPBACK_CMD,
  DIAG_I2CTEST_CMD,
  DIAG_IFREPORT_CMD,
  DIAG_MSREPORT_CMD,
  DIAG_HPIMEMACCESS_CMD,
  DIAG_HPIECHOTEST_CMD,
  DIAG_HOREPORT_CMD,
  DIAG_FPGATEST_CMD,
  DIAG_DSPMEMTEST_CMD,
  DIAG_DSPLOOPBACK_CMD,
  DIAG_DSPEXTMEMTEST_CMD,
  DIAG_DSPBBLBTEST_CMD,
  DIAG_DLREPORT_CMD
};


enum {
  TYPE_TEST_ALL = 0,
  TYPE_WALK_ZERO,
  TYPE_WALK_ONES,
  TYPE_INCREMENT,
  TYPE_INCR_COMP,
  TYPE_RANDOM_PATTERN
};


enum {
  TYPE_LOCAL_LOOP = 1,
  TYPE_BASE_LOOP,
  TYPE_IQ_LOOP,
  TYPE_TCH_LOOP,
  TYPE_TCHTOH_LOOP
};


enum {
  TEST_DSP_1 = 1,
  TEST_DSP_0,
  TEST_DSP_ALL
};


static keyword_options verbose_options[] = {
       { "verbose", "Display all error messages", VERBOSE},
       { "no-verbose", "Display only final result", NO_VERBOSE},
       {NULL,NULL,0}
};



static keyword_options extmemtest_type_options[] = {
       { "0", "All", TYPE_TEST_ALL},
       { "1", "Walk Zeroes", TYPE_WALK_ZERO},
       { "2", "Walk Ones", TYPE_WALK_ONES},
       { "3", "Increment", TYPE_INCREMENT},
       { "4", "Increment and Complement", TYPE_INCR_COMP},
       { "5", "Random pattern", TYPE_RANDOM_PATTERN},
       {NULL,NULL,0}
};


static keyword_options loopback_type_options[] = {
       { "1", "Create a local loopback", TYPE_LOCAL_LOOP},
       { "2", "Create a baseband loopback", TYPE_BASE_LOOP},
       { "3", "Create an IQ loopback", TYPE_IQ_LOOP},
       { "4", "Create a TCH loopback", TYPE_TCH_LOOP},
       { "5", "Creae a TCH-to-host loopback", TYPE_TCHTOH_LOOP},
       {NULL,NULL,0}
};


static keyword_options memtest_type_options[] = {
       { "1", "Walk Zeroes", TYPE_WALK_ZERO},
       { "2", "Walk Ones", TYPE_WALK_ONES},
       { "3", "All", TYPE_TEST_ALLMEM},
       {NULL,NULL,0}
};

static keyword_options echotest_type_options[] = {
       { "10", "Test dsp 0 only", TEST_DSP_0},
       { "01", "Test dsp 1 only", TEST_DSP_1},
       { "11", "Test both dsp 0 and 1", TEST_DSP_ALL},
       {NULL,NULL,0}
};

static keyword_options memaccess_type_options[] = {
       { "1", "Walk Zeroes", TYPE_WALK_ZERO},
       { "2", "Walk Ones", TYPE_WALK_ONES},
       { "4", "Incrmenting memory locations", TYPE_INCR_MEM},
       { "7", "All", TYPE_ALL_MEM},
       {NULL,NULL,0}
};

static keyword_options halt_on_error[] = {
   { "haltonerror", "To stop the diagnostic on error", HALT_ON_ERROR},
   {NULL,NULL,0}
};


EOLS (stopdiag_eol,diagnostic_cmds,DIAG_STOPDIAG_CMD);

STRING (stopdiag_name,stopdiag_eol,no_alt,OBJ(string,1),
	"Name of the diagnostic to stop");

KEYWORD (diag_stopdiag,stopdiag_name,no_alt,"stopdiag",
	 "Stop a diagnostic test",PRIV_ROOT);


EOLS    (diag_i2cloopback_eol,diagnostic_cmds,DIAG_I2CLOOPBACK_CMD);

KEYWORD_OPTIONS (diag_i2cloopback_halt,diag_i2cloopback_eol,diag_i2cloopback_eol,
	 halt_on_error,OBJ(int,4),PRIV_ROOT,0);

KEYWORD_OPTIONS(i2cloopback_verb,diag_i2cloopback_halt,diag_i2cloopback_eol,verbose_options,
		OBJ(int,3),PRIV_ROOT,0);

NUMBER (diag_i2cloopback_durnum,i2cloopback_verb,no_alt,OBJ(int,2),
	0,255,"Number of seconds");

KEYWORD (diag_i2cloopback_dur,diag_i2cloopback_durnum,diag_i2cloopback_eol,
	 "duration","Number of seconds to run the test",PRIV_ROOT);

NUMBER (diag_i2cloopback_num,diag_i2cloopback_dur,no_alt,OBJ(int,1),
	0,65535,"Number of bytes");

KEYWORD (diag_i2cloopback_nob,diag_i2cloopback_num,diag_i2cloopback_eol,
	 "number-of-bytes","Number of bytes to send in loopback",PRIV_ROOT);

KEYWORD (diag_i2cloopback,diag_i2cloopback_nob,no_alt,"i2cloopback",
	 "To run i2cloopback diagnostic",PRIV_ROOT | PRIV_HIDDEN);
       


EOLS (i2ctest_eol,diagnostic_cmds,DIAG_I2CTEST_CMD);

KEYWORD (diag_i2ctest,i2ctest_eol,diag_i2cloopback,"i2ctest",
	 "To run the i2ctest diagnostic",PRIV_ROOT | PRIV_HIDDEN);


EOLS   (msreport_eol,diagnostic_cmds,DIAG_MSREPORT_CMD);

NUMBER (msreport_duration,msreport_eol,msreport_eol,OBJ(int,3),0,255,
	"Number of seconds to run");

NUMBER (msreport_trx1_mask,msreport_duration,no_alt,OBJ(int,2),0,255,
	"Mask ot time slots on the first transceiver");

NUMBER (msreport_trx0_mask,msreport_trx1_mask,no_alt,OBJ(int,1),0,255,
	"Mask ot time slots on the first transceiver");

KEYWORD (diag_msreport,msreport_trx0_mask,diag_i2ctest,"msreport",
	 "To run the MS Report diagnostic",PRIV_ROOT);


EOLS   (ifreport_eol,diagnostic_cmds,DIAG_IFREPORT_CMD);

NUMBER (ifreport_duration,ifreport_eol,ifreport_eol,OBJ(int,3),0,255,
	"Number of seconds to run");

NUMBER (ifreport_trx1_mask,ifreport_duration,no_alt,OBJ(int,2),0,255,
	"Mask ot time slots on the first transceiver");

NUMBER (ifreport_trx0_mask,ifreport_trx1_mask,no_alt,OBJ(int,1),0,255,
	"Mask ot time slots on the first transceiver");

KEYWORD (diag_ifreport,ifreport_trx0_mask,diag_msreport,"ifreport",
	 "To run the IF report diagnostic ",PRIV_ROOT);



EOLS (hpimemaccess_eol,diagnostic_cmds,DIAG_HPIMEMACCESS_CMD);

KEYWORD_OPTIONS(hpimemaccess_verb,hpimemaccess_eol,hpimemaccess_eol,verbose_options,
		OBJ(int,3),PRIV_ROOT,0);

KEYWORD_OPTIONS(hpimemaccess_type,hpimemaccess_verb,no_alt,memaccess_type_options,
		OBJ(int,2),PRIV_ROOT,0);

NUMBER (hpimemaccess_dsp,hpimemaccess_type,no_alt,OBJ(int,1),0,1,
	"Transceiver to test");

KEYWORD (diag_hpimemaccess,hpimemaccess_dsp,diag_ifreport,"hpimemoryaccess",
	 "Run a diagnostic against the transmit and receive memory used for HPI transfers on the CDC",PRIV_ROOT | PRIV_HIDDEN);



EOLS (hpiechotest_eol,diagnostic_cmds,DIAG_HPIECHOTEST_CMD);

KEYWORD_OPTIONS(hpiechotest_verb,hpiechotest_eol,hpiechotest_eol,verbose_options,
		OBJ(int,5),PRIV_ROOT,0);

NUMBER (hpiechotest_switch,hpiechotest_verb,no_alt,OBJ(int,4),0,255,
	"0-permit dsp task switching, 1 or higher - prohibit dsp task switching");

NUMBER (hpiechotest_num_ticks,hpiechotest_switch,no_alt,OBJ(int,3),1,255,
	"Number of clock ticks to between each group of messages");

NUMBER (hpiechotest_num_msg,hpiechotest_num_ticks,no_alt,OBJ(int,2),1,10,
	"Number of messages to send in each burst");

KEYWORD_OPTIONS(hpiechotest_combo,hpiechotest_num_msg,no_alt,echotest_type_options,
		OBJ(int,1),PRIV_ROOT,0);

KEYWORD (diag_hpiechotest,hpiechotest_combo,diag_hpimemaccess,"hpiechotest",
	 "Command to send test messages to the DSPs",PRIV_ROOT | PRIV_HIDDEN);



EOLS   (horeport_eol,diagnostic_cmds,DIAG_HOREPORT_CMD);

NUMBER (horeport_duration,horeport_eol,horeport_eol,OBJ(int,3),0,255,
	"Number of seconds to run");

NUMBER (horeport_trx1_mask,horeport_duration,no_alt,OBJ(int,2),0,255,
	"Mask ot time slots on the first transceiver");

NUMBER (horeport_trx0_mask,horeport_trx1_mask,no_alt,OBJ(int,1),0,255,
	"Mask ot time slots on the first transceiver");

KEYWORD (diag_horeport,horeport_trx0_mask,diag_hpiechotest,"horeport",
	 "Test to determine whether handoff is required",PRIV_ROOT);






EOLS (fpgatest_eol,diagnostic_cmds,DIAG_FPGATEST_CMD);

KEYWORD_OPTIONS(fpgatest_verb,fpgatest_eol,fpgatest_eol,verbose_options,
		OBJ(int,1),PRIV_ROOT,0);

KEYWORD (diag_fpgatest,fpgatest_verb,diag_horeport,"fpgatest",
	 "Command to check FPGA 0 and FPGA 1 ",PRIV_ROOT | PRIV_HIDDEN);




EOLS (dspmemtest_eol,diagnostic_cmds,DIAG_DSPMEMTEST_CMD);

KEYWORD_OPTIONS(dspmemtest_verb,dspmemtest_eol,dspmemtest_eol,verbose_options,
		OBJ(int,4),PRIV_ROOT,0);

NUMBER (dspmemtest_duration,dspmemtest_verb,dspmemtest_eol,OBJ(int,3),0,255,
	"Time to run the test");

KEYWORD_OPTIONS(dspmemtest_type,dspmemtest_duration,no_alt,memtest_type_options,
		OBJ(int,2),PRIV_ROOT,0);

NUMBER (dspmemtest_dsp,dspmemtest_type,no_alt,OBJ(int,1),0,1,
	"Transceiver to test");

KEYWORD (diag_dspmemtest,dspmemtest_dsp,diag_fpgatest,"dspmemtest",
	 "Test both the data and external RAM ",PRIV_ROOT | PRIV_HIDDEN);





EOLS (dsploopback_eol,diagnostic_cmds,DIAG_DSPLOOPBACK_CMD);

NUMBER (dsploopback_duration,dsploopback_eol,dsploopback_eol,OBJ(int,3),0,255,
	"Time to run the test");

KEYWORD_OPTIONS(dsploopback_type,dsploopback_duration,no_alt,loopback_type_options,
		OBJ(int,2),PRIV_ROOT,0);

NUMBER (dsploopback_dsp,dsploopback_type,no_alt,OBJ(int,1),0,1,
	"Transceiver to test");


KEYWORD (diag_dsploopback,dsploopback_dsp,diag_dspmemtest,"dsploopback",
	 "Set loopbacks in the dsp ",PRIV_ROOT | PRIV_HIDDEN);




EOLS (dspextmemtest_eol,diagnostic_cmds,DIAG_DSPEXTMEMTEST_CMD);

KEYWORD_OPTIONS(dspextmemtest_verb,dspextmemtest_eol,dspextmemtest_eol,verbose_options,
		OBJ(int,3),PRIV_ROOT,0);

KEYWORD_OPTIONS(dspextmemtest_type,dspextmemtest_verb,no_alt,extmemtest_type_options,
		OBJ(int,2),PRIV_ROOT,0);

NUMBER (dspextmemtest_dsp,dspextmemtest_type,no_alt,OBJ(int,1),0,1,
	"Transceiver to test");

KEYWORD (diag_dspextmemtest,dspextmemtest_dsp,diag_dsploopback,"dspextmemtest",
	 "Start the dsp external memory test",PRIV_ROOT | PRIV_HIDDEN);



EOLS (dspbblbtest_eol,diagnostic_cmds,DIAG_DSPBBLBTEST_CMD);

KEYWORD_OPTIONS(dspbblbtest_verb,dspbblbtest_eol,dspbblbtest_eol,verbose_options,
		OBJ(int,5),PRIV_ROOT,0);

NUMBER (dspbblbtest_gain,dspbblbtest_verb,no_alt,OBJ(int,4),0,7,
	"The gain shift");

NUMBER (dspbblbtest_frequency,dspbblbtest_gain,no_alt,OBJ(int,3),0,7,
	"Index of the frequency to be tested");

NUMBER (dspbblbtest_destination,dspbblbtest_frequency,no_alt,OBJ(int,2),0,1,
	"Destination transceiver");

NUMBER (dspbblbtest_source,dspbblbtest_destination,no_alt,OBJ(int,1),0,1,
	"Source transceiver");

KEYWORD (diag_dspbblbtest,dspbblbtest_source,diag_dspextmemtest,"dspbblbtest",
	 "Start the DSP baseband loopback test",PRIV_ROOT | PRIV_HIDDEN);



EOLS   (dlreport_eol,diagnostic_cmds,DIAG_DLREPORT_CMD);

NUMBER (dlreport_duration,dlreport_eol,dlreport_eol,OBJ(int,3),0,255,
	"Number of seconds to run");

NUMBER (dlreport_trx1_mask,dlreport_duration,no_alt,OBJ(int,2),0,255,
	"Mask ot time slots on the first transceiver");

NUMBER (dlreport_trx0_mask,dlreport_trx1_mask,no_alt,OBJ(int,1),0,255,
	"Mask ot time slots on the first transceiver");

KEYWORD (diag_dlreport,dlreport_trx0_mask,diag_dspbblbtest,"dlreport",
	 "Display the downlink power level and quality",PRIV_ROOT);




EOLS   (cftest_eol,diagnostic_cmds,DIAG_CFTEST_CMD);

NUMBER (cftest_repeat,cftest_eol,cftest_eol,OBJ(int,1),1,255,
	"Number of times to repeat");

KEYWORD (diag_cftest,cftest_repeat,diag_dlreport,"cftest",
	 "Test the compact flash interface",PRIV_ROOT | PRIV_HIDDEN);


KEYWORD(exec_diag, diag_cftest,diag_stopdiag,
                "startdiag", "Run a diagnostic test", PRIV_ROOT);

#undef ALTERNATE
#define ALTERNATE exec_diag






