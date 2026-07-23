/*var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var wserrcnt = 0;
var wstimeout;

window.addEventListener('load', onLoad);

function initWebSocket() {
  clearTimeout(wstimeout);
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log('Connection opened');
}
function onClose(event) {
  console.log('Connection closed');
  location.href="/";
}

function onMessage(event) {
  var data = JSON.parse(event.data);
  if(data.ircode){
    document.getElementById('protocol').innerText=data.protocol;
    var elements = document.getElementsByClassName("irrecordvalue");
    for (var i = 0; i < elements.length; i++) {
      if(elements[i].classList.contains("active")){
        elements[i].innerText='0x'+data.ircode.toString(16).toUpperCase();
        break;
      }
    }
  }
  if(data.irvals){
    var elements = document.getElementsByClassName("irrecordvalue");
    for (var i = 0; i < elements.length; i++) {
      var val = data.irvals[i];
      if(val>0){
        elements[i].innerText='0x'+val.toString(16).toUpperCase();
      }else{
        elements[i].innerText="";
      }
    }
  }
}
*/
var irloaded = false;

function setIrCsvStatus(message, isError) {
  var status = document.getElementById('ircsvstatus');
  if (!status) return;
  status.innerText = message || '';
  status.classList.remove('error');
  status.classList.remove('success');
  if (message) {
    status.classList.add(isError ? 'error' : 'success');
  }
}

function exportIrCsv() {
  setIrCsvStatus('Export in progress...', false);
  try {
    var url = '/ircodes.csv?ts=' + Date.now();
    var a = document.createElement('a');
    a.href = url;
    a.download = 'ircodes.csv';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(function () {
      setIrCsvStatus('Export started.', false);
    }, 150);
  } catch (err) {
    setIrCsvStatus('Export failed.', true);
  }
}

async function importIrCsv(file) {
  if (!file) return;
  setIrCsvStatus('Import in progress...', false);
  try {
    var response = await fetch('/ircodes.csv', {
      method: 'POST',
      headers: { 'Content-Type': 'text/csv' },
      body: file
    });

    if (!response.ok) {
      var errText = 'Import failed.';
      try {
        var json = await response.json();
        if (json && json.error) errText = 'Import failed: ' + json.error;
      } catch (e) {
      }
      throw new Error(errText);
    }

    setIrCsvStatus('Import done.', false);
  } catch (err) {
    setIrCsvStatus(err.message || 'Import failed.', true);
  }
}

function checkSelect() {
  var elements = document.getElementsByClassName("irradio");
  var chkid = 0;
  for (var i = 0; i < elements.length; i++) {
    elements[i].classList.remove("active");
    elements[i].parentElement.getElementsByClassName("irrecordvalue")[0].classList.remove("active");
    if (elements[i] === this) chkid = i;
  }
  var ts = this !== window ? this : elements[0];
  ts.classList.add("active");
  ts.parentElement.getElementsByClassName("irrecordvalue")[0].classList.add("active");
  if (this !== window) websocket.send('chkid=' + chkid);
  document.getElementById('protocol').innerText = "";
}

function irbuttonClick() {
  var elements = document.getElementsByClassName("irbutton");
  var hasactive = this.classList.contains("active");
  var btnid = -1;
  for (var i = 0; i < elements.length; i++) {
    elements[i].classList.remove("active");
    if (!hasactive && elements[i] == this) btnid = i;
  }
  if (!hasactive) {
    document.getElementById("irrecordtitle").innerHTML = 'REC codes for <span>' + this.innerHTML + '</span>';
    document.getElementById("irrecord").classList.remove("hidden");
    document.getElementById("irstartrecord").classList.add("hidden");
    this.classList.add("active");
    checkSelect();
  } else {
    document.getElementById("irrecord").classList.add("hidden");
    document.getElementById("irstartrecord").classList.remove("hidden");
  }
  document.getElementById('protocol').innerText = "";
  websocket.send('irbtn=' + btnid);
}

function backRecord() {
  var elements = document.getElementsByClassName("irbutton");
  for (var i = 0; i < elements.length; i++) {
    elements[i].classList.remove("active");
  }
  document.getElementById("irrecord").classList.add("hidden");
  document.getElementById("irstartrecord").classList.remove("hidden");
  websocket.send('irbtn=-1');
}
function irClear(el) {
  el.parentElement.getElementsByClassName("irrecordvalue")[0].innerText = "";
  document.getElementById('protocol').innerText = "";
  websocket.send('irclr=' + el.parentElement.getElementsByClassName("irradio")[0].getAttribute('data-id'));
}
function initControls() {
  if (irloaded) return;
  irloaded = true;
  var elements = document.getElementsByClassName("irbutton");
  for (var i = 0; i < elements.length; i++) {
    elements[i].addEventListener('click', irbuttonClick, false);
  }
  elements = document.getElementsByClassName("irradio");
  for (var i = 0; i < elements.length; i++) {
    elements[i].addEventListener('click', checkSelect, false);
  }

  var exportBtn = document.getElementById('ir_export_csv');
  if (exportBtn) {
    exportBtn.addEventListener('click', exportIrCsv, false);
  }

  var importInput = document.getElementById('ir_import_csv');
  if (importInput) {
    importInput.addEventListener('change', function () {
      importIrCsv(this.files && this.files[0]);
      this.value = '';
    }, false);
  }
}
/*function onLoadIR(event) {
  initWebSocket();
  initControls();
}*/
