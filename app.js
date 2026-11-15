'use strict';
const $ = (s, r = document) => r.querySelector(s);
const PAL = ['#0F6B7D', '#B7791F', '#7A4E9C', '#2F7D4F', '#B5483A', '#3F5FA8', '#8A6D3B', '#4B8B8B'];
const EXAMPLE = '1,2\n2,3\n3,1\n3,4\n4,5\n5,3\n5,6\n6,7\n7,5\n7,8\n8,9\n9,7\n2,6\n4,8\n1,9\n3,7';
const f = (x, d = 3) => Number(x).toFixed(d);
const esc = s => String(s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));

/* ---------- layouts ---------- */
function forceLayout(ids, edges, ticks = 320) {
  const W = 1000, H = 600, n = ids.length, idx = new Map(ids.map((id, i) => [id, i]));
  const x = new Float64Array(n), y = new Float64Array(n);
  ids.forEach((_, i) => { const a = (i / n) * 6.2832, r = 220 + ((i * 7) % 5) * 12; x[i] = W / 2 + r * Math.cos(a); y[i] = H / 2 + r * 0.6 * Math.sin(a); });
  const E = edges.map(([u, v]) => [idx.get(u), idx.get(v)]).filter(e => e[0] !== undefined && e[1] !== undefined);
  const k = Math.sqrt((W * H) / Math.max(n, 1)) * 0.75;
  for (let t = 0; t < ticks; t++) {
    const temp = (1 - t / ticks) * k * 0.6 + 0.4, dx = new Float64Array(n), dy = new Float64Array(n);
    for (let i = 0; i < n; i++) for (let j = i + 1; j < n; j++) {
      const ex = x[i] - x[j], ey = y[i] - y[j], fr = (k * k) / (ex * ex + ey * ey + 0.01);
      dx[i] += ex * fr; dy[i] += ey * fr; dx[j] -= ex * fr; dy[j] -= ey * fr;
    }
    for (const [a, c] of E) {
      const ex = x[a] - x[c], ey = y[a] - y[c], fa = Math.sqrt(ex * ex + ey * ey) / k;
      dx[a] -= ex * fa; dy[a] -= ey * fa; dx[c] += ex * fa; dy[c] += ey * fa;
    }
    for (let i = 0; i < n; i++) {
      dx[i] += (W / 2 - x[i]) * 0.03; dy[i] += (H / 2 - y[i]) * 0.03;
      const d = Math.hypot(dx[i], dy[i]) || 1, m = Math.min(d, temp);
      x[i] += (dx[i] / d) * m; y[i] += (dy[i] / d) * m;
    }
  }
  const minX = Math.min(...x), maxX = Math.max(...x), minY = Math.min(...y), maxY = Math.max(...y);
  const pos = new Map();
  ids.forEach((id, i) => pos.set(id, [0.06 + 0.88 * (x[i] - minX) / ((maxX - minX) || 1), 0.08 + 0.84 * (y[i] - minY) / ((maxY - minY) || 1)]));
  return pos;
}
function circleLayout(ids) {
  const pos = new Map();
  ids.forEach((id, i) => { const a = (i / ids.length) * 6.2832 - 1.5708; pos.set(id, [0.5 + 0.36 * Math.cos(a), 0.5 + 0.38 * Math.sin(a)]); });
  return pos;
}

