#include "options_x.h"
#include "options.h"
#include "rlstddef.h"
#include "linklist.h"
#include "ocpgroup.h"

void Ignite_Groups()
{
    OCPGRP_CreateGroup("QSOneGroup");
    OCPGRP_AddGroupMember("QSOneGroup", "QSErrorData");

    OCPGRP_CreateGroup("QSTwoGroup");
    OCPGRP_AddGroupMember("QSTwoGroup", "QSEvent");
    OCPGRP_AddGroupMember("QSTwoGroup", "QSPoll");

}


