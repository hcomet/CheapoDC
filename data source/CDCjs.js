var modeCount = 0;
var outstandingQueries = 0;
var refreshCount = 0;
// MD5 function from https://github.com/simetrio/md5-js-tools
var MD5;(()=>{"use strict";var r={d:(n,t)=>{for(var e in t)r.o(t,e)&&!r.o(n,e)&&Object.defineProperty(n,e,{enumerable:!0,get:t[e]})},o:(r,n)=>Object.prototype.hasOwnProperty.call(r,n),r:r=>{"undefined"!=typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(r,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(r,"__esModule",{value:!0})}},n={};(()=>{r.r(n),r.d(n,{MD5:()=>d,generate:()=>e});var t=function(r){r=r.replace(/\r\n/g,"\n");for(var n="",t=0;t<r.length;t++){var e=r.charCodeAt(t);e<128?n+=String.fromCharCode(e):e>127&&e<2048?(n+=String.fromCharCode(e>>6|192),n+=String.fromCharCode(63&e|128)):(n+=String.fromCharCode(e>>12|224),n+=String.fromCharCode(e>>6&63|128),n+=String.fromCharCode(63&e|128))}return n};function e(r){var n,e,o,d,l,C,h,v,S,m;for(n=function(r){for(var n,t=r.length,e=t+8,o=16*((e-e%64)/64+1),u=Array(o-1),a=0,f=0;f<t;)a=f%4*8,u[n=(f-f%4)/4]=u[n]|r.charCodeAt(f)<<a,f++;return a=f%4*8,u[n=(f-f%4)/4]=u[n]|128<<a,u[o-2]=t<<3,u[o-1]=t>>>29,u}(t(r)),h=1732584193,v=4023233417,S=2562383102,m=271733878,e=0;e<n.length;e+=16)o=h,d=v,l=S,C=m,h=a(h,v,S,m,n[e+0],7,3614090360),m=a(m,h,v,S,n[e+1],12,3905402710),S=a(S,m,h,v,n[e+2],17,606105819),v=a(v,S,m,h,n[e+3],22,3250441966),h=a(h,v,S,m,n[e+4],7,4118548399),m=a(m,h,v,S,n[e+5],12,1200080426),S=a(S,m,h,v,n[e+6],17,2821735955),v=a(v,S,m,h,n[e+7],22,4249261313),h=a(h,v,S,m,n[e+8],7,1770035416),m=a(m,h,v,S,n[e+9],12,2336552879),S=a(S,m,h,v,n[e+10],17,4294925233),v=a(v,S,m,h,n[e+11],22,2304563134),h=a(h,v,S,m,n[e+12],7,1804603682),m=a(m,h,v,S,n[e+13],12,4254626195),S=a(S,m,h,v,n[e+14],17,2792965006),h=f(h,v=a(v,S,m,h,n[e+15],22,1236535329),S,m,n[e+1],5,4129170786),m=f(m,h,v,S,n[e+6],9,3225465664),S=f(S,m,h,v,n[e+11],14,643717713),v=f(v,S,m,h,n[e+0],20,3921069994),h=f(h,v,S,m,n[e+5],5,3593408605),m=f(m,h,v,S,n[e+10],9,38016083),S=f(S,m,h,v,n[e+15],14,3634488961),v=f(v,S,m,h,n[e+4],20,3889429448),h=f(h,v,S,m,n[e+9],5,568446438),m=f(m,h,v,S,n[e+14],9,3275163606),S=f(S,m,h,v,n[e+3],14,4107603335),v=f(v,S,m,h,n[e+8],20,1163531501),h=f(h,v,S,m,n[e+13],5,2850285829),m=f(m,h,v,S,n[e+2],9,4243563512),S=f(S,m,h,v,n[e+7],14,1735328473),h=i(h,v=f(v,S,m,h,n[e+12],20,2368359562),S,m,n[e+5],4,4294588738),m=i(m,h,v,S,n[e+8],11,2272392833),S=i(S,m,h,v,n[e+11],16,1839030562),v=i(v,S,m,h,n[e+14],23,4259657740),h=i(h,v,S,m,n[e+1],4,2763975236),m=i(m,h,v,S,n[e+4],11,1272893353),S=i(S,m,h,v,n[e+7],16,4139469664),v=i(v,S,m,h,n[e+10],23,3200236656),h=i(h,v,S,m,n[e+13],4,681279174),m=i(m,h,v,S,n[e+0],11,3936430074),S=i(S,m,h,v,n[e+3],16,3572445317),v=i(v,S,m,h,n[e+6],23,76029189),h=i(h,v,S,m,n[e+9],4,3654602809),m=i(m,h,v,S,n[e+12],11,3873151461),S=i(S,m,h,v,n[e+15],16,530742520),h=c(h,v=i(v,S,m,h,n[e+2],23,3299628645),S,m,n[e+0],6,4096336452),m=c(m,h,v,S,n[e+7],10,1126891415),S=c(S,m,h,v,n[e+14],15,2878612391),v=c(v,S,m,h,n[e+5],21,4237533241),h=c(h,v,S,m,n[e+12],6,1700485571),m=c(m,h,v,S,n[e+3],10,2399980690),S=c(S,m,h,v,n[e+10],15,4293915773),v=c(v,S,m,h,n[e+1],21,2240044497),h=c(h,v,S,m,n[e+8],6,1873313359),m=c(m,h,v,S,n[e+15],10,4264355552),S=c(S,m,h,v,n[e+6],15,2734768916),v=c(v,S,m,h,n[e+13],21,1309151649),h=c(h,v,S,m,n[e+4],6,4149444226),m=c(m,h,v,S,n[e+11],10,3174756917),S=c(S,m,h,v,n[e+2],15,718787259),v=c(v,S,m,h,n[e+9],21,3951481745),h=u(h,o),v=u(v,d),S=u(S,l),m=u(m,C);return g(h)+g(v)+g(S)+g(m)}function o(r,n){return r<<n|r>>>32-n}function u(r,n){var t,e,o,u,a;return o=2147483648&r,u=2147483648&n,a=(1073741823&r)+(1073741823&n),(t=1073741824&r)&(e=1073741824&n)?2147483648^a^o^u:t|e?1073741824&a?3221225472^a^o^u:1073741824^a^o^u:a^o^u}function a(r,n,t,e,a,f,i){return r=u(r,u(u(function(r,n,t){return r&n|~r&t}(n,t,e),a),i)),u(o(r,f),n)}function f(r,n,t,e,a,f,i){return r=u(r,u(u(function(r,n,t){return r&t|n&~t}(n,t,e),a),i)),u(o(r,f),n)}function i(r,n,t,e,a,f,i){return r=u(r,u(u(function(r,n,t){return r^n^t}(n,t,e),a),i)),u(o(r,f),n)}function c(r,n,t,e,a,f,i){return r=u(r,u(u(function(r,n,t){return n^(r|~t)}(n,t,e),a),i)),u(o(r,f),n)}function g(r){var n,t="",e="";for(n=0;n<=3;n++)t+=(e="0"+(r>>>8*n&255).toString(16)).substr(e.length-2,2);return t}var d={generate:e}})(),MD5=n})();

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    document.getElementById("loader").style.display = "block";
    initButtons();
    document.addEventListener('keydown', function (event) {
        if (event.ctrlKey && (event.key === 'z')) {
            initButtons();
        }
    });
}

