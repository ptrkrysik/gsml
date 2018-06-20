function clickRadio(theRadio, valueHolder)
{
	if (valueHolder.value != 0 && valueHolder.value != 1)
	{ 
		window.alert("Incorrect MIB value for " + valueHolder.name + "  "+ valueHolder.value + "Corrected to 0");
		valueHolder.value = 0;
	}
	
	for (var i = 0; i < theRadio.length; i++)
	{
		if (theRadio[i].value == valueHolder.value) 
		{
			theRadio[i].click();
			theRadio[i].defaultChecked = true;
			break;
		}
	}
}

function selectOption(theSelect, valueHolder)
{
	if (valueHolder.value < 0 && valueHolder.value >= theSelect.length)
	{ 
		window.alert("Incorrect MIB value for " + valueHolder.name + "  "+ valueHolder.value + "Corrected to 0");
		valueHolder.value = 0;
	}
	for (var i = 0; i < theSelect.length; i++)
	{
		if (theSelect.options[i].value == valueHolder.value) 
		{
			theSelect.options[i].selected = true;
			theSelect.options[i].defaultSelected = true;
			break;
		}
	}
			
}

function selectDisabledOption(theSelect, theValue)
{
	theSelect.disabled = true
	for (var i = 0; i < theSelect.length; i++)
	{
		if (theSelect.options[i].value == theValue) 
		{
			theSelect.options[i].selected = true;
			break;
		}
		theSelect.options[i].selected = true;
	}			
}


function clickCheckBox(theCheckBox, valueHolder)
{
	if (!theCheckBox.length || theCheckBox.length==1)
	{
		if (valueHolder.value == true)
		{
			theCheckBox.checked = true;
			theCheckBox.defaultChecked = true;
			
		} else {
			theCheckBox.checked = false;
			theCheckBox.defaultChecked = false;
		}

	} else {			
		for (var i = 0; i < theCheckBox.length; i++)
		{
			if (theCheckBox.elements[i].value == valueHolder.name) 
			{
				if (valueHolder.value == "1")
				{
					theCheckBox[i].checked = true;
					theCheckBox[i].defaultChecked = true;
				
				}
				else
				{
					theCheckBox[i].checked = false;
					theCheckBox[i].defaultChecked = false;
				}
			}
		}
	}	
}


function parseRadio(radioButton, valueHolder)
{
	for (var i = 0; i < radioButton.length; i++)
	{
		if (radioButton[i].checked)
		{
			valueHolder.value = radioButton[i].value;
			break;
		}
	}
}
	
function parseSelect(theSelect, valueHolder)
{
	for (var i = 0; i < theSelect.length; i++)
	{
		if (theSelect.options[i].selected) 
		{
			valueHolder.value = theSelect.options[i].value;
			break;
		}
	}
}

function parseCheckBox(theCheckBox, valueHolder)
{

	if (!theCheckBox.length || theCheckBox.length == 1)
	{
		if (theCheckBox.checked == true)
		{
			valueHolder.value = "1";
		} else {
			valueHolder.value = "0";
		}

	} else {			

		for (var i = 0; i < theCheckBox.length; i++)
		{
			if (theCheckBox.elements[i].value == valueHolder.name) 
			{
				if (theCheckBox[i].checked == true)
				{
					valueHolder.value = "1";
				} else {
					valueHolder.value = "0";
				}
			}
		}
	}
}

function cvtIntToBCD(theTextField, digitCount)
{
	var allZeros = "";
	for (var i= theTextField.value.length; i <digitCount; i++)
	{
		allZeros += "0";
	}
	theTextField.value = allZeros+ theTextField.value;
	return true
}
