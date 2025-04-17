// Connect to the robot
async function connectToRobot() {
    const ip = document.getElementById("ip").value;
    const port = document.getElementById("port").value;

    const response = await fetch("/connect", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ip, port: parseInt(port) })
    });

    const msg = await response.text();
    alert(msg);
}

// Handle drive form submission
async function handleSend(event) {
    event.preventDefault(); // Prevent form from refreshing the page

    const cmd = document.getElementById("direction").value;
    const duration = parseInt(document.getElementById("duration").value);
    const speed = parseInt(document.getElementById("speed").value);

    await sendCommand(cmd, duration, speed);
}

// Send a generic command to the robot
async function sendCommand(cmd, duration, speed) {
    const response = await fetch("/telecommand/", {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command: cmd, duration: duration, angle: speed })
    });

    const result = await response.text();
    document.getElementById("response").innerText = result;
    showToast("✅ Command sent successfully");
}

// Send a dedicated sleep command
async function sendSleepCommand() {
    const confirmSleep = confirm("Are you sure you want to put the robot to sleep?");
    if (!confirmSleep) return;

    const response = await fetch("/telecommand/", {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command: "sleep", duration: 0, angle: 0 })
    });

    const result = await response.text();
    document.getElementById("response").innerText = result;
    showToast("💤 Robot put to sleep");
}

// Request telemetry from robot
async function requestTelemetry() {
    const response = await fetch("/telementry_request/");
    const result = await response.text();
    document.getElementById("response").innerText = result;
    showToast("📡 Telemetry received");
}

// Show toast notification
function showToast(message) {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.className = "toast show";

    setTimeout(() => {
        toast.className = toast.className.replace("show", "");
    }, 3000);
}