function convertHTML(str) {
    const symbols = {
        "&amp;" : "&",
        "&lt;" : "<",
        "&gt;" : ">",
        "&quot;" : "\"",
        "&apos;" : "'",
        "&deg;" : '\u00B0',
        "&percnt;" : "%",
        "&commat;" : "@"
    };
    let newStr = str;
    let startStr = str;
    for (const symbol in symbols) {
      if (startStr.indexOf(symbol) >= 0) {
        newStr = startStr.replaceAll(symbol, symbols[symbol]);
        startStr = newStr;
      }
    }
     return newStr;
  }

  function encodeForHTTP(str) {
    const symbols = {
        "%" : "%25",
        "&" : "%26",
        "<" : "%30",
        ">" : "%3E",
        "?" : "%3F",
        "@" : "%40",
        "$" : "%24",
        "#" : "%23"
    };
    let newStr = str;
    let startStr = str;
    //console.log("In string: " + str);
    for (const symbol in symbols) {
      if (startStr.indexOf(symbol) >= 0) {
        newStr = startStr.replaceAll(symbol, symbols[symbol]);
        startStr = newStr; 
      }
    }
     return newStr;
  }

  function removeUnits( str ) {
    const units = { 0 : '\u00B0', 1 : "%", 2 : "min", 3 : "msec", 4 : "sec", 5 : "&" };
    let newStr = str
    for ( j = 0; j <= 5; j++) {
        var i = str.indexOf(units[j]);
      if ( i >= 0 ) {
        newStr = str.substr(0, i);
        j = 6;
      }
    }
     return newStr;
  }