/* ---------- canvas graph ---------- */
class GraphCanvas {
  constructor(wrap, h, dark = false) {
    this.h = h; this.dark = dark;
    wrap.innerHTML = '<canvas role="img"></canvas><div class="tip"></div>';
    this.cv = $('canvas', wrap); this.tip = $('.tip', wrap); this.wrap = wrap;
    this.data = { nodes: [], edges: [], pos: new Map() }; this.hover = null; this.hit = [];
    this.cv.addEventListener('mousemove', e => this.onMove(e));
    this.cv.addEventListener('mouseleave', () => { this.hover = null; this.tip.style.display = 'none'; this.draw(); });
    new ResizeObserver(() => this.draw()).observe(wrap);
  }
  set(data, label) { this.data = data; this.cv.setAttribute('aria-label', label || 'Graph'); this.draw(); }
  draw() {
    const r = this.cv.getBoundingClientRect(), dpr = window.devicePixelRatio || 1, W = r.width, H = this.h;
    if (!W) return;
    this.cv.width = W * dpr; this.cv.height = H * dpr; this.cv.style.height = H + 'px';
    const c = this.cv.getContext('2d'); c.setTransform(dpr, 0, 0, dpr, 0, 0); c.clearRect(0, 0, W, H);
    const { nodes, edges, pos, top = new Set(), labelsAll } = this.data, n = nodes.length, dark = this.dark;
    const P = id => { const p = pos.get(id); return [p[0] * W, p[1] * H]; };
    const faint = dark ? 'rgba(255,255,255,.34)' : (edges.length > 400 ? 'rgba(22,34,46,.12)' : 'rgba(22,34,46,.26)');
    const hov = this.hover;
    for (const e of edges) {
      const a = P(e.u), b = P(e.v), on = hov !== null && (e.u === hov || e.v === hov);
      c.beginPath(); c.moveTo(a[0], a[1]); c.lineTo(b[0], b[1]);
      c.setLineDash(e.kind === 'removed' ? [6, 5] : []);
      c.strokeStyle = e.kind === 'added' ? '#35C98A' : e.kind === 'removed' ? '#F0735B' : on ? (dark ? '#fff' : '#0F6B7D') : faint;
      c.lineWidth = e.kind ? 2.6 : on ? 2 : 1.2; c.stroke();
    }
    c.setLineDash([]); this.hit = [];
    const small = labelsAll || n <= 40;
    for (const nd of nodes) {
      const [px, py] = P(nd.id), rad = labelsAll ? 16 : small ? 9 + Math.sqrt(nd.degree) * 2 : 3 + Math.sqrt(nd.degree) * 1.5;
      this.hit.push({ id: nd.id, px, py, rad: Math.max(rad, 7), nd });
      c.beginPath(); c.arc(px, py, rad, 0, 6.2832);
      c.fillStyle = dark ? '#7FD0E0' : PAL[nd.community % PAL.length]; c.fill();
      if (top.has(nd.id) || hov === nd.id) { c.lineWidth = 3; c.strokeStyle = hov === nd.id ? (dark ? '#fff' : '#16222E') : '#B7791F'; c.stroke(); }
      if (small || top.has(nd.id)) {
        c.font = '600 12px "Segoe UI",system-ui,sans-serif'; c.textAlign = 'center'; c.textBaseline = 'middle';
        c.fillStyle = dark ? '#14202B' : '#fff'; if (rad >= 9) c.fillText(nd.id, px, py);
      }
    }
  }
  onMove(e) {
    const r = this.cv.getBoundingClientRect(), mx = e.clientX - r.left, my = e.clientY - r.top;
    let best = null, bd = 1e9;
    for (const h of this.hit) { const d = Math.hypot(h.px - mx, h.py - my); if (d < h.rad + 4 && d < bd) { best = h; bd = d; } }
    const id = best ? best.id : null;
    if (id !== this.hover) { this.hover = id; this.draw(); }
    if (best) {
      const nd = best.nd;
      this.tip.innerHTML = `Node ${nd.id} · degree ${nd.degree}` + (nd.pageRank !== undefined ? ` · PageRank ${f(nd.pageRank, 4)}` : '');
      this.tip.style.left = mx + 'px'; this.tip.style.top = my + 'px'; this.tip.style.display = 'block';
    } else this.tip.style.display = 'none';
  }
}

