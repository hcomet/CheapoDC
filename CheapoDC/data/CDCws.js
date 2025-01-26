var gateway=`ws://${window.location.hostname}/ws`;var websocket;var modeCount=0;var outstandingQueries=0;window.addEventListener('load',onLoad);function onLoad(event){initWebSocket();initButtons();loadAllItems();document.addEventListener('keydown',function(event){if(event.ctrlKey&&(event.key==='z')){resetInputButtons();}});var req=new XMLHttpRequest();req.open('GET',document.location,true);req.send(null);req.onload=function(){var headers=req.getAllResponseHeaders().toLowerCase();console.log(headers);};}
function initWebSocket(){console.log('Trying to open a WebSocket connection...');websocket=new WebSocket(gateway);websocket.onopen=onOpen;websocket.onclose=onClose;websocket.onmessage=onMessage;}
function onOpen(event){console.log('Connection opened');}
function onClose(event){console.log('Connection closed');setTimeout(initWebSocket,2000);}
function convertHTML(str){const symbols={"&amp;":"&","&lt;":"<","&gt;":">","&quot;":"\"","&apos;":"'","&deg;":'\u00B0',"&percnt;":"%","&commat;":"@"};let newStr=str;let startStr=str;for(const symbol in symbols){if(startStr.indexOf(symbol)>=0){newStr=startStr.replaceAll(symbol,symbols[symbol]);startStr=newStr;}}
return newStr;}
function onMessage(event){const msg=JSON.parse(event.data);var keys=Object.keys(msg);outstandingQueries=outstandingQueries-1;if(outstandingQueries<=0){document.getElementById("loader").style.display="none";outstandingQueries=0;}
if(keys[0]=="ERROR"){console.log("GET error: "+msg.value);}else{if(document.getElementById(keys[0])){document.getElementById(keys[0]).disabled=true;}
if(document.getElementById(keys[0]+"I")){document.getElementById(keys[0]+"I").placeholder=msg[keys[0]]+convertHTML(msg["UNITS"]);document.getElementById(keys[0]+"I").value=msg[keys[0]];}
if(document.getElementById(keys[0]+"S")){document.getElementById(keys[0]+"S").value=msg[keys[0]];console.log("Set select value for "+keys[0]+"S"+" to "+msg[keys[0]]);}
switch(keys[0]){case"DCM":if(msg[keys[0]]==1){document.getElementById("DCO").className="cButton";document.getElementById("DCOI").disabled=false;}else{document.getElementById("DCO").className="cButton hidden";document.getElementById("DCOI").disabled=true;}
case"OMIN":document.getElementById("DCOI").setAttribute("min",msg[keys[0]]);var newMin=parseInt(msg[keys[0]])+1;document.getElementById("OMAXI").setAttribute("min",newMin);break;case"OMAX":document.getElementById("DCOI").setAttribute("max",msg[keys[0]]);var newMax=parseInt(msg[keys[0]])-1;document.getElementById("OMINI").setAttribute("max",newMax);break;case"WS":if(document.getElementById("WSS").value==2){document.getElementById("ATPQ").className="cButton";document.getElementById("ATPQI").disabled=false;document.getElementById("HU").className="cButton";document.getElementById("HUI").disabled=false;}else{document.getElementById("ATPQ").className="cButton hidden";document.getElementById("ATPQI").disabled=true;document.getElementById("HU").className="cButton hidden";document.getElementById("HUI").disabled=true;}
default:break;}}}
function waitForSocketConnection(socket,callback){setTimeout(function(){if(socket.readyState===1){console.log("Connection is made")
if(callback!=null){callback();}}else{console.log("wait for connection...")
waitForSocketConnection(socket,callback);}},5);}
function sendMessage(msg){waitForSocketConnection(websocket,function(){websocket.send(msg);});}
function resetInputButtons(){allcButtons=[...document.getElementsByClassName("cButton")];allcButtons.forEach(function(element){if(!element.disabled){element.disabled=true;sendMessage('{"GET":"'+element.id+'"}');}});}
function setChangeItem(item){var itemId=item.id;var inputId=itemId+"I";var selectId=itemId+"S";item.disabled=true;if(document.getElementById(inputId)){document.getElementById(inputId).oninput=(event)=>{document.getElementById(itemId).disabled=false;};}else if(document.getElementById(selectId)){document.getElementById(selectId).onchange=(event)=>{document.getElementById(itemId).disabled=false;console.log(event);};console.log("Add event for: "+selectId);}else{console.log("No Input or Select found for: "+itemId);}
item.addEventListener("click",function(){sendCmd(this.id);});sendMessage('{"GET":"'+itemId+'"}');}
function initButtons(){allcButtons=[...document.getElementsByClassName("cButton")];allcButtons.forEach(function(element){setChangeItem(element);});}
function sendSetCommand(command,value){sendMessage('{"SET":{"'+command+'":"'+value+'"}}');}
function sendGetCommand(command){outstandingQueries=outstandingQueries+1;if(outstandingQueries>1){document.getElementById("loader").style.display="block";}
sendMessage('{"GET":"'+command+'"}');}
function sendCmd(cdcCmd){var inputId=cdcCmd+"I";if(!document.getElementById(inputId)){inputId=cdcCmd+"S";}
var updValue=document.getElementById(inputId).value;sendMessage('{"SET":{"'+cdcCmd+'":"'+updValue+'"}}');loadAllItems();}
function loadAllItems(){outstandingQueries=0;allcButtons=[...document.getElementsByClassName("cButton")];allcButtons.forEach(function(element){sendGetCommand(element.id);});}
function logoutCDC(){var xhr=new XMLHttpRequest();websocket.close();xhr.open("GET","/logout",true);xhr.send();setTimeout(function(){window.open("/dashboard","_self");},1000);}