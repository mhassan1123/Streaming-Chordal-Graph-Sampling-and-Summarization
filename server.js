// Zero-dependency Node server (Node 18+).
//   browser  --HTTP-->  server.js  --stdin/stdout-->  chordal_api (C++)
// Run:  node server/server.js     then open http://localhost:3000
const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const PORT = Number(process.env.PORT) || 3000;
const ROOT = __dirname;
const WEB = __dirname;
const EXE = process.platform === 'win32' ? '.exe' : '';

const LIMITS = { edges: 5000, iterations: 5000, nodeId: 100000, topK: 25, maxSnapshots: 200, timeoutMs: 30000, bodyBytes: 1_000_000 };

const candidates = [
  process.env.ENGINE_PATH,
 ...['.', 'build/bin', 'build'].map(d => path.join(ROOT, d, 'chordal_api' + EXE)), 
].filter(Boolean);
const findEngine = () => candidates.find(p => fs.existsSync(p));

const MIME = { '.html': 'text/html; charset=utf-8', '.css': 'text/css', '.js': 'text/javascript', '.svg': 'image/svg+xml', '.json': 'application/json' };

function send(res, status, body, type = 'application/json') {
  res.writeHead(status, { 'Content-Type': type, 'Cache-Control': 'no-store' });
  res.end(typeof body === 'string' || Buffer.isBuffer(body) ? body : JSON.stringify(body));
}

function readBody(req) {
  return new Promise((resolve, reject) => {
    let size = 0; const chunks = [];
    req.on('data', c => { size += c.length; if (size > LIMITS.bodyBytes) { reject(new Error('Request body is too large (limit 1 MB).')); req.destroy(); } else chunks.push(c); });
    req.on('end', () => resolve(Buffer.concat(chunks).toString('utf8')));
    req.on('error', reject);
  });
}

function parseEdges(text) {
  const edges = [];
  for (const [i, raw] of String(text || '').split(/\r?\n/).entries()) {
    const line = raw.trim();
    if (!line || line.startsWith('#')) continue;
    const parts = line.split(/[,;\s]+/);
    if (parts.length < 2 || !/^\d+$/.test(parts[0]) || !/^\d+$/.test(parts[1]))
      throw new Error(`Line ${i + 1} is not an edge ("${line.slice(0, 30)}"). Use one "u,v" pair of non-negative integers per line.`);
    const u = +parts[0], v = +parts[1];
    if (u > LIMITS.nodeId || v > LIMITS.nodeId) throw new Error(`Node ids must be at most ${LIMITS.nodeId}.`);
    edges.push([u, v]);
  }
  if (!edges.length) throw new Error('No edges found. Paste at least one "u,v" pair.');
  if (edges.length > LIMITS.edges) throw new Error(`Too many edges (${edges.length}). The limit is ${LIMITS.edges}.`);
  return edges;
}

const clamp = (x, lo, hi, d) => Number.isFinite(+x) ? Math.min(hi, Math.max(lo, Math.floor(+x))) : d;

function runEngine(exe, input) {
  return new Promise((resolve, reject) => {
    const child = spawn(exe, [], { stdio: ['pipe', 'pipe', 'pipe'] });
    let out = '', err = '';
    const timer = setTimeout(() => { child.kill(); reject(new Error('The C++ engine timed out. Try fewer iterations or edges.')); }, LIMITS.timeoutMs);
    child.stdout.on('data', d => out += d);
    child.stderr.on('data', d => err += d);
    child.on('error', e => { clearTimeout(timer); reject(e); });
    child.on('close', code => {
      clearTimeout(timer);
      if (code !== 0) return reject(new Error(`Engine exited with code ${code}. ${err.slice(0, 200)}`));
      try { resolve(JSON.parse(out)); } catch { reject(new Error('Engine returned invalid JSON.')); }
    });
    child.stdin.end(input);
  });
}

async function handleAnalyze(req, res) {
  const exe = findEngine();
  if (!exe) return send(res, 503, { error: 'C++ engine not found. Build it first (see README) or set ENGINE_PATH.' });
  let p;
  try { p = JSON.parse(await readBody(req) || '{}'); } catch (e) { return send(res, 400, { error: e.message || 'Invalid JSON body.' }); }
  try {
    const edges = parseEdges(p.edges);
    const iterations = clamp(p.iterations, 0, LIMITS.iterations, 200);
    const logInterval = Math.max(clamp(p.logInterval, 1, LIMITS.iterations, 10), Math.ceil(iterations / LIMITS.maxSnapshots) || 1);
    const topK = clamp(p.topK, 1, LIMITS.topK, 5);
    const seed = clamp(p.seed, 0, 2 ** 31 - 1, 1);
    const input = `CONFIG ${seed} ${iterations} ${logInterval} ${topK}\n` + edges.map(e => e.join(',')).join('\n') + '\n';
    const t0 = process.hrtime.bigint();
    const result = await runEngine(exe, input);
    result.meta = { engineMs: Number(process.hrtime.bigint() - t0) / 1e6, inputEdges: edges.length };
    send(res, 200, result);
  } catch (e) {
    send(res, 400, { error: e.message });
  }
}

function serveStatic(req, res) {
  const url = decodeURIComponent(new URL(req.url, 'http://x').pathname);
  const file = path.normalize(path.join(WEB, url === '/' ? 'index.html' : url));
  const allowed = ['index.html', 'styles.css', 'app.js'];
  if (path.dirname(file) !== path.normalize(WEB) || !allowed.includes(path.basename(file))) return send(res, 404, 'Not found', 'text/plain');
  fs.readFile(file, (err, data) => err ? send(res, 404, 'Not found', 'text/plain') : send(res, 200, data, MIME[path.extname(file)] || 'application/octet-stream'));
}

http.createServer((req, res) => {
  if (req.method === 'GET' && req.url === '/api/health') return send(res, 200, { ok: true, engine: !!findEngine(), limits: LIMITS });
  if (req.method === 'POST' && req.url === '/api/analyze') return handleAnalyze(req, res);
  if (req.method === 'GET') return serveStatic(req, res);
  send(res, 405, { error: 'Method not allowed' });
}).listen(PORT, () => {
  console.log(`Chordal Graph Explorer: http://localhost:${PORT}`);
  console.log(findEngine() ? `Engine: ${findEngine()}` : 'Engine not found yet. Build chordal_api first (see README).');
});
