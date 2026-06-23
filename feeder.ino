#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP32Servo.h>
#include <WebServer.h>

// ======= CONFIGURAÇÕES DO WIFI =======
const char* ssid     = "AMF";
const char* password = "amf@2025";

// ======= HORÁRIOS DE ALIMENTAÇÃO =======
// Formato: {hora, minuto}
int horarios[][2] = {
  {8,  0},
  {14, 0},
  {20, 0}
};

int totalHorarios = 3;

// ======= CONFIGURAÇÕES DO SERVO =======
const int servoPin = 13;

const int posFechado = 12;
int posAberto  = 55;

int tempoAberto = 5; // segundos

Servo feederServo;

// ======= CONTROLE DA COMPORTA =======
bool alimentando = false;
unsigned long inicioAlimentacao = 0;

// ======= NTP / HORÁRIO =======
WiFiUDP ntpUDP;

// Brasil UTC-3
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000);

// Guarda o último horário acionado
int ultimaHoraAcionada = -1;
int ultimoMinutoAcionado = -1;

// ======= SERVIDOR WEB =======
WebServer server(80);

// ======= SENSOR DE NÍVEL =======
const int sensorPowerPin = 25;
const int trigPin = 26;
const int echoPin = 27;

float distanciaCheio = 4.0;
float distanciaVazio = 20.0;

float distanciaAtual = 0;
int percentualRacao = 100;
bool nivelBaixo = false;
unsigned long ultimaLeituraSensor = 0;


// ======= CONTROLE DE PRINT =======
unsigned long ultimoPrintHorario = 0;

