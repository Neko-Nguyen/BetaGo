const canvas = document.querySelector("#board");
const ctx = canvas.getContext("2d");
const sizeSelect = document.querySelector("#sizeSelect");
const statusEl = document.querySelector("#status");
const whiteCountEl = document.querySelector("#whiteCount");
const blackCountEl = document.querySelector("#blackCount");
const chainInfoEl = document.querySelector("#chainInfo");
const historyEl = document.querySelector("#history");

const EMPTY = "Empty";
const WHITE = "White";
const BLACK = "Black";

let size = 9;
let board = [];
let rootMap = [];
let chains = {};
let turn = WHITE;
let history = [];
let hoverPoint = null;

function newState(nextSize = size) {
  size = nextSize;
  board = Array(size * size).fill(EMPTY);
  rootMap = Array(size * size).fill(-1);
  chains = {};
  turn = WHITE;
  history = [];
  hoverPoint = null;
  render();
}

function idx(x, y) {
  return x * size + y;
}

function pointFromEvent(event) {
  const rect = canvas.getBoundingClientRect();
  const scale = canvas.width / rect.width;
  const x = (event.clientX - rect.left) * scale;
  const y = (event.clientY - rect.top) * scale;
  const g = geometry();
  const col = Math.round((x - g.pad) / g.step);
  const row = Math.round((y - g.pad) / g.step);
  const radius = Math.max(12, g.step * 0.42);
  const cx = g.pad + col * g.step;
  const cy = g.pad + row * g.step;
  if (row < 0 || col < 0 || row >= size || col >= size) return null;
  if (Math.hypot(x - cx, y - cy) > radius) return null;
  return { x: row, y: col };
}

function neighbors(x, y) {
  return [
    [x, y - 1],
    [x - 1, y],
    [x + 1, y],
    [x, y + 1],
  ].filter(([r, c]) => r >= 0 && c >= 0 && r < size && c < size);
}

function rebuildChains() {
  rootMap = Array(size * size).fill(-1);
  chains = {};
  const seen = Array(size * size).fill(false);

  for (let i = 0; i < board.length; i += 1) {
    if (board[i] === EMPTY || seen[i]) continue;

    const color = board[i];
    const stack = [i];
    const stones = [];
    const liberties = new Set();
    seen[i] = true;

    while (stack.length) {
      const current = stack.pop();
      stones.push(current);
      const x = Math.floor(current / size);
      const y = current % size;

      for (const [nx, ny] of neighbors(x, y)) {
        const next = idx(nx, ny);
        if (board[next] === EMPTY) {
          liberties.add(next);
        } else if (board[next] === color && !seen[next]) {
          seen[next] = true;
          stack.push(next);
        }
      }
    }

    const root = stones[0];
    stones.forEach((stone) => {
      rootMap[stone] = root;
    });
    chains[root] = { color, stones: stones.length, liberties: liberties.size };
  }
}

function removeCapturedAround(x, y, color) {
  const opposite = color === WHITE ? BLACK : WHITE;
  const roots = new Set();

  for (const [nx, ny] of neighbors(x, y)) {
    const id = idx(nx, ny);
    if (board[id] === opposite) roots.add(rootMap[id]);
  }

  for (const root of roots) {
    if (chains[root]?.liberties === 0) {
      for (let i = 0; i < rootMap.length; i += 1) {
        if (rootMap[i] === root) board[i] = EMPTY;
      }
    }
  }
}

function playMove(x, y, color = turn) {
  const id = idx(x, y);
  if (board[id] !== EMPTY) return false;

  board[id] = color;
  rebuildChains();
  removeCapturedAround(x, y, color);
  rebuildChains();

  const root = rootMap[id];
  if (root >= 0 && chains[root]?.liberties === 0) {
    board[id] = EMPTY;
    rebuildChains();
    return false;
  }

  history.push({ x: x + 1, y: y + 1, color });
  turn = color === WHITE ? BLACK : WHITE;
  render();
  return true;
}

function randomMove() {
  const empty = [];
  for (let i = 0; i < board.length; i += 1) {
    if (board[i] === EMPTY) empty.push(i);
  }

  while (empty.length) {
    const pick = Math.floor(Math.random() * empty.length);
    const id = empty.splice(pick, 1)[0];
    if (playMove(Math.floor(id / size), id % size)) return;
  }
}

function simulate() {
  let guard = size * size * 2;
  while (board.includes(EMPTY) && guard > 0) {
    randomMove();
    guard -= 1;
  }
}

