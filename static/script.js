// Connect to the robot with selected protocol
async function connectToRobot() {
    const ip = document.getElementById("ip").value;
    const port = document.getElementById("port").value;
    const protocol = document.getElementById("protocol").value;

    const response = await fetch("/connect", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ip, port: parseInt(port), protocol })
    });

    const msg = await response.text();
    alert(msg);
}

// Handle drive form submission
async function handleSend(event) {
    event.preventDefault(); // Prevent form refresh
    const cmd = document.getElementById("direction").value;
    const duration = parseInt(document.getElementById("duration").value);
    const speed = parseInt(document.getElementById("speed").value);
    await sendCommand(cmd, duration, speed);
}

// Send drive or sleep command to robot
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

// Send sleep command with confirmation
async function sendSleepCommand() {
    const confirmSleep = confirm("Are you sure you want to put the robot to sleep?");
    if (!confirmSleep) return;

    await sendCommand("sleep", 0, 0);
}

// Request telemetry data from robot
async function requestTelemetry() {
    const response = await fetch("/telementry_request/");
    const result = await response.text();
    document.getElementById("response").innerText = result;
    showToast("📡 Telemetry received");
}

// Toast notification
function showToast(message) {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.className = "toast show";
    setTimeout(() => {
        toast.className = toast.className.replace("show", "");
    }, 3000);
}