// ======= SITE BONITINHO NOVO =======
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />

  <title>PetFeeder</title>

  <style>
    :root {
      --surface: #f8f9fa;
      --surface-low: #f3f4f5;
      --surface-high: #e7e8e9;
      --on-surface: #191c1d;
      --on-surface-variant: #564338;
      --primary: #9b4500;
      --primary-container: #ff8c42;
      --on-primary-container: #6a2d00;
      --secondary: #006970;
      --secondary-container: #9df0f8;
      --inverse-surface: #2e3132;
    }

    * {
      box-sizing: border-box;
      -webkit-tap-highlight-color: transparent;
    }

    body {
      margin: 0;
      min-height: 100vh;
      background: var(--surface);
      color: var(--on-surface);
      font-family: Georgia, "Times New Roman", serif;
      overflow-x: hidden;
    }

    header {
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 66px;
      padding: 0 20px;
      background: rgba(248, 249, 250, 0.96);
      box-shadow: 0 2px 12px rgba(0,0,0,0.05);
      display: flex;
      align-items: center;
      justify-content: space-between;
      z-index: 20;
    }

    .brand {
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .avatar {
      width: 43px;
      height: 43px;
      border-radius: 999px;
      background: var(--primary-container);
      border: 3px solid var(--primary-container);
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 26px;
      box-shadow: 0 5px 12px rgba(155, 69, 0, 0.25);
      overflow: hidden;
    }

    h1 {
      margin: 0;
      color: var(--primary);
      font-size: 31px;
      line-height: 1;
      font-weight: 900;
      letter-spacing: -1px;
    }

    .settings {
      width: 44px;
      height: 44px;
      border: none;
      border-radius: 999px;
      background: transparent;
      color: var(--primary);
      font-size: 30px;
      cursor: pointer;
      font-weight: 900;
    }

    main {
      min-height: 100vh;
      padding: 112px 20px 112px;
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
    }

    .content {
      width: 100%;
      max-width: 420px;
      margin-top: 62px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    .status-pill {
      display: flex;
      align-items: center;
      gap: 8px;
      background: rgba(157, 240, 248, 0.42);
      border: 1px solid #88e6ef;
      color: var(--secondary);
      border-radius: 999px;
      padding: 6px 14px;
      font-size: 17px;
      line-height: 1;
      font-weight: 900;
      margin-bottom: 18px;
      box-shadow: 0 5px 15px rgba(0, 105, 112, 0.08);
    }

    .dot {
      width: 21px;
      height: 21px;
      background: rgba(0, 105, 112, 0.55);
      border-radius: 999px;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .dot::after {
      content: "";
      width: 14px;
      height: 14px;
      background: var(--secondary);
      border-radius: 999px;
      display: block;
    }

    .subtitle {
      margin: 0;
      max-width: 280px;
      color: var(--on-surface-variant);
      font-size: 22px;
      line-height: 1.35;
      font-weight: 500;
    }

    .button-area {
      position: relative;
      width: 285px;
      height: 330px;
      margin-top: 24px;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .paw-bg {
      position: absolute;
      top: -62px;
      right: 5px;
      width: 160px;
      height: 150px;
      opacity: 0.23;
      transform: rotate(18deg);
      pointer-events: none;
    }

    .paw-bg .pad {
      position: absolute;
      border-radius: 50%;
      background: #ddc1b3;
    }

    .paw-bg .main-pad {
      width: 96px;
      height: 78px;
      left: 34px;
      top: 58px;
      border-radius: 50% 50% 45% 45%;
      transform: rotate(8deg);
    }

    .paw-bg .toe1 {
      width: 44px;
      height: 44px;
      left: 0;
      top: 45px;
    }

    .paw-bg .toe2 {
      width: 44px;
      height: 44px;
      left: 43px;
      top: 12px;
    }

    .paw-bg .toe3 {
      width: 44px;
      height: 44px;
      right: 32px;
      top: 12px;
    }

    .paw-bg .toe4 {
      width: 44px;
      height: 44px;
      right: -10px;
      top: 45px;
    }

    .glow {
      position: absolute;
      width: 275px;
      height: 275px;
      border-radius: 999px;
      background: rgba(255, 140, 66, 0.16);
      filter: blur(28px);
    }

    .feed-button {
      position: relative;
      z-index: 2;
      width: 255px;
      height: 255px;
      border-radius: 999px;
      border: 8px solid white;
      background: var(--primary-container);
      color: #5a2500;
      box-shadow:
        0 22px 42px rgba(155, 69, 0, 0.28),
        0 0 0 1px rgba(255,255,255,0.8);
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      transition: transform .15s ease, opacity .15s ease;
      overflow: hidden;
      font-family: Georgia, "Times New Roman", serif;
    }

    .feed-button:active {
      transform: scale(0.93);
    }

    .feed-button:disabled {
      opacity: 0.72;
      pointer-events: none;
    }

    .button-icon {
      width: 112px;
      height: 112px;
      border-radius: 999px;
      background: rgba(255, 188, 133, 0.75);
      display: flex;
      align-items: center;
      justify-content: center;
      color: #6a2d00;
      font-size: 58px;
      margin-bottom: 24px;
      font-family: Arial, Helvetica, sans-serif;
      font-weight: 900;
    }

    .button-text {
      font-size: 30px;
      line-height: 1.15;
      font-weight: 900;
      color: #5a2500;
    }

    .loading {
      animation: spin 1s linear infinite;
    }

    @keyframes spin {
      from { transform: rotate(0deg); }
      to { transform: rotate(360deg); }
    }

    .hearts {
      position: absolute;
      bottom: 14px;
      color: rgba(155, 69, 0, 0.22);
      font-size: 34px;
      letter-spacing: 3px;
      z-index: 3;
      pointer-events: none;
    }

    .last-feed {
      margin-top: 12px;
      color: var(--on-surface);
    }

    .last-feed small {
      display: block;
      color: rgba(86, 67, 56, 0.58);
      font-size: 16px;
      font-weight: 900;
      letter-spacing: 1.8px;
      text-transform: uppercase;
      margin-bottom: 16px;
    }

    .last-line {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 12px;
      color: var(--on-surface);
      font-size: 20px;
      line-height: 1.25;
      font-weight: 500;
    }

    .clock {
      font-size: 16px;
      font-family: Arial, Helvetica, sans-serif;
    }

    nav {
      position: fixed;
      bottom: 0;
      left: 0;
      width: 100%;
      height: 76px;
      background: rgba(243, 244, 245, 0.96);
      box-shadow: 0 -4px 18px rgba(0,0,0,.06);
      border-radius: 18px 18px 0 0;
      display: flex;
      justify-content: space-around;
      align-items: center;
      z-index: 20;
      padding: 0 42px;
    }

    .nav-item {
      color: var(--on-surface-variant);
      font-size: 16px;
      font-weight: 900;
      text-decoration: none;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      min-width: 84px;
      gap: 2px;
    }

    .nav-item.active {
      background: var(--primary-container);
      color: var(--on-primary-container);
      width: 78px;
      height: 58px;
      border-radius: 999px;
    }

    .nav-icon {
      font-size: 25px;
      line-height: 1;
      font-family: Arial, Helvetica, sans-serif;
    }

    .overlay {
      position: fixed;
      inset: 0;
      background:
        radial-gradient(circle at center, rgba(255, 140, 66, 0.5), transparent 38%),
        var(--primary);
      z-index: 100;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      opacity: 0;
      pointer-events: none;
      transition: opacity .35s ease;
      color: white;
      text-align: center;
      padding: 20px;
    }

    .overlay.show {
      opacity: 1;
      pointer-events: all;
    }

    .check {
      width: 145px;
      height: 145px;
      background: white;
      color: var(--primary);
      border-radius: 999px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 84px;
      margin-bottom: 28px;
      animation: bounce .7s ease infinite alternate;
      box-shadow: 0 20px 40px rgba(0,0,0,0.22);
      font-family: Arial, Helvetica, sans-serif;
    }

    @keyframes bounce {
      from { transform: translateY(0); }
      to { transform: translateY(-14px); }
    }

    .overlay h2 {
      margin: 0;
      font-size: 32px;
      font-weight: 900;
    }

    .overlay p {
      margin-top: 10px;
      font-size: 18px;
      opacity: 0.86;
    }

    .toast {
      position: fixed;
      left: 50%;
      bottom: 92px;
      transform: translateX(-50%);
      max-width: calc(100% - 40px);
      background: var(--inverse-surface);
      color: white;
      padding: 12px 18px;
      border-radius: 14px;
      font-size: 14px;
      opacity: 0;
      pointer-events: none;
      transition: opacity .25s ease, bottom .25s ease;
      z-index: 200;
      box-shadow: 0 12px 28px rgba(0,0,0,0.22);
      font-family: Arial, Helvetica, sans-serif;
    }

    .toast.show {
      opacity: 1;
      bottom: 104px;
    }

    @media (max-height: 780px) {
      .content {
        margin-top: 35px;
      }

      .button-area {
        margin-top: 8px;
        transform: scale(0.92);
      }

      .last-feed {
        margin-top: -10px;
      }
    }

    @media (max-width: 360px) {
      h1 {
        font-size: 26px;
      }

      .feed-button {
        width: 230px;
        height: 230px;
      }

      .button-area {
        width: 260px;
      }

      .button-text {
        font-size: 26px;
      }

      nav {
        padding: 0 24px;
      }
    }
  </style>
</head>

<body>
  <header>
    <div class="brand">
      <div class="avatar">🐶</div>
      <h1>PetFeeder</h1>
    </div>

    <button class="settings" onclick="abrirStatus()">⚙</button>
  </header>

  <main>
    <div class="content">
      <section class="status-pill">
        <span class="dot"></span>
        <span id="connectionText">Conectado</span>
      </section>

      <p class="subtitle">
        O alimentador está conectado
      </p>

      <section class="button-area">
        <div class="paw-bg">
          <span class="pad main-pad"></span>
          <span class="pad toe1"></span>
          <span class="pad toe2"></span>
          <span class="pad toe3"></span>
          <span class="pad toe4"></span>
        </div>

        <div class="glow"></div>

        <button class="feed-button" id="feedBtn" onclick="liberarRacao()">
          <div class="button-icon" id="buttonIcon">🍴</div>
          <div class="button-text" id="buttonText">Liberar<br>Ração</div>
        </button>

        <div class="hearts">♡ ♡ ♡</div>
      </section>

      <section class="last-feed">
        <small>Última Refeição</small>

        <div class="last-line">
          <span class="clock">◷</span>
          <span id="lastFeed">Hoje às 08:30 •<br>120g</span>
        </div>

      </section>

      <section class="last-feed">
        <small>Nível de Ração</small>

        <div style="margin-top:12px">
          <div style="width:100%;height:18px;background:#ddd;border-radius:999px;overflow:hidden;">
            <div id="foodFill" style="width:100%;height:100%;background:#4caf50;transition:0.4s;"></div>
          </div>

          <div id="foodPercent" style="margin-top:10px;font-size:22px;font-weight:900;color:#006970;">
            100%
          </div>
        </div>

      </section>
    </div>
  </main>

  <nav>
    <a class="nav-item active" href="/">
      <span class="nav-icon">🐾</span>
      <span>Feed</span>
    </a>

    <a class="nav-item" href="/schedule">
      <span class="nav-icon">◷</span>
      <span>Schedule</span>
    </a>
  </nav>

  <div class="overlay" id="successOverlay">
    <div class="check">✓</div>
    <h2>Ração Liberada!</h2>
    <p>Comando enviado para o alimentador.</p>
  </div>

  <div class="toast" id="toast"></div>

  <script>
    const feedBtn = document.getElementById("feedBtn");
    const buttonIcon = document.getElementById("buttonIcon");
    const buttonText = document.getElementById("buttonText");
    const overlay = document.getElementById("successOverlay");
    const lastFeed = document.getElementById("lastFeed");
    const toast = document.getElementById("toast");
    const connectionText = document.getElementById("connectionText");

    function showToast(message) {
      toast.innerText = message;
      toast.classList.add("show");

      setTimeout(() => {
        toast.classList.remove("show");
      }, 3000);
    }

    function abrirStatus() {
      window.location.href = "/status";
    }

    function horaAtual() {
      const agora = new Date();
      return agora.toLocaleTimeString("pt-BR", {
        hour: "2-digit",
        minute: "2-digit"
      });
    }

    async function liberarRacao() {
      feedBtn.disabled = true;
      buttonIcon.innerText = "⟳";
      buttonIcon.classList.add("loading");
      buttonText.innerHTML = "Liberando...";

      try {
        const resposta = await fetch("/api/alimentar");

        if (!resposta.ok) {
          throw new Error("Erro HTTP: " + resposta.status);
        }

        const dados = await resposta.json();

        if (dados.status === "ok") {
          lastFeed.innerHTML = "Hoje às " + horaAtual() + " •<br>120g";

          overlay.classList.add("show");

          setTimeout(() => {
            overlay.classList.remove("show");
          }, 1700);

          showToast("Ração liberada com sucesso.");
        } else if (dados.status === "busy") {
          showToast("O alimentador já está liberando ração.");
        } else {
          showToast(dados.mensagem || "Não foi possível liberar a ração.");
        }

      } catch (erro) {
        showToast("Erro ao conectar com o ESP32.");
      }

      buttonIcon.classList.remove("loading");
      buttonIcon.innerText = "🍴";
      buttonText.innerHTML = "Liberar<br>Ração";
      feedBtn.disabled = false;
    }

    async function carregarStatus() {
      try {
        const resposta = await fetch("/api/status");
        const dados = await resposta.json();

        if (dados.status === "ok") {
          if (dados.isFeeding) {
            connectionText.innerText = "Liberando";
          } else {
            connectionText.innerText = "Conectado";
          }

          const foodFill = document.getElementById("foodFill");
          const foodPercent = document.getElementById("foodPercent");

          if (foodFill && foodPercent) {
            foodFill.style.width = dados.percentualRacao + "%";
            foodPercent.innerText = dados.percentualRacao + "%";

            if (dados.percentualRacao <= 10) {
              foodFill.style.background = "#f44336";
            } else if (dados.percentualRacao <= 30) {
              foodFill.style.background = "#ff9800";
            } else {
              foodFill.style.background = "#4caf50";
            }
          }
        } else {
          connectionText.innerText = "Sem status";
        }
      } catch (erro) {
        connectionText.innerText = "Sem resposta";
      }
    }

    carregarStatus();
    setInterval(carregarStatus, 3000);
  </script>
</body>
</html>
)rawliteral";

// ======================================================
// SETUP / LOOP
// ======================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===================================");
  Serial.println("  ALIMENTADOR AUTOMATICO ESP32");
  Serial.println("===================================");

  configurarServo();
  configurarSensor();
  conectarWiFi();
  configurarHorario();
  configurarServidor();

  Serial.println("-----------------------------------");
  Serial.println("Comandos pelo Monitor Serial:");
  Serial.println("Digite 'a' e aperte Enter para liberar racao.");
  Serial.println("Digite 'h' e aperte Enter para ver o horario atual.");
  Serial.println("-----------------------------------");
}

void loop() {
  server.handleClient();

  atualizarAlimentacao();
  atualizarNivelRacao();

  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    verificarHorarioProgramado();
  }

  verificarComandoSerial();
  mostrarHorarioPeriodicamente();

  delay(50);
}

// ======================================================
// CONFIGURAÇÕES
// ======================================================

void configurarServo() {
  Serial.println("Configurando servo...");

  feederServo.setPeriodHertz(50);

  // Valores comuns para servo: 500 a 2400 microssegundos
  feederServo.attach(servoPin, 500, 2400);

  fecharComporta();

  Serial.print("Servo configurado no GPIO ");
  Serial.println(servoPin);
}

void conectarWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi conectado!");
    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Nao foi possivel conectar ao WiFi.");
    Serial.println("O sistema continuara funcionando pelo Monitor Serial.");
    Serial.println("Sem WiFi, os horarios por NTP nao funcionam.");
  }
}

