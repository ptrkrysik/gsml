function printOnOffMode(theValue)
{
	var textValue = "";
	switch (theValue)
	{
		case "0": 
			textValue = "Off";
			break;
		case "1": 
			textValue = "On";
			break;
		default:
			textValue = "Undefined"
			break;
	}
	document.write(textValue);
}

var index = 1;
function displayIndex()
{
	document.write(" " + index++ + " ");
}

function displayTime(utctime)
{
	var gmtDate = new Date(utctime*1000);
	document.write(gmtDate.toLocaleString());
}

function cvtTicks2Sec(ticks)
{
	var secTime = parseFloat(ticks)/ parseFloat("60.0");
	var secStr = secTime.toString();
	var i = secStr.indexOf(".");
	document.write(secStr.substr(0, i+4) + " (sec)");
}