function geometry() {
  const pad = canvas.width * 0.07;
  const step = (canvas.width - pad * 2) / (size - 1);
  return { pad, step };
}

function drawBoard() {
  const { pad, step } = geometry();
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  const gradient = ctx.createLinearGradient(0, 0, canvas.width, canvas.height);
  gradient.addColorStop(0, "#e3b86e");
  gradient.addColorStop(1, "#c98d45");
  ctx.fillStyle = gradient;
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  ctx.strokeStyle = "#4b3319";
  ctx.lineWidth = 1.5;
  for (let i = 0; i < size; i += 1) {
    const p = pad + i * step;
    ctx.beginPath();
    ctx.moveTo(pad, p);
    ctx.lineTo(canvas.width - pad, p);
    ctx.moveTo(p, pad);
    ctx.lineTo(p, canvas.height - pad);
    ctx.stroke();
  }

  const hoshi = size === 19 ? [3, 9, 15] : size === 13 ? [3, 6, 9] : [2, 4, 6];
  ctx.fillStyle = "#4b3319";
  for (const r of hoshi) {
    for (const c of hoshi) {
      ctx.beginPath();
      ctx.arc(pad + c * step, pad + r * step, 4, 0, Math.PI * 2);
      ctx.fill();
    }
  }
}

function drawStone(x, y, color, active) {
  const { pad, step } = geometry();
  const cx = pad + y * step;
  const cy = pad + x * step;
  const radius = step * 0.43;

  if (active) {
    ctx.beginPath();
    ctx.arc(cx, cy, radius + 6, 0, Math.PI * 2);
    ctx.fillStyle = "rgba(47, 125, 109, 0.24)";
    ctx.fill();
  }

  const gradient = ctx.createRadialGradient(cx - radius * 0.35, cy - radius * 0.45, 2, cx, cy, radius);
  if (color === WHITE) {
    gradient.addColorStop(0, "#ffffff");
    gradient.addColorStop(1, "#d7d3c7");
  } else {
    gradient.addColorStop(0, "#555");
    gradient.addColorStop(1, "#080808");
  }

  ctx.beginPath();
  ctx.arc(cx, cy, radius, 0, Math.PI * 2);
  ctx.fillStyle = gradient;
  ctx.fill();
  ctx.strokeStyle = color === WHITE ? "#8a877e" : "#000";
  ctx.lineWidth = 1.5;
  ctx.stroke();
}

function updatePanel() {
  const white = board.filter((cell) => cell === WHITE).length;
  const black = board.filter((cell) => cell === BLACK).length;
  whiteCountEl.textContent = white;
  blackCountEl.textContent = black;
  statusEl.textContent = board.includes(EMPTY) ? `${turn} to move` : "Board full";

  historyEl.innerHTML = "";
  history.slice(-80).forEach((move) => {
    const li = document.createElement("li");
    li.textContent = `${move.color} (${move.x}, ${move.y})`;
    historyEl.appendChild(li);
  });

  if (!hoverPoint) {
    chainInfoEl.textContent = "Hover a stone";
    return;
  }

  const id = idx(hoverPoint.x, hoverPoint.y);
  const root = rootMap[id];
  const chain = chains[root];
  chainInfoEl.textContent = chain
    ? `${chain.color}: ${chain.stones} stones, ${chain.liberties} liberties`
    : "Empty point";
}

function render() {
  drawBoard();
  const activeRoot = hoverPoint ? rootMap[idx(hoverPoint.x, hoverPoint.y)] : -1;
  for (let i = 0; i < board.length; i += 1) {
    if (board[i] === EMPTY) continue;
    drawStone(Math.floor(i / size), i % size, board[i], rootMap[i] === activeRoot);
  }
  updatePanel();
}

canvas.addEventListener("click", (event) => {
  const point = pointFromEvent(event);
  if (point) playMove(point.x, point.y);
});

canvas.addEventListener("mousemove", (event) => {
  hoverPoint = pointFromEvent(event);
  render();
});

canvas.addEventListener("mouseleave", () => {
  hoverPoint = null;
  render();
});

sizeSelect.addEventListener("change", () => newState(Number(sizeSelect.value)));
document.querySelector("#reset").addEventListener("click", () => newState(size));
document.querySelector("#aiMove").addEventListener("click", randomMove);
document.querySelector("#simulate").addEventListener("click", simulate);

newState(9);