void configurarHorario() {
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.begin();
    timeClient.update();

    Serial.println("Cliente NTP iniciado.");
    mostrarHorarioAtual();
  }
}

void configurarServidor() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Servidor web nao iniciado porque nao ha WiFi.");
    return;
  }

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/alimentar", HTTP_GET, []() {
    Serial.println("Comando recebido pela rota antiga /alimentar.");

    bool iniciou = iniciarLiberacaoRacao();

    if (iniciou) {
      server.send(200, "text/html",
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>Racao liberada</title></head>"
        "<body style='font-family:Arial;text-align:center;padding:40px;background:#f8f9fa;'>"
        "<h1 style='color:#9b4500;'>Racao liberada!</h1>"
        "<p>O comando foi enviado para o servo.</p>"
        "<p><a href='/'><button style='font-size:22px;padding:14px 24px;border-radius:999px;background:#ff8c42;border:0;'>Voltar</button></a></p>"
        "</body></html>"
      );
    } else {
      server.send(200, "text/html",
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>Ocupado</title></head>"
        "<body style='font-family:Arial;text-align:center;padding:40px;background:#f8f9fa;'>"
        "<h1 style='color:#9b4500;'>Alimentador ocupado</h1>"
        "<p>Ele ja esta liberando racao.</p>"
        "<p><a href='/'><button style='font-size:22px;padding:14px 24px;border-radius:999px;background:#ff8c42;border:0;'>Voltar</button></a></p>"
        "</body></html>"
      );
    }
  });

  server.on("/api/alimentar", HTTP_GET, []() {
    Serial.println("Comando recebido pelo site bonito.");

    bool iniciou = iniciarLiberacaoRacao();

    if (iniciou) {
      server.send(200, "application/json",
        "{\"status\":\"ok\",\"mensagem\":\"Liberando racao\"}"
      );
    } else {
      server.send(200, "application/json",
        "{\"status\":\"busy\",\"mensagem\":\"Alimentador ja esta liberando racao\"}"
      );
    }
  });

  server.on("/api/status", HTTP_GET, []() {
    String json = "{";

    json += "\"status\":\"ok\",";
    json += "\"device\":\"PetFeeder ESP32\",";
    json += "\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",";
    json += "\"horario\":\"";
    json += obterHorarioFormatado();
    json += "\",";
    json += "\"isFeeding\":";
    json += alimentando ? "true" : "false";
    json += ",";
    json += "\"servoPin\":";
    json += String(servoPin);
    json += ",";
    json += "\"posFechado\":";
    json += String(posFechado);
    json += ",";
    json += "\"posAberto\":";
    json += String(posAberto);
    json += ",";
    json += "\"tempoAberto\":";
    json += String(tempoAberto);
    json += ",";
    json += "\"percentualRacao\":";
    json += String(percentualRacao);
    json += ",";
    json += "\"distancia\":";
    json += String(distanciaAtual);
    json += ",";
    json += "\"nivelBaixo\":";
    json += nivelBaixo ? "true" : "false";

    json += "}";

    server.send(200, "application/json", json);
  });

  server.on("/status", HTTP_GET, []() {
    enviarPaginaStatus();
  });

  server.on("/schedule", HTTP_GET, []() {
    enviarPaginaSchedule();
  });

  server.begin();

  Serial.println("Servidor web iniciado.");
  Serial.print("Acesse pelo navegador: http://");
  Serial.println(WiFi.localIP());
}

