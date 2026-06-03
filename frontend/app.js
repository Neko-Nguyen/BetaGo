const SIZE = 9;
const Colors = {
  Empty: 'Empty',
  Black: 'Black',
  White: 'White'
};

let currentColor = Colors.Black;
let board = Array(SIZE * SIZE).fill(Colors.Empty);
let parent = Array(SIZE * SIZE).fill(-1);
let chainColor = Array(SIZE * SIZE).fill(Colors.Empty);
let chainStoneCount = Array(SIZE * SIZE).fill(0);
let chainLibertyCount = Array(SIZE * SIZE).fill(0);

const boardEl = document.getElementById('board');
const blackBtn = document.getElementById('blackBtn');
const whiteBtn = document.getElementById('whiteBtn');
const resetBtn = document.getElementById('resetBtn');
const messageEl = document.getElementById('message');
const selectedEl = document.getElementById('selected');
const statsEl = document.getElementById('stats');

function index(x, y) {
  return y * SIZE + x;
}

function decode(id) {
  return {
    x: id % SIZE,
    y: Math.floor(id / SIZE)
  };
}

function isValid(x, y) {
  return x >= 0 && y >= 0 && x < SIZE && y < SIZE;
}

function neighbors(x, y) {
  return [
    { x: x - 1, y },
    { x: x + 1, y },
    { x, y: y - 1 },
    { x, y: y + 1 }
  ].filter(p => isValid(p.x, p.y));
}

function setMessage(text) {
  messageEl.textContent = text;
}

function setSelected(text) {
  selectedEl.textContent = text;
}

function setStats(text) {
  statsEl.textContent = text;
}

function root(i) {
  if (parent[i] === i) return i;
  parent[i] = root(parent[i]);
  return parent[i];
}

function union(a, b) {
  a = root(a);
  b = root(b);
  if (a === b) return a;
  parent[b] = a;
  return a;
}

function rebuildDSU() {
  parent.fill(-1);
  for (let id = 0; id < board.length; id++) {
    if (board[id] !== Colors.Empty) parent[id] = id;
  }

  for (let id = 0; id < board.length; id++) {
    if (board[id] === Colors.Empty) continue;
    const { x, y } = decode(id);
    for (const n of neighbors(x, y)) {
      const nid = index(n.x, n.y);
      if (board[nid] === board[id]) union(id, nid);
    }
  }
}

function rebuildChains() {
  chainColor.fill(Colors.Empty);
  chainStoneCount.fill(0);
  chainLibertyCount.fill(0);

  const libertySets = Array.from({ length: SIZE * SIZE }, () => new Set());

  for (let id = 0; id < board.length; id++) {
    if (board[id] === Colors.Empty) continue;
    const rootId = root(id);
    chainColor[rootId] = board[id];
    chainStoneCount[rootId] += 1;

    const { x, y } = decode(id);
    for (const n of neighbors(x, y)) {
      const nid = index(n.x, n.y);
      if (board[nid] === Colors.Empty) {
        libertySets[rootId].add(nid);
      }
    }
  }

  for (let id = 0; id < board.length; id++) {
    if (chainColor[id] !== Colors.Empty) {
      chainLibertyCount[id] = libertySets[id].size;
    }
  }
}

function countLibertiesForRoot(rootId) {
  const seen = new Set();
  for (let id = 0; id < board.length; id++) {
    if (board[id] === Colors.Empty) continue;
    if (root(id) !== rootId) continue;
    const { x, y } = decode(id);
    for (const n of neighbors(x, y)) {
      const nid = index(n.x, n.y);
      if (board[nid] === Colors.Empty) seen.add(nid);
    }
  }
  return seen.size;
}

function removeGroup(rootId) {
  for (let id = 0; id < board.length; id++) {
    if (board[id] === Colors.Empty) continue;
    if (root(id) === rootId) board[id] = Colors.Empty;
  }
}

function placeStone(x, y) {
  const id = index(x, y);
  if (board[id] !== Colors.Empty) {
    setMessage('That point is already occupied.');
    return;
  }

  board[id] = currentColor;
  rebuildDSU();

  const opponent = currentColor === Colors.Black ? Colors.White : Colors.Black;
  const capturedRoots = new Set();

  for (const n of neighbors(x, y)) {
    const nid = index(n.x, n.y);
    if (board[nid] !== opponent) continue;
    const rootId = root(nid);
    if (countLibertiesForRoot(rootId) === 0) {
      capturedRoots.add(rootId);
    }
  }

  capturedRoots.forEach(rootId => removeGroup(rootId));
  rebuildDSU();

  const placedRoot = root(id);
  if (placedRoot !== -1 && countLibertiesForRoot(placedRoot) === 0) {
    board[id] = Colors.Empty;
    rebuildDSU();
    rebuildChains();
    render();
    setMessage('Illegal move: suicide is not allowed.');
    return;
  }

  rebuildDSU();
  rebuildChains();
  render();
  setMessage(`Placed ${currentColor} at (${x + 1}, ${y + 1}).`);
}

function selectCell(id) {
  if (board[id] === Colors.Empty) {
    setSelected('Empty point');
    setStats('No group data available.');
    return;
  }

  const rootId = root(id);
  const { x, y } = decode(id);
  setSelected(`Point (${x + 1}, ${y + 1}), ${board[id]}`);
  setStats(
    `Root: ${rootId + 1}\n` +
    `Color: ${chainColor[rootId]}\n` +
    `Stones: ${chainStoneCount[rootId]}\n` +
    `Liberties: ${chainLibertyCount[rootId]}`
  );
}

function render() {
  boardEl.innerHTML = '';
  for (let y = 0; y < SIZE; y++) {
    for (let x = 0; x < SIZE; x++) {
      const id = index(x, y);
      const cell = document.createElement('button');
      cell.type = 'button';
      cell.className = 'cell';
      if (board[id] === Colors.Black) cell.classList.add('black');
      if (board[id] === Colors.White) cell.classList.add('white');
      cell.addEventListener('click', () => {
        placeStone(x, y);
        selectCell(id);
      });
      boardEl.appendChild(cell);
    }
  }
}

function resetBoard() {
  board.fill(Colors.Empty);
  parent.fill(-1);
  chainColor.fill(Colors.Empty);
  chainStoneCount.fill(0);
  chainLibertyCount.fill(0);
  rebuildDSU();
  rebuildChains();
  render();
  setMessage('Board reset. Choose a color and play.');
  setSelected('None');
  setStats('Place a stone to show chain stats.');
}

blackBtn.addEventListener('click', () => {
  currentColor = Colors.Black;
  blackBtn.classList.add('selected');
  whiteBtn.classList.remove('selected');
  setMessage('Black selected.');
});

whiteBtn.addEventListener('click', () => {
  currentColor = Colors.White;
  whiteBtn.classList.add('selected');
  blackBtn.classList.remove('selected');
  setMessage('White selected.');
});

resetBtn.addEventListener('click', resetBoard);

window.addEventListener('load', () => {
  rebuildDSU();
  rebuildChains();
  render();
  setMessage('Ready. Click any cell to place a stone.');
});
