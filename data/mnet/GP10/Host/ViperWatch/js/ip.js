function cvtIntToIP(theField)
{
	var ip = theField.value
	var byte1 = (ip >> 24) & 0x000000FF
	var byte2 = (ip >> 16) & 0x000000FF
	var byte3 = (ip >> 8) & 0x000000FF
	var byte4 = (ip & 0x000000FF)
    theField.value =  byte1 + "." + byte2 + "." + byte3 + "." + byte4;
    // alert ("IP Address  = " + theField.value);
    return true; 
}

function cvtIPToInt(theIP)
{
  var checkOK = "0123456789.";
  var checkStr = theIP.value;
  var allValid = true;
  for (i = 0;  i < checkStr.length;  i++)
  {
    ch = checkStr.charAt(i);
    for (j = 0;  j < checkOK.length;  j++)
      if (ch == checkOK.charAt(j))
        break;
    if (j == checkOK.length)
    {
      allValid = false;
      break;
    }
  }
  if (!allValid)
  {
    alert("Invalid character in the IP address field: " + theIP.name);
    theIP.focus();
    return (false);
  }
  
  ipBytes = theIP.value.split(".",4);
  if(ipBytes.length != 4)
  {
  		alert("Invalid IP address:"  + theIP.name);
  		theIP.focus();
  		return false;
  }
  
  for(var i=0; i<4; i++)
  {
	if (ipBytes[i] <= 0 || ipBytes[i] >= 0xFF)
	{
		alert("Invalid IP address:"  + theIP.name);
		theIP.focus();
		return false
	}
  }
  
  // Now finally we are converting here
  var ip =  ((ipBytes[0] << 24) & 0xFF000000) 
	      | ((ipBytes[1] << 16) & 0x00FF0000)
	      | ((ipBytes[2] << 8)  & 0x0000FF00)
	      |  (ipBytes[3] & 0x000000FF);

  theIP.value = ip;
  return (true);
}


function validateIp(theIP, name)
{
  var checkOK = "0123456789.";
  var checkStr = theIP.value;
  var allValid = true;
  for (i = 0;  i < checkStr.length;  i++)
  {
    ch = checkStr.charAt(i);
    for (j = 0;  j < checkOK.length;  j++)
      if (ch == checkOK.charAt(j))
        break;
    if (j == checkOK.length)
    {
      allValid = false;
      break;
    }
  }
  if (!allValid)
  {
    alert("Invalid character in the IP address field: " + name);
    theIP.focus();
    return (false);
  }
  
  ipBytes = theIP.value.split(".",4);
  if(ipBytes.length != 4)
  {
  		alert("Invalid IP address:"  + name);
  		theIP.focus();
  		return false;
  }
  
  for(var i=0; i<4; i++)
  {
	if (ipBytes[i] < 0 || ipBytes[i] > 0xFF)
	{
		alert("Invalid IP address:"  + name);
		theIP.focus();
		return false
	}
  }
  
  return (true);
}