// ======================================================
// PÁGINAS EXTRAS
// ======================================================

void enviarPaginaStatus() {
  String status = "";

  status += "<!DOCTYPE html>";
  status += "<html lang='pt-BR'>";
  status += "<head>";
  status += "<meta charset='UTF-8'>";
  status += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  status += "<title>Status PetFeeder</title>";
  status += "<style>";
  status += "body{font-family:Georgia,'Times New Roman',serif;background:#f8f9fa;color:#191c1d;padding:24px;margin:0;}";
  status += ".card{background:white;border-radius:24px;padding:22px;box-shadow:0 12px 30px rgba(0,0,0,.08);max-width:440px;margin:40px auto;}";
  status += "h1{color:#9b4500;margin-top:0;}";
  status += "p,li{font-size:17px;line-height:1.5;}";
  status += ".pill{display:inline-block;background:#9df0f8;color:#006970;border-radius:999px;padding:8px 14px;font-weight:bold;}";
  status += "a{display:inline-block;margin-top:16px;background:#ff8c42;color:#6a2d00;padding:14px 20px;border-radius:999px;text-decoration:none;font-weight:bold;}";
  status += "</style>";
  status += "</head>";
  status += "<body>";
  status += "<div class='card'>";
  status += "<h1>🐾 Status do PetFeeder</h1>";

  status += "<p><span class='pill'>";
  status += alimentando ? "Liberando racao" : "Pronto";
  status += "</span></p>";

  status += "<p><strong>WiFi:</strong> ";
  status += WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado";
  status += "</p>";

  status += "<p><strong>IP:</strong> ";
  status += WiFi.localIP().toString();
  status += "</p>";

  status += "<p><strong>Horario atual:</strong> ";
  status += obterHorarioFormatado();
  status += "</p>";

  status += "<p><strong>Servo:</strong> GPIO ";
  status += String(servoPin);
  status += "</p>";

  status += "<p><strong>Posicao fechada:</strong> ";
  status += String(posFechado);
  status += " graus</p>";

  status += "<p><strong>Posicao aberta:</strong> ";
  status += String(posAberto);
  status += " graus</p>";

  status += "<p><strong>Tempo aberto:</strong> ";
  status += String(tempoAberto);
  status += " segundos</p>";

  status += "<a href='/'>Voltar</a>";
  status += "</div>";
  status += "</body>";
  status += "</html>";

  server.send(200, "text/html", status);
}

