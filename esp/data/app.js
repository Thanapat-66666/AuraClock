const wifiList = document.getElementById("wifiList");
const savedList = document.getElementById("savedList");
const savedCount = document.getElementById("savedCount");

const scanBtn = document.getElementById("scanBtn");

const modal = document.getElementById("modal");
const ssidTitle = document.getElementById("ssidTitle");
const passwordInput = document.getElementById("password");
const connectBtn = document.getElementById("connectBtn");
const cancelBtn = document.getElementById("cancelBtn");
const togglePass = document.getElementById("togglePass");

let selectedSSID = "";
let remembered = [];

// ----------------
function signalText(rssi) {
  if (rssi >= -55) return "Excellent";
  if (rssi >= -67) return "Strong";
  if (rssi >= -75) return "Good";
  if (rssi >= -85) return "Fair";
  return "Weak";
}

// ----------------
function openModal(ssid) {
  selectedSSID = ssid;
  ssidTitle.textContent = ssid;
  passwordInput.value = "";
  passwordInput.type = "password";
  togglePass.textContent = "👁";
  modal.classList.remove("hidden");

  setTimeout(() => passwordInput.focus(), 150);
}

// ----------------
function closeModal() {
  modal.classList.add("hidden");
}

// ----------------
function renderSaved() {
  if (!remembered.length) {
    savedCount.textContent = "0 saved";

    savedList.innerHTML = `
      <div class="loading">
        No saved networks
      </div>
    `;
    return;
  }

  savedCount.textContent =
    `${remembered.length} saved`;

  savedList.innerHTML =
    remembered.map(ssid => `
      <div class="wifi-item">
        <div class="wifi-name">${ssid}</div>
        <div class="saved-badge">Saved</div>
      </div>
    `).join("");
}

// ----------------
function renderWiFi(list) {
  if (!list.length) {
    wifiList.innerHTML = `
      <div class="loading">
        No Wi-Fi found
      </div>
    `;
    return;
  }

  wifiList.innerHTML = list.map(item => {
    const isSaved =
      remembered.includes(item.ssid);

    return `
      <div
        class="wifi-item"
        data-ssid="${item.ssid}"
      >
        <div class="wifi-name">
          ${item.ssid}
        </div>

        <div class="wifi-strength">
          ${isSaved ? "Saved • " : ""}
          ${signalText(item.rssi)}
        </div>
      </div>
    `;
  }).join("");

  document
    .querySelectorAll(".wifi-item[data-ssid]")
    .forEach(el => {
      el.onclick = () =>
        openModal(el.dataset.ssid);
    });
}

// ----------------
async function loadSaved() {
  try {
    const res = await fetch("/saved");
    const data = await res.json();

    remembered = data || [];

    renderSaved();
  } catch {
    remembered = [];
    renderSaved();
  }
}

// ----------------
async function syncTime() {
  try {
    await fetch("/time", {
      method: "POST",
      headers: {
        "Content-Type":"application/json"
      },
      body: JSON.stringify({
        epoch: Math.floor(Date.now()/1000)
      })
    });
  } catch {}
}

// ----------------
async function scanWiFi() {
  wifiList.innerHTML = `
    <div class="loading">
      Scanning nearby Wi-Fi...
    </div>
  `;

  try {
    const res = await fetch("/scan");
    const data = await res.json();

    renderWiFi(data);
  } catch {
    wifiList.innerHTML = `
      <div class="loading">
        Scan failed
      </div>
    `;
  }
}

// ----------------
async function connectWiFi() {
  const password =
    passwordInput.value.trim();

  connectBtn.textContent =
    "Connecting...";

  connectBtn.disabled = true;

  try {
    const res = await fetch("/connect", {
      method: "POST",
      headers: {
        "Content-Type":"application/json"
      },
      body: JSON.stringify({
        ssid: selectedSSID,
        password
      })
    });

    const data = await res.json();

    if (data.ok) {
      document.body.innerHTML = `
        <main class="app">
          <section class="hero">
            <div class="logo">AuraClock</div>
            <h1>Connected</h1>
            <p>Returning to clock...</p>
          </section>
        </main>
      `;
      return;
    }

    alert("Connection failed");
  } catch {
    alert("Connection failed");
  }

  connectBtn.textContent = "Connect";
  connectBtn.disabled = false;
}

// ----------------
togglePass.onclick = () => {
  const hidden =
    passwordInput.type === "password";

  passwordInput.type =
    hidden ? "text" : "password";

  togglePass.textContent =
    hidden ? "🙈" : "👁";
};

// ----------------
scanBtn.onclick = scanWiFi;
cancelBtn.onclick = closeModal;
connectBtn.onclick = connectWiFi;

modal.onclick = e => {
  if (e.target === modal) {
    closeModal();
  }
};

// boot
(async () => {
  await syncTime();
  await loadSaved();
  await scanWiFi();
})();