/******************************************************************
 * show configuration -- OBSOLETE
 * 
 */
EOLNS   (show_config_eol, show_configuration);
KEYWORD (show_config, show_config_eol, no_alt,
         "running-configuration", "Contents of Non-Volatile memory", PRIV_ROOT);

#undef  ALTERNATE
#define ALTERNATE   show_config