void enviarPaginaSchedule() {
  String pagina = "";

  pagina += "<!DOCTYPE html>";
  pagina += "<html lang='pt-BR'>";
  pagina += "<head>";
  pagina += "<meta charset='UTF-8'>";
  pagina += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  pagina += "<title>Schedule PetFeeder</title>";
  pagina += "<style>";
  pagina += "body{font-family:Georgia,'Times New Roman',serif;background:#f8f9fa;color:#191c1d;padding:24px;margin:0;}";
  pagina += ".card{background:white;border-radius:24px;padding:22px;box-shadow:0 12px 30px rgba(0,0,0,.08);max-width:440px;margin:40px auto;}";
  pagina += "h1{color:#9b4500;margin-top:0;}";
  pagina += "p,li{font-size:18px;line-height:1.5;}";
  pagina += "li{background:#f3f4f5;margin:10px 0;padding:14px;border-radius:16px;list-style:none;}";
  pagina += "ul{padding:0;}";
  pagina += "a{display:inline-block;margin-top:16px;background:#ff8c42;color:#6a2d00;padding:14px 20px;border-radius:999px;text-decoration:none;font-weight:bold;}";
  pagina += "</style>";
  pagina += "</head>";
  pagina += "<body>";
  pagina += "<div class='card'>";
  pagina += "<h1>◷ Schedule</h1>";
  pagina += "<p>Horários programados para liberar ração:</p>";
  pagina += "<ul>";

  for (int i = 0; i < totalHorarios; i++) {
    pagina += "<li>";

    if (horarios[i][0] < 10) pagina += "0";
    pagina += String(horarios[i][0]);

    pagina += ":";

    if (horarios[i][1] < 10) pagina += "0";
    pagina += String(horarios[i][1]);

    pagina += "</li>";
  }

  pagina += "</ul>";
  pagina += "<a href='/'>Voltar</a>";
  pagina += "</div>";
  pagina += "</body>";
  pagina += "</html>";

  server.send(200, "text/html", pagina);
}