// ----------------------------------------------------------------------------
// Button handling
// ----------------------------------------------------------------------------

function setValue( item ) {
    var itemId = item.id;
    var xhr = new XMLHttpRequest();
    xhr.open('POST', "/getvalue", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.responseType = "json";
    xhr.onreadystatechange = () => {
        if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
            var jsonResponse = xhr.response;
            // console.log("JSON Response: ", jsonResponse);
            var itemId = Object.keys(jsonResponse)[0];
            var inputId = itemId + "I";
            var selectId = itemId + "S";
            var value = Object.values(jsonResponse)[0];

            if (document.getElementById(inputId)) {
                if (document.getElementById(inputId).type == "number")
                    document.getElementById(inputId).dataset.cdc = value;
                else
                    document.getElementById(inputId).dataset.cdc = convertHTML(value);
            } else if (document.getElementById(selectId)) {
                document.getElementById(selectId).dataset.cdc = value;
            } else {
                console.log("No Input or Select found for: " + itemId);
            }
            refreshCount = refreshCount -1;

            if (refreshCount<=0) {
                refreshCount = 0;
                initButtons();
            }
        }
    }
    xhr.send("get=" + itemId);
    refreshCount = refreshCount + 1;
}

function refreshData() {
    //console.log("***Refresh Data***");
    allcButtons = [...document.getElementsByClassName("cButton")];
// console.log(allcButtons);
    allcButtons.forEach( function(element){setValue(element);} );
}

function sendValue( itemId ) {
    var inputId = itemId + "I";
    var selectId = itemId + "S";
    var value;

    document.getElementById("loader").style.display = "block";

    if (document.getElementById(inputId)) {
        value = document.getElementById(inputId).value;
    } else if (document.getElementById(selectId)) {
        value = document.getElementById(selectId).value;
    } else {
        console.log("No Input or Select found for: " + itemId);
        return;
    }

    var xhr = new XMLHttpRequest();
    xhr.open('POST', "/setvalue", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.responseType = "json";
    xhr.onreadystatechange = () => {
        // Call a function when the state changes.
        if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
          refreshData();
        }
      };
    xhr.send(itemId + "=" + encodeForHTTP(value));
    // console.log("Sending: " + itemId + "=" + encodeForHTTP(value));
    document.getElementById(itemId).disabled = true;

    return;
}

