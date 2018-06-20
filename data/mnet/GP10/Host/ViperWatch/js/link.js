
var orgLinkColor =""; // used to remember link color prior to highlight.
var linkHilightColor = "green"; // Link highlight color

function highlight(obj, color)
{
  orgLinkColor = obj.style.color;
  obj.style.color=color;
}

function undoHighlight(obj) 
{
  obj.style.color=orgLinkColor;
}