/* ---------- step chart (SVG) ---------- */
function stepChart(el, title, recs, key, digits, color, onSeek) {
  const W = 320, H = 150, L = 40, R = 8, T = 10, B = 24;
  const xs = recs.map(r => r.iteration), ys = recs.map(r => r[key]);
  const x0 = xs[0], x1 = xs[xs.length - 1] || 1;
  let lo = Math.min(...ys), hi = Math.max(...ys); if (lo === hi) { lo -= 0.5; hi += 0.5; }
  const pad = (hi - lo) * 0.12; lo -= pad; hi += pad;
  const X = v => L + ((v - x0) / ((x1 - x0) || 1)) * (W - L - R), Y = v => T + (1 - (v - lo) / (hi - lo)) * (H - T - B);
  let d = `M${X(xs[0])},${Y(ys[0])}`;
  for (let i = 1; i < xs.length; i++) d += `H${X(xs[i])}V${Y(ys[i])}`;
  const ticks = [lo + pad, (lo + hi) / 2, hi - pad];
  el.innerHTML = `<h3>${title}</h3><svg viewBox="0 0 ${W} ${H}" role="img" aria-label="${title} over MCMC iterations">
    ${ticks.map(t => `<line x1="${L}" x2="${W - R}" y1="${Y(t)}" y2="${Y(t)}" stroke="#E3E8ED"/><text x="${L - 6}" y="${Y(t) + 4}" text-anchor="end" font-size="10.5" fill="#7A8794">${f(t, digits)}</text>`).join('')}
    <text x="${L}" y="${H - 6}" font-size="10.5" fill="#7A8794">${x0}</text><text x="${W - R}" y="${H - 6}" text-anchor="end" font-size="10.5" fill="#7A8794">${x1} iterations</text>
    <path d="${d}" fill="none" stroke="${color}" stroke-width="2"/>
    <line class="cur" y1="${T}" y2="${H - B}" stroke="#16222E" stroke-width="1.2" stroke-dasharray="3 3"/><circle class="dot" r="4.5" fill="${color}" stroke="#fff" stroke-width="2"/></svg>`;
  const svg = $('svg', el), cur = $('.cur', svg), dot = $('.dot', svg);
  svg.addEventListener('click', e => {
    const r = svg.getBoundingClientRect(), vx = ((e.clientX - r.left) / r.width) * W;
    const t = x0 + ((vx - L) / (W - L - R)) * (x1 - x0);
    let bi = 0; xs.forEach((v, i) => { if (Math.abs(v - t) < Math.abs(xs[bi] - t)) bi = i; }); onSeek(bi);
  });
  return i => { const px = X(xs[i]); cur.setAttribute('x1', px); cur.setAttribute('x2', px); dot.setAttribute('cx', px); dot.setAttribute('cy', Y(ys[i])); };
}

/* ---------- rendering ---------- */
const tile = (v, l) => `<div class="tile"><b>${v}</b><span>${l}</span></div>`;
const yn = b => `<span class="badge ${b ? 'yes' : 'no'}">${b ? 'Chordal' : 'Not chordal'}</span>`;
let timer = null;