// ======================================================
// HORÁRIO
// ======================================================

void verificarHorarioProgramado() {
  int hora = timeClient.getHours();
  int minuto = timeClient.getMinutes();

  for (int i = 0; i < totalHorarios; i++) {
    if (hora == horarios[i][0] && minuto == horarios[i][1]) {

      bool jaAcionouNesseHorario =
        hora == ultimaHoraAcionada &&
        minuto == ultimoMinutoAcionado;

      if (!jaAcionouNesseHorario) {
        Serial.println("-----------------------------------");
        Serial.println("Horario programado atingido!");
        mostrarHorarioAtual();

        iniciarLiberacaoRacao();

        ultimaHoraAcionada = hora;
        ultimoMinutoAcionado = minuto;
      }
    }
  }
}

String obterHorarioFormatado() {
  if (WiFi.status() != WL_CONNECTED) {
    return "Sem WiFi / sem NTP";
  }

  int hora = timeClient.getHours();
  int minuto = timeClient.getMinutes();
  int segundo = timeClient.getSeconds();

  String horario = "";

  if (hora < 10) horario += "0";
  horario += String(hora);

  horario += ":";

  if (minuto < 10) horario += "0";
  horario += String(minuto);

  horario += ":";

  if (segundo < 10) horario += "0";
  horario += String(segundo);

  return horario;
}