function setChangeItem( item ) {
    var itemId = item.id;
    var inputId = itemId + "I";
    var selectId = itemId + "S";
    // console.log(itemId + " - " + inputId );
    item.disabled = true;
    //item.onclick = "sendValue(" + itemId + ");";

    if (document.getElementById(inputId)) {
        var element = document.getElementById(inputId);
        element.oninput = (event) => { document.getElementById(itemId).disabled = false;
            document.getElementById(itemId).onclick = function() {sendValue(itemId);};};
        // console.log("Found inputID: " + inputId + " type: " + element.type);
        switch (element.type){
            case "number":
                element.value = removeUnits(element.dataset.cdc);
                break;
            case "text":
                element.value = element.dataset.cdc;
                break;
            case "url":
                element.value = element.dataset.cdc;
                break;
            default:
                console.log("Bad input type: " + element.type);
                break;
        }
    } else if (document.getElementById(selectId)) {
        var element = document.getElementById(selectId);
        element.onchange = (event) => { document.getElementById(itemId).disabled = false; 
            document.getElementById(itemId).onclick = function() {sendValue(itemId);};};
;
        switch (element.dataset.cdc) {
            case "Automatic":
            case "Weather Query": 
            case "Dew Point":
            case "Open-Meteo":
            case "Disabled":
            case "0":
                element.value = 0;
                break;
            case "Manual":
            case "External Input":
            case "Temperature":
            case "OpenWeather":
            case "Controller":
            case "1":
                element.value = 1;
                break;
            case "Off":
            case "Midpoint":
            case "External Source":
            case "PWM":
                element.value = 2;
                break;
            case "Boolean":
                element.value = 3;
                break;
            default:
                console.log("Bad Select value: " + element.dataset.cdc);
                break;
        }

    } else {
        console.log("No Input or Select found for: " + itemId);
        return
    }

    switch (itemId) {
        case "DCM":
            if (document.getElementById("DCMS").value== 1) {
                document.getElementById("DCO").className = "cButton";
                document.getElementById("DCOI").disabled = false;
            } else {
                document.getElementById("DCO").className = "cButton hidden";
                document.getElementById("DCOI").disabled = true;
            }
            break;
        case "OMIN":
            var min = parseInt(document.getElementById("OMINI").value);
            document.getElementById("DCOI").setAttribute("min", min);
            document.getElementById("OMAXI").setAttribute("min", min + 1);
            break;
        case "OMAX":
            var max = parseInt(document.getElementById("OMAXI").value);
            document.getElementById("DCOI").setAttribute("max", max);
            document.getElementById("OMINI").setAttribute("max", max - 1);
            break;
        case "WS":
            if (document.getElementById("WSS").value == 2) {
                document.getElementById("ATPQ").className = "cButton";
                document.getElementById("ATPQI").disabled = false;
                document.getElementById("HU").className = "cButton";
                document.getElementById("HUI").disabled = false;
            } else {
                document.getElementById("ATPQ").className = "cButton hidden";
                document.getElementById("ATPQI").disabled = true;
                document.getElementById("HU").className = "cButton hidden";
                document.getElementById("HUI").disabled = true;
            }
            break;
        case "CPP0":
        case "CPP1":
        case "CPP2":
        case "CPP3":
        case "CPP4":
        case "CPP5":
            var pin = itemId.substr(3, 1);
            if (document.getElementById(inputId).value == -1) {
                document.getElementById("CPM"+pin).className = "cButton hidden";
                document.getElementById("CPM"+pin+"S").disabled = true;
            } else {
                document.getElementById("CPM"+pin).className = "cButton";
                document.getElementById("CPM"+pin+"S").disabled = false;
            }
            break;
        case "CPM0":
        case "CPM1":
        case "CPM2":
        case "CPM3":
        case "CPM4":
        case "CPM5":
            var pin = itemId.substr(3, 1);
            var mode = document.getElementById(selectId).value;
            switch (mode) {
                case "0":
                case "1":
                    document.getElementById("CPO"+pin).className = "cButton hidden";
                    document.getElementById("CPO"+pin+"I").disabled = true;
                    break;
                case "2":
                    document.getElementById("CPO"+pin).className = "cButton";
                    document.getElementById("CPO"+pin+"I").disabled = false;
                    document.getElementById("CPO"+pin+"I").setAttribute("step", 1);
                    break;
                case "3":
                    document.getElementById("CPO"+pin).className = "cButton";
                    document.getElementById("CPO"+pin+"I").disabled = false;
                    document.getElementById("CPO"+pin+"I").setAttribute("step", 100);
                    break;
                default:
                    break; 
            } 
            break;
        default:
            break;
        
    }
    

}

function initButtons() {
    allcButtons = [...document.getElementsByClassName("cButton")];
// console.log(allcButtons);
    allcButtons.forEach( function(element){setChangeItem(element);} );
    document.getElementById("loader").style.display = "none";
}

function logoutCDC() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/logout", true);
    xhr.send();
    setTimeout(function(){ window.open("/dashboard","_self"); }, 1000);
}

function httpOTAAvailability( value) {
    if (value == "NOSUPPORT") {
        document.getElementById("fwUpdateNotAvailable").style.display = "block";
        document.getElementById("fwUpdateNotAvailable").innerHTML = "Not supported.";
        document.getElementById("fwUpdateAvailable").style.display = "none";
    } else {
        if (value != "NOFWUPDATE") {
            document.getElementById("fwUpdateAvailable").style.display = "block";
            document.getElementById("fwUpdateNotAvailable").style.display = "none";
        } else {
            document.getElementById("fwUpdateNotAvailable").style.display = "block";
            document.getElementById("fwUpdateNotAvailable").innerHTML = "No update available. Currently at latest version.";
            document.getElementById("fwUpdateAvailable").style.display = "none";
        }
    }
}
function updateOTAError(message) {
    var errorElement = document.getElementById("otaerror");
    console.log("OTA update error: " + message)
    if (message=="") {
        document.getElementById("otaerror").style.display = "hidden";
    } else {
        document.getElementById("otaerror").style.display = "block";
    }
    document.getElementById("otaerror").innerHTML = message;
 }

