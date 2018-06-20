// index is a global variable on the script
function isInt(theField)
{	
  if (theField.value == "")
  {
    alert("Please enter a value for the " + theField.name + " field.");
    theField.value = theField.defaultValue;
    theField.focus();
    return (false);
  }
 
  var checkOK = "-0123456789";
  var checkStr = theField.value;
  var allValid = true;
  var decPoints = 0;
  for (i = 0;  i < checkStr.length;  i++)
  {
    ch = checkStr.charAt(i);
    for (j = 0;  j < checkOK.length;  j++)
    {
      if (ch == checkOK.charAt(j))
        break;
    }
    if (j == checkOK.length)
    {
      allValid = false;
      break;
    }
  }
  if (!allValid)
  {
		alert("Please enter only digit characters in the " + theField.name + " field.");
		theField.value = theField.defaultValue;
		theField.focus();
		return (false);
  }
  return (true);
}

function isValidInt(theField, minValue, maxValue)
{	
  if (!isInt(theField))
	return (false);
   
  var prsVal = parseInt(theField.value);
  if (prsVal < minValue || prsVal > maxValue)
  {
    if (minValue == maxValue)
		alert("Only valid value in the " + theField.name + " field is " + minValue + ".");
	else
		alert("Please enter a value greater than or equal to " + minValue + " and less than or equal to " + maxValue +" in the " + theField.name + " field.");
    theField.value = theField.defaultValue;
    theField.focus();
    return (false);
  }
  return (true);
}

function nextIndex(prevIndex)
{
	var tmp = Number(prevIndex);
	tmp++
	document.write(" " + tmp + " ");
}

function isIntName(theField, name)
{	
  if (theField.value == "")
  {
    alert("Please enter a value for the " + name + " field.");
    theField.value = theField.defaultValue;
    theField.focus();
    return (false);
  }
 
  var checkOK = "-0123456789";
  var checkStr = theField.value;
  var allValid = true;
  var decPoints = 0;
  for (i = 0;  i < checkStr.length;  i++)
  {
    ch = checkStr.charAt(i);
    for (j = 0;  j < checkOK.length;  j++)
    {
      if (ch == checkOK.charAt(j))
        break;
    }
    if (j == checkOK.length)
    {
      allValid = false;
      break;
    }
  }
  if (!allValid)
  {
		alert("Please enter only digit characters in the " + name + " field.");
		theField.value = theField.defaultValue;
		theField.focus();
		return (false);
  }
  return (true);
}

function isValidIntName(theField, minValue, maxValue, name)
{	
  if (!isIntName(theField, name))
	return (false);
   
  var prsVal = parseInt(theField.value);
  if (prsVal < minValue || prsVal > maxValue)
  {
    alert("Please enter a value greater than or equal to " + minValue + " and less than or equal to " + maxValue +" in the " + name + " field.");
    theField.value = theField.defaultValue;
    theField.focus();
    return (false);
  }
  return (true);
}

function isValidTxPower(theField, theRadioType)
{
	var max
	var min
	var textRadioType

  	if (!isIntName(theField, "Tx Power"))
		return (false);

	switch(theRadioType)
	{
	case  0: 
		max = 39
		min = 5
		textRadioType = "GSM 900";
		break;
	case  1:  
		max = 30
		min = 0
		textRadioType = "DCS 1800";
		break;	
	case 2:
		max = 33
		min = 0
		textRadioType = "PCS 1900";
		break;
	default:
    		alert("Invalid Radio type configured at the MIB. Please reboot the GP10.");
		theField.value = theField.defaultValue;
		return (false)
	}
	if (theField.value < min || theField.value > max) 
  	{
    		alert("For GP10 product TX Power in " + textRadioType + ": " + min + ".." + max + " dBm");
    		theField.value = theField.defaultValue;
    		theField.focus();
    		return (false);
  	}
	return (true);
}