void mostrarHorarioAtual() {
  Serial.print("Horario atual: ");
  Serial.println(obterHorarioFormatado());
}

void mostrarHorarioPeriodicamente() {
  if (millis() - ultimoPrintHorario >= 5000) {
    ultimoPrintHorario = millis();

    if (WiFi.status() == WL_CONNECTED) {
      mostrarHorarioAtual();
    }
  }
}

// ======================================================
// COMANDOS
// ======================================================

void verificarComandoSerial() {
  if (Serial.available() > 0) {
    char comando = Serial.read();

    if (comando == 'a' || comando == 'A') {
      Serial.println("Comando recebido pelo Monitor Serial.");
      iniciarLiberacaoRacao();
    }

    if (comando == 'h' || comando == 'H') {
      mostrarHorarioAtual();
    }

    if (comando == 's' || comando == 'S') {
      float distancia = medirDistancia();

      Serial.println();
      Serial.println("===== SENSOR =====");
      Serial.print("Distancia: ");
      Serial.print(distancia);
      Serial.println(" cm");

      Serial.print("Percentual: ");
      Serial.print(calcularPercentualRacao(distancia));
      Serial.println("%");

      Serial.print("Nivel baixo: ");
      Serial.println(calcularPercentualRacao(distancia) <= 10 ? "SIM" : "NAO");
      Serial.println("==================");
      Serial.println();
    }
  }
}

