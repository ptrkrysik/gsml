# depend.tcl  - pattern substitution file for dependency lists
#
# modification history
# -------------------------------------
# Sept 19, 1999  Bhawani  Initial Draft

# DESCRIPTION
# This file converts path names generated by the compiler back to Makefile
# variables and appends the the variable $(OBJDIR)/ to the object being 
# created so that at compile time the object will get placed where we want
# it. 
# OBJDIR and OBJSUF need to be defined in the Makefile
# 

if {$argc != 1} {
	error "usage : depend.tcl \$\(TGT_DIR\)"
}
set uppat [lindex $argv 0]
#double backslashes
regsub -all {\\}  $uppat {\\\\}  uppat2

while {[gets stdin line] >= 0} {
        regsub {^.*\.o} $line "\$\(OBJDIR\)/&" newline
        regsub {\.o} $newline "\$\(OBJSUF\)" newline1
	  regsub -all $uppat2 $newline1 "\$\(TGT_DIR\)" newline2
        puts stdout $newline2
}
