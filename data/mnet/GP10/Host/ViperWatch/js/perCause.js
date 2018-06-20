function displayCause(theCause)
{
	var textCause = "";
	var longDesc = "";
	
	switch(theCause)
	{
	case  "160":  // 0xa0
		// Emergency call
		textCause = "Emergency call (" + theCause + ")";
		break;
	case "192":  //0xc0
		//	Call re-establishment; TCH/F was in use
		 textCause = "Call re-establishment (" + theCause + ")"
		 longDesc = "TCH/F was in use";		
		break;

	case "128":  //0x80
		// Answer to paging
		 textCause = "Answer to paging (" + theCause + ")";
		 longDesc = "Full Rate only";		
		break;

	case "16":  //0x10
		// Answer to paging
		textCause = "Answer to paging (" + theCause + ")";
		longDesc = "SDCCH only";		
		
		break;

	case "32":  //0x20
		// Answer to paging
		textCause = "Answer to paging (" + theCause + ")";
		longDesc = "Dual rate mobile";		
		break;

	case "224": //0xe0
		// Originating call and TCH/F is needed
		textCause = "Originating call (" + theCause + ")";
        longDesc = "TCH/F is needed to complete the originating call";				
		break;

	case "0": // 0x00
		// Location updating 
		textCause = "Location updating (" + theCause + ")";
        longDesc = "TCH/F is needed to complete the location update";				

		break;

	case "12":
		// Other procedures which can be completed with
		//	an SDCCH and the network sets NECI bit to 1
		textCause = "Other procedures (" + theCause + ")";
		longDesc = "the procedures which can be completed with an SDCCH and the network sets NECI bit to 1";	
		break;
	default:
		textCause = "Unknown Cause (" + theCause + ")";
		longDesc = "Invalid Cause Number in the MIB";	

		
		break;
	}
	document.write(textCause + "<br> <font size='-1'> <I>" + longDesc + "</I> </font>");
}