// ======================================================
// SERVO / COMPORTA
// ======================================================

void abrirComporta() {
  Serial.print("Servo indo para posicao ABERTA: ");
  Serial.println(posAberto);

  feederServo.write(posAberto);
}

void fecharComporta() {
  Serial.print("Servo indo para posicao FECHADA: ");
  Serial.println(posFechado);

  feederServo.write(posFechado);
}

bool iniciarLiberacaoRacao() {
  if (alimentando) {
    Serial.println("O alimentador ja esta liberando racao. Comando ignorado.");
    return false;
  }

  Serial.println("-----------------------------------");
  Serial.println("Liberando racao...");

  abrirComporta();

  Serial.print("A comporta ficara aberta por ");
  Serial.print(tempoAberto);
  Serial.println(" segundos.");

  alimentando = true;
  inicioAlimentacao = millis();

  return true;
}

void atualizarAlimentacao() {
  if (alimentando) {
    unsigned long tempoDecorrido = millis() - inicioAlimentacao;
    unsigned long tempoTotal = tempoAberto * 1000UL;

    if (tempoDecorrido >= tempoTotal) {
      fecharComporta();

      alimentando = false;

      Serial.println("Comporta fechada.");
      Serial.println("Racao liberada com sucesso.");
      Serial.println("-----------------------------------");
    }
  }
}

// Mantive esta funcao caso voce queira usar em algum teste antigo.
// Agora ela usa o sistema novo, sem delay.
void liberarRacao() {
  iniciarLiberacaoRacao();
}

// ===== SENSOR =====
void configurarSensor() {
  pinMode(sensorPowerPin, OUTPUT);
  digitalWrite(sensorPowerPin, HIGH);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracao = pulseIn(echoPin, HIGH, 30000);
  if (duracao == 0) return -1;
  return duracao * 0.0343 / 2.0;
}

int calcularPercentualRacao(float distancia) {

  if (distancia <= 3.0) return 100;
  if (distancia <= 4.0) return 90;
  if (distancia <= 5.0) return 80;
  if (distancia <= 6.0) return 70;
  if (distancia <= 8.0) return 50;
  if (distancia <= 10.0) return 35;
  if (distancia <= 12.0) return 25;
  if (distancia <= 14.0) return 15;
  if (distancia <= 16.0) return 5;

  return 0;
}

void atualizarNivelRacao() {
  if (millis() - ultimaLeituraSensor < 5000) return;
  ultimaLeituraSensor = millis();

  float leitura = medirDistancia();
  if (leitura < 0) return;

  distanciaAtual = leitura;
  percentualRacao = calcularPercentualRacao(leitura);
  nivelBaixo = percentualRacao <= 10;
}
