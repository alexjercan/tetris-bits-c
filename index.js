const fps = 5;

const BLOCK_SIZE = 50;
const BUFFER = 4;
const WIDTH = 8;
const HEIGHT = 12;

let div = document.getElementById("game");
let canvas = document.createElement("canvas");
canvas.width = WIDTH * BLOCK_SIZE;
canvas.height = HEIGHT * BLOCK_SIZE;
div.appendChild(canvas);

const MOVE_LEFT = 0;
const MOVE_RIGHT = 1;
const MOVE_NONE = 2;

let input = MOVE_NONE;

document.addEventListener('keydown', (event) => {
    if (event.key === "ArrowLeft" || event.key === "a") {
        input = MOVE_LEFT;
    } else if (event.key === "ArrowRight" || event.key === "d") {
        input = MOVE_RIGHT;
    } else {
        input = MOVE_NONE;
    }
});

let isGameOver = false;

function decodeTetrisBoard(view, tetrisPointer) {
    let score = 0; // 4 bytes
    let board = []; // 16 bytes

    for (let i = 0; i < BUFFER; i++) {
        score += view[tetrisPointer + i];
    }

    for (let i = 0; i < HEIGHT + BUFFER; i++) {
        board.push(view[tetrisPointer + 4 + i]);
    }

    return { score, board };
}

function render(tetris) {
    const ctx = canvas.getContext('2d');

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    for (let i = BUFFER; i < HEIGHT + BUFFER; i++) {
        let line = tetris.board[i];
        for (let j = WIDTH - 1; j >= 0; j--) {
            if (line & (1 << j)) {
                ctx.fillStyle = "black";
                ctx.fillRect((WIDTH - 1 - j) * BLOCK_SIZE, (i - 4) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
            }
        }
    }
}

function arraysEqual(a, b) {
    if (a === b) return true;
    if (a == null || b == null) return false;
    if (a.length !== b.length) return false;

    for (let i = 0; i < a.length; ++i) {
        if (a[i] !== b[i]) return false;
    }

    return true;
}

WebAssembly.instantiateStreaming(fetch("tetris.wasm"), {
    env: {
        memory: new WebAssembly.Memory({ initial: 8 }),
        tetris_rand_int: (max) => {
            return Math.floor(Math.random() * max);
        },
    }
}).then(obj => {
    let fpsInterval = 1000 / fps;
    let then = Date.now();

    let instance = obj.instance;
    let memory = instance.exports.memory;

    const view = new Uint8Array(memory.buffer);
    const tetrisPointer = instance.exports.__heap_base;

    instance.exports.tetris_init(tetrisPointer);

    function gameLoop() {
        requestAnimationFrame(gameLoop);

        let now = Date.now();
        let elapsed = now - then;

        if (elapsed > fpsInterval) {
            then = now - (elapsed % fpsInterval);

            let tmp = decodeTetrisBoard(view, tetrisPointer);

            instance.exports.tetris_tick(tetrisPointer, input);
            input = MOVE_NONE;

            let tetris = decodeTetrisBoard(view, tetrisPointer);

            if (arraysEqual(tetris.board, tmp.board)) {
                isGameOver = instance.exports.tetris_spawn(tetrisPointer);
            }

            render(tetris);
            if (isGameOver) {
                console.log(`Game over! Score: ${tetris.score}`);
                instance.exports.tetris_init(tetrisPointer);
            }
        }
    }

    requestAnimationFrame(gameLoop);
}).catch(e => {
    console.error("Error instantiating the WebAssembly module", e);
});

