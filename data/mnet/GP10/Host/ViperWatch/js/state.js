function displayOpState(theState)
{
	var textState = "";
	var Color = "";
	var help = ""

	switch(theState)
	{
	case  "0":  
		// Disabled
		textState = "Disabled";
		Color += "Red";
		help = "The operation state is disabled. Check if there is an outstanding alarm."
		break;
	case  "1":  
		// Enabled
		textState = "Enabled";
		Color += "Green";
		help = "The operation state is enabled."
		break;	
	default:
		textState = "Invalid";
		Color += "Yellow";
		help = "Invalid value of operational state!!!"
		break;
	}
	document.write("<td height=\"14\" title=\"" + help + "\" > <font size=1>  Op. State: <font Color=\"" + Color + "\" >"  + textState +  "</font> </font> </td>");
}

function displayAdminState(theState)
{
	var textState = "";
	var Color = "";
	var help = ""

	switch(theState)
	{
	case  "0":  
		// Disabled
		textState = "Locked";
		Color = "Red";
		help = "The Admin. state is locked by the operator."
		break;
	case  "1":  
		// Enabled
		textState = "Unlocked";
		Color = "Green";
		help = "The Admin. state is unlocked."		
		break;	
	case  "2":  
		// Shuttig Down
		textState = "Shutting down";
		Color = "Pink";
		help = "The GP10 is being shutdown"
		break;
	default:
		textState = "Invalid";
		Color = "Yellow";
		help = "Invalid value of Admin. state"
		break;
	}
	document.write("<td height=\"14\" title=\"" + help + "\" > <font size=1> Admin. State: <font Color=\"" + Color + "\" >"  + textState +  "</font> </font></td>");
	
}


function displayState(theState)
{
	var textState = "";
	var Color = "";

	switch(theState)
	{
	case  "0":  
		// Disabled
		textState = "Disabled";
		Color += "Red";
		break;
	case  "1":  
		// Enabled
		textState = "Enabled";
		Color += "Green";
		break;	
	default:
		textState = "Invalid";
		Color += "Yellow";
		help = "Invalid value of operational state!!!"
		break;
	}
	document.write("<font color=\"" + Color + "\"> " + textState +  "</font>");
}

function displayAlarmStatus(theAlarmCode)
{
	var textStatus = "";
	var Color = "";

	switch(theAlarmCode)
	{
	case  "0":  
		// Disabled
		textState = "Cleared";
		Color += "Green";
		break;
	case  "1":  
		// Enabled
		textState = "Critical";
		Color += "Red";
		break;	
	case  "2":  
		// Disabled
		textState = "Major";
		Color += "#FF00FF";
		break;
	case  "3":  
		// Enabled
		textState = "Minor";
		Color += "#800080";
		break;	
	case  "4":  
		// Enabled
		textState = "Alarm Outsanding";
		Color += "Yellow";
		break;	
	default:
		textState = "Unknown";
		Color += "black";
		break;
	}
	document.write("<font color=\"" + Color + "\"> " + textState +  "</font>");
}

function ConfirmAdminState(theSelect, valueHolder, changeUnit, effectiveUnit, parentState, theParent)
{
	var status = true;
	var locked = "0";
	var unlocked = "1";
	var shuttingDown = "2";
	
	var orgState = valueHolder.value;
	var newState;
	var status = true;
	
	for (var i = 0; i < theSelect.length; i++)
	{
		if (theSelect.options[i].selected) 
		{
			newState = theSelect.options[i].value;
			break;
		}
	}

	if(parentState == locked && newState != locked)
    { 
       alert("Please first unlock the admin state of the " + theParent + " before changing " + changeUnit + ".");
	   orgState  = locked;
       status = false;
    } else {

	switch(orgState)
	{
	   case "0": // locked
	      if (newState == shuttingDown)
	      {
		      alert ("Changing the Admin. State of " + changeUnit + " from \"Locked\" to \"Shutting Down\" is not allowed.");
		      status = false;
	      }
	     break;
	  case  "1": //unlocked
	    if( newState == locked)
	    {
		status = confirm("Warning: Changing Admin State of " + changeUnit + " from \"Unlocked\" to \"Locked\" will prevent " +
		"new calls being made on this " + effectiveUnit + ", and existing Call(s) on this " + effectiveUnit +
		 " will be aborted immediately. Do you want to proceed?");
	   } else if (newState == shuttingDown) {		   
		status = confirm("Warning: Changing Admin State of " + changeUnit + " from \"Unlocked\" to \"Shutting Down\" will prevent " +
		"new calls being made on this " + effectiveUnit + ". Existing Call(s) on this " + effectiveUnit + 
		" will still be preserved until it terminates normally. Do you want to proceed?");
	  }
	  break;
	case "2":
	    if (newState == locked)
	    {
		   status = confirm("Warning: Changing Admin. State "+ changeUnit + " from \"Shutting Down\" to \"Locked\" will immediately abort " +
		   "all existing Call(s) on this" + effectiveUnit +". Do you want to proceed?");
	    } 
        } // switch
       }	
		
       if (!status)
       {
          theSelect.options[orgState].selected = true;
       }
   return status;
} 

function displayPowerClass(theClass)
{
	var textClass= "";
	var Color = "";
	switch(theClass)
	{
	case  "1":  
		textClass = "M1 (0.2W - GSM 900)";
		Color += "Black";
		break;
	case  "2":  
		textClass = "M3 (0.1W - DCS 1800)";
		Color += "Black";
		break;	
	case "3":
		textClass = "M2 (0.2W - PCS 1900)";
		Color += "Black";
		break;
	default:
	    Color += "Red";
		 textClass = "Invalid Power Class (corrupted MIB data)!!!"
		break;
	}
	document.write("<font color=\"" + Color + "\"> " + textClass +  "</font>");
}


