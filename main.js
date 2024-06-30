import * as THREE from 'three';
import { grid, currentTetromino, position, moveTetromino, hasCollision, removeFullLines, rotateTetromino, mergeTetrominoToGrid, resetTetromino, TETROMINOS, gameOver } from './game';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';

const ROWS = 20;
const COLS = 10;

const SCORE_SINGLE = 100;
const SCORE_DOUBLE = 200;
const SCORE_TRIPLE = 400;
const SCORE_QUADRUPLE = 800;
const SCORE_CONSECUTIVE = 1200;
const SCORE_DOWN_PRESS = 1; // Points for each down key press


const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
// Imposta il colore di sfondo della scena
renderer.setClearColor(0x000000);
document.body.appendChild(renderer.domElement);

camera.position.set(0, 0, 17);
camera.lookAt(0, 0, 0);

const controls = new OrbitControls(camera, renderer.domElement);

const light = new THREE.DirectionalLight(0xffffff, 1);
light.position.set(0, 0, 10).normalize();
scene.add(light);

// Funzione per creare un asse
/*
const axesHelper = new THREE.AxesHelper(22);
axesHelper.position.set(-5.6, -9.5, -0.5);
scene.add(axesHelper);*/

// Definisci la risoluzione del piano
const resolution = new THREE.Vector2(30, 30);

// Crea la geometria del piano
const planeGeometry = new THREE.PlaneGeometry(
  resolution.x,
  resolution.y
);
planeGeometry.rotateX(-Math.PI * 0.5);

// Carica la texture
const textureLoader = new THREE.TextureLoader();
const texture = textureLoader.load('./src/tetrisimg.jpeg', function (texture) {
  // Una volta caricata la texture, crea il materiale e applicalo al piano
  const planeMaterial = new THREE.MeshBasicMaterial({ map: texture });
  const plane = new THREE.Mesh(planeGeometry, planeMaterial);
  plane.position.set(0, -9.6, 0); // Imposta la posizione del centro del piano
  scene.add(plane);

  // Crea la geometria del bordo
  const edgesGeometry = new THREE.EdgesGeometry(planeGeometry);
  const edgesMaterial = new THREE.LineBasicMaterial({ color: 0x00BFFF }); // Colore rosso per il bordo
  const edges = new THREE.LineSegments(edgesGeometry, edgesMaterial);
  
  // Aggiusta la posizione del bordo per allinearlo con il piano
  edges.position.set(plane.position.x, plane.position.y, plane.position.z);
  edges.rotation.set(plane.rotation.x, plane.rotation.y, plane.rotation.z);
  
  scene.add(edges);
});


// Gruppo per la griglia e i tetramini
const tetrominoGroup = new THREE.Group();
scene.add(tetrominoGroup);

Object.assign(OrbitControls.prototype, {
    rotateLeft: function (angle) {
        if (angle === undefined) {
            angle = this.getAutoRotationAngle();
        }
        this.thetaDelta -= angle;
    },
    rotateRight: function (angle) {
        if (angle === undefined) {
            angle = this.getAutoRotationAngle();
        }
        this.thetaDelta += angle;
    }
});

let gameInterval;
let isGameRunning = false;

function render() {
    renderer.render(scene, camera);
    requestAnimationFrame(render);
    controls.update();
}

function clearGridGroup() {
    while (tetrominoGroup.children.length > 0) {
        tetrominoGroup.remove(tetrominoGroup.children[0]);
    }
}

function drawGrid() {
    clearGridGroup();
    scene.add(light);
    drawTetrominoOnGrid();
    drawGridLines();
    drawCurrentTetromino();

    // Update score element in HTML
    const scoreElement = document.getElementById('score');
    scoreElement.textContent = score;

    const levelElement = document.getElementById('level');
    levelElement.textContent = level;

}

function drawGridLines() {
    const lineMaterial = new THREE.LineBasicMaterial({ color: 0xffffff });
    const lineGeometry = new THREE.BufferGeometry();
    const vertices = [];

    const halfCols = COLS / 2;
    const halfRows = ROWS / 2;
    const depth = 1;

    const offsetX = -0.5;
    const offsetY = 0.5;
    const offsetZ = -0.5;

    for (let y = 0; y <= ROWS; y++) {
        if (y == 0 || y == ROWS){
        vertices.push(-halfCols + offsetX, halfRows - y + offsetY, offsetZ);
        vertices.push(halfCols + offsetX, halfRows - y + offsetY, offsetZ);

        vertices.push(-halfCols + offsetX, halfRows - y + offsetY, depth + offsetZ);
        vertices.push(halfCols + offsetX, halfRows - y + offsetY, depth + offsetZ);
        }
    }

    for (let x = 0; x <= COLS; x++) {
        if (x == 0 || x == COLS){
        vertices.push(-halfCols + x + offsetX, halfRows + offsetY, offsetZ);
        vertices.push(-halfCols + x + offsetX, -halfRows + offsetY, offsetZ);

        vertices.push(-halfCols + x + offsetX, halfRows + offsetY, depth + offsetZ);
        vertices.push(-halfCols + x + offsetX, -halfRows + offsetY, depth + offsetZ);
        }
    
    }

    for (let y = 0; y <= ROWS; y++) {
        for (let x = 0; x <= COLS; x++) {
            if (x == 0 && y == 0 || x == COLS && y == 0 || x == 0 && y == ROWS || x == COLS && y == ROWS){
            vertices.push(-halfCols + x + offsetX, halfRows - y + offsetY, offsetZ);
            vertices.push(-halfCols + x + offsetX, halfRows - y + offsetY, depth + offsetZ);
            }
        }
    }

    lineGeometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
    const gridLines = new THREE.LineSegments(lineGeometry, lineMaterial);
    tetrominoGroup.add(gridLines);
}