function httpOTAUpload() {
    var password = MD5.generate("admin:CheapoDC:" + document.getElementById("HTTPOTAPWD").value);
    var xhr = new XMLHttpRequest();
    var source = new EventSource('/events');
    //console.log("Set up HTTP OTA");
    if (!!window.EventSource) {
       source.addEventListener('open', (e) => {document.getElementById("status").innerHTML = "HTTP OTA Connected."});
       source.addEventListener('status', (e) => {document.getElementById("filename").innerHTML = e.data});
       source.addEventListener('error', (e) => {if (e.target.readyState != EventSource.OPEN) {updateOTAError("Events Disconnected");}else{updateOTAError(e.data);}});
       source.addEventListener('message', (e) => {document.getElementById("status").innerHTML = e.data;});
       source.addEventListener('progress', (e) => {httpOTAProgress(e.data);});
       source.addEventListener('reboot', (e) => {otaReboot(e.data);});
    } else {
       console.log("something is broken");
    }
    xhr.open("POST", "/updatehtml", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.send("password=" + encodeForHTTP(password));
    //console.log("Sent OTA request");
 }
 function httpOTAProgress( value ) {
    
    if (value > 0) {
        document.getElementById("filesize").innerHTML = " " + value + "&percnt; ";
    } else {
        document.getElementById("filesize").innerHTML = " ";
    }

 }
 function otaReboot( message ) {
    if (message == "Refresh") {
        setTimeout(function () { location.reload(); }, 5000);
    }
 }
 function fileOTAUpload() {
    var file = document.getElementById("file1").files[0];
    document.getElementById("filename").innerHTML = document.getElementById("file1").value.replace(/.*[\/\\]/, '') + ": ";
    var formdata = new FormData();
    formdata.append("method", "file");
    formdata.append("file1", file);
    var ajax = new XMLHttpRequest();
    ajax.upload.addEventListener("progress", progressHandler, false);
    ajax.open("POST", "/update");
    ajax.send(formdata);
 }

 function progressHandler(event) {
    document.getElementById("filesize").innerHTML = event.loaded + " bytes";
    var percent = (event.loaded / event.total) * 100;
    document.getElementById("status").innerHTML = "Completed: " + Math.round(percent) + "&percnt;";
    if (percent >= 100) {
       document.getElementById("status").innerHTML = "Completed: 100&percnt; Wait for Reload.";
       document.getElementById("iupload").disabled = true;
       setTimeout(function () { location.reload(); }, 5000);
    }
 }

 function rebootDevice() {
    if (confirm("Confirm Reboot")) {
       xmlhttp = new XMLHttpRequest();
       xmlhttp.open("GET", "/reboot", true);
       xmlhttp.send();
    }
 }

 function logoutCDC() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/logout", true);
    xhr.send();
    setTimeout(function () { window.open("/dashboard", "_self"); }, 1000);
 }

 function checkPasswordUpdate() {
    if ((document.getElementById("newpassword").value != "") &&
       (document.getElementById("newpassword").value == document.getElementById("confpassword").value) &&
       (document.getElementById("oldpassword").value != "")) {
       document.getElementById('passwordButton').disabled = false;
    } else {
       document.getElementById('passwordButton').disabled = true;
    }
 }

 function changePassword() {
    // MD5(user:realm:pass)
    var newPassword = MD5.generate("admin:CheapoDC:" + document.getElementById("newpassword").value);
    var oldPassword = MD5.generate("admin:CheapoDC:" + document.getElementById("oldpassword").value);
    var confPassword = MD5.generate("admin:CheapoDC:" + document.getElementById("confpassword").value);
    if (newPassword != confPassword) {
       alert("New Passwords do not match");
       document.getElementById('passwordButton').disabled = true;
       return;
    }
    var xhr = new XMLHttpRequest();
    document.getElementById("loader").style.display = "block";
    xhr.open("POST", "/setpassword", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.send("newpassword=" + encodeForHTTP(newPassword) + "&oldpassword=" + encodeForHTTP(oldPassword));
    setTimeout(function () { location.reload(); }, 1000);
 }
 
async function loadWifi() {
    var wifiJson = {};
    var wifiInfo = [];
    var response;
    var wifiSelect = document.getElementById("wifiName");
    var wifiSSID = document.getElementById("wifiSSID");
    var wifiPassword = document.getElementById("wifiPassword");
    var option;
    document.getElementById("wifiButton").disabled = true;
    document.getElementById("wifiPwdCheck").checked = false;
    document.getElementById("wifiPassword").type = "password";
    document.getElementById("loader").style.display = "block";
    try {
      response = await fetch(`CDCWiFi.json`);
      if (!response.ok) {
         throw new Error(`Response status: ${response.status}`);
         wifiJson = {};
         wifiInfo = [];
      } else {
         wifiJson = await response.json();
         wifiInfo = wifiJson.wifi;
      }
    } catch (error) {
      console.error(error.message);
    }
    
    for (var i = wifiSelect.options.length-1; i >= 0; i--) {
      wifiSelect.remove(i);
    }
    for (var i = 0; i < wifiInfo.length; i++) {
        option = document.createElement("option");
        option.text = wifiInfo[i].ssid;
        option.value = i;
        wifiSelect.add(option, wifiSelect[i]);
    }
    
    option = document.createElement("option");
    option.text = "Add Wifi";
    option.value = wifiInfo.length;
    wifiSelect.add(option, wifiSelect[wifiInfo.length]);
    if (wifiInfo.length == 0) {
      wifiSSID.value = "";
      wifiSSID.placeholder = "Enter SSID";
      wifiPassword.value = "";
      wifiPassword.placeholder = "Enter Password";
    } else {
        wifiSSID.value = wifiInfo[0].ssid;  
        wifiPassword.value = wifiInfo[0].password;
    }
    document.getElementById("loader").style.display = "none";
    wifiSelect.addEventListener("change", function() {
        var index = wifiSelect.value;
        document.getElementById("wifiPassword").type = "password";
        document.getElementById("wifiPwdCheck").checked = false;
        if (index == wifiInfo.length) {
            wifiSSID.value = "";
            wifiSSID.placeholder = "Enter SSID";
            wifiPassword.value = "";
            wifiPassword.placeholder = "Enter Password";
            return;
        }
        wifiSSID.value = wifiInfo[index].ssid;
        wifiPassword.value = wifiInfo[index].password;
    });
    wifiSSID.addEventListener("input", function() {document.getElementById("wifiButton").disabled = false;  });
    wifiPassword.addEventListener("input", function() {document.getElementById("wifiButton").disabled = false;  });
    document.getElementById("wifiButton").addEventListener("click", function() {updateWifi(wifiSelect, wifiSSID, wifiPassword, wifiInfo, wifiJson);});
}
 
function updateWifi(wifiSelect, wifiSSID, wifiPassword, wifiInfo, wifiJson) {
    var index = wifiSelect.value;
    let newWifiJson = {...wifiJson};
    document.getElementById("loader").style.display = "block";
    if (index == wifiInfo.length) {
        var newWifi = {};
        newWifi.ssid = wifiSSID.value;
        newWifi.password = wifiPassword.value;
        wifiInfo.push(newWifi);
    } else {
        wifiInfo[index].ssid = wifiSSID.value;
        wifiInfo[index].password = wifiPassword.value;
    }
    var newWifiInfo = [];

    for (var i = 0; i < wifiInfo.length; i++) {
        if (wifiInfo[i].ssid == "") {
            continue;
        }
        newWifiInfo.push(wifiInfo[i]);
    }
    newWifiJson.wifi = newWifiInfo;

    var xhr = new XMLHttpRequest();
    document.getElementById("loader").style.display = "block";
    xhr.open("POST", "/setwifi", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            //document.getElementById("loader").style.display = "none";
            loadWifi();
        }
    };
    xhr.send("wifi=" + encodeForHTTP(JSON.stringify(newWifiJson)));
}
function showWifiPassword() {
  var x = document.getElementById("wifiPassword");
  var wifiSelect = document.getElementById("wifiName");
  if (wifiSelect.value == wifiSelect.options.length-1) {
    if (x.type === "password") {
      x.type = "text";
    } else {
      x.type = "password";
    }
  } else {
   document.getElementById("wifiPwdCheck").checked = false;
  }
}