function render(d) {
  clearInterval(timer);
  const s = d.stream, sub = d.subgraph, m = d.mcmc, recs = m.records, last = recs[recs.length - 1], st = m.stats;
  const topSet = new Set(d.topK.map(t => t.node)), maxDeg = Math.max(...d.topK.map(t => t.degree), 1);
  const maxPR = Math.max(...d.nodes.map(n => n.pageRank), 1e-9);
  const approx = ok => ok ? '' : '≈';
  const finalChordal = sub.chordal || st.accepted > 0;
  const nodeById = new Map(d.nodes.map(n => [n.id, n]));
  const topByPR = [...d.nodes].sort((a, b) => b.pageRank - a.pageRank).slice(0, d.topK.length);

  $('#out').innerHTML = `
  <section class="stage"><h2><small>Stage 1</small>Stream ingestion</h2>
    <p class="lede">${s.edges} edges were read from the stream in ${f(d.meta.engineMs, 0)} ms and loaded into the C++ graph.</p>
    <div class="tiles">${tile(s.nodes, 'Nodes')}${tile(s.edges, 'Edges')}${tile(f(s.avgDegree), 'Average degree')}${tile(f(s.density), 'Density')}${tile(s.components, 'Connected components')}${tile(approx(s.chordal) + s.maxClique, 'Max clique')}${tile(yn(s.chordal), 'Whole graph')}</div>
    <div class="cv-wrap" id="cvStream"></div>
    <div class="legend"><span>Node size shows degree.</span><span><i style="background:${PAL[0]}"></i>Colour marks a connected component.</span><span><i class="ring"></i>Top-${d.topK.length} by degree</span><span>Hover a node for details.</span></div>
  </section>

  <section class="stage"><h2><small>Stage 2</small>Feature and component extraction</h2>
    <p class="lede">The highest-degree nodes feed the subgraph in the next stage. PageRank gives a second view of which nodes matter.</p>
    <div class="two"><div><h3>Top nodes by degree</h3>
      <table><thead><tr><th>Node</th><th>Degree</th><th class="n">PageRank</th></tr></thead><tbody>
      ${d.topK.map(t => `<tr><td>${t.node}</td><td><span class="bar" style="width:${(t.degree / maxDeg) * 90}px"></span>${t.degree}</td><td class="n">${f(nodeById.get(t.node).pageRank, 4)}</td></tr>`).join('')}</tbody></table></div>
      <div><h3>Highest PageRank</h3>
      <table><thead><tr><th>Node</th><th>Score</th></tr></thead><tbody>
      ${topByPR.map(n => `<tr><td>${n.id}</td><td><span class="bar" style="width:${(n.pageRank / maxPR) * 90}px;background:${PAL[1]}"></span>${f(n.pageRank, 4)}</td></tr>`).join('')}</tbody></table></div></div>
    <h3 style="margin-top:18px">${d.communities.length} connected component${d.communities.length === 1 ? '' : 's'}</h3>
    <div>${d.communities.slice(0, 12).map((c, i) => `<span class="chip"><i style="background:${PAL[i % PAL.length]}"></i>${c.length} nodes${c.length <= 6 ? ': ' + c.join(', ') : ''}</span>`).join('')}${d.communities.length > 12 ? `<span class="chip">+${d.communities.length - 12} more</span>` : ''}</div>
  </section>

  <section class="stage"><h2><small>Stage 3</small>Induced subgraph for chordal analysis</h2>
    <p class="lede">The subgraph keeps the top-${d.topK.length} nodes and every edge between them.</p>
    <div class="tiles">${tile(sub.nodes.length, 'Nodes')}${tile(sub.edges, 'Edges')}${tile(f(sub.density), 'Density')}${tile(approx(sub.chordal) + sub.maxClique, 'Max clique')}${tile(yn(sub.chordal), 'Subgraph')}</div>
    <div class="cv-wrap" id="cvSub"></div>
    ${sub.chordal ? '' : '<p class="note">This subgraph has a cycle of four or more nodes with no chord, so the chain starts outside the set of chordal graphs. It can only move once a proposal lands on a chordal graph, and it will never leave the chordal set after that.</p>'}
  </section>

  <section class="stage chain"><h2><small>Stage 4</small>MCMC walk over chordal graphs</h2>
    <p class="lede">Each step proposes adding, removing or toggling one edge. The C++ engine keeps the proposal only if the result is chordal. Drag the slider, press play, or click any chart to move through the chain.</p>
    <div class="cv-wrap" id="cvChain"></div>
    <div class="legend"><span><i class="ln" style="background:#35C98A"></i>Edge added since the previous record</span><span><i class="ln" style="background:#F0735B"></i>Edge removed</span><span>Seed ${m.seed}, recorded every ${m.logInterval} iterations</span></div>
    <div class="ctrl"><button type="button" class="play" id="play">Play</button><input type="range" id="scrub" min="0" max="${recs.length - 1}" value="0" aria-label="MCMC iteration"></div>
    <div class="readout"><div><b id="rIter">0</b><span>Iteration</span></div><div><b id="rEdges">0</b><span>Edges</span></div><div><b id="rDens">0</b><span>Density</span></div><div><b id="rClq">0</b><span>Max clique</span></div></div>
    <div class="charts" id="charts"></div>
    <div class="accept"><h3>What happened to the ${st.proposals} proposals</h3>
      <div class="stack">${[['accepted', st.accepted, '#1E8A5A'], ['rejected', st.rejected, '#C4452D'], ['no-op', st.noOps, '#B8C2CC']].map(([n, v, c]) => `<div style="flex:${v};background:${c}" title="${n}: ${v}"></div>`).join('')}</div>
      <div class="stack-key"><span><i style="background:#1E8A5A"></i>Accepted ${st.accepted} (${f(100 * st.accepted / (st.proposals || 1), 1)}%): result was chordal</span><span><i style="background:#C4452D"></i>Rejected ${st.rejected} (${f(100 * st.rejected / (st.proposals || 1), 1)}%): result was not chordal</span><span><i style="background:#B8C2CC"></i>No-op ${st.noOps} (${f(100 * st.noOps / (st.proposals || 1), 1)}%): move could not apply</span></div></div>
  </section>

  <section class="stage"><h2><small>Stage 5</small>Structural comparison</h2>
    <p class="lede">The raw stream, the extracted subgraph and the final state of the chain, side by side.</p>
    <table class="cmp"><thead><tr><th>Metric</th><th class="n">Raw stream</th><th class="n">Extracted subgraph</th><th class="n">Final MCMC state</th></tr></thead><tbody>
    <tr><td>Nodes</td><td class="n">${s.nodes}</td><td class="n">${sub.nodes.length}</td><td class="n">${sub.nodes.length}</td></tr>
    <tr><td>Edges</td><td class="n">${s.edges}</td><td class="n">${sub.edges}</td><td class="n">${last.edges}</td></tr>
    <tr><td>Density</td><td class="n">${f(s.density)}</td><td class="n">${f(sub.density)}</td><td class="n">${f(last.density)}</td></tr>
    <tr><td>Average degree</td><td class="n">${f(s.avgDegree)}</td><td class="n">${f(sub.avgDegree)}</td><td class="n">${f(last.avgDegree)}</td></tr>
    <tr><td>Max clique size</td><td class="n">${approx(s.chordal)}${s.maxClique}</td><td class="n">${approx(sub.chordal)}${sub.maxClique}</td><td class="n">${approx(finalChordal)}${last.maxClique}</td></tr>
    <tr><td>Chordal</td><td class="n">${s.chordal ? 'Yes' : 'No'}</td><td class="n">${sub.chordal ? 'Yes' : 'No'}</td><td class="n">${finalChordal ? 'Yes' : 'No'}</td></tr></tbody></table>
    <details><summary>About these numbers and the sampler</summary>
      <p>Max clique size comes from a maximum cardinality search ordering. It is exact for chordal graphs and only a lower-bound estimate otherwise, so non-chordal values carry a ≈ sign.</p>
      <p>The sampler accepts a proposal only when the candidate graph is chordal, so every accepted state is chordal. The remove-edge move picks a random vertex and then one of its edges, which is not symmetric with the add-edge move. The chain therefore explores chordal graphs but is not guaranteed to sample them uniformly without a Metropolis-Hastings correction.</p></details>
  </section>`;

  /* stream + subgraph graphs */
  const adj = new Map(d.nodes.map(n => [n.id, n]));
  const sg = new GraphCanvas($('#cvStream'), 440);
  sg.set({ nodes: d.nodes, edges: s.edgeList.map(([u, v]) => ({ u, v })), pos: forceLayout(d.nodes.map(n => n.id), s.edgeList), top: topSet }, 'Streamed graph');
  const subNodes = sub.nodes.map(id => { const n = adj.get(id); return { ...n, degree: sub.edgeList.filter(e => e[0] === id || e[1] === id).length }; });
  const subPos = circleLayout(sub.nodes);
  new GraphCanvas($('#cvSub'), 300).set({ nodes: subNodes, edges: sub.edgeList.map(([u, v]) => ({ u, v })), pos: subPos, labelsAll: true }, 'Induced subgraph');

  /* MCMC viewer */
  const chain = new GraphCanvas($('#cvChain'), 380, true);
  const key = ([u, v]) => u + '-' + v;
  const seek = i => {
    const r = recs[i], prev = i > 0 ? recs[i - 1] : r, cur = new Set(r.edgeList.map(key)), old = new Set(prev.edgeList.map(key));
    const edges = r.edgeList.map(([u, v]) => ({ u, v, kind: i > 0 && !old.has(key([u, v])) ? 'added' : '' }));
    if (i > 0) prev.edgeList.forEach(([u, v]) => { if (!cur.has(key([u, v]))) edges.push({ u, v, kind: 'removed' }); });
    const deg = new Map(); r.edgeList.forEach(([u, v]) => { deg.set(u, (deg.get(u) || 0) + 1); deg.set(v, (deg.get(v) || 0) + 1); });
    chain.set({ nodes: subNodes.map(n => ({ ...n, degree: deg.get(n.id) || 0 })), edges, pos: subPos, labelsAll: true }, `Chain state at iteration ${r.iteration}`);
    $('#rIter').textContent = r.iteration; $('#rEdges').textContent = r.edges; $('#rDens').textContent = f(r.density); $('#rClq').textContent = r.maxClique;
    $('#scrub').value = i; cursors.forEach(c => c(i)); idx = i;
  };
  let idx = 0; const cursors = [];
  const specs = [['Edges', 'edges', 0, '#0F6B7D'], ['Density', 'density', 3, '#B7791F'], ['Average degree', 'avgDegree', 2, '#7A4E9C'], ['Max clique size', 'maxClique', 0, '#B5483A']];
  specs.forEach(([t, k, dg, col]) => { const el = document.createElement('div'); el.className = 'chart'; $('#charts').appendChild(el); cursors.push(stepChart(el, t, recs, k, dg, col, seek)); });
  $('#scrub').addEventListener('input', e => { stop(); seek(+e.target.value); });
  const stop = () => { clearInterval(timer); timer = null; $('#play').textContent = 'Play'; };
  $('#play').addEventListener('click', () => {
    if (timer) return stop();
    if (idx >= recs.length - 1) seek(0);
    $('#play').textContent = 'Pause';
    timer = setInterval(() => { if (idx >= recs.length - 1) return stop(); seek(idx + 1); }, 350);
  });
  seek(0);
}

/* ---------- app wiring ---------- */
function clustered() {
  let s = 12345; const rnd = () => (s = (s * 1664525 + 1013904223) >>> 0) / 4294967296;
  const out = [], C = 5, S = 12;
  for (let c = 0; c < C; c++) for (let i = 0; i < S; i++) for (let j = i + 1; j < S; j++) if (rnd() < 0.3) out.push(`${c * S + i + 1},${c * S + j + 1}`);
  for (let k = 0; k < 14; k++) out.push(`${Math.floor(rnd() * C * S) + 1},${Math.floor(rnd() * C * S) + 1}`);
  return out.filter(e => { const [a, b] = e.split(','); return a !== b; }).join('\n');
}
async function run() {
  const btn = $('#run'), err = $('#error'); err.hidden = true; btn.disabled = true; btn.textContent = 'Running…';
  try {
    const res = await fetch('/api/analyze', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({
      edges: $('#edges').value, topK: +$('#topK').value, iterations: +$('#iterations').value, logInterval: +$('#logInterval').value, seed: +$('#seed').value }) });
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'Request failed.');
    render(data);
    $('#out').firstElementChild.scrollIntoView({ block: 'start' });
  } catch (e) { err.textContent = e.message; err.hidden = false; }
  btn.disabled = false; btn.textContent = 'Run analysis';
}
$('#edges').value = EXAMPLE;
$('#run').addEventListener('click', run);
$('#loadExample').addEventListener('click', () => { $('#edges').value = EXAMPLE; });
$('#loadClustered').addEventListener('click', () => { $('#edges').value = clustered(); });
$('#newSeed').addEventListener('click', () => { $('#seed').value = Math.floor(Math.random() * 1e6); });
$('#file').addEventListener('change', e => { const fl = e.target.files[0]; if (fl) fl.text().then(t => { $('#edges').value = t; }); });
fetch('/api/health').then(r => r.json()).then(h => {
  const el = $('#status'); el.classList.add(h.engine ? 'ok' : 'bad');
  $('#statusText').textContent = h.engine ? 'C++ engine connected' : 'Server up, C++ engine not built yet';
}).catch(() => { $('#status').classList.add('bad'); $('#statusText').textContent = 'Server not reachable'; });