function drawTetrominoOnGrid() {
    for (let y = 0; y < ROWS; y++) {
        for (let x = 0; x < COLS; x++) {
            if (grid[y][x] !== 0) {
                const geometry = new THREE.BoxGeometry(1, 1, 1);
                const material = new THREE.MeshBasicMaterial({ color: grid[y][x] });
                const cube = new THREE.Mesh(geometry, material);
                cube.position.set(x - COLS / 2, ROWS / 2 - y, 0);
                tetrominoGroup.add(cube);
            }
        }
    }
}

function drawCurrentTetromino() {
    if (!currentTetromino || !currentTetromino.shape) return;

    const currentGroup = new THREE.Group();

    for (let y = 0; y < currentTetromino.shape.length; y++) {
        for (let x = 0; x < currentTetromino.shape[y].length; x++) {
            if (currentTetromino.shape[y][x] !== 0) {
                const geometry = new THREE.BoxGeometry(1, 1, 1);
                const material = new THREE.MeshBasicMaterial({ color: currentTetromino.color });
                const cube = new THREE.Mesh(geometry, material);
                cube.position.set(position.x + x - COLS / 2, ROWS / 2 - (position.y + y), 0);
                currentGroup.add(cube);
            }
        }
    }

    tetrominoGroup.add(currentGroup);
    currentTetromino.group = currentGroup;
}

// Esempio di variabile globale per il punteggio
let score = 0;
let level = 1;
let cont = 0;

function update() {
    if (!isGameRunning) return;

    const moved = moveTetromino(0, 1);

    if (!moved) {
        mergeTetrominoToGrid();
        const linesRemoved = removeFullLines();

        switch (linesRemoved) {
            case 1:
                score += SCORE_SINGLE;
                break;
            case 2:
                score += SCORE_DOUBLE;
                break;
            case 3:
                score += SCORE_TRIPLE;
                break;
            case 4:
                score += SCORE_QUADRUPLE;
                break;
        }

        // Additional score for consecutive removals
        if (linesRemoved > 4) {
            score += (linesRemoved) * SCORE_CONSECUTIVE + SCORE_QUADRUPLE;
        }

        cont += linesRemoved;

        if(cont >= 10){
            level += 1;
            cont = 0;
        }
        console.log('Score:', score); // Display or use score as needed

        resetTetromino();
    }

    drawGrid();
    gameInterval = setTimeout(update, 1000);
}


function handleKeyPress(event) {
    if (!isGameRunning) return;

    switch (event.key) {
        case 'ArrowLeft':
            moveTetromino(-1, 0);
            break;
        case 'ArrowRight':
            moveTetromino(1, 0);
            break;
        case 'ArrowDown':
            moveTetromino(0, 1);
            // Add score for each down key press
            score += SCORE_DOWN_PRESS;
            break;
        case 'ArrowUp':
            rotateTetromino();
            break;
    }
    drawGrid();
}

document.addEventListener('keydown', handleKeyPress);

document.getElementById('start-button').addEventListener('click', () => {
    if (!isGameRunning) {
        isGameRunning = true;
        update();
    }
});

document.getElementById('stop-button').addEventListener('click', () => {
    isGameRunning = false;
    clearTimeout(gameInterval);
});

document.getElementById('reset-button').addEventListener('click', () => {
    console.log("Reset button clicked");
    reset();
});

const result = gameOver();
if (result === true) {
    console.log('La funzione checkCondition ha ritornato true.');
    reset();
} else {
    console.log('La funzione checkCondition non ha ritornato true.');
}

function reset(){
    isGameRunning = false;
    score = 0;
    level = 1;
    clearTimeout(gameInterval);
    resetGrid();
    resetTetromino();
    drawGrid();
}

function resetGrid() {
    for (let y = 0; y < ROWS; y++) {
        for (let x = 0; x < COLS; x++) {
            grid[y][x] = 0;
        }
    }
}

drawGrid();
render();
