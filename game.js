const ROWS = 20;
const COLS = 10;

let grid = Array.from({ length: ROWS }, () => Array(COLS).fill(0));

const TETROMINOS = {
    I: { shape: [[0, 0, 0, 0], [1, 1, 1, 1], [0, 0, 0, 0], [0, 0, 0, 0]], color: 0x00ffff, center: [1, 2] },
    J: { shape: [[1, 0, 0], [1, 1, 1], [0, 0, 0]], color: 0x0000ff, center: [1, 1] },
    L: { shape: [[0, 0, 1], [1, 1, 1], [0, 0, 0]], color: 0xffa500, center: [1, 1] },
    O: { shape: [[1, 1], [1, 1]], color: 0xffff00, center: [0.5, 0.5] },
    S: { shape: [[0, 1, 1], [1, 1, 0], [0, 0, 0]], color: 0x00ff00, center: [1, 1] },
    T: { shape: [[0, 1, 0], [1, 1, 1], [0, 0, 0]], color: 0x800080, center: [1, 1] },
    Z: { shape: [[1, 1, 0], [0, 1, 1], [0, 0, 0]], color: 0xff0000, center: [1, 1] },
};

function getRandomTetromino() {
    const tetrominos = 'IJLOSTZ';
    const randTetromino = tetrominos[Math.floor(Math.random() * tetrominos.length)];
    return createTetromino(randTetromino);
}

function createTetromino(type) {
    return { ...TETROMINOS[type], type };
}

let currentTetromino = getRandomTetromino();
let position = { x: 3, y: 0 };

function moveTetromino(dx, dy) {
    const newPos = { x: position.x + dx, y: position.y + dy };
    if (!hasCollision(newPos)) {
        position = newPos;
        return true;
    } else {
        console.log('Collision detected, not moving');
        return false;
    }
}

function hasCollision(newPos = position, shape = currentTetromino.shape) {
    if (!currentTetromino || !shape) return true;

    for (let y = 0; y < shape.length; y++) {
        for (let x = 0; x < shape[y].length; x++) {
            if (shape[y][x] !== 0) {
                if (newPos.y + y >= ROWS) {
                    console.log('Collision with bottom of the grid');
                    return true;
                }
                if (newPos.x + x >= COLS) {
                    console.log('Collision with right edge of the grid');
                    return true;
                }
                if (newPos.x + x < 0) {
                    console.log('Collision with left edge of the grid');
                    return true;
                }
                if (newPos.y + y < 0) { // Controllo per game over
                    console.log('Game Over');
                    gameOver();
                    return true;
                }
                if (grid[newPos.y + y] && grid[newPos.y + y][newPos.x + x] !== 0) {
                    console.log('Collision with existing block');
                    return true;
                }
            }
        }
    }
    return false;
}

function removeFullLines() {
    let linesRemoved = 0;

    for (let y = ROWS - 1; y >= 0; y--) {
        if (grid[y].every(cell => cell !== 0)) {
            // Rimuovi la riga dalla griglia
            grid.splice(y, 1);
            linesRemoved++;
            // Aggiungi una nuova riga vuota all'inizio della griglia
            grid.unshift(Array(COLS).fill(0));
            // Dopo aver rimosso una riga e aggiunto una nuova riga vuota,
            // incrementa y per controllare nuovamente la stessa posizione
            y++;
        }
    }

    return linesRemoved; // Ritorna il numero di righe rimosse
}

function rotateTetromino() {
    // Salva la posizione e la forma corrente
    const originalPosition = { ...position };
    const originalShape = currentTetromino.shape;

    // Ruota la matrice della forma del tetromino
    const rotatedShape = rotateMatrix(originalShape);

    // Calcola il centro geometrico del Tetromino
    const centerX = Math.floor(originalShape[0].length / 2);
    const centerY = Math.floor(originalShape.length / 2);

    // Calcola la differenza tra la posizione originale e la nuova posizione
    const deltaY = centerY - Math.floor(rotatedShape.length / 2);
    const deltaX = centerX - Math.floor(rotatedShape[0].length / 2);

    // Applica la nuova posizione considerando la differenza
    position.y += deltaY;
    position.x += deltaX;

    // Verifica se la rotazione è valida
    if (!hasCollision(originalPosition, rotatedShape)) {
        currentTetromino.shape = rotatedShape;
    } else {
        // Ripristina la posizione originale se la rotazione non è valida
        position = { ...originalPosition };
        console.log('Rotation is invalid');

        // Check if collision is with the left or right edge
        if (isEdgeCollision(originalPosition, rotatedShape)) {
            console.log('Game Over due to edge collision during rotation');
            alert('Game Over');
            resetGrid();
        }
    }
}

function isEdgeCollision(pos, shape) {
    for (let y = 0; y < shape.length; y++) {
        for (let x = 0; x < shape[y].length; x++) {
            if (shape[y][x] !== 0) {
                if (pos.x + x < 0 || pos.x + x >= COLS) {
                    return true;
                }
            }
        }
    }
    return false;
}

function rotateMatrix(matrix) {
    const rows = matrix.length;
    const cols = matrix[0].length;
    const rotatedMatrix = [];

    for (let col = 0; col < cols; col++) {
        const newRow = [];
        for (let row = rows - 1; row >= 0; row--) {
            newRow.push(matrix[row][col]);
        }
        rotatedMatrix.push(newRow);
    }

    return rotatedMatrix;
}

function mergeTetrominoToGrid() {
    for (let y = 0; y < currentTetromino.shape.length; y++) {
        for (let x = 0; x < currentTetromino.shape[y].length; x++) {
            if (currentTetromino.shape[y][x] !== 0) {
                const newY = position.y + y;
                const newX = position.x + x;
                if (newY >= 0 && newY < ROWS && newX >= 0 && newX < COLS) {
                    grid[newY][newX] = currentTetromino.color;
                }
                // Check if merging with the top of the grid
                if (newY < 0) {
                    console.log('Game Over: Collision with the top of the grid');
                    alert('Game Over');
                    resetTetromino();
                    return;
                }
            }
        }
    }
    console.log('Merged current tetromino into the grid', grid);
}

function resetTetromino() {
    currentTetromino = getRandomTetromino();
    position = { x: Math.floor(COLS / 2) - Math.floor(currentTetromino.shape[0].length / 2), y: 0 };
    console.log('Resetting tetromino to:', currentTetromino, 'at position:', position);
    if (hasCollision()) {
        console.log('Game Over');
        //alert('Game Over');
        
        resetGrid();
    }
}

function resetGrid() {
    console.log('Resetting grid...');
    grid = Array.from({ length: ROWS }, () => Array(COLS).fill(0));
    gameOver();
    position = { x: 3, y: 0 };
    gameOver();
}

// Funzione per gestire il game over
function gameOver() {
    // Ferma il gioco o gestisci la fine del gioco
    // Esempio: disabilita il movimento, il timer, etc.
    // Esempio: mostra un alert di game over
    alert("Game Over!");
    return true;
}

export {
    grid,
    currentTetromino,
    position,
    moveTetromino,
    hasCollision,
    removeFullLines,
    rotateTetromino,
    mergeTetrominoToGrid,
    resetTetromino,
    TETROMINOS,
    gameOver